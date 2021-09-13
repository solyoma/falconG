#include <QApplication>
#include <QApplication>
#include <QtCore>
#include <QString>
#include <QTextCodec>
#include <time.h>
#include <atomic>
#include <chrono>

#include "albums.h"
#include "falcong.h"
#include "csscreator.h"
#include "structwriter.h"

#if QT_VERSION < 0x051000
    #define created created
#endif


//******************************************************
AlbumGenerator	albumgen;		// global

//******************************************************
/////////////////////////
struct BadStruct 
{ 
	QString msg;
	int cnt;  
	BadStruct(int lineNo, QString msg) : msg(msg),cnt(lineNo) {}
};		// simple exception class
////////////////////////////

// static members
LanguageTexts TextMap::invalid;
Image ImageMap::invalid;
Video VideoMap::invalid;
QString AlbumGenerator::lastUsedItemPath;

Album AlbumMap::invalid;

// used directories put here and not into class 'AlbumGenerator'
// because of include order
// defaults of these are not set, str always valid
_CDirStr	__DestDir,		// generate gallery into _RootDir inside this
			__RootDir,		// .htaccess, constants.php, index.php
			__AlbumDir,		// for all albums
			__CssDir,		// for falconG.css
			__ImageDir,		// for all images
			__ThumbDir,		// for all images
			__VideoDir,		// for all videos
			__FontDir,		// for some of used fonts
			__JsDir,		// for falconG.js
			__ResourceDir;	// for icons

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*--------------------------------------------------------------------------*/
static void __SetBaseDirs()
{
	//   Directory contains
	__RootDir = config.dsGallery + config.dsGRoot;	// .htaccess, constants.php, index.php
	__AlbumDir = __RootDir + config.dsAlbumDir;		// all albums
	__CssDir = __RootDir + config.dsCssDir;	  		// falconG.css
	__ImageDir = __RootDir + config.dsImageDir;		// contains all images
	__ThumbDir = __RootDir + config.dsThumbDir;		// contains all thumbnails
	__VideoDir = __RootDir + config.dsVideoDir;		// contains all videos
	__FontDir = __RootDir + config.dsFontDir;		// some of used fonts
	__ResourceDir = __RootDir + _CDirStr("res/", "dirstr");	// icons
	__JsDir = __RootDir + _CDirStr("js/", "dirstr");			// falconG.js
}

