#pragma once
#ifndef _ALBUMS_H
	#define _ALBUMS_H

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

// DEBUG
#ifdef DEBUG_ME
	#include <QtDebug>
#endif

#include "support.h"
#include "config.h"
#include "crc32.h"
#include "stylehandler.h"
#include "languages.h"

const QString TITLE_TAG = "Title-";	// used in 'struct' files
const QString DESCRIPTION_TAG = "Descr-";	// e.g. [Tytle-hu:<text>]
const QString THUMBNAIL_TAG = "Icon";	// "album icon"
//const QString SAVED_IMAGE_DESCRIPTIONS = "images.desc";
//const QString TEMP_SAVED_IMAGE_DESCRIPTIONS = "images.tmp";

#define ID_T_DEFINED
using ID_t = int64_t;		// almost all ID's are CRC32 values extended with leading bits when collison
using IdList = QVector<ID_t>; 

const ID_t EXCLUDED_FLAG = 0x8000000000000000ull;
const ID_t ALBUM_ID_FLAG = 0x4000000000000000ull;	// when set ID is for an album (used for albums as folder thumbnails)
const ID_t ROOT_ALBUM_ID = 0x4000000000000001ull;

const ID_t RECENT_ALBUM_ID = 0x4000000000000002ull;

const ID_t VIDEO_ID_FLAG = 0x2000000000000000ull;	// when set ID is for a video
const ID_t IMAGE_ID_FLAG = 0x1000000000000000ull;	// when set ID is for a video
const ID_t BASE_ID_MASK	 = 0xF0000000FFFFFFFFull;	// values & BASE_ID_MASK = CRC + image/video/album flag
const ID_t ID_MASK		 = 0x0FFFFFFFFFFFFFFFull;	// values & ID_MASK determines names of images, videos and albums
const ID_t ID_INCREMENT  = 0x0000000100000000ull;	// when image id clash add this to id 
const ID_t MAX_ID = 0xFFFFFFFFFFFFFFFFull ^ (ALBUM_ID_FLAG | VIDEO_ID_FLAG);	 
const int ID_COLLISION_FACTOR = 32;		// id >> ID_COLLISION_FACTOR = overflow index

extern QString BackupAndRename(QString name, QString tmpName, QWidget *parent, bool keepBackup);	// in support.cpp

//------------------------ base class for albums and images ------------------
struct IABase
{
	ID_t ID = 0;			// CRC of image name + collision avoidance  (0: invalid) + type bits (ALBUM_ID_FLAG or VIDEO_ID_FLAG)
	bool exists = false;	// set to true if it exists on the disk somewhere
							// some image names may come from comment files
	ID_t titleID = 0;	// default text ID of image title
	ID_t descID = 0;	// default text ID of image description
	QString name;		// without path but with extension and no ending '/' even for albums
	QString path;		// either relative to confg.dsSrc or an absolute path. may be empty otherwise ends with '/'

	bool Valid() const { return ID > 0; }
	QString FullSourceName() const { return path + name; }				// does not end with '/'
	QString ShortSourcePathName();	// cuts the common part of paths (config.dsSrc)
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
	bool changed = false;		// true: image is recreated (dimensions or file size or data changed)
								// check this and create a new struct file on disk if any image changed
								// inside any albums. Alse set the album changed flag if any of its
								// images/thu,bnails, etc changed
	QString checksum = 0;		// of content not used YET
	bool dontResize = false;	// when image name is preceeded by double exclamation marks: !!
								// either in the original path (this/image/!!notResized.jpg) or 
								// in the precessed line (!!notresized(...)this/image/)
	QDate uploadDate;
	int64_t fileSize=0;			// of source file, set together with 'exists' (if file does not exist fileSize is 0)
	QSize	osize = { 0, 0 },	// original dimensions read from struct file,
			dsize = { 0, 0 },	// transformed size read either from .struct file or from destination file if it exists
			rsize = { 0, 0 },	// image to be resized to this size
			tsize;				// thumbnail size. Determined externally

	bool	bSizeDifferent;		// from width and height

