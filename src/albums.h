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
#ifdef DEBUG
	#include <QtDebug>
#endif

#include "enums.h"
#include "support.h"
#include "config.h"
#include "crc32.h"
#include "stylehandler.h"
#include "languages.h"

const QString OLD_TITLE_TAG = "Title-";	// used in 'struct' files
const QString OLD_DESCRIPTION_TAG = "Descr-";	// e.g. [Tytle-hu:<text>]
const QString OLD_THUMBNAIL_TAG = "Icon";	// "album icon"

const QString TITLE_TAG = "T-";			// used in 'struct' files
const QString DESCRIPTION_TAG = "D-";	// e.g. [Tytle-hu:<text>]
const QString THUMBNAIL_TAG = "I";		// "album icon"

const QString ORPHAN_ID = "[Thumbnails not in any album";	// closed with a line only having a ']'
const QString PATH_TABLE = "[Path table";
//const QString SAVED_IMAGE_DESCRIPTIONS = "images.desc";
//const QString TEMP_SAVED_IMAGE_DESCRIPTIONS = "images.tmp";


/* =============== Id To Path and Path to Id Mapping ==================*/
class PathMap 
{
	typedef QMap<uint64_t, QString> base_type1;
	typedef QMap<QString, uint64_t> base_type2;

	typedef base_type1::iterator iterator1;
	typedef base_type2::iterator iterator2;

	base_type1 _idToPath;
	base_type2 _pathToId;
	iterator1 _it1;
	iterator2 _it2;

	uint64_t _CalcId(const QString & path);
	friend QTextStream& operator<<(QTextStream &ofs, const PathMap &map);

public:
	PathMap() {}

	uint64_t Add(const QString &path);		// handles empty paths - returns NO_ID,
											// existing paths - return existing id
											// first cuts source folder path from path
											// path may end in a '/' which will be dropped ?

	int Size() const { return _pathToId.size(); }
	QString AbsPath(uint64_t id) const;		// differs from operator[] as it adds source path if needed
	uint64_t Id(QString path) const;
	inline bool Contains(const QString &path) const
	{
		return _pathToId.contains(path);
	}
	inline bool Contains(uint64_t id) const
	{
		return _idToPath.contains(id);
	}

	uint64_t &operator[](const QString &pPath);
	QString &operator[](const uint64_t id);
	//uint64_t insert(QString path, uint64_t id);	// if id is an existing id then path is replaced only
	QString Insert(uint64_t id, const QString &path);	// path must not be already in the map
	QString Insert(const QString ids, const QString &path);	// path must not be already in the map
	void Remove(uint64_t id);	// removes both path and id
	uint64_t FirstId();	// returns the first id from _idToPath
	uint64_t NextId();	// returns the next id from _idToPath
};

QTextStream& operator<<(QTextStream &ofs, const PathMap &map);
// globals
extern PathMap pathMap;	// paths stored here are either absolute paths or 
						// relative to root folder. The root folder is stored
						// in config.dsSrc and not here


//------------------------ base class for albums and images ------------------
struct IABase
{
	enum IAType { iatUnknown, iatImage, iatVideo, iatAlbum};

	enum SearchCond : int { byID,		// ID only
							byBaseID,	// ID for image w. o path: compare names (no path) as well
							byName,		// just image name
							byFullSourceName	// full image path
						  };

	ID_t ID;				// CRC of image name + collision avoidance  (0: invalid)
	bool changed = false;	// Images (or videos): true: image is recreated (dimensions or file size or data changed)
							//		check this and create a new struct file on disk if any image changed
							//		inside any albums. 
							// Alse set the album changed flag if any of its
							//		images/thumbnails, etc changed
							//		some image names may come from comment files
							// albums: set to true when any text, album thumbnail, images, albums, exluded changed
	int64_t titleID = NO_ID;// default text ID of image title
	int64_t descID = NO_ID;	// default text ID of image description
	QString name;			// without path but with extension and no ending '/' even for albums
	