/*============================================================================
* TASK:		gets image ID for album thumbnail recursively
* EXPECTS: album - album.thumbnail is an ID of an image or another album or 0
*		   albums: map for finding the album which is a thumbnail in
* GLOBALS:
* REMARKS:	- may modify 'album'
*			- if the album thumbnail is the path of an image its thumbnail
*			  will be the ID of that image
*			- if the thumbnail is 0	and the album has images than its thumbnail
*				will be the first of its images
*			- if the thumbnail is 0	and the album has sub-albums than its thumbnail
*				will be the thumnbnail of the first of its sub-albums if 
*				the sub-album has one, or continue to search for a thumbnail
*				recursively
*			- if the thumbnail is 0	and the album contains no images and 
*				no asub-albums, then its thumbnail will be 0
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::ThumbnailID(Album & album, AlbumMap& albums)
{
	if (album.thumbnail == 0) // then the first image  or sub-album
	{						  // will be set as its thumbnail
		if (!album.ImageCount() && !album.VideoCount() && !album.SubAlbumCount())
			return 0;
		if (!album.ImageCount() && !album.VideoCount()) // then get thumbnail for first sub-album
			return ThumbnailID(albums[album.IdOfItemOfType(ALBUM_ID_FLAG,0,0)], albums);
		else
			return album.IdOfItemOfType(IMAGE_ID_FLAG | VIDEO_ID_FLAG, 0, 0);
	}
	return album.thumbnail;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
static QDate DateFromString(QString s)
{
	return QDate(s.mid(0, 4).toInt(), s.mid(5, 2).toInt(), s.mid(8, 2).toInt());
}

/*============================================================================
* TASK:		 Template to calculate CRC of either a text string or of a file content
* EXPECTS:	 stsr      - string to calculate CRC32 of or file name
*			 isContent - calculate content of file named as 'str'
* GLOBALS:	none
* RETURNS:  crc 32
* REMARKS:	only returns 0 if str is empty
*--------------------------------------------------------------------------*/
ID_t CalcCrc(QString &str, bool isContent)
{
	ID_t id;
	if (isContent)							// file contente
	{
		CRC32 crc(str);
		id = crc.crc();
	}
	else									//  just the text
	{
		CRC32 crc(str);
		id = crc.crc();
	}
	if (!id && !str.isEmpty())
		id = ID_INCREMENT;		// id == 0 is invalid id
	return id;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
template<typename T> ID_t GetUniqueID(T &t, QString &name, bool isContent = false)
{

	ID_t id = CalcCrc(name, isContent);
	while (t.contains(id))
	{
		id += ID_INCREMENT;
	}
	return id;
}

/**************************** LanguageTexts *****************************/
Image::SearchCond Image::searchBy = byID;		// 0: by ID, 1: by name, 2 by full name
Video::SearchCond Video::searchBy = byID;		// 0: by ID, 1: by name, 2 by full name
Album::SearchCond Album::searchBy = byID;

/**************************** LanguageTexts *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
LanguageTexts::LanguageTexts(const QStringList &texts)
{
	*this = texts;
}

/*=============================================================
 * TASK:	set text for language into 'texts'
 * EXPECTS: str - text to set
 *			lang: for this language
 * GLOBALS:
 * RETURNS:	base ID of text
 * REMARKS:
 *------------------------------------------------------------*/
void LanguageTexts::SetTextForLanguageNoID(const QString str, int lang)
{
	if (lang < 0 || lang > Languages::Count())
		return;
QString stmp = EncodeLF(str.trimmed());
	int len = stmp.length();
	if (lenghts.isEmpty())
		Clear(Languages::Count());
	QString s;
	for (int i = 0; i < Languages::Count(); ++i)
		if (i == lang)
			s += stmp;
		else
			s += operator[](i);
	textsForAllLanguages = s;
	lenghts[lang] = len;
}

/*=============================================================
* TASK:	set text for language into 'texts'
* EXPECTS: str - text to set
*			lang: for this language
* GLOBALS:
* RETURNS:	base ID of text
* REMARKS:
*------------------------------------------------------------*/
//ID_t LanguageTexts::SetText(const QString str, int lang)
//{
//	if (lang < 0 || lang > Languages::Count())
//		return 0;
//	int len = str.length();
//	QString s;
//	for (int i = 0; i < Languages::Count(); ++i)
//		if (i == lang)
//			s += str;
//		else
//			s += operator[](lang);
//	textsForAllLanguages = s;
//	lenghts[lang] = len;
//	return CalcID(albumgen.Texts());
//}

/*============================================================================
* TASK:		Calculate base ID for a given text
* EXPECTS:  txts - list aof texts in all languages used
* GLOBALS:
* REMARKS: 	with empty texts the result is 0, otherwise
*			- the ID is the CRC32 of the longest of the language texts
*              unless if that is 0, when ID_INCREMENT will bw the base ID
*			- in the latter case sets collision to 1
*--------------------------------------------------------------------------*/
ID_t LanguageTexts::CalcBaseID()
{
	CRC32 crc(textsForAllLanguages);			// same ID for all languages
	ID = crc.crc();
	if (!ID)					// CRC was 0, but ID can't be
	{
		ID = ID_INCREMENT;
		collision = 1;			// not really a collison, but hey
	}
	return ID;
}

/*============================================================================
  * TASK:	Calculate and set unique ID for text in TextMap 'map'
  * EXPECTS: 'map' get the ID relative to this
  * RETURNS: unique ID of text. If usageCount is 0 the text is new
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
ID_t LanguageTexts::CalcID(TextMap & map)
{
	if (config.majorStructVersion == 1)
	{
		if (config.minorStructVersion == 0 || collision < 0)
		{          // otherwise ID is already OK
			CalcBaseID();
			int refCount = 0;
			if (collision < 0)	// set in 'Clear()  - othervise minor STRUCT version is 0
				collision = map.Collision(*this, refCount);
			ID += ((ID_t)collision << ID_COLLISION_FACTOR);
			usageCount = refCount;
		}
	}
	return ID;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void LanguageTexts::Clear(int newSize)
{
	if (newSize > 0 && newSize != lenghts.size())
	{
		lenghts.clear();
		for (int i = 0; i < newSize; ++i)
			lenghts.push_back(0);
	}
	else
		for (int i = 0; i < lenghts.size(); ++i)
			lenghts[i] = 0;
	textsForAllLanguages.clear();
	ID = 0;
	collision = -1;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
LanguageTexts &LanguageTexts::operator=(const LanguageTexts &t)
{
	ID = t.ID; 
	collision = t.collision;
//	lang = t.lang;
	textsForAllLanguages = t.textsForAllLanguages;
	lenghts = t.lenghts;
	usageCount = t.usageCount;
	return *this;
}
/*============================================================================
* TASK:	  set up text from string list and get its base ID
* EXPECTS: txts - stringlist with as many elements as there are languages
* GLOBALS:
* REMARKS: 
*--------------------------------------------------------------------------*/
LanguageTexts &LanguageTexts::operator=(const QStringList &txts)
{
	lenghts.clear();
	textsForAllLanguages.clear();
	QString s;
	for (int i = 0; i < txts.size(); ++i)
	{
		s = EncodeLF(txts[i]);
		lenghts.push_back(s.length());
		textsForAllLanguages += txts[i];
	}
	
	ID = CalcBaseID(); // just base ID  (no collision test)

	return *this;
}

/*============================================================================
* TASK:		'less then' operator
* EXPECTS:	t - other text with ID set up
* GLOBALS:
* REMARKS:	does not use lang
*--------------------------------------------------------------------------*/
int LanguageTexts::operator<(const LanguageTexts &t)
{
	return ID < t.ID;
}

/*============================================================================
* TASK:		text equality operator for texts using text id and the
 *				texts themselves
 * EXPECTS:	t - other text, id is set up already
 *			this and t has the same number of language texts
 * GLOBALS:
 * REMARKS: 
*--------------------------------------------------------------------------*/
bool LanguageTexts::operator==(const LanguageTexts &t)
{
	if (ID != t.ID || lenghts != t.lenghts || textsForAllLanguages != t.textsForAllLanguages)
		return false;
	else
		return true;
}

/*============================================================================
* TASK:		text inequality operator for texts using text id and lang
* EXPECTS:	t - other text, both id and lang is set up already
* GLOBALS:
* REMARKS: if t.lang < 0 only checks the ID, otherwise it also checks lang
*--------------------------------------------------------------------------*/
bool LanguageTexts::operator!=(const LanguageTexts & t)
{
	return !(*this == t);
}

//***********************************************
//--------- TextMap -----------------------------
//***********************************************

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
LanguageTexts &TextMap::Find(ID_t id)
{
	return contains(id) ? (*this)[id] : invalid;  // value(id, invalid);			// uses id only (lang = -1)
}

/*============================================================================
* TASK:		find search string in text with language code lang
* EXPECTS:	'search' search string in language 'lang'
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
LanguageTexts & TextMap::Find(QString search, int lang)
{
	QStringList qsl;
	for (int i = 0; i < Languages::Count(); ++i)
		if (i == lang)
			qsl << search;
		else
			qsl << "";

	LanguageTexts text(qsl);	// sets id from longest text, which is now 'search'

	return contains(text.ID) ? (*this)[text.ID] : invalid;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
LanguageTexts& TextMap::Find(const QStringList & texts)
{
	LanguageTexts text(texts);
	return contains(text.ID) ? (*this)[text.ID] : invalid;
}

/*============================================================================
  * TASK:	  Check if a different text with the same ID as of 'text'is 
  *				already on the list
  * EXPECTS:  'text': text to check for, its ID is set (may be the base ID, 
  *				may be the real ID)
  * RETURNS:  collision count and 'usageCount'
  * GLOBALS:
  * REMARKS:  - does not change the data base
  *			  - If the text is not yet in data base usageCount is 0
  *			  - if it is in data base then 'usageCount' is set to the
  *				1 + usage count of it and the collision count is the same as for
  *				the existing text
  *			  - if there's at least one other text with the same ID as 'text' 
  *				(ID collision -'text.ID' may be base ID or a collided one), 
  *				ID of the new text is : BASE_ID + collision count * ID_INCREMENT
 *--------------------------------------------------------------------------*/
int TextMap::Collision(LanguageTexts &text, int &usageCount) const
{
	ID_t id = text.ID;
	usageCount = 0;
	int collision = (id & BASE_ID_MASK) >> 32;	// get original collision from id
	while (contains(id))	// with maybe different text
	{
		const LanguageTexts &textInMap = operator[](id);
		if (text == textInMap)	// then all language texts matched i.e.
		{						// this text is already in data base
			usageCount = textInMap.usageCount + 1;
			return collision;
		}
		++collision;
		id += ID_INCREMENT;		// try next ID
	}
	return collision;
}

/*============================================================================
* TASK:		Add an existing text to the text map
* EXPECTS: text - has non 0 base ID and no text.collision set
*		   added - (output parameter) signals if new text was added or the
*					ID of an existing one is returned
* GLOBALS:'_textMap'
* RETURN: ID of text added / found
* REMARKS: - if the same text is in _textMap already just increments its usage count
*			else adds it to '_textMap'
*		   - the text ID may be modified if a different text with this id 
*			is already on the list
*		   - two texts are only identical if all language texts are the same
*--------------------------------------------------------------------------*/
ID_t TextMap::Add(LanguageTexts & text, bool &added)
{
	int usageCount,
	collision = Collision(text, usageCount);
	added = usageCount == 0;		// should add?
	if (added)
	{
		text.ID = (text.ID & BASE_ID_MASK) + collision * ID_INCREMENT;
		text.collision = collision;
	}
	++text.usageCount;
	(*this)[text.ID] = text;
	return text.ID;
}

/*============================================================================
* TASK:
* EXPECTS: texts - as many texts as tehere are languages
*		   added - output parameter signals if new text was added or the
*					ID of an existing one is returned
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
ID_t TextMap::Add(QStringList &texts, bool &added)
{
	LanguageTexts txt(texts);
	return Add(txt, added);
}

/*============================================================================
* TASK:	decrements usage count and removes text from map if the usage count becomes 0
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void TextMap::Remove(ID_t id)
{
	LanguageTexts *pt = nullptr;
	if (!contains(id))
		return;

	pt = &(*this)[id];
	if (--pt->usageCount == 0)
			remove(id);
}

/**************************** Image *****************************/
int Image::operator<(const Image &i)
{
	switch (searchBy)
	{
		case byID  : return ID < i.ID;
		case byName: return name < i.name;
		default: return path + name < i.path + i.name;
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
bool Image::operator==(const Image &i)
{
	switch (searchBy)
	{
		case byID: return ID == i.ID;
		case byBaseID: return (ID & BASE_ID_MASK) == (i.ID & BASE_ID_MASK) ? (name.toLower() == i.name.toLower() ? true : false) : false;
		case byName: return name == i.name;
		default: return path + name == i.path + i.name;
	}
}

/*============================================================================
* TASK:  writes image info as 
*			<image name = ID.jpg> (size, WxH, OWxOH, date)<path. rel. to src>
* EXPECTS:
* GLOBALS:
* REMARKS: If image does not exist writes the image path into the file 
*			without any additional data
*--------------------------------------------------------------------------*/
QTextStream & Image::WriteInfo(QTextStream & _ofs) const
{
	QString fullName = FullName();
	int len = config.dsSrc.Length();
	if (fullName.mid(0, len) == config.dsSrc.ToString())
		fullName = fullName.mid(len);
	if (exists)
	{
		_ofs << LinkName() << " ("
			<< fileSize << ","
			<< rsize.width() << "x" << rsize.height() << ","
			<< osize.width() << "x" << osize.height() << ","
			// ISO 8601 extended format: either yyyy-MM-dd for dates or 
			<< uploadDate.toString(Qt::ISODate)
			<< ") => " << fullName << "\n";
	}
	else
		_ofs << name << "# not found\n";
	return _ofs;
}

/**************************** ImageMap *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Image &ImageMap::Find(ID_t id, bool useBase)
{
	ID_t mask = useBase ? BASE_ID_MASK : 0xFFFFFFFFFFFFFFFFul;
	Image im; im.ID = id;
	for (auto i = begin(); i != end(); ++i)
		if ( (i.key() & mask)  == (im.ID & mask) )
			return i.value();
	return invalid;
}

/*============================================================================
* TASK:		searches for image by its path name.
* EXPECTS: FullName - absolute or source relative path of image
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Image &ImageMap::Find(QString FullName)
{
	Image img;
	SeparateFileNamePath(FullName, img.path, img.name);

	Image::searchBy = Image::byFullName;
	for (auto i = begin(); i != end(); ++i)
		if (i.value() == img)
			return i.value();

	return invalid;
}

/*============================================================================
* TASK: Add an image to the list of all images
* EXPECTS:  path - full or config.dsSrc relative path name of the image
*			added- output parameter: was this a new image?
* GLOBALS:
* REMARKS: - calculates the CRC32 of the file contents
*		   - when collisions occur adds a unique value above 0xFFFFFFFF to the ID
*		   - even non-existing images are added to map
*		   - if an image with the same name, but with different path is found
*				and the one in the data base does not exist, replace the one
*				in the data base with the new image even when it also does not exist
*--------------------------------------------------------------------------*/
ID_t ImageMap::Add(QString path, bool &added)	// path name of source image
{
	Image img;

	SeparateFileNamePath(path, img.path, img.name);
	img.SetResizeType();	// handle '!!'

	if (img.path.isEmpty())
		img.path = AlbumGenerator::lastUsedItemPath;

	if (!QDir::isAbsolutePath(path))     // common part of path is not stored
		path = config.dsSrc.ToString() + img.path + img.name;	 // but we need it

	added = false;
	ID_t id = CalcCrc(img.name, false) | IMAGE_ID_FLAG;	// just by name. CRC can but id can never be 0 

	QFileInfo fi(path);			
	img.exists = fi.exists();	

	Image *found = &Find(id); // check if a file with this same base id is already in data base? 
							  
	if( found->Valid())		  // yes an image with the same base ID as this image was found
	{						  // the ID can still be different from the id of the found image though
		do					  // loop thorugh all images with the same base ID			
		{
			if (found->name == img.name)	// if name is the same too then same image	
			{								// even when they are in different directories!
				if (found->exists)
					if( !img.exists || (img.exists && (found->uploadDate >= fi.lastModified().date()  || found->fileSize >= fi.size())) )
						return found->ID;	  // do not change path as image already existed

				id = found->ID;		// else found image did not exist,
				break;				// replace it with this image but with the same ID as the old one 
			}
			else						// same ID, different name - new image
				id += ID_INCREMENT;
			// repeat until a matching name with the new ID is found or no more images
		} while ((found = &Find(id, false))->Valid()); // full by ID
	}
	// now I have a new image to add
	// new image: 
	if (img.exists)
	{
		img.fileSize = fi.size();
		img.uploadDate = fi.lastModified().date();
	}

	added = true;
	img.ID = id;
	insert(id, img);
	return id;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Image &ImageMap::Item(int index)
{
	if (index < 0 || index > size())
		return invalid;
	iterator it = begin();
	it += index;
	return *it;
}

/**************************** AlbumMap *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::operator<(const Album &i)
{
	switch (searchBy)
	{
		case byID: return ID < i.ID;
		case byName: return path+name < i.path+i.name;
	}
	return -1;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
bool Album::operator==(const Album &i)
{
	switch (searchBy)
	{
		case byID: return ID == i.ID;
		case byName: return path + name == i.path + i.name;
	}
	return 0;
}

/*============================================================================
* TASK: get number of non-excluded images in album and remove excluded ones
*		from local image list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::ImageCount()
{
	if (_imageCount < 0)
	{
		_imageCount = 0;
		for (auto a : items)
			if (a > 0 && (a & IMAGE_ID_FLAG))
				++_imageCount;
	}
	return _imageCount;
}

/*============================================================================
* TASK: get number of non-excluded videos in album and remove excluded ones
*		from local video list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::VideoCount()
{
	if (_videoCount < 0)
	{
		_videoCount = 0;
		for (auto a : items)
			if (a > 0 && (a & VIDEO_ID_FLAG))
				++_videoCount;
	}
	return _videoCount;
}

/*============================================================================
* TASK: get number of non-excluded sub-albums in album and remove excluded ones
*		from local sub-album list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::SubAlbumCount()
{
	if (_albumCount < 0)
	{

		_albumCount = 0;
		for (auto a : items)
			if (a > 0 && (a & ALBUM_ID_FLAG))
				++_albumCount;
	}
	return _albumCount;
}


/*========================================================
 * TASK:	Calculates how many title text this album has
 * PARAMS:
 * GLOBALS:
 * RETURNS: count ( >= 0)
 * REMARKS: - only calculates count when the titleCount < 0
 *			- not recursive
 *-------------------------------------------------------*/
int Album::TitleCount()
{
	if (_titleCount < 0)
	{
		_titleCount = 0;
		for (auto a : items)
			if ((a > 0) &&
				(
					((a & ALBUM_ID_FLAG) && albumgen.Albums()[a].titleID) ||
					((a & VIDEO_ID_FLAG) && albumgen.Videos()[a].titleID) ||
					((a & IMAGE_ID_FLAG) && albumgen.Images()[a].titleID)
				)
			)
				++_titleCount;
	}
	return _titleCount;
}

/*========================================================
*TASK:	Calculates how many description text this album has
 * PARAMS :
 * GLOBALS :
 * RETURNS : count(>= 0)
 * REMARKS :- only calculates count when the descCount < 0
 *			- not recursive
 * ------------------------------------------------------- */
int Album::DescCount()
{
	if (_descCount < 0)
	{
		_descCount = 0;
		for (auto a : items)
			if (a > 0 &&
				(
					((a & ALBUM_ID_FLAG) && albumgen.Albums()[a].descID) ||
					((a & VIDEO_ID_FLAG) && albumgen.Videos()[a].descID) ||
					((a & IMAGE_ID_FLAG) && albumgen.Images()[a].descID)
				)
			)
				++_descCount;
	}
	return _descCount;
}


/*========================================================
 * TASK:	find the ID of the item of a given type starting
 *			from 'startPos (default: 0)
 * PARAMS:	type: ALBUM_ID_FLAG, etc
 *			index:  ordinal of element of given type
 *			startPos: start searching from here
 * GLOBALS:
 * RETURNS:
 * REMARKS: - does not change or store startPos
 *			- 'index' must be less than 'items.size()'
 *-------------------------------------------------------*/
ID_t Album::IdOfItemOfType(int64_t type, int index, int startPos)
{
	if (index >= items.size())
		return -1;		// MUST NOT 

	for (; startPos < items.size(); ++startPos)
		if (items[startPos] & type)
			if(!index--)
				return items[startPos];
	return -1;	// invalid
}

/*============================================================================
* TASK: Create full link name of album
* EXPECTS: language -  index of language
*			http - true: http or https prefix plus add albumdir to the 
*			front true: needed, false: just name
* GLOBALS: config
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::LinkName(int language, bool http) const
{
	QString s;
	if (http)
	{
		if (config.sServerAddress.ToString().toLower().left(4) == "http")
			s = config.sServerAddress + "/";
		else
			s = (config.bForceSSL ? "https://" : "http://") + config.sServerAddress + "/";
        s += NameFromID(ID, language, http);
	}
	return s;
}

/*============================================================================
* TASK: returns base name of album w.o. language or  extension
* EXPECTS: none
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::BareName()
{
	return QString("%1%2").arg(config.sBaseName.ToString()).arg(ID & ID_MASK);
}

/*============================================================================
* TASK: Create album name for given ID
* EXPECTS: id - album id
*			  - language -  index of language
* GLOBALS: config
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::NameFromID(ID_t id, int language, bool withAlbumPath)
{
	QString s;

	id &= ID_MASK;

	if (id == 1)	// root album
	{
		s = config.sMainPage.ToString();
		s = Languages::FileNameForLanguage(s, language);
	}
	else
	{
		if (id == 2)	// recent uploads
			s = "latest" + Languages::abbrev[language] + ".html";
		else
			s = Languages::FileNameForLanguage(QString("%1%2.html").arg(config.sBaseName.ToString()).arg(id), language);

		if (withAlbumPath)
			s = config.dsAlbumDir.ToString() + s;
	}
	return s;
}

/*============================================================================
* TASK:	Create album name
* EXPECTS: language -  index of language
* GLOBALS: config
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::NameFromID(int language)
{
	return NameFromID(ID, language, false);
}

/**************************** AlbumMap *****************************/

/*============================================================================
* TASK:	  Add an album to the album map if it is not already in there
* EXPECTS: path:  IN - either absolute path or relative to source directory (!)
*          added: OUT - et when this is added
* RETURNS: id of album ORed with ALBUM_ID_FLAG
* GLOBALS:
* REMARKS:	path may contains a logical name only it need not exist
*			if it exists then lastUsedItemPath is adjusted
*--------------------------------------------------------------------------*/
ID_t AlbumMap::Add(QString path, bool &added)
{
	Album ab;

	Album &found = Find(path);		// already in database?
	added = false;
	if (found.Valid())
		return found.ID;	// same base ID, same name then same album

	SeparateFileNamePath(path, ab.path, ab.name);
	ab.ID = GetUniqueID(*this, path, false) | ALBUM_ID_FLAG;   // using full path name only
	if(!QDir::isAbsolutePath(path) )
		ab.exists = QFile::exists((config.dsSrc + path).ToString());
	else
		ab.exists = QFile::exists(path);
	if (!ab.exists)	// but maybe its parent does
	{				// in that case get path from its parent
		if (!QDir::isAbsolutePath(ab.path))
			ab.exists = QFile::exists((config.dsSrc + ab.path).ToString());
		else
			ab.exists = QFile::exists(ab.path);
	}
	if (ab.exists)
		AlbumGenerator::lastUsedItemPath = ab.path + ab.name + "/";
	ab.exists = true; // no need for real directoy to exist --  QFileInfo::exists(path);
	added = true;		// always add, even when it does not exist
	(*this)[ab.ID] = ab;
	return ab.ID;
}

/*============================================================================
  * TASK:	 return the index-th element of the map
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
Album & AlbumMap::Item(int index)
{
	if (index < 0 || index > size())
		return invalid;
	iterator it = begin();
	it += index;
	return *it;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Album & AlbumMap::Find(QString albumPath)
{
	Album ab;
	SeparateFileNamePath(albumPath, ab.path, ab.name);
	
	ab.searchBy = Album::byName;
	for (auto i = begin(); i != end(); ++i)
		if (i.value()== ab)
			return i.value();
	return invalid;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Album& AlbumMap::Find(ID_t id)
{
	Album ab;
	ab.searchBy = Album::byID;
	ab.ID = id;

	return contains(id) ? (*this)[id] : invalid;
}

/************************** Albumgenerator *************************/
/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString AlbumGenerator::SiteLink(int language)
{
	QString s;
	if (config.sServerAddress.ToString().toLower().left(4) == "http")
		s = config.sServerAddress + "/";
	else
		s = (config.bForceSSL ? "https://" : "http://") + config.sServerAddress + "/";
	return s;
}

/*============================================================================
  * TASK:	 add a new image from structure to image map
  * EXPECTS:  imagePath: path name relative to source directory
  *			  
  * RETURNS:  id of image/video added/found with type flag set
  *				It will not have any title or description as those
  *				will be added after this image is processed
  * GLOBALS:
  * REMARKS:- new images may be added to the structure file by using the path
  *			  name, (absolute or relative to the gallery source) and nothing more. 
  *			  In this case the new image must be processed and added to the 
  *			  data base.
  *			 - sets '_structChanged' to true
 *--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrVideoFromPathInStruct(QString imagePath, FileTypeImageVideo ftyp)
{
	bool added;
	++_structChanged;

	if (ftyp == ftImage)
		return _imageMap.Add(imagePath, added);	// add new image to global image list or get id of existing
	else
		return _videoMap.Add(imagePath, added);
}

/*==========================================================================
* TASK:		add a sub-album or an image to global arrays and to album 'ab'
* EXPECTS:	ab - album to add data to (parent)
*			path - full path name relative to  album root '_root' or absolute path
*			pos: - put position in images or in albums here
*			folderIcon: if the file should be used as album thumbnail for this album
* RETURNS:	ID of new or already present album or image
* REMARKS:	does not set/modify the title and description IDs
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrAlbum(Album &ab, QString path, bool folderIcon)
{
	if (!QDir::isAbsolutePath(path))
		path = _root.path + path;
	QFileInfo fi(path);
	ID_t id = _AddImageOrAlbum(ab, fi);
	if ( id > 0  && folderIcon)	// an album can be a folder icon which means that its
		ab.thumbnail = id;		// folder icon is used here. This will be resolved later on

	return id;
}

/*==========================================================================
* TASK:		add a sub-album or an image to global arrays and album 'ab'
*			from directory unless they are excluded
* EXPECTS:	ab - album to add images and sub albums to
*			fi - info of a file and album
* RETURNS:	id of data , for albums with ALBUM_ID_FLAG, 
*				with videos with VIDEO_ID_FLAG set
* REMARKS:  - 'exists' member of not excluded old oe new images and sub-albums 
*				are set to true when 'fromDisk' is true
*			- adds new images/videos and id's to the corresponding id list of 'ab'
*			- shows progress using frmMain's progress bar
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrAlbum(Album &ab, QFileInfo & fi/*, bool fromDisk*/)
{
	ID_t id = 0;
	
	QString s = fi.filePath();
	if (fi.fileName()[0] == '.')			// hidden file?
		return 0;

	bool added = false;
	FileTypeImageVideo type;
	if (fi.isDir())
	{
		QString ds = fi.fileName();
		if (ds[0] == '.' || ds == "res" || ds == "cache" || ds == "thumbs")
			return 0;
		id = _albumMap.Add(s, added);	// add new album to global album list
	}
	else if ((type=IsImageOrVideoFile(s))!= ftUnknown)
	{
		if (type == ftImage)
			id = _imageMap.Add(s, added);	// add new image to global image list or get id of existing
		else
			id = _videoMap.Add(s, added);	// add new video to global video list or get id of existing

	}
	if ((id & ID_MASK) && added)
		ab.items.push_back(id);	// add to ordered item list for this album
	// progress bar
	emit SignalProgressPos(_albumMap.size(), _ItemSize() );	  // both changes

	_remDsp.Update(_albumMap.size());
	emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _ItemSize(), false);
	return id;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_IsExcluded(const Album &ab, QString name)
{
	ID_t id;
	for (auto i : ab.items)
	{
		if (((i & ALBUM_ID_FLAG) && _albumMap[id].name == name) ||
			((i & IMAGE_ID_FLAG) && _imageMap[id].name == name) ||
			((i & VIDEO_ID_FLAG) && _videoMap[id].name == name))
			return i > 0;
	}

	return false;
}

/*============================================================================
* TASK:		 if an album does not have a title in a '.info' file then
*				set the album directory name as the default title.
* EXPECTS:
* GLOBALS:
* REMARKS:	use only for albums
*--------------------------------------------------------------------------*/
void AlbumGenerator::_TitleFromPath(QString path, LangConstList & ltl)
{
	QString p, n;
	SeparateFileNamePath(path, p, n);
	ltl.Fill(n);
}

/*==========================================================================
 * TASK:	read 'albumfiles.txt' (custom file filtering and ordering)
 *			if exists from the directory 'path' into
 * EXPECTS:	ab: existing Album
 * RETURNS:	true: data read into 'images' and 'albums' without calculating CRC
 *			false no such file
 * REMARKS: - 'albumfiles.txt' 
 *					max 3 fields in every line separated by\TAB(s): 
 *				<image name>	[<link image path>]	[<time in seconds>]
 *					if no link image path then 2 TABs after each other
 *        	- images/albums not mentioned in file are added to the end of
 *				the ordered list of images
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadFromDirsFile(Album &ab)
{
	QString path = ab.FullName();  
	if(path[path.length()-1] != '/') 
		path += '/';

	FileReader fr(path + "albumfiles.txt", frfTrim);
	if (!fr.Ok())
		return false;
	QString line;
	QStringList sl;
	bool excluded = false;
	

	while (!(line = fr.ReadLine()).isEmpty())
	{
		if( (excluded = (line[0] == '-')) == true )	// '-' : do not use this image/album in this album
			line = line.mid(1);						// but still process it to put its ID to the excluded list

		sl = line.split('\t');
		if (sl.size() == 1 || sl[1].isEmpty())		// if name is not a link to an external file
			line = path + sl[0];	// then this is its full name
		else
			line = QDir().cleanPath(sl[1]);			// else this is the real full name

		ID_t id = _AddImageOrAlbum(ab, line);   // to lists (_albumMap or _imageMap) and (albums or images) and order
		if (excluded)
			id |= EXCLUDED_FLAG;

		ab.items.push_back(id);
	}
	return true;
}

/*============================================================================
  * TASK: test the sizesof an existing thumbnail against config and see 
  *		  if it must be recreated
  * EXPECTS: thumbPath - full path of thumbnail file
  *	     img - image data from data base with correct thumbnail size set in it
  * RETURNS: 'true' : actual size of thumbnail is not the same as the one in data base
  *	     'false' : otherwise
  * GLOBALS:
  * REMARKS: - the thumbnail is already a rotated image when needed
 *--------------------------------------------------------------------------*/
bool AlbumGenerator::MustRecreateThumbBasedOnImageDimensions(QString thumbPath, Image & img)
{
	ImageReader reader(thumbPath);
	QSize 	// tsize = config.ThumbSize(),
			thumbSize = reader.size();	// read thumbnail image dimensions from file
							// because integer arithmetic the calculated w & H may be different
							// from that of the converted thumbnail file
	return abs(thumbSize.width() - img.tsize.width()) > 2 || abs(thumbSize.height() - img.tsize.height()) > 2;
}

/*==========================================================================
* TASK:	separate lines containing text delimited by '@@' ot "{)" delimiters
*		into a string list
*		fills in data
* EXPECTS: line: line w. delimiters
* RETURNS: separated lists of text for all languages
* REMARKS:	- a line may not contain any delimiter (same for all languages)
*			- if we have N languages the line may contain more than N+1
*			  delimiters, as part(s) of the line may be common to all languages
*			  so split() may not be used 
*--------------------------------------------------------------------------*/
QStringList AlbumGenerator::_SeparateLanguageTexts(QString line)
{
	QStringList list;
	int cntLang = Languages::Count();
	for (int i = 0; i < cntLang; ++i)
		list << "";
	int lastPos = (line.mid(0, 2) == "@@" || line.mid(0, 2) == "{)") ? 2 : 0, 
		pos, 
		li = lastPos ? 1 : 0;		// language index
	for (pos = lastPos; pos < line.length(); ++pos)
	{			// for @@ delimiters plus EOL
		if (line.mid(pos, 2) == "@@" || line.mid(pos, 2) == "{)" || pos == line.length()-1) 
		{
			if (pos == line.length() - 1) 	   // otherwise last character is lost
				++pos;
			if (pos - lastPos)	// text between last and actual positions
			{
				QString s = line.mid(lastPos, pos - lastPos);
				if (li)			// then one of the languages
					list[li - 1] += s;
				else			// common text: add to all languages 
				{					
					for (int i = 0; i < cntLang; ++i)
						list[i] += s;
				}
				lastPos = pos + 2;		// skip @@
			}
			else
			{
				lastPos += 2;	// first character after the delimiter
			}
			if (pos == line.length() - 1)	// last character (can't be a delimiter)
				++pos;						// increase length to copy (do not loose last character)
			else
				if (++li == cntLang+1) 
					li = 0;	// normal text again

			++pos;	// to second character in delimiter
		}
	}
	return list;
}

/*============================================================================
  * TASK:	sets a relativ path to the image if it has no path
  *			or stores the path as the last used path if it had one
  * EXPECTS: imagerPath - absolute or relative image path or just file name
  * RETURNS: imagerPath
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString& AlbumGenerator::_GetSetImagePath(QString & imagePath)
{
	if (imagePath.indexOf('/') < 0)		// no path at all
		imagePath = lastUsedItemPath + imagePath;
	else
		lastUsedItemPath = imagePath.left(imagePath.lastIndexOf('/') + 1); // including last '/'
	return imagePath;
}

/*==========================================================================
* TASK:	read dexcriptions for images and albums in the given album and
*		fills in data
* EXPECTS: ab - existing album
* RETURNS: success or error
* REMARKS: - if an image or sub-album was not set earlier it is created here
*		   - structure of file:
*				<name of image or directory>=<description in many languages>
*		   - languages ar separated by double '@' characters
*		   - only as many languages are used as are set in global 'Languages'
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadJCommentFile(Album &ab)
{
	QString path = ab.FullName();
	if (path[path.length() - 1] != '/')
		path += '/';

	FileReader fr(path + "comments.properties", frfTrim);
	if (!fr.Ok())
		return false;
	QString	line,		// <ianame>=<texts>
			ianame;
	QStringList sl;
	ID_t id;
	FileTypeImageVideo type;
	while (!(line = fr.ReadLine()).isEmpty())
	{
		int posText = line.indexOf('=');
		if (posText < 0)
			continue;
		ianame = line.left(posText).trimmed(); // image/directory name
		line = line.mid(posText + 1).trimmed();
		if(line.isEmpty())
			continue;

		id = _AddImageOrAlbum(ab, path + ianame, false);
		type = id & VIDEO_ID_FLAG ? ftVideo : ftImage;
		if (id)		// then image existed
		{			// set text to image or album description
			sl = _SeparateLanguageTexts(line);
			bool added;
			ID_t tid = _textMap.Add(sl, added);	// same id for all languages
			if (id & ALBUM_ID_FLAG)		// description to sub-album
				_albumMap[id].descID = tid;
			else if(id & VIDEO_ID_FLAG)
				_videoMap[id].descID = tid;
			else
				_imageMap[id].descID = tid;
		}
	}
	return true;
}

/*==========================================================================
* TASK:		read 'meta.properties' from album directory
*			and sets album thumbnail (image or folder) ID and descriptions
* EXPECTS:	ab - album to add parameters to
* RETURNS:	true: file existed, false: file did not exist or read error
* REMARKS: - structure of lines in 'meta.properties'
*				'ordering='custom'
*				'folderIcon'=<image file name for 'ab'>
*				'descript'=<description for 'ab'>
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadJMetaFile(Album &ab)
{
	QString path = ab.FullName();
	if (path[path.length() - 1] != '/')
		path += '/';

	FileReader fr(path + "meta.properties", frfTrim);
	if (!fr.Ok())
		return false;

	QString	line,		// <ianame>=<texts>
		ianame;
	QStringList sl;
	int pos;
	bool added;
	while (!(line = fr.ReadLine()).isEmpty())
	{
		pos = line.indexOf('=');
		if (pos < 0)
			continue;
		ianame = line.left(pos).trimmed(); // image/directory name
		line = line.mid(pos + 1).trimmed();
		if (line.isEmpty())
			continue;

		if (ianame[0] == 'o')			// ordering 
			continue;					// don't care
		else if (ianame[0] == 'f')		// folder icon
		{
			(void) _AddImageOrAlbum(ab, path + line, true);	// add image or folder
			continue;														// as folder thumbnail too
		}
		else if (ianame[0] != 'd')			// safety
			continue;			
					// else 'description line for 'ab'

		sl = _SeparateLanguageTexts(line);
		// set text to image or album description
		int slSize = sl.size();
		if (slSize)
		{
			ID_t tid = _textMap.Add(sl, added);	  // more languages than texts
			ab.descID = tid;			  // description to sub-album
		}
	}
	return true;
}


/*=================================================================
* TASK: put album and image title texts from a '*.info' files in
*		the '.jalbum' subdirectory of the album's directory into
*		ab and the images inside it
* EXPECTS:	ab - album (already on global album list '_albumMap')
* RETURNS: true...
* REMARKS: - '.info' files contain album titles,
*			 '<image name>.info' files image titles
*			- format:
*				line #x			<string>title</string>
*				line #(x+1)		<string>title text</string>
*		    - use after reading all image files and comments
*			- if the same iamge has different titles in different albums
*				the latest ones will be used
*----------------------------------------------------------------*/
void AlbumGenerator::_JReadInfoFile(Album &ab, QString & path, QString name)
{
	QString line;
	FileReader reader(path + ".jalbum/" + name, frfNeedUtf8 | frfTrim);  // name of .info file

	LangConstList ltl(Languages::Count());			// set default title here

	if (!reader.Ok())		// file read problem
		return;

	bool titleLineNext = false;
	while (!(line = reader.ReadLine()).isEmpty())
	{
		if (titleLineNext)
		{
			int pos = line.lastIndexOf('<');
			line = line.mid(8, pos - 8);	// title in all languages
			break;
		}
		titleLineNext = line == "<string>title</string>";
	}

	bool added;
	if (line.isEmpty()) // no title
	{
		if (name == ".info")			// just for the album
		{
			_TitleFromPath(path, ltl);
			ab.titleID = _textMap.Add(ltl, added);
		}
	}
	else
	{
		QStringList sl;
		sl = _SeparateLanguageTexts(line);
		ID_t tid = _textMap.Add(sl, added);	// same id for all languages

		if (name[0] == '.') //  ".info"	fo actual album
			ab.titleID = tid;
		else			   // image
		{
			name = name.left(name.length() - 5);
			Image &img = _imageMap.Find(path + name);
			if (img.Valid())
				img.titleID = tid;
		}
	}
}

/*=================================================================
* TASK: put album and image title texts from '*.info' files in 
*		the '.jalbum' subdirectory of the album's directory into
*		ab and the images inside it
* EXPECTS:	ab - album (already on global album list '_albumMap')
* RETURNS: true...
* REMARKS:  - use after reading all image files and comments
*			- if the same iamge has different titles in different albums
*				the latest ones will be used
*----------------------------------------------------------------*/
bool AlbumGenerator::_JReadInfo(Album & ab)
{
	QString path = ab.FullName();
	if (path[path.length() - 1] != '/')
		path += '/';

	QDir infoDir(path + ".jalbum");	// place for the info files
	if (infoDir.exists())
	{
		QStringList filters;
		filters << "*.info";

		QStringList list = infoDir.entryList(filters, QDir::Files, QDir::NoSort);	// get all files and sub directories

		for (QString &s : list)
		{
			if(!_IsExcluded(ab, s))
				_JReadInfoFile(ab, path, s);
		}
	}
	return true;
}

/*=================================================================
 * TASK: read a directory tree of images and order them into an album
 *		  hierarchy
 * Reads whole gallery hierarchy starting at 'ab'
 *			If it is a JAlbum directory with corresponding files
 *			adds only images and albums set in those files
 *			otherwise adds each image and sub-folders to it
 * EXPECTS:		ab - album (must already be on the global album list '_albumMap')
 * GLOBALS: _justChanges
 * REMARKS: - first reads meta.properties and set parameters for this album
 *			- then reads file 'albumfiles.txt' if it exists and adds
 *				image/video/album names from it to album
 *			- then reads all file and sub-directory names from the album directory
 *			  and add not stored ones to lists recursively processing sub albums
 *			- then reads comments.properties and sets them for images and sub-albums
 *			- then call itself recursively for sub-folders
 *----------------------------------------------------------------*/
void AlbumGenerator::_JReadOneLevel(Album &ab)
{
	// TODO justChanges
	_isAJAlbum = false;		// set only if jalbum's file are in this directory
	_isAJAlbum |= _ReadFromDirsFile(ab);		// ordering set in file ( including exlusions and real names)
	_isAJAlbum |= _ReadJMetaFile(ab);		// thumbnail (image or sub-album) ID and description for the actual album

	// Append images and albums from disk 

	// at path 'ab.FullName()' and add
	// those not yet listed in 'albumfile.txt' to the list of files and albums
	// in the order they appear

	QDir dir(ab.FullName());
	dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::NoSort);
	QFileInfoList list = dir.entryInfoList();	// get all files and sub directories

	ID_t id;
			
	for (auto fi : list)		// elements in directory
	{
		if( (id = _AddImageOrAlbum(ab, fi)) == 0)	// nothing added? (hidden file)
			continue;
		if (id & ALBUM_ID_FLAG)		// sub album
		{
			_albumMap[id].parent = ab.ID;	// signals this sub-album is in 'ab' 
											// Must change when sub-albums moved to other album
			if (_albumMap[id].exists)
				_JReadOneLevel(_albumMap[id]);
		}
		emit SignalProgressPos(_albumMap.size(), _imageMap.size());
		if (!_processing)
			return;
	}
	_isAJAlbum |= _ReadJCommentFile(ab);	// descriptions for files and folders in ab
	_isAJAlbum |= _JReadInfo(ab);			// titles for album and images inside
	(void)ab.ImageCount();	// removes excluded ID s of 'ab.images'
	(void)ab.SubAlbumCount();	// removes excluded ID s of 'ab.albums'
//	ab.excluded.clear();	// do not need it any more
}

/*==========================================================================
* TASK:		reads whole album hierarchy starting at directory 'root'
* EXPECTS: 'root' path name of uppermost source album directory
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::Read()
{
	_processing = true;

//	_structAlreadyInMemory = false;		// reset so if an error catched we will not think the struct is OK

	bool result = false;

	bool _justChanges = (AlbumCount() != 0 && ImageCount() != 0 && !config.bGenerateAll);		// else full creation

	if (!_justChanges)
	{
		_imageMap.clear();
		_textMap.clear();
		_videoMap.clear();
		_albumMap.clear();
	// add default image when no image is found This is the only one with id == 0
	// the file 'NoImage.jpg' must be put into the 'res' directory
		Image im;
		im.exists = true;
		im.name = "NoImage.jpg";
		im.ID = 0 | IMAGE_ID_FLAG;
		im.dsize = im.osize = QSize(800, 800);
		_imageMap[0] = im;
	}

	PROGRAM_CONFIG::MakeValidLastConfig();		// may add new config
	QString s = PROGRAM_CONFIG::NameForConfig(false, ".struct");
	_structChanged = 0;

	if(!config.bReadJAlbum && QFileInfo::exists(s))
	{
		try
		{
			result = _ReadStruct(s);
		}
		catch (.../*BadStruct b*/)
		{
			_imageMap.clear();		  // reading aborted
			_textMap.clear();
			_albumMap.clear();
		}
	}
	else
		result = _ReadFromDirs();
	emit SignalAlbumStructChanged();	// show list of albums in GUI
