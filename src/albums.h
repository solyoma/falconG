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
#include <QThread>
#include <QMutex>


#include "support.h"
#include "deletableitems.h"
#include "config.h"
#include "crc32.h"
#include "stylehandler.h"
#include "languages.h"

using QString=QString;
using ID_t = uint64_t;		// almost all ID's are CRC32 values extended with leading bits when collison
using IdList = DeletableItemList<ID_t>;

const ID_t INVALID_ID_FLAG = 0x8000000000000000ull;
const ID_t ALBUM_ID_FLAG= 0x4000000000000000ull;	// when set ID is for an album (used for albums as folder thumbnails)
const ID_t ID_MASK = ~(INVALID_ID_FLAG | ALBUM_ID_FLAG);
const ID_t BASE_ID_MASK	= 0x00000000FFFFFFFFull;	// values & BASE_ID_MASK = CRC
const ID_t ID_INCREMENT = BASE_ID_MASK + 1;	// when image id clash add this to id 
const int ID_COLLISION_FACTOR = 32;		// id >> ID_COLLISION_FACTOR = overflow index

extern bool BackupAndRename(QString name, QString tmpName, QWidget *parent, bool keepBackup);	// in support.cpp

//------------------------------------------
class TextMap;			// used in text for ID calculation

/*============================================================================
* LanguageTexts and TextMap
*	- LanguageTexts objects contains one QString for each language language
*	- Texts are represented by their ID, which may not be 0
*	- text IDs are calculated for the longest QString. This is the base ID
*	- two or more texts may have the same base ID but different QStrings in 
*		them (text ID collision)
*	- his may occur even when the longest QString (
*	  in which case the ID of the new text (the one that was added later on)
*		is modified using the 'collision' member, which is 
*			1 + the number of the other texts with the same base ID
*	- the usage count of each text is the number of Albums and Images that
*		use it. If 'usageCount' is 0 thex is not used anywhere
*	- More than one Album or Image can use the same text, or more accurately the
*	  same text ID
*	
*----------------------------------------------------------------------------*/

struct LanguageTexts				// both Album and Image uses this
{
	ID_t ID = 0;		// same for all translations of this in all languages (0: invalid)
	int collision = -1;	// (-1: not set) set when this text collided with an other one: ID = base id + (collision << ID_COLLISION_FACTOR)
	QList<int>	lenghts;	// set to have as many elements as there are languages, lang ID is index in text
	QString textsForAllLanguages;
	UsageCount usageCount; // how many times this same text is added to list

	LanguageTexts(int languageCount = 0) { Clear(languageCount); }
	LanguageTexts(const QStringList &texts);
	LanguageTexts(const LanguageTexts &t) : ID(t.ID), collision(t.collision), lenghts(t.lenghts), textsForAllLanguages(t.textsForAllLanguages), usageCount(t.usageCount) {}

	LanguageTexts &operator=(const LanguageTexts &t);
	LanguageTexts &operator=(const QStringList &txts);
	bool operator==(const LanguageTexts &t);		// compare first ID then all texts
	bool operator!=(const LanguageTexts &t);		// compare first ID then lang (unless lang < 0, when only ID is used)
	int operator<(const LanguageTexts &t);

	const QString operator[](int index) const			   // ***  can't change text using this! Use SetText() instead
	{ 
		static QString dummy;
		if (textsForAllLanguages.isEmpty())
			return dummy;
		if (index == 0)
			return textsForAllLanguages.left(lenghts[0]);
		return (index > 0 && index < lenghts.size()) ? textsForAllLanguages.mid(lenghts[index-1], lenghts[index]) : dummy;
	}
	void SetTextForLanguageNoID(const QString str, int lang);		 // just set
	
	bool Valid() const { return ID > 0; }
	ID_t CalcBaseID();			// changes 'ID' but does not change 'collision'
	ID_t CalcID(TextMap &map);	// changes 'ID' and 'collision' unless already OK (read in)

	void Clear(int newSize = 0);  // 0: does not change size of 'lenghts'
	bool IsEmpty() { return textsForAllLanguages.isEmpty(); }
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
	QString path;		// either relative to confg.dsSrc or an absolute path. may be empty otherwise ends with '/'

	bool Valid() const { return ID > 0; }
	QString FullName() const { return path + name; }				// does not end with '/'
	QString ShortPathName();	// cuts the common part of paths (config.dsSrc)
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
	bool dontResize = false;	// when image name is preceeded by double exclamation marks: !!
								// either in the original path (this/image/!!notResized.jpg) or 
								// in the precessed line (!!notresized(...)this/image/)
	QDate uploadDate;
	int64_t fileSize=0;		// set together with 'exists' (if file does not exist fileSize is 0)
	int width =0, height=0,		// of resized image
		owidth=0, oheight = 0;  // original size, 
							// set in ProcessImages (oheight = 0 -> not set)
	double aspect = 1.0;	// aspect ratio


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

	int operator<(const Image &i);		 // uses searchBy
	bool operator==(const Image &i);

	QString LinkName() const 
	{ 
		if (ID)
		{
			int pos = name.lastIndexOf('.');
			return QString().setNum(ID) + name.mid(pos);	// e.g. 12345.jpg
		}
		else
			return name;
	}	
	QTextStream & WriteInfo(QTextStream &ofs) const;
	void SetResizeType()
	{
		if (name.length() > 2 && name[0] == '!' && name[1] == '!')
		{
			name = name.mid(2);
			dontResize = true;
		}

	}
};

//------------------------------------------
struct Album : IABase			// ID == 1 root  (0: invalid)
{
	ID_t parent = 0;	// just a single parent is allowed
	IdList images,		// 'images' and 'albums' inside this album are lists of IDs for faster searches
		   albums,		// (search images in one and albums in the other)
		   excluded;	// id's in list are for images and folders excluded from this album
	ID_t thumbnail = 0;	// image ID	or 0

	bool changed = false;		// set to true when: any text, album thumbnail, images, albums, exluded changed
	int imageCount = -1;
	int albumCount = -1;

	enum SearchCond {byID, byName};
	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Album &i);		 // uses searchBy
	bool operator==(const Album &i);

	void Clear() { images.clear(); albums.clear(); excluded.clear(); }
	int ImageCount();		// only non-excluded images
	int SubAlbumCount();	// only non excluded albums (removes excluded albums) = count of children
	static QString NameFromID(ID_t id, int language, bool withAlbumPath);			// <basename><id><lang>.html
	QString NameFromID(int language);
	QString SiteLink(int language );
	QString LinkName(int language, bool addHttpOrsPrefix = false) const;	// like https://<server URL>/<base name><ID><lang>.html
	QString BareName();			// '<base name>ID'
};

//------------------------------------------
class TextMap : public QMap<ID_t, LanguageTexts>
{
	static LanguageTexts invalid;
public:
	LanguageTexts & Find(ID_t id);
	LanguageTexts& Find(QString text, int lang);
	LanguageTexts& Find(const QStringList &texts);
	int Collision(LanguageTexts &text, int &usageCount) const;		// do not add when text is the same
	ID_t Add(QStringList &texts, bool &added);	// add unique text, returns ID and if added
	ID_t Add(LanguageTexts & text, bool &added);			// add based on ID and lang and text
	void Remove(ID_t id);					// decrements usage cont and removes text if it is 0
};

//------------------------------------------
class ImageMap : public QMap<ID_t, Image>
{
	//friend class ALbumGenerator; miert nem volt eleg
	//ahhoz, hogy ezt lassa:	static Image invalid;
public:
	static Image invalid;
	static QString lastUsedImagePath;	// config.dsSrc relative path to image so that we can add 
										// an image by its name (relative to this path) only

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
	static QString lastAlbumPath;	//relative to confg.dsSrc
	Album &Find(ID_t id);
	Album &Find(QString albumPath);
	ID_t Add(QString relativeAlbumPath, bool &added);			// returns ID and if it was added
	Album &Item(int index);
};

struct IdsFromStruct
{
	enum IDS { nothing, title, description, thumbnail } what = nothing;

	ID_t titleID = 0, descID = 0, thumbnailID = 0;
	void Clear() { titleID = descID = thumbnailID = 0; what = nothing; }
	bool IsEmpty() const { return titleID == 0 && descID == 0 && thumbnailID == 0; }	// what does not important
};

//------------------------------------------
class AlbumStructWriterThread;		// forward

class AlbumGenerator : public QObject
{
	Q_OBJECT

	enum {ok, ready, rerr, nosuch} _status=ok; // can read further, no sub directories, read error or empty, no such file/dir
	struct _RemainingDisplay
	{
		int max = 0;
		time_t tAct, tTot;
		void Init(int maximum);
		void Update(int cnt);		// show elapsed and remaining time and if speed then cnt/elapsed too
	private:
		time_t t0 = 0,tprev= 0;
	} _remDsp;

	bool _running = false;

	QString _upLink;		// to parent page if there's one
	TextMap _textMap;		// all texts for all albums and inmages
	AlbumMap _albumMap;		// all source albums
	ImageMap _imageMap;		// all images for all albums
	Album _root;			// top level album (first in '_albumMap', ID = 1)
	QDate _latestDateLimit; // depends on config.
// no need: read is fast enough 	bool _structAlreadyInMemory = false;
	UsageCount _structChanged;	// signals wheather a new 'struct' file must be written
							// increase after a jalbum style read and after an album struct change
							// this is used to determine that actual album was changed or not
							// record its value before the changes and compare with this after the changes
							// only used as a bool value otherwise

	AlbumStructWriterThread *pWriteStructThread;		// write structure in separate thread
	bool _structWritten = true;
	bool _mustRecreateAllAlbums = false;				// set to true when signaled from FalconG class that uplink etc changed
	bool _keepPreviousBackup = false; // rename temporary file ?

	int _actLanguage = 0;	// actual language used in album generation

	QTextStream _ofs, _ifs;		// write (read) data to (from) here

	bool _MustRecreateImageBasedOnSize(Image &img);
	bool _MustRecreateThumbBasedOnSize(QString thumbName, Image &img);
	QStringList _SeparateLanguageTexts(QString line);		  // helpers
	QString& _GetSetImagePath(QString &img);
	bool _IsExcluded(const Album& album, QString name);
	void _TitleFromPath(QString path, LangConstList &ltl);
	QString _ShadowToString(QString s, _CElem &elem);
	QString _ElemColorCssToString(QString selector, _CElem &elem, int what);
	QString _GradientCssToString(_CElem &elem, bool invert = false);
	QString _MenuColorCSSToString();
	QString _ColorCSSToString();
	QString _CssToString();
	QString _HtaccessToString();
	QString _GoogleAnaliticsOn();
	QString _PageHeaderToString(ID_t id);

	bool _CreateDirectories();	// from data in config

						// writing 

						  // reading (and copying) data
	bool _ReadAlbumFile(Album &ab);		// albumfiles.txt
	bool _ReadCommentFile(Album &ab);	// comments.properties
	bool _ReadMetaFile(Album &ab);		// meta.properties
	void _ReadInfoFile(Album &ab, QString &path, QString name);	// '.info' files, add to _textMap and album or image title
	bool _ReadInfo(Album &ab);			// album and image titles in hidden .jalbum sub directories
	void _ReadOneLevel(Album &ab);
	ID_t _AddImageOrAlbum(Album &ab, QString path, bool hidden=false);
	ID_t _AddImageFromPathInStruct(QString imagePath);
	ID_t _AddImageOrAlbum(Album &ab, QFileInfo& fi/*, bool fromDisk = false*/);
	
					// write gallery files
	void _OutputFacebookLink(QString linkName, ID_t ID);
	QString _IncludeFacebookLibrary();
	void _OutputMenuLine(Album &album, QString uplink);
	int _OuputHeaderSection(Album &album);
	int _WriteFooterSection(const Album &album);
	int _WriteGalleryContainer(const Album &album, bool albums, int i);
	int _ProcessImages(); // into image directory
	int __CreatePageInner(QFile &f, Album &album, int language, QString uplink, int &processedCount);
	int _CreatePage(Album &album, int language, QString parent, int &processedCount);
	int _CreateHomePage();

	int _DoCopyRes();
	int _DoColorsCss();
	int _DoStyleFG();
	int _DoPages();
	int _DoLatest();
	int _DoHtAccess();
				// read 'gallery.struct
	bool _LanguageFromStruct(FileReader &reader);
	ID_t _ImageFromStruct(FileReader &reader, int level, Album &album, bool thumbnail);
	ID_t _ReadAlbumFromStruct(FileReader &reader, ID_t parent, int level);
	void _GetTextAndThumbnailIDsFromStruct(FileReader &reader, IdsFromStruct &ids, int level);
	bool _ReadStruct(QString from);	// from gallery.struct (first dest, then src directory) 

			// read 'Jalbum' file structure
	bool _ReadJalbum();	//or if it does not exist loads them from supposedly jalbum directory dsSrc
	bool _ReadFromGallery();	// recrates album structure but can't recover album paths and image names or dimensions
private:
	void _WriteStructReady(QString );		// slot !
public:
	AlbumGenerator() {};
	bool Read();	 // reads all albums recursively from Config::dsSrc
	int Write();	 // writes album files into directory Config::sDestDir return error code or 0
	int WriteDirStruct(bool keep=false);		
	bool StructWritten() const { return _structWritten; }
	bool StructChanged() const { return _structChanged;  }
	void SaveStyleSheets();
	void SetRecrateAlbumFlag(bool Yes) { _mustRecreateAllAlbums = Yes; };

	static QString RootNameFromBase(QString base, int language, bool toServerPath = false);
	
	int AlbumCount() const { return _albumMap.size(); }
	int ImageCount() const { return _imageMap.size(); }
	int TextCount() const { return _textMap.size(); }
	// careful: these are not const so that their elements could be modified
	Album &AlbumRoot()  { return _root; }
	ImageMap &Images() { return _imageMap; }
	TextMap &Texts()   { return _textMap;  }
	AlbumMap &Albums() { return _albumMap; }
signals:
	void SignalToSetProgressParams(int min, int max, int pos, int phase);
	void SignalProgressPos(int cnt1, int cnt2);
	void SignalSetLanguagesToUI();
	void SignalToEnableEditTab(bool on);
//	void SignalImageMapChanged();
	void SignalAlbumStructChanged();
	void SignalToShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void SignalToCreateUplinkIcon(QString destPathName);
	void SetDirectoryCountTo(int cnt);
public:		// SLOT: connected with new syntax: no need for MOC to use this slot
	void Cancelled() { _running = false; }
};

//------------------------------------------
// Thread to write album structure (simplest: no message loop inside thread)
class AlbumStructWriterThread : public QThread
{
	Q_OBJECT

	QMutex _albumMapStructIsBeingWritten;

	TextMap &_textMap;		// all texts for all albums and inmages
	AlbumMap &_albumMap;		// all source albums
	ImageMap &_imageMap;		// all images for all albums
	Album _root;			// top level album (first in '_albumMap', ID = 1)
	QTextStream _ofs;

	void _WriteStructImagesThenSubAlbums(Album& album, QString indent);
	void _WriteStructAlbums(Album& album, QString indent);


// DEBUG	void run() override;  // the actual writing
signals:
	void resultReady(QString s);
public:
	// DEBUG 1 line
	void run() override;  // the actual writing
	AlbumStructWriterThread(AlbumGenerator&generator, QObject *parent = Q_NULLPTR);
};

//---------------------------------------------
extern AlbumGenerator albumgen;
