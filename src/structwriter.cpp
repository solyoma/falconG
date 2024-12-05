#include "structwriter.h"


/*============================================================================
* TASK:		write texts for all languages into structure file
* EXPECTS: ofs: write here
*			texts:	from this map
*			what: 	write 'T'itle or 'D'escription
*			id:		with this id
*			indent: after this many spaces
* GLOBALS: config.bAddTitlesToAll and config.AddDescriptionsToAll
* REMARKS:	If there is no text (id == 0) and need not write empty text holder
*			then just returns
*--------------------------------------------------------------------------*/
static void WriteStructLanguageTexts(QTextStream& ofs, TextMap& texts, QString what, uint64_t id, QString indent)
{
	bool b = what[0] == 'T' ? config.bAddTitlesToAll : config.bAddDescriptionsToAll;

	if (id == 0 && !b)		// no text
		return;
	LanguageTexts& text = texts[id];

	for (int i = 0; i < languages.LanguageCount(); ++i)
	{
		ofs << indent << "[" << what << (*languages["language"])[i] << ":";
		if (id != 0)
			ofs << text[i];
		ofs << "]";
		if (id && !i)					// only write text ID for first language
			ofs << "*" << id;

		ofs << "\n";

		//if (id & ~BASE_ID_MASK)			// then collision: save level
		//	ofs << "*" << (id >> TEXT_ID_COLLISION_FACTOR)  << "\n";
	}
}




//***************************** class AlbumStructWriterThread ****************
AlbumStructWriterThread::AlbumStructWriterThread(AlbumGenerator& generator, bool recordChanges, QObject* parent) :
	_textMap(generator.Texts()), _albumMap(generator.Albums()), _imageMap(generator.Images()), 
	_videoMap(generator.Videos()), _recordChanges(recordChanges),
	QThread(parent)
{
}

/*============================================================================
* TASK:		start writing the .struct file in a separate thread
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumStructWriterThread::run()
{
	QString result;
	QMutexLocker locker(&_albumMapStructIsBeingWritten);
			// mutex is locked and will bw unlocked when function returns

	QString p, n;
	SeparateFileNamePath(config.dsSrc.ToString(), p, n);

	QString sStructPath = config.dsSrc.ToString() + n + QString(".struct"),
		sStructTmp = config.dsSrc.ToString() + n + QString(".tmp");
	QFile f(sStructTmp);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		// error
		result = "Can't write file";
		emit resultReady(result, sStructPath, sStructTmp);
		return;
	}
	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << versionStr << majorStructVersion << "." << minorStructVersion << "." << subStructVersion
		<< "\n#  © - András Sólyom (2018-2024)"  // default values may differ from 'config'
		<< "\n\n#Created at " << QDateTime::currentDateTime().toString(Qt::ISODate)
		<< "\n\n#Source=" << config.dsSrc.ToString()
		<< "\n#Destination=" << config.dsGallery.ToString()
		<< "\n\n#Image rectangle: " << config.imageWidth << "x" << config.imageHeight
		<< "\n#Thumb rectangle: " << config.thumbWidth << "x" << config.thumbHeight << "\n\n";
	_WriteLanguageTable();
	_WritePathsTable();
	_WriteOrphanThumbnails();
	_ofs << "# Album structure:\n";

	QString indent;	// for directory structure file: indent line with spaces to indicate hierarchy
	_WriteStructAlbums(_albumMap[TOPMOST_ALBUM_ID], indent);
	_ofs.flush();
	f.close();

	emit resultReady(result, sStructPath, sStructTmp);
}

void AlbumStructWriterThread::_WriteImageRecord(Image* pImg, QString indent)
{
	if (pImg->Exists())
	{
		if (!pImg->rsize.width())	// transformed size is 0, if we did not process images
			pImg->rsize = pImg->dsize;

		_ofs << indent;
		if (pImg->dontResize)
			_ofs << "!!";
		_ofs << pImg->name << "||" 										   // field #1
			<< (pImg->ID.Val()) << "|"								   // field #2
			<< pImg->rsize.width() << "x" << pImg->rsize.height() << "|"   // field #3 - #4
			<< pImg->osize.width() << "x" << pImg->osize.height() << "|"   // field #5 - #6
			// ISO 8601 extended format: yyyy-MM-dd for dates
			<< pImg->uploadDate.toString(Qt::ISODate) << "|"			   // field #7
			<< pImg->fileSize	<< "|"									   // field #8
			<< pImg->pathId		<< "\n";
		// field #9
	}
	else
		_ofs << indent << pImg->name << " # is missing\n";
}

void AlbumStructWriterThread::_WriteStructImage(Album& album, ID_t id, QString indent)
{
	Image* pImg;
	QString s;

	pImg = &_imageMap[id];
	if (pImg->changed)
		albumgen.SetAlbumModified(album);

	_WriteImageRecord(pImg, indent);

	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, pImg->descID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, pImg->titleID, indent);
}

void AlbumStructWriterThread::_WriteStructVideo(Album& album, ID_t id, QString indent)
{
	Video* pVid;
	QString s;

	pVid = &_videoMap[id];
	if (pVid->changed)
	{
		albumgen.SetAlbumModified(album);
	}
	if (pVid->Exists())
	{
		_ofs << indent;
		_ofs << pVid->name << "(V"									   // field #1
			<< pVid->ID.Val()	<< ","								   // field #2
			// ISO 8601 extended format: yyyy-MM-dd for dates
			<< pVid->uploadDate.toString(Qt::ISODate) << ","		   // field #3
			<< pVid->fileSize << ")"								   // field #4
			<< pVid->pathId		<< "\n";
		// field #9
	}
	else
		_ofs << indent << pVid->name << " # is missing\n";
	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, pVid->descID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, pVid->titleID, indent);
}

/*============================================================================
* TASK:		writes out the new album structure.
* EXPECTS:
* GLOBALS:
* REMARKS: - for every album first write the images or videos, 
*			 followed by the sub albums:
*             album
*                image 1  in album
*					...
*                album 1
*					image 1 in album 1
*						...
*					a;bum 2 in album 1
*					***
*			- this is a co routin with '_WriteStructAlbum'
*--------------------------------------------------------------------------*/
void AlbumStructWriterThread::_WriteStructImagesThenSubAlbums(Album& album, QString indent)
{
	// IMAGES
	// format: [!!]<name>'('<id>,<width>'x'<height>,<owidth>'x'<oheight>,<date string><file size>')'<dSrc relative or absolute path>
	if (album.ImageCount() || album.VideoCount())
	{
		for (ID_t id : album.items)
			if (!id.IsExcluded())
				if (id.IsImage())		// not excluded
					_WriteStructImage(album, id, indent);	// DEBUG
				else if (id.IsVideo())
					_WriteStructVideo(album, id, indent);
	}

	if (album.SubAlbumCount())
	{
		for (ID_t id : album.items)
			if (!id.IsExcluded() && id.IsAlbum())		// not excluded
				_WriteStructAlbums(_albumMap[id], indent);
	}
}

/*============================================================================
* TASK:		write the text for a sub album into the '.struct' file
*			First the images, then the sub albums will be written
* EXPECTS: album  - to write
*		   indent - level of this album in the hierarchy
* GLOBALS:
* REMARKS: co-routine with '_WriteStructImagesThenSubAlbums'
*			if the sub album has sub albums then this function is called
*			again from '_WriteStructImagesThenSubAlbums'
*--------------------------------------------------------------------------*/
void AlbumStructWriterThread::_WriteStructAlbums(Album& album, QString indent)
{
	//if (album.Exists() == exNot)
	//	return;

	// QString s = config.RemoveSourceFromPath(pathMap[album.pathId]);

	ID_t thumbnail = album.thumbnailId = AlbumGenerator::ThumbnailID(album, _albumMap);		// thumbnail may have been a folder
																// name  originally, now it is an ID
	_ofs << "\n" << indent
		<< album.name << ( album.changed ? "(C:" :"(A:") << album.ID.Val() << ")" << /*s*/ album.pathId << "\n"; // always write album unchanged, but do not modify' changed' flag
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, album.titleID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, album.descID, indent);

	_ofs << indent << "[" << THUMBNAIL_TAG << ":" << thumbnail.Val() << "]\n"; // ID may be 0!

	_WriteStructImagesThenSubAlbums(album, indent + " ");
}

void AlbumStructWriterThread::_WritePathsTable()
{
	_ofs << PATH_TABLE << "\n" << pathMap << "]\n\n";
}

void AlbumStructWriterThread::_WriteLanguageTable()
{
	_ofs << "\n\n[Language count:" << languages.LanguageCount() << "\n";
	for (int i = 0; i < languages.LanguageCount(); ++i)
	{
		_ofs << i << "\n";
		for (auto it = languages.begin(); it != languages.end(); ++it)
			_ofs << " " << it.key() << "=" << (*it.value())[i] << "\n";
	}
	_ofs << "]\n\n";
}

void AlbumStructWriterThread::_WriteOrphanThumbnails()
{
	_ofs << ORPHAN_ID << "\n";
   for(auto &img : _imageMap)
	   if (img.ID.IsOrphan())
	   {
		   Image* pImg;
		   QString s;

		   pImg = &_imageMap[img.ID];
		   _WriteImageRecord(pImg, 0);
	   }
   _ofs << "]\n\n";
}


