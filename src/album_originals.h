#pragma once
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "support.h"

using QString=QString;
using ID_t = uint64_t;		// almost all ID's are CRC32 values extended with leading bits when collison
using IdList = QList<ID_t>;

const ID_t INVALID_ID_FLAG = 0x8000000000000000ull;
const ID_t ALBUM_ID_FLAG= 0x4000000000000000ull;	// when set ID is for an album (used for albums as folder thumbnails)
const ID_t ID_MASK = ~(INVALID_ID_FLAG | ALBUM_ID_FLAG);
const ID_t BASE_ID_MASK	= 0x00000000FFFFFFFFull;	// values & BASE_ID_MASK = CRC
const ID_t ID_INCREMENT = BASE_ID_MASK + 1;	// when image id clash add this to id 

extern bool BackupAndRename(QString name, QString tmpName, QWidget *parent);	// in support.cpp

//*****************************************
class LanguageTextList : public QStringList
{
public:	
	LanguageTextList(int size = 0) : QStringList() { Prepare(size); }

	void Prepare(int languageCount) { clear();  for (int i = 0; i < languageCount; ++i) push_back(""); }
	void Clear() { for (int i = 0; i < size(); ++i) (*this)[i].clear(); }
	bool IsEmpty() {for (int i = 0; i < size(); ++i) if (!(*this)[i].isEmpty()) return false; return true;}
	void Fill(QString s) { for (int i = 0; i < size(); ++i) (*this)[i] = s; }

	LanguageTextList &operator=(const LanguageTextList & txt) { QStringList::operator=(dynamic_cast<const QStringList &>(txt)); }
	LanguageTextList &operator=(const QStringList &sl) { QStringList::operator=(sl); }
};
//*****************************************

struct Languages	// each language has a text file named <text from 'languages'>.php
{					
	static LanguageTextList abbrev;			// abbreviations: "hu", "en", etc
	static LanguageTextList names;			// used on menus to switch language, eg. "Magyarul"
	static LanguageTextList icons;			// icons to use instead of names
		// these are read from language definition files '<xxx>.lang in program directory
	static LanguageTextList toImages;	// image section in HTML files
	static LanguageTextList toAlbums;// album section in HTML files
	static LanguageTextList toHomePage;		// 'Kezdõlapra'
	static LanguageTextList toAboutPage;		// 'Rólam'
	static LanguageTextList toContact;		// 'Kapcsolat'
	static LanguageTextList share;			// 'Megosztás'
	static LanguageTextList showCaptions;	// 'Képleírások'
	static LanguageTextList latestTitle;		// 'Kedvcsinálónak'
	static LanguageTextList latestDesc;		// 'Ebbe a könyvtárba...'

	static int _Read(QString name);		// defult: single language, name: "English" abbrev: "en", no icon
	static int Read();					// reads all language files from program directory
	static void Clear(int newSize = 0);
	static int Count() { return abbrev.size(); }
};

//------------------------------------------
struct Text
{
	ID_t ID = 0;		// same for all translations of this in all languages (0: invalid)
	int lang=-1;		// lang is index of ID text  in 'texts' (set for longest text) <0 : ID not calculated yet
	QStringList	texts;	// set to have as many elements as there are languages, lang ID is index in text
	UsageCount _usageCount; // how many times this same text is added to list
	Text(int languageCount = 0) { Clear(languageCount); }
	Text(const QStringList &texts);
	Text(const Text &t) : ID(t.ID),  lang(t.lang), texts(t.texts) {}

	bool Valid() const { return ID > 0; }
	ID_t CalcID();

	void Clear(int newSize = 0);  // 0: does not change size of 'texts'
	bool IsEmpty() { for (int i = 0; i < texts.size(); ++i) if (!texts[i].isEmpty()) return false; return true; }

	Text &operator=(const Text &t);
	Text &operator=(const QStringList &txts);

	int operator<(const Text &t);
	bool operator==(const Text &t);		// compare first ID then all texts
	bool operator!=(const Text &t);		// compare first ID then lang (unless lang < 0, when only ID is used)
	QString & operator[](int index) { static QString dummy;  return (index >= 0 && index < texts.size()) ? texts[index] : dummy; }
};