//	if (result)
//		_structAlreadyInMemory = true;
	return result;
}
//*****      END OF READ PART, START OF WRITE PART  *****

//---------------------- helpers --------------------------

/*============================================================================
* TASK:	   creates necessary directories
* EXPECTS:
* GLOBALS:	config
* REMARKS: - if directories does not have an absolute path
*				(windows: starting with "drive:\" or "\", linux: starting with '/')
*			 they are created in this hierarchy:
*			  config.dsGallery		destination directory corresponding to server address
*				config.dsGRoot		gallery root folder on server (if empty then
*									it is set to '/')
*					config.dsAlbumDir	albums (HTML files) here
*					config.dsImageDir	images for all albums
*					config.dsCssDir		css files
*					config.dsFontDir		fonts
*					js					javascript files
*					res					icon files, stb
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_CreateDirectories()
{
	__DestDir = config.dsGallery;		// destination directory the new gallery is
										// put inside it
	QString sActDir = __DestDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;

	__SetBaseDirs();

	sActDir = __RootDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __AlbumDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __CssDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __ImageDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __ThumbDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __VideoDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __FontDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __ResourceDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	sActDir = __JsDir.ToString();
	if (!CreateDir(sActDir))	// cancelled
		return false;
	return true;
}

/*==========================================================================
* TASK:		reads language texts from structure
* EXPECTS: reader - opened, flags set: needUtf8
* RETURNS: success code
* REMARKS: throws on error
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_LanguageFromStruct(FileReader & reader)
{
	QString line = reader.ReadLine(); // [Language count: X: 
// 'n' is temporary
	int n = line[0] == '[' ? 1 : 0;
	if (line.mid(n, 15) != "Language count:")
//	if (line.left(15) != "Language count:")
		throw BadStruct(reader.ReadCount(), "Language Count");

	QString sFalcong = ToUTF8((QString)sFalcongEnglishCopyright), s;

	int langcnt = line.mid(n+15).toInt();
	QStringList sl;
	Languages::Clear(langcnt);		// allocates this many empty strings in each string list
	reader.ReadLine();				// first language index :0 
	for (int i = 0; i < langcnt; ++i)
	{
		while((sl = reader.ReadLine().split('=')).size() == 2 ) // finished when next language index is read in
		{
			s = sl[0].toLower();
			if (s == "name")
				Languages::names[i] = sl[1];
			else if (s == "abbrev")
				Languages::abbrev[i] = sl[1];
			else if (s == "language")
				Languages::language[i] = sl[1];
			else if (s == "icon")
				Languages::icons[i] = sl[1];
			else if (s == "images")
				Languages::images[i] = sl[1];
			else if (s == "videos")
				Languages::videos[i] = sl[1];
			else if (s == "albums")
				Languages::albums[i] = sl[1];
			else if (s == "toalbums")
				Languages::toAlbums[i] = sl[1];
			else if (s == "totop")
				Languages::toTop[i] = sl[1];
			else if (s == "uponelevel")
				Languages::upOneLevel[i] = sl[1];
			else if (s == "homepage")
				Languages::toHomePage[i] = sl[1];
			else if (s == "about")
				Languages::toAboutPage[i] = sl[1];
			else if (s == "contact")
				Languages::toContact[i] = sl[1];
			else if (s == "descriptions")
				Languages::showDescriptions[i] = sl[1];
			else if (s == "cdcaptions")
				Languages::coupleCaptions[i] = sl[1];
			else if (s == "share")
				Languages::share[i] = sl[1];
			else if (s == "latesttitle")
				Languages::latestTitle[i] = sl[1];
			else if (s == "latestdesc")
				Languages::latestDesc[i] = sl[1];
			else if (s == "countrycode")
				Languages::countryCode[i] = sl[1];
			else if (s == "countofimages")
				Languages::countOfImages[i] = sl[1];
			else if (s == "falcong")
			{
				if (sl[1].indexOf("©") >= 0 && sl[1].indexOf(ToUTF8(QString("Sólyom"))) < 0)
					sl[1] = sFalcongEnglishCopyright;
				Languages::falconG[i] = sl[1];
			}
		}
		if (Languages::abbrev[i].isEmpty())	// default
			Languages::abbrev[i] = Languages::countryCode[i];
	}
	if (Languages::countryCode[0].isEmpty())
	{
		for (int i = 0; i < langcnt; ++i)
			Languages::countryCode[i] = "en_US";
	}
	if (Languages::falconG[0].isEmpty())
	{
		for (int i = 0; i < langcnt; ++i)
			Languages::falconG[i] = "Site generated by <strong>falconG</strong> - © A. Sólyom 2018";
	}
//	reader.ReadLine();		// drop next language index or closing ']'
	if(reader.l()[0] != ']')
		throw BadStruct(reader.ReadCount(), "missing']'");

	return true;
}

#if 0
/*==========================================================================
* TASK:		determine hierarchy (indent) level
*--------------------------------------------------------------------------*/
static int __LevelFromString(QString line)
{
	int level = 0;
	for (; level < line.length() && line[level] != ' '; ++level)
		;
	return level;
}
/*==========================================================================
* TASK:		determine hierarchy (indent) level
*--------------------------------------------------------------------------*/
static bool __SameLevel(int level, QString line)
{
	return level == __LevelFromString(line);
}
#endif