	void GetResizedDimensions(QSize &destSize)	// 'destSize' is size of existing destination image or 0,0
	{							// if destination image does not exist or a wrong size
		rsize = osize;			// recalculate destination size into 'rsize' 
		QSize csize = config.ImageSize();	// required destination image dimensions

		if (!dontResize &&		// can resize and
				( 				// original size is too big
					(osize.width() > csize.width() || osize.height() > csize.height()) ||
								// original size is to small and enlargement is allowed
					(!config.doNotEnlarge && osize.width() < csize.width() && osize.height() < csize.height())
				)
		)
			rsize.scale(csize, Qt::KeepAspectRatio);	// calculate destination size
			// and compare it with previous size (if any given)
			// dsize can be 0,0 if no destination image exists or specified
		QSize dSize = dsize.isEmpty() ? destSize : dsize;
		bSizeDifferent = (abs(rsize.width() - dSize.width()) > 2) || (abs(rsize.height() - dSize.height()) > 2);

		SetThumbSize();
		//if (tsize.width() > ctsize.width())	// crop thumbnail from image
		//{
		//	// was TODO but now cropping and distorsion is set into falconG.css !
		//}
	}

	void SetThumbSize()
	{
		QSize ctsize= config.ThumbSize();
		tsize = osize;
		// get minimum size that fills the 'tsize' rectangle. It may extend outside the allowed rectangle
		tsize.scale(ctsize, Qt::KeepAspectRatio);
		// thumbnails must have the vertical size the same as in ctsize
		if (tsize.height() != ctsize.height())
		{
			tsize.setWidth(round((double)ctsize.height() / (double)tsize.height() * tsize.width()));
			tsize.setHeight(ctsize.height());
		}
	}

	void SetNewDimensions()
	{
		if (dsize.width() == 0)	   // image did not exist
			GetResizedDimensions(dsize);
		dsize = rsize;
	}

