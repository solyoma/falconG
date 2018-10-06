#include <QApplication>
#include <QApplication>
#include <QApplication>
#include <QApplication>
#include <QApplication>
#include <QApplication>
#include <QString>
#include <QTextCodec>
// DEBUG
#include <QtDebug>

#include "support.h"
#include "config.h"
#include "crc32.h"
#include "stylehandler.h"
#include "albums.h"
#include "falcong.h"

//******************************************************
AlbumGenerator albumgen;		// global
//******************************************************

/////////////////////////
struct BadStruct {};		// simple exception class
////////////////////////////

const QString TITLE_TAG       = "Title-";	// used in 'struct' files
const QString DESCRIPTION_TAG = "Descr-";	// e.g. [Tytle-hu:<text>]
const QString THUMBNAIL_TAG   = "AIcon-";	// "album icon"

// static members
Text TextMap::invalid;
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
			__FontDir,		// some of used fonts
			__ResourceDir;

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* RETURNS:
* REMARKS:
*--------------------------------------------------------------------------*/
static void __SetBaseDirs()
{
	//   Directory contains
	__RootDir = config.dsGallery + config.dsGRoot;	// .htaccess, constants.php, index.php
	__AlbumDir = __RootDir + config.dsAlbumDir;		// all albums
	__CssDir = __RootDir + config.dsCssDir;	  		// colors.css, stylefg.css
	__ImageDir = __RootDir + config.dsImageDir;		// all images. image name with date (?)
	__FontDir = __RootDir + config.dsFontDir;		// some of used fonts
}




/*--------------------- languages -------------------------*/
LanguageTextList Languages::abbrev;		// abbreviations: "hu", "en", etc
LanguageTextList Languages::names;			// used on menus to switch language, eg. "Magyarul"
LanguageTextList Languages::icons;			// icons to use instead of names
LanguageTextList Languages::toImages;	// image section in HTML files
LanguageTextList Languages::toAlbums;// album section in HTML files
LanguageTextList Languages::toHomePage;		// 'Kezdõlapra'
LanguageTextList Languages::toAboutPage;		// 'Rólam'
LanguageTextList Languages::toContact;		// á'Kapcsolat'
LanguageTextList Languages::showCaptions;    // 'Képleírások'
LanguageTextList Languages::share;			// 'Megosztás'
LanguageTextList Languages::latestTitle;		// 'Kedvcsinálónak'
LanguageTextList Languages::latestDesc;		// 'Ebbe a könyvtárba...'
									/*============================================================================
* TASK:		reads language definitions from files in the
*				a) source 
*				b) destination directory or
*				c) program directory
*	in this order
* EXPECTS:	name of file to read in
* GLOBALS:  
* RETURNS: 0: error, actual number of languages read so far: success
* REMARKS:	- file names: <lang abbrev>.lang	
*			- the program reads all such file from directory
*			- file formats:  
*			   1st line		"falconG Language file"
*				others may follow in any order and the right hand side may be empty
*				abbrev=<2 letter abbrev>
*				name=<text to put buttons or lists>
*				icon=<icon (png) file name>
*				Images=<text for Images section>
*				Albums=<text for Albums section>
*				HomePage=<e.g. Home>		
*				About=<about>		
*				Contact=<contact>
*				captions=<text to show descriptions";
*            	share=<text for sharing links>
*			- if no file does not exist sets a single languag: English
*			- do not use when languages are set from GUI
*--------------------------------------------------------------------------*/
int Languages::_Read(QString name)
{
	FileReader reader(name, FileReader::isUtf8 | FileReader::trim);
	QString line = reader.ReadLine();
	if (line != "falconG Language file")
		return 0;

	QString s;
	int next = abbrev.size();
	abbrev.push_back(s);
	names.push_back(s);
	icons.push_back(s);
	toImages.push_back(s);
	toAlbums.push_back(s);
	toHomePage.push_back(s);
	toAboutPage.push_back(s);
	toContact.push_back(s);
	showCaptions.push_back(s);
	share.push_back(s);
	latestTitle.push_back(s);
	latestDesc.push_back(s);

	QStringList sl;
	while (!(line = reader.ReadLine()).isEmpty())
	{
		sl = line.split('=');
		s = (sl.size() < 2 ? "" : sl[1]);
		if (sl[0].toLower() == "")
			;
		else if (sl[0].toLower() == "abbrev")
			abbrev[next] = s;
		else if (sl[0].toLower() == "name")
			names[next] = s;
		else if (sl[0].toLower() == "icon")
			icons[next] = s;
		else if (sl[0].toLower() == "images")
			toImages[next] = s;
		else if (sl[0].toLower() == "albums")
			toAlbums[next] = s;
		else if (sl[0].toLower() == "homepage")
			toHomePage[next] = s;
		else if (sl[0].toLower() == "about")
			toAboutPage[next] = s;
		else if (sl[0].toLower() == "contact")
			toContact[next] = s;
		else if (sl[0].toLower() == "captions")
			showCaptions[next] = s;
		else if (sl[0].toLower() == "share")
			share[next] = s;
		else if (sl[0].toLower() == "latesttitle")
			latestTitle[next] = s;
		else if (sl[0].toLower() == "latestdesc")
			latestDesc[next] = s;
	}
	return ++next;
}

/*============================================================================
* TASK:		reads all language definition files in program directory
* EXPECTS:	nothing
* GLOBALS:
* RETURNS: the number of languages read
* REMARKS:	
*
*			- if no file exists sets a single languag: English
*	?????		- do not use when languages are set from GUI
*--------------------------------------------------------------------------*/
int Languages::Read()
{
	Clear();

	QDir dir;
	QFileInfoList list;
	QStringList names; names << "*.lang";

	QDir d = QDir::current();

	dir.setCurrent(config.dsSrc.ToString());
	list = dir.entryInfoList(names, QDir::Files);
	if (!list.size())
	{
		dir.setCurrent(config.dsGallery.ToString());
		list = dir.entryInfoList(names, QDir::Files);
	}
	if (!list.size())
	{
		// actual program directory
		list = d.entryInfoList(names, QDir::Files);
		dir.setCurrent(d.path());	// go back to program directory
	}
	if (list.size() == 0)	// no files: set default language: English
	{
		if (QMessageBox(QMessageBox::Warning,
			QMainWindow::tr("falconG - Warning"),
			QMainWindow::tr("No language (*.lang) files are found.\nDo you want to cancel or continue with English as the single language?\n\n"
							 "To avoid this message create language files and put them into the\n"
							"    a) source\n"
							"    b) destination directory or\n"
							"    c) program directory\n"
							 "A language file is a UTF-8 coded text file with 13 lines.\n"
							 " (the single quotes in the examples below are not used.)"
							 "  First line: 'falconG Language file'\n"
							 "    followed by definitions of the texts used in the album. these are:\n"
							 "  abbrev=<2 letter language abbreviation> e.g. 'en'\n"
							 "  name=<language name to put on language switch menu> e.g.  'to English'\n"
							 "  icon=<name of icon (.png) file for language> may be left empty\n"
							 "  images=<text header for the images section of the actual album>\n"
							 "  albums=<text header for the album section of the  actual album>>\n"
							 "  homePage=<text for menu to home page>\n"
							 "  about=<text for the 'About' button>\n"
							 "  contact=<text for the 'contact' button\n"
							 "  share=<text for the facebook share button>"
							 "  latestTitle=<title for album latest.html>"
							 "  latestDesc=<description for album latest.html>"
			),
					QMessageBox::Ok | QMessageBox::Cancel, frmMain).exec() == QMessageBox::Cancel)
			return 0;
		abbrev.push_back("en");
		names.push_back("English");
		icons.push_back("");
		toImages.push_back("images");
		toAlbums.push_back("Albums");
		toHomePage.push_back("Home");
		toAboutPage.push_back("About");
		toContact.push_back("Contact");
		showCaptions.push_back("Captions");
		share.push_back("Share");
		latestTitle.push_back("Latest uploads");
		latestDesc.push_back("Randomly selected max " + QString().setNum(config.nLatestCount) + "Latest uploads");
		return 1;
	}
	for (auto fi : list)
	{
		_Read(fi.fileName());
	}
	return abbrev.size();
}

/*============================================================================
* TASK:   creates new empty language text vectors
* EXPECTS: newsize - when 0 it just clears all and creates lists with no
*						elements
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void Languages::Clear(int newsize)
{
	abbrev.Prepare(newsize);
	names.Prepare(newsize);
	icons.Prepare(newsize);
	toImages.Prepare(newsize);
	toAlbums.Prepare(newsize);
	toHomePage.Prepare(newsize);
	toAboutPage.Prepare(newsize);
	toContact.Prepare(newsize);
	share.Prepare(newsize);
	showCaptions.Prepare(newsize);
	latestTitle.Prepare(newsize);
	latestDesc.Prepare(newsize);
}

/*============================================================================
* TASK:		 Template to calculate CRC of either a text string or of a file content
* EXPECTS:	 stsr      - string to calculate CRC32 of or file name
*			 isContent - calculate content of file named as 'str'
* GLOBAL:	none
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
* GLOBAL:
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

/**************************** Text *****************************/
Image::SearchCond Image::searchBy = byID;		// 0: by ID, 1: by name, 2 by full name
Album::SearchCond Album::searchBy = byID;