//------------------------ base class for albums and images ------------------
struct IABase
{
	ID_t ID = 0;		// CRC of image content + collision avoidance  (0: invalid)
	bool exists = false;	// set to true if it exists on the disk somewhere
							// some image names may come from comment files
	ID_t titleID = 0;	// text ID of image title
	ID_t descID = 0;	// text ID of image description
	QString name;		// without path but with extension and no ending '/' even for albums
	QString path;		// empty or ends with '/'

	bool Valid() const { return ID > 0; }
	QString FullName() const { return path + name; }				// does not end with '/'
	IABase &operator=(const IABase &a)
	{
		ID		=  a.ID		;
		exists  =  a.exists ;
		titleID =  a.titleID;
		descID	=  a.descID	;
		name    =  a.name   ;
		path    =  a.path   ;
		return *this;
	}
};
//------------------------------------------
struct Image : public IABase
{
	QString checksum = 0;	// of content not used YET
	QDate uploadDate;
	uint64_t fileSize;	// set together with 'exists'


	//Image() {}
	//Image(const Image & im) : ID(im.ID), titleID(im.titleID), descID(im.descID),
	//						  name(im.name), path(im.path), checksum(im.checksum),
	//						  uploadTime(im.uploadTime) {}

	enum SearchCond : int { byID,		// ID only
							byBaseID,	// ID for image w. o path: compare names (no path) as well
							byName,		// just image name
							byFullName	// full image path
						  };

	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name
	int operator<(const Image &i);
	bool operator==(const Image &i);

	QString LinkName() const { int pos = name.lastIndexOf('.'); return QString().setNum(ID) + name.mid(pos);  }	// e.g. 12345.jpg
	QTextStream & WriteTranslation(QTextStream &ofs) const;
};

//------------------------------------------
struct Album : IABase			// ID == 1 root  (0: invalid)
{
	IdList images,		// 'images' and 'albums' are lists of IDs inside this album for faster searches
		   albums,		// (search images in one and albums in the other)
		   order,		// both image and album IDs in the order they should be displayed
		   excluded;	// id's in list are for images and folders excluded from this album
	ID_t thumbnail = 0;	// image ID	or 0
	IdList parent;		// multiple parents possible

	int imageCount = -1;
	int albumCount = -1;

	enum SearchCond {byID, byName};
	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name
	int operator<(const Album &i);
	bool operator==(const Album &i);

	void Clear() { images.clear(); albums.clear(); order.clear(); excluded.clear(); }
	int ImageCount();		// only non-excluded images
	int SubAlbumCount();	// only non excluded albums ( only calculates once)
	static QString NameFromID(ID_t id, int language);
	QString LinkName(int language, bool http = false) const;	// like https://<base name><ID><lang>.html
	QString BareName();			// '<base name>ID'
};

//------------------------------------------
class TextMap : public QMap<ID_t, Text>
{
	static Text invalid;
public:
	Text & Find(ID_t id);
	Text& Find(QString text, int lang);
	Text& Find(const QStringList &texts);
	ID_t Add(QStringList &texts, bool &added);	// add unique text, returns ID and if added
	ID_t Add(Text & text, bool added);			// add based on ID and lang and text
	void Remove(ID_t id);					// decrements usage cont and removes text if it is 0
};

//------------------------------------------
class ImageMap : public QMap<ID_t, Image>
{
	//friend class ALbumGenerator; miert nem volt eleg
	//ahhoz, hogy ezt lassa:	static Image invalid;
public:
	static Image invalid;
	Image& Find(ID_t id, bool useBase = true);
	Image& Find(QString FullName);
	ID_t Add(QString image, bool &added);	// returns ID and if added
	Image &Item(int index);
};

//------------------------------------------
class AlbumMap : public QMap<ID_t, Album>
{
	static Album invalid;
public:
	Album &Find(ID_t id);
	Album &Find(QString albumPath);
	ID_t Add(QString albumPath, bool &added);			// returns ID and if added
	Album &Item(int index);
};

struct IdsFromStruct
{
	enum { nothing, title, description, thumbnail } what = nothing;

	ID_t titleID = 0, descID = 0, thumbnailID = 0;
	void Clear() { titleID = descID = thumbnailID = 0; what = nothing; }
	bool IsEmpty() const { return titleID == 0 && descID == 0 && thumbnailID == 0; }	// what does not important
};

//------------------------------------------
class AlbumGenerator
{
	enum {ok, ready, rerr, nosuch} _status=ok; // can read further, no sub directories, read error or empty, no such file/dir
	QString _upLink;		// to parent page if there's one
	TextMap _texts;		// all texts for all albums and inmages
	AlbumMap _albums;		// all source albums
	ImageMap _images;		// all images for all albums
	Album _root;			// top level album (first in '_albums', ID = 1)
	QDate _latestDateLimit; // depends on config.
	bool _justChanges;		// only process changes. set by 'Read()'
	bool _dontProcessImages; 

	int _actLanguage = 0;	// actual language used in album generation

	QTextStream ofs, ifs;		// write (read) data (from) here

	QStringList _SeparateLanguageTexts(QString line);

	bool _IsExcluded(const Album& album, QString name);
	void _TitleFromPath(QString path, LanguageTextList &ltl);
	bool _ReadAlbumFile(Album &ab);		// albumfiles.txt
	bool _ReadCommentFile(Album &ab);	// comments.properties
	bool _ReadMetaFile(Album &ab);		// meta.properties
	void _AddTitleTo(Album &ab, QString &path, QString name);	// add to _texts and album or image title
	bool _ReadInfo(Album &ab);			// album and image titles in hidden .kalbum sub directories
	void _ReadOneLevel(Album &ab);
	ID_t _AddImageOrAlbum(Album &ab, QString path, bool hidden=false);
	ID_t _AddImageOrAlbum(Album &ab, QFileInfo& fi, bool fromDisk = false);
					// 'gallery.struct' file
	ID_t _ThumbnailID(Album & album); // recursive search when required
	void _WriteStructLanguageTexts(QString what, ID_t id, QString indent);
	void _WriteStructSubAlbumsAndImages(Album& album, QString indent);
	void _WriteStructAlbums(Album& album, QString indent);
	void _WriteImageTranslations();		// into img directory

	QString _ShadowToString(QString s, _CElem &elem);
	QString _ElemColorCssToString(QString selector, _CElem &elem, int what);
	QString _GradientCssToString(_CElem &elem, bool invert = false);
	QString _MenuColorCSSToString();
	QString _ColorCSSToString();
	QString _CssToString();
	QString _HtaccessToString();
	QString _PageHeaderToString();
	int _OuputHeaderSection(Album &album, const QString parent);
	int _WriteFooterSection(const Album &album);
	int _WriteGalleryContainer(const Album &album, bool albums, int i);
	int _ProcessImages(); // into image directory
	int _CreatePage(Album &album, int language, const QString parent, int &processedCount);
	int _DoCopyRes();
	int _DoColorsCss();
	int _DoStyleFG();
	int _DoPages();
	int _DoLatest();
	int _DoHtAccess();
	bool _CreateDirectories();	// from data in config

	bool _LanguageFromStruct(FileReader &reader);
	ID_t _ImageFromStruct(FileReader &reader, int level);
	ID_t _AlbumFromStruct(FileReader &reader, int level);
	void _GetIDsFromStruct(FileReader &reader, IdsFromStruct &ids, int level);

	bool _ReadStruct(QString from);	// from gallery.struct (first dest, then src directory) 
	bool _ReadJalbum();	//or if it does not exist loads them from supposedly jalbum directory dsSrc
public:
	AlbumGenerator() {}
	bool Read(bool justChanges, bool bDisregardStruct = false, bool dontProcessImages = false);	 // reads all albums recursively from Config::dsSrc
	int Write();	 // writes album files into directory Config::sDestDir return error code or 0
	int WriteDirStruct();		
	void SaveStyleSheets();
	int AlbumCount() const { return _albums.size(); }
	int ImageCount() const { return _images.size(); }
	int TextCount() const { return _texts.size(); }
	// careful: these are not const so that their elements could be modified
	Album &AlbumRoot()  { return _root; }
	ImageMap &Images() { return _images; }
	TextMap &Texts()   { return _texts;  }
	AlbumMap &Albums() { return _albums; }
};

//---------------------------------------------
extern AlbumGenerator albumgen;