	uint64_t pathId = NO_ID;// index in albumgen's IdToPathFrom() function
	// the actual and the previous value of the index that determines which directory this item is to be stored
	// If it isn't 0 it is stored in the .struct file for all items after the ID and a trailing letter 'i'
	// // E.g. if the ID is 123456789 and dirIndex = 7 then the ID is stored as 123456789i7
	// Recalculated at each album generation and if changed image and video files that changed dirIndex
	// but didn't re-generated are moved from one directory to another on disk
	// TODO
	uint dirIndex = 0,		
		lastDirIndex = 0;	// == 0: directories: albums: albums/album<ID>_<lang>.html, images: imgs/<ID>.jpg, thumbs: thumbs/<ID>.jpg, videos: vids/<ID>.mp4
							//  > 0:			  albums: album<dirIndex>/album<ID>.html, images: imgs<dirIndex>/<ID>.jpg, 
							//						thumbs: thumbs<dirIndex>/<ID>.jpg, videos: vids<dirIndex>/<ID>.mp4

	IABase& operator=(const IABase& a);

	bool Valid() const;
	bool Exists(bool bPhysicalCheck = DONTCHECK);

	QString Extension() const;
	IAType Type() const;
	QString FullSourceName() const;
	QString ShortSourcePathName() const;	// cuts the common part of paths (config.dsSrc)
	QString LinkName(bool bLCExtension = false) const;		// used in HTML files, not the source name
	QString FullLinkName(bool bLCExtension = false) const;		// used in HTML files, not the source name
	QString Path() const;		// returns real or virtual path from pathID (can't be const)

	virtual QSize ThumbSize() const { return QSize(); }
};
//------------------------------------------
struct Image : public IABase
{
	UsageCount usageCount = 0;		// image can be removed from disk only if both this 
	UsageCount thumbnailCount = 0;	// and this are 0	Orphan thumbnails must have 0 usageCount
	QString checksum = 0;		// of content not used YET
	bool dontResize = false;	// when image name is preceeded by double exclamation marks: !!
								// either in the original path (this/image/!!notResized.jpg) or 
								// in the precessed line (!!notresized(...)this/image/)
	QDate uploadDate;
	int64_t fileSize=0;			// of source file, set together with 'exists' (if file does not exist fileSize is 0)
	QSize	osize = { 0, 0 },	// original dimensions read from struct file,
			dsize = { 0, 0 },	// transformed size read either from .struct file or from destination file if it exists
			rsize = { 0, 0 },	// image to be resized to this size
			tsize;				// thumbnail size. Determined in SetThumbSize(). Not stored in struct file

	bool	bDestFileChangedOrMissing;		// from width and height

	void GetResizedDimensions(QSize& destSize);
	void SetThumbSize();		// calculates the size of the thumbnail when the height equals to config.thumbHeight

	void SetNewDimensions();

	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Image &i);		 // uses searchBy
	bool operator==(const Image& i);
	double Image::Aspect();
	double Image::ThumbAspect() const;

	QTextStream & WriteInfo(QTextStream &ofs) const;
	void SetResizeType();
	bool IsOrphan() const { return thumbnailCount && !usageCount;  }
	QSize ThumbSize() const override { return tsize; }

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

	UsageCount usageCount = 1;	// video can be deleted when this is 0
	QString checksum = 0;		// of content not used YET
	QDate uploadDate;
	ID_t  thumbnailId = { IMAGE_ID_FLAG, 0 };		// manually selected image as a thumbnail for this video
	int64_t fileSize = 0;		// of source file, set together with 'exists' (if file does not exist fileSize is 0)

	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Video& i);		 // uses searchBy
	bool operator==(const Video &i);

	//IABase& operator=(const IABase& a)
	//{
	//	type = ((Video&)a).type;
	//	usageCount	= a.usageCount	;			// can be removed when this is 0
	//	checksum	= a.checksum	;		// of content not used YET
	//	uploadDate	= a.uploadDate	;
	//	fileSize	= a.fileSize	;		// of source file, set together with 'exists' (if file does not exist fileSize is 0)
	//}

	QString AsString(int width = 320, int height = -1);
	QSize ThumbSize() const override;
};