/**************************** Text *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Text::Text(const QStringList &texts)
{
	*this = texts;
}
/*============================================================================
* TASK:		Calculate ID for a given text
* EXPECTS:
* GLOBAL:
* REMARKS: the ID is the CRC32 of the longest of the language texts
*--------------------------------------------------------------------------*/
ID_t Text::CalcID()
{
	int len = 0;

	lang = 0; // index of longest text in texts
	for (int l = 0; l < Languages::Count(); ++l)
		if (len < texts[l].length())
		{
			len = texts[l].length();
			lang = l;
		}

	CRC32 crc(texts[lang]);		// same ID for all languages
	ID = crc.crc();
	if (!ID)					// CRC was 0, but ID can't be
		ID = ID_INCREMENT;
	return ID;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void Text::Clear(int newSize)
{
	if (newSize > 0 && newSize != texts.size())
	{
		texts.clear();
		for (int i = 0; i < newSize; ++i)
			texts.push_back("");
	}
	else
		for (int i = 0; i < texts.size(); ++i)
			texts[i].clear();
	ID = 0;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Text &Text::operator=(const Text &t)
{
	ID = t.ID; 
	lang = t.lang;
	texts = t.texts; //  ToUTF8(const_cast<Text&>(t).texts);
	return *this;
}
/*============================================================================
* TASK:	  set up text from string list
* EXPECTS: txts - stringlist with as many elements as there are languages
* GLOBAL:
* REMARKS: text ID is determined from the longest language text
*--------------------------------------------------------------------------*/
Text &Text::operator=(const QStringList &txts)
{
	texts = txts;
	ID = CalcID();

	return *this;
}

/*============================================================================
* TASK:		'less then' operator
* EXPECTS:	t - other text with ID set up
* GLOBAL:
* REMARKS:	does not use lang
*--------------------------------------------------------------------------*/
int Text::operator<(const Text &t)
{
	return ID < t.ID;
}

/*============================================================================
* TASK:		text equality operator for texts using text id and lang and the
 *				texts themselves
 * EXPECTS:	t - other text, both id and lang is set up already
 *			this and t has the same number of language texts
 * GLOBAL:
 * REMARKS: if t.lang < 0 only checks the ID, otherwise it also checks lang
*--------------------------------------------------------------------------*/
bool Text::operator==(const Text &t)
{
	if (ID == t.ID)
		if (lang != t.lang)
			return false;
		else
			for (int i = 0; i < texts.size(); ++i)
				if (texts[i].length() != t.texts[i].length())
					return false;
				else if (texts[i] != t.texts[i])
					return false;
	return true;
}

/*============================================================================
* TASK:		text inequality operator for texts using text id and lang
* EXPECTS:	t - other text, both id and lang is set up already
* GLOBAL:
* REMARKS: if t.lang < 0 only checks the ID, otherwise it also checks lang
*--------------------------------------------------------------------------*/
bool Text::operator!=(const Text & t)
{
	return !(*this == t);
}

/**************************** TextMap *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Text &TextMap::Find(ID_t id)
{
	return contains(id) ? (*this)[id] : invalid;  // value(id, invalid);			// uses id only (lang = -1)
}

/*============================================================================
* TASK:		find search string in text with language code lang
* EXPECTS:	'search' search string in language 'lang'
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Text & TextMap::Find(QString search, int lang)
{
	QStringList qsl;
	for (int i = 0; i < Languages::Count(); ++i)
		if (i == lang)
			qsl << search;
		else
			qsl << "";

	Text text(qsl);	// sets id from longest text, which is now 'search'

	return contains(text.ID) ? (*this)[text.ID] : invalid;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Text& TextMap::Find(const QStringList & texts)
{
	Text text(texts);
	return contains(text.ID) ? (*this)[text.ID] : invalid;
}

/*============================================================================
* TASK:		Add an existing text to the text map
* EXPECTS: text - data with ID already set and ID can't be 0
*		   added - output parameter signals if new text was added or the
*					ID of an existing one is returned
* GLOBAL:
* RETURN: ID of text added
* REMARKS: if the same text is there already jus increments its usage count
*			else adds new text to it
*		   the text ID may be modified if a different text with this id 
*			is already on the list
*--------------------------------------------------------------------------*/
ID_t TextMap::Add(Text & text, bool added)
{
	added = false;
	while (contains(text.ID))	// same low 62 bits with maybe different text
	{
		if ((text.lang == (*this)[text.ID].lang) && (text.texts[text.lang] == (*this)[text.ID].texts[text.lang]))
		{
			++(*this)[text.ID]._usageCount;
			return text.ID;			// found, not added
		}
		text.ID += ID_INCREMENT;		// try next ID
	}
	added = true;
	++text._usageCount;
	(*this)[text.ID] = text;
	return text.ID;
}

/*============================================================================
* TASK:
* EXPECTS: texts - as many texts as tehere are languages
*		   added - output parameter signals if new text was added or the
*					ID of an existing one is returned
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
ID_t TextMap::Add(QStringList &texts, bool &added)
{
	Text txt(texts);
	return Add(txt, added);
}

/*============================================================================
* TASK:	decrements usage count and removes text from map if the usage count becomes 0
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void TextMap::Remove(ID_t id)
{
	Text *pt = nullptr;
	if (contains(id))
		pt = &(*this)[id];
	if (pt)
		if (--pt->_usageCount == 0)
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
* GLOBAL:
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
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QTextStream & Image::WriteTranslation(QTextStream & ofs) const
{
	if (exists)
	{
		ofs << LinkName() << " (" 
			// ISO 8601 extended format: either yyyy-MM-dd for dates or 
			<< uploadDate.toString(Qt::ISODate)
			<< ") // " << FullName() << "\n";
	}
	return ofs;
}

/**************************** ImageMap *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Image &ImageMap::Find(ID_t id, bool useBase)
{
	if (id & (ALBUM_ID_FLAG|INVALID_ID_FLAG)) return invalid;		// not an image
	ID_t mask = useBase ? BASE_ID_MASK : 0xFFFFFFFFFFFFFFFFul;
	Image im; im.ID = id;
	for (auto i = begin(); i != end(); ++i)
		if ( (i.key() & mask)  == (im.ID & mask) )
			return i.value();
	return invalid;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
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
* EXPECTS:  path is the full path name of an existing image
*			pos - output parameter: position in '_images'
* GLOBAL:
* REMARKS: - calculates the CRC32 of the file contents
*		   - when collisions occur adds a unique value above 0xFFFFFFFF to the ID
*--------------------------------------------------------------------------*/
ID_t ImageMap::Add(QString path, bool &added)	// path name of source image
{
	Image img;

	SeparateFileNamePath(path, img.path, img.name);

	added = false;
	ID_t id = CalcCrc(img.name, false);	// just by name. CRC is never 0

	QFileInfo fi(path);		// wee may need to check existence

	Image *found = &Find(id); // already in data base? (use base ID first)
	if( found->Valid())		// yes base ID found
	{
		do								// if name is the same too 
		{
			if (found->name == img.name)	// then same image
			{				// even when they are in different directories!
				if((found->path != img.path) && !found->exists)
				{      // then existence is not yet established
					found->exists = fi.exists();
				}
				return found->ID;
			}
			else						// same ID, different name - new image
				id += ID_INCREMENT;
		} while ((found = &Find(id, false))->Valid()); // full ID
	}
	// now I have a new ID
	// new image: 
	if (fi.exists())
	{
		img.exists = true;
		img.fileSize = fi.size();
	}

	if (!id)		// then --- NOT image does not exist
		return 0;

	added = true;
	img.ID = id;
	(*this)[id] = img;
	return id;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
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
* GLOBAL:
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
* GLOBAL:
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
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/

/*============================================================================
* TASK: get number of non-excluded images in album
* EXPECTS:
* GLOBAL:
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
* TASK: get number of non-excluded sub-albums in album
* EXPECTS:
* GLOBAL:
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

/*============================================================================
* TASK: Create full link name of album
* EXPECTS: language -  index of language
*			http - treuL force http(s) prefix
* GLOBAL:
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
	}
//	s += QString("%1%2%3.html").arg(config.sBaseName.ToString()).arg(ID).arg(Languages::abbrev[language] );
	s += NameFromID(ID, language);
	return s;
}

/*============================================================================
* TASK: returns base name of album w.o. language or  extension
* EXPECTS: none
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::BareName()
{
	return QString("%1%2").arg(config.sBaseName.ToString()).arg(ID & ID_MASK);
}

/*============================================================================
* TASK: Create album name for given ID
* EXPECTS: language -  index of language
*			http - treuL force http(s) prefix
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::NameFromID(ID_t id, int language)
{
	return QString("%1%2%3.html").arg(config.sBaseName.ToString()).arg(id & ID_MASK).arg(Languages::abbrev[language]);
}

/**************************** AlbumMap *****************************/

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
ID_t AlbumMap::Add(QString path, bool &added)
{
	Album ab;

	Album &found = Find(path);		// already in database?
	added = false;
	if (found.Valid())
		return found.ID;	// same base ID, same name then same album
	added = true;
	SeparateFileNamePath(path, ab.path, ab.name);
	ab.ID = GetUniqueID(*this, path, false) + ALBUM_ID_FLAG;   // using full path name only
	ab.exists = QFileInfo::exists(path);
	(*this)[ab.ID] = ab;
	return ab.ID;
}

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
* GLOBAL:
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
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
Album& AlbumMap::Find(ID_t id)
{
	if( (id & ALBUM_ID_FLAG)==0) return invalid;		// not an album

	Album ab;
	ab.searchBy = Album::byID;
	ab.ID = id;

	return contains(id) ? (*this)[id] : invalid;
}

/*==========================================================================
* TASK:		add a sub-album or an image to global arrays and to album 'ab'
* EXPECTS:	ab - album to add data to (parent)
*			path - full path name relative to  album root '_root'
*			pos: - put position in images or in albums here
*			folderIcon: if the file should be used as album thumbnail for this album
* RETURNS:	ID of new or already present album or image
* REMARKS:	does not set/modify the title and description IDs
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrAlbum(Album &ab, QString path, bool folderIcon)
{
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
*			fromDisk: - used when elem to be added comes from the disk
*				in this case just new data is added, but the 'exists'
*				flag is set for old data too
* RETURNS:	fromDisk = false (default) id of data
*			fromDisk = true -> 0 for hidden files, or if excluded
*				for this subdirectory otherwise id
* REMARKS:  - 'exists' member of not excluded old oe new images and sub-albums 
*				are set to true when 'fromDisk' is true
*			- adds new images and id's to the corresponding id list of 'ab'
*			- showss progress using frmMain's progress bar
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrAlbum(Album &ab, QFileInfo & fi, bool fromDisk)
{
	ID_t id = 0;
	
	QString s = fi.filePath();
	if (fi.fileName()[0] == '.')			// hidden file?
		return 0;

	bool added;
	if (fi.isDir())
	{
		id = _albums.Add(s, added);		// add new album to global album list
		if (id && fromDisk)
			_albums[id].exists = true;
		if (added)
		{
			ab.albums.push_back(id);	// add to ordered sub-album ID list for this album
			ab.order.push_back(id);		// add to ordered ID list for this album
		}
	}
	else if (IsImageFile(s))
	{
		id = _images.Add(s, added);	// add new image to global image list or get id of existing
		if (id)						// then image existed
		{
			if (added)
			{
				ab.images.push_back(id);	// add to ordered image ID list for this album
				ab.order.push_back(id);		// add to ordered ID list for this album
			}
		}
	}
	// progress bar
	frmMain->SetProgressBarPos(_albums.size(), _images.size() );	  // both changes
	return id;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_IsExcluded(const Album &ab, QString name)
{
	ID_t id;
	for (int i = 0; i < ab.excluded.size(); ++i)
	{
		id = ab.excluded[i];
		if (id & ALBUM_ID_FLAG)
		{
			if (_albums[id].name == name)
				return true;
		}
		else
			if (_images[id].name == name)
				return true;
	}
	return false;
}

/*============================================================================
* TASK:		 if an album does not have a title in a '.info' file then
*				set the album directory name as the default title.
* EXPECTS:
* GLOBAL:
* REMARKS:	use only for albums
*--------------------------------------------------------------------------*/
void AlbumGenerator::_TitleFromPath(QString path, LanguageTextList & ltl)
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

	FileReader fr(path + "albumfiles.txt", FileReader::trim);
	if (!fr.Ok())
		return false;
	QString line;
	QStringList sl;
	int pos;
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

		ID_t id = _AddImageOrAlbum(ab, line);   // to lists (_albums or _images) and (albums or images) and order
		if (excluded)
			ab.excluded.push_back(id);	// this is excluded (needed when reading names from disk)
	}
	return true;
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

	FileReader fr(path + "comments.properties", FileReader::trim);
	if (!fr.Ok())
		return false;
	QString	line,		// <ianame>=<texts>
			ianame;
	QStringList sl;
	ID_t id;
	bool hidden;
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

					// set text to image or album description
		sl = _SeparateLanguageTexts(line);
		int slSize = sl.size();
		bool added;
		ID_t tid = _texts.Add(sl, added);	// same id for all languages
		if ( (tid & INVALID_ID_FLAG) == 0)
			if (id & ALBUM_ID_FLAG)		// description to sub-album
				_albums[id].descID = tid;
			else
				_images[id].descID = tid;
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

	FileReader fr(path + "meta.properties", FileReader::trim);
	if (!fr.Ok())
		return false;

	QString	line,		// <ianame>=<texts>
		ianame;
	QStringList sl;
	int pos,
		slSize = sl.size();
	ID_t id;
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
			id = _AddImageOrAlbum(ab, path + line, true);	// add image or folder
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
			ID_t tid = _texts.Add(sl, added);	  // more languages than texts
			ab.descID = tid;			  // description to sub-album
		}
	}
	return true;
}