/*==========================================================================
* TASK:		reads text, description and thumbnail IDs from structure file
* PARAMS:   reader - open file reader with the last line read in it
*					which after 'level' number of spaces starts with a '[' 
*					and ends with a ']'
*					
*	
*			ids   - output parameter, filled with  title, description and thumbnail ID
*			level - number of spaces before the '['
* RETURNS: nothing
* REMARKS:  - when called the last line read contains a title, description or thumbnail
*			  inside square brackets []
*		    - the kind of the line is determined by the first string of characters after
*			  the opening '[' brace: TITLE_TAG, DESCRCRIPTION_TAG or THUMBNAIL_TAG
*           - except for the THUMBNAIL_TAG there must be as many such lines as are 
*			  languages, each starting with the text of the tag followed by a dash ('-') 
*			  and the language abbreviation. (e.g for TITLE_TAG  [Title-en_US:<text>])
*			- the first line for either tag may end with a '*' followed by the text ID. 
*			  If it is not present the text ID will be calculated.
*			- The THUMBNAIL_TAG may contain the ID of the thumbnail image or the path name
*			  of the thumbnail image. In the latter case the path name will be replaced
*			  with the image ID
*			- the order of the tags are arbitrary, but all language texts for the same tag 
*			  must immediately follow each other: mixing tag lines are not allowed
*           - reads lines until the first line without a starting '[' is read
*				then returns
*			- before returning it puts the last language texts into _textMap
*			- version 1.0 - texts in [] may be followed by a '*' and a collision number
*			- version 1.1 - first language texts may be followed by a '*' and text ID
*							if there is no '*' in line (e.g. manually entered lines)
*							then calculates text ID like for version 1.0
*--------------------------------------------------------------------------*/
void AlbumGenerator::_GetTextAndThumbnailIDsFromStruct(FileReader &reader, IdsFromStruct & ids, int level)
{
	LanguageTexts texts(Languages::Count());

	// order of tags are arbitrary
	// any and all of these may be missing
	// ALL TAG TEXT MUST BE OF THE SAME LENGTH!
	// and all tags must start with a different letter

	ID_t textID = 0;		// ID of last processed text
	IdsFromStruct::IDS newTag = IdsFromStruct::nothing;

	ids.what = IdsFromStruct::nothing;

	do		 // for each line with text in []);
	{
		int lang = -1;		// THUMBNAIL_TAG
		if (reader.l()[level + 1] != THUMBNAIL_TAG[0])
		{

			for (lang = 0; lang < Languages::Count(); ++lang)
			{
				int len = Languages::language[lang].length();
				if (reader.l().mid(level + TITLE_TAG.length() + 1, len) == Languages::language[lang])
					break;
			}
			if (lang == Languages::Count())		// safety
				lang = 0;
		}

		if (reader.l()[level + 1] == THUMBNAIL_TAG[0])
			newTag = IdsFromStruct::thumbnail;
		else if (reader.l()[level + 1] == TITLE_TAG[0])
			newTag = IdsFromStruct::title;
		else
			newTag = IdsFromStruct::description;

						// when tag changes store texts collected for previous tag
						// except for thumbnails
		if (newTag != ids.what && ids.what != IdsFromStruct::thumbnail && !texts.IsEmpty())	// there are text(s) or ID read in 
		{
			if (textID && _textMap.contains(textID))	  // need this for texts with ID read from struct file
				texts.usageCount = ++_textMap[textID].usageCount;
			else if (!textID)						// when no id in file
			{
				textID = texts.CalcID(_textMap);	// sets new unique id and usage count as well
				++_structChanged;
			}

			if (!texts.usageCount)					// then not yet added
				texts.usageCount = 1;

			_textMap[textID] = texts;				// add new record or modify old record with the new usage count

			if (ids.what == IdsFromStruct::title)
				ids.titleID = textID;
			else
				ids.descID = textID;

			texts.Clear();
			textID = 0;								// processed
		}
				
		if(newTag == IdsFromStruct::thumbnail)
		{
			int len = (reader.l().length() - 1) - level - THUMBNAIL_TAG.length() - 2; // cut ending ']'
			if (!len)		   // empty thumbnail text?
				continue;
			// see if this is a processed line or an image path string
			// image path strings starting with a character other than a decimal digit may follow after a space or a colon
			// but image names in the source directory that start with a decimal digit must be written after a colon!
			bool b = reader.l()[level + THUMBNAIL_TAG.length() + 1] == ':';
			QString s = reader.l().mid(level + THUMBNAIL_TAG.length() + 2, len);  // ID or path name
			if (b && s[0].isDigit())
				ids.thumbnailID = s.toULongLong() | IMAGE_ID_FLAG;
			else
			{
				ids.thumbnailID = _AddImageOrVideoFromPathInStruct(s, ftImage);
				++_structChanged;
			}

			ids.what = IdsFromStruct::thumbnail;
		}
		else		// common part for title and description
		{			// because textID is always calculated using all texts for all languages
					// we can only calculate it after all lines are read in
			QString s = reader.l();
			int len = s.length();
			while (len && s[--len] != ']')	// find last ']'
				;
			++len;		// include closing ']' but not the possible ID/collision
			int clen = TITLE_TAG.length() + Languages::abbrev[lang].size() + 2;	// 1 for '[' + 1 for ':'
			texts.SetTextForLanguageNoID(s.mid(level + clen, len - level - clen-1), lang);
			if (config.majorStructVersion == 1)
			{						// none = there is no asterix followed by a number
				if (textID == 0)	// then this is the first line for the text
				{
					if (s.length() == len)
						++_structChanged;		// because textID will be calculated
					else
					{
						textID = texts.ID = s.mid(len + 1).toULongLong(); // len+1: length including '*'
						texts.collision = textID == 0 ? 0 : (textID >> ID_COLLISION_FACTOR);
					}
				}
			}
		}
		ids.what = newTag;
				// reads next line aand repeat loop
	} while (!reader.NextLine().isEmpty() && reader.l()[level] == '[');

	if ( !texts.IsEmpty())
	{
		if (textID && _textMap.contains(textID))	  // need this for texts with ID read from struct file
			texts.usageCount = ++_textMap[textID].usageCount;
		else if (!textID)						// when no id in file
		{
			textID = texts.CalcID(_textMap);	// sets new unique id and usage count as well
			++_structChanged;
		}

		if (!texts.usageCount)					// then not yet added
			texts.usageCount = 1;

		_textMap[textID] = texts;				// add new record or modify old record with the new usage count

		if (ids.what == IdsFromStruct::title)
			ids.titleID = textID;
		else
			ids.descID = textID;
	}
}

/*==========================================================================
* TASK:		helper - splits an image or video definition line into 
*				max 10 constituents (see below):
* EXPECTS: s : definition string w.o. leading spaces that determines the level
*				Formats (text not in < > is verbatim,  '-' starts remark):
*				  yet unprocessed files	
*					<full file path>
*					<file name&ext only> - this uses last used path either
*												from directory or previous file
*				  processed files 
*					Images:
*						<file name&ext only>(<id w.o. flags>,<width><height>,<orig width>x<orig height>,<upload date>,<file length)<directory path>
*					Videos:
*						<file name&ext only>(<id w.o. flags>,<upload date>,<file length)<directory path>
*					if 'V' is present it is a video file else it is an image file
*																		Image Video
* RETURNS: list of 0, 2, 5/6 or 9/10 strings:
*		empty list:	  unknown file type
*		2 element list:
*				full file path
*			always present
*				file name 												(#0)	(#0)
*				file type ('I', or 'V')									(#1)	(#1)
*			 this may or may not be present:
*				file ID													(#2)	(#2)
*			 if ID is present then for images these are also present	
*				image width												(#3)
*				image height											(#4)
*				image original width									(#5)
*				image original height									(#6)
*			 and this is present for both images and videos			
*				image date												(#7)	(#3)
*				file size - in bytes									(#8)	(#4)
*			this may be missing if so use last path)
*				folder path for files									(#9)	(#5)
* REMARKS:	- "normal" regular function split() could not be used as names
*					may contain braces and commas
*			- file name may contain any number of brackets
*			- result may contain fewer or more fields than required
*--------------------------------------------------------------------------*/
QStringList __imageMapStructLineToList(QString s)
{
	QStringList sl;
	int pos0 = 0, 
		pos = s.lastIndexOf('(');	// either the opening for file parameters after the file name
									// or, when no such parameters) inside the file name
	FileTypeImageVideo typ;

	auto typeStr = [&]() { return typ == ftImage ? "I" : "V"; };

	typ = IsImageOrVideoFile(s.mid(0, pos < 0 ? -1 : pos));	// the part before the last '(', based on file extension
	if (typ == ftUnknown)			// then maybe '(' inside file name
	{
		typ = IsImageOrVideoFile(s);// so check full name based on file extension
		if (typ == ftUnknown)	// this is still an unknown file type, return empty list
			return sl;
		else 
			pos = s.length();
	}
	
	sl.push_back(s.mid(0, pos));	// index #0 - file name + extension, possibly with path
	sl.push_back(typeStr());		// index #1 - file type

	if (pos < 0)					// unprocessed line: no parameter block (inside brackets) found in line 
		return sl;					// count == 2
		   
	// processed lines
	pos0 = pos + 1;	   				// after the opening brace into parameter block
	pos = s.indexOf(')',pos0);
	if (pos < 0)					// invalid parameters
		return sl;					// same as for unprocessed lines

	QRegExp rexp(",|x");
	sl += s.mid(pos0, pos - pos0).split(rexp);	// index #2..#9 for image: ID, width, height, owidth, oheight, length, date
												// index #2..#5 for video: ID, length, date	

//	if (typ == ftVideo && sl[2][0] == QChar('V'))	// for video files iD starts with 'V'
//		sl[2] = sl[2].mid(1); no need

	if ( (sl.size() != 9 && sl.size() != 5) || pos == s.length() )   // some problem, maybe path is missing (was s[pos + 1] != '/') 
		return sl;

	sl.push_back(s.mid(pos + 1));		// index #6 or #10 - original path 
	return sl;
}

/*==========================================================================
* TASK:		reads data for one image/video from structure
* EXPECTS: 	reader - open file reader, reader.l() contains 
*						the file specification line
*			level -  the level of images
*			album: add image to this album
*			thumbnail - is this the album thumbnail?
*
* RETURNS: ID of new item (image or vide) added. 
*			For images the IMAGE_ID_FLAG, For videos the VIDEO_ID_FLAG is set
*			in ID
* REMARKS: - line formats in reader.l() is described at '__imageMapStructLineToList'
*		   - image/video flag is set for them here and in _imageMap, and _videoMap
*				but not in the file
*		   - throws 'BadStruct' on error
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ImageOrVideoFromStruct(FileReader &reader, int level, Album &album, bool thumbnail)
{
	Image img;
	Video vid;

	QStringList sl = __imageMapStructLineToList(reader.l().mid(level));
	int n = sl.size();		// when all fields are present this should be 6 or 10 :
							// if no path was given then 5 or 9
							// if only the path name then 2
		// 
		// index in sl:  0       1      2    3      4             5               6         7     8     9
		// images:		name, "image", id, width, height, original width, original height, date, size, path
		// videos:		name, "video", id, date,  size,         path 

	ID_t id;
		// n:  images: 2 or 10, videos 2 or 5
	FileTypeImageVideo type = n >= 2 ? (sl[1][0] == QChar('I') ? ftImage : (sl[1][0] == QChar('V') ? ftVideo : ftUnknown)) : ftUnknown;
	if (n != 9 && n != 10 && n != 5 && n != 6)		   // maybe new image/video added and not yet processed
	{
		if (n != 2)	// it should be at least <config.dsSrc relative full path name> and <file type> (image or video or unknown)
			throw BadStruct(reader.ReadCount(), 
							QMainWindow::tr(StringToUtf8CString(QString("Wrong image parameter count:%1").arg(n)))); // or just the image name (from the same folder as the previous one)

		// expects: original/image/directory/inside/source/name.ext and 'type' set
		id = _AddImageOrVideoFromPathInStruct(sl[0],type);	 // then structure is changed
		if (id)		// has type flag set
		{
			if (thumbnail)		 // It's parent also changes
			{
				album.thumbnail = id;
				if (album.parent)
					_albumMap[album.parent].changed = true;
			}
			album.changed = true;			// set it as changed always 
		}
		if (type == ftImage)
			img = _imageMap[id];
		else
			vid = _videoMap[id];
	}
	else
	{
		id = sl[2].toULongLong();
		if (type == ftImage)	// n == 9 or 10
		{
			id |= IMAGE_ID_FLAG;
			if (_imageMap.contains(id))
				img = _imageMap[id];

			img.name = sl[0];
			img.SetResizeType();	// handle starting '!!'

			img.ID = id;
			img.dsize = QSize(sl[3].toInt(), sl[4].toInt());	// scaled size from .struct file
			img.osize = QSize(sl[5].toInt(), sl[6].toInt());	// original size - " -

	// calculated when asked for		img.aspect = img.height ? (double)img.width / (double)img.height : 1;
			img.uploadDate = DateFromString(sl[7]);
			if (img.uploadDate > _latestDateLimit)
				_latestDateLimit = img.uploadDate;
			// DEBUG:
			if (img.uploadDate.toJulianDay() < 0)
				return 0;

			img.fileSize = sl[8].toULongLong();
			if (n == 10)
				lastUsedItemPath = sl[9];

			img.path = lastUsedItemPath;

			img.exists = (img.fileSize != 0);	// non existing images have 0 size
		}
		else		// video
		{
			id |= VIDEO_ID_FLAG;
			if (_videoMap.contains(id))
				vid = _videoMap[id];

			vid.name = sl[0];
			vid.ID = id;
			vid.uploadDate = DateFromString(sl[3]);
			if (vid.uploadDate > _latestDateLimit)
				_latestDateLimit = vid.uploadDate;
			// DEBUG:
			if (vid.uploadDate.toJulianDay() < 0)
				return 0;

			vid.fileSize = sl[4].toULongLong();
			if (n == 6)
				lastUsedItemPath = sl[5];

			vid.path = lastUsedItemPath;

			vid.exists = (vid.fileSize != 0);	// non existing videos have 0 size
		}
	}
	IdsFromStruct ids;
	reader.NextLine();
	if (!reader.l().isEmpty() && reader.l()[level] == '[')
	{
		int nChanges = _structChanged;				  // original value
		_GetTextAndThumbnailIDsFromStruct(reader, ids, level);
		album.changed |= (nChanges != _structChanged);  // then image came from path in struct
	}
	if(!img.titleID || ids.titleID)		  // do not delete already existing text
		img.titleID = ids.titleID;		  // but overwrite if new text is entered
	if(!img.descID || ids.descID)
		img.descID = ids.descID;

	if(type == ftImage)
		_imageMap[id] = img;
	else
		_videoMap[id] = vid;

	return id;	// has the correct flag set
}

/*==========================================================================
* TASK:		helper - splits an album definition line into max 3 fields:
*				name, ID,  and path, some of these may be missing
* EXPECTS: s : INPUT string of definition w.o. leading spaces
*				format can be: 
*					<relative path name of album> -> new album
*				or
*					(A:1) or (C:1) - root album: no name, no path (C: - changed)
*				or
*					<original album name>(A:<ID>)<relative path> - 
*						exisiting album possibly unchanged
*			    or
*					<original album name>(C:<ID>)<relative path> - 
*						existing album defineitely changed
*          changed: OUTPUT if the album changed
*					true when the album is new (given with full path name)
*						or when the type is set to 'C'
* RETURNS: list of fields found  and 'changed' flag
*			line count in list can be:
*						0 - root album	:  (A:1)
*						1 - new album	:  this/is/a/new/album
*						2 - name and ID :  thisAlbum(A:ID)
*							album is in parent's path
*						3 - name, ID, path: name(A:ID)path/to/this
* REMARKS:	- "normal" regular function split() could not be used as names
*					may contain braces and commas
*			- result may contain fewer or more fields than required
*			- for root album sl is a single field, the ID which is 1
*			- when the album exists, but marked as 'C' ('changed') then its
*				parent (if it has any) - and only its parent -  must also change
*--------------------------------------------------------------------------*/
QStringList __albumMapStructLineToList(QString s, bool &changed)
{
	QStringList sl;
	int pos0 = 0, pos;
	do
	{
		pos = s.indexOf("(", pos0);
		pos0 = pos + 1;
	} 
	while (pos > 0 && s.at(pos0).unicode() != 'A' && s.at(pos0).unicode() != 'C' && s[pos+2].unicode() != ':');	// then '(A:<id>)<parent path>' is in file name

	if (pos == 0)			// root album
	{
		changed = (s.at(pos0).unicode() == 'C');
		return sl;
	}

	if (pos < 0)			 // full (virtual) path name of new album w.o. ID 
	{
		sl.push_back(s);
		changed = true;
		return sl;
	}

	changed = s[pos0].unicode() == 'A' ? false : true;

	if(pos)
		sl.push_back(s.mid(0, pos));			// album name

	//pos0 = pos + 1;	   		// after the opening brace
	pos0 += 2;				// skip the 'A:' or 'C:'
	pos = s.indexOf(')', pos0);
	if (pos < 0)
		return sl;

	sl.push_back(s.mid(pos0, pos - pos0));	// ID
	if (pos + 1 < s.length())
		sl.push_back(s.mid(pos + 1));			// path
	return sl;
}

/*==========================================================================
* TASK:		recursively reads album and all of its sub-albums from ".struct"
*			file
* EXPECTS: reader - open file reader,
*					reader.l() :
*						album definition line: 
*							<name>(A:id)'/'<original path>
*							or <original path>'/'<name>
*           parent: ID of parent
*			level:	album level (number of spaces before name)
* RETURNS:	ID of new album from structure
* REMARKS:	-if an album id line is in reader then the id inside it must be new
*			 otherwise the file is damaged
*			- when a new album added, then its parent also changed
*			- album definitions are delimited with a single empty line
*			- each album line starts with as many spaces as its level
*			- if the next album definition is on the same level
*			- as this album, then returns
*			- expects a single root album
*			- albums are added to '_albumMap' as soon as they are identified
*				and that record is modified iater by adding the images and 
*				albums to it
*		   - throws 'BadStruct' on error
*			- the album created here must be added to _albumMap in caller
*				therefore it will be added later than its sub albums
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ReadAlbumFromStruct(FileReader &reader, ID_t parent, int level)
{
	Album album;	// temporary album to add to map

	bool albumDefnitelyChanged;
	QStringList sl = __albumMapStructLineToList(reader.l().mid(level), albumDefnitelyChanged);
	int n = sl.size();		// should 0,1,2 or 3
							//	0: root album
							//	1: sl[0] = path name (not yet processed)
							//	2: top level album, sl[0] = album name sl[1] = ID
							//	3: sub album, sl[0] = album name sl[1] = ID, sl[2]= album path
	if (n > 3)
		throw BadStruct(reader.ReadCount(),"Wrong album parameter count");

	Album *aParent = nullptr;
	if (n)						 // else root album with no name
	{
		album.name = sl[0];
		album.parent = parent;
		aParent = &_albumMap[parent];
	}

	if (albumDefnitelyChanged)		// type was 'C' and not 'A'
	{								// in this case immediate parent must be changed too
		album.changed = true;
		if (parent != 0)
			aParent->changed = true;
		++_structChanged;
	}

	int	changeCountBefore = _structChanged;		// save to determine if album changed
												// if any text or the icon ID is new then _structChanged incremented
	ID_t id;
	if (n == 0)		// root album
	{
			// dummy album for latest Images must come first as sets album's ID
		album.ID = id = (2 | ALBUM_ID_FLAG);
		album.exists = true;		// do not check: virtual!
		_albumMap[id] = album;
				// root album with ID 1
		album.ID = id = (1 | ALBUM_ID_FLAG);
		album.exists = true;		// do not check: virtual!
		_albumMap[id] = album;		// and not with _albumMap.Add
	}
	else if (n == 1)	// then no ID is determined yet, sl[0] contains the whole config.dsSrc relative path
	{
		if (parent != 0)
			aParent->changed = true;
		if(QFile::exists(sl[0]))				// then images are inside this (not virtual directory)
			lastUsedItemPath = sl[0] + "/";

		bool added;
		id = _albumMap.Add(aParent->path + sl[0], added);	// add new album to global album list
															// may invalidate aParent
		album = _albumMap[id];
		++_structChanged;		// and also album is changed
	}
	else   // n == 3 || n == 2
	{
		if (n == 2)	// name & ID but no path
		{
			if (level < 2)	   // level == 0: root, level == 1 top level album
				lastUsedItemPath.clear();
			album.path = lastUsedItemPath;
			if (level == 1)	// top level album> its name is path for images and sub albums
				lastUsedItemPath = sl[0] + "/";
		}
		else // n==3 -> name, ID and path
		{
			album.path = sl[2];	// images are inside this album
			lastUsedItemPath = album.FullName() + "/";
		}
		id = sl[1].toULongLong() | ALBUM_ID_FLAG;
		if (_albumMap.contains(id) && _albumMap[id].FullName() != album.FullName() )
			throw BadStruct(reader.ReadCount(), QMainWindow::tr(StringToUtf8CString(QString("'%1' - duplicated album ID").arg(id))));


		album.ID = id;				// has ALBUM_ID_FLAG set
		album.exists = true;		// do not check: these can be virtual!  QFileInfo::exists(album.FullName());
	}

	reader.NextLine(); // next line after an album definition line
					   // when empty same level or sub albums follows
	IdsFromStruct ids;
	while (reader.Ok())
	{
		if (reader.l().isEmpty())
		{
			if (!ids.IsEmpty())
			{
				if (ids.titleID)
					album.titleID = ids.titleID;
				if (ids.descID)
					album.descID = ids.descID;
				if (ids.thumbnailID)
					album.thumbnail = ids.thumbnailID;
				ids.Clear();
			}
			while(reader.Ok() && reader.l().isEmpty())	// drop all consecutive empty lines
				reader.NextLine();					// next album definition line
			while (reader.Ok() && reader.l()[level] == ' ')		// new sub album
			{													// process it 
				ID_t aid = _ReadAlbumFromStruct(reader, id, level + 1); // returns when same level sub album is found
				album.items.push_back(aid);
			}
			if (_albumMap.contains(id))						 // album is not yet filled in, but the changed flag may be set
				album.changed |= _albumMap[id].changed;		 // for it because of a sub-album
			_albumMap[id] = album;
			return id;			 // this new album definition is at the same level as we are
		}
		else
		{				// strings in lText must be empty
			if (reader.l()[level] == '[') // then title, description or thumbnail line
			{		
				_GetTextAndThumbnailIDsFromStruct(reader, ids, level);
				if (changeCountBefore != _structChanged)
				{
					album.changed = true;
					if (parent)
						_albumMap[parent].changed = true;	// aParent pointer might be invalidated
				}
			}
			else if(reader.Ok())		   // this must be an image line, unless file is ended
			{							   // but image files are inside the album (one level down)
				ID_t iid = _ImageOrVideoFromStruct(reader, level+1, album, false);		   // false:not album thumbnail
				album.items.push_back(iid);
			}
		}
	} 
		// last album
	if (!ids.IsEmpty())
	{
		if (ids.titleID)
			album.titleID = ids.titleID;
		if (ids.descID)
			album.descID = ids.descID;
		if (ids.thumbnailID)
			album.thumbnail = ids.thumbnailID;
		ids.Clear();
	}
	if(!_albumMap.contains(id))
		_albumMap[id] = album;

	return id;
}

/*==========================================================================
* TASK:		reads whole album hierarchy from structure file
* EXPECTS: 'from' is the full path name of the file
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadStruct(QString from)
{
	bool error = false;

	try
	{
		FileReader reader(from);	// with ReadLine() discard empty lines, and trim lines
		if (!reader.Ok())
			throw BadStruct(reader.ReadCount(), QMainWindow::tr("Read error"));

		QStringList sl;
		QString line = reader.NextLine(true);	// version line (comment format)

		if (line.left(versionStr.length()) != versionStr)
			throw BadStruct(reader.ReadCount(), QMainWindow::tr("Bad version string"));
		else
		{
			line = line.mid(versionStr.length());
			int pos = line.indexOf('.');
			if(pos < 0)
				throw BadStruct(reader.ReadCount(), QMainWindow::tr("Missing '.' from version"));

			config.majorStructVersion = line.left(pos).toInt();
			config.minorStructVersion = line.mid(pos+1).toInt();

			_LanguageFromStruct(reader);  // throws when error
			emit SignalSetLanguagesToUI();

			// from now on we need the empty lines as well
			reader.ReadLine();		// discard empty and comment
			if (reader.l()[0] != '(' || (reader.l()[1] != 'A' && reader.l()[1] != 'C'))
				throw BadStruct(reader.ReadCount(), QMainWindow::tr("Invalid / empty root album line"));
					// recursive album read. there is only one top level album
					// with id == ROOT_ALBUM_ID (id == ALBUM_ID_FLAG is not used)
			_ReadAlbumFromStruct(reader, 0, 0); 
			if (error)
				throw BadStruct(reader.ReadCount(),"Error");
		}
		return true;
	}
	catch (BadStruct b)
	{
		QMessageBox(QMessageBox::Critical, QMainWindow::tr("falconG - Error"), 
						QMainWindow::tr("Damaged structure file!\n"
										"Message: '") +  
						b.msg + 
						QMainWindow::tr("\n\n"
										"Processing aborted, because continuing\n"
										"could destroy your old .struct file!\n"
										"%1 lines read so far. ").arg(b.cnt), 
							QMessageBox::Abort, frmMain).exec();
		b.cnt = 1;
		throw(b);
		return false;
	}
}

/*==========================================================================
* TASK:		Reads whole gallery hierarchy starting at directory 'root'
*			If it is a JAlbum directory with corresponding files
*			adds only images and albums set in those files
*			otherwise adds each image and sub-folders to it
* EXPECTS: 'root' path name of uppermost source album directory
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadFromDirs()
{
	if (!Languages::Read())		// cancelled, no default set
		return false;
			// first languages from en.lang, etc
	emit SignalSetLanguagesToUI();

			// get number of sub albums for display only
	QDirIterator dirIter(config.dsSrc.ToString(), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);	// count directories
	int directoryCount = 0;												// iterator points BEFORE first entry
	QString s;
	emit SignalToSetProgressParams(0, 0, 0, 0);	// phase 0
	while (dirIter.hasNext())
	{
		QFileInfo fi(dirIter.next());
		if (fi.isDir())
		{
			s = dirIter.fileName();
			if (s[0] != '.' && s != "res" && s != "cache" && s != "thumbs")	// not a hidden or resource directory?
			{
				++directoryCount;
				emit SignalProgressPos(directoryCount, directoryCount);
			}
		}
	}

	emit SetDirectoryCountTo(directoryCount);
	emit SignalToSetProgressParams(0, directoryCount, 0, 0);	// phase 0


	QString root = config.dsSrc.ToString();
	QDir dir(root);

	ID_t rootId = 1 | ALBUM_ID_FLAG;
	_root.Clear();
	_root.ID = rootId;							// id = 0: invalid
	_root.path = root;
	// root has no name and no path it is inside config.dsSrc
	_root.exists = true;

	_albumMap[_root.ID] = _root;	// a copy of _root is first on list	

	_remDsp.Init(directoryCount);
	_JReadOneLevel(_albumMap[rootId] );	// recursive read of all levels
	if (!_processing)
		return false;

	_root = _albumMap[rootId];	// get values stored in hierarchy back to _root
	++_structChanged;					// i.e. must write 'gallery.struct'
	return true;
}

/*============================================================================
  * TASK:   recrates album structure but can't recover album paths and 
  *			image names or dimensions
  * EXPECTS:config is set up
  * RETURNS: true wen success and album in memory and writes gallery.struct.recovered
  * GLOBALS: config
  * REMARKS: use in an emergency when both the gallery.struct and the gallerry.struct~
  *				files are lost
  *			 if any version of the same gallery struct exists then at least some of the
  *				lost information may be recovered by hand
 *--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadFromGallery()
{
	// read file list from directory
	// determine number of languages
	// get language texts
	// for each file sets (one file for each languages) read
	//   uplink, image names, album names, image and album titles and descriptions
	// 
	return false;
}

/*============================================================================
* TASK:	copies directory 'js'
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 1: error writing some file
* REMARKS:  - directory 'js' exists
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoCopyJs()
{
	QString src = PROGRAM_CONFIG::samplePath+"js/",
			dest = (config.dsGallery + config.dsGRoot).ToString() + "js/";
	QDir dir(src);  // js in actual progam directory
	QFileInfoList list = dir.entryInfoList(QDir::Files);
	int res = 0;
	QString dname;
	for (QFileInfo& fi : list)
		res |= CopyOneFile(src + fi.fileName(), dest + fi.fileName()) ? 0: 1;
//		res |= CopyOneFile(fi.absoluteFilePath(), dest + fi.fileName()) ? 0: 1;

	return res;
}

/*============================================================================
* TASK:	Creates constants.php		-   - " -
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 1: error writing file
* REMARKS:  - directory 'res' exists
*			- the file 'up-icon.png' and 'left-icon.png' are not copied, 
*			  but created from icon images in the res directory of falconG
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoCopyRes()
{

	QString src = PROGRAM_CONFIG::samplePath+"res/",
			dest = (config.dsGallery + config.dsGRoot).ToString() + "res/";
	QDir dir(src);  // res in actual progam directory
	QFileInfoList list = dir.entryInfoList(QDir::Files);
	for (QFileInfo &fi : list)
	{
		if(fi.fileName() != QString("left-icon.png") && fi.fileName() != QString("up-icon.png"))
			CopyOneFile(fi.absoluteFilePath(), dest + fi.fileName());	// overwrite existing
	}
//	emit SignalToCreateIcon(dest, "left-icon.png");
	emit SignalToCreateIcon(dest, "up-icon.png");
	return 0;	
}

/*============================================================================
* TASK:		Writes whole directory structure into 'gallery.struct' in a separate thread
* EXPECTS: keep: do not replace backup file with original
* GLOBALS:
* REMARKS: Directory structure is written into file gallery.tmp, so if an 
*			error occurs then the originla structure file is still available.
*			After a successfull write an existing backup file 'gallery.struct~'
*			is deleted,  the gallery.struct file is renamed to 'gallery.struct~'
*			and then the temporary file is renamed to 'gallery.struct'. 
*			If 'keep' is true, then old backup file is kept and the 
*			temporary file is renamed directly to 'gallery.struct'
*			This way the original state (before any changes) can be 
*			retsored if required.
*
*--------------------------------------------------------------------------*/
int AlbumGenerator::WriteDirStruct(bool keep)
{
	_structWritten = false;
	pWriteStructThread = new AlbumStructWriterThread(*this);
	connect(pWriteStructThread, &AlbumStructWriterThread::resultReady, this, &AlbumGenerator::_WriteStructReady);
	connect(pWriteStructThread, &AlbumStructWriterThread::finished, pWriteStructThread, &QObject::deleteLater);
	_keepPreviousBackup = keep;

	pWriteStructThread->start();
// DEBUG:
//	pWriteStructThread->run();
	return 0;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_HtaccessToString()
{
	QString sS = config.sServerAddress;
	if (sS.lastIndexOf('/'))					// cut 'http(s)://' if present
		sS = sS.mid(sS.lastIndexOf('/') + 1);	

	QString s = "RewriteEngine On\n";
	if(config.bForceSSL)
		s += QString("# change web access to https by redirecting page with code 301 (moved permanently)\n"
		"RewriteCond %{SERVER_PORT} 80\n"
		"RewriteRule ^(.*)$ https ://"	+ sS +	"/$1 [R=301,L]\n");

	if (!config.bCanDownload)
		s += QString("# only allow image download from a web page hosted here on " + s + "\n"
			"RewriteCond %{REQUEST_URI}	\\.(jpg|gif|png)[NC]\n"
			"RewriteCond %{HTTP_REFERER} !" + sS + "\n"
			"RewriteRule ^.+$	notallowed.html\n");

	//s += QString("# change addresses from <your site on server>/" + config.sBaseName + "<page id><language code>\n"
	//	"# to <your site on server>/index.php?gid=<page id>&lang=<language code>\n"
	//	"RewriteCond %{REQUEST_URI} " + config.sBaseName + "(^.+$)\n"
	//	"RewriteRule"  + config.sBaseName + "([^\\d+])([a-z]{0:2})	index.php?$1\\&lang=$2\n"
	//	);
	return s;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString AlbumGenerator::_GoogleAnaliticsOn()
{
	return 	QString("<!--Global site tag(gtag.js) - Google Analytics-->\n"
		"<script async src=\"https://www.googletagmanager.com/gtag/js?id=%1\"></script>\n"
		"<script>\n"
		"window.dataLayer = window.dataLayer || [];\n"
		"function gtag() { dataLayer.push(arguments); }\n"
		"gtag('js', new Date());\n"
		"gtag('config', '%1');\n"
		"</script>\n").arg(config.googleAnalTrackingCode);

}

/*============================================================================
* TASK:		falconG.css		-  from all parameters
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_SaveFalconGCss()
{
	// generate new falconG.css into temporary files
	CssCreator cssCreator;
	QString tmpName = QString(__CssDir.ToString()) + "tmpfg.css";
	cssCreator.Create(tmpName,false);
	tmpName = BackupAndRename(QString(__CssDir.ToString()) + "falconG.css", tmpName);
	if (!tmpName.isEmpty())
	{
		QMessageBox::warning(nullptr, tr("falconG - Warning"), tmpName, QMessageBox::Ok);
		return 8;
	}
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/

/*============================================================================
* TASK:		creates name for album
* EXPECTS:	base: path name of album (may be a file name only and may even w.o. extension)
*			language - actual language
*			toServerPath - add a http/https prefix too, else show file name in
*				destination directory
* GLOBALS: config
* RETURNS: path name of album, always with an extension
* REMARKS: if no extension is in base it adds a '.html' extension to it
*--------------------------------------------------------------------------*/
QString AlbumGenerator::RootNameFromBase(QString base, int language, bool toServerPath)
{
	bool multipleLanguages = Languages::Count() > 1;

	QString path, name, ext;						// see if name has a path, which may be either outside or inside of gallery root
	SeparateFileNamePath(base, path, name, &ext);
	if (ext.isEmpty())
		ext = ".html";
	if (path.isEmpty())
		base = config.dsGRoot.ToString() + name;
	else
		base = (config.dsGRoot + path).ToString() + name;
	if (multipleLanguages)
		base += Languages::abbrev.at(language); // e.g. index_en.html

	if (toServerPath)
	{
		path = config.sServerAddress;
		if (path.toLower().left(4) == "http")
			path = path + "/";
		else
			path = (config.bForceSSL ? "https://" : "http://") + path + "/";
		if (path[path.length() - 1] != '/')
			path += '/';
		base = path + base;
	}
	return base + ext;
}

/*========================================================
 * TASK:	Process one image and thumbnail using
 *			converters
 * PARAMS:	im - actual source image 
 *			converter - common converter for images
 *			thumbConverter - same for thumbnails
 * GLOBALS:
 * RETURNS:	nothing
 * REMARKS:	- 
 *			- im may have its dimensions read from structure
 *				file, but it also be a new image w.o. any
 *				size data read in
 *			- im may contain an image different from a
 *				previous process. It may be new, it may be
 *				of a different size or a newr date than the
 *				processed one, whose name is <im.ID>.jpg
 *-------------------------------------------------------*/
void AlbumGenerator::_ProcessOneImage(Image &im, ImageConverter &converter, std::atomic_int &cnt)
{
	int doProcess = prImage | prThumb;	// suppose both image and thumb must be created 

										// resize and copy and  watermark
	QString src = (config.dsSrc + im.path).ToString() + im.name,   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)
			dst = config.LocalImageDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions), // e.g. imgs/123456789.jpg
			thumbName = config.LocalThumbnailDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions);// e.g. thumbs/123456789.jpg

	QFileInfo fiSrc(src), fiThumb(thumbName), fiDest(dst);						// test for source image
	bool srcExists = fiSrc.exists(),
		dstExists = fiDest.exists(dst),
		thumbExists = fiThumb.exists();

	if (!srcExists)	// then it might have been removed
	{
		im.exists = false;
		if (dstExists)	// source file was deleted,remove destination image
		{
			QFile::remove(dst);				
			if (thumbExists)
				QFile::remove(thumbName);	// and thumbnail
		}
		return;
	}

	QSize dstSize;		// real dimensions of existing destination 
						// image or empty if no such image exists
	if (dstExists)
	{
		QImageReader destReader(dst, QByteArray("jpg"));
		dstSize = destReader.size();
	}
	ImageReader imgReader(src, im.dontResize);			// constructor opens file, but doesn`t read image! 'autoTransform' is set to true
	QSize imgSize = imgReader.size();					// size read from file (autoTransform affects only read() and not this size())

	QImageIOHandler::Transformations tr = imgReader.transformation(); // from file
	if (tr & (QImageIOHandler::TransformationRotate90 | QImageIOHandler::TransformationMirrorAndRotate90))
	{
		imgSize.transpose();
		if (!dstSize.isEmpty())
			dstSize.transpose();
	}

	if (im.osize.isEmpty())	// no structure file or image is not processed yet
		im.osize = imgSize;	// then set original size from existing image

	im.GetResizedDimensions(dstSize);	// into 'size' and set 'bSizeDifferent'

	if (imgSize.width() != im.osize.width() || imgSize.height() != im.osize. height())	// source changed rel. to data base: must re-create everything
	{
		im.osize = imgSize;

		im.GetResizedDimensions(dstSize);// must recalculate, 
		im.bSizeDifferent = true;		// this signals	dest. image changed
		im.SetNewDimensions();			// process to these sizes
	}

	QDateTime dtSrc = fiSrc.lastModified();		// date of creation of source image.
// DEBUG
//		QString gsDate = dtSrc.toString();
// /DEBUG

	if (dstExists)		// then test if it should be re-created (width = 0: only added by name)
	{					// and if not then do not process it
		QDateTime dtDestCreated = fiDest.lastModified();	   // created() => birthTime() since Qt 5.10
// DEBUG
//			gsDate = dtDestCreated.toString();
// /DEBUG
		bool destIsNewer = dtDestCreated > dtSrc;	   // false for invalid date (no file) 
		int64_t	fiSize = fiSrc.size();

		if (config.bGenerateAll && config.bButImages)
			doProcess -= prImage;			// then do not process
		else if(!config.bGenerateAll)
		{
			if ( !im.bSizeDifferent && (fiSize == im.fileSize) && destIsNewer)
				doProcess -= prImage;			// then do not process
		}
		if (destIsNewer)
			im.uploadDate = dtDestCreated.date();

		if (im.fileSize != fiSize)			// source image size changed: set new size in structure
			im.fileSize = fiSize;
	}
	if (thumbExists)		// then test if this image was modified (width = 0: only added by name)
	{
		QDateTime dtThumb = fiThumb.lastModified();		  // date of creation of thumbnail image
		bool thumbIsNewer = dtThumb > dtSrc;
	//	// order of these checks is important! (for thumbnails always must check size)
		if (!config.bGenerateAll && (config.bButImages || (!MustRecreateThumbBasedOnImageDimensions(thumbName, im) && thumbIsNewer)) )
			doProcess -= prThumb;			// then do not process
// DEBUG
		//else
		//	doProcess = doProcess;
// /DEBUG
	}
	// debug
	//		doProcess = doProcessThumb = false;
	// /debug
	if (doProcess)
	{
		//		int btn;
				//if (!QFile::exists(dst) && !config.bOvrImages)
				//	if ((btn = QMessageBox(QMessageBox::Warning, QMainWindow::tr("falconG warning"), QMainWindow::tr("Image \n'%s'\n exists").arg(dst), QMessageBox::Yes | QMessageBox::Abort).exec()) == QMessageBox::Abort)
				//	{ 
				//			emit SignalToEnableEditTab(true);
				//			return false;
				//  }
		++_structChanged;			// when image changes structure must be re-saved

		WaterMark *pwm = nullptr;
		if (config.waterMark.used)
			pwm = &config.waterMark;
		converter.flags = doProcess + (im.dontResize ? dontResize : 0) + 
							(config.doNotEnlarge ? dontEnlarge : 0);

		imgReader.thumbSize = im.tsize;
		imgReader.imgSize = im.rsize;
		converter.Process(imgReader, dst, thumbName, pwm);
		//im.owidth = converter.oSize.width();
		//im.oheight = converter.oSize.height();
		//im.width = converter.newSize.width();
		//im.height = converter.newSize.height();
		im.changed = true;			// so check in struct file before writing to disk
		_imageMap[im.ID] = im;

	}
	if (im.uploadDate > _latestDateLimit)	// find latest upload date
		_latestDateLimit = im.uploadDate;	// will be put into "latest.html"

	++cnt;
}

/*============================================================================
* TASK: gets images from source directories resize them if needed, then 
*	optionally add watermark. Puts all images in image directory, plus 
*	 creates a list of the images processed in the image directory
* EXPECTS:	none, 
*			these are set: _imageMap,
*							_actLanguage - index of actual language
* GLOBALS:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:	- must come before ProcessVideos()
*			- initializes _remDsp
*--------------------------------------------------------------------------*/
int AlbumGenerator::_ProcessImages()
{
	if (config.bButImages)
		return 0;

	// progress bar
	emit SignalToSetProgressParams(0, _imageMap.size()+_videoMap.size(), 0, 1); // phase = 1
	std::atomic_int cnt = 0;	// count of images copied

	QRect maxSize{ config.imageWidth, config.imageHeight, config.thumbWidth, config.thumbHeight};
	ImageConverter converter(config.doNotEnlarge);

	emit SignalToEnableEditTab(false);

	_remDsp.Init(_imageMap.size()+_videoMap.size());
	if (config.waterMark.used)
		config.waterMark.SetupMark();

	for (auto &im : _imageMap)
	{
		if (!_processing)		// stopped?
		{
			emit SignalToEnableEditTab(true);
			return 8;
		}
		if (im.name.isEmpty())
			continue;
		if (cnt > 10)			// delayed display of elapsed/remaining time
		{
			_remDsp.Update(cnt);
			emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, cnt, true);
		}

		_ProcessOneImage(im, converter, cnt);
		emit SignalProgressPos(cnt, _ItemSize()); // progress bar
	}
//	emit SignalToEnableEditTab(true);
	return 0;
}
/*============================================================================
* TASK: gets videos from source directories 
*		Puts all videso in 'vids' directory, plus
*	 creates a list of the videos processed in the image directory
* EXPECTS:	none,
*			these are set: _imageMap,_videoMap
*							_actLanguage - index of actual language
* GLOBALS:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_ProcessVideos()
{
	if (config.bButImages || !_processing)
		return 0;

	// progress bar
	emit SignalToSetProgressParams(0, _ItemSize(), 0, 1); // phase = 1
	std::atomic_int cnt = TotalCount();	// count of images and videos copied so far

	for (auto im : _videoMap)
	{
		if (!_processing)		// stopped?
		{
			emit SignalToEnableEditTab(true);
			return 8;
		}
		if (im.name.isEmpty())
			continue;
		// TODO here we should convert mp4 files which won't display in an HTML5 video tag
		// to one that does
		// at the moment: just copy file to vids directory
		QString src = (config.dsSrc + im.path).ToString() + im.name,   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)
				dst = config.LocalVideoDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions);

		QFile file(src);
		file.copy(dst);

		if (cnt > 10)			// delayed display of elapsed/remaining time
		{
			_remDsp.Update(cnt);
			emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, cnt, true);
		}
		emit SignalProgressPos(++cnt, _ItemSize()); // progress bar
	}
	emit SignalToEnableEditTab(true);
	return 0;
}

//----------------------------------------------------------------------------------

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString AlbumGenerator::_IncludeFacebookLibrary()
{
/*
    <script async defer crossorigin="anonymous" 
            src="https://connect.facebook.net/en_US/sdk.js#xfbml=1&version=v8.0" nonce="ouGJwYtd">
    </script>
*/
	return QString(
		R"(<!--get facebooks js code-->
	<script async defer crossorigin = "anonymous"
			src = "https://connect.facebook.net/)") + Languages::countryCode[_actLanguage] + 
					QString( R"(/sdk.js#xfbml=1&version=v8.0" nonce = "ouGJwYtd">
	</script> )");
}

/*============================================================================
  * TASK:	emit facebook link 	when required
  * EXPECTS: linkNam - name to link to e.g. https://your.site/directory
  *			ID	- different handlig for root album
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteFacebookLink(QString linkName, ID_t ID)
{
	if (!config.bFacebookLink)
		return;
		// facebook link
	QString updir = (ID == ROOT_ALBUM_ID ? "" : "../");
	QUrl url(linkName); // https://andreasfalco.com/albums/album1234.html\" 

	_ofs << R"(<div class="fb-share-button" data-href=")"
		<< url.toEncoded()
		<< R"(" data-layout="button_count" data-size="small"><a target="_blank" href="https://www.facebook.com/sharer/sharer.php?u=)"
		<< url.toEncoded()
		<< R"(%2F&amp;src=sdkpreparse" class="fb-xfbml-parse-ignore">)"
		<< Languages::share[_actLanguage]
		<< "</a></div>\n<div id=\"fb-root\"></div>";
}


QString AlbumGenerator::_PageHeadToString(ID_t id)
{
	QString supdir = (id == ROOT_ALBUM_ID ? "" : "../"),
			sCssLink = QString("<link rel=\"stylesheet\" href=\"") +
					supdir + 
					config.dsCssDir.ToString();

	QString s = QString("<!DOCTYPE html>\n"
		"<html lang = \"" + Languages::abbrev[_actLanguage] + "\">\n"		  // e.g. en from en_GB
		"<head>\n");
	if (config.googleAnalyticsOn)
		s += _GoogleAnaliticsOn();

	s += QString("<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
		"<meta name=\"generator\" content=\"falconG\">\n"
		"<title>" + config.sGalleryTitle + "</title>\n");
	if (!config.sDescription.IsEmpty())
		s += QString("<meta name=\"description\" content=\"" + config.sDescription + "\"/>\n");
	if (!config.sKeywords.IsEmpty())
		s += QString("<meta name=\"keywords\" content=\"" + config.sKeywords + "\"/>\n");

	s += QString(sCssLink + "falconG.css\">\n");
	if (id == ROOT_ALBUM_ID)
		s += "<script type=\"text/javascript\" src=\"" + supdir + "js/latestList"+ Languages::abbrev[_actLanguage] +".js\"></script>" +
		"<script type=\"text/javascript\" src=\"" + supdir + "js/latest.js\"></script>";

	s += "<script type=\"text/javascript\" src=\"" + supdir + "js/falconG.js\"></script>";

	if (config.bFacebookLink)
		s += _IncludeFacebookLibrary();

	s += QString("\n</head>\n");

	return s;
}

/*========================================================
 * TASK:	emit the sticky menu line at the top of the window
 * EXPECTS: album - album to be created  
 *			uplink - page link ifany
 * GLOBALS:	Languages, 'config'
 * RETURNS: nothing
 * REMARKS: -if and no uplink is given in config no UP button is generated
 *-------------------------------------------------------*/
void AlbumGenerator::_OutputNav(Album &album, QString uplink)
{
	QString updir = (album.ID == ROOT_ALBUM_ID) ? "" : "../";

	auto outputMenuLine = [&](QString id, QString href, QString text, QString hint=QString())
	{
		_ofs << "    <a class = \"menu-item\" id=\"" << id << "\" href=\"" << href << "\">" << text <<"</a>\n";
	};

	QString qsParent = album.parent > RECENT_ALBUM_ID ? _albumMap[album.parent].BareName() : config.sMainPage.ToString();
	qsParent = RootNameFromBase(qsParent, _actLanguage);
	if (album.parent <= RECENT_ALBUM_ID)	// for uplinks to index you need to go up one directory, for others you do not
		qsParent = "../" + qsParent;
	// menu buttons
	_ofs << "   <nav>\n";
	if (!updir.isEmpty() && !uplink.isEmpty())
		outputMenuLine("uplink", qsParent, "&nbsp;", Languages::upOneLevel[_actLanguage]);
//		<< "\"><img src=\"" + updir + "res/up-icon.png\" style=\"height:14px;\" title=\"" + Languages::upOneLevel[_actLanguage] + "\" alt=\""
//		+ Languages::upOneLevel[_actLanguage] + "\"></a>\n";			  // UP link
	outputMenuLine("home", updir + config.homeLink, Languages::toHomePage[_actLanguage]);
	if (config.bMenuToAbout)
	{
		QString s = config.sAbout;
		if (s.isEmpty())
			s = "about.html";
		outputMenuLine("about", updir + Languages::FileNameForLanguage(s, _actLanguage), Languages::toAboutPage[_actLanguage]);
	}
	if (config.bMenuToContact)
		outputMenuLine("contact", "mailto:" + config.sMailTo, Languages::toContact[_actLanguage]);
	if (config.bMenuToDescriptions && album.DescCount() > 0)
		_ofs << "	<a class = \"menu-item\" id=\"descriptions\", href=\"#\" onclick=\"javascript:ShowHide()\">" << Languages::showDescriptions[_actLanguage] << "</a>\n";
	if (config.bMenuToToggleCaptions && (album.TitleCount() > 0 || album.ImageCount() > 0) && !Languages::coupleCaptions[_actLanguage].isEmpty())
		_ofs << "	<a class = \"menu-item\" id=\"captions\", href=\"#\" onclick=\"javascript:ShowHide()\">" << Languages::coupleCaptions[_actLanguage] << "</a>\n";
	if (album.SubAlbumCount() > 0  && album.ImageCount() > 0 &&  !Languages::toAlbums[_actLanguage].isEmpty())	// when no images or no albums no need to jump to albums
		outputMenuLine("toAlbums", "#albums", Languages::toAlbums[_actLanguage]);								  // to albums
	if (album.ID > RECENT_ALBUM_ID && config.bGenerateLatestUploads && _latestImages.list.size())	// if there are no images in this caregory, do not add link
	{
		QString qs = "latest";
		if (Languages::Count() > 1)
			qs += Languages::abbrev[_actLanguage];
		qs += ".html";
		outputMenuLine("latest", qs, Languages::latestTitle[_actLanguage]);
	}
	//// debug
	//"<p style=\"margin-left:10px; font-size:8pt;font-family:Arial;\"id=\"felbontas\"></p>  <!--debug: display screen resolution-->
	_ofs << "&nbsp;&nbsp;";
	_WriteFacebookLink(album.LinkName(_actLanguage, true), 1);	// only when required
	
	_ofs << "\n   </nav>\n";
}

/*============================================================================
* TASK: emits header section of a web page for a given album in a given language
* EXPECTS:	album - album to be created  
*			_actLanguage - index of actual language
* GLOBALS:	Languages, 'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS: - special handling for root album(s) (ID is 1) 
*				no '../' prefix for css, res, etb
*				albums/ prepended to album links
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteHeaderSection(Album &album)
{

	_ofs << "   <div class=\"header\">\n"
		"    <a href=\"" << SiteLink(_actLanguage) << "\">"
		"<span class=\"falconG\">" << config.sGalleryTitle << "</span>"
		"</a>&nbsp; &nbsp;";

// _actLanguage switch texts
// TODO: when language count is larger than, say 3 use a combo box for selection
	for (int i = 0; i < Languages::Count(); ++i)
		if (i != _actLanguage)
			_ofs << "     <a class=\"langs\" href=\"" + album.NameFromID(i) + "\">" << Languages::names[i] << "</a>&nbsp;&nbsp\n";
	_ofs << "     <br><br><br>\n";
	if (album.ID == RECENT_ALBUM_ID)
	{
		QString qs = Languages::latestTitle[_actLanguage];
		if(!qs.isEmpty())
			_ofs << "     <h2 class=\"gallery-title\">" << DecodeLF(qs, 1) << "</h2><br><br>\n";
		qs = Languages::latestDesc[_actLanguage];
		if(!qs.isEmpty())
			_ofs << "     <p class=\"gallery-desc\">"   << DecodeLF(qs, 1) << "</p>\n";
	}
	else
	{
		if (album.titleID)
			_ofs << "     <h2 class=\"gallery-title\">" << DecodeLF(_textMap[album.titleID][_actLanguage], 1) << "</h2><br><br>\n";
		if (album.descID)
			_ofs << "     <p class=\"gallery-desc\">"   << DecodeLF(_textMap[album.descID][_actLanguage], 1) << "</p>\n";
	}

	_ofs << R"X(
    <div id="lightbox" class="lightbox">
	  <div id="lb-flex">
		<img id="lightbox-img" onclick="LightBoxFadeOut()">
		<p id="lightbox-caption" class="lightbox-caption"></p>
	  </div>
	</div>
			  )X";
	_ofs << "</div>\n";	 //  header
	return 0;
}

/*============================================================================
* TASK:		write HTML footer with image and album counts
* EXPECTS:	album - album for page
* GLOBALS:
* REMARKS: image count includes videos
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteFooterSection(Album & album)
{
	_ofs << " <!-- footer section with social media links -->\n"
		"<footer class = \"footer\">\n"
		<< QString(Languages::countOfImages[_actLanguage]).arg(album.ImageCount()+album.VideoCount()).arg(album.SubAlbumCount()) << "<br><br>\n"
		<< Languages::falconG[_actLanguage] << "<br>\n"
			"</footer>\n";

	return 0;
}

/*============================================================================
* TASK:		writes either an image or a gallery section
* EXPECTS:	album		- actual album whose content (images and sub-albums) is written
*			typeFlag	- only write items of this type
*			idIndex		- index in ID list for images or albums (when bWriteSubAlbums is true)
*							-1: this is an image for  the latest uploads
* GLOBALS: config, Languages, _actLanguage
* RETURNS:	0: OK, -1: album does not exist, -2: image does not exist
* REMARKS:  - root albums (ID == 1+ALBUM_ID_FLAG) are written into gallery root, others
*			  are put into directory 'albums' 
*			- common for images and albums as both have thumbnail
*			- video section is written in _WriteVideoContainer()
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteGalleryContainer(Album & album, ID_t typeFlag, int idIndex)
{

	IdList& idList = album.items;
	if (idIndex >= 0 && idList.isEmpty())
		return typeFlag == ALBUM_ID_FLAG ? -1 : -2;

	ID_t id = idIndex >= 0?  idList[idIndex].t : RECENT_ALBUM_ID;	// const non const values
	if ((id & typeFlag) == 0)	// not this type
		return 0;

	QString title, desc, sImageDir, sImagePath, sThumbnailDir, sThumbnailPath;

	QString sOneDirUp, sAlbumDir;
	if (typeFlag == ALBUM_ID_FLAG && !album.exists)
		return -1;

	if (album.ID == ROOT_ALBUM_ID )		// root album
	{
		sAlbumDir = config.dsAlbumDir.ToString();	// root album: all other albums are inside 'albums'
		sOneDirUp = "";								// and the img directory is here
	}
	else
	{
		sAlbumDir = "";			   // non root album: other albums are here
		sOneDirUp = "../";		   // and img directory here
	}

	sImageDir = (QDir::isAbsolutePath(config.dsImageDir.ToString()) ? "" : sOneDirUp) + config.dsImageDir.ToString();
	sThumbnailDir = (QDir::isAbsolutePath(config.dsThumbDir.ToString()) ? "" : sOneDirUp) + config.dsThumbDir.ToString();

	Image *pImage = nullptr;
	ID_t thumb = 0;
	if (typeFlag & ALBUM_ID_FLAG)
	{
		thumb = idIndex >= 0 ? ThumbnailID(_albumMap[id], _albumMap) : _latestImages.list[QRandomGenerator::global()->generate() % (_latestImages.list.size()) ].t;
		if (thumb)
		{
			if (_imageMap.contains(thumb))
				pImage = &_imageMap[thumb];
			else	  // the thumbnail image is not in data base, but 
			{		  // it can be present on the disk so set paths from it
				sImagePath = sImageDir + QString("%1.jpg").arg(thumb & ID_MASK);
				sThumbnailPath = sThumbnailDir + QString("%1.jpg").arg(thumb & ID_MASK);
			}
		}
		else
			pImage = &_imageMap[0];
	}
	else
	{
		pImage = &_imageMap[id];
		if (!pImage->exists)	// then exclude it
		{
			album.items[idIndex] = album.items[idIndex] | EXCLUDED_FLAG;
			return -2;
		}
	}

	if (pImage)
	{
		if (pImage->tsize.width() <= 0)
			pImage->SetThumbSize();
	}

	_ofs << "   <div class=\"img-container\">\n"
		"     <div";
	if (pImage)
		_ofs << " id=\"" << (pImage->ID & ID_MASK) << "\" w=" << pImage->tsize.width() << " h=" << pImage->tsize.height();
	_ofs << ">";

	if (typeFlag & ALBUM_ID_FLAG)
	{
		if(idIndex < 0)
		{ 
			title = Languages::latestTitle[_actLanguage];
			desc = Languages::latestDesc[_actLanguage];
		}
		else
		{
			title = DecodeLF(_textMap[_albumMap[id].titleID][_actLanguage], 1);
			desc = DecodeLF(_textMap[_albumMap[id].descID][_actLanguage], 1);
		}

		if (sImagePath.isEmpty())		// otherwise name for image and thumbnail already set
		{
			sImagePath = (pImage->ID ? sImageDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName() : pImage->name);
			sThumbnailPath = (pImage->ID ? sThumbnailDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName() : pImage->name);
		}
	}
	_ofs << "\n       <a class=\"thumb\" href=\"";

	if (typeFlag & ALBUM_ID_FLAG)
	{
		_ofs << sAlbumDir << _albumMap[id].NameFromID(id, _actLanguage, false) << "\">";	// non root albums are in the same sub directory
	}
	else
	{
		title = DecodeLF(_textMap[pImage->titleID][_actLanguage], 1);
		desc = DecodeLF(_textMap[(pImage->descID)][_actLanguage], 1);
		sImagePath = sImageDir + ( (album.ImageCount() > 0 ? pImage->LinkName() : QString()) );
		sThumbnailPath = sThumbnailDir + ((album.ImageCount() > 0 ? pImage->LinkName() : QString()));
		_ofs << "javascript:ShowImage('" +sImagePath+"', '" + title + "')\">";		// image in the image directory
	}

	// the first 3 images will always be loaded immediately
	_ofs << (idIndex > 2 ? "<img data-src=\"" : "<img src=\"") + sThumbnailPath + "\" alt=\"" + title + "\"";

	if (idIndex < 0)	   // latest uploads
		_ofs << " id=\"latest\"";
	
	if (pImage)
	{
		if (pImage->tsize.width() >= 700)		// too wide thumbnail image
			_ofs << " style=\"max-height:calc(99vw / " << (int)(pImage->Aspect()) << ")\"";
//		else
//			_ofs << " style=\"width: " << pImage->tsize.width() << "; height: " << pImage->tsize.height() << ";\"";
	}
	_ofs << "></a>\n"
			"     </div>\n";									   // end of div thumb
	
    //  -------------------------- description
	if(!desc.isEmpty() )
		_ofs << "     <div class=\"desc\">\n"
				"       <p lang=\"" << Languages::language[_actLanguage] << "\">"
			 << desc << "</p>\n"
				"     </div>\n";
	//  -------------------------- end of description

	//  -------------------------- links with  album/image title
	QString qsLoc;		// empty for non root albums/images
	if (typeFlag == ALBUM_ID_FLAG)
		qsLoc = "javascript:LoadAlbum('" + 
					sAlbumDir + _albumMap[id].NameFromID(id, _actLanguage, false);
	else
		qsLoc = sImagePath.isEmpty() ? "#" : "javascript:ShowImage('" + sImagePath + "', '" + title;
	qsLoc += +"')\">";

	_ofs << "     <div class=\"links\">\n"
			"        <div class=\"title\" onclick=\""
		 << qsLoc;		   // closes <div>

	if (pImage && config.bDebugging)
		title += QString(" <br>%1<br>%2").arg(pImage->name).arg(pImage->ID);
	_ofs << (title.isEmpty() ? "&nbsp;" : title)	// was "---"
		<< "</div>\n"
		   "     </div>\n";	// for "title" "links"
	
	_ofs << "   </div>\n";		// "img-container"

	return 0;
}

/*============================================================================
* TASK:		writes a video section
* EXPECTS:	album		- actual album whose content (images and sub-albums) is written
*			i			- index in ID list for videos
* GLOBALS: config, Languages
* RETURNS:	0: OK, -1: error
* REMARKS:  - image and asub-album sections are written in _WritegalleryContainer()
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteVideoContainer(Album& album, int i)
{
	if (!album.VideoCount())
		return -1;
	ID_t id = album.items[i];
	if (!(id & VIDEO_ID_FLAG))
		return 0;

	QString title, desc, sVideoDir, sVideoPath, sThumbnailDir, sThumbnailPath;

	QString sOneDirUp, sAlbumDir;
	if (album.ID == (1|ID_MASK))
	{
		sAlbumDir = config.dsAlbumDir.ToString();	// root album: all other albums are inside 'albums'
		sOneDirUp = "";					// and the img directory is here
	}
	else
	{
		sAlbumDir = "";			   // non root album: other albums are here
		sOneDirUp = "../";		   // and img directory here
	}

	sVideoDir = (QDir::isAbsolutePath(config.dsVideoDir.ToString()) ? "" : sOneDirUp) + config.dsVideoDir.ToString();
	sThumbnailDir = (QDir::isAbsolutePath(config.dsThumbDir.ToString()) ? "" : sOneDirUp) + config.dsThumbDir.ToString();
	Video* pVideo = &_videoMap[id];	// must exist
	sVideoPath = sVideoDir + ((album.VideoCount() > 0 ? pVideo->LinkName() : QString()));
	QString sVideoType = sVideoPath.right(3).toLower();
	_ofs << "		<div class=\"img-container\">\n"		//#1
			"			<div";								//#2
	if (pVideo)
		_ofs << " id=\"V" << pVideo->ID << "\" w=" << config.thumbWidth << " h=" << config.thumbHeight;
	_ofs << ">\n"
			"				<video width=\"" << config.thumbWidth << "\" height=\"" << (int)config.thumbWidth*1080.0/1920.0
		 << "\" onmouseover = \"this.controls=true\" onmouseout=\"this.controls=false\"";
	if (!config.bCanDownload)
		_ofs << "controlsset=\"nodownload\"";
	_ofs << ">\n"
		    "					<source src=\"" << sVideoPath << "\" type = \"video/" << sVideoType << "\">\n"
		 << "				</video>\n";

	title = DecodeLF(_textMap[pVideo->titleID][_actLanguage], 1);
	desc = DecodeLF(_textMap[(pVideo->descID)][_actLanguage], 1);
	_ofs << "					<div class=\"title\">\n";	//#3	// video in the video directory
	if (pVideo && config.bDebugging)
		title += QString(" <br>%1<br>%2").arg(pVideo->name).arg(pVideo->ID);
	_ofs << (title.isEmpty() ? "&nbsp;" : title)	// was "---"
		<< "</div>\n"					//#3 // "title"		//#2
		    "				</div>\n";		// "links"		//#1

	_ofs << "		</div>\n";		// "img-container"		//#1
	return 0;
}

/*============================================================================
* TASK: Creates one html file for a given album
* EXPECTS:	album ,
*			Language - index of actual language
*			uplink - uplink : one level up
*			processedCount - output parameter
* GLOBALS:	'config'
*				uplink - used as 'uplink' with the uplink button (unless top level)
*				homeLink - The 'home' button links to this page
*
* RETURNS: 0: OK, 16: error writing file
* REMARKS: - root albums (ID == 1+ALBUM_ID_FLAG) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreateOneHtmlAlbum(QFile &f, Album & album, int language, QString uplink, int & processedCount)
{
	if (!f.open(QIODevice::WriteOnly))
		return 16;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_remDsp.Update(processedCount);
	emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
	emit SignalProgressPos(++processedCount, _albumMap.size() * Languages::Count());

	_ofs << _PageHeadToString(album.ID)	// for root album set random thumbnail to most recent gallery
		 << " <body onload = \"falconGLoad(" + QString(album.ID == ROOT_ALBUM_ID ? "1":"0") + ")\" onbeforeunload=\"BeforeUnload()\"";
	if (config.bRightClickProtected)
		_ofs << " oncontextmenu=\"return false;\"";
	_ofs << ">\n"
			"  <div class=\"area\">\n";

	_OutputNav(album, uplink);	/* sticky menu line*/
	_WriteHeaderSection(album);

	_ofs << "<!-- Main section -->\n"
		"   <div class=\"main\" id=\"main\">\n";

	if (album.ID == ROOT_ALBUM_ID && config.bGenerateLatestUploads)	// root album and last uploaded?
	{
		_ofs << "<br><br><!-- section for latest uploads -->\n"
			<< "    <section>\n";
		_WriteGalleryContainer(album, ALBUM_ID_FLAG, -1);		// -1: latest uploads> the thumbnail is set by javascript 'GetRandomLastImage()'

		_ofs << "    </section>\n<!-- end section for latest uploads -->\n";
	}
	// get number of images and sub-albums
	if (album.ImageCount())
	{
		_ofs << "<!--the images in this sub gallery-->\n"
			<< "    <div id=\"images\" class=\"fgsection\">" << Languages::images[_actLanguage] << "</div>\n"
			<< "    <section>\n";
		// first the images
		for (int i = 0; _processing && i < album.items.size(); ++i)
			if(album.items[i] && IMAGE_ID_FLAG)
				_WriteGalleryContainer(album, IMAGE_ID_FLAG, i);
		_ofs << "    </section>\n<!-- end section Images -->\n";
	}

	if (album.VideoCount() > 0)
	{
		_ofs << "<!--start section videos -->\n"
			 << "    <div id=\"videos\" class=\"fgsection\">" << Languages::videos[_actLanguage] << "</div>\n"
			    "    <section>\n";

		for (int i = 0; _processing && i < album.items.size(); ++i)
			if (album.items[i] && VIDEO_ID_FLAG)
				_WriteVideoContainer(album, i);
		_ofs << "    </section>\n<!-- end section Videos -->\n";
	}

	if (album.SubAlbumCount() > 0)
	{
		_ofs << "<!--start section albums -->\n"
			<<	"    <div id=\"albums\" class=\"fgsection\">" << Languages::albums[_actLanguage] << "</div>\n"
				"    <section>\n";

		for (int i = 0; _processing && i < album.items.size(); ++i)
			if (album.items[i] && ALBUM_ID_FLAG)
				_WriteGalleryContainer(album, ALBUM_ID_FLAG, i);
		_ofs << "    </section>\n<!-- end section Albums -->\n";
	}

	if (_processing)		// else leave the page unfinished
	{
		// main section is finished
		_ofs << "\n   </main> <!-- end of main section -->\n";
		// footer section
		_WriteFooterSection(album);
		// end html
		_ofs <<	"  </div>\n"		// end of div 'area'
				" </body>\n"
				"</html>\n";
		_ofs.flush();
		f.close();
	}
	return 0;
}

/*============================================================================
* TASK: Creates one html file for a given album
* EXPECTS:	album , 
*			Language - index of actual language
*			uplink - uplink : one level up
*			processedCount - output parameter
* GLOBALS:	'config'
*				uplink - used as 'uplink' with the uplink button (unless top level) 
*				homeLink - The 'home' button links to this page
*
* RETURNS: 0: OK, 16: error writing file
* REMARKS: - root albums (ID == 1+ALBUM_ID_FLAG) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root: when it not changed and need not re-create all albums, just signals
*			 it processed the album
*			- uses _CreatePageInner (co-function) to recursively create sub albums
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreatePage(Album &album, int language, QString uplink, int &processedCount)
{
	_actLanguage = language;

	QString s;
	if ( album.ID == ROOT_ALBUM_ID)		// top level: use name from config 
		s = config.dsGallery.ToString() + RootNameFromBase(config.sMainPage.ToString(), language);
	else		 // all non root albums are either in the same directory or in separate language directories inside it
		s = config.LocalAlbumDirectory().ToString() + album.NameFromID(language);

	QFile f(s);
	if (!_mustRecreateAllAlbums && f.exists() && !album.changed /*&& !_structChanged*/)
	{
		_remDsp.Update(processedCount);
		emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
		emit SignalProgressPos(++processedCount, _albumMap.size() * Languages::Count());
	}
	else
		_CreateOneHtmlAlbum(f, album, language, uplink, processedCount);

	if (_processing) 		// create sub albums
	{
		if (album.SubAlbumCount() >0)
		{
			uplink = album.NameFromID(language);
			if (album.ID == ROOT_ALBUM_ID)
				uplink = QString("../") + uplink;

			for (int i = 0; _processing && i < album.items.size(); ++i)
			{
				if((album.items[i] & ALBUM_ID_FLAG) && !(album.items[i] & EXCLUDED_FLAG) )
					_CreatePage(_albumMap[album.items[i]], language, uplink, processedCount);
			}
		}
	}
	return 0;
}

/*============================================================================
* TASK: Create a very basic home page
* EXPECTS:	config fields are set, albums are read in (_albumMap, etc)
* GLOBALS:	'config
* RETURNS: 0: OK, -1: error writing file
* REMARKS: file is named <main gallery page name> except when it is the same
*			as the config.sUplink, in which case it has an anderscore at the front
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreateHomePage()
{
	if (Languages::Count() == 1)
		return 0;

	QString name = config.sMainPage.ToString(),	 // e.g. index.html
		s, path, fn, ext;
	SeparateFileNamePath(name, path, fn, &ext);

	if (name == config.sUplink.ToString())		// uplink  default name is empty
		s = QStringLiteral("_") + name;
	else
		s = name;
	s = (config.dsGallery + config.dsGRoot).ToString() + s;

	QFile f(s);
	if (!f.open(QIODevice::WriteOnly))
		return -1;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << _PageHeadToString(1)
		<< "<body>\n";

	_OutputNav(_albumMap[0], QString());	/* sticky menu line*/

	_ofs << "<!--Header section-->\n"
		<< "<header>\n<span class=\"falconG\">" << config.sGalleryTitle << "</span>&nbsp; &nbsp;";
	_WriteFacebookLink(_albumMap[1].LinkName(_actLanguage, true), 1);	// when required
	_ofs << "<p class=\"menu-line\">\n";
	// languages links
	for (int i = 0; i < Languages::Count(); ++i)
	{
		_ofs << QString("<a href=\"%1\">%3</a>").arg(Languages::FileNameForLanguage(fn, i)).arg(Languages::names[i]);
		// menu buttons
		if (config.bMenuToAbout)
		{
			_ofs << "<a href=\"";
			_ofs << Languages::FileNameForLanguage(config.sAbout,i) << "\">" << Languages::toAboutPage[i];
			_ofs << "</a>\n";
		}
		if (config.bMenuToContact)
			_ofs << "<a href=\"mailto:" << config.sMailTo << "\">" << Languages::toContact[i] << "</a>\n";
		_ofs << "<br>\n";
	}
	_ofs << "</header>\n"
			"<!-- Main section -->\n"
			"<main id=\"main\">\n"
			" <!** ... TODO ... -->\n"
			"</main>"
			"<footer>\n"
			" <!** ... TODO ... -->\n"
			"</footer>"
		    "</body>\n</html>\n";
	_ofs.flush();
	f.close();

	return 0;
}

/*============================================================================
* TASK: Creates all html files
* EXPECTS:	config fields are set, albums are read in (_albumMap, etc)
* GLOBALS:	'config.sUplink'  - if set root page's up link points into this/these
*			'config.sMainPage' - base or full name of root page(s) inside dsGRoot'
*							   Menu 'home' brings here
* RETURNS: 0: OK, 16: error writing file
* REMARKS: each file is named <config.baseName><album.ID><Language::countryCode[]>,html
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoPages()
{
	if (!_processing)
		return 0;

	emit SignalToSetProgressParams(0, _albumMap.size() * Languages::Count(), 0, 3); // phase = 3

	_remDsp.Init(_albumMap.size() * Languages::Count());


	QString uplink = config.sUplink.ToString();	 // points to embedding page, relative to html 
											 // directory on server (inside e.g. public_html)
											 // e.g. single language: 
											 //		index.html corresponds to https://your.site.com/index.html 
											 // multiple languages:  as many pages as are languages:
											 // https://your.site.com/index_en.html, https://your.site.com/index_hu.html, etc)
											 // no default: can be empty or name w. or w.o extension: .htm, .html or .php
//	bool multipleLanguages = Languages::Count() > 1;
	QString path, name;				 

	int cnt = 0;
	
	if (Languages::Count() != 1 && config.bSeparateFoldersForLanguages)
	{
		int ask = config.doNotShowTheseDialogs;
		config.doNotShowTheseDialogs.v |= int(dbAskCreateDir);
		config.defaultAnswers[dboAskCreateDir] = QMessageBox::Yes;

		for (int lang = 0; _processing && lang < Languages::Count(); ++lang)
			CreateDir(Languages::abbrev[lang]);

		config.doNotShowTheseDialogs = ask;
	}

	for (int lang = 0; _processing && lang < Languages::Count(); ++lang)
	{
										// home of gallery root	 like 'index' or 'index.html'
		config.homeLink = uplink.isEmpty() ? config.sMainPage.ToString() : config.sUplink.ToString();

		_CreatePage(_albumMap[ROOT_ALBUM_ID], lang, uplink, cnt);	// all albums go into the same directory!
	}
	_CreateHomePage();

	return 0;
}

/*=============================================================
 * TASK:	goes through all images to collect latest uploads
 * EXPECTS:
 * GLOBALS:
 * RETURNS:	count of images found and list of found in array
 * REMARKS:
 *------------------------------------------------------------*/
int AlbumGenerator::_CollectLatestImagesAndVideos(LatestImages& here)
{
	QDate dt = dt.fromJulianDay(_latestDateLimit.toJulianDay() - config.newUploadInterval);

	here.Clear();
	const int MAX_LATEST_COUNT = 100;		// max 100 images from latest uploads
	int cnt = MAX_LATEST_COUNT;

	for (auto a : _imageMap)
		if (a.uploadDate >= dt)
		{
			here.list.push_back(a.ID);
			if (a.descID)
				++here.cntDescs;
			if (a.titleID)
				++here.cntTitles;
			if (!--cnt)
				break;
		}

	if (cnt)
	{
		for (auto a : _videoMap)
			if (a.uploadDate >= dt)
			{
				here.list.push_back(a.ID);
				if (a.descID)
					++here.cntDescs;
				if (a.titleID)
					++here.cntTitles;
				if (!--cnt)
					break;
			}
	}

	return here.size();
}

/*=============================================================
 * TASK:	create latest_XX.js javascript data file for the 
 *				latest uploads
 * EXPECTS:	
 * GLOBALS:	config.nLatestCount, _albumMap, _textMap, _imageMap, 
 *			_videoMap, _actLanguage
 * RETURNS: 0: OK, any other error code
 * REMARKS:	- only the data array is put into these js files, one
 *				for each language. These are included into the
 *				correct HTML files before the 'latest_common.js'
 *				file
 *			- generated into javascript directory
 *			- real generation is set in javascript file latest.js
 *------------------------------------------------------------*/
int AlbumGenerator::_DoLatestJs()
{
	// create array of latest uploads
	int n = _CollectLatestImagesAndVideos(_latestImages);	// limited # can be selected
	if (!n)
		return 0;

	// create one file for each languages named 'latestList_XX.js'

	for (int lang = 0; lang < Languages::Count(); ++lang)
	{
		QString qsName = config.GalleryRoot().ToString() + "js/latestList" + Languages::abbrev[lang] + ".js";

		QFile f(qsName);
		if (!f.open(QIODevice::WriteOnly))
			return 16;

		QTextStream ofjs(&f);
		ofjs.setCodec("UTF-8");

		ofjs << "// Copyright  2021 András Sólyom\n"
			"// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com).\n"
			"\n"
			"// date of latest upload: " << _latestDateLimit.toString() << "\n"
			"// period: "<< config.newUploadInterval << " days, count: "<< n << ",max count : "<< config.nLatestCount <<"\n\n"
			"const lang='" << (Languages::language[lang]) << "'\n"
			"const imagePath='"<< config.dsImageDir.ToString() <<"'\n"
			"const thumbsPath='"<< config.dsThumbDir.ToString() <<"'\n"
			"const ids = [\n";

		for (auto latest : _latestImages.list)
		{
			Image* pim;
			Video* pvid;
			ID_t idt = (ID_t)latest;
			if (idt & IMAGE_ID_FLAG)
			{
				pim = &_imageMap[idt];
				pim->SetThumbSize();
				ofjs << "{ id:" << (idt & ID_MASK) << ",w:" << pim->tsize.width() << ",h:" << pim->tsize.height();
				if (pim->titleID)
					ofjs << ",t:\"" << DecodeLF(_textMap[pim->titleID][lang], 2, true) << "\"";
				if (pim->descID)
					ofjs << ",d:\"" << DecodeLF(_textMap[pim->descID][lang], 2, true) << "\"";
				ofjs << "},\n";
			}
			else if (idt & VIDEO_ID_FLAG)
			{
				pvid = &_videoMap[idt];
				ofjs << "{ id:" << (idt & ID_MASK) << ",w:" << config.thumbWidth << ",h:" << config.thumbHeight;
				if (pvid->titleID)
					ofjs << ",t:\"" << DecodeLF(_textMap[pvid->titleID][lang], 2, true) << "\"";
				if (pvid->descID)
					ofjs << ",d:\"" << DecodeLF(_textMap[pvid->descID][lang], 2, true) << "\"";
				ofjs << "},\n";
			}
		}
		ofjs << "];\n"
			<< "var cnt = " << (config.nLatestCount <= n ? config.nLatestCount.v : n) << ";\n"; // # of random images to show
		f.close();
	}
/* ----------------
			content of 'latest.js':

// has array 'selectedList[]', 'cnt' and'imagePath' set
let selected=[]
//--------------------------------------------------------------------
function select() {
	// DEBUG
	// console.log(window.location.pathname)

	selected.length = 0;
	let s = []
	if(cnt > ids.count)
		cnt = ids.count;
	// DEBUG
	//console.log("cnt:"+cnt);

	while(cnt) {
		let i = Math.floor(Math.random()*cnt); // index:0..cnt
	// DEBUG
	// console.log("i:"+i);
		if(!s.includes(i))
		{
			s.push(i);
			selected.push(ids[i]);
			--cnt;
		}
	}
	const section = document.getElementsByTagName('section')[0];

	selected.forEach(item => {
		const divImgContainer = document.createElement('div');
		divImgContainer.classList.add('img-container');
		// DEBUG
		// console.log('item:'+item)
			const divId = document.createElement('div');
			divId.id=item.id;
			divId.w= item.w;
			divId.h= item.h;

				const a = document.createElement('a');
				a.classList.add('thumb');
				a.href=`javascript:ShowImage('${imagePath}${item.id}.jpg','')`;
		// DEBUG
		// console.log('href:'+a.href)

				const img = document.createElement('img');
				img.src = `${imagePath}${item.id}.jpg`;
				img.alt= `Image ${item.id}.jpg`;
				img.classList.add('galleryImg');

			const divDesc = document.createElement('div');
			divDesc.classList.add('desc');

				const pw = document.createElement('p');
				pw.lang=lang;
				if(typeof item.d === 'undefined')
					pw.innerHTML ="&nbsp;";
				else
					pw.innerHTML = item.d;

			const divLinks = document.createElement('div');
			divLinks.classList.add('links');

				const divTitle = document.createElement('div');
				divTitle.classList.add('title');
				divTitle.onclick = `javascript:ShowImage(''${imagePath}${item.id}.jpg','')`;
				if(typeof item.t !== 'undefined')
					divTitle.innerHTML ="&nbsp;";
				else
					divTitle.innerHTML = item.t;

			divLinks.appendChild(divTitle);
			divDesc.appendChild(pw);
			a.appendChild(img);
			divId.appendChild(a);
		divImgContainer.appendChild(divId);
		divImgContainer.appendChild(divDesc);
		divImgContainer.appendChild(divLinks);

		section.appendChild(divImgContainer);

		// console.log(img.src + ' - processed');
	})
}
 -- end of latest.js */
	return 0;
}

/*=============================================================
 * TASK:	create one file for the latest uploads
 * EXPECTS:	"baseName" - path name (in album folder)	 w.o.
 *					language postfix & extension!
 * GLOBALS:
 * RETURNS: 0 for success else error code
 * REMARKS: uses a dummy album with ID == 2
 *------------------------------------------------------------*/
int AlbumGenerator::_DoLatestHelper(QString baseName, int lang)
{
	QString postFix, fileName;
	int saveLang = _actLanguage;
	_actLanguage = lang;

	if(Languages::Count() > 1)
		postFix = Languages::abbrev[lang];
	fileName = baseName + postFix + ".html";

	Album albumLatest;
	albumLatest.ID = RECENT_ALBUM_ID;

	QFile f(fileName);
	if (!f.open(QIODevice::WriteOnly))
		return 16;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << _PageHeadToString(RECENT_ALBUM_ID)
		 << " <body onload = \"select()\"";
	if (config.bRightClickProtected)
		_ofs << " oncontextmenu=\"return false;\"";
	_ofs << ">\n"
		"  <div class=\"area\">\n";

		/* sticky menu line  */
	_OutputNav(albumLatest, "../index" + Languages::abbrev[_actLanguage]+".html");

	_WriteFacebookLink(fileName, 1);	// only when required

	_ofs << "</nav>\n";
	// - end of navigation menus -----

	_WriteHeaderSection(albumLatest);

	_ofs << "<!-- Main section -->\n"
			"<br><br>"
			"   <div class=\"main\" id=\"main\">\n"
			"		<section>\n"
            "		</section>\n"
			"	</div>\n"
			"</div>\n"
			"<script language=\"javascript\" src=\"../js/latestList"<< (Languages::Count() > 1 ? Languages::abbrev[_actLanguage] : "") << ".js\"></script>\n"
			"<script language=\"javascript\" src=\"../js/latest.js\"></script>\n"
			"</body>\n"
			"</html>\n";
	_ofs.flush();
	_actLanguage = saveLang;
	return 0;
}

/*============================================================================
* TASK:	Creates latest uploads	- latest.html
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 32: error writing file
* REMARKS: album generated into root directory on server
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoLatest()
{
	if (!config.bGenerateLatestUploads)
		return 0;

	QString qsBase = config.LocalAlbumDirectory().ToString()+"latest";
	int res = _DoLatestJs();
	if (res || !_latestImages.list.size())	// only generate if one exists
		return res;

	for (int i = 0; i < Languages::Count() && !res; ++i)
			res |= _DoLatestHelper(qsBase, i);

	return res;
}

/*============================================================================
* TASK: Creates .htaccess file
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 2: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoHtAccess()
{
	QFile htx(QString(__RootDir.ToString()) + ".htaccess");
	if (!config.bCanDownload && !config.bForceSSL)
	{
		htx.remove();
		return 0;
	}

	if(! htx.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return 2;
	_ofs.setDevice(&htx);
	_ofs << _HtaccessToString();
	htx.close();
	return 0;
}

/*============================================================================
* TASK:		Creates album in directory 'config.dsGallery
*			- constants.php		-  from all parameters		(Error code 1)
*			- falconG.css		-  from all parameters		(error code 4)
*			- index.php			- 							(error code 8)
*									contains album index -> album name table
*			- all html files	-							(error code 16)
*			- latest uploads	- latest.html				(error code 32)
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS:	0: OK, other error code
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::Write()
{
	if (!_CreateDirectories()) // from 'config'
		return 64;

	_ProcessImages();	// copy from from source into image directory
						// determine image dimensions and latest upload date
	_ProcessVideos();
	if(_processing)
		if (_structChanged)		// do not save after read of structure
			WriteDirStruct();   // all album and image data is read in

	int i = 0;					// returns:
	if (_processing)
	{
		i = _DoCopyRes();			// 0 | 1	modifes 'up-link.png' !
		i |= _DoCopyJs();
		i |= _DoLatest();			// 0 | 16 | 32 latest HTML + JS files must come before _DoPages
	}
	if (_processing)
		i |= _DoHtAccess();			// 0 | 2
	if (_processing)
		i |= _SaveFalconGCss();			// 0 | 8
	if (_processing)
		i |= _DoPages();			// 0 | 16

	emit SignalToSetProgressParams(0, 100, 0, 0);		// reset phase to 0
	_structChanged = 0;
	_processing = false;

	return i;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::SaveStyleSheets()
{
	__SetBaseDirs();
	return albumgen._SaveFalconGCss();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteStructReady(QString s, QString sStructPath, QString sStructTmp)
{
	if (!s.isEmpty())		// then error message
	{
		QMessageBox(QMessageBox::Warning, "falconG - Generate", s, QMessageBox::Close, frmMain).exec();
		return;
	}

	_structWritten = true;
	s = BackupAndRename(sStructPath, sStructTmp, _keepPreviousBackup);
	if(!s.isEmpty())
		QMessageBox(QMessageBox::Warning, "falconG - Generate", s, QMessageBox::Close, frmMain).exec();
}



/*==========================================================================
* TASK:
* EXPECTS:
* RETURNS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_RemainingDisplay::Init(int maximum)
{
	max = maximum;
	t0 = time(0);
	tprev = 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_RemainingDisplay::Update(int cnt)
{
	tAct = time(nullptr) - t0;
	if (tAct != tprev)
	{
		tTot = (time_t)((double)tAct / (double)cnt * max);
		tprev = tAct;
	}
}

/*============================================================================
  * TASK:	 Returns the path name relative to dsSrc
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString IABase::ShortPathName()
{
	QString s = path + name;
	if (s.left(config.dsSrc.ToString().length()) == config.dsSrc.ToString())
		s = s.mid(config.dsSrc.ToString().length());
	return s;
}

Video& VideoMap::Find(ID_t id, bool useBase)
{
	ID_t mask = useBase ? BASE_ID_MASK : 0xFFFFFFFFFFFFFFFFul;
	Video vid; vid.ID = id;
	for (auto i = begin(); i != end(); ++i)
		if ((i.key() & mask) == (vid.ID & mask))
			return i.value();
	return invalid;
}

Video& VideoMap::Find(QString FullName)
{
	Video vid;
	SeparateFileNamePath(FullName, vid.path, vid.name);

	Video::searchBy = Video::byFullName;
	for (auto i = begin(); i != end(); ++i)
		if (i.value() == vid)
			return i.value();

	return invalid;
}

/*============================================================================
* TASK: Add a video to the list of all videos
* EXPECTS:  path - full or config.dsSrc relative path name of the video
*			added- output parameter: was this a new video?
* GLOBALS:
* RETURNS: id of video in map with the VIDEO_ID_FLAG ORed
* REMARKS: - id stored in video map has no VIDEO_ID_FLAG set
*		   - when collisions occur adds a unique value above 0xFFFFFFFF to the ID
*		   - even non-existing videos are added to map
*		   - if an video with the same name, but with different path is found
*				and the one in the data base does not exist, replace the one
*				in the data base with the new video even when it also does not exist
*--------------------------------------------------------------------------*/
ID_t VideoMap::Add(QString path, bool& added)
{
		Video vid;

		SeparateFileNamePath(path, vid.path, vid.name);

		if (vid.path.isEmpty())
			vid.path = AlbumGenerator::lastUsedItemPath;

		if (!QDir::isAbsolutePath(path))     // common part of path is not stored
			path = config.dsSrc.ToString() + vid.path + vid.name;	 // but we need it

		added = false;
		ID_t id = CalcCrc(vid.name, false);	// just by name. CRC can but id can never be 0 

		QFileInfo fi(path);
		vid.exists = fi.exists();

		Video* found = &Find(id); // check if a file with this same base id is already in data base? 

		if (found->Valid())		  // yes an video with the same base ID as this video was found
		{						  // the ID can still be different from the id of the found video though
			do					  // loop thorugh all videos with the same base ID			
			{
				if (found->name == vid.name)	// if name is the same too then same video	
				{								// even when they are in different directories!
					if (found->exists)
						if (!vid.exists || (vid.exists && (found->uploadDate >= fi.lastModified().date() || found->fileSize >= fi.size())))
							return found->ID;	  // do not change path as video already existed

					id = found->ID;		// else found video did not exist,
					break;				// replace it with this video but with the same ID as the old one 
				}
				else						// same ID, different name - new video
					id += ID_INCREMENT;
				// repeat until a matching name with the new ID is found or no more videos
			} while ((found = &Find(id, false))->Valid()); // full by ID
		}
		// now I have a new video to add
		// new video: 
		if (vid.exists)
		{
			vid.fileSize = fi.size();
			vid.uploadDate = fi.lastModified().date();
		}

		added = true;
		vid.ID = id | VIDEO_ID_FLAG;
		insert(id,vid);
		return id;
}

Video& VideoMap::Item(int index)
{
	if (index < 0 || index > size())
		return invalid;
	iterator it = begin();
	it += index;
	return *it;
}

int Video::operator<(const Video& i)
{
	switch (searchBy)
	{
		case byID: return ID < i.ID;
		case byName: return name < i.name;
		default: return path + name < i.path + i.name;
	}
}

bool Video::operator==(const Video& i)
{
	switch (searchBy)
	{
		case byID: return ID == i.ID;
		case byBaseID: return (ID & BASE_ID_MASK) == (i.ID & BASE_ID_MASK) ? (name.toLower() == i.name.toLower() ? true : false) : false;
		case byName: return name == i.name;
		default: return path + name == i.path + i.name;
	}
}