//------------------------------------------
// A gallery is a hierarchy of albums which may contain other albums images and videos.
// It resides in a physical folder on disk. A gallery folder contains the following:
// - a '<name>.struct' file, a database which contains the structure of the gallery, and
// - a '<name>.ini' file which contains the configuration of the gallery.
// the <name> is (initially) the name of the gallery folder.
// All albums in this hierarchy are considered virtual folders in the data base
// even when they correspond to real folders on disk.
// The albums in this hierarchy are identified by their ID which is a CRC32 of their path name
// which is, for albumms that have physical folders, is the absolute path name of these folders.
// Album folders may be outside the gallery folder. For virtual albums the path name is the
// <absolute path of source folder>/<random 10 character string>/<logical name of album>
// The random 10 character string is only used to calculate the ID of the album.
// In the '.struct' file pure virtual albums (i.e. albums which have no physical presence on disk) 
// have a pathID of 0.


// The topmost album is the root album. It has no parent and its ID is 0
// all albums (even when they correspond to real folders on disk)
// are virtual. No folder is created when they are created or moved 
// when they are moved.

struct Album : IABase			// ID == TOPMOST_ALBUM_ID root  (0: invalid)
{
	ID_t parentId = { ALBUM_ID_FLAG, 0 };	// just a single parent is allowed Needed to re-generate parent's HTML files too when
						// this album changes. Must be modified when this album is moved into another one(**TODO**)
	ID_t thumbnailId = ID_t(IMAGE_ID_FLAG, 0);	// image ID	or 0

	IdList items;		// for all images, videos and albums in this album GET item for position using IdOfItem!!
	ID_t IdOfItem(int pos) const 
	{ 
		return items.isEmpty() ||  (pos >= items.size()) ?  ID_t::Invalid() : items[pos];
	}

	enum SearchCond {byID, byName};
	static SearchCond searchBy;	// 0: by ID, 1: by name, 2 by full name

	int operator<(const Album &i);		 // uses searchBy
	bool operator==(const Album &i);

	bool Valid() const { return ID.Val() != NO_ID; }

	void Clear() { items.clear(); _imageCount = _videoCount = _albumCount = -1; }
	int ImageCount(bool forced = false);		// only non-excluded existing images
	int VideoCount(bool forced=false);		// only non-excluded existing videos
	int SubAlbumCount(bool forced=false);	// only non excluded existing albums (removes excluded albums) = count of children
	void RecalcItemCounts();

	void DecrementImageCount(int n = 1) { _imageCount -= n; if (_imageCount < 0)_imageCount = 0; }
	void DecrementVideoCount(int n = 1) { _videoCount -= n; if (_videoCount < 0)_videoCount = 0; }
	void DecrementAlbumCount(int n = 1) { _albumCount -= n; if (_albumCount < 0)_albumCount = 0; }

	int TitleCount();		// sets/returns titleCount
	int DescCount();		// sets/returns descCount
	ID_t ThumbID();			// returns ID of thumbnail recursively, sets it if not yet set
	ID_t SetThumbnail(ID_t id); // sets album thumbnail and increase the usage count of it
	ID_t SetThumbnail(uint64_t id); // sets album thumbnail and increase the usage count of it
	QSize ThumbSize() const override;

	ID_t IdOfItemOfType(int64_t type, int index);

	static QString NameFromID(ID_t id, int language, bool withAlbumPath);			// <basename><id><lang>.html
	QString NameFromID(int language);
	QString LinkName(int language, bool addHttpOrHttpsPrefix = false) const;	// like https://<server URL>/<base name><ID><lang>.html
	QString BareName();			// '<base name>ID'

	void AddItem(ID_t id, int pos = -1);	// -1: append
private:
	int _imageCount = -1;	// these are not equal to the count of the corresponding items
	int _albumCount = -1;	// because items could be excluded from display
	int _videoCount = -1;
	int _titleCount = -1;
	int _descCount  = -1;
};
//------------------------------------------
class ImageMap : public QMap<ID_t, Image>
{
	//friend class ALbumGenerator; miert nem volt eleg
	//ahhoz, hogy ezt lassa:	static Image invalid;
public:
	static Image invalid;
	static uint64_t lastUsedPathId;	// config.dsSrc relative path to image so that we can add 

	Image *Find(ID_t id, bool useBase = true);
	Image *Find(QString FullSourceName);
	ID_t Add(QString image, bool &added, bool isThumbnail=false);	// returns ID and if added
	void Remove(ID_t id, bool isThumbnail);
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
	static uint64_t lastUsedPathId;	// config.dsSrc relative path to image so that we can add 
									// an video by its name (relative to this path) only
	Video *Find(ID_t id, bool useBase = true);
	Video *Find(QString FullSourceName);
	ID_t Add(QString image, bool& added);	// returns ID and if added
	Video& Item(int index);
};

//------------------------------------------
// key contains ALBUM_ID_FLAG
// all albums (even when they correspond to real folders on disk)
// are virtual.
class AlbumMap : public QMap<ID_t, Album>
{
	static Album invalid;
	// last used album path is in 'albumGenerator::lastUsedAlbumPath'
	// root path is in config.dsSrc
public:
	Album *AlbumForID(ID_t id) 
	{ 
		id.SetFlag(EXISTING_FLAG, false);
		Album* pAlbum = nullptr;
		if (contains(id))
			pAlbum = &(*this)[id];
		return pAlbum;
	}
	Album *Find(ID_t id);
	Album *Find(QString albumFullPath);
	Album *Find(uint64_t pathID, QString albumName);
	Album *Find(uint64_t pathID, ID_t albumId);
	bool Exists(QString albumPath);
	ID_t Add(ID_t parentId, const QString &name, bool &added);			// add from 'relativeAlbumPath which can be an absolute path returns ID and if it was added
	Album &Item(int index);
	bool RemoveRecursively(ID_t id);		// album and it sll sub-albums
};

struct IdsFromStruct
{
	enum IDS { nothing, title, description, thumbnail } what = nothing;

	int64_t titleIDVal = 0, descIDVal = 0, thumbnailIDVal = 0;

	void Clear() { titleIDVal = descIDVal = thumbnailIDVal = 0; what = nothing; }
	bool IsEmpty() const { return titleIDVal == 0 && descIDVal == 0 && thumbnailIDVal == 0; }	// 'what' is unimportant
};

//------------------------------------------
class AlbumStructWriterThread;		// forward
class ProcessImageThread;
/* =============== Albumgenerator ==================*/

class AlbumGenerator : public QObject
{
	friend class ProcessImageThread;

	Q_OBJECT

public:
	static uint64_t lastUsedAlbumPathId;	// config.dsSrc relative path to image so that we can add 
											// an image by its name (relative to this path) only
	enum class BackupMode {bmKeep, bmReplace};
	enum class WriteMode {wmOnlyIfChanged, wmAlways};
	AlbumGenerator() { Init();  };
	void Init();
	void Clear();
	bool SetChangesWritten();
	void AddDirsRecursively(ID_t albumId);	// for existing album when structure modified
	void RecursivelyAddAlbums(ID_t albumId);
	bool Read(bool bMustReRead);	 // reads .struct or creates structure from folder hierarchy

	void RemoveItems(ID_t albumID, IntList ilx, bool fromdisk, bool iconsForThisAlbum = true);

	void AddThumbnailBeforeImageRead(ID_t id)
	{
		_addedThumbnailIDsForImagesNotYetRead.push_back(id);	// same id can be added any number of times
	}

	int ProcessAndWrite();	 // writes album files into directory Config::sDestDir return error code or 0
	int WriteDirStruct(BackupMode bm=BackupMode::bmKeep, WriteMode wm=WriteMode::wmOnlyIfChanged);		
	bool StructWritten() const { return !_structIsBeingWritten; }
	bool StructChanged() const { return _structFileChangeCount;  }
	void SetStructChanged(bool val) { _structFileChangeCount = (val ? 1 : 0); }
	int SaveStyleSheets();
	void SetRecrateAllAlbumsFlag(bool Yes) { _mustRecreateAllAlbums = Yes; };

	void SetAlbumModified(Album& album);
	void SetAlbumModified(ID_t albumId);		// albumId must be valid

	static QString RootNameFromBase(QString base, int language, bool toServerPath = false);
	int ActLanguage() const { return _actLanguage; }
	
	int AlbumCount() const { return _albumMap.size(); }
	int ImageCount() const { return _imageMap.size(); }
	int VideoCount() const { return _videoMap.size(); }
	int TotalCount() const { return _albumMap.size() + _videoMap.size(); }
	int TextCount() const { return _textMap.size(); }
	static ID_t ThumbnailID(Album& album, AlbumMap& albums);
	// careful: these are not const so that their elements could be modified
	Album& RootAlbum() { return _albumMap[TOPMOST_ALBUM_ID]; }
	ImageMap &Images() { return _imageMap; }
	bool Contains(ID_t id)	// inside albumgen
	{	
		return ( (id.IsAlbum()) && _albumMap.contains(id)  ||
				 (id.IsImage()) && _imageMap.contains(id)  ||
				 (id.IsVideo()) && _videoMap.contains(id) ) ;
	}
	Album* AlbumForID(ID_t id)
	{
		return _albumMap.AlbumForID(id);
	}
	Image* ImageAt(ID_t id) 
	{ 
		return &_imageMap[id]; 
	}
	VideoMap& Videos() { return _videoMap; }
	Video* VideoAt(ID_t id) { return &_videoMap[id]; }
	TextMap &Texts()   { return _textMap;  }
	LanguageTexts *TextsAt(int64_t id) { return &_textMap[id]; }
	AlbumMap &Albums() { return _albumMap; }

	IABase* IdToItem(ID_t id)
	{
		IABase* pItem = nullptr;
		if (id.IsImage())
			pItem = ImageAt(id);
		else if (id.IsVideo())
			pItem = VideoAt(id);
		else
			pItem = _albumMap.AlbumForID(id);

		return pItem;
	}
	QString SiteLink(int language);

	ID_t AddItemToAlbum(ID_t albumId, QString path, bool isThumbnail=false, bool doSignalElapsedTime=true, bool doNotAddToAlbumItemList = false);
	bool AddImageOrVideoFromString(QString inpstr, Album& album, int pos = -1);

signals:
	void SignalToSetProgressParams(int min, int max, int pos, int phase);
	void SignalProgressPos(int cnt1, int cnt2);
	void SignalSetupLanguagesToUI();
	void SignalToEnableEditTab(bool on);
//	void SignalImageMapChanged();
	void SignalAlbumStructWillChange();
	void SignalAlbumStructChanged(bool yes);
	void SignalToShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void SignalToCreateUplinkIcon(QString destPath, QString destName);
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
	bool _signalProgress = true;
	QList<ID_t> _slAlbumsModified;
	QString _upLink;		// to parent page if there's one

	TextMap	 _textMap;		// all texts for all albums and images
	AlbumMap _albumMap;		// all source albums			ID.flag has ALBUM_ID_FLAG
	ImageMap _imageMap;		// all images for all albums	ID.flag has IMAGE_ID_FLAG and possibly ORPHAN_FLAG
	VideoMap _videoMap;		// all videos from all albums	ID.flag has ALBUM_ID_FLAG

	IdList	_addedThumbnailIDsForImagesNotYetRead;
	// ID of top level album (first in '_albumMap', TOPMOST_ALBUM_ID)
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
	UsageCount _structFileChangeCount;	// signals whether a new 'struct' file must be written
							// increase after a jalbum style read and after an album struct change
							// this is used to determine that actual album was changed or not
							// record its value before the changes and compare with this after the changes
							// only used as a bool value otherwise

	bool _isAJAlbum = false;
	//------------

	int _ItemSize() const { return _imageMap.size() + _videoMap.size(); }
									  // structure write
	AlbumStructWriterThread *pWriteStructThread=nullptr;// write structure in separate thread
	volatile bool _structIsBeingWritten = false;		// set true before write and false when writing is finished
	bool _keepPreviousBackup	= false;				// do not overwrite backup
	bool _mustRecreateAllAlbums = false;				// set to true when signaled from FalconG class that uplink etc changed

	int _actLanguage = 0;	// actual language used in album generation

	QString _EncodeTitle(QString title);

	QTextStream _ofs, _ifs;		// write (read) data to (from) here

	bool _MustRecreateThumbBasedOnImageDimensions(QString thumbName, Image &img);
	QStringList _SeparateLanguageTexts(QString line);		  // helpers
	bool _IsExcluded(const Album& album, QString name);
	void _TitleFromPath(QString path, LangConstList &ltl);
	QString _HtaccessToString();
	QString _GoogleAnaliticsOn();
	QString _PageHeadToString(const Album& album);

	bool _CreateDirectories();	// from data in config

						// writing 

						  // reading (and copying) data
	bool _JAlbumReadOrderFile(ID_t albumId);		// albumfiles.txt
	bool _JAlbumReadCommentFile(ID_t albumId);	// comments.properties
	bool _JAlbumReadMetaFile(ID_t albumId);		// meta.properties
	void _JAlbumReadInfoFile(ID_t albumId, QString &path, QString name);	// '.info' files, add to _textMap and album or image title
	bool _JAlbumReadInfoFile(ID_t albumId);			// album and image titles in hidden .jalbum sub directories
	void _RecursivelyReadSubAlbums(ID_t albumId);
	ID_t _AddItemToAlbum(ID_t albumId, QFileInfo& fi, bool signalElapsedTime = true, bool doNotAddToAlbumItemList = false);
	ID_t _AddImageOrVideoFromPathInStruct(QString imagePath, FileTypeImageVideo ftyp, bool&added);

	bool _IsAlbumAndItsSubAlbumsEmpty(Album&);	// use inside _CleanupAlbums()
	void _CleanupAlbums();	// exclude empty albums and albums with only albums in them each empty
	
					// write gallery files
	void _WriteFacebookLink(QString linkName, ID_t ID);
	QString _IncludeFacebookLibrary() const;
	void _LightboxCodeIntoHtml(int cntItems);
	void _OutputNavForAboutPage(int lang);
	int  _OutputAboutText(int lang);
	void _OutputNav(Album &album, QString uplink);
	int _WriteHeaderSection(Album &album);
	int _WriteFooterSection(Album &album);
	int _WriteGalleryContainer(Album &album, uint8_t typeFlag, int i);
	int _WriteVideoContainer(Album &album, int i);
	void _ProcessOneImage(Image &im, ImageConverter &converter, std::atomic_int &cnt);
	int _ProcessImages(); // into 'imgs' directory
	int _ProcessVideos(); // into 'vids' directory
	int _CreateOneHtmlAlbum(QFile &f, Album &album, int language, QString uplink, int &processedCount);
	int _CreatePage(Album &album, int language, QString parent, int &processedCount);
	int _CreateAboutPages();
	int _CreateHomePage();

	int _DoCopyRes();	// copy directory 'res', modify png colors
	int _DoCopyJs();	// copy directory 'js'
	int _SaveFalconGCss();
	int _DoPages();
	int _CleanUpOutput();	// remove files from output that were removed in input
	// latest images
	int _CollectLatestImagesAndVideos(LatestImages &here);
	int _DoLatestJs();	// creates language dependent "javascript latest_XX.js"
	int _DoLatestHelper(QString fileName, int lang);
	int _DoLatest();
	int _DoHtAccess();
				// read 'gallery.struct
	bool _LanguageFromStruct(FileReader &reader);
	ID_t _ReadImageOrVideoFromStruct(FileReader &reader, int level, Album *album, bool thumbnail);
	ID_t _ReadAlbumFromStruct(FileReader &reader, ID_t parentId, int level);
	void _AddAlbumThumbnail(Album &album, uint64_t id);
	void _GetTextAndThumbnailIDsFromStruct(FileReader &reader, IdsFromStruct &ids, int level);
	bool _ReadPathTable(FileReader& reader);
	void _CleanupPathTable();	// remove unused paths from pathMap
	bool _ReadOrphanTable(FileReader& reader);
	bool _ReadStruct(QString from);	// from gallery.struct (first dest, then src directory) 

			// read 'Jalbum' file structure
	bool _ReadFromDirs();	//or if it does not exist loads them from supposedly jalbum directory dsSrc
	bool _ReadFromGallery();	// recrates album structure but can't recover album paths and image names or dimensions
private:
	void _WriteStructReady(QString s, QString sStructPath, QString sStructTmp);		// slot !
	void _RemoveItem(Album &album, int which, bool fromdisk); // which: index in album's items
	void _RemoveItems(ID_t albumID, bool iconsForThisAlbum, IntList ilx, bool fromdisk);
	void _RemoveAllItemsFrom(ID_t albumID, bool fromdisk);
};

extern AlbumGenerator albumgen;
//------------------------------------------
#endif