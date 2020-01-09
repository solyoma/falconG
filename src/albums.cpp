#include <QApplication>
#include <QtCore>
#include <QString>
#include <QTextCodec>
#include <time.h>
#include <atomic>
#include <chrono>

#include "languages.h"
#include "albums.h"
#include "falcong.h"

#if QT_VERSION < 0x051000
    #define created created
#endif

QString ImageMap::lastUsedImagePath;

//******************************************************
AlbumGenerator	albumgen;		// global
static QString	sStructPath,	// path of .struct file
				sStructTmp;		// these are accessed from 
								// two threads, but no conflict exists

//******************************************************
/////////////////////////
struct BadStruct 
{ 
	QString msg;
	int cnt;  
	BadStruct(int lineNo, QString msg) : msg(msg),cnt(lineNo) {}
};		// simple exception class
////////////////////////////

const QString TITLE_TAG       = "Title-";	// used in 'struct' files
const QString DESCRIPTION_TAG = "Descr-";	// e.g. [Tytle-hu:<text>]
const QString THUMBNAIL_TAG   = "Icon";	// "album icon"
//const QString SAVED_IMAGE_DESCRIPTIONS = "images.desc";
//const QString TEMP_SAVED_IMAGE_DESCRIPTIONS = "images.tmp";

// static members
LanguageTexts TextMap::invalid;
Image ImageMap::invalid;
Album AlbumMap::invalid;

// used directories put here and not into class 'AlbumGenerator'
// because of include order
// defaults of these are not set, str always valid
_CDirStr	__DestDir,		// generate gallery into _RootDir inside this
			__RootDir,		// .htaccess, constants.php, index.php
			__AlbumDir,		// all albums
			__CssDir,		// colors.css, stylefg.css
			__ImageDir,		// all images. image name with date (?)
			__ThumbDir,		// all images. image name with date (?)
			__FontDir,		// some of used fonts
			__ResourceDir;

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
	__CssDir = __RootDir + config.dsCssDir;	  		// colors.css, stylefg.css
	__ImageDir = __RootDir + config.dsImageDir;		// contains all images
	__ThumbDir = __RootDir + config.dsThumbDir;		// contains all thumbnails
	__FontDir = __RootDir + config.dsFontDir;		// some of used fonts
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
static ID_t _ThumbnailID(Album & album, AlbumMap& albums)
{
	if (album.thumbnail == 0) // then the first image  or sub-album
	{						  // will be set as its thumbnail
		if (album.images.isEmpty() && album.albums.isEmpty())
			return 0;
		if (album.images.isEmpty()) // then get thumbnail for first sub-album
			return _ThumbnailID(albums[album.albums[0]], albums);
		else
			return album.images[0];
	}
	return album.thumbnail;
}


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
static void WriteStructLanguageTexts(QTextStream &ofs, TextMap &texts, QString what, ID_t id, QString indent)
{
	bool b = what[0] == 'T' ? config.bAddTitlesToAll : config.bAddDescriptionsToAll;

	if (id == 0 && !b)		// no text
		return;
	LanguageTexts &text = texts[id];

	for (int i = 0; i < Languages::Count(); ++i)
	{
		ofs << indent << "[" << what << Languages::abbrev[i] << ":";
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
			<< width << "x" << height << ","
			<< owidth << "x" << oheight << ","
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
		img.path = ImageMap::lastUsedImagePath;

	if (!QDir::isAbsolutePath(path))     // common part of path is not stored
		path = config.dsSrc.ToString() + img.path + img.name;	 // but we need it

	added = false;
	ID_t id = CalcCrc(img.name, false);	// just by name. CRC can but id can never be 0 

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
	(*this)[id] = img;
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
* TASK: 
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/

/*============================================================================
* TASK: get number of non-excluded images in album and remove excluded ones
*		from local image list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::ImageCount()
{
	if (excluded.isEmpty())
		return images.size();

	for (int i = images.size() -1; i >= 0; --i)	// from end to start
		if (excluded.indexOf(images[i]) >= 0)
			images.removeAt(i);
	return images.size();
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
	if (excluded.isEmpty())
		return albums.size();

	for (int i = albums.size()-1; i >= 0; --i)
		if (excluded.indexOf(albums[i]) >= 0)
			albums.removeAt(i);

	return albums.size();
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
	if (titleCount >= 0)
		return titleCount;

	titleCount = 0;
	for (auto a : albums)
		if (albumgen.Albums()[a].titleID)
			++titleCount;
	for (auto a : images)
		if (albumgen.Images()[a].titleID)
			++titleCount;
	return titleCount;
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
	if (descCount >= 0)
		return descCount;

	for (auto a : albums)
		if (albumgen.Albums()[a].descID)
			++descCount;
	for (auto a : images)
		if (albumgen.Images()[a].descID)
			++descCount;
	return descCount;
}

/*============================================================================
* TASK: Create full link name of album
* EXPECTS: language -  index of language
*			http - true: http or https prefix plus add albumdir to the 
*			front wgen needed, false: just name
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
	return QString("%1%2").arg(config.sBaseName.ToString()).arg(ID);
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

	if (id == 1)	// root album
	{
		s = config.sMainPage.ToString();
		s = Languages::FileNameForLanguage(s, language);
	}
	else
	{
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

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString Album::SiteLink(int language)
{
	QString s;
	if (config.sServerAddress.ToString().toLower().left(4) == "http")
		s = config.sServerAddress + "/";
	else
		s = (config.bForceSSL ? "https://" : "http://") + config.sServerAddress + "/";
	return s;
}

/**************************** AlbumMap *****************************/

/*============================================================================
* TASK:	  Add an album to the album map if it is not already in there
* EXPECTS: path:  IN - either absolute path or relative to source directory (!)
*          added: OUT - et when this is added
* GLOBALS:
* REMARKS:	path may contains a logical name only it need not exist
*			if it exists then ImageMap::lastUsedImagePath is adjusted
*--------------------------------------------------------------------------*/
ID_t AlbumMap::Add(QString path, bool &added)
{
	Album ab;

	Album &found = Find(path);		// already in database?
	added = false;
	if (found.Valid())
		return found.ID;	// same base ID, same name then same album

	SeparateFileNamePath(path, ab.path, ab.name);
	ab.ID = GetUniqueID(*this, path, false);   // using full path name only
	if(!QDir::isAbsolutePath(path) )
		ab.exists = QFile::exists((config.dsSrc + path).ToString());
	else
		ab.exists = QFile::exists(path);
	if (!ab.exists)	// bit maybe its parent does
	{
		if (!QDir::isAbsolutePath(ab.path))
			ab.exists = QFile::exists((config.dsSrc + ab.path).ToString());
		else
			ab.exists = QFile::exists(ab.path);
	}
	if (ab.exists)
		ImageMap::lastUsedImagePath = ab.path + ab.name + "/";
	ab.exists = true; // no need for real directoy to exist --  QFileInfo::exists(path);
	added = true;		// alwaays add, even when it does not exist
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

/*============================================================================
  * TASK:	 add a new image from structure to image map
  * EXPECTS:  imagePath: path name relative to source directory
  *			  
  * RETURNS:  image added/found. It will not have any title or description as those
  *				will be added after this image is processed
  * GLOBALS:
  * REMARKS:- new images may be added to the structure file by using the path
  *			  name, (absolute or relative to the gallery source) and nothing more. 
  *			  In this case the new image must be processed and added to the 
  *			  data base.
  *			 - sets '_structChanged' to true
 *--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageFromPathInStruct(QString imagePath)
{
	bool added;
	++_structChanged;

	return _imageMap.Add(imagePath, added);	// add new image to global image list or get id of existing
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
* RETURNS:	id of data
* REMARKS:  - 'exists' member of not excluded old oe new images and sub-albums 
*				are set to true when 'fromDisk' is true
*			- adds new images and id's to the corresponding id list of 'ab'
*			- showss progress using frmMain's progress bar
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrAlbum(Album &ab, QFileInfo & fi/*, bool fromDisk*/)
{
	ID_t id = 0;
	
	QString s = fi.filePath();
	if (fi.fileName()[0] == '.')			// hidden file?
		return 0;

	bool added;
	if (fi.isDir())
	{
		QString ds = fi.fileName();
		if (ds[0] == '.' || ds == "res" || ds == "cache" || ds == "thumbs")
			return 0;
		id = _albumMap.Add(s, added);	// add new album to global album list
		if (id && added)
			ab.albums.push_back(id);	// add to ordered sub-album ID list for this album
		id += ALBUM_ID_FLAG;			// signal it was an album that was added
	}
	else if (IsImageFile(s))
	{
		id = _imageMap.Add(s, added);	// add new image to global image list or get id of existing
		if (id && added)
			ab.images.push_back(id);	// add to ordered image ID list for this album

	}
	// progress bar
	emit SignalProgressPos(_albumMap.size(), _imageMap.size() );	  // both changes

	_remDsp.Update(_albumMap.size());
	emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
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
	const IdList &exList = ab.excluded;
	for (int i = 0; i < ab.excluded.size(); ++i)
	{
		id =  (const_cast<IdList &>(exList)[i]);
		if (id)
		{
			if (_albumMap[id].name == name)
				return true;
		}
		else
			if (_imageMap[id].name == name)
				return true;
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
bool AlbumGenerator::_ReadAlbumFile(Album &ab)
{
	QString path = ab.FullName();  
	if(path[path.length()-1] != '/') 
		path += '/';

	FileReader fr(path + "albumfiles.txt", FileReader::frfTrim);
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
			ab.excluded.push_back(id);	// this is excluded (needed when reading names from disk)
	}
	return true;
}

/*============================================================================
  * TASK: test the sizesof an existing thumbnail against config and see 
  *		  if it must be recreated
  * EXPECTS: thumbPath - full path of thumbnail file
  *			 img - image data from data base
  * RETURNS: 'true' :  
  *				actual size of thumbnail is not the same as the one in data base
  *			'false' : otherwise
  * GLOBALS:
  * REMARKS: - the thumbnail  already a rotated image wjen needed
  *			 - 
 *--------------------------------------------------------------------------*/
bool AlbumGenerator::MustRecreateThumbBasedOnImageDimensions(QString thumbPath, Image & img)
{
	ImageReader reader(thumbPath);
	QSize size = reader.size();		// read thumbnail image dimensions from file
									// because integer arithmetic the calculated w & H may be different
									// from that of the converted thumbnail file
									// assumption: this may only happen to the width and not the height
	return img.rdata.bThumbDifferent = abs(size.width() - img.rdata.tw) > 2 || abs(size.height() - img.rdata.th) > 2;
}

/*==========================================================================
* TASK:	sepatae lines containing text delimited by '@@' ot "{)" delimiters
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
		imagePath = ImageMap::lastUsedImagePath + imagePath;
	else
		ImageMap::lastUsedImagePath = imagePath.left(imagePath.lastIndexOf('/') + 1); // including last '/'
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
bool AlbumGenerator::_ReadCommentFile(Album &ab)
{
	QString path = ab.FullName();
	if (path[path.length() - 1] != '/')
		path += '/';

	FileReader fr(path + "comments.properties", FileReader::frfTrim);
	if (!fr.Ok())
		return false;
	QString	line,		// <ianame>=<texts>
			ianame;
	QStringList sl;
	ID_t id;
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

		if (id)		// then image existed
		{			// set text to image or album description
			sl = _SeparateLanguageTexts(line);
			bool added;
			ID_t tid = _textMap.Add(sl, added);	// same id for all languages
			if (id)		// description to sub-album
				_albumMap[id].descID = tid;
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
bool AlbumGenerator::_ReadMetaFile(Album &ab)
{
	QString path = ab.FullName();
	if (path[path.length() - 1] != '/')
		path += '/';

	FileReader fr(path + "meta.properties", FileReader::frfTrim);
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
void AlbumGenerator::_ReadInfoFile(Album &ab, QString & path, QString name)
{
	QString line;
	FileReader reader(path + ".jalbum/" + name, FileReader::frfNeedUtf8 | FileReader::frfTrim);  // name of .info file

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
bool AlbumGenerator::_ReadInfo(Album & ab)
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
				_ReadInfoFile(ab, path, s);
		}
	}
	return true;
}

/*=================================================================
 * TASK: read a directory tree of images and order them into an album
 *		  hierarchy
 * EXPECTS:		ab - album (already on global album list '_albumMap')
 * GLOBALS: _justChanges
 * REMARKS: - first reads meta.properties and set parameters for this album
 *			- then reads file 'albumfiles.txt' if it exists and add
 *				image/album names from it to album
 *			- then reads all file and sub-directory names from the album directory
 *			  and add not stored ones to lists recursively processing sub albums
 *			- then reads comments.properties and sets them for images and sub-albums
 *			- then call itself recursively for sub-folders
 *----------------------------------------------------------------*/
void AlbumGenerator::_ReadOneLevel(Album &ab)
{
	// TODO justChanges
	_ReadAlbumFile(ab);		// ordering set in file ( including exlusions and real names)
	_ReadMetaFile(ab);		// thumbnail (image or sub-album) ID and description for the actual album

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
			id -= ALBUM_ID_FLAG;
			_albumMap[id].parent = ab.ID;
			if (_albumMap[id].exists)
				_ReadOneLevel(_albumMap[id]);
		}
		emit SignalProgressPos(_albumMap.size(), _imageMap.size());
		if (!_processing)
			return;
	}
	_ReadCommentFile(ab);	// descriptions for files and folders in ab
	_ReadInfo(ab);			// titles for album and images inside
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

	bool _justChanges = ((albumgen.AlbumCount() == 0 || albumgen.ImageCount() == 0)); // else full creation

	if (!_justChanges)
	{
		_imageMap.clear();
		_textMap.clear();
		_albumMap.clear();
	}

	// add default image when no image is found This is the only one with id == 0
	// the file 'NoImage.jpg' must be put into the 'res' directory
	Image im;
	im.exists = true;
	im.name = "NoImage.jpg";
	im.ID = 0;
	im.width = im.height = im.owidth = im.oheight = 800;
	_imageMap[0] = im;

	QString s = CONFIGS_USED::NameForConfig(".struct");
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
		result = _ReadJalbum();
	emit SignalAlbumStructChanged();	// show list of albums in GUI
//	if (result)
//		_structAlreadyInMemory = true;
	return result;
}
//*****      END OF READ PART, START OF WRITE PART  *****

//---------------------- helpers --------------------------

/*==========================================================================
* TASK:		ask if to cancel directory creation
* EXPECTS: s - name of directory to display in message box
* RETURNS: true: ok to create, false: cancel create
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
static bool __CancelCreate(QString s)
{
	return  QMessageBox(QMessageBox::Question, QMainWindow::tr("falconG"),
		QMainWindow::tr("Directory '%1' does not exist.\n\nCreate?").arg(s),
		QMessageBox::Yes | QMessageBox::Cancel).exec() == QMessageBox::Cancel;
}
/*==========================================================================
* TASK:		Creates directory and conditionally asks for confirmation
* EXPECTS: sdir - directory to create
*			aks	- if directory does not exist then ask permission to proceed?
* RETURNS:  true: directory either exists or created successfully
* REMARKS: 	 ask set to true if dir. existed and false qhwn it did not
*--------------------------------------------------------------------------*/
static bool __CreateDir(_CDirStr sdir, bool &ask) // only create if needed
{										// ask - if does not exist ask what to do
	QDir folder(sdir.ToString());
	if (folder.exists())
		return ask = true;

	bool b = ask;
	ask = false;

	if (b && __CancelCreate(sdir.ToString()))
		return false;
	return QDir().mkdir(sdir.ToString());
}

/*============================================================================
* TASK:	   creates necessary directories
* EXPECTS:
* GLOBALS:	config
* REMARKS: - if directories does not have an absolute path
*				(windows: starting with "drive:\" or "\", linux: starts with '/')
*			 they are created in this hierarchy:
*			  config.dsGallery						destination directory
*				config.dsGRoot				gallery root	(may be empty then
*									all thes below will be in config.dsGallery)
*					config.dsAlbumDir	albums (HTML files) here
*					config.dsImageDir	images for all albums
*					config.dsCssDir		css files
*					config.dsFontDir		fonts
*					res					javascript files, icon files
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_CreateDirectories()
{
	__DestDir = config.dsGallery;		// destination directory the new gallery is
										// put inside it
	bool ask = true;
	if (!__CreateDir(__DestDir, ask))
		return false;

	__SetBaseDirs();

	_CDirStr res;
	res = "res";
	__ResourceDir = __RootDir + res;				// afalco.js, solyoma.js

	ask |= QDir::isAbsolutePath(config.dsGRoot.ToString());
	if (!__CreateDir(__RootDir, ask))
		return false;
	ask |= QDir::isAbsolutePath(config.dsAlbumDir.ToString());
	if (!__CreateDir(__AlbumDir, ask))
		return false;
	ask |= QDir::isAbsolutePath(config.dsCssDir.ToString());
	if (!__CreateDir(__RootDir + config.dsCssDir, ask))
		return false;
	ask |= QDir::isAbsolutePath(config.dsImageDir.ToString());
	if (!__CreateDir(__RootDir + config.dsImageDir, ask))
		return false;
	ask |= QDir::isAbsolutePath(config.dsThumbDir.ToString());
	if (!__CreateDir(__RootDir + config.dsThumbDir, ask))
		return false;
	ask |= QDir::isAbsolutePath(config.dsFontDir.ToString());
	if (!__CreateDir(__RootDir + config.dsFontDir, ask))
		return false;
	ask = false;
	if (!__CreateDir(__RootDir + res, ask))
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
			if ( s== "abbrev")
				Languages::abbrev[i] = sl[1];
			else if (s == "name")
				Languages::names[i] = sl[1];
			else if (s == "icon")
				Languages::icons[i] = sl[1];
			else if (s == "images")
				Languages::Images[i] = sl[1];
			else if (s == "albums")
				Languages::Albums[i] = sl[1];
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
			else if (s == "captions")
				Languages::showCaptions[i] = sl[1];
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
*			  and the language abbreviation. (e.g for TITLE_TAG  [Title-en:<text>])
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
		int lang = (reader.l()[level + 1] == THUMBNAIL_TAG[0] ? -1 : Languages::abbrev.indexOf(reader.l().mid(level + TITLE_TAG.length()+1, 2)));

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
				ids.thumbnailID = s.toULongLong();
			else
			{
				ids.thumbnailID = _AddImageFromPathInStruct(s);
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
			texts.SetTextForLanguageNoID(s.mid(level + 10, len - level - 11), lang);
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
* TASK:		helper - splits an image definition line intothe five constituents:
*				name, ID, dat, file length and path
* EXPECTS: s : string of definition w.o. leading spaces
* RETURNS: list of fields found
* REMARKS: - "normal" regular function split() could not be used as names
*					may contain braces and commas
*			result may contain fiewer or more fields than required
*--------------------------------------------------------------------------*/
QStringList __imageMapstructLineToList(QString s)
{
	QStringList sl;
	int pos0 = 0, pos;
	do
	{
		pos = s.indexOf("(", pos0);
		pos0 = pos + 1;
	} while (pos > 0 && !IsImageFile(s.mid(0, pos)));	// then ( in file name
	if (pos < 0)				// not image line or absolute/ relative image path only
	{
		if (IsImageFile(s))
			sl.push_back(s);
		return sl;
	}
	else
		sl.push_back(s.mid(0, pos));		// index #0 - file name

	pos0 = pos + 1;	   					// after the opening brace
	pos = s.indexOf(')',pos0);
	if (pos < 0)
		return sl;

	QRegExp rexp(",|x");
	sl += s.mid(pos0, pos - pos0).split(rexp);	// index #1..#7 ID, width, height, owidth, oheight, length, date
	if (sl.size() != 8 || pos == s.length() )    // was s[pos + 1] != '/')
		return sl;
	sl.push_back(s.mid(pos + 1));		// index #8 - original path (was pos + 2
	return sl;
}

/*==========================================================================
* TASK:		reads one image from structure
* EXPECTS: 	reader - open file reader, reader.l() contains 
*						the image specification line
*			level -  the level of images
*			album: add image to this album
*			thumbnail - is this the album thumbnail?
*
* RETURNS: success code
* REMARKS: - first image line: <spaces><file name> (<file id>) // <original path
*		   - throws 'BadStruct' on error
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ImageFromStruct(FileReader &reader, int level, Album &album, bool thumbnail)
{
	Image img;

	QStringList sl = __imageMapstructLineToList(reader.l().mid(level));
	int n = sl.size();		// when all fields are present this should be 9 :
		// image name, id, width, height, original width, original height, date, size, path

	ID_t id;
	if (n != 9 && n != 8)		   // maybe new image added and not yet processed
	{
		if (n != 1)	// and it should be either <config.dsSrc relative full path name>
			throw BadStruct(reader.ReadCount(), QString("Wrong image parameter count:%1").arg(n)); // or just the image name (from the same folder as the previous one)

		// expects: original/image/directory/inside/source/name.ext
		id = _AddImageFromPathInStruct(sl[0]);	 // structure is changed
		if (id)	
		{
			if (thumbnail)		 // It's parent also changes
			{
				album.thumbnail = id;
				if (album.parent)
					_albumMap[album.parent].changed = true;
			}
			album.changed = true;			// set it as changed always 
		}
		img = _imageMap[id];
	}
	else	// n == 8 or n == 9
	{
		id = sl[1].toULongLong();
		if (_imageMap.contains(id))
			img = _imageMap[id];

		img.name = sl[0];
		img.SetResizeType();	// handle starting '!!'

		img.ID = id;
		img.width = sl[2].toInt();
		img.height = sl[3].toInt();
		img.owidth = sl[4].toInt();
		img.oheight = sl[5].toInt();

// calculated when asked for		img.aspect = img.height ? (double)img.width / (double)img.height : 1;
		img.uploadDate = DateFromString(sl[6]);
		if (img.uploadDate > _latestDateLimit)
			_latestDateLimit = img.uploadDate;
		// DEBUG:
		if (img.uploadDate.toJulianDay() < 0)
			return 0;

		img.fileSize = sl[7].toULongLong();
		if (n == 9)
			ImageMap::lastUsedImagePath = sl[8];

		img.path = ImageMap::lastUsedImagePath;

		img.exists = (img.fileSize != 0);	// non existing images have 0 size
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

	_imageMap[id] = img;

	return img.ID;
}

/*==========================================================================
* TASK:		helper - splits an album definition line into 3 fields:
*				name, ID,  and path, some of these may be missing
* EXPECTS: s : INPUT string of definition w.o. leading spaces
*				format: 
*					<relative path name of album> - new album
*				or
*					(A:1) - root album: no name, no path (can be (C:1) too)
*				or
*					<original album name>(A:<ID)><relative path> - exisiting 
*						possibly unchanged
*			    or
*					<original album name>(C:<ID)><relative path> - existing
*						defineitely changed
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
	while (pos > 0 && s[pos0].unicode() != 'A' && s[pos0].unicode() != 'C' && s[pos+2].unicode() != ':');	// then '(A:<id>)<parent path>' is in file name

	if (pos == 0)			// root album
	{
		changed = (s[pos0].unicode() == 'C');
		return sl;
	}

	if (pos < 0)			 // new album w.o. ID 
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
							//	1: path name
							//	2: album name and ID
							//	3: album name, ID and path
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
		album.ID = id = 1;
		album.exists = true;		// do not check: virtual!
		_albumMap[id] = album;
	}
	else if (n == 1)	// then no ID is determined yet, sl[0] contains the whole config.dsSrc relative path
	{
		if (parent != 0)
			aParent->changed = true;
		if(QFile::exists(sl[0]))				// then images are inside this (not virtual directory)
			ImageMap::lastUsedImagePath = sl[0] + "/";

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
				ImageMap::lastUsedImagePath.clear();
			album.path = ImageMap::lastUsedImagePath;
			if (level == 1)	// top level album> its name is path for images and sub albums
				ImageMap::lastUsedImagePath = sl[0] + "/";
		}
		else // n==3 -> name, ID and path
		{
			album.path = sl[2];	// images are inside this album
			ImageMap::lastUsedImagePath = album.path + album.name + "/";
		}
		id = sl[1].toULongLong();
		if (_albumMap.contains(id))
			throw BadStruct(reader.ReadCount(), QString("'%1' - duplicated album ID").arg(id));


		album.ID = id;
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
				album.albums.push_back(aid);
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
				ID_t iid = _ImageFromStruct(reader, level+1, album, false);		   // false:not thumbnail
				album.images.push_back(iid);
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
			throw BadStruct(reader.ReadCount(), "Read error");

		QStringList sl;
		QString line = reader.NextLine(true);	// version line (comment format)

		if (line.left(versionStr.length()) != versionStr)
			throw BadStruct(reader.ReadCount(),"Bad version string");
		else
		{
			line = line.mid(versionStr.length());
			int pos = line.indexOf('.');
			if(pos < 0)
				throw BadStruct(reader.ReadCount(),"Missing '.' from version");

			config.majorStructVersion = line.left(pos).toInt();
			config.minorStructVersion = line.mid(pos+1).toInt();

			_LanguageFromStruct(reader);  // throws when error
			emit SignalSetLanguagesToUI();

			// from now on we need the empty lines as well
			reader.ReadLine();		// discard empty and comment
			if (reader.l()[0] != '(' || (reader.l()[1] != 'A' && reader.l()[1] != 'C'))
				throw BadStruct(reader.ReadCount(),"Invalid / empty root album line");
					// recursive album read. there is only one top level album
					// with id == 1 + ALBUM_ID_FLAG (id == ALBUM_ID_FLAG is not used)
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
						QMainWindow::tr("'\n\n"
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
* TASK:		reads whole Jalbum hierarchy starting at directory 'root'
* EXPECTS: 'root' path name of uppermost source album directory
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadJalbum()
{

	if (!Languages::Read())		// cancelled, no default set
		return false;
			// first languages from en.lang, etc
	emit SignalSetLanguagesToUI();
			// get number of sub albums
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


	_root.Clear();
	_root.ID = 1;							// id = 0: invalid
	_root.path = config.dsSrc.ToString();
	// root has no name and no path it is inside config.dsSrc
	_root.exists = true;

	_albumMap[1] = _root;	// a copy of _root is first on list	

	_remDsp.Init(directoryCount);
	_ReadOneLevel(_albumMap[1] );
	if (!_processing)
		return false;

	_root = _albumMap[1];	// get values stored in hierarchy back to _root
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
* TASK:	Creates constants.php		-   - " -
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 1: error writing file
* REMARKS: the file 'up-icon.png' and 'left-icon.png' are not copied, 
*			but created from icon images in the res directory of falconG
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoCopyRes()
{

	QString src = config.dsApplication.ToString() + "res/",
			dest = (config.dsGallery + config.dsGRoot).ToString() + "res/";
	QDir dir(src);  // res in actual progam directory
	QFileInfoList list = dir.entryInfoList(QDir::Files);
	for (QFileInfo &fi : list)
	{
		if(fi.fileName() != QString("left-icon.png") && fi.fileName() != QString("up-icon.png"))
			QFile::copy(fi.absoluteFilePath(), dest + fi.fileName());
	}
	emit SignalToCreateUplinkIcon(dest + "left-icon.png");
	emit SignalToCreateUplinkIcon(dest + "up-icon.png");
	return 0;	
}

enum _What : int { wNone, wColor, wBackground, wNoClosingBrace = 0x8000 };
/*===========================================================================
* TASK: write shadow parameter
* EXPECTS:	s			- name of shadow (e.g. "text-shadow, -moz-box-shadow)
*			elem		- name from CONFIG
* GLOBALS: config,
* RETURNS: none
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_ShadowToString(QString s, _CElem &elem)
{
	if (!elem.shadow.use)
		return QString();

	s += QString().setNum(elem.shadow.Horiz()) + "px " + QString().setNum(elem.shadow.Vert()) + "px";
	if (elem.shadow.Blur())
		s += " " + QString().setNum(elem.shadow.Blur());
	s += " " + elem.shadow.color + ";\n";
	return s;
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

/*===========================================================================
* TASK: write color and background css with shadow
* EXPECTS: selector	- CSS selector
*			elem		- name from CONFIG
*			what		- 1: color, 2: background, 3 : both
* GLOBALS: config
* RETURNS: none
* REMARKS: CSS ends with '}\n"
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_ElemColorCssToString(QString selector, _CElem &elem, int what)
{
	if (elem.color.name.isEmpty())
		return QString();

	selector += "{\n";
	if (what & wColor)
		selector += " color:" + ColorToStr(elem.color) + ";\n";
	if (what & wBackground)
		selector +=" background-color:" + ColorToStr(elem.background) + ";\n";
	if (elem.shadow.IsSet())
		selector += _ShadowToString(" text-shadow:", elem);
	if ((what & wNoClosingBrace) == 0)
		selector +="}\n\n";
	return selector;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_GradientCssToString(_CElem &elem, bool invert)
{
	QString s;
	if (invert)
		s ="linear-gradient(#"
		+ elem.gradient.gs[0].color + " " + QString().setNum(elem.gradient.gs[2].percent) + "%, #"
		+ elem.gradient.gs[1].color + " " + QString().setNum(elem.gradient.gs[1].percent) + "%, #"
		+ elem.gradient.gs[2].color + " " + QString().setNum(elem.gradient.gs[0].percent) + "%);";
	else
		s ="linear-gradient(#"
		+ elem.gradient.gs[0].color + " " + QString().setNum(elem.gradient.gs[0].percent) + "%,#"
		+ elem.gradient.gs[1].color + " " + QString().setNum(elem.gradient.gs[1].percent) + "%,#"
		+ elem.gradient.gs[2].color + " " + QString().setNum(elem.gradient.gs[2].percent) + "%);\n";
	return s;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_MenuColorCSSToString()
{
	QString s;
	// state: UP
	s = "div.menu-line {\n"
		"	position: -webkit-sticky;\n"	 /* Safari */
		"   position: sticky;\n"
		"   top : 10px;\n"
		"   display:inline-block;\n"
		"   margin-bottom:1rem;\n"
		"}\n"
		"div.menu-line a {\n";
	if (config.Menu.gradient.used)
		s += " background:" + _GradientCssToString(config.Menu);
	else
		s += " color:" + ColorToStr(config.Menu.color) + ";\n background-color:" + ColorToStr(config.Menu.background) + ";\n";
	s += " border: 1px solid " + config.Menu.border.color.name + ";";
	if (config.Menu.shadow.use)
		s += _ShadowToString(" -webkit-box-shadow", config.Menu) +
		_ShadowToString(" -moz-box-shadow", config.Menu) +
		_ShadowToString(" box-shadow", config.Menu);
	s += "\n}\n"
		// state: Down
		// menu button pressed: change color order
		"div.menu-line a:hover {\n";
	if (config.Menu.gradient.used)
		s += " background:" + _GradientCssToString(config.Menu, true);
	else
		s += " color:" + ColorToStr(config.Menu.background) + ";\n background-color:" + ColorToStr(config.Menu.color) + ";\n";
	s += " border: 1px solid " + config.Menu.border.color.name + ";";
	if (config.Menu.shadow.use)
		s += _ShadowToString(" -webkit-box-shadow", config.Menu) +
		_ShadowToString(" -moz-box-shadow", config.Menu) +
		_ShadowToString(" box-shadow", config.Menu);
	s += "\n}\n";
	return s;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_ColorCSSToString()
{
	QString s;

	s = "/* Header Style  first for mobile then for tablet and desktop*/\n" +
		_ElemColorCssToString("html,\nbody,main", config.Web, wBackground) +
		_ElemColorCssToString(".falconG", config.SmallGalleryTitle, wColor) +
		".about{\n"
		"	color:#ddd;\n"
		"	margin:auto;\n"
		"	}\n"
		+
		_ElemColorCssToString("h2.album-title", config.GalleryTitle, wColor) +
		_ElemColorCssToString(".album-title,a:visited.album-title", config.AlbumTitle, wColor) +
		_ElemColorCssToString(".album-desc", config.AlbumDesc, wColor) +
		_ElemColorCssToString("h2.desc", config.AlbumDesc, wColor) +
		_ElemColorCssToString("a, a:visited", config.Menu, wColor) +
		_ElemColorCssToString("a.doboz", config.Lang, wColor) +
		_ElemColorCssToString("a[name=\"images\"],a[name=\"galleries\"]", config.Section, wColor) +
		_ElemColorCssToString(".folders p", config.AlbumDesc, wColor) +
		_ElemColorCssToString("#images p", config.ImageDesc, wColor) +
		_ElemColorCssToString("footer", config.ImageDesc, wColor) + 

		// block for menu buttons
		_MenuColorCSSToString() + 
		// end of menu button

		_ElemColorCssToString("#images, .folders", config.images, wColor);
	if (config.imageBorder.used)
		s += "section div.thumb img{\n 	border:" + QString().setNum(config.imageBorder.width) + "px solid " + config.imageBorder.color.name + ";\n}\n";
// DEBUG
//	QMessageBox(QMessageBox::Information, "falconG - info", s, QMessageBox::Ok, frmMain).exec();

	return s;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString _FontToCss(_CElem & elem)
{
	QString s = "	font-size:" + elem.font.sizestr + ";\n"
		"	font-family:" + elem.font.family + ";\n";

	if (elem.font.features & _CFont::fBold)
		s += "	font-weight:bold;\n";
	if (elem.font.features & _CFont::fItalic)
		s += "	font-style:italic;\n";

	if (elem.font.features & (_CFont::fUnderline | _CFont::fStrikethrough))
		s += "	text-decoration:";
	if (elem.font.features & _CFont::fUnderline)
		s += "underline";
	if (elem.font.features & _CFont::fStrikethrough)
		s += " strikethrough";
	if (elem.font.features & (_CFont::fUnderline | _CFont::fStrikethrough))
		s += ";\n";
	return s;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_CssToString()
{
	QString s =
		"@import url(\"https://fonts.googleapis.com/css?family=Raleway\"); /*--- Nav Font --*/\n"
		"@import url(\"https://fonts.googleapis.com/css?family=Playfair+Display:700|Raleway\"); /*--- Heading Font --*/\n"
		// TODO: scan font directory and embed fonts from it into here
		// e.g. @font-face {
		//				font-family: 'RieslingRegular';
		//				src: url('fonts/riesling.eot');
		//				src: local('Riesling Regular'), local('Riesling'), url('fonts/riesling.ttf') format('truetype');
		//		}
		// usage example: h1 {font-family: 'RieslingRegular', Arial, sans-serif;}
		// universal selector to include padding and border in box size
		"*, *::before, *::after {\n"
		"  box-sizing:border-box;\n"
		"  padding:0;\n"
		"  margin:0\n;"
		"}\n\n"
		"html,\n"
		"body{\n"
		"	height: 100% ;\n"
		"	width: 100%;\n"
		"   overflow: auto;\n"
		"}\n"
		"\n"
// .falconG
		".falconG{\n" 
		+ _FontToCss(config.GalleryTitle) + 
		"	margin-left: 10px;\n"
		"}\n"
		"\n"
// h2
		"h2.album-title{\n" // only for gallery title else have overriding class
		+ _FontToCss(config.GalleryTitle) +
		"	text-align: center;\n"
		"}\n"
		"\n"
// about
		".about{\n"
		"	margin:auto;\n"
		"	width:400px;\n"
		"}\n"

// a.title
		"a.title{\n"
		+ _FontToCss(config.GalleryTitle) +
		"}\n"
		"\n"
		// a, a:visited
		"a, a:visited{\n"
		"	text-decoration: none;\n"
		"}\n"
		"\n"
		"a.doboz {\n"
		"	font-size: 8pt;\n"
		"}\n"
// a[name = \"images\"], a[name = \"galleries\"]
		"a[name=\"images\"], a[name=\"galleries\"]{\n"
		+ _FontToCss(config.Section) +
		"	text-align:center;\n"
		"	padding:1em 0 1em 0;\n"
		"}\n"
		"\n"
// p
		"p{\n"
		"	font-size: 13px;\n"
		"}\n"
		"\n"
// div.menu-line a
		"div.menu-line a{\n"
		+ _FontToCss(config.Menu) +
		"	line-height: 18px;\n"
		"	border-radius: 5px;\n"
		"	box-shadow: 0 1px 1px rgba(0, 0, 0, 0.1);\n"
		"	display: inline-block;\n"
		"	margin: 2px 3px 2px 2px;\n"
		"	padding: 4px 10px 3px;\n"
		"}\n"
		"\n"
// a.facebook
		"a.facebook{\n"
		"	display: inline-block;\n"
		"	border: 1px solid #00266d;\n"
		"	padding: 4px 10px 3px;\n"
		"	font-size: 13px;\n"
		"	line-height: 18px;\n"
		"	margin: 2px 3px;\n"
		"	font-weight: 800;\n"
		"	-webkit-box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
		"	-moz-box-shadow:0px 1px 1px rgba(0,0,0,0.1);\n"
		"	box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
		"	color:#fff;\n"
		"	-webkit-border-radius: 5px;\n"
		"	-moz-border-radius: 5px;\n"
		"	border-radius: 5px;\n"
		"	background-color: #033aa9;\n"
		"	background: -moz-linear-gradient(top, #033aa9 0%, #022a8f 44%, #00266d 100%);\n"
		"	background: -webkit-linear-gradient(top, #033aa9 0%,#022a8f 44%,#00266d 100%);\n"
		"	background: -o-linear-gradient(top, #033aa9 0%,#022a8f 44%,#00266d 100%);\n"
		"	background: -ms-linear-gradient(top, #033aa9 0%,#022a8f 44%,#00266d 100%);\n"
		"	background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#033aa9), color-stop(44%,#022a8f), color-stop(100%,#00266d));\n"
		"	background: linear-gradient(#033aa9 0% ,#022a8f 44% ,#00266d 100% );\n"
		"}\n"
		"\n"
// <header>
		"header{\n"
		+ _FontToCss(config.Web) +
		"	text-align: left;\n"
		"	text-shadow: 1px 1px 1px rgba(0, 0, 0, 0.2);\n"
		"}\n"
		"\n"
// <footer>
		"footer{\n"
		+ _FontToCss(config.Web) +
		"	text-align: center;\n"
		"	margin:auto;\n"
		"}\n"
		"\n"
// Album title
		".album-title{\n"
		+ _FontToCss(config.AlbumTitle) +
		"}\n"
// album-desc
		".album-desc{\n"
		+ _FontToCss(config.AlbumDesc) +
		"	margin:auto;\n"
		"	width:100%;\n"
		"   text-align:center;\n"
		"}\n"
		"\n"
// <main>
		"	/* --- main section --*/\n"
		"main{\n"
		"	display: flex;\n"
		"	flex-direction: column;\n"
		"	flex-wrap:wrap;\n"
		"	width: 100% ;\n"
		"}\n"
		"	/* --- end main section --*/\n"
		"\n"
// #images, #folders
		"#images, .folders{\n"
		"	display: flex;\n"
		"	flex-flow: row wrap;\n"
		"	justify-content: center;\n"
		"	max-width: 100% ;\n"
		"}\n"
		"\n"
// .img-container .gallery container
		"		/* ---- img----- */\n"
		".img-container, .gallery-container{\n"
		"	display:flex;\n"
		"	flex-direction: column;\n"
		"   padding:0 3px;\n"
		"}\n"
		"\n"
// .desc
		".desc{\n"
		"	display:none;\n"
		"	hyphens: auto;\n"
		"	-moz-hyphens: auto;\n"
		"	-webkit-hyphens: auto;\n"
		"	-ms-hyphens: auto;\n"
		"   max-width:600px\n"
		"}\n"
		"\n"
// p.album-title
		"p.album-title{\n"
		"	padding-left:10px;\n"
		"}\n"
		"\n"
// .desc p
		".desc p{\n"
		"	font-size: 12pt;\n"
		"	line-height: 12pt;\n"
			"   text-align: center;\n"
		"}\n"
		"\n"
		"section div.thumb{\n"
		"	max-width:100% ;\n"
		"	display:-webkit-flex;\n"
		"	display:-ms-flexbox;\n"
		"	display:flex; \n"
		"	justify-content: center;\n"
		"	flex-wrap: wrap;\n"
		"	flex-direction: row;\n"
		"	padding: 2px;\n"
		"   margin: 0 2;\n"
		"}\n";
		// image border is set in _ColorCSSToString()
		s += "\n"
		".img-container img, .gallery-container img{\n"
		"	margin:auto;\n"
		"	max-width: 100% ;\n"
		"}\n"
		"\n"
		"div.links{\n"
		"	display:flex;\n"
		"	width:100% ;\n"
		"	justify-content:center;\n"
		"	margin-bottom: 10px;\n"
		"	padding-bottom: 40px;\n"
		"}\n"
		"div.links a, .showhide{\n"
		"	text-decoration:none;\n"
		"	font-family:\"Playfair Display\", sans-serif, Arial;\n"
		"	font-weight:bold;\n"
		"	cursor:pointer;\n"
		"}\n"
		"\n"
		"div.links a:hover, .showhide:hover{\n"
		"	font-weight: 700;\n"
		"}\n"
		"\n"
		"	/* -- media queries ***/\n"
		"@media only screen and (orientation: landscape) {\n"
		"	img{ max-height: 90vh; }\n"
		"}\n"
		"@media only screen and (min-width:700px) {\n"
		"	.img-container, .gallery-container\n"
		"	{\n"
		"		max-width:100vw;\n"
		"		padding:0 3px;\n"
		"	}\n"
		"	img{\n"
		"		max-width: 100%;\n" 
			//max-height:267px; 
		"	}\n"
		"	p{\n"
		"		font-size:12pt\n"
		"	}\n"
		// about
		"   .about{\n"
		"		margin:auto;\n"
		"		width:600px;\n"
		"  	}\n"
		"	.album-desc{\n"
		"		width:600px;\n"
		"	}\n"
		"}\n"
		"\n"
		"	/* large screens */\n"
		"@media only screen and (min-width:1200px) {\n"
		"	.img-container, .gallery-container\n"
		"	{\n"
		"		max-width:400px;\n"
		"		flex-direction:column;\n"
		"		padding:0 3px;\n"
		"	}\n"
		"	img{\n"
		"		max-width: 100%;\n" 
		"		max-height:267px;\n"
		"	}\n"
		"	p{\n"
		"		font-size: 12pt;\n"
		"	}\n"
		// about
		"   .about{\n"
		"		margin:auto;\n"
		"		width:800px;\n"
		"	}\n"
		"	.album-desc{\n"
		"		width:800px;\n"
		"	}\n"
		"}\n";
	return s;
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
		QString("# change web access to https by redirecting page with code 301 (moved permanently)\n"
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
* TASK:		Creates colors.css		-  from design parameters
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 4: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoColorsCss()
{
	// generate new colors.css
	QFile fcssC(QString(__CssDir.ToString()) + "tmpcolor.css");
	if(!fcssC.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return 4;

	_ofs.setDevice(&fcssC);
	_ofs << _ColorCSSToString();
	fcssC.close();

	BackupAndRename(QString(__CssDir.ToString()) + "colors.css", QString(__CssDir.ToString()) + "tmpcolor.css");
	return 0;
}

/*============================================================================
* TASK:		falconG.css		-  from all parameters
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoStyleFG()
{
	// generate new falconG.css into temporary files
	QFile fcssG(QString(__CssDir.ToString()) + "tmpfg.css");
	if (!fcssG.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return 8;

	_ofs.setDevice(&fcssG);
	_ofs << _CssToString();
	fcssG.close();

	BackupAndRename(QString(__CssDir.ToString()) + "falconG.css", QString(__CssDir.ToString()) + "tmpfg.css");
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_PageHeaderToString(ID_t id)
{
	QString supdir = (id == 1 ? "" : "../"),
			sCssLink = QString("<link rel=\"stylesheet\" href=\"") +
					supdir + 
					config.dsCssDir.ToString();

	QString s = QString("<!DOCTYPE html>\n"
		"<html lang = \"" + Languages::abbrev[_actLanguage] + "\">\n"
		"<head>\n");
	if (config.googleAnalyticsOn)
		s += _GoogleAnaliticsOn();

	s += QString("<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">"
		"<title>" + config.sGalleryTitle + "</title>\n");
	if (!config.sDescription.IsEmpty())
		s += QString("<meta name=\"description\" content=\"" + config.sDescription + "/>\n");
	if (!config.sKeywords.IsEmpty())
		s += QString("<meta name=\"keywords\" content=\"" + config.sKeywords + "/>\n");
	s += QString(sCssLink + "colors.css\">\n" +
		sCssLink + "falconG.css\">\n" + 
		"<script type=\"text/javascript\"  src=\"" + supdir + "res/falconG.js\"></script>"	);

	s += QString("</head>\n");

	return s;
}

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

	QString path, name, ext;						// see if name has a path, which may be outside /inside of gallery root
	SeparateFileNamePath(base, path, name, &ext);
	if (ext.isEmpty())
		ext = ".html";
	if (path.isEmpty())
		base = config.dsGRoot.str + name;
	else
		base = (config.dsGRoot + path).ToString() + name;
	if (multipleLanguages)
		base += "_" + Languages::abbrev.at(language); // e.g. index_en.html

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
/*============================================================================
  * TASK:	emit facebook link 	when required
  * EXPECTS: linkNam - name to link to e.g. https://your.site/directory
  *			ID	- different handlig for root album
  * RETURNS:
  * GLOBALS:
  * REMARKS: 
 *--------------------------------------------------------------------------*/
void AlbumGenerator::_OutputFacebookLink(QString linkName, ID_t ID)
{
	if (!config.bFacebookLink)
		return;
		// facebook link
	QString updir = (ID == 1 ? "" : "../");

	_ofs << "\n<div class = \"fb-like\"  style=\"float:right\"; data-href=\"" << linkName		// https://andreasfalco.com/albums/album1234.html\" 
		<< "\"\ndata-layout=\"box_count\"\n"
		"data-action=\"share\"\n"
		"data-size=\"small\"\n"
		"data-show-faces=\"true\"\n"
		"data-share=\"true\"></div>\n";
		//	"<span class=\"fb-share-button\" style=\"float:right; color:white\" data-href=\"" << linkName
		//<< "<img src=\"" << updir << "res/facebook.png\" width=\"24px\" height=\"24px\" valign=\"center\">" << Languages::share[_actLanguage] << "</a>\n</span>\n";
}
/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString AlbumGenerator::_IncludeFacebookLibrary()
{
	return QString("<!--get facebooks js code-->\n"
		"<div id = \"fb-root\"></div>\n"
		"<script>(function(d,s,id) {\n"
		"var js, fjs=d.getElementsByTagName(s)[0];\n"
		"if (d.getElementById(id)) return;\n"
		"js=d.createElement(s); js.id=id;\n"
		"js.src='https://connect.facebook.net/" + Languages::countryCode[_actLanguage] + "/sdk.js#xfbml=1&version=v3.1';\n"
		"fjs.parentNode.insertBefore(js, fjs);\n"
		"}(document,'script','facebook-jssdk'));\n"
		"</script>\n");
}


/*========================================================
 * TASK:	emit the sticky menu line at the top of the window
 * EXPECTS: album - album to be created  
 *			uplink - page link ifany
 * GLOBALS:	Languages, 'config'
 * RETURNS: nothing
 * REMARKS: -if and no uplink is given in config no UP button is generated
 *-------------------------------------------------------*/
void AlbumGenerator::_OutputMenuLine(Album &album, QString uplink)
{
	QString updir = (album.ID == 1) ? "" : "../";
	// menu buttons
	_ofs << "<div class=\"menu-line\">\n";
	if (!updir.isEmpty() && !uplink.isEmpty())
		_ofs << "<a href=\"" << uplink
		<< "\"><img src=\"" + updir + "res/up-icon.png\" style=\"height:14px;\" title=\"" + Languages::upOneLevel[_actLanguage] + "\" alt=\""
		+ Languages::upOneLevel[_actLanguage] + "\"></a>\n";			  // UP link
	_ofs << "<a href=\""
		<< updir << config.homeLink << "\">" << Languages::toHomePage[_actLanguage]
		<< "</a>\n";																											  // Home page
	if (config.bMenuToAbout)
		_ofs << "<a href=\"" << updir << Languages::FileNameForLanguage(config.sAbout, _actLanguage) << "\">" << Languages::toAboutPage[_actLanguage] << "</a>\n";
	if (config.bMenuToContact)
		_ofs << "<a href=\"mailto:" << config.sMailTo << "\">" << Languages::toContact[_actLanguage] << "</a>\n";
	if (config.bMenuToToggleCaptions && album.DescCount())
		_ofs << "<a href=\"#\" onclick=\"javascript:ShowHide()\">" << Languages::showCaptions[_actLanguage] << "</a>\n";
	if (album.SubAlbumCount() == 0  || (album.SubAlbumCount() && album.ImageCount()) )	// when no images or no albums no need to jump to albums
		_ofs << "<a href='#folders'>" << Languages::toAlbums[_actLanguage] << "</a>\n";								  // to albums
	if (config.generateLatestUploads)
		_ofs << "<a href=\"" << config.dsGRoot.ToString() << "latest_" + Languages::abbrev[_actLanguage] << ".php\">" << Languages::latestTitle[_actLanguage] << "</a>\n";

	_ofs << "</div>\n";
	//// debug
	//"<p style=\"margin-left:10px; font-size:8pt;font-family:Arial;\"id=\"felbontas\"></p>  <!--debug: display screen resolution-->
	_ofs << "<br>\n";
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
int AlbumGenerator::_OuputHeaderSection(Album &album)
{

	_ofs << "<header>\n"
		    "<a href=\""<< album.SiteLink(_actLanguage) << "\">"
				"<span class=\"falconG\">" << config.sGalleryTitle << "</span>"
			"</a>&nbsp; &nbsp;";
	// _actLanguage switch texts
	for (int i = 0; i < Languages::Count(); ++i)
		if (i != _actLanguage)
			_ofs << "<a class=\"doboz\" href=\"" + album.NameFromID(i) + "\">"   << Languages::names[i] << "</a>&nbsp;&nbsp\n";
	// facebook link
	_OutputFacebookLink(album.LinkName(_actLanguage, true), album.ID);
	_ofs << "<br><br><br>\n";
	if(album.titleID)
		_ofs << "<h2 class=\"album-title\">" << DecodeLF(_textMap[album.titleID][_actLanguage], true) << "</h2>\n";
	if (album.descID)
		_ofs << "<p class=\"album-desc\">" << DecodeLF(_textMap[album.descID][_actLanguage], true) << "</p>\n";
	_ofs << "</header>\n";
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteFooterSection(const Album & album)
{
		// TODO
	_ofs << " <!-- footer section with social media links -->\n"
		"<footer class = \"footer\">\n"
		<< QString(Languages::countOfImages[_actLanguage]).arg(album.images.size()).arg(album.albums.size()) << "<br>\n"
		<< Languages::falconG[_actLanguage] <<
			"</footer>\n";
	return 0;
}

/*============================================================================
* TASK:		writes eiter an image or a gallery section
* EXPECTS:	album - actual album whose content (images and sub-albums) is written
* GLOBALS: config, Languages
* REMARKS:  - root albums (ID == 1) are written into gallery root, others
*			  are put into directory 'albums' 
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteGalleryContainer(Album & album, bool itIsAnAlbum, int i)
{
	IdList &idList = (itIsAnAlbum ? album.albums : album.images);
	ID_t id = idList[i];  
	QString title, desc, sImageDir, sImagePath, sThumbnailDir, sThumbnailPath;

	QString sOneDirUp, sAlbumDir;
	if (album.ID == 1)
	{
		sAlbumDir = config.dsAlbumDir.ToString();	// root album: all other albums are inside 'albums'
		sOneDirUp = "";					// and the img directory is here
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
	if (itIsAnAlbum)
	{
		thumb = _ThumbnailID(_albumMap[id], _albumMap);
		if (thumb)
		{
			if (_imageMap.contains(thumb))
				pImage = &_imageMap[thumb];
			else
			{
				sImagePath = sImageDir + QString("%1.jpg").arg(thumb);
				sThumbnailPath = sThumbnailDir + QString("%1.jpg").arg(thumb);
			}
		}
		else
			pImage = &_imageMap[0];
	}
	else
		pImage = &_imageMap[id];

	_ofs << "   <section class=\"img-container\""
		 << ">\n"
		    "     <div class=\"thumb\">\n"
		    "       <a href=\"";
	if (itIsAnAlbum)
	{
		title = DecodeLF(_textMap[_albumMap[id].titleID][_actLanguage], true);
		desc = DecodeLF(_textMap[_albumMap[id].descID][_actLanguage], true);
		
		if (sImagePath.isEmpty())		// otherwise name for image and thumbnail already set
		{
			sImagePath = (pImage->ID ? sImageDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName() : pImage->name);
			sThumbnailPath = (pImage->ID ? sThumbnailDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName() : pImage->name);
		}
		_ofs <<  sAlbumDir << _albumMap[id].NameFromID(id, _actLanguage, false);	// non root albums are in the same sub directory
	}
	else
	{
		title = DecodeLF(_textMap[pImage->titleID][_actLanguage], true);
		desc = DecodeLF(_textMap[(pImage->descID)][_actLanguage], true);
		sImagePath = sImageDir + ( (album.images.size() > 0 ? pImage->LinkName() : QString()) );
		sThumbnailPath = sThumbnailDir + ((album.images.size() > 0 ? pImage->LinkName() : QString()));
		_ofs << sImagePath;		// image in the image directory
	}


	_ofs << "\"><img src=\"" + sThumbnailPath + "\" alt=\"\"></a>\n"
			"     </div>\n";									   // end of div thumb
	
    //  -------------------------- description
	if(!desc.isEmpty() )
		_ofs << "     <div class=\"desc\">\n"
		"       <p lang=\"" << Languages::abbrev[_actLanguage] << "\">"
			 << desc << "</p>\n"
		"     </div>\n";
	//  -------------------------- end of description

	//  -------------------------- links with  album/image title
	_ofs << "     <div class=\"links\">\n"		
		/*	"        <a href=\"#top\"><img src=\""+sOneDirUp+"res/up-icon.png\" style=\"height:14px;\" title=\""+Languages::upOneLevel[_actLanguage] +
																					"\" alt=\"" + Languages::upOneLevel[_actLanguage]  + "\"></a>\n"
		 */
				"        <a class=\"album-title\" href=\"" + sAlbumDir;
	if (itIsAnAlbum)
		_ofs << _albumMap[id].NameFromID(id, _actLanguage, false) + "\">";
	else
		_ofs << (sImagePath.isEmpty() ? "#" : sImagePath) + "\">";
	_ofs << (title.isEmpty() ? "&nbsp;" : title)	// was "---"
		<< "</a>\n";
	/*
		    "        <div class=\"showhide\" onclick=\"ShowHide()\"><img src=\""+sOneDirUp + "res/content-icon.png\" style=\"height:32px;\" title=\"" + Languages::showCaptions[_actLanguage] +
								"\" alt=\"" + Languages::showCaptions[_actLanguage] + "\"></a></div>\n"
			"     </div>\n";											 
    */
	// -----------------------------end of div links
	_ofs << "   </section>\n";

	return 0;
}

/*========================================================
 * TASK:	Process one image and thumbnail using
 *			converters
 * PARAMS:	im - actual Image (may be modified)
 *			converter - common converter for images
 *			thumbConverter - same for thumbnails
 * GLOBALS:
 * RETURNS:	nothing
 * REMARKS: -
 *-------------------------------------------------------*/
void AlbumGenerator::_ProcessOneImage(Image &im, ImageConverter &converter, std::atomic_int &cnt)
{
	int doProcess = ImageConverter::prImage | ImageConverter::prThumb;	// suppose both image and thumb changed

										// resize and copy and  watermark
	QString src = (config.dsSrc + im.path).ToString() + im.name,   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)
		dst = config.ImageDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions),
		thumb = config.ThumbnailDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions);

	QFileInfo fiSrc(src), fiThumb(thumb), fiDest(dst);						// test for source image
	bool srcExists = fiSrc.exists(),
		dstExists = fiDest.exists(dst),
		thumbExists = fiThumb.exists();

	if (!srcExists)	// then must check if it is changed
	{
		if (dstExists)						// source file was deleted
		{
			QFile::remove(dst);				// delete
			if (thumbExists)
				QFile::remove(thumb);
		}
		return;
	}

	ImageReader imgReader(src, im.dontResize);			   // constructor doesn`t read image!
	QSize size = imgReader.size();
	QImageIOHandler::Transformations tr = imgReader.transformation();
	if (tr & (QImageIOHandler::TransformationRotate90 | QImageIOHandler::TransformationMirrorAndRotate90))
		size.transpose();
	im.GetResizedDimensions();

	if (size.width() != im.owidth || size.height() != im.oheight)	// source changed rel. to data base: must re-create everything
	{
		im.owidth = size.width();
		im.oheight = size.height();

		im.GetResizedDimensions();		// must recalculate, 

		im.rdata.bSizeDifferent = true; // but it is sure changed
		im.rdata.bThumbDifferent = true;

		im.SetNewDimensions();			// process to these sizes
	}

	QDateTime dtSrc = fiSrc.lastModified();		// date of creation of source image.
// DEBUG
//		QString gsDate = dtSrc.toString();
// /DEBUG

	if (dstExists)		// then test if the source image was modified (width = 0: only added by name)
	{					// and if not then do not process it
		QDateTime dtDestCreated = fiDest.lastModified();	   // created() => birthTime() since Qt 5.10
// DEBUG
//			gsDate = dtDestCreated.toString();
// /DEBUG
		bool destIsNewer = dtDestCreated > dtSrc;	   // false for invalid date (no file) 
		int64_t	fiSize = fiSrc.size();

		if (config.bGenerateAll && config.bButImages)
			doProcess -= ImageConverter::prImage;			// then do not process
		else
		{
			if ( !im.rdata.bSizeDifferent && (fiSize == im.fileSize) && destIsNewer)
				doProcess -= ImageConverter::prImage;			// then do not process
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
		if (config.bButImages || (!MustRecreateThumbBasedOnImageDimensions(thumb, im) && thumbIsNewer))
			doProcess -= ImageConverter::prThumb;			// then do not process
// DEBUG
		else
			doProcess = doProcess;
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
		_structChanged = 1;			// when image changes structure must be re-saved

		WaterMark *pwm = nullptr;
		if (config.waterMark.used)
			pwm = &config.waterMark.wm;
		converter.flags = doProcess + (im.dontResize ? ImageConverter::dontResize : 0) + 
							(config.doNotEnlarge ? ImageConverter::dontEnlarge : 0);
		converter.oSize.setWidth(im.owidth);
		converter.oSize.setHeight(im.oheight);
		converter.newSize.setWidth(im.width);
		converter.newSize.setHeight(im.height);
		converter.thumbSize.setWidth(im.rdata.tw);
		converter.thumbSize.setHeight(im.rdata.th);

		/*im.aspect = */ converter.Process(imgReader, dst, thumb, config.bOvrImages, pwm);
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
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_ProcessImages()
{
	if (config.bButImages)
		return 0;

	// progress bar
	emit SignalToSetProgressParams(0, _imageMap.size(), 0, 1); // phase = 1
	std::atomic_int cnt = 0;	// count of images copied

	QRect maxSize{ config.imageWidth, config.imageHeight, config.thumbWidth, config.thumbHeight};
	ImageConverter converter(maxSize, config.doNotEnlarge);

	emit SignalToEnableEditTab(false);

	_remDsp.Init(_imageMap.size());
	if (config.waterMark.used)
		config.waterMark.wm.SetupMark();

	for (auto im : _imageMap)
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
		emit SignalProgressPos(cnt, _imageMap.size()); // progress bar
	}
	emit SignalToEnableEditTab(true);
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
* REMARKS: - root albums (ID == 1) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root
*--------------------------------------------------------------------------*/
int AlbumGenerator::__CreatePageInner(QFile &f, Album & album, int language, QString uplink, int & processedCount)
{
	if (!f.open(QIODevice::WriteOnly))
		return 16;

	_remDsp.Update(processedCount);
	emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
	emit SignalProgressPos(++processedCount, _albumMap.size() * Languages::Count());

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << _PageHeaderToString(album.ID);

	if (config.bRightClickProtected)
		_ofs << "<body oncontextmenu=\"return false;\">;\n";
	else
		_ofs << "<body>\n";

	if (config.bFacebookLink)
		_ofs << _IncludeFacebookLibrary(); 

	_OutputMenuLine(album, uplink);	/* sticky menu line*/
	_ofs << "<!--Header section-->\n";
	_OuputHeaderSection(album);

	_ofs << "<!-- Main section -->\n"
		"<main id=\"main\">\n";

	// get number of images and sub-albums
	if (album.ImageCount())
	{
		_ofs << "<!--the images in this sub gallery-->\n"
			<< "<a name=\"images\">" << Languages::Images[_actLanguage] << "</a>\n""<section id=\"images\">\n";
		// first the images
		for (int i = 0; _processing && i < album.images.size(); ++i)
			//			if (album.excluded.indexOf(album.images[i]) < 0)
			_WriteGalleryContainer(album, false, i);
		_ofs << "</section>\n<!-- end section Images -->\n";
	}
	if (album.SubAlbumCount())
	{
		_ofs << "\n<!--start section albums -->\n"
			<< "<a name = \"galleries\"  id = \"folders\">" << Languages::Albums[_actLanguage] << "</a>\n"
			"<section class=\"folders\">\n";

		for (int i = 0; _processing && i < album.albums.size(); ++i)
			//			if (album.excluded.indexOf(album.albums[i]) < 0)
			_WriteGalleryContainer(album, true, i);
		_ofs << "</section>\n<!-- end section Albums -->\n";
	}
	if (_processing)		// else leave tha page unfinished
	{
		// main section is finished
		_ofs << "\n</main> <!-- end of main section -->\n";
		// footer section
		_WriteFooterSection(album);
		// end html
		_ofs << "</body>\n</html>\n";
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
* REMARKS: - root albums (ID == 1) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root: when it not changed and need not re-create all albums, just signals
*			 it processed the album
*			- uses _CreatePageInner (co-function) to recursively create sub albums
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreatePage(Album &album, int language, QString uplink, int &processedCount)
{
	_actLanguage = language;

	QString s;
	if ( album.ID == 1)		// top level: use name from config 
		s = config.dsGallery.ToString() + RootNameFromBase(config.sMainPage.ToString(), language);
	else
		s = (config.dsGallery + config.dsGRoot + config.dsAlbumDir).ToString() + album.NameFromID(language); // all non root albums are in the same directory

	QFile f(s);
	if (!_mustRecreateAllAlbums && f.exists() && !album.changed /*&& !_structChanged*/)
	{
		_remDsp.Update(processedCount);
		emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
		emit SignalProgressPos(++processedCount, _albumMap.size() * Languages::Count());
	}
	else
		__CreatePageInner(f, album, language, uplink, processedCount);

	if (_processing) 		// create sub albums

	{
		if (album.SubAlbumCount())
		{
			uplink = album.NameFromID(language);
			if (album.ID == 1)
				uplink = QString("../") + uplink;;

			for (int i = 0; _processing && i < album.albums.size(); ++i)
			{
				if (album.excluded.indexOf(album.albums[i]) < 0)
					_CreatePage(_albumMap[album.albums[i]], language, uplink, processedCount);
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

	QString name = config.sMainPage.ToString(),
		s, path, fn, ext;
	SeparateFileNamePath(name, path, fn, &ext);

	if (name == config.sUplink.str)		// uplink  default name is empty
		s = QStringLiteral("_") + name;
	else
		s = name;
	s = (config.dsGallery + config.dsGRoot).ToString() + s;

	QFile f(s);
	if (!f.open(QIODevice::WriteOnly))
		return -1;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << _PageHeaderToString(1)
		<< "<body>\n"
		<< _IncludeFacebookLibrary()
		<< "<!--Header section-->\n"
		<< "<header>\n<span class=\"falconG\">" << config.sGalleryTitle << "</span>&nbsp; &nbsp;";
	_OutputFacebookLink(_albumMap[1].LinkName(_actLanguage, true), 1);	// when required
	_ofs << "<p class=\"menu-line\">\n";
	// languages
	for (int i = 0; i < Languages::Count(); ++i)
	{
		_ofs << QString("<a href=\"%1%2\">%3</a>").arg(fn + QStringLiteral("_") + Languages::abbrev[i]).arg(ext).arg(Languages::names[i]);
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
* REMARKS: each file is named <config.baseName><album.ID><Language::abbrev[]>,html
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoPages()
{
	if (!_processing)
		return 0;

	emit SignalToSetProgressParams(0, _albumMap.size() * Languages::Count(), 0, 3); // phase = 3

	_remDsp.Init(_albumMap.size() * Languages::Count());


	QString uplink = config.sUplink.str;	 // points to embedding page, relative to html 
											 // directory on server (inside e.g. public_html)
											 // e.g. single language: 
											 //		index.html corresponds to https://your.site.com/index.html 
											 // multiple languages:  as many pages as are languages:
											 // https://your.site.com/index_en.html, https://your.site.com/index_hu.html, etc)
											 // no default: can be empty or name w. or w.o extension: .htm, .html or .php
//	bool multipleLanguages = Languages::Count() > 1;
	QString path, name;				 

	int cnt = 0;

	for (int lang = 0; _processing && lang < Languages::Count(); ++lang)
	{
										// home of gallery root	 like 'index' or 'index.html'
		config.homeLink = uplink.isEmpty() ? config.sMainPage.ToString() : config.sUplink.ToString();

		_CreatePage(_albumMap[1], lang, uplink, cnt);	// all albums go into the same directory!
	}
	_CreateHomePage();

	return 0;
}

/*============================================================================
* TASK:	Creates latest uploads	- latest.html
* EXPECTS:	config fields are set
* GLOBALS:	'config'
* RETURNS: 0: OK, 32: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoLatest()
{
	if (!config.generateLatestUploads)
		return 0;
	// TODO: add code to falconG.js, add data to page Latest (special processing)
	return 0;
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
*			- colors.css		-  from design parameters	(error code 2)
*			- stylesa.css		-  from all parameters		(error code 4)
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
	if(_processing)
		if (_structChanged)		// do not save after read of structure
			WriteDirStruct();   // all album and image data is read in

	int i;					// returns:
	if (_processing)
		i = _DoCopyRes();			// 0 | 1	does not copy 'up-link.png' !
	if (_processing)
		i |= _DoHtAccess();			// 0 | 2
	if (_processing)
		i |= _DoColorsCss();		// 0 | 4
	if (_processing)
		i |= _DoStyleFG();			// 0 | 8
	if (_processing)
		i |= _DoPages();			// 0 | 16
	if (_processing)
		i |= _DoLatest();			// 0 | 32

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
void AlbumGenerator::SaveStyleSheets()
{
	__SetBaseDirs();
	albumgen._DoColorsCss();
	albumgen._DoStyleFG();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteStructReady(QString s)
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


//***************************** class AlbumStructWriterThread ****************
AlbumStructWriterThread::AlbumStructWriterThread(AlbumGenerator & generator, QObject * parent)	:
	_textMap(generator.Texts()), _albumMap(generator.Albums()), _imageMap(generator.Images()), QThread(parent)
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
	QMutexLocker locker( &_albumMapStructIsBeingWritten); 
			// mutex is locked and will bw unlocked when function returns

	QString p, n;
	SeparateFileNamePath(config.dsSrc.str, p, n);

/*
    QString s = QString(config.dsSrc.ToString()) + "gallery.struct",
		stmp = QString(config.dsSrc.ToString()) + "gallery.tmp";
 */
	sStructPath	= config.dsSrc.str + n + QString(".struct"),
	sStructTmp	= config.dsSrc.str + n + QString(".tmp");
	QFile f(sStructTmp);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		// error
		result = "Can't write file";
		emit resultReady(result);
		return;
	}
	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << versionStr << majorStructVersion << "." << minorStructVersion
		<< "\n#  © - András Sólyom (2018)\n\n"  // default values may differ from 'config'
		<< "#Source=" << config.dsSrc.ToString() << "\n"
		<< "#Destination=" << config.dsGallery.ToString() << "\n"
		<< "#Created at " << QDateTime::currentDateTime().toString() << "\n"
		<< "[Language count:" << Languages::Count() << "\n";
	for (int i = 0; i < Languages::Count(); ++i)
	{
		_ofs << i << "\n"
			<< "  abbrev=" << Languages::abbrev[i] << "\n"
			<< "  name=" << Languages::names[i] << "\n"
			<< "  icon=" << Languages::icons[i] << "\n"
			<< "  images=" << Languages::Images[i] << "\n"
			<< "  albums=" << Languages::Albums[i] << "\n"
			<< "  toAlbums=" << Languages::toAlbums[i] << "\n"
			<< "  homePage=" << Languages::toHomePage[i] << "\n"
			<< "  about=" << Languages::toAboutPage[i] << "\n"
			<< "  contact=" << Languages::toContact[i] << "\n"
			<< "  captions=" << Languages::showCaptions[i] << "\n"
			<< "  share=" << Languages::share[i] << "\n"
			<< "  latestTitle=" << Languages::latestTitle[i] << "\n"
			<< "  latestDesc=" << Languages::latestDesc[i] << "\n"
			<< "  countryCode=" << Languages::countryCode[i] << "\n"
			<< "  countOfImages=" << Languages::countOfImages[i] << "\n"
			<< "  falconG=" << Languages::falconG[i] << "\n"
			;
	}
	_ofs << "]\n\n# Album structure:\n";

	QString indent;	// for directory structure file: indent line with spaces to indicate hierarchy
	_WriteStructAlbums(_albumMap[1], indent);
	_ofs.flush();
	f.close();

	emit resultReady(result);
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
void AlbumStructWriterThread::_WriteStructImagesThenSubAlbums(Album & album, QString indent)
{
	QString s;
	int len = config.dsSrc.Length();
	Image *pImg;

	for (ID_t id : album.images)
		if (id && album.excluded.indexOf(id) < 0)		// not excluded
		{
			pImg = &_imageMap[id];
			if (pImg->changed)
			{
				album.changed = true;
			}
			if (pImg->exists)
			{
				_ofs << indent;
				if (pImg->dontResize)
					_ofs << "!!";
				_ofs << pImg->name << "(" 										   // field #1
					<< pImg->ID << ","											   // field #2
					<< pImg->width << "x" << pImg->height << ","				   // field #3 - #4
					<< pImg->owidth << "x" << pImg->oheight << ","				   // field #5 - #6
					// ISO 8601 extended format: yyyy-MM-dd for dates
					<< pImg->uploadDate.toString(Qt::ISODate) << ","			   // field #7
					<< pImg->fileSize << ")";									   // field #8
				s = pImg->path;
				if (s.left(len) == config.dsSrc.ToString())
					s = s.mid(len);
				_ofs << s << "\n";
				// field #9
			}
			else
				_ofs << indent << pImg->name << " # is missing\n";
			WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, pImg->descID, indent);
			WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, pImg->titleID, indent);
		}
	for (ID_t id : album.albums)
		if (id && album.excluded.indexOf(id) < 0)		// not excluded
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

	QString s = album.path;	// album.FullName()  +'/';
	//int len = config.dsSrc.Length();
	//if (s.left(len) == config.dsSrc.ToString())	// drop sorce path ending '/'
	//	s = s.mid(len);				// so root album will have no path 

	ID_t thumbnail = album.thumbnail = _ThumbnailID(album, _albumMap);		// thumbnail may have been a folder
																// name  originally, now it is an ID
	_ofs << "\n" << indent
		<< album.name << "(A:" << (album.ID) << ")" << s << "\n";
	WriteStructLanguageTexts(_ofs, _textMap, TITLE_TAG, album.titleID, indent);
	WriteStructLanguageTexts(_ofs, _textMap, DESCRIPTION_TAG, album.descID, indent);

	_ofs << indent << "[" << THUMBNAIL_TAG << ":" << thumbnail << "]\n"; // ID may be 0!

	_WriteStructImagesThenSubAlbums(album, indent + " ");
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
	if (s.left(config.dsSrc.str.length()) == config.dsSrc.str)
		s = s.mid(config.dsSrc.str.length());
	return s;
}