	enum SearchCond : int { byID,		// ID only
							byBaseID,	// ID for image w. o path: compare names (no path) as well
							byName,		// just image name
							byFullSourceName	// full image path
						  };

	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Image &i);		 // uses searchBy
	bool operator==(const Image& i);
	double Aspect() 
	{ 
		if (_aspect) 
			return _aspect; 
		return _aspect = (osize.height() > 0) ? (double)osize.width() / (double)osize.height() : 1.0;
	}
	double ThumbAspect()
	{
		if (config.ImageAndThumbAspectDiffer())
			return config.ThumbAspect();
		else 
			return _aspect;
	}

	QString LinkName(bool bLCExtension = false) const		// used in HTML files, not the source name
	{ 
		if (ID)
		{
			int pos = name.lastIndexOf('.');
			QString ext = name.mid(pos);
			if (bLCExtension)
				ext = ext.toLower();
			return QString().setNum(ID & ID_MASK) + ext;	// e.g. 12345.jpg (images IDs has no flag set)
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

private:
	double _aspect = 0;			// for actual image
								// 0: unset, else if owidth/oheight : aspect ratio >1 => landscape orientation
};
//------------------------------------------
struct Video : IABase			// format: MP4, OOG, WebM
{
	// each video file should be accomplished by a JPG file for the thumbnail
	// of the same name + ".jpg". Example: apple.mp4 and apple.mp4.jpg
	// If no such file is present a default jpg will be supplied
	enum Type {vtMp4, vtOgg, vtWebM} type;

	bool changed = false;		// true: video and thumbnail is recreated (dimensions or file size or data changed)
								// check this and create a new struct file on disk if any image changed
								// inside any albums. Alse set the album changed flag if any of its
								// images/thu,bnails, etc changed
	QString checksum = 0;		// of content not used YET
	QDate uploadDate;
	int64_t fileSize = 0;		// of source file, set together with 'exists' (if file does not exist fileSize is 0)
	enum SearchCond : int {
		byID,		// ID only
		byBaseID,	// ID for image w. o path: compare names (no path) as well
		byName,		// just image name
		byFullSourceName	// full image path
	};

	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name
	QString LinkName(bool bLCExtension = false) const		// used in HTML files, not the source name
	{
		if (ID)
		{
			int pos = name.lastIndexOf('.');
			QString s = name.mid(pos);
			if (bLCExtension)
				s = s.toLower();
			return QString().setNum(ID & ID_MASK) + s;	// e.g. 12345.mp4
		}
		else
			return name;
	}

	int operator<(const Video& i);		 // uses searchBy
	bool operator==(const Video &i);

	//IABase& operator=(const IABase& a)
	//{
	//	IABase::operator=(a);
	//	type = ((Video&)a).type;
	//}

	QString AsString(int width=320, int height=-1) 
	{ 
		const char* vs = "<video width=\"%1\" height=\"%2\" controls>/n"
						 " <source src=\"%3\" type=\"video/%4\">\n"
						 "</video>";
		if (height < 0)
			height = width * 1080 / 1920;

		switch (type)
		{
			case vtWebM: return QString(vs).arg(width).arg(height).arg(FullSourceName()).arg("webm");
			case vtOgg: return QString(vs).arg(width).arg(height).arg(FullSourceName()).arg("ogg");
			default:
			case vtMp4: return QString(vs).arg(width).arg(height).arg(FullSourceName()).arg("mp4");
		}
	}

};

//------------------------------------------
struct Album : IABase			// ID == 1+ALBUM_ID_FLAG root  (0: invalid)
{
	ID_t parent = 0;	// just a single parent is allowed Needed to re-generate parent's HTML files too when
						// this album changes. Must be modified when this album is moved into another one(**TODO**)
	IdList items;		// for all images, videos and albums in this album
	ID_t thumbnail = 0;	// image ID	or 0

	bool changed = false;	// set to true when: any text, album thumbnail, images, albums, exluded changed

	enum SearchCond {byID, byName};
	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Album &i);		 // uses searchBy
	bool operator==(const Album &i);

	bool Valid() const { return ID > 0; }
	void Clear() { items.clear(); _imageCount = _videoCount = _albumCount = -1; }
	int ImageCount(bool forced = false);		// only non-excluded existing images
	int VideoCount(bool forced=false);		// only non-excluded existing videos
	int SubAlbumCount(bool forced=false);	// only non excluded existing albums (removes excluded albums) = count of children
	int TitleCount();		// sets/returns titleCount
	int DescCount();		// sets/returns descCount
	ID_t IdOfItemOfType(int64_t type, int index, int startPos = 0);

	static QString NameFromID(ID_t id, int language, bool withAlbumPath);			// <basename><id><lang>.html
	QString NameFromID(int language);
	QString LinkName(int language, bool addHttpOrsPrefix = false) const;	// like https://<server URL>/<base name><ID><lang>.html
	QString BareName();			// '<base name>ID'
private:
	int _imageCount = -1;	// these are not equal to the count of the corresponding items
	int _albumCount = -1;	// because items could be excluded from display
	int _videoCount = -1;
	int _titleCount = -1;
	int _descCount  = -1;
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
	static QString lastUsedPath;	// config.dsSrc relative path to video so that we can add 

	Image& Find(ID_t id, bool useBase = true);
	Image& Find(QString FullSourceName);
	ID_t Add(QString image, bool &added);	// returns ID and if added
	Image &Item(int index);
};

//------------------------------------------
// key contains VIDEO_ID_FLAG
class VideoMap : public QMap<ID_t, Video>
{
	//friend class ALbumGenerator; miert nem volt eleg
	//ahhoz, hogy ezt lassa:	static Image invalid;
public:
	static Video invalid;
	static QString lastUsedPath;	// config.dsSrc relative path to video so that we can add 
									// an video by its name (relative to this path) only

	Video& Find(ID_t id, bool useBase = true);
	Video& Find(QString FullSourceName);
	ID_t Add(QString image, bool& added);	// returns ID and if added
	Video& Item(int index);
};

//------------------------------------------
// key contains ALBUM_ID_FLAG
class AlbumMap : public QMap<ID_t, Album>
{
	static Album invalid;
	// last used album path is in 'albumGenerator::lastUsedAlbumPath'
public:
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
	bool IsEmpty() const { return titleID == 0 && descID == 0 && thumbnailID == 0; }	// 'what' is unimportant
};

//------------------------------------------
class AlbumStructWriterThread;		// forward
class ProcessImageThread;

//------------------------------------------
class AlbumGenerator : public QObject
{
	friend class ProcessImageThread;

