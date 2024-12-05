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

const QString TITLE_TAG = "Title-";	// used in 'struct' files
const QString DESCRIPTION_TAG = "Descr-";	// e.g. [Tytle-hu:<text>]
const QString THUMBNAIL_TAG = "Icon";	// "album icon"
const QString ORPHAN_ID = "[Thumbnails not in any album";	// closed with a line only having a ']'
const QString PATH_TABLE = "[Path table";
//const QString SAVED_IMAGE_DESCRIPTIONS = "images.desc";
//const QString TEMP_SAVED_IMAGE_DESCRIPTIONS = "images.tmp";


/* =============== Id To Path and Path to Id Mapping ==================*/
class PathMap 
{
	typedef QMap<uint64_t, QString> base_type1;
	typedef QMap<QString, uint64_t> base_type2;

	base_type1 _idToPath;
	base_type2 _pathToId;

	uint64_t _CalcId(const QString & path);
	friend QTextStream& operator<<(QTextStream &ofs, const PathMap &map);

public:
	PathMap() {}

	uint64_t &Add(const QString &path);		// handles empty paths - returns NO_ID,
											// existing paths - return existing id
											// first cuts source folder path from path
											// path may end in a '/' which will be dropped ?

	int Size() const { return _pathToId.size(); }
	QString Path(uint64_t id) const;
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
	QString insert(uint64_t id, const QString &path);	// path must not be already in the map
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
	// QString path;		// either relative to confg.dsSrc or an absolute path. may be empty otherwise ends with '/'	  @ just until path ids implemented
	uint64_t pathId = NO_ID;// index in albumgen's IdToPathFrom() function

	IABase &operator=(const IABase &a)
	{
		ID		=  a.ID		;
		changed =  a.changed;
		titleID =  a.titleID;
		descID	=  a.descID	;
		name    =  a.name   ;
		pathId  =  a.pathId   ;
		return *this;
	}

	bool Valid() const { return ID.Val(); }
	bool Exists(bool bPhysicalCheck = DONTCHECK)
	{ 
		if (bPhysicalCheck)
			ID.SetFlag(EXISTING_FLAG, QFile::exists(FullSourceName()));
		return ID.TestFlag(EXISTING_FLAG); 
	}

	QString Extension() const { int pos = name.lastIndexOf('.'); return(pos >= 0 ? name.mid(pos) : ""); }
	IAType Type() const
	{
		if (ID.IsImage()) return iatImage;
		if (ID.IsVideo()) return iatVideo;
		if (ID.IsAlbum()) return iatAlbum;
		return iatUnknown;
	}
	QString FullSourceName() const;
	QString ShortSourcePathName() const;	// cuts the common part of paths (config.dsSrc)
	QString LinkName(bool bLCExtension = false) const;		// used in HTML files, not the source name
	QString FullLinkName(bool bLCExtension = false) const;		// used in HTML files, not the source name
	QString Path() const;		// returns path from ID

	uint64_t SetPathId(QString fromPath);
};
//------------------------------------------
struct Image : public IABase
{
	UsageCount usageCount = 0;	// image can be removed from disk only if both this is 0
	UsageCount thumbnailCount = 0;		// and this is also 0	Orphan thumbnails must have 0 usageCount
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

	bool	bDestFileChangedOrMissing;		// from width and height

	/*=============================================================
	 * TASK   :
	 * PARAMS : destSize: invalid (-1,-1) if destination does not exist
	 *						otherwise size of existing file
	 * EXPECTS: - file name osize and perhaps dsize and
	 *			  last upload time are set from database
	 *			- 
	 * GLOBALS:
	 * RETURNS: nothing
	 * REMARKS:	- sets 'rsize' to the required destination size
	 *			- sets 'bDestFileChangedOrMissing' true if 'rsize' and 'dsize' are different
	 *					or 'destSize' is different from 'rsize'
	 *------------------------------------------------------------*/
	void GetResizedDimensions(QSize& destSize)	// 'destSize' is size of existing destination image or -1,-1
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
		auto sizediff = [&](QSize& dSize) -> bool
			{
				return (abs(rsize.width() - dSize.width()) > 2) || (abs(rsize.height() - dSize.height()) > 2);
			};

			// and compare it with previous size (if any given)
			// dsize can be 0,0 if no destination image exists or specified
		bDestFileChangedOrMissing = !destSize.isValid() ||							// no dest. file: must create, or
							(dsize.isEmpty() && sizediff(destSize)) ||	// dsize is not set, but destSize is valid and set
							(!dsize.isEmpty() && sizediff(dsize));		// dsize is set but different from required size

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

	UsageCount usageCount = 1;			// can be removed when this is 0
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
};

//------------------------------------------
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
	int TitleCount();		// sets/returns titleCount
	int DescCount();		// sets/returns descCount
	ID_t ThumbID();			// returns ID of thumbnail recursively, sets it if not yet set
	ID_t SetThumbnail(ID_t id); // sets album thumbnail and increase the usage count of it
	ID_t SetThumbnail(uint64_t id); // sets album thumbnail and increase the usage count of it

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
	ID_t Add(QString image, bool &added, uint64_t orphanID=0);	// returns ID and if added
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
class AlbumMap : public QMap<ID_t, Album>
{
	static Album invalid;
	// last used album path is in 'albumGenerator::lastUsedAlbumPath'
	// root path is in config.dsSrc
public:
	Album *Find(ID_t id);
	Album *Find(QString albumPath);
	bool Exists(QString albumPath);
	ID_t Add(QString relativeAlbumPath, bool &added);			// add from 'relativeAlbumPath which can be an absolute path returns ID and if it was added
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
	AlbumGenerator() { Init();  };
	void Init();
	void Clear();
	void SetChangesWritten();
	void AddDirsRecursively(ID_t albumId);	// for existing album when structure modified
	void RecursivelyAddAlbums(ID_t albumId);
	bool Read(bool bMustReRead);	 // reads .struct or creates structure from folder hierarchy

	void RemoveItems(ID_t albumID, IntList ilx, bool fromdisk, bool iconsForThisAlbum = true);

	void AddThumbnailBeforeImageRead(ID_t id)
	{
		_addedThumbnailIDsForImagesNotYetRead.push_back(id);	// same id can be added any number of times
	}

	int ProcessAndWrite();	 // writes album files into directory Config::sDestDir return error code or 0
	int WriteDirStruct(bool doNotReplaceExistingBackupFile=false);		
	bool StructWritten() const { return _structWritten; }
	bool StructChanged() const { return _structChanged;  }
	int SaveStyleSheets();
	void SetRecrateAllAlbumsFlag(bool Yes) { _mustRecreateAllAlbums = Yes; };

	void SetAlbumModified(Album& album)
	{

		if (_slAlbumsModified.indexOf(album.ID) < 0)
		{
			_slAlbumsModified << album.ID;
			album.changed = true;
		}
	}
	void SetAlbumModified(ID_t albumId)		// albumId must be valid
	{ 
		if (_slAlbumsModified.indexOf(albumId) < 0)
		{
			Album& album = _albumMap[albumId];
			SetAlbumModified(album);
		}
	}

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
	Image* ImageAt(ID_t id) 
	{ 
		return &_imageMap[id]; 
	}
	VideoMap& Videos() { return _videoMap; }
	Video* VideoAt(ID_t id) { return &_videoMap[id]; }
	TextMap &Texts()   { return _textMap;  }
	LanguageTexts *TextsAt(int64_t id) { return &_textMap[id]; }
	AlbumMap &Albums() { return _albumMap; }

	Album *AlbumForID(ID_t id) 
	{ 
		id.SetFlag(EXISTING_FLAG, false);
		return &_albumMap[id]; 
	}
	IABase* IdToItem(ID_t id)
	{
		IABase* pItem = nullptr;
		if (id.IsImage())
			pItem = ImageAt(id);
		else if (id.IsVideo())
			pItem = VideoAt(id);
		else
			pItem = AlbumForID(id);

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
	UsageCount _structChanged;	// signals wheather a new 'struct' file must be written
							// increase after a jalbum style read and after an album struct change
							// this is used to determine that actual album was changed or not
							// record its value before the changes and compare with this after the changes
							// only used as a bool value otherwise

	bool _isAJAlbum = false;
	//------------

	int _ItemSize() const { return _imageMap.size() + _videoMap.size(); }

	AlbumStructWriterThread *pWriteStructThread=nullptr;		// write structure in separate thread
	volatile bool _structWritten = true;
	bool _mustRecreateAllAlbums = false;				// set to true when signaled from FalconG class that uplink etc changed
	bool _keepPreviousBackup = false; // rename temporary file ?

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
	bool _ReadFromJAlbumOrderFile(ID_t albumId);		// albumfiles.txt
	bool _ReadJAlbumCommentFile(ID_t albumId);	// comments.properties
	bool _ReadJAlbumMetaFile(ID_t albumId);		// meta.properties
	void _ReadJAlbumInfoFile(ID_t albumId, QString &path, QString name);	// '.info' files, add to _textMap and album or image title
	bool _ReadJAlbumInfoFile(ID_t albumId);			// album and image titles in hidden .jalbum sub directories
	void _RecursivelyReadSubAlbums(ID_t albumId);
	ID_t _AddItemToAlbum(ID_t albumId, QFileInfo& fi, bool signalElapsedTime = true, bool doNotAddToAlbumItemList = false);
	ID_t _AddImageOrVideoFromPathInStruct(QString imagePath, FileTypeImageVideo ftyp, bool&added);

	bool _IsAlbumAndItsSubAlbumsEmpty(Album&);	// use inside _CleanupAlbums()
	void _CleanupAlbums();	// exclude empty albums and albums with only albums in them each empty
	
					// write gallery files
	void _WriteFacebookLink(QString linkName, ID_t ID);
	QString _IncludeFacebookLibrary() const;
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
	int _DoLatestJs();	// creates "javascript latest.js"
	int _DoLatestHelper(QString fileName, int lang);
	int _DoLatest();
	int _DoHtAccess();
				// read 'gallery.struct
	bool _LanguageFromStruct(FileReader &reader);
	ID_t _ImageOrVideoFromStruct(FileReader &reader, int level, Album *album, bool thumbnail);
	ID_t _ReadAlbumFromStruct(FileReader &reader, ID_t parentId, int level);
	void _AddAlbumThumbnail(Album &album, uint64_t id);
	void _GetTextAndThumbnailIDsFromStruct(FileReader &reader, IdsFromStruct &ids, int level);
	bool _ReadPathTable(FileReader& reader);
	bool _ReadOrphanTable(FileReader& reader);
	bool _ReadStruct(QString from);	// from gallery.struct (first dest, then src directory) 

			// read 'Jalbum' file structure
	bool _ReadFromDirs();	//or if it does not exist loads them from supposedly jalbum directory dsSrc
	bool _ReadFromGallery();	// recrates album structure but can't recover album paths and image names or dimensions
private:
	void _WriteStructReady(QString s, QString sStructPath, QString sStructTmp);		// slot !
	void _RemoveItem(ID_t id, bool fromdisk);
	void _RemoveItems(ID_t albumID, bool iconsForThisAlbum, IntList ilx, bool fromdisk);
	void _RemoveAllItems(ID_t albumID, bool fromdisk);
};

extern AlbumGenerator albumgen;
//------------------------------------------
#endif