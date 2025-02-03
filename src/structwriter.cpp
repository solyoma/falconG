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
			ofs << EncodeText(text[i]);
		ofs << "]";
		if (id && !i)					// only write text ID for first language
			ofs << "*" << id;

		ofs << "\n";

		//if (id & ~BASE_ID_MASK)			// then collision: save level
		//	ofs << "*" << (id >> TEXT_ID_COLLISION_FACTOR)  << "\n";
	}
}




//***************************** class AlbumStructWriter ****************
AlbumStructWriter::AlbumStructWriter(AlbumGenerator& generator, bool recordChanges, QObject* parent) :
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
void AlbumStructWriter::run()
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
		<< "\n#  © - András Sólyom (2018-)" << QString().setNum(PROGRAM_CONFIG::copyrightYear)  // default values may differ from 'config'
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
	_WriteAlbums(_albumMap[TOPMOST_ALBUM_ID], indent);
	_ofs.flush();
	f.close();

	emit resultReady(result, sStructPath, sStructTmp);
}

void AlbumStructWriter::_WriteImageRecord(Image* pImg, QString indent)
{
	QString s;
	QString delim = QStringLiteral("|"),
		ddelim = QStringLiteral("||");
	if (pImg->Exists())
	{
		if (!pImg->rsize.width())	// transformed size is 0, if we did not process images
			pImg->rsize = pImg->dsize;

		s +=indent;
		if (pImg->dontResize)
			s +="!!";
		s += pImg->name  															// field #1
			+ ddelim 
			+ pImg->ID.ValToString()												// field #2
			+ delim
			+ QString("%1x%2").arg(pImg->rsize.width()).arg(pImg->rsize.height())	// field #3 - #4
			+ delim
			+ QString("%1x%2").arg(pImg->osize.width()).arg(pImg->osize.height())	// field #5 - #6
			+ delim
			// ISO 8601 extended format: yyyy-MM-dd for dates
			+ pImg->uploadDate.toString(Qt::ISODate) 								// field #7
			+ delim
			+ QString().setNum(pImg->fileSize)										// field #8
			+ delim
			+ QString("%1\n").arg(pImg->pathId);									// field #9

		_ofs << s;
	}
	else
		_ofs << indent << pImg->name << " # is missing\n";
}

void AlbumStructWriter::_WriteStructImage(Album& album, ID_t id, QString indent)
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

void AlbumStructWriter::_WriteVideo(Album& album, ID_t id, QString indent)
{
	QString s;
	QString delim = QStringLiteral("|"),
		ddelim = QStringLiteral("||");
	Video* pVid;

	pVid = &_videoMap[id];
	if (pVid->changed)
	{
		albumgen.SetAlbumModified(album);
	}
	if (pVid->Exists())
	{
		s = indent +
			pVid->name + ddelim +								   // field #1
			pVid->ID.ValToString() + delim +					   // field #2
			// ISO 8601 extended format: yyyy-MM-dd for dates
		    pVid->uploadDate.toString(Qt::ISODate) + delim		   // field #3
			+ QString().setNum(pVid->fileSize) + delim			   // field #4
			+ QString("%1\n").arg(pVid->pathId);
		_ofs << s;
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
void AlbumStructWriter::_WriteImagesThenSubAlbums(Album& album, QString indent)
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
					_WriteVideo(album, id, indent);
	}

	if (album.SubAlbumCount())
	{
		for (ID_t id : album.items)
			if (!id.IsExcluded() && id.IsAlbum())		// not excluded
				_WriteAlbums(_albumMap[id], indent);
	}
}

/*============================================================================
* TASK:		write the text for a sub album into the '.struct' file
*			First the images, then the sub albums will be written
* EXPECTS: album  - to write
*		   indent - level of this album in the hierarchy
* GLOBALS:
* REMARKS: co-routine with '_WriteImagesThenSubAlbums'
*			if the sub album has sub albums then this function is called
*			again from '_WriteImagesThenSubAlbums'
*--------------------------------------------------------------------------*/
void AlbumStructWriter::_WriteAlbums(Album& album, QString indent)
{
	ID_t thumbnail = album.thumbnailId = AlbumGenerator::ThumbnailID(album, _albumMap);		// thumbnail may have been a folder
																// name  originally, now it is an ID
	_ofs << "\n" << indent
		<< album.name << (album.changed ? "(C:" : "(A:") << album.ID.ValToString()
		<< ")" << album.pathId << "\n"; // always write album unchanged, but do not modify' changed' flag
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, album.titleID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, album.descID, indent);

	_ofs << indent << "[" << THUMBNAIL_TAG << ":" << thumbnail.Val() << "]\n"; // ID may be 0!

	_WriteImagesThenSubAlbums(album, indent + " ");
}

void AlbumStructWriter::_WritePathsTable()
{
	_ofs << PATH_TABLE << "\n" << pathMap << "]\n\n";
}

void AlbumStructWriter::_WriteLanguageTable()
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

void AlbumStructWriter::_WriteOrphanThumbnails()
{
	_ofs << ORPHAN_ID << "\n";
   for(auto &img : _imageMap)
	   if (img.IsOrphan())
	   {
		   Image* pImg;
		   QString s;

		   pImg = &_imageMap[img.ID];
		   _WriteImageRecord(pImg, 0);
	   }
   _ofs << "]\n\n";
}