/*=================================================================
* TASK: put album and image title texts from a '*.info' files in
*		the '.jalbum' subdirectory of the album's directory into
*		ab and the images inside it
* EXPECTS:	ab - album (already on global album list '_albums')
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
void AlbumGenerator::_AddTitleTo(Album &ab, QString & path, QString name)
{
	QString line;
	FileReader reader(path + ".jalbum/" + name, FileReader::isUtf8 | FileReader::trim);  // name of .info file

	LanguageTextList ltl(Languages::Count());			// set default title here

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
			ab.titleID = _texts.Add(ltl, added);
		}
	}
	else
	{
		QStringList sl;
		sl = _SeparateLanguageTexts(line);
		ID_t tid = _texts.Add(sl, added);	// same id for all languages
		ID_t id;	// album or image
		if (name[0] == '.') //  ".info"	fo actual album
			ab.titleID = tid;
		else			   // image
		{
			name = name.left(name.length() - 5);
			Image &img = _images.Find(path + name);
			if (img.Valid())
				img.titleID = tid;
		}
	}
}

/*=================================================================
* TASK: put album and image title texts from '*.info' files in 
*		the '.jalbum' subdirectory of the album's directory into
*		ab and the images inside it
* EXPECTS:	ab - album (already on global album list '_albums')
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
				_AddTitleTo(ab, path, s);
		}
	}
	return true;
}

/*=================================================================
 * TASK: read a directory tree of images and order them into an album
 *		  hierarchy
 * EXPECTS:		ab - album (already on global album list '_albums')
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
		if( (id = _AddImageOrAlbum(ab, fi, true)) == 0)	// nothing added? (hidden file)
			continue;
		if (ab.excluded.indexOf(id) >= 0)
			continue;
		if ((id & INVALID_ID_FLAG) == 0)		// valid image or album
			if (id & ALBUM_ID_FLAG)		// sub album
			{
				_albums[id].parent.push_back(ab.ID);
				_ReadOneLevel(_albums[id]);
			}
			else
			{
				_images[id].uploadDate = fi.birthTime().date();

			}
		if (!frmMain->IsRunning())
			return;
		QApplication::processEvents();
	}
	_ReadCommentFile(ab);	// descriptions for files and folders in ab
	_ReadInfo(ab);			// titles for album and images inside
	(void)ab.ImageCount();	// removes excluded ID s of 'ab.images'
	(void)ab.SubAlbumCount();	// removes excluded ID s of 'ab.albums'
	ab.excluded.clear();	// do not need it any more
}

/*==========================================================================
* TASK:		reads whole album hierarchy starting at directory 'root'
* EXPECTS: 'root' path name of uppermost source album directory
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::Read(bool justChanges, bool bDisregardStruct, bool dontProcessImages)
{
	_justChanges = justChanges;
	_dontProcessImages = dontProcessImages;

	if (!_justChanges)
	{
		_images.clear();
		_texts.clear();
		_albums.clear();
	}

	QString s = config.dsGallery.ToString() + QString("gallery.struct");
	if (! QFileInfo::exists(s))
		s = config.dsSrc.ToString() + QString("gallery.struct");

	if(!bDisregardStruct && QFileInfo::exists(s))
	{
		try
		{
			return _ReadStruct(s);
		}
		catch (BadStruct)
		{
			_images.clear();
			_texts.clear();
			_albums.clear();

			return _ReadJalbum();
		}
	}
	else
		return _ReadJalbum();
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
* REMARKS: 
*--------------------------------------------------------------------------*/
static bool __CreateDir(_CDirStr &sdir, bool ask) // only create if needed
{										// ask - if does not exist ask what to do
	__SetBaseDirs();
	QDir folder(sdir.ToString());
	if (folder.exists())
		return true;
	if (ask && __CancelCreate(sdir.ToString()))
		return false;
	return QDir().mkdir(sdir.ToString());
}

/*============================================================================
* TASK:	   creates necessary directories
* EXPECTS:
* GLOBAL:	config
* REMARKS: - if directories does not have an absolute path
*				(windows: starting with "drive:\" or "\", linux: starts with '/')
*			 they are created in this hierarchy:
*		config.dsGallery						destination directory
*				config.dsGRoot				gallery root	(may be empty then
all thes below will be in config.dsGallery)
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
	if (!__CreateDir(__DestDir, true))
		return false;

	__SetBaseDirs();

	_CDirStr res;
	res = "res";
	__ResourceDir = __RootDir + res;				// afalco.js, solyoma.js

	if (!__CreateDir(__RootDir, QDir::isAbsolutePath(config.dsGRoot.ToString())))
		return false;
	if (!__CreateDir(__AlbumDir, QDir::isAbsolutePath(config.dsAlbumDir.ToString())))
		return false;
	if (!__CreateDir(__RootDir + config.dsCssDir, QDir::isAbsolutePath(config.dsCssDir.ToString())))
		return false;
	if (!__CreateDir(__RootDir + config.dsImageDir, QDir::isAbsolutePath(config.dsImageDir.ToString())))
		return false;
	if (!__CreateDir(__RootDir + config.dsFontDir, QDir::isAbsolutePath(config.dsFontDir.ToString())))
		return false;
	if (!__CreateDir(__RootDir + res, false))
		return false;
	return true;
}

/*==========================================================================
* TASK:		reads language texts from structure
* EXPECTS: reader - opened, flags set: isUtf8
* RETURNS: success code
* REMARKS: throws on error
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_LanguageFromStruct(FileReader & reader)
{
	QString line = reader.ReadLine(); // [Language count: X: 
	if (line.left(16) != "[Language count:")
		throw BadStruct();

	int langcnt = line.mid(16).toInt();
	QStringList sl;
	Languages::Clear(langcnt);		// allocates this many empty strings in each string list
	for (int i = 0; i < langcnt; ++i)
	{
		reader.ReadLine();	// language index must be in order therefore do not use

		sl = reader.ReadLine().split('=');
		Languages::abbrev[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::names[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::icons[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::toImages[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::toAlbums[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::toHomePage[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::toAboutPage[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::toContact[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::showCaptions[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::share[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::latestTitle[i] = sl[1];
		sl = reader.ReadLine().split('=');
		Languages::latestDesc[i] = sl[1];
	}
	reader.ReadLine();		// drop next language index or closing ']'
	if(line[0] == ']')
		throw BadStruct();

	return true;
}

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

/*==========================================================================
* TASK:		reads text, description and thumbnail IDs from structure file
* EXPECTS: reader - open file reader with the last read line in it
*					whic after 'level' number of spaces starts with a '[' 
*					and ends with a ']'
*					i.e. it is a title, description or thumbnail line
* RETURNS: nothing
* REMARKS: - reads lines until the first line without a starting '[' is read
*				then returns
*			- before returning it puts the last language texts into _texts
*--------------------------------------------------------------------------*/
void AlbumGenerator::_GetIDsFromStruct(FileReader &reader, IdsFromStruct & ids, int level)
{
	Text texts(Languages::Count());

	// order: title lines then description then thumbnail
	// any and all of these may be missing
	// ALL TAGS MUST BE OF THE SAME LENGTH!
	// and all tags must start with a different letter
	do
	{
		int lang = (reader.l()[level + 1] == THUMBNAIL_TAG[0] ? -1 : Languages::abbrev.indexOf(reader.l().mid(level + TITLE_TAG.length()+1, 2)));

		if (reader.l()[level + 1] == THUMBNAIL_TAG[0])
		{
			if (ids.what != IdsFromStruct::nothing && !texts.IsEmpty())	// there were title OR description lines
			{
				_texts[texts.CalcID()] = texts;
				if (ids.what == IdsFromStruct::title)
					ids.titleID = texts.ID;
				else
					ids.descID = texts.ID;
			}
			int len = (reader.l().length() - 1) - level - THUMBNAIL_TAG.length() - 2; // cut ending ']'
			ids.thumbnailID = reader.l().mid(level + THUMBNAIL_TAG.length() + 2, len).toULongLong();
			texts.Clear();
			ids.what = ids.nothing;
		}
		else  if (reader.l()[level + 1] == DESCRIPTION_TAG[0])	// description line comes after the title lines
		{
			if (ids.what != IdsFromStruct::description && !texts.IsEmpty())	// there were title lines
			{
				_texts[texts.CalcID()] = texts;
				ids.titleID = texts.ID;
				texts.Clear();
			}
			ids.what = ids.description;
			int len = reader.l().length() - 11;			// from after : till ending ']'
			texts[lang] = reader.l().mid(level + 10, len);
		}
		else if (reader.l()[level + 1] == TITLE_TAG[0])	// title line,omes first: ids.what must be 'nothing'
		{
			ids.what = ids.title;
			int len = reader.l().length() - 11;			// from after : till ending ']'
			texts[lang] = reader.l().mid(level + 10, len);
		}
			   // reads next line aand repeat loop
	} while (!reader.NextLine().isEmpty() && reader.l()[level] == '[');

	if ( (reader.l().isEmpty() || reader.l()[level] != '[') && !texts.IsEmpty())
	{
		_texts[texts.CalcID()] = texts;
		ids.titleID = texts.ID;
		texts.Clear();
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
QStringList __ImageStructLineToList(QString s)
{
	QStringList sl;
	int pos0 = 0, pos;
	do
	{
		pos = s.indexOf("(", pos0);
		pos0 = pos + 1;
	} while (pos > 0 && !IsImageFile(s.mid(0, pos)));	// then ( in file name
	if(pos < 0)
		return sl;
	
	sl.push_back(s.mid(0, pos));		// file name

	pos0 = pos + 1;	   		// after the opening brace
	pos = s.indexOf(')',pos0);
	if (pos < 0)
		return sl;

	sl += s.mid(pos0, pos - pos0).split(',');	// ID, length, date
	if (sl.size() != 4 || s[pos + 1] != '/')
		return sl;
	sl.push_back(s.mid(pos + 2));
	return sl;
}

/*==========================================================================
* TASK:		reads one image from structure
* EXPECTS: reader - open file reader, 
*			level -  the level of images
* RETURNS: success code
* REMARKS: - first image line: <spaces><file name> (<file id>) // <original path
*		   - throws 'BadStruct' on error
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ImageFromStruct(FileReader & reader, int level)
{
	Image img;
/*
	Does not work, because file name and path both may contain braces and other
	inconvenient letters
	QRegExp regexp("(\\()|,|(\\))|(\\)/)");
	QStringList sl = reader.l().mid(level).split(regexp);	// separate img definition
*/
	QStringList sl = __ImageStructLineToList(reader.l().mid(level));
	int n = sl.size();		// should be 4 image name, id, date, size, path
	if (n != 5)
		throw BadStruct();

	ID_t id;

	img.name = sl[0];
	img.ID = id = sl[1].toULongLong();
	img.uploadDate = QDate(sl[2].mid(0,4).toInt(), sl[2].mid(5, 2).toInt(), sl[2].mid(8, 2).toInt());
// DEBUG:
	if (img.uploadDate.toJulianDay() < 0)
		return 0;

	img.fileSize = sl[3].toULongLong();
	img.path = config.dsSrc.ToString() + sl[4];
	img.exists = QFileInfo::exists(config.ImageDirectory().ToString() + img.FullName());

	IdsFromStruct ids;
	reader.NextLine();
	if(reader.l()[level] == '[')
		_GetIDsFromStruct(reader, ids, level);
	img.titleID = ids.titleID;
	img.descID = ids.descID;

	_images[id] = img;

	return img.ID;
}

/*==========================================================================
* TASK:		helper - splits an album definition line intothe 3 constituents:
*				name, ID,  and path
* EXPECTS: s : string of definition w.o. leading spaces
* RETURNS: list of fields found
* REMARKS: - "normal" regular function split() could not be used as names
*					may contain braces and commas
*			result may contain fiewer or more fields than required
*--------------------------------------------------------------------------*/
QStringList __AlbumStructLineToList(QString s)
{
	QStringList sl;
	int pos0 = 0, pos;
	do
	{
		pos = s.indexOf("(", pos0);
		pos0 = pos + 1;
	} while (pos > 0 && s.mid(pos+1, 2)!= "A:");	// then ( in file name
	if (pos < 0)
		return sl;

	sl.push_back(s.mid(0, pos));			// album name

	pos0 = pos + 1;	   		// after the opening brace
	pos0 += 2;				// skip the 'A:'
	pos = s.indexOf(')', pos0);
	if (pos < 0)
		return sl;

	sl.push_back(s.mid(pos0, pos - pos0));	// ID
	sl.push_back(s.mid(pos + 2));			// path
	return sl;
}

/*==========================================================================
* TASK:		recursively reads album and all of its sub-albums  from structure
* EXPECTS: reader - open file reader,
*			reader.l()  - album definition line "" +"<name>(A:id)'/'original path"
* RETURNS: success code
* REMARKS:	- album definitions are delimited with a single empty line
*			- each album line starts with as many spaces as its level
*			- if the next album definition is on the same level
*			- as this album, then returns
*			- expects a single root album
*			- albums are added to '_albums' as soon as they are identified
*				and that record is modified iater by adding the images and 
*				albums to it
*		   - throws 'BadStruct' on error
*			- the album created here must be added to _albums in caller
*				therefore it will be addwd later than its sub albums
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AlbumFromStruct(FileReader &reader, int level)
{
	Album album;	// temporary album to add to map

	QStringList sl = __AlbumStructLineToList(reader.l().mid(level));
	int n = sl.size();		// should be 3: album name, id and path
	if (n != 3)
		throw BadStruct();

	ID_t id;
	album.name = sl[0];
	album.ID = id = sl[1].toULongLong() + ALBUM_ID_FLAG; 
	album.path = (config.dsSrc + sl[2]).ToString();
	album.exists = QFileInfo::exists( album.FullName());

	IdsFromStruct ids;

	reader.ReadLine(); // nect line after an album definition line
					   // when empty same level or sub albums follows
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
			reader.NextLine();					// next album definition line
			while (reader.Ok() && reader.l()[level] == ' ')		// new sub album
			{													// process it 
				ID_t aid = _AlbumFromStruct(reader, level + 1); // returns when same level sub album is found
				album.albums.push_back(aid);
			}
			_albums[id] = album;
			return id;			 // this new album definition is at the same level as we are
		}
		else
		{				// strings in lText must be empty
			if (reader.l()[level] == '[') // then title, description or thumbnail line
			{		
				_GetIDsFromStruct(reader, ids, level);
			}
			else if(reader.Ok())		   // this must be an image line, unless file is ended
			{							   // but image files are inside the album (one level down)
				ID_t iid = _ImageFromStruct(reader, level+1);
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
	if(!_albums.contains(id))
		_albums[id] = album;

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
		FileReader reader(from, FileReader::isUtf8);		// UTF-8, no empty lines, no trimming
		if (!reader.Ok())
			throw BadStruct();

		QStringList sl;
		QString line = reader.NextLine();	// comment line is OK

		if (line != versionStr)
			throw BadStruct();
		else
		{
			_LanguageFromStruct(reader);  // throws when error
			frmMain->SetupLanguagesToUI();

			sl = reader.ReadLine().split('='); // Source=....
			config.dsSrc = sl[1];
			sl = reader.ReadLine().split('='); // destination=....
			config.dsGallery = sl[1];
			// from now on we need the empty lines as well
			reader.NextLine();		// empty line above album definition
			if (!reader.l().isEmpty())
				throw BadStruct();
			reader.NextLine();		// album definition line
					// recursive album read. there is only one top level album
			ID_t rootID = _AlbumFromStruct(reader, 0); 
			if (error)
				throw BadStruct();
		}
		return true;
	}
	catch (BadStruct)
	{
		QMessageBox(QMessageBox::Critical, QMainWindow::tr("falconG - Error"), QMainWindow::tr("Damaged structure file! Reading aborted.\n"
																								"Trying to read directory (jalbum) structure"), QMessageBox::Ok, frmMain).exec();
		throw(BadStruct());
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
	frmMain->SetupLanguagesToUI();
			// get number of sub albums
	QDirIterator dirIter(config.dsSrc.ToString(), QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);	// count image directories
	int directoryCount = 0;												// iterator points BEFORE first entry
	QString s;
	while (dirIter.hasNext())
	{
		QFileInfo fi(dirIter.next());
		if (fi.isDir())
		{
			s = dirIter.fileName();
			if (s[0] != '.' && s != "res" && s != "cache" && s != "thumbs")	// not a hidden or resource directory?
				++directoryCount;
		}
	}

	frmMain->SetDirectoryCountTo(directoryCount);
	frmMain->SetProgressBar(0, directoryCount, 0, 0);	// phase 0

	_justChanges = (_justChanges && (albumgen.AlbumCount() == 0 || albumgen.ImageCount() == 0)); // else full creation

	QString root = config.dsSrc.ToString();

	QDir dir(root);


	_root.Clear();
	_root.ID = 1;		// id = 0: invalid
	_root.path = config.dsSrc.ToString();
	// root has no name and no path it is inside config.dsSrc
	_root.exists = true;

	_albums[1] = _root;	// a copy of _root is first on list	

	_ReadOneLevel(_albums[1]);
	if (!frmMain->IsRunning())
		return false;

	_root = _albums[1];	 // get values stored in hierarchy back to _root
	return true;
}

/*============================================================================
* TASK:	Creates constants.php		-   - " -
* EXPECTS:	config fields are set
* GLOBAL:	'config'
* RETURNS: 0: OK, 1: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoCopyRes()
{
	QString src = QDir::current().path() + "/res/",
			dest = (config.dsGallery + config.dsGRoot).ToString() + "res/";
	QDir dir(src);  // res in actual progam directory
	QFileInfoList list = dir.entryInfoList(QDir::Files);
	for (QFileInfo &fi : list)
		QFile::copy(fi.absoluteFilePath(), dest + fi.fileName());
	return 0;	
}

enum _What : int { wNone, wColor, wBackground, wNoClosingBrace = 0x8000 };
/*===========================================================================
* TASK: write shadow parameter
* EXPECTS: ofs			- text stream
*			s			- name of shadow (e.g. "text-shadow, -moz-box-shadow)
*			elem		- name from CONFIG
* GLOBALS: config, ofs
* RETURNS: none
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_ShadowToString(QString s, _CElem &elem)
{
	if (!elem.shadow.use)
		return QString();

	s += QString().setNum(elem.shadow.Horiz()) + " " + QString().setNum(elem.shadow.Vert());
	if (elem.shadow.Blur())
		s += " " + QString().setNum(elem.shadow.Blur());
	s += " " + elem.shadow.color + ";\n";
	return s;
}

/*============================================================================
* TASK:		gets image ID for album thumbnail
* EXPECTS:
* GLOBAL:
* REMARKS: if the album thumbnail is an other album then its thumbnail
*			will be the same image as the thumbnail of that album
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ThumbnailID(Album & album)
{
	if (album.thumbnail == 0) // then thumbnail is the first image in the album
	{
		if(album.images.isEmpty() && album.albums.isEmpty())
			return 0;
		if (album.images.isEmpty()) // then get thumbnail for first sub-album
			return _ThumbnailID(_albums[album.albums[0]]);
		else
			return album.images[0];
	}
	if (album.thumbnail & ALBUM_ID_FLAG)
		return _ThumbnailID(_albums[album.thumbnail]);
	return album.thumbnail;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteStructLanguageTexts(QString what, ID_t id, QString indent)
{
	if (id == 0)		// no text
		return;

	if (!_texts.contains(id) || _texts[id].texts.size() < Languages::Count())
	{
		// DEBUG
		qDebug() << "id:" << id << ", what:" << what << "present? " << (_texts.contains(id) ? "yes" : "no") << '\n';
		return;
	}

	for (int i = 0; i < Languages::Count(); ++i)
		ofs << indent << "[" << what << Languages::abbrev[i] << ":"
		<< _texts[id].texts[i] << "]\n";
}

/*============================================================================
* TASK:		writes out the new album structure.
* EXPECTS:
* GLOBAL:
* REMARKS: for every album first there come the images, then the sub albums:
*             album
*                image 1  in album
*					...
*                album 1
*					image 1 in album 1
*						...
*					a;bum 2 in album 1
*					***
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteStructSubAlbumsAndImages(Album & album, QString indent)
{
	QString s;
	int len = config.dsSrc.Length();
	Image *pImg;

	for (ID_t id : album.images)
		if (id && album.excluded.indexOf(id) < 0)		// not excluded
		{
			pImg = &_images[id];
			if (pImg->exists)
			{
				s = pImg->path; 
				ofs << indent
					<< pImg->name << "(" << pImg->ID << "," 
											// ISO 8601 extended format: either yyyy-MM-dd for dates or 
					<< pImg->uploadDate.toString(Qt::ISODate)
					<< ","
					<< pImg->fileSize
					<< ")/";
				if (s.left(len) == config.dsSrc.ToString())
					s = s.mid(len);
				ofs << s << "\n";
				_WriteStructLanguageTexts(TITLE_TAG, pImg->titleID, indent);
				_WriteStructLanguageTexts(DESCRIPTION_TAG, pImg->descID, indent);
			}
		}
	for (ID_t id : album.albums)
		if (id && album.excluded.indexOf(id) < 0)		// not excluded
				_WriteStructAlbums(_albums[id], indent);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteStructAlbums(Album& album, QString indent)
{
	if (!album.exists)
		return;

	QString s = album.path;	// album.FullName()  +'/';
	int len   = config.dsSrc.Length(); 
	if (s.left(len) == config.dsSrc.ToString())	// drop sorce path ending '/'
		s = s.mid(len);				// so root album will have a path of '/'

	ID_t thumbnail = album.thumbnail = _ThumbnailID(album);		// thumbnail may have been a folder
																// name  originally, now it is an ID
	ofs << "\n" << indent
		<< album.name << "(A:" << (album.ID & ~ALBUM_ID_FLAG) << ")/"	<< s << "\n";
	_WriteStructLanguageTexts(TITLE_TAG, album.titleID, indent);
	_WriteStructLanguageTexts(DESCRIPTION_TAG, album.descID, indent);
	Image &img = _images.Find(thumbnail);
	if (img.Valid())
	{
		ofs << indent << "[" << THUMBNAIL_TAG << ":" <<  thumbnail << "]\n";
	}
	_WriteStructSubAlbumsAndImages(album, indent + " ");
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::WriteDirStruct()
{
	QString s = QString(config.dsGallery.ToString()) + "gallery.struct",
		stmp = QString(config.dsGallery.ToString()) + "gallery.tmp";
	QFile f(stmp);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		// error
		return 0;
	}
	ofs.setDevice(&f);
	ofs.setCodec("UTF-8");

	ofs << versionStr << "\n#  © - András Sólyom (2018)\n"
		<< "[Language count:"	<< Languages::Count() << "\n";
	for (int i = 0; i < Languages::Count(); ++i)
	{
		ofs << i << "\n"
			<< "  abbrev=" << Languages::abbrev[i] << "\n"
			<< "  name=" << Languages::names[i] << "\n"
			<< "  icon=" << Languages::icons[i] << "\n"
			<< "  images=" << Languages::toImages[i] << "\n"
			<< "  albums=" << Languages::toAlbums[i] << "\n"
			<< "  homePage=" << Languages::toHomePage[i] << "\n"
			<< "  about=" << Languages::toAboutPage[i] << "\n"
			<< "  contact=" << Languages::toContact[i] << "\n"
			<< "  captions=" << Languages::showCaptions[i] << "\n"
			<< "  share=" << Languages::share[i] << "\n"
			<< "  latestTitle=" << Languages::latestTitle[i] << "\n"
			<< "  latestDesc=" << Languages::latestDesc[i] << "\n"
			;
	}
	ofs << "]\n\n# Album structure:\n"
		<< "Source=" << config.dsSrc.ToString()
		<< "\nDestination=" << config.dsGallery.ToString() << "\n";

	QString indent;			// for directory structure file: indent line
	_WriteStructAlbums(_albums[1], indent);

	f.close();
	BackupAndRename(s, stmp, frmMain);
	_WriteImageTranslations(); //into destination imgae directory
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::_WriteImageTranslations()
{
	QString sDestPath = (config.dsGallery + config.dsGRoot + config.dsImageDir).ToString();
	QFile f( sDestPath + "imgnames.tmp");
	if (!f.open(QIODevice::WriteOnly))
		; // TODO error message
	ofs.setDevice(&f);
	QString indent = "  ";
	for (auto im : _images)
	{
		if (!im.ID)	// invalid
			continue;
		im.WriteTranslation(ofs);
		_WriteStructLanguageTexts(TITLE_TAG, im.titleID, indent);
		_WriteStructLanguageTexts(DESCRIPTION_TAG, im.descID, indent);
	}
	f.close();
	BackupAndRename(sDestPath + "imgnames.txt", sDestPath + "imgnames.tmp", frmMain);
}

/*===========================================================================
* TASK: write color and background css with shadow
* EXPECTS: ofs			- text stream
*			selector	- CSS selector
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
		selector += _ShadowToString(" text-shadow", elem);
	if ((what & wNoClosingBrace) == 0)
		selector +="}\n\n";
	return selector;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
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
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_MenuColorCSSToString()
{
	QString s;
	// state: UP
	s = "p.menu-line a {\n";
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
		"p.menu-line a:hover {\n";
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
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_ColorCSSToString()
{
	QString s;

	s = "/* Header Style  first for mobile then for tablet and desktop*/\n" +
		_ElemColorCssToString("html,\nbody,main", config.Web, wBackground) +
		_ElemColorCssToString(".falconG", config.SmallGalleryTitle, wColor) +
		_ElemColorCssToString("h2.title,a.title,a:visited.title", config.GalleryTitle, wColor) +
		_ElemColorCssToString(".album-title", config.AlbumTitle, wColor) +
		_ElemColorCssToString(".album-desc", config.AlbumDesc, wColor) +
		_ElemColorCssToString("h2.desc", config.AlbumDesc, wColor) +
		_ElemColorCssToString("a, a:visited", config.Menu, wColor) +
		_ElemColorCssToString("a.doboz", config.Lang, wColor) +
		_ElemColorCssToString("a[name=\"images\"],a[name=\"galleries\"]", config.Section, wColor) +
		_ElemColorCssToString("#folders p", config.AlbumDesc, wColor) +
		_ElemColorCssToString("#images p", config.ImageDesc, wColor) +

		// block for menu buttons
		_MenuColorCSSToString() + 
		// end of menu button

		_ElemColorCssToString("#images, #folders", config.images, wColor);
	if (config.imageBorder.used)
		s += "section div.thumb img{\n 	border:" + QString().setNum(config.imageBorder.width) + "px solid " + config.imageBorder.color.name + ";\n}\n";
// DEBUG
//	QMessageBox(QMessageBox::Information, "falconG - info", s, QMessageBox::Ok, frmMain).exec();

	return s;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
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
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_CssToString()
{
	QString s =
		"@import url(\"https://fonts.googleapis.com/css?family=Raleway\"); /*--- Nav Font --*/\n"
		"@import url(\"https://fonts.googleapis.com/css?family=Playfair+Display:700|Raleway\"); /*--- Heading Font --*/\n"
		// TODO: scan fobt directory and embed fonts from it into here
		// e.g. @font-face {
		//				font-family: 'RieslingRegular';
		//				src: url('fonts/riesling.eot');
		//				src: local('Riesling Regular'), local('Riesling'), url('fonts/riesling.ttf') format('truetype');
		//		}
		// usage example: h1 {font-family: 'RieslingRegular', Arial, sans-serif;}
		"html,\n"
		"body{\n"
		"	margin: 0;\n"
		"	padding: 0;\n"
		"	height: 100% ;\n"
		"	width: 100% ;\n"
		"}\n"
		"\n"
// .falconG
		".falconG{\n" 
		+ _FontToCss(config.GalleryTitle) + 
		"	margin-left: 10px;\n"
		"}\n"
		"\n"
// h2
		"h2{\n" // only for gallery title else have overriding class
		+ _FontToCss(config.GalleryTitle) +
		"	text-align: center;\n"
		"}\n"
		"\n"
// a.title
		"a.title{\n"
		+ _FontToCss(config.GalleryTitle) +
		"}\n"
		"\n"
		// a, a:visited
		"a, a:visited{\n"
		"	text-decoration: none;\n"
		"	/*padding: 0 4px 2px 4px;*/\n"
		"}\n"
		"\n"
// a.jump_link
		"a.jump_link{\n"
		"	text-decoration: none;\n"
		"	padding: 0 4px 2px 4px;\n"
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
// p.menu-line a
		"p.menu-line a{\n"
		+ _FontToCss(config.Menu) +
		"	line-height: 18px;\n"
		"	border-radius: 5px;\n"
		"	box-shadow: 0 1px 1px rgba(0, 0, 0, 0.1);\n"
		"	display: inline-block;\n"
		"	margin: 2px 3px 2px 10px;\n"
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
// Album title
		".album-title{\n"
		+ _FontToCss(config.AlbumTitle) +
		"}\n"
// album-desc
		".album-desc{\n"
		+ _FontToCss(config.AlbumDesc) +
		"	margin:auto;\n"
		"	width:90% ;\n"
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
		"#images, #folders{\n"
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
		"	justify-content: stretch;\n"
		"}\n"
		"\n"
// .desc
		".desc{\n"
		"	display:none;\n"
		"	hyphens: auto;\n"
		"	-moz-hyphens: auto;\n"
		"	-webkit-hyphens: auto;\n"
		"	-ms-hyphens: auto;\n"
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
		"	display:flex; justify-content: center;\n"
		"	flex-wrap: wrap;\n"
		"	flex-direction: row;\n"
		"	padding: 2px;\n"
		"}\n";
		if (config.imageBorder.used)
			s += "div.thumb a img{\n"
			" border :" + QString("%1px solid %2").arg(config.imageBorder.width).arg(config.imageBorder.color.name, 0, 16)
			+ "}\n";
		s += "\n"
		".img-container img, .gallery-container img{\n"
		"	margin:auto;\n"
		"	max-width: 100% ;\n"
		"}\n"
		"\n"
		"div.links{\n"
		"	display:flex;\n"
		"	width:100% ;\n"
		"	justify-content:space-between;\n"
		"	margin-bottom: 10px;\n"
		"	padding-bottom: 10px;\n"
//		"	border-bottom: 1px solid black;\n"
		"}\n"
		"div.links a, .showhide{\n"
		"	text-decoration:none;\n"
		"	font-family:\"Playfair Display\", sans-serif, Arial;\n"
		"	font-weight:bold;\n"
		"}\n"
		"\n"
		"div.links a:hover, .showhide:hover{\n"
		"	font-weight: 700;\n"
		"}\n"
		"\n"
		"	/* -- media queries ***/\n"
		"@media only screen and (orientation: landscape) {\n"
		"	img{ max-height: 95vh; }\n"
		"}\n"
		"@media only screen and (min-width:700px) {\n"
		"	.img-container, .gallery-container\n"
		"	{\n"
		"		max-width:250px;\n"
		"		justify-content: flex-end;\n"
		"		padding:0 3px 0 3px;\n"
		"	}\n"
		"	img{ max-width: 100% ; max-height:267px; }\n"
		"		p{ font-size: 12pt }\n"
		"		main{ display:flex; flex-direction: column; }\n"
		"}\n"
		"   \n"
		"	/* large screens */\n"
		"@media only screen and (min-width:1200px) {\n"
		"	.img-container, .gallery-container\n"
		"	{\n"
		"		max-width:400px;\n"
		"		justify-content: flex-end;\n"
		"		flex-direction: column;\n"
		"		padding:0 3px 0 3px;\n"
		"	}\n"
		"	img{ max-width: 100% ; max-height:267px; }\n"
		"		p{ font-size: 12pt }\n"
		"	main{ display:flex; flex-direction: column; }\n"
		"}\n"
		"\n";
	return s;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
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
* TASK:		Creates colors.css		-  from design parameters
* EXPECTS:	config fields are set
* GLOBAL:	'config'
* RETURNS: 0: OK, 4: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoColorsCss()
{
	// generate new colors.css
	QFile fcssC(QString(__CssDir.ToString()) + "tmpcolor.css");
	if(!fcssC.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return 4;

	ofs.setDevice(&fcssC);
	ofs << _ColorCSSToString();
	fcssC.close();

	BackupAndRename(QString(__CssDir.ToString()) + "colors.css", QString(__CssDir.ToString()) + "tmpcolor.css", frmMain);
	return 0;
}

/*============================================================================
* TASK:		falconG.css		-  from all parameters
* EXPECTS:	config fields are set
* GLOBAL:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoStyleFG()
{
	// generate new falconG.css into temporary files
	QFile fcssG(QString(__CssDir.ToString()) + "tmpfg.css");
	if (!fcssG.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return 8;

	ofs.setDevice(&fcssG);
	ofs << _CssToString();
	fcssG.close();

	BackupAndRename(QString(__CssDir.ToString()) + "falconG.css", QString(__CssDir.ToString()) + "tmpfg.css", frmMain);
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
QString AlbumGenerator::_PageHeaderToString()
{
	QString s = QString("<!DOCTYPE html>\n"
		"<html lang = \"" + Languages::abbrev[_actLanguage] + "\">\n"
		"<head>\n");
	if(config.googleAnalyticsOn)
		s += QString("<!--Global site tag(gtag.js) - Google Analytics-->\n"
					 "<script async src=\"https://www.googletagmanager.com/gtag/js?id=%1\"></script>\n"
					"<script>\n"
					"window.dataLayer = window.dataLayer || [];\n"
					"function gtag() { dataLayer.push(arguments); }\n"
					"gtag('js', new Date());\n"
					"gtag('config', '%1');\n"
					"</script>\n").arg(config.googleAnalTrackingCode);

	s += QString("<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">"
		"<title>" + config.sGalleryTitle + "</title>\n"
		"<link rel=\"stylesheet\" href=\"../" + config.dsCssDir.ToString() + "colors.css\">\n"
		"<link rel=\"stylesheet\" href=\"../" + config.dsCssDir.ToString() + "falconG.css\">\n"
		"<!--get facebooks js code-->\n"
		"<script>(function(d, s, id) {var js, fjs=d.GetItementsByTagName(s)[0];	if(d.GetItementById(id))return;"
		"js=d.createElement(s); js.id=id; js.src = \"//connect.facebook.net/hu_HU/sdk.js#xfbml=1&version=v2.8\";"
		"fjs.parentNode.insertBefore(js, fjs);}(document,'script','facebook-jssdk')); </script>\n\n"
		"<script src=\"../res/falconG.js\" type=\"text/javascript\"></script>\n"
		"</head>\n");
	return s;
}

/*============================================================================
* TASK: emits header section of a web page for agiven album in the given _actLanguage
* EXPECTS:	album - album to be created  
*			_actLanguage - index of actual _actLanguage
* GLOBALS:	Languages, 'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_OuputHeaderSection(Album &album, const QString parent)
{
	ofs << "<header>\n<span class=\"falconG\">" << config.sGalleryTitle << "</span>&nbsp; &nbsp;";
	// _actLanguage switch texts
	for (int i = 0; i < Languages::Count(); ++i)
		if (i != _actLanguage)
			ofs << "<a class=\"doboz\" href=\"" + album.LinkName(i) + "\">"   << Languages::names[i] << "</a>&nbsp;&nbsp;";
	// facebook link
	ofs << "<span class=\"fb-share-button\" style=\"float:right; color:white\" data-href=\"" << album.LinkName(_actLanguage, true)
		<< "\" data-layout=\"button_count\" data-mobile-iframe=\"true\">\n"
		   "<a class=\"fb-xfbml-parse-ignore facebook\" target=\"_blank\" href=\"https://www.facebook.com/sharer/sharer.php?u=http%3A%2F%2F"
		<< album.LinkName(_actLanguage, true)
		<< "%2F&amp;src=sdkpreparse\">\n"
		   "<img src=\"../res/facebook.png\" width=\"24px\" height=\"24px\" valign=\"center\">" << Languages::share[_actLanguage] << "</a>\n</span>\n";
	// menu buttons
	ofs << "<p class=\"menu-line\">\n"
		   "<a href=\"" << parent << "\"><img src=\"../res/up-icon.png\" style=\"height:14px;\"></a>\n"			  // UP link
	       "<a href=\"" << _root.LinkName(_actLanguage, true) << "\">" << Languages::toHomePage[_actLanguage] << "</a>\n";	// Home
	if(album.SubAlbumCount())
		   ofs << "<a href='#folders'>" << Languages::toAlbums[_actLanguage] << "</a>\n";								  // to albums
	ofs << "<a href=\"" << config.sAbout << "\">" << Languages::toAboutPage[_actLanguage] << "</a>\n"
			"<a href=\"mailto:" << config.sMailTo << "\">" << Languages::toContact[_actLanguage] << "</a>\n"
			"<a href=\"#\" onclick=\"javascript:ShowHide()\">" << Languages::showCaptions[_actLanguage] << "</a>\n";
	if (config.generateLatestUploads)
		ofs << "<a href=\"" << config.dsGRoot.ToString() << "latest_" + Languages::abbrev[_actLanguage] << ".php\">" << Languages::latestTitle[_actLanguage] << "</a>\n";

	ofs << "</p>\n";
	//// debug
	//"<p style=\"margin-left:10px; font-size:8pt;font-family:Arial;\"id=\"felbontas\"></p>  <!--debug: display screen resolution-->
	ofs << "<br>\n";
	if(album.titleID)
		ofs << "<h2 class=\"album-title\">" << _texts[album.titleID][_actLanguage] << "</h2>\n";
	if (album.descID)
		ofs << "<p class=\"album-desc\">" << _texts[album.descID][_actLanguage] << "</p>\n";
	ofs << "</header>\n";
	return 0;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteFooterSection(const Album & album)
{
	ofs << " <!-- footer section with social media links -->\n"
		"<footer class = \"footer\">";
		// TODO
	ofs << "</footer>\n";
	return 0;
}

/*============================================================================
* TASK:		writes eiter an image or a gallery section
* EXPECTS:	album - actual album whose image or sub-album is written
* GLOBAL: config, Languages
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteGalleryContainer(const Album & album, bool itIsAnAlbum, int i)
{
	const IdList &idList = (itIsAnAlbum ? album.albums : album.images);
	ID_t id = idList[i];  
	QString title, desc, sImageDir, sImagePath;

	ofs << "<section class=\"img-container\">\n"
			"   <div class=\"thumb\">\n"
		"   <a href=\"";
	sImageDir = (QDir::isAbsolutePath(config.dsImageDir.ToString()) ? "" : "../") + config.dsImageDir.ToString();
	if (itIsAnAlbum)
	{
		title = _texts[_albums[id].titleID][_actLanguage];
		desc = _texts[_albums[id].descID][_actLanguage];
		ID_t thumb = _albums[id].thumbnail;
		sImagePath = thumb ? sImageDir + _images[thumb].LinkName() : QString("---%1---").arg(thumb);
		ofs <<  _albums[idList[i]].LinkName(_actLanguage);	// albums are in the same directory
	}
	else
	{
		title = _texts[_images[id].titleID][_actLanguage];
		desc = _texts[(_images[id].descID)][_actLanguage];
		sImagePath = (album.images.size() > 0 ? (sImageDir + _images[album.images[i]].LinkName()) : QString());
		ofs << sImagePath;		// image in the image directory
	}

	ofs << "\">\n <img src=\"" + sImagePath + "\" alt=\"\">\n"
		<<	"</a>\n"
			"</div>\n"
			"<div class=\"links\">\n"
			"<a href=\"#top\"><img src=\"../res/up-icon.png\" style=\"height:14px;\"></a>\n"
			"<a class=\"title\" href=\"";
	if (itIsAnAlbum)
		ofs << _albums[idList[i]].LinkName(_actLanguage) + "\">";
	else
		ofs << (sImagePath.isEmpty() ? "#" : sImagePath) + "\">";
	ofs << (title.isEmpty() ? "---" : title)
		<< "</a>\n"
		"<div class=\"showhide\" onclick=\"ShowHide()\"><img src=\"../res/content-icon.png\" style=\"height:32px;\"></div>\n"
			"</div>\n"
		"<div class=\"desc\">\n"
		"<p lang=\"" << Languages::abbrev[_actLanguage] << "\">"
		<< (desc.isEmpty() ? "&nbsp;" : desc) << "</p>\n"
		"</div>\n"
		"</section>\n";

	return 0;
}

/*============================================================================
* TASK: gets images from source directories resize them if needed, then 
*	optionally add watermark. Puts all images in image directory, plus 
*	 creates a list of the images processed in the image directory
* EXPECTS:	_images,
*			_actLanguage - index of actual language
* GLOBALS:	'config'
* RETURNS: 0: OK, 8: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_ProcessImages()
{
	// progress bar
	frmMain->SetProgressBar(0, _images.size(), 0, 1); // phase = 1
	int cnt = 0;	// count of images copied

	bool doProcess = true;	// only process image if this flag is set

	for (auto im : _images)
	{
		if (!frmMain->IsRunning())		// stopped?
			return 8;
		if (im.name.isEmpty())
			continue;

		// resize and copy
		// TODO: watermark
		QString src = (config.dsSrc + im.path).ToString() + im.name,   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)
			    dst = config.ImageDirectory().ToString() + im.LinkName();

		QFileInfo fi(src);						// test for source image
		bool srcExists = fi.exists();

		if (!_dontProcessImages)		// except get creation time
		{
			bool dstExists = QFile::exists(dst);

			if (!srcExists)
			{
				if (dstExists)						// source file was deleted
					QFile::remove(dst);				// delete
				continue;
			}

			if (dstExists && config.bJustChanges)		// then test if this image was modified 
			{
				QDate dt = fi.birthTime().date();
				if (dt <= im.uploadDate &&		// not newer and
					fi.size() == im.fileSize)   // same size  (TODO: checksum)
					doProcess = false;			// then do not process
			}
			if (doProcess)
			{
				//		int btn;
						//if (!QFile::exists(dst) && !config.bOvrImages)
						//	if ((btn = QMessageBox(QMessageBox::Warning, QMainWindow::tr("falconG warning"), QMainWindow::tr("Image \n'%s'\n exists").arg(dst), QMessageBox::Yes | QMessageBox::Abort).exec()) == QMessageBox::Abort)
						//	return false;
				QImage img = LoadImage(src, config.imageWidth, config.imageHeight, config.doNotEnlarge);
				img.save(dst);

				frmMain->ImageMapChanged();	// modify list of images
			}
		if (im.uploadDate > _latestDateLimit)	// find latest upload date
			_latestDateLimit = im.uploadDate;	// will be put into "latest.html"
							// progress bar
		}
		frmMain->SetProgressBarPos(++cnt, _images.size());
		//QApplication::processEvents();
	}
	return 0;
}

/*============================================================================
* TASK: Creates one html file for a given album
* EXPECTS:	album , 
*			actLanguage - index of actual language
* GLOBALS:	'config'
* RETURNS: 0: OK, 16: error writing file
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreatePage(Album &album, int language, const QString parent, int &processedCount)
{
	_actLanguage = language;

	QString s = (config.dsGallery + config.dsGRoot + config.dsAlbumDir).ToString() + album.LinkName(language); // all albums are in the same directory
	if (QFile::exists(s))
		return 0;
	QFile f(s);
	if (!f.open(QIODevice::WriteOnly))
		return 16;

	frmMain->SetProgressBarPos(++processedCount, _albums.size() * Languages::Count());
	QApplication::processEvents();

	ofs.setDevice(&f);
	ofs.setCodec("UTF-8");

	ofs << _PageHeaderToString();

	if (config.bRightClickProtected)
		ofs << "<body oncontextmenu=\"return false;\">;\n";
	else
		ofs << "<body>\n";
	ofs << "<!--Header section-->\n";
	_OuputHeaderSection(album, parent);

	ofs	<< "<!-- Main section -->\n"
		   "<main id=\"main\">\n";

	// get number of images and sub-album
	if (album.ImageCount())
	{
		ofs << "<!--the images in this sub gallery-->\n"
			<< "<a name=\"images\">" << Languages::toImages[_actLanguage] << "</a>\n""<section id=\"images\">";
		// first the images
		for (int i = 0; frmMain->IsRunning() && i < album.images.size(); ++i)
			if (album.excluded.indexOf(album.images[i]) < 0)
				_WriteGalleryContainer(album, false, i);
		ofs << "</section>\n<!-- end section Images -->\n";
	}
	if (album.SubAlbumCount())
	{
		ofs << "\n<!--start section albums -->\n"
			<< "<a name = \"galleries\">" << Languages::toAlbums[_actLanguage] << "</a>\n"
			"<section id = \"folders\">\n";

		for (int i = 0; frmMain->IsRunning() && i < album.albums.size(); ++i)
			if (album.excluded.indexOf(album.albums[i]) < 0)
				_WriteGalleryContainer(album, true, i);
		ofs << "</section>\n<!-- end section Albums -->\n";
	}
	if (frmMain->IsRunning())		// else leave tha page unfinished
	{
		// main section is finished
		ofs << "\n</main> <!-- end of main section -->\n";
		// footer section
		_WriteFooterSection(album);
		// end html
		ofs << "</body>\n</html>\n";
		f.close();
		// now create sub albums
		if (album.SubAlbumCount())
		{
			for (int i = 0; frmMain->IsRunning() && i < album.albums.size(); ++i)
				if (album.excluded.indexOf(album.albums[i]) < 0)
					_CreatePage(_albums[album.albums[i]], language, album.LinkName(language, false), processedCount);
		}
	}
	return 0;
}

/*============================================================================
* TASK: Creates all html files
* EXPECTS:	config fields are set, albums are read in (_albums, etc)
* GLOBAL:	'config'
* RETURNS: 0: OK, 16: error writing file
* REMARKS: each file is named <config.baseName><album.ID><Language::abbrev[]>,html
*--------------------------------------------------------------------------*/
int AlbumGenerator::_DoPages()
{
	if (!frmMain->IsRunning())
		return 0;

	QString parent;
	frmMain->SetProgressBar(0, _albums.size() * Languages::Count(), 0, 3); // phase = 3
	QApplication::processEvents();
	int cnt = 0;
	for (int lang = 0; frmMain->IsRunning() & lang < Languages::Count(); ++lang)
	{
			parent = config.sUplink.ToString();
			if (parent.right(4) != 'html' && parent.right(4) != '.htm' && parent.right(4) != '.php')
				parent += Languages::abbrev[lang] + ".html";
		_CreatePage(_albums[1], lang, parent, cnt);	// all albums goe into the same directory!
	}

	return 0;
}

/*============================================================================
* TASK:	Creates latest uploads	- latest.html
* EXPECTS:	config fields are set
* GLOBAL:	'config'
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
* GLOBAL:	'config'
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
	ofs.setDevice(&htx);
	ofs << _HtaccessToString();
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
* GLOBAL:	'config'
* RETURNS:	0: OK, other error code
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumGenerator::Write()
{
	if (!_CreateDirectories()) // from 'config'
		return 64;
	WriteDirStruct();

	_ProcessImages();	// copy from from source into image directory
							// and determine latest upload date

	int i;					// returns:
	i = _DoCopyRes();			// 0 | 1
	i |= _DoHtAccess();			// 0 | 2
	i |= _DoColorsCss();		// 0 | 4
	i |= _DoStyleFG();			// 0 | 8
	i |= _DoPages();			// 0 | 16
	i |= _DoLatest();			// 0 | 32

	frmMain->SetProgressBar(0, 100, 0, 0);		// reset phase to 0
	return i;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBAL:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumGenerator::SaveStyleSheets()
{
	__SetBaseDirs();
	albumgen._DoColorsCss();
	albumgen._DoStyleFG();
}