	Q_OBJECT

public:
	static QString lastUsedAlbumPath;	// config.dsSrc relative path to image so that we can add 
										// an image by its name (relative to this path) only
	AlbumGenerator() {};
	void Clear();
	bool Read(bool bMustReRead);	 // reads .struct or creates structure from folder hierarchy
	int Write();	 // writes album files into directory Config::sDestDir return error code or 0
	int WriteDirStruct(bool keep=false);		
	bool StructWritten() const { return _structWritten; }
	bool StructChanged() const { return _structChanged;  }
	int SaveStyleSheets();
	void SetRecrateAllAlbumFlag(bool Yes) { _mustRecreateAllAlbums = Yes; };

	void SetGalleryModified(ID_t albumId) { _slAlbumsModified << albumId; }

	static QString RootNameFromBase(QString base, int language, bool toServerPath = false);
	int ActLanguage() const { return _actLanguage; }
	
	int AlbumCount() const { return _albumMap.size(); }
	int ImageCount() const { return _imageMap.size(); }
	int VideoCount() const { return _videoMap.size(); }
	int TotalCount() const { return _albumMap.size() + _videoMap.size(); }
	int TextCount() const { return _textMap.size(); }
	static ID_t ThumbnailID(Album& album, AlbumMap& albums);
	// careful: these are not const so that their elements could be modified
	Album &AlbumRoot()  { return _root; }
	ImageMap &Images() { return _imageMap; }
	Image* ImageAt(ID_t id) { return &_imageMap[id]; }
	VideoMap& Videos() { return _videoMap; }
	Video* VideoAt(ID_t id) { return &_videoMap[id]; }
	TextMap &Texts()   { return _textMap;  }
	LanguageTexts *TextsAt(ID_t id) { return &_textMap[id]; }
	AlbumMap &Albums() { return _albumMap; }
	Album *AlbumForID(ID_t id) { return &_albumMap[id]; }
	QString SiteLink(int language);

	ID_t AddImageOrAlbum(Album &ab, QString path, bool isThumbnail=false, bool doSignalElapsedTime=true, bool doNotAddToAlbumItemList = false);
	bool AddImageOrVideoFromString(QString inpstr, Album& album, int pos = -1);

signals:
	void SignalToSetProgressParams(int min, int max, int pos, int phase);
	void SignalProgressPos(int cnt1, int cnt2);
	void SignalSetLanguagesToUI();
	void SignalToEnableEditTab(bool on);
//	void SignalImageMapChanged();
	void SignalAlbumStructChanged();
	void SignalToShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void SignalToCreateIcon(QString destPath, QString destName);
	void SetDirectoryCountTo(int cnt);
public:		// SLOT: connected with new syntax: no need for MOC to use this slot
	void Cancelled() { _processing = false; }

private:
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

	bool _processing = false;
	QList<ID_t> _slAlbumsModified;
	QString _upLink;		// to parent page if there's one
	TextMap _textMap;		// all texts for all albums and images
	AlbumMap _albumMap;		// all source albums			id has ALBUM_ID_FLAG set!
	ImageMap _imageMap;		// all images for all albums	id has IMAGE_ID_FLAG set!
	VideoMap _videoMap;		// all videos from all albums	id has ALBUM_ID_FLAG set!
	Album _root;			// top level album (first in '_albumMap', ID = 1+ALBUM_ID_FLAG)
		// --------- latest images collection
	QDate _latestDateLimit = QDate::fromJulianDay(0); // date of latest upload for generating the latest files
	struct LatestImages
	{
		int cntDescs =0;
		int cntTitles=0;
		IdList list;		// ID's for latest images/videos
		void Clear() { cntDescs = cntTitles = 0; list.clear(); }
		int size() const { return list.size(); }
	} _latestImages;
// no need: read is fast enough 	bool _structAlreadyInMemory = false;
	UsageCount _structChanged;	// signals wheather a new 'struct' file must be written
							// increase after a jalbum style read and after an album struct change
							// this is used to determine that actual album was changed or not
							// record its value before the changes and compare with this after the changes
							// only used as a bool value otherwise

	bool _isAJAlbum = false;
	//------------

	int _ItemSize() const { return _imageMap.size() + _videoMap.size(); }

