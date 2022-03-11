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
static void WriteStructLanguageTexts(QTextStream& ofs, TextMap& texts, QString what, ID_t id, QString indent)
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
		//	ofs << "*" << (id >> ID_COLLISION_FACTOR)  << "\n";
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
		<< "\n#  © - András Sólyom (2018-2022)"  // default values may differ from 'config'
		<< "\n\n#Created at " << QDateTime::currentDateTime().toString(Qt::ISODate)
		<< "\n\n#Source=" << config.dsSrc.ToString()
		<< "\n#Destination=" << config.dsGallery.ToString()
		<< "\n\n#Image rectangle: " << config.imageWidth << "x" << config.imageHeight
		<< "\n#Thumb rectangle: " << config.thumbWidth << "x" << config.thumbHeight
		<< "\n\n[Language count:" << languages.LanguageCount() << "\n";
	for (int i = 0; i < languages.LanguageCount(); ++i)
	{
		_ofs << i << "\n";
		for(auto it=languages.begin(); it != languages.end(); ++it)
			_ofs << " " << it.key() << "=" << (*it.value())[i] << "\n";
	}
	_ofs << "]\n\n# Album structure:\n";

	QString indent;	// for directory structure file: indent line with spaces to indicate hierarchy
	_WriteStructAlbums(_albumMap[ROOT_ALBUM_ID], indent);
	_ofs.flush();
	f.close();

	emit resultReady(result, sStructPath, sStructTmp);
}

/*============================================================================
* TASK:		writes out the new album structure.
* EXPECTS:
* GLOBALS:
* REMARKS: - for every album first there come the images, then the sub albums:
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
	QString s;
	Image* pImg;
	Video* pVid;

	// IMAGES
	// format: [!!]<name>'('<id>,<width>'x'<height>,<owidth>'x'<oheight>,<date string><file size>')'<dSrc relative or absolute path>
	for (ID_t id : album.items)
		if(!(id & EXCLUDED_FLAG) && (id & IMAGE_ID_FLAG) )		// not excluded
		{
			pImg = &_imageMap[id];
			if (pImg->changed)
				album.changed = true;

			if (pImg->exists)
			{
				if (!pImg->rsize.width())	// transformed size is 0, if we did not process images
					pImg->rsize = pImg->dsize;

				_ofs << indent;
				if (pImg->dontResize)
					_ofs << "!!";
				_ofs << pImg->name << "(" 										   // field #1
					<< (pImg->ID & ID_MASK) << ","								   // field #2
					<< pImg->rsize.width() << "x" << pImg->rsize.height() << ","   // field #3 - #4
					<< pImg->osize.width() << "x" << pImg->osize.height() << ","   // field #5 - #6
					// ISO 8601 extended format: yyyy-MM-dd for dates
					<< pImg->uploadDate.toString(Qt::ISODate) << ","			   // field #7
					<< pImg->fileSize << ")";									   // field #8
				s = config.RemoveSourceFromPath(pImg->path);
				_ofs << s << "\n";
				// field #9
			}
			else
				_ofs << indent << pImg->name << " # is missing\n";
			WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, pImg->descID, indent);
			WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, pImg->titleID, indent);
		}
	// DEBUG
//	int n = album.videos.size();
//	if (n)
//		n = n;
	// ?DEBUG
	// VIDEOS
	// format: <name>'(''V'<id>, <date string>,<file size>')'<dSrc relative or absoluth path>
	for (ID_t id : album.items)
		if (!(id & EXCLUDED_FLAG) && (id & VIDEO_ID_FLAG))		// not excluded
		{
			pVid = &_videoMap[id];
			if (pVid->changed)
			{
				album.changed = true;
			}
			if (pVid->exists)
			{

				_ofs << indent;
				_ofs << pVid->name << "(V" 										   // field #1
					<< (pVid->ID & ID_MASK) << ","								   // field #2
					// ISO 8601 extended format: yyyy-MM-dd for dates
					<< pVid->uploadDate.toString(Qt::ISODate) << ","			   // field #3
					<< pVid->fileSize << ")";									   // field #4
				s = pVid->path;
				s = config.RemoveSourceFromPath(pVid->path);
				_ofs << s << "\n";
				// field #9
			}
			else
				_ofs << indent << pVid->name << " # is missing\n";
			WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, pVid->descID, indent);
			WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, pVid->titleID, indent);
		}

	// ALBUMS
	for (ID_t id : album.items)
		if (!(id & EXCLUDED_FLAG) && (id & ALBUM_ID_FLAG))		// not excluded
			_WriteStructAlbums(_albumMap[id], indent);
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
	if (!album.exists)
		return;

	QString s = config.RemoveSourceFromPath(album.path);

	ID_t thumbnail = album.thumbnail = AlbumGenerator::ThumbnailID(album, _albumMap);		// thumbnail may have been a folder
																// name  originally, now it is an ID
	_ofs << "\n" << indent
		<< album.name << ( album.changed ? "(C:" :"(A:") << (album.ID & ID_MASK) << ")" << s << "\n"; // always write album unchanged, but do not modify' changed' flag
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, album.titleID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, album.descID, indent);

	_ofs << indent << "[" << THUMBNAIL_TAG << ":" << (thumbnail & ID_MASK) << "]\n"; // ID may be 0!

	_WriteStructImagesThenSubAlbums(album, indent + " ");
}