	AlbumStructWriterThread *pWriteStructThread=nullptr;		// write structure in separate thread
	bool _structWritten = true;
	bool _mustRecreateAllAlbums = false;				// set to true when signaled from FalconG class that uplink etc changed
	bool _keepPreviousBackup = false; // rename temporary file ?

	int _actLanguage = 0;	// actual language used in album generation

	QTextStream _ofs, _ifs;		// write (read) data to (from) here

	bool _MustRecreateThumbBasedOnImageDimensions(QString thumbName, Image &img);
	QStringList _SeparateLanguageTexts(QString line);		  // helpers
	bool _IsExcluded(const Album& album, QString name);
	void _TitleFromPath(QString path, LangConstList &ltl);
	QString _HtaccessToString();
	QString _GoogleAnaliticsOn();
	QString _PageHeadToString(ID_t id);

	bool _CreateDirectories();	// from data in config

						// writing 

						  // reading (and copying) data
	bool _ReadFromDirsFile(Album &ab);		// albumfiles.txt
	bool _ReadJCommentFile(Album &ab);	// comments.properties
	bool _ReadJMetaFile(Album &ab);		// meta.properties
	void _JReadInfoFile(Album &ab, QString &path, QString name);	// '.info' files, add to _textMap and album or image title
	bool _JReadInfo(Album &ab);			// album and image titles in hidden .jalbum sub directories
	void _ReadOneLevelOfDirs(Album &ab);
	ID_t _AddImageOrAlbum(Album &ab, QFileInfo& fi, bool signalElapsedTime = true, bool doNotAddToAlbumItemList = false);
	ID_t _AddImageOrVideoFromPathInStruct(QString imagePath, FileTypeImageVideo ftyp, bool&added);

	bool _IsAlbumAndItsSubAlbumsEmpty(Album&);	// use inside _CleanupAlbums()
	void _CleanupAlbums();	// exclude empty albums and albums with only albums in them each empty
	
					// write gallery files
	void _WriteFacebookLink(QString linkName, ID_t ID);
	QString _IncludeFacebookLibrary();
	void _OutputNavForAboutPage(int lang);
	int  _OutputAboutText(int lang);
	void _OutputNav(Album &album, QString uplink);
	int _WriteHeaderSection(Album &album);
	int _WriteFooterSection(Album &album);
	int _WriteGalleryContainer(Album &album, ID_t typeFlag, int i);
	int _WriteVideoContainer(Album &album, int i);
	void _ProcessOneImage(Image &im, ImageConverter &converter, std::atomic_int &cnt);
	int _ProcessImages(); // into 'imgs' directory
	int _ProcessVideos(); // into 'vids' directory
	int _CreateOneHtmlAlbum(QFile &f, Album &album, int language, QString uplink, int &processedCount);
	int _CreatePage(Album &album, int language, QString parent, int &processedCount);
	int _CreateAboutPages();
	int _CreateAboutPageForLanguage(int lang);	// 
	int _CreateHomePage();

	int _DoCopyRes();	// copy directory 'res', modify png colors
	int _DoCopyJs();	// copy directory 'js'
	int _SaveFalconGCss();
	int _DoPages();
	// latest images
	int _CollectLatestImagesAndVideos(LatestImages &here);
	int _DoLatestJs();	// creates "javascript latest.js"
	int _DoLatestHelper(QString fileName, int lang);
	int _DoLatest();
	int _DoHtAccess();
				// read 'gallery.struct
	bool _LanguageFromStruct(FileReader &reader);
	ID_t _ImageOrVideoFromStruct(FileReader &reader, int level, Album &album, bool thumbnail);
	ID_t _ReadAlbumFromStruct(FileReader &reader, ID_t parent, int level);
	void _GetTextAndThumbnailIDsFromStruct(FileReader &reader, IdsFromStruct &ids, int level);
	bool _ReadStruct(QString from);	// from gallery.struct (first dest, then src directory) 

			// read 'Jalbum' file structure
	bool _ReadFromDirs();	//or if it does not exist loads them from supposedly jalbum directory dsSrc
	bool _ReadFromGallery();	// recrates album structure but can't recover album paths and image names or dimensions
private:
	void _WriteStructReady(QString s, QString sStructPath, QString sStructTmp);		// slot !
};

extern AlbumGenerator albumgen;
//------------------------------------------
#endif