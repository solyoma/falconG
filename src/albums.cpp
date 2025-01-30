#include <QApplication>
#include <QApplication>
#include <QtCore>
#include <QString>
#include <QTextCodec>
#include <time.h>
#include <atomic>
#include <chrono>

#include "enums.h"
#include "albums.h"
#include "falcong.h"
#include "csscreator.h"
#include "structwriter.h"

#if QT_VERSION < 0x051000
    #define created created
#endif

PathMap pathMap;	// id -> path, path->id of source folders relative to dsSrc

static QSize ThumbSizeFromId(ID_t id)	//id: thumbnail id
{
	Image* pI = albumgen.Images().Find(id);
	if (pI)
	{
		if (!pI->tsize.isValid())
			pI->SetThumbSize();
		return pI->tsize;
	}
	else
		return QSize();
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
static uint64_t CalcCrc(const QString& str, bool isContent)
{
	uint64_t id;
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

		// ******************* PathMap ********************
uint64_t PathMap::_CalcId(const QString&path)
{
	uint64_t id = CalcCrc(path, false);
	while (_idToPath.contains(id))
		id += ID_INCREMENT;
	return id;
}

uint64_t PathMap::Add(const QString &path) // only new paths added
{
	uint64_t id;

	if (path.isEmpty())
		return id = NO_ID;
	QString p = CutSourceRootFrom(path);
	if (p.isEmpty())
		return id = TOPMOST_PATH_ID;

	if (_pathToId.contains(p))
		return _pathToId[p];

	id =  _CalcId(p);
	_pathToId.insert(p, id);
	_idToPath.insert(id, p);
	return id;
}

QString PathMap::AbsPath(uint64_t id) const
{
	QString path;
	if (_idToPath.contains(id))
	{
		path = _idToPath[id];
		if (QDir::isAbsolutePath(path))
			return path;
	}
	return (config.dsSrc + path).ToString();
}

uint64_t PathMap::Id(QString path) const
{
	if (_pathToId.contains(path))
		return _pathToId[path];
	return NO_ID;
}

//inline bool PathMap::Contains(const QString &path) const
//{
//	return _pathToId.contains(path);
//}
//
//inline bool PathMap::Contains(uint64_t id) const
//{
//	return _idToPath.contains(id);
//}

uint64_t &PathMap::operator[](const QString &path)
{
	static uint64_t dummy;
	if (path.isEmpty())
		return dummy;

	if (!_pathToId.contains(path))
		Add(path);

	return _pathToId[path];
}

QString &PathMap::operator[](uint64_t id)
{   
	//static QString dummy;

	//if( (id <= TOPMOST_PATH_ID) || !_idToPath.contains(id))
	//	return dummy;
	
	return _idToPath[id];
}


QString PathMap::Insert(uint64_t id, const QString& path)	// same id new path
{
	if (Contains(id))	// then replace the path
		_pathToId.remove(AbsPath(id));

	_idToPath.insert(id, path);	 // add or possibly replaces path
	_pathToId.insert(path, id);	 // new path with this id

	return path;
}

QString PathMap::Insert(const QString ids, const QString& path)
{
	int64_t tid = ids.toULongLong();
	_idToPath.insert(tid, path);	 // add or possibly replaces path
	_pathToId.insert(path, tid);	 // new path with this id

	return path;
}

/*=============================================================
 * TASK   :	remove an item from both maps
 * PARAMS :	id: id of item to remove
 * EXPECTS:
 * GLOBALS:
 * RETURNS:none
 * REMARKS:
 *------------------------------------------------------------*/
void PathMap::Remove(uint64_t id)
{
	if (_idToPath.contains(id))
	{
		QString path = _idToPath[id];
		_idToPath.remove(id);
		_pathToId.remove(path);
	}
}

uint64_t PathMap::FirstId()
{
	if (_idToPath.size())
	{
		_it1 = _idToPath.begin();
		return _it1.key();
	}
	return 0;
}

uint64_t PathMap::NextId()
{
	if (_it1 == _idToPath.end() || ++_it1 == _idToPath.end())
		return 0;
	return (_it1).key();
}

// ************* PathMap output *******************
QTextStream& operator<<(QTextStream& ofs, const PathMap& map)
{
	for (PathMap::base_type1::const_iterator i = map._idToPath.constBegin(); i != map._idToPath.constEnd(); ++i)
		ofs << i.key() << '|' << i.value() << "\n";
	return ofs;
}

// ****************** IABase ***************

IABase& IABase::operator=(const IABase& a)
{
	ID = a.ID;
	changed = a.changed;
	titleID = a.titleID;
	descID = a.descID;
	name = a.name;
	pathId = a.pathId;
	return *this;
}

bool IABase::Valid() const { return ID.Val(); }
bool IABase::Exists(bool bPhysicalCheck)
{
	if (bPhysicalCheck)
		ID.SetFlag(EXISTING_FLAG, QFile::exists(FullSourceName()));
	return ID.TestFlag(EXISTING_FLAG);
}

QString IABase::Extension() const { int pos = name.lastIndexOf('.'); return(pos >= 0 ? name.mid(pos) : ""); }
IABase::IAType IABase::Type() const
{
	if (ID.IsImage()) return iatImage;
	if (ID.IsVideo()) return iatVideo;
	if (ID.IsAlbum()) return iatAlbum;
	return iatUnknown;
}

QString IABase::FullSourceName() const
{
	QString path = Path();
	return config.AddSourceToPath(path + name);				// does not end with '/'
}

/*============================================================================
  * TASK:	 Returns the path name relative to dsSrc
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString IABase::ShortSourcePathName() const
{
	QString path = Path();
	return config.RemoveSourceFromPath(path + name);
}

QString IABase::LinkName(bool bLCExtension) const
{
	if (ID.Val())
	{
		int pos = name.lastIndexOf('.');
		QString ext = name.mid(pos);
		if (bLCExtension)
			ext = ext.toLower();
		return QString().setNum(ID.Val()) + ext;	// e.g. 12345.jpg (images IDs has no flag set)
	}
	else
		return name;
}

QString IABase::FullLinkName(bool bLCExtension) const
{
	QString s = LinkName(bLCExtension);
	return config.dsGallery.ToString() + s;
}

QString IABase::Path() const	// uses member 'pathID'. For Virtual albums path is empty
{
	if (pathId)
		return pathMap.AbsPath(pathId);
	else
		return QString();
}

/**************************** Image *****************************/
int Image::operator<(const Image& i)
{
	switch (searchBy)
	{
	case byID: return ID < i.ID;
	case byName: return name < i.name;
	default: return pathId < i.pathId || (pathId == i.pathId && name < i.name);
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
bool Image::operator==(const Image& i)
{
	switch (searchBy)
	{
	case byID: return ID == i.ID;
	case byBaseID: return (ID.Val()) == (i.ID.Val()) ? (name.toLower() == i.name.toLower() ? true : false) : false;
	case byName: return name == i.name;
	default: return pathId == i.pathId && name == i.name;
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
QTextStream& Image::WriteInfo(QTextStream& _ofs) const
{
	QString fullSourceName = FullSourceName();
	int len = config.dsSrc.Length();
	if (fullSourceName.mid(0, len) == config.dsSrc.ToString())
		fullSourceName = fullSourceName.mid(len);
	if (ID.DoesExist())
	{
		_ofs << LinkName() << " ("
			<< fileSize << ","
			<< rsize.width() << "x" << rsize.height() << ","
			<< osize.width() << "x" << osize.height() << ","
			// ISO 8601 extended format: either yyyy-MM-dd for dates or
			<< uploadDate.toString(Qt::ISODate)
			<< ") => " << fullSourceName << "\n";
	}
	else
		_ofs << name << "# not found\n";
	return _ofs;
}

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
void Image::GetResizedDimensions(QSize& destSize)	// 'destSize' is size of existing destination image or -1,-1
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

void Image::SetThumbSize()	// calculates the size of the thumbnail when the height equals to config.thumbHeight
{
	QSize ctsize = config.ThumbSize();
	double factor = (double)ctsize.height() / (double)osize.height();
#if 1
	tsize.setHeight(ctsize.height());
	tsize.setWidth(round(factor * osize.width()));
#else
	// get minimum size that fills the 'tsize' rectangle. It may extend outside the allowed rectangle
	tsize.scale(ctsize, Qt::KeepAspectRatio);
	// thumbnails must have the vertical size the same as in ctsize
	if (tsize.height() != ctsize.height())
	{
		tsize.setWidth(round((double)ctsize.height() / (double)tsize.height() * tsize.width()));
		tsize.setHeight(ctsize.height());
	}
#endif
}

void Image::SetNewDimensions()
{
	if (dsize.width() == 0)	   // image did not exist
		GetResizedDimensions(dsize);
	dsize = rsize;
}

double Image::Aspect()
{
	if (_aspect)
		return _aspect;
	return _aspect = (osize.height() > 0) ? (double)osize.width() / (double)osize.height() : 1.0;
}
double Image::ThumbAspect() const
{
	if (config.ImageAndThumbAspectDiffer())
		return config.ThumbAspect();
	else
		return _aspect;
}

void Image::SetResizeType()
{
	if (name.length() > 2 && name[0] == '!' && name[1] == '!')
	{
		name = name.mid(2);
		dontResize = true;
	}

}

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
Image ImageMap::invalid;
Video VideoMap::invalid;
Album AlbumMap::invalid;
uint64_t ImageMap::lastUsedPathId;		// first check if next pathless image
uint64_t VideoMap::lastUsedPathId;		// or video is here
uint64_t AlbumGenerator::lastUsedAlbumPathId;		// or album is here

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
* EXPECTS: album - album.thumbnailId is an ID of an image or another album or 0
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
	if (album.thumbnailId.Val() == 0) // then the first image  or sub-album
	{						  // will be set as its thumbnailId
		if (!album.ImageCount() && !album.VideoCount() && !album.SubAlbumCount())
			return { IMAGE_ID_FLAG, 0 };
		if (!album.ImageCount() && !album.VideoCount()) // then get thumbnailId for first sub-album
			return ThumbnailID(albums[album.IdOfItemOfType(ALBUM_ID_FLAG,0)], albums);
		else
			return album.IdOfItemOfType(IMAGE_ID_FLAG | VIDEO_ID_FLAG, 0);
	}
	return album.thumbnailId;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
template<typename Map> ID_t GetUniqueID(Map &map, int8_t typeFlag, QString &name, bool isContent = false)
{

	uint64_t id = CalcCrc(name, isContent);
	while (map.contains(ID_t(typeFlag,id)))
	{
		id += ID_INCREMENT;
	}
	return ID_t(typeFlag, id);
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
	if (lang < 0 || lang > languages.LanguageCount())
		return;
	QString stmp = /*EncodeText*/(str.trimmed());
	int len = stmp.length();
	if (lenghts.isEmpty())
		Clear(languages.LanguageCount());
	QString s;
	for (int i = 0; i < languages.LanguageCount(); ++i)
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
//	if (lang < 0 || lang > languages.LanguageCount())
//		return 0;
//	int len = str.length();
//	QString s;
//	for (int i = 0; i < languages.LanguageCount(); ++i)
//		if (i == lang)
//			s += str;
//		else
//			s += operator[](lang);
//	textsForAllLanguages = s;
//	lenghts[lang] = len;
//	return CalcID(albumgen.Texts());
//}

/**************************** ImageMap *****************************/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Image *ImageMap::Find(ID_t id, bool useBase)
{
	uint64_t mask = useBase ? BASE_ID_MASK : 0xFFFFFFFFFFFFFFFFul;
	Image im; 
	im.ID = id;
	uint64_t v = im.ID.Val() & mask;
	for (auto i = begin(); i != end(); ++i)
		if ( (i.key().Val() & mask)  == v )
			return &i.value();
	return nullptr;
}

/*============================================================================
* TASK:		searches for image by its path name.
* EXPECTS: FullSourceName - absolute or source relative path of image
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Image *ImageMap::Find(QString FullSourceName)
{
	Image img;
	QString path;
	FullSourceName = CutSourceRootFrom(FullSourceName);
	SeparateFileNamePath(FullSourceName, path, img.name);
	img.pathId = pathMap.Id(path);
	Image::searchBy = Image::byFullSourceName;
	for (auto i = begin(); i != end(); ++i)
		if (i.value() == img)
			return &i.value();

	return nullptr;
}

/*============================================================================
* TASK: Add an image to the list of all images
* EXPECTS:  path - full or config.dsSrc relative path name of the image
*			added- output parameter: was this a new image?
*			isThumbnail - true: if this image is not used in any albums then
*						it must be an orphan and appear in the orphan list
*						in the config file.
*					   false: if the image existed in imageMap and it was
*						an orphan then it will not be an orphan any more	
* GLOBALS: config
* RETURNS: Id of image added + if it was added in 'added'
* REMARKS: - Id of image is the CRC32 of the file name only
*		   - If the same image ID is used and not added then
* 			 the usage Count is increased
*		   - If two images have the same name then 'config.bKeepDuplicates`
*				determines what happens:
*				- true: if the two images are of different sizes
*				        or have different file sizes the newer or larger will be used
*				- false: each image will have a different ID
*		   - when collisions occur adds a unique value above 0xFFFFFFFF to the ID
*		   - even non-existing images are added to map
*		   - if an image with the same name, but with different path is found
*				and the one in the data base does not exist, replace the one
*				in the data base with the new image even when it also does not exist
*		   - if an image is already present in database and 
*--------------------------------------------------------------------------*/
ID_t ImageMap::Add(QString pathName, bool &added, bool forThumbNail)	// path name of source image
{
	Image img, 
		 *pImg = Find(pathName);

	added = false;
	if (pImg)
	{
		if (!forThumbNail)			// thumbnailCount will be increased in SetThumbnail
			++pImg->usageCount;
		return pImg->ID;
	}
					  // now this is a new image, not yet in the data base
	QString imgPath;
	SeparateFileNamePath(pathName, imgPath, img.name);
	img.SetResizeType();	// using img.name, handles '!!'

	if (imgPath.isEmpty())
		imgPath = pathMap.AbsPath(lastUsedPathId);

	img.pathId = pathMap.Add(imgPath);	// only adds if it is not already there

	if (img.Exists(CHECK))	// checks if the image exists on disk and sets EXISTING_FLAG in it if it does
		// return NOIMAGE_ID;
		img.ID.SetFlag(EXISTING_FLAG, true);


	img.ID.SetFlag(IMAGE_ID_FLAG, true);
	ID_t id(img.ID);

	AlbumGenerator::lastUsedAlbumPathId = lastUsedPathId = img.pathId;
	QFileInfo fi(pathName);		   // new image to add

	id.SetValue( CalcCrc(img.name, false) );	// crc from just the name. Paths may be different

	Image *found = Find(id, true); // check if a file with this same base id is already in data base?
							  // can't use reference as we may want to modify 'found' below

	if(found)		  // yes an image with the same base ID as this image was found
	{				  // the ID can still be different from the id of the found image though
		if (config.bKeepDuplicates) // then doesn't matter if the image is the same or different
		{
			do
				id.Increment(ID_INCREMENT);
			while (Find(id, false)); // full by ID
		}
		else					  // no duplicates are allowed
		{
			do					  // loop thorugh all images with the same base ID
			{
				if (found->pathId == img.pathId && found->name == img.name)	// if full path names are the same too then same image
				{								// even when they are in different directories!
					if (found->Exists())
						if (!img.Exists() || (img.Exists() && (found->uploadDate >= fi.lastModified().date() || found->fileSize >= fi.size())))
						{
							if (forThumbNail)
								++found->thumbnailCount;
							else
								++found->usageCount;	// used again somewhere else
							return found->ID;	    // do not change path as image already existed
						}

					id = found->ID;		// else found image either did not exist, or existing is older or of a different size
					break;				// replace it with this image but with the same ID as the old one
				}
				else						// same ID, different name - test the next ID for this name
					id.Increment(ID_INCREMENT);
				// repeat until a matching name with the new ID is found or no more images
			} while ((found = Find(id, false))); // full by ID
		}
	}
	// now I have a new image to add
	// new image:
	if (img.Exists())
	{
		img.fileSize = fi.size();
		img.uploadDate = fi.lastModified().date();
	}

	added = true;
	img.ID = id;

	if (!forThumbNail)		// thumbNailCount is increased in SetThumbNail
		++img.usageCount;

	insert(id, img);
	return id;
}

/*=============================================================
 * TASK   : remove on image from imageMap
 * PARAMS :	id: ID of image to remove
 * EXPECTS:
 * GLOBALS:	Image::usageCount, Image:: thumbnailCount
 * RETURNS: none
 * REMARKS: - if the usage count of the image is 0 and it is not
 *				an orphan just the usage count is decreased
 *			- if it is an orphan and thumbnailCount is not 0
 *				just the thumbnailCount is decreased
 *			- when bot usageCount and thumbnailCount is 0
 *				the image data is removed from the data base
 *			- if the image down't exis nothing happens
 *------------------------------------------------------------*/
void ImageMap::Remove(ID_t id, bool isThumbnail)
{
	if (!contains(id))
		return;

	Image* pimg = &(*this)[id];
	if (isThumbnail && pimg->thumbnailCount)
		--pimg->thumbnailCount;
	else if (pimg->usageCount)
		--pimg->usageCount;

	if (!pimg->usageCount && !pimg->thumbnailCount)
		remove(id);
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
		case byName: return (pathId < i.pathId) || (pathId == i.pathId) && (name < i.name);
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
		case byName: return (pathId == i.pathId) && (name == i.name);
	}
	return 0;
}

/*============================================================================
* TASK: get number of non-excluded images from local image list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::ImageCount(bool forced)
{
	if (forced || _imageCount <= 0)
	{
		_imageCount = 0;
		for (auto &a : items)
			if (a.Val() && a.IsImage() && !a.IsExcluded() )
				++_imageCount;
	}
	return _imageCount;
}

/*============================================================================
* TASK: get number of non-excluded videos in album from local video list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::VideoCount(bool forced)
{
	if (forced || _videoCount <= 0)
	{
		_videoCount = 0;
		for (auto &a : items)
			if (a.Val() && a.IsVideo() && !a.IsExcluded() )
				++_videoCount;
	}
	return _videoCount;
}

/*============================================================================
* TASK: get number of non-excluded sub-albums in album from local sub-album list
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int Album::SubAlbumCount(bool forced)
{
	if (items.isEmpty())
		return 0;		// but don't modify _albumCount
	if (forced || _albumCount <= 0)
	{
		_albumCount = 0;
		for (auto &a : items)
			if (a.Val() && a.IsAlbum() && !a.IsExcluded() )
				++_albumCount;
	}
	return _albumCount;
}

void Album::RecalcItemCounts()
{
	_imageCount = _videoCount = _albumCount = 0;
	for (auto& a : items)
		if (a.IsImage())
			++_imageCount;
		else if (a.IsVideo())
			++_videoCount;
		else
			++_albumCount;
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
		for (auto &a : items)
			if ((a.Val()) &&
				(
					((a.IsAlbum()) && albumgen.Albums()[a].titleID) ||
					((a.IsVideo()) && albumgen.Videos()[a].titleID) ||
					((a.IsImage()) && albumgen.Images()[a].titleID)
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
		for (auto &a : items)
			if (a.Val() &&
				(
					((a.IsAlbum()) && albumgen.Albums()[a].descID) ||
					((a.IsVideo()) && albumgen.Videos()[a].descID) ||
					((a.IsImage()) && albumgen.Images()[a].descID)
				)
			)
				++_descCount;
	}
	return _descCount;
}

ID_t Album::ThumbID()
{
	ID_t id = thumbnailId;
	if (id.Val())
		return thumbnailId;
	if (items.isEmpty())
		return { IMAGE_ID_FLAG,0 };
	// else thumbnail was not yet set. Get first image as a thumbnail
	for (auto &iid : items)
		if (iid.IsImage())
			return SetThumbnail(iid);
	// if here then no image inside album, just sub-albums
	for (auto &iid : items)
		if ((id = albumgen.AlbumForID(iid)->ThumbID()).Val())
			return SetThumbnail(id);
	return { IMAGE_ID_FLAG,0 };
}

/*=============================================================
 * TASK:	Adds the image with id as an album thumbnail.
 *			If the album had a thumbnail it is replaced by this one
 * PARAMS:	id - image ID, including type flag
 * GLOBALS:
 * RETURNS:	id
 * REMARKS: - the usage count of the new thumbnail is increased,
 *				the old thumbnail's is decreased
 *			- set the 'changed' flag in caller
 *------------------------------------------------------------*/
ID_t Album::SetThumbnail(ID_t id)
{
	if (id == thumbnailId)	// new and old are the same image
		return id;

	id.SetFlag(uint8_t(~TYPE_FLAGS), false);
	ID_t oldThumbnail = thumbnailId;
	thumbnailId = id;
	if(albumgen.Contains(id))
		++albumgen.ImageAt(id)->thumbnailCount;
	else
		albumgen.AddThumbnailBeforeImageRead(id);

	if (oldThumbnail.Val())		// must be in data base
	{
		--albumgen.ImageAt(oldThumbnail)->thumbnailCount;
		Q_ASSERT(albumgen.ImageAt(oldThumbnail)->thumbnailCount >= 0);
	}
	return id;
}

ID_t Album::SetThumbnail(uint64_t id)
{
	return SetThumbnail(ID_t(IMAGE_ID_FLAG, id));
}

QSize Album::ThumbSize() const
{
	return ThumbSizeFromId(thumbnailId);
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
ID_t Album::IdOfItemOfType(int64_t type, int index)
{
	if (index >= items.size())
		return ID_t();		// MUST NOT

	for (int startPos = 0; startPos < items.size(); ++startPos)
		if (items[startPos].IsSameType(type) )
			if(!index--)
				return items[startPos];
	return ID_t();	// invalid
}

/*============================================================================
* TASK: Create full link name of album
* EXPECTS: language -  index of language, -1: no language or extension
*			http - true: http or https prefix plus add albumdir to the
*			front true: needed, false: just name
* GLOBALS: config
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::LinkName(int language, bool http) const
{
	QString s = NameFromID(ID, language, http);
	if (http)
	{
		if (config.sServerAddress.ToString().toLower().left(4) == "http")
			s = config.sServerAddress + "/" + s;
		else
			s = (config.bForceSSL ? "https://" : "http://") + config.sServerAddress + "/" + s;
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
	return QString("%1%2").arg(config.sBaseName.ToString()).arg(ID.Val());
}

/*=============================================================
 * TASK   : add or insert a new item into _items
 * PARAMS :	id: id of item, pos position to insert: -1: at end
 * EXPECTS:
 * GLOBALS:
 * RETURNS: none
 * REMARKS:
 *------------------------------------------------------------*/
void Album::AddItem(ID_t id, int pos)
{
	if (pos < 0)
		items.push_back(id);
	else
		items.insert(pos, id);
	if (id.IsAlbum())
		_albumCount < 0 ? _albumCount = 1 : ++_albumCount;
	else if (id.IsImage())
		_imageCount < 0 ? _imageCount = 1 : ++_imageCount;
	else if (id.IsVideo())
		_videoCount < 0? _videoCount = 1 : ++_videoCount;
}

/*============================================================================
* TASK: Create album name for given ID
* EXPECTS: id - album id
*			  - language -  index of language or -1: no language added
* GLOBALS: config
* REMARKS:
*--------------------------------------------------------------------------*/
QString Album::NameFromID(ID_t id, int language, bool withAlbumPath)
{
	QString s;

	if (id == TOPMOST_ALBUM_ID)	// root album
	{
		s = config.sMainPage.ToString();
		s = languages.FileNameForLanguage(s, language);
	}
	else
	{
		if (id == RECENT_ALBUM_ID)	// recent uploads
		{
			s = "latest";
			if(language >= 0)
				s += (*languages["abbrev"])[language] + ".html";
		}
		else
			s = languages.FileNameForLanguage(QString("%1%2.html").arg(config.sBaseName.ToString()).arg(id.Val()), language);

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
* TASK:	  Add a new album to the album map if it is not already in there
* EXPECTS: name: IN - either just a name, an absolute path name or a path relative to 
*						 the source album directory (!)
*						e.g. "newAlbum"	- logical path name, no real path exists 
*						     "this/is/a/new/album"	- path relative to source directory 
*							 "c:/source/dir/this/is/a/new/album" 
*									absolute path file in source dir
*							 "c:/not/source/dir/this/is/a/new/album" absolute path 
*									file not in source dir
*		  added: OUT - set when the new album is added
* RETURNS: unique id of album with ALBUM_ID_FLAG
* GLOBALS:
* REMARKS: - path may contain a logical name only, it need not exist
*			if it exists then lastUsedAlbumPath is adjusted
*		   - when a new album is created inside the program no real folder
*			is created on disk. If later on a folder with the same name is
*			created on disk and images are added from that folder, the virtual
*			album's empty path is not modified with the path of the real album 
*--------------------------------------------------------------------------*/
ID_t AlbumMap::Add(ID_t parentId, const QString &name, bool &added)
{
	Album ab;

	QString relativeParentPath = CutSourceRootFrom(name); // no cut if it isn't there
	QString path;	// just the path that contains the album
	SeparateFileNamePath(relativeParentPath, path, ab.name);
	added = false;
	if (path.isEmpty())
	{
		QString vpath = MakeRandomStringOfLength(10) + "/" + ab.name;
		ab.ID = GetUniqueID(*this, ALBUM_ID_FLAG, vpath, false);   // using full path name only and not file content
		ab.pathId = 0;
	}
	else
	{
		Album* found = Find(relativeParentPath);		// already in database?
		if (found)
			return found->ID;	// same base ID, same name then same album
		ab.ID = GetUniqueID(*this, ALBUM_ID_FLAG, relativeParentPath, false);   // using full path name only and not file content
		ab.pathId = pathMap.Add(path);
	}

	ab.parentId = parentId;

	if (QDir::isAbsolutePath(relativeParentPath))
		ab.ID.SetFlag(EXISTING_FLAG, QFile::exists(relativeParentPath));
	else
		ab.ID.SetFlag(EXISTING_FLAG, QFile::exists((config.dsSrc + relativeParentPath).ToString()));

	if (ab.Exists())
		AlbumGenerator::lastUsedAlbumPathId = pathMap.Add(path + ab.name + "/");
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

/*=============================================================
 * TASK:	remove album and all of its sub-albums, but
 *			leave the images and videos intact, as they might
 * 			be used in other albums
 * PARAMS:	id - id of album to remove. MUST have the ALBUM_ID_FLAG bit set
 * GLOBALS:
 * RETURNS:	true: success, false: album or any sub albums not found
 * REMARKS:
 *------------------------------------------------------------*/
bool AlbumMap::RemoveRecursively(ID_t id)
{
	bool res = true;
	if (id.Val())
	{
		Album *album = Find(id);
		if ((album->ID.IsAlbum()) == 0)
			return false;
		for (int i = 0; res && i < album->items.size(); ++i)
			if (album->items[i].IsAlbum())
				res = RemoveRecursively(album->items[i]);
		remove(id);
		return res;
	}
	return false;
}

/*============================================================================
* TASK:		find album with a given path name
* EXPECTS:
* GLOBALS:
* RETURNS: pointer to found record or nullotr
* REMARKS:
*--------------------------------------------------------------------------*/
Album *AlbumMap::Find(QString albumPath)
{
	QString n,p;
	albumPath = CutSourceRootFrom(albumPath);
	SeparateFileNamePath(albumPath, p, n);

	for (auto i = begin(); i != end(); ++i)
		if (i.value().name==n)
			return &i.value();
	return nullptr;
}

Album* AlbumMap::Find(uint64_t pathID, QString albumName)
{
	QString fullPath = pathMap.AbsPath(pathID) + albumName;
	return Find(fullPath);
}

Album* AlbumMap::Find(uint64_t pathID, ID_t albumId)
{
	QString fullPath = pathMap.AbsPath(pathID) + AlbumForID(albumId)->name;
	return Find(fullPath);
}

/*=============================================================
 * TASK   :	test if folder path is an existing folder
 * PARAMS :
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool AlbumMap::Exists(QString albumPath)
{
	return (Find(albumPath) != nullptr);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
Album *AlbumMap::Find(ID_t id)
{
	Album ab;
	ab.searchBy = Album::byID;
	ab.ID = id;

	return contains(id) ? &(*this)[id] : nullptr;
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
  * TASK:	 add a new image/video from structure to image/video map
  * EXPECTS:  imagePath: path name is either relative to source directory
  *						 or absolute path
  *
  * RETURNS:  id of image/video added/found with type flag set
  *				It will not have any title or description as those
  *				will be added after this image is processed
  * GLOBALS:
  * REMARKS: - not called for already processed images/videos
  *			 - new images may be added to the structure file by using the path
  *			  name, (absolute or relative to the gallery source) and nothing more.
  *			  In this case the new image must be processed and added to the
  *			  data base.
  *			 - sets '_structFileChangeCount' to true
 *--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddImageOrVideoFromPathInStruct(QString imagePath, FileTypeImageVideo ftyp, bool& added)
{
	++_structFileChangeCount;	 // always changed when a non-processed image line is in the structure

	ID_t id;

	if (ftyp == ftImage)
		id = _imageMap.Add(imagePath, added);	// add new image to global image list or get id of existing
	else
		id = _videoMap.Add(imagePath, added);
	return id.ClearNonTypeFlags();
}

/*==========================================================================
* TASK:		add a sub-album or an image to global arrays and album 'parentID'
*			from directory unless they are excluded
* EXPECTS:	albumId - ID of album to add images and sub albums to
*			fi - info of a file and album
* GLOBALS:	
* RETURNS:	id of data , for albums with ALBUM_ID_FLAG,
*				with videos with VIDEO_ID_FLAG set
* REMARKS:  - must use id because album address may change in _albumMap
*			- 'exists' flag on ID for not excluded old or new images and sub-albums
*				are set to true when 'fromDisk' is true
*			- adds new images/videos and id's to the corresponding id list of 'ab'
*			- shows progress using frmMain's progress bar
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_AddItemToAlbum(ID_t parentID, QFileInfo & fi, bool signalElapsedTime, bool doNotAddToAlbumItemList)
{
	Album* ab = AlbumForID(parentID);
	Q_ASSERT(ab);
	ID_t id = ID_t::Invalid();

	QString s = fi.filePath();
	if (fi.fileName()[0] == '.')			// hidden file?
		return id;

	bool added = false;
	FileTypeImageVideo type;
	if (fi.isDir())
	{
		QString ds = fi.fileName();
		if (ds == "res" || ds == "cache" || ds == "thumbs")
			return { ALBUM_ID_FLAG,0 };
		id = _albumMap.Add(parentID, s, added);		// add new album to global album list
		ab = AlbumForID(parentID);	// might have change item in memory
		Q_ASSERT(ab);
	}
	else if ((type=IsImageOrVideoFile(s))!= ftUnknown)
	{
		if (type == ftImage)
			id = _imageMap.Add(s, added);	// add new image to global image list or get id of existing
		else
			id = _videoMap.Add(s, added);	// add new video to global video list or get id of existing

	}

	doNotAddToAlbumItemList |= !(id.Val()) || (id.IsExcluded()) || (ab->items.indexOf(id) >= 0);
	if (!doNotAddToAlbumItemList)
		ab->items.push_back(id);		// add to ordered item list for this album except if you must not or excluded
	// progress bar
	if (signalElapsedTime)
	{
		emit SignalProgressPos(_albumMap.size(), _ItemSize());	  // both changes

		_remDsp.Update(_albumMap.size());
		emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _ItemSize(), false);
	}
	return id;
}

bool AlbumGenerator::_IsAlbumAndItsSubAlbumsEmpty(Album& a)
{
	if (a.ID.IsExcluded())
		return true;

	if (a.ImageCount() == 0 && a.VideoCount() == 0)	// just sub-albums
	{
		for (auto b : a.items)
			if (!_IsAlbumAndItsSubAlbumsEmpty(_albumMap[b]))
				return false;

		a.SubAlbumCount(true);	// recalculate and set to 0
		return true;
	}
	else
		return false;
}

/*=============================================================
 * TASK:	set excluded flag for all empty albums except the
 * 			root and latest pre-defined albums and all albums
 *			which only has empty albums inside it
 * PARAMS:
 * GLOBALS: _albumMap
 * RETURNS:
 * REMARKS: TODO:
 *			what about excluded folder if later items added to
 *			it?
 *------------------------------------------------------------*/
void AlbumGenerator::_CleanupAlbums()
{
		// pass #1: mark empty albums with no items
	auto it = _albumMap.begin();	// root ID (smallest)
	++it;							// last used (dummy)
	for (++it; it != _albumMap.end() ; ++it)	// first album
		if (it.value().items.isEmpty())
			it.value().ID.SetFlag(EXCLUDED_FLAG);
		// pass #2: mark non empty albums which only contain empty albums recursively
	it = _albumMap.begin();
	++it;							// last used (dummy)
	for (++it; it != _albumMap.end(); ++it)	// first album
		if (!(it.value().ID.IsExcluded()) && _IsAlbumAndItsSubAlbumsEmpty(it.value()))
			it.value().ID.SetFlag(EXCLUDED_FLAG);
}

/*============================================================================
* TASK:		checks if given
* EXPECTS:	ab - album whose items list we check
*			name - name of info file, e.g. alma.jpg.info
* GLOBALS:	maps
* REMARKS:
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_IsExcluded(const Album &ab, QString name)
{
	if (name.size() == 5)	// "file: .info
		return true;

	name = name.left(name.lastIndexOf('.'));	// cut ".info" from file name
	for (auto &i : ab.items)
	{
		if ((i.IsAlbum()) && _albumMap[i].name == name)
			return _albumMap[i].ID.IsExcluded();
		else if ((i.IsImage()) && _imageMap[i].name == name)
			return _imageMap[i].ID.IsExcluded();
		else if ((i.IsVideo()) && _videoMap[i].name == name)
			return _videoMap[i].ID.IsExcluded();
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
	path = CutSourceRootFrom(path);
	SeparateFileNamePath(path, p, n);
	ltl.Fill(n);
}


QString AlbumGenerator::_EncodeTitle(QString title)
{
	QRegularExpression reg("[\n'\"]");
	if(title.contains(reg))
	{
		title.replace('\n', ' ');
		title.replace("'", "\\'");
		title.replace("\"", "\\\"");
	}
	return title;
}

/*============================================================================
  * TASK:	test the sizes of an existing thumbnail against config and see
  *			if it must be recreated
  * EXPECTS:thumbPath - full path of thumbnail file
  *			img - image data from data base with correct thumbnail size set in it
  * RETURNS:'true' : actual size of thumbnail is not the same as the one in data base
  *			'false' : otherwise
  * GLOBALS:
  * REMARKS: - the thumbnail is already a rotated image when needed
 *--------------------------------------------------------------------------*/
bool AlbumGenerator::_MustRecreateThumbBasedOnImageDimensions(QString thumbPath, Image & img)
{
	ImageReader reader(thumbPath);
	QSize 	// tsize = config.ThumbSize(),
			thumbSize = reader.size();	// read thumbnail image dimensions from file
							// because integer arithmetic the calculated w & H may be different
							// from that of the converted thumbnail file
	return abs(thumbSize.width() - img.tsize.width()) > 3 || abs(thumbSize.height() - img.tsize.height()) > 3;
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
	int cntLang = languages.LanguageCount();
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

/*=================================================================
 * TASK: read a directory tree of images and order them into an album
 *		  hierarchy recursively
 *			Reads whole gallery hierarchy starting at 'albumId'
 * EXPECTS: albumId (must already be on the global album list '_albumMap')
 * GLOBALS:
 * REMARKS: - first reads meta.properties and set parameters for this album
 *			- then reads file 'albumfiles.txt' if it exists and adds
 *				image/video/album names from it to album
 *			- then reads all file and sub-directory names from the album directory
 *			  and add not stored ones to lists recursively processing sub albums
 *			- then reads comments.properties and sets them for images and sub-albums
 *			- then call itself recursively for sub-folders
 *----------------------------------------------------------------*/
void AlbumGenerator::_RecursivelyReadSubAlbums(ID_t albumId)
{
	// get number of images and albums already read in
	// TODO justChanges
	// Append images and albums from disk

	// at path 'ab.FullSourceName()' and add
	// those not yet listed in 'albumfile.txt' to the list of files and albums
	// in the order they appear

	Album &ab = _albumMap[albumId];

	QDir dir(ab.FullSourceName());
	dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::NoSort);
	QFileInfoList list = dir.entryInfoList();	// get all files and sub directories
	if (list.size())
		SetAlbumModified(ab);

	ID_t id;

	static int debugCnt = 0;
	for (auto &fi : list)		// elements in directory
	{
		++debugCnt;
		if( (id = _AddItemToAlbum(albumId, fi,_signalProgress)).Val() == 0)	// nothing added? (hidden file)
			continue;
		if (id.IsAlbum())		// sub album
		{
			Album& subA = _albumMap[id];
			subA.parentId = albumId;	// signals this sub-album is in 'ab'
											// Must change when sub-albums moved to other album
			if (subA.ID.DoesExist())
				_RecursivelyReadSubAlbums(id);
		}
		if(_signalProgress)
			emit SignalProgressPos(_albumMap.size(), _imageMap.size());
		if (!_processing)
			break;
	}
	if (_processing)
	{
		(void)ab.ImageCount();		// removes excluded ID s of 'ab.images'
		(void)ab.SubAlbumCount();	// removes excluded ID s of 'ab.albums'
		_AddAlbumThumbnail(ab, 0);	// only if there's no thumbnail already
	}
}

void AlbumGenerator::Init()
{
	Album album;
	// add root album with ID 1
	album.ID = TOPMOST_ALBUM_ID;
	// album.ID.SetFlag(EXISTING_FLAG,false);	// do not check: virtual!
	_albumMap[album.ID] = album;				// and not with _albumMap.Add()

	// add "latest" album if required

	album.ID = RECENT_ALBUM_ID;
	_albumMap[RECENT_ALBUM_ID] = album;

	// add default image when no image is found This is the only one with id == 0
	// the file 'NoImage.jpg' must be put into the 'res' directory
	Image im;
//	im.ID.SetFlag(EXISTING_FLAG);
	im.name = "NoImage.jpg";
	im.ID = NOIMAGE_ID;
	im.dsize = im.osize = QSize(800, 800);
	_imageMap[im.ID] = im;

}

void AlbumGenerator::Clear()
{
	_imageMap.clear();
	_textMap.clear();
	_videoMap.clear();
	_albumMap.clear();
	_addedThumbnailIDsForImagesNotYetRead.clear();
	Init();
}

/*=============================================================
 * TASK:	clears all changed flags
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool  AlbumGenerator::SetChangesWritten()
{
	bool res = false;
	for (auto& a : _albumMap)
	{
		if (a.changed)
			res = true;
		a.changed = false;
	}
	for (auto &a : _imageMap)
	{
		if (a.changed)
			res = true;
		a.changed = false;
	}
	for (auto &a : _videoMap)
	{
		if (a.changed)
			res = true;
		a.changed = false;
	}
	_slAlbumsModified.clear();
	_structFileChangeCount = res ? 1 : 0 ;
	return res;
}

void AlbumGenerator::AddDirsRecursively(ID_t albumId)
{
	_signalProgress = false;
	_processing = true;
	 RecursivelyAddAlbums(albumId);
	_processing = false;
	_signalProgress = true;
	return;
}

void AlbumGenerator::RecursivelyAddAlbums(ID_t albumId)
{
	_RecursivelyReadSubAlbums(albumId);
}

/*==========================================================================
* TASK:		reads or creates whole album hierarchy
*				if a corresponding '.struct' file exists and not yet read into
*					memory then tries to read it. If the read is unsuccessful
*					then all data is cleared from memory
* PARAMS:	'bMustReRead' must read original '.struct' file even when one
*					is already in memory
* EXPECTS: 'root' path name of uppermost source album directory
* RETURNS: success
* REMARKS: - prepares root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::Read(bool bMustReRead)
{
	_processing = true;

	emit SignalAlbumStructWillChange();

	bool result = false;

	if (bMustReRead) // wrong: !_justChanges)
		Clear();
	PROGRAM_CONFIG::MakeValidLastConfig();		// may add new config
	QString s = PROGRAM_CONFIG::NameForConfig(false, ".struct");
	_structFileChangeCount = 0;

	bool newDataExists = true;

	if (QFileInfo::exists(s))
	{
		if ((bMustReRead || AlbumCount() <= 1))	// only root album?
		{
			try
			{
				result = _ReadStruct(s);
			}
			catch (.../*BadStruct b*/)
			{
				Clear();		  // reading aborted
			}
		}
		else
			result = true;
	}
	else
	{
		if(bMustReRead)
			ShowWarning(tr("Album %1 does not exist").arg(s), frmMain);
		newDataExists = false;
	}

	if (newDataExists)
	{
		_CleanupAlbums();	// recursively exclude empty ones
		 // mark usage for those thumbnail that were not in the database when the album was read
		IdList tmplst;
		for (auto &id : _addedThumbnailIDsForImagesNotYetRead)
		{
			if (Contains(id))
				++_imageMap[id].thumbnailCount;
			else
				tmplst.push_back(id);
		}
		_addedThumbnailIDsForImagesNotYetRead = tmplst;
	}
	emit SignalAlbumStructChanged(true);	// show list of albums in GUI
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
bool AlbumGenerator::_CreateBaseDirectories()
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
*			if at the first run of the program no .lang files were present then
*			the created '.struct' file may not contain valid texts
*			this can only be corrected manually
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
	languages.Clear(langcnt);		// allocates this many empty strings in each string list
	reader.ReadLine();				// first language index :0
	for (int lang = 0; lang < langcnt; ++lang)
	{				// while loop finished when next language index is read in
		while((sl = reader.ReadLine().split('=')).size() == 2)
			if(!sl[1].isEmpty())
				languages.SetTextFor(sl[0], sl[1], lang);
	}
	if ((*languages["countryCode"])[0].isEmpty())
	{
		for (int i = 0; i < langcnt; ++i)
			(*languages["countryCode"])[i] = "en_US";
	}
	if ((*languages["falconG"])[0].isEmpty())
	{
		for (int i = 0; i < langcnt; ++i)
			(*languages["falconG"])[i] = QString("Site generated by <strong>falconG</strong> - © A. Sólyom 2018-%1").arg(PROGRAM_CONFIG::copyrightYear);
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
	const QString& sTitleTag = (config.majorStructVersion == 1 && config.minorStructVersion < 4) ? OLD_TITLE_TAG : TITLE_TAG,
//				 & sDescTag  = (config.majorStructVersion == 1 && config.minorStructVersion < 4) ? OLD_DESCRIPTION_TAG : DESCRIPTION_TAG,
				 & sThumbTag  = (config.majorStructVersion == 1 && config.minorStructVersion < 4) ? OLD_THUMBNAIL_TAG : THUMBNAIL_TAG;

	LanguageTexts texts(languages.LanguageCount());

	// order of tags are arbitrary
	// any and all of these may be missing
	// ALL TAG TEXT MUST BE OF THE SAME LENGTH!
	// and all tags must start with a different letter

	uint64_t textID = 0;		// ID of last processed text
	IdsFromStruct::IDS newTag = IdsFromStruct::nothing;

	ids.what = IdsFromStruct::nothing;

	do		 // for each line with text in []);
	{
		int lang = -1;		// THUMBNAIL_TAG
		if (reader.l()[level + 1] != sThumbTag[0])
		{

			for (lang = 0; lang < languages.LanguageCount(); ++lang)
			{
				int len = (*languages["language"])[lang].length();
				if (reader.l().mid(level + sTitleTag.length() + 1, len) == (*languages["language"])[lang])
					break;
			}
			if (lang == languages.LanguageCount())		// safety
				lang = 0;
		}

		if (reader.l()[level + 1] == sThumbTag[0])
			newTag = IdsFromStruct::thumbnail;
		else if (reader.l()[level + 1] == sTitleTag[0])
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
				++_structFileChangeCount;
			}

			if (!texts.usageCount)					// then not yet added
				texts.usageCount = 1;

			_textMap[textID] = texts;				// add new record or modify old record with the new usage count

			if (ids.what == IdsFromStruct::title)
				ids.titleIDVal = textID;
			else
				ids.descIDVal = textID;

			texts.Clear();
			textID = 0;								// processed
		}

		if(newTag == IdsFromStruct::thumbnail)
		{
			int len = (reader.l().length() - 1) - level - sThumbTag.length() - 2; // cut ending ']'
			if (!len)		   // empty thumbnail text?
				continue;
			// see if this is a processed line or an image path string
			// image path strings starting with a character other than a decimal digit may follow after a space or a colon
			// but image names in the source directory that start with a decimal digit must be written after a colon!
			bool b = reader.l()[level + sThumbTag.length() + 1] == ':';
			QString s = reader.l().mid(level + sThumbTag.length() + 2, len);  // ID or path name
			if (b && s[0].isDigit())
				ids.thumbnailIDVal = s.toULongLong();
			else
			{
				bool added;
				ids.thumbnailIDVal = _AddImageOrVideoFromPathInStruct(s, ftImage, added).Val(); // sets _structFileChangeCount inside the function
			}
		}
		else		// common part for title and description
		{			// because textID is always calculated using all texts for all languages
					// we can only calculate it after all lines are read in
			QString s = reader.l();
			int len = s.length();
			while (len && s[--len] != ']')	// find last ']'
				;
			++len;		// include closing ']' but not the possible ID/collision
			int clen = sTitleTag.length() + (*languages["language"])[lang].length() + 2;	// 1 for '[' + 1 for ':'
			texts.SetTextForLanguageNoID(s.mid(level + clen, len - level - clen-1), lang);
			if (config.majorStructVersion == 1)
			{						// none = there is no asterix followed by a number
				if (textID == 0)	// then this is the first line for the text
				{
					if (s.length() == len)
						++_structFileChangeCount;		// because textID will be calculated
					else
					{
						textID = texts.ID = s.mid(len + 1).toULongLong(); // len+1: length including '*'
						texts.collision = textID == 0 ? 0 : (textID >> TEXT_ID_COLLISION_FACTOR);
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
			++_structFileChangeCount;
		}

		if (!texts.usageCount)					// then not yet added
			texts.usageCount = 1;

		_textMap[textID] = texts;				// add new record or modify old record with the new usage count

		if (ids.what == IdsFromStruct::title)
			ids.titleIDVal = textID;
		else
			ids.descIDVal = textID;
	}
}

bool AlbumGenerator::_ReadPathTable(FileReader& reader)
{
	QString rline = reader.l();

	if( (config.majorStructVersion == 1 && config.minorStructVersion < 3) || rline != PATH_TABLE) // older versions have no path table
		return true;

	auto __isID = [](QString s) ->bool
		{
			if (s.isEmpty())
				return false;
			for (auto& c : s)
				if (!c.isDigit())
					return false;
			return true;
		};
	bool ok = reader.Ok();
	while (ok && (rline = reader.ReadLine()) != ']')
	{
		// rline format: <full, or source relative path name of thumbnail>[:<id>]

		QStringList sl = rline.split('|');
		sl[1] = CutSourceRootFrom(sl[1]);
		int n = sl.size();

		if (n != 2 || !__isID(sl[0]))
			throw BadStruct(reader.ReadCount(), tr("Bad Path line"));

		pathMap.Insert(sl[0], sl[1]);
		ok = reader.Ok();
	}
	rline = reader.ReadLine();	// first album line or orphans
	if (!reader.Ok())
		throw BadStruct(reader.ReadCount(), FalconG::tr("Missing ']'"));
	return true;
}

void AlbumGenerator::_CleanupPathTable()
{
	QMap<uint64_t,uint64_t> pathIdVector;
	for (auto& i : _imageMap)
		pathIdVector[i.pathId] = i.pathId;
	for (auto& i : _videoMap)
		pathIdVector[i.pathId] = i.pathId;
	for (auto& i : _albumMap)
		pathIdVector[i.pathId] = i.pathId;

	QVector<uint64_t> idsToDelete;
	uint64_t pathID = pathMap.FirstId();
	do
	{
		if (!pathIdVector.contains(pathID))
			idsToDelete.push_back(pathID);
	} while (pathID = pathMap.NextId());
	for (auto& id : idsToDelete)
		pathMap.Remove(id);
}

bool AlbumGenerator::_ReadOrphanTable(FileReader& reader)
{
	QString rline = reader.l();
	// read orphan thumbnails: images which are just thumbnails, but not in any album
	// they may be anywhere not just in the sorce album folder or one of its subfolder
	if (rline != ORPHAN_ID)
		return true;

	bool ok = reader.Ok();
	reader.ReadLine();
	ok = reader.Ok();
	while (ok && (reader.l() != ']'))
	{
		// rline format:  same as for any other image
		ID_t id = _ReadImageOrVideoFromStruct(reader, 0, nullptr, true);
		ok = reader.Ok();
	}
	rline = reader.ReadLine();	// path table or first album line
	if (!reader.Ok())
		throw BadStruct(reader.ReadCount(), FalconG::tr("Missing ']'"));
	return true;
}

/*==========================================================================
* TASK:		helper - splits an image or video definition text line into
*				max 10 fields (see below) and returns a string list	of those
* EXPECTS: s : definition string w.o. the leading spaces (which determine the level)
*				Formats (notation: text not between < > is verbatim,  '-' starts remark):
*				  for yet unprocessed images/videos:
*					<full file path> OR
*					<file name&ext only> - this uses last used path either
*												from directory or previous file
*				  processed files:
*					Format before Version 1.3
*						Images:
*							<file name&ext only>(<id>,<width><height>,<orig width>x<orig height>,<upload date>,<file length)<directory path>
*						Videos:
*							<file name&ext only>(<id>,<upload date>,<file length)<directory path>
*						if 'V' is present it is a video file else it is an image file
*					Format from V1.3
*						Images:
*							<file name&ext only>||<id>|<width>x<height>|<orig width>x<orig height>|<upload date>*<file length|<numeric directory path ID as string>
*						Videos:
*							<file name&ext only>||<id>|<upload date>|<file length>|<directory path ID as string>
*																		Image Video
* RETURNS: list of 0, 2, 5/6 or 9/10 strings:
*		empty list:	  unknown file type
*		for lists of 2 elements, when the unprocessed line has the full file path or just
*		the file name, but no id and so on:
*				file name or path										(#0)	(#0)
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
*				folder path/path Id for files							(#9)	(#5)
* REMARKS:	- "normal" regular function split() could not be used as names
*					may contain braces and commas
*			- file name may contain any number of brackets, but it must not have
*				a file type/extension before a '('. (E.g. 'foo.jpg(bar.jpg' is invalid.)
*			- result may contain fewer or more fields than required
*--------------------------------------------------------------------------*/
static QStringList __imageMapStructLineToList(QString s)
{
	QStringList sl;
	bool oldType = false;

	int pos0 = 0,
		pos = s.indexOf("||"); 	// the opening for file parameters after the file name
								// because no file name may contain the '|' character (pair)
	if (pos < 0)				// old format
	{
		pos = s.lastIndexOf('(');	// start of parameters ('(' can appear in file name)
		if (pos >= 0)
		{							  
			oldType = true;
			pos0 = pos + 1;		// skip the '('
		}
	}
	else
		pos0 = pos + 2;			// skip the '||'

	FileTypeImageVideo typ;

	auto typeStr = [&]() { return typ == ftImage ? "I" : "V"; };

	typ = IsImageOrVideoFile(s.mid(0, pos < 0 ? -1 : pos));	// the part before the last '(' (old) or ('*') new name, based on file extension
	if (typ == ftUnknown)			// then maybe '('/'*' inside file name
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
	if (oldType)
	{
		pos = s.indexOf(')', pos0);
		if (pos < 0)					// invalid parameters
			return sl;					// same as for unprocessed lines
	}
	else
		pos = s.length(); // -1;

	QRegExp rexp("[,|x]");
	sl += s.mid(pos0, pos - pos0).split(rexp);	// index #2..#9 for image: ID, width, height, owidth, oheight, length, date
												// index #2..#5 for video: ID, length, date

//	if (typ == ftVideo && sl[2][0] == QChar('V'))	// for video files iD starts with 'V'
//		sl[2] = sl[2].mid(1); no need

	if ( (sl.size() != 9 && sl.size() != 5) || pos == s.length() )   // some problem, maybe path is missing (was s[pos + 1] != '/')
		return sl;

	sl.push_back(s.mid(pos + 1));		// index #6 or #10 - original path	or path ID (Version dependent)
	return sl;
}

/*==========================================================================
* TASK:		add a sub-album or an image to global arrays and to album 'ab'
* EXPECTS:	ab - album to add data to (parent)
*			path - full path name relative to  album root  or absolute path
*			pos: - put position in images or in albums here
*			folderIcon: if the file should be used as album thumbnail for this album
* RETURNS:	ID of new or already present album or image
* REMARKS:	does not set/modify the title and description IDs
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::AddItemToAlbum(ID_t albumId, QString path, bool isThumbnail, bool doSignalElapsedTime, bool doNotAddToAlbumItemList)
{
	if (!QDir::isAbsolutePath(path))
		path = (config.dsSrc + path).ToString();
	QFileInfo fi(path);
	ID_t id = _AddItemToAlbum(albumId, fi, doSignalElapsedTime, doNotAddToAlbumItemList);
	if (id.Val() && isThumbnail)	// an album can be a folder icon which means that its
		_albumMap[albumId].SetThumbnail(id);		// folder icon is used here. This will be resolved later on

	return id;
}

/*=============================================================
 * TASK:	add new image or video to 'album',
 * PARAMS:	'fullFilePath':
 *			'album' - image or video is in this album
 *			'pos' -	position of new image or video in album
 *					-1: append, other: insert before this position
 * GLOBALS: config
 * RETURNS: sucess or failure of the operation
 * REMARKS: C.f. :_ReadImageOrVideoFromStruct()
 *			marks album as changed, so it's get regenerated
 *------------------------------------------------------------*/
bool AlbumGenerator::AddImageOrVideoFromString(QString fullFilePath, Album& album, int pos)
{
	Image img;
	Video vid;

	FileTypeImageVideo type = IsImageOrVideoFile(fullFilePath);
	if (type == ftUnknown)
	{
		ShowWarning(QString(tr("Unknown file type\nFile name:\n%1")).arg(fullFilePath), frmMain);
		return false;
	}

	bool added;
	ID_t id = _AddImageOrVideoFromPathInStruct(fullFilePath, type, added);	 // either add to maps or get id for image already added, has type flag set
	SetAlbumModified(album.ID);			// set it as changed always	 '_structFileChangeCount' is incremented in caller
	if (type == ftImage)
	{
		if (_imageMap.contains(id))
			img = _imageMap[id];
		else
			_imageMap.insert(id, img);

		if (img.pathId != NO_ID)
			ImageMap::lastUsedPathId = img.pathId;
	}
	else
	{
		if (_videoMap.contains(id))
			vid = _videoMap[id];
		else
			_videoMap.insert(id, vid);

		if (vid.pathId != NO_ID)
			VideoMap::lastUsedPathId = vid.pathId;
	}
	// add it to album even when it was already added
	album.AddItem(id, pos);	// and modify item count

	return true;
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
ID_t AlbumGenerator::_ReadImageOrVideoFromStruct(FileReader &reader, int level, Album *album, bool thumbnail)
{
	IABase* pItem = nullptr;
	Image img;
	Video vid;

	QStringList sl = __imageMapStructLineToList(reader.l().mid(level));
	int n = sl.size();		// when all fields are present this should be 6 or 10 :
							// n = 0 the invalid file type
	if(!n)
	{
		ShowWarning(tr("Invalid file type for image or video in line %1").arg(reader.ReadCount()), frmMain);
		return { 0,INVALID_ID_FLAG };
	}
	// if no path was given then 5 or 9
							// if only the path name was given then just the first two
		//
		// index in sl:  0     1    2    3      4               5               6         7              8             9
		// images:		name, "I", id, width, height,    original width, original height, date, original file size, path ID/path
		// videos:		name, "V", id, date,  file size, path/path ID

	ID_t id;
		// n:  images: 2 or 10, videos 2 or 5
	FileTypeImageVideo type = n >= 2 ? (sl[1][0] == QChar('I') ? ftImage : (sl[1][0] == QChar('V') ? ftVideo : ftUnknown)) : ftUnknown;
	if (n < 9 && n != 5 && n != 6)		   // maybe new image/video added and not yet processed
	{
		if (n != 2)	// it should be at least an ID for <config.dsSrc relative full path name> and <file type> (image or video or unknown)
			throw BadStruct(reader.ReadCount(),
							QString(FalconG::tr("Wrong image parameter count:")) + QString().setNum(n)); // or just the image name (from the same folder as the previous one)

		// expects: original/image/directory/inside/source/name.ext and 'type' 
		bool added;
		id = _AddImageOrVideoFromPathInStruct(sl[0],type, added);	 // then structure is changed
		if (id.Val())		// has type flag set
		{
			if (thumbnail && album)		 // It's parent also changes
			{
				album->SetThumbnail(id);		// and increments 'thumbNailCount'
				if (album->parentId.Val() )
					SetAlbumModified(album->parentId);
			}
			SetAlbumModified(album->ID);			// set it as changed, always
		}
		if (type == ftImage)
		{
			pItem = &_imageMap[id];
			if (added)
				ImageMap::lastUsedPathId = img.pathId;
		}
		else
		{
			pItem = &_videoMap[id];
			if (added)
				VideoMap::lastUsedPathId = vid.pathId;
		}
	}
	else  // processed video or image
	{
		id = ID_t::Invalid( sl[2].toULongLong() );	// no type yet so at the moment an invalid type in ID is in sl[2]
		if (type == ftImage)	// n == 9 or 10
		{
			id.SetFlag(IMAGE_ID_FLAG);		// now we know the type, not invalid any more
			if (!_imageMap.contains(id))
			{
				img.name = sl[0];
				img.SetResizeType();	// removes possible starting '!!' and sets dontResize true in that case

				img.ID = id;
				img.dsize = QSize(sl[3].toInt(), sl[4].toInt());	// scaled size from .struct file
				img.osize = QSize(sl[5].toInt(), sl[6].toInt());	// original size - " -

		// calculated when asked for		img.aspect = img.height ? (double)img.width / (double)img.height : 1;
				img.uploadDate = DateFromString(sl[7]);
				if (img.uploadDate > _latestDateLimit)
					_latestDateLimit = img.uploadDate;
				// DEBUG:
				if (img.uploadDate.toJulianDay() < 0)
					return ID_t::Invalid(id.Val());

				img.fileSize = sl[8].toULongLong();
				if (n == 10)
				{
					if (config.majorStructVersion == 1 && config.minorStructVersion < 3)
						ImageMap::lastUsedPathId = pathMap.Add(sl[9]);
					else
						ImageMap::lastUsedPathId = sl[9].toULongLong();
				}

				QString path = pathMap.AbsPath(ImageMap::lastUsedPathId);
				img.pathId = ImageMap::lastUsedPathId;
				if (!img.fileSize)	// maybe file does not exist
				{
					QString sn = path + img.name;
					if (!QDir::isAbsolutePath(sn))
						sn = config.dsSrc.ToString() + sn;
					QFile f(sn);
					if (f.exists())
					{
						img.fileSize = f.size();
						++_structFileChangeCount;
					}
				}

				img.ID.SetFlag(EXISTING_FLAG, (img.fileSize != 0));	// non existing images have 0 size

				_imageMap[id] = img;
			}
			if(thumbnail)
				++_imageMap[id].thumbnailCount;
			else
				++_imageMap[id].usageCount;
			pItem = &_imageMap[id];

		}
		else		// video
		{
			id.SetFlag(VIDEO_ID_FLAG );
			if (!_videoMap.contains(id))
			{
				vid.name = sl[0];
				vid.ID = id;
				vid.uploadDate = DateFromString(sl[3]);
				if (vid.uploadDate > _latestDateLimit)
					_latestDateLimit = vid.uploadDate;
				// DEBUG:
				if (vid.uploadDate.toJulianDay() < 0)
					return ID_t::Invalid();

				QString path = pathMap.AbsPath(VideoMap::lastUsedPathId);
				vid.fileSize = sl[4].toULongLong();
				if (n == 6)
					if (config.majorStructVersion == 1 && config.minorStructVersion < 3)
						VideoMap::lastUsedPathId = pathMap.Add(sl[5]);
					else
						VideoMap::lastUsedPathId = sl[5].toULongLong();

				vid.pathId = VideoMap::lastUsedPathId;

				vid.ID.SetFlag(EXISTING_FLAG, vid.fileSize != 0);	// non existing videos have 0 size
				_videoMap[id] = vid;
			}
			pItem = &_videoMap[id];
			++((Video*)pItem)->usageCount;
		}
	}
	IdsFromStruct ids;
	reader.NextLine();
	if (!reader.l().isEmpty() && reader.l()[level] == '[')
	{
		int nChanges = _structFileChangeCount;			  // original value
		_GetTextAndThumbnailIDsFromStruct(reader, ids, level);
		if(nChanges != _structFileChangeCount && album)  // then image came from path in struct
			SetAlbumModified(album->ID);
	}

	if(!pItem->titleID || ids.titleIDVal)		  // do not delete already existing text
		pItem->titleID = ids.titleIDVal;		  // but overwrite if new text is entered
	if(!pItem->descID || ids.descIDVal)
		pItem->descID = ids.descIDVal;


	return id;	// has the correct flag set
}

/*==========================================================================
* TASK:		helper - splits an album definition line into max 3 fields:
*				name, ID,  and path, some of these may be missing
* EXPECTS: s : INPUT string of definition w.o. leading spaces
*		line format can be:
*		   Verson 1.2 and before:
*					<relative path name of album> -> new album
*				or
*					(A:1) or (C:1) - root album: no name, no path (C: - changed)
*				or
*					<original album name>(A:<ID>)<absolute/relative path> -
*						exisiting album possibly unchanged
*			    or
*					<original album name>(C:<ID>)<absolute/relative path> -
*						existing album definitely changed
*			Version 1.3 and later:
*					<full path name of album>
*								i.e. no id, no path
*				or
*					<Name of album>(A:xxx)<path ID of album>
*				or
*					<Name of album>(C:xxx)<path ID of album>
*				where the name may be empty (root album)
*				and path ID is the key in map _idToPathMap
* 
*          changed: OUTPUT if the album changed
*					true when the album is new (given with full path name)
*						or when the type is set to 'C'
* RETURNS: list of fields found  and 'changed' flag
*		   Verson 1.2 and before:
*			line count in list can be:
*						1 - new album	:  this/is/a/new/album
*						2 - name and ID :  thisAlbum(A:ID)
*							album is in parent's path
*						3 - name, ID, path: name(A:ID)path/to/this
*		   Version 1.3 and later:
*			line count in list can be:
*						1 - new album	:  this/is/a/new/album
*						2 - name and ID :  thisAlbum (A:ID)
*							for logical album
*						3 - name, ID, path: name(A:ID)pathID
*							for physical album
* 
* 
* REMARKS:	- "normal" regular function split() could not be used as names
*					may contain braces and commas
*			- result may contain fewer or more fields than required
*			- for root album sl is a single field, the ID which is 1
*			- when the album exists, but marked as 'C' ('changed') then its
*				parent (if it has any) - and only its parent -  must also change
*			- when an old style line is read it is converted to new format
*			when written out
*--------------------------------------------------------------------------*/
static QStringList __albumMapStructLineToList(QString s, bool &changed)
{
	QStringList sl;
	int pos = 0, pos0 = s.lastIndexOf(QChar('('));
	if (pos0 < 0 || (s[pos0+1] != QChar('A') && s[pos0 + 1] != QChar('C')) || s[pos0+2] != QChar(':'))		// no path
	{
		sl.push_back(s);	// unprocessed data: full path name
		changed = true;		// should be changed when processed
		return sl;
	}
	changed = (s.at(pos0+1).unicode() != 'A');

	sl.push_back(s.mid(0, pos0));		// album name, may be empty

	pos0 += 3;				// skip the '(A:' or '(C:'
	pos = s.indexOf(')', pos0);
	if (pos < 0)
		return sl;

	sl.push_back(s.mid(pos0, pos - pos0));	// ID as string
	if (pos + 1 < s.length())
		sl.push_back(s.mid(pos + 1));		// path, or path ID as string
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
*           parent: ID of parent,
*					Set to 0 for root album and NOT to TOPMOST_ALBUM_ID !
*			level:	album level (number of spaces before name in the struct file)
* RETURNS:	ID of new album from structure
* REMARKS:	- 'structFileChangCount' only  differs from zero after read 
*				* if any album was marked as 'C' (changed) instead of 'A'
*				* if any album, image or video is specified with its path
*					(relative to source dir, or absolute if anywhere els)
*					insted of a processed way
*			-if an album id line is in reader then the id inside it must be new
*			 otherwise the file is damaged
*			- when a new album added, then its parent also changed
*			- album definitions are delimited with a single empty line
*			- each album line starts with as many spaces as its level
*			- if the next album definition is on the same level
*				as this album, then returns
*			- expects a single root album
*			- albums are added to '_albumMap' as soon as they are identified
*				and that record is modified later by adding the images and
*				albums to it
*			- throws 'BadStruct' on error
*			- album created here must be added to _albumMap in caller
*				later than its sub albums, because its items may change
*--------------------------------------------------------------------------*/
ID_t AlbumGenerator::_ReadAlbumFromStruct(FileReader &reader, ID_t parent, int level)
{
	Album album = _albumMap[ TOPMOST_ALBUM_ID ];	// temporary album to add to map, root album already added to map
	album.parentId = parent;
	album.pathId = 0;

	bool albumDefinitelyChanged;
	const QStringList sl = __albumMapStructLineToList(reader.l().mid(level), albumDefinitelyChanged);
	int n = sl.size();		// should be 1,2 or 3
							//	1: sl[0] = full path name (not yet processed)
							//	2: top level album, just below album root, sl[0] = album name sl[1] = ID
							//	3: sub album, sl[0] = album name sl[1] = ID, sl[2]= album path	  - also for root album since 1.2.1
	if (!n || n > 3)
		throw BadStruct(reader.ReadCount(),"Wrong album parameter count");
	ID_t id;

	if (n == 1 && sl[0][0] == QChar('('))	// either root album without a name and path or full unprocessed path name 
	{
		album.ID = id = TOPMOST_ALBUM_ID;				// root album, no name
	}
	else
	{
		album.name = sl[0];
		id = (n > 1 ? ID_t(ALBUM_ID_FLAG, sl[1].toULongLong()) : TOPMOST_ALBUM_ID);	// n == 0 => root album, already added;
		album.ID = id;
	}
	// DEBUG
	//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
	//QString sLevels;
	//{
	//	if(level > 0)
	//		for (int i = level; --i; i)
	//			sLevels += ' ';
	//}

	//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
	//{
	//	QFile f("falconG_debug.log");
	//	f.open(QIODevice::Append);
	//	QTextStream ofsdbg(&f);
	//	ofsdbg << sLevels << album.name << QString(": %1\n").arg(album.ID.Val());
	//}
	//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
	// /DEBUG

	Album *aParent = parent.Val() ? &_albumMap[parent] : nullptr;
	uint64_t parentPathId = aParent ? aParent->pathId : NO_ID;

	int	changeCountBefore = _structFileChangeCount;		// save to determine if album changed
												// if any text or the icon ID is new then _structFileChangeCount incremented
						// n == 3 most of the time that's why we check for it first
						// expecting a little speedup
	//int pathId = parentPathId;		// path id of this album, either from struct line or from parent
					
	// album name, ID, parentId are set now set paths
	bool isOldVersion = config.majorStructVersion == 1 && config.minorStructVersion < 3;

	if (n > 1 && !isOldVersion)	// path Id's are already established
	{
		if (n == 2)
			album.pathId = aParent ? aParent->pathId : 1;
		else
			album.pathId = sl[2].toULongLong();	// images are inside this album
		lastUsedAlbumPathId = album.pathId;
	}
	else	// older versions: path is set as text relative to gallery root
	{
		if (n == 3 || n == 2)
		{
			if (n == 3)	// name, ID and path (relative to source gallery's root path or absolute path)
			{
				if (!level)
					album.pathId = 1;	// for topmost album already added to pathMap
				else
				{
					uint64_t id1 = pathMap.Add(sl[2]);	// root relative name or absolute name
					album.pathId = id1;
				}
				lastUsedAlbumPathId = album.pathId;
			}
			else  // name and ID only, level == 0:root, level == 1 top level album path relative to config.dsSrc
			{
				if (!level)	  	// name & ID but no path -> this is relative to (invisible) root
					lastUsedAlbumPathId = NO_ID;
				if (level == 1)	// top level album => its name is path for images and sub albums
				{
					album.pathId = TOPMOST_PATH_ID;
					//	will be path for children, unless set there again
					QString sPath = pathMap[parentPathId];
					lastUsedAlbumPathId = pathMap.Add(sPath + sl[0] + "/");
				}
			}
			// id = sl[1].toULongLong();
			if (id != TOPMOST_ALBUM_ID && _albumMap.contains(id) && _albumMap[id].FullSourceName() != album.FullSourceName())
				throw BadStruct(reader.ReadCount(), QString("'%1'").arg(id.Val()) + FalconG::tr(" - duplicated album ID"));

			if (album.changed)
				albumgen.SetAlbumModified(album);
			album.ID.SetFlag(EXISTING_FLAG, QFileInfo::exists(album.FullSourceName()));
		}
	}
	if (n == 1)   // n == 1  	no ID is determined yet, sl[0] contains either the path (existing image) or just the name (top level folder)
	{								 // TODO
		if (aParent)
			SetAlbumModified(*aParent);		// aParent must exist for each non-root album

		QString path;
		SeparateFileNamePath(sl[0], path, album.name);

		uint64_t uid = pathMap.Add(path);	// handles empty paths, absolute paths, source relative paths and existing paths
		if (uid == NO_ID)	// no path was set in sl[0]
			album.pathId = aParent ? parentPathId : lastUsedAlbumPathId;
		else
			album.pathId = uid;

		path = PrependSourcePathTo(CutSourceRootFrom(path));

		if (QFile::exists(path))		// then not virtual directory => images are inside this folder
		{
			lastUsedAlbumPathId = uid;
			album.ID.SetFlag(EXISTING_FLAG, true);
		}

		bool added = false;
		id = _albumMap.Add(parent, sl[0], added);	// add new album to global album list
		// may invalidate aParent
		album = _albumMap[id];	// and also album is changed
		++_structFileChangeCount;		// then we will write down the processed line into the new file
	}
	if (albumDefinitelyChanged)		// type was 'C' and not 'A'
	{								// in this case immediate parent must be changed too
		SetAlbumModified(album);
		if (aParent != nullptr)
			SetAlbumModified(*aParent);
		++_structFileChangeCount;
	}

	reader.NextLine(); // next line after an album definition line
					   // when empty same level or sub albums follows
	IdsFromStruct ids;
	while (reader.Ok())
	{
		if (reader.l().isEmpty())  // albums are separated by an empty line between them
		{
			if (!ids.IsEmpty())	   
			{
				if (ids.titleIDVal)
					album.titleID = ids.titleIDVal;
				if (ids.descIDVal)
					album.descID = ids.descIDVal;
				_AddAlbumThumbnail(album, ids.thumbnailIDVal);
				ids.Clear();
			}
			while(reader.Ok() && reader.l().isEmpty())	// drop all consecutive empty lines
				reader.NextLine();					// next album definition line
			if (reader.Ok() && reader.l()[level] == ' ')		// new sub album)
			{
				if (level)
				{
					if (isOldVersion)
						if (n == 3)
							album.pathId = pathMap.Add(pathMap[parentPathId] + sl[2]);
						else
							album.pathId = TOPMOST_PATH_ID;
					// else path already added
				}
				while (reader.Ok() && reader.l()[level] == ' ')
				{													// process it
					ID_t aid = _ReadAlbumFromStruct(reader, id, level + 1); // returns when same level sub album is found
					album.items.push_back(aid);
				}
			}
			// all sub-albums are processed
			if (_albumMap.contains(id) && _albumMap[id].changed)	// if the album in _albumMap is not yet filled in, the changed flag still may be set
				SetAlbumModified(album);							// for it because of a sub-album
			_albumMap[id] = album;									// modify album
			// DEBUG
			//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
			//{
			//	QFile f("falconG_debug.log");
			//	f.open(QIODevice::Append);
			//	QTextStream ofsdbg(&f);
			//	ofsdbg << sLevels << "=> " + _albumMap[id].name << "\n";
			//	for(int i =0; i < _albumMap[id].items.size(); ++i) 
			//		ofsdbg << sLevels << QString("   : items[%1] = (%2,%3)\n").arg(i).arg(_albumMap[id].items[i].Val()).arg(_albumMap[id].items[i].Flags());
			//}
			//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
			// /DEBUG
			return id;			 // this new album definition is at the same level as we are
		}
		else
		{				// strings in lText must be empty
			if (reader.l()[level] == '[') // then title, description or thumbnail line
			{
				_GetTextAndThumbnailIDsFromStruct(reader, ids, level);
				if (changeCountBefore != _structFileChangeCount)
				{
					SetAlbumModified(album);
					if (parent.Val())
						SetAlbumModified(parent);	// aParent pointer might be invalidated
				}
			}
			else if(reader.Ok())		   // this must be an image line, unless file is ended
			{							   // but image files are inside the album (one level down)
				ID_t iid = _ReadImageOrVideoFromStruct(reader, level+1, &album, false);		   // false:not album thumbnail
				if (!iid.Val())
					ShowWarning(FalconG::tr("Image not Found in line #%1").arg(reader.ReadCount()), frmMain);
					//throw BadStruct(reader.ReadCount(), FalconG::tr("Image id is 0! Try to remove text after image name in the .struct file!"));
				album.items.push_back(iid);
			}
		}
	}
		// last album
	if (!ids.IsEmpty())
	{
		if (ids.titleIDVal)
			album.titleID = ids.titleIDVal;
		if (ids.descIDVal)
			album.descID = ids.descIDVal;
		if (ids.thumbnailIDVal)
			album.SetThumbnail(ids.thumbnailIDVal);
		ids.Clear();
	}

	_albumMap[id] = album;

	return id;
}

/*=============================================================
 * TASK:	If the album has no thumbnailId, add the first image
 *			in the album as thumbnailId, and if it doesn't have
 *			any image just sub albums then adds the thumbnailId
 *			from the first sub album that has one
 * PARAMS:	album - add thumbnail to this album
 *			id	  - if not 0, it sets this as thumbnail
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void AlbumGenerator::_AddAlbumThumbnail(Album& album, uint64_t id)
{

	if (album.thumbnailId.Val() || album.ID == TOPMOST_ALBUM_ID)
		return;

	if (id)
	{
		album.SetThumbnail(id);	 // when id given thumbnail is from structure file
		return;
	}

	SetAlbumModified(album);	 // now add the first item as a thumbnail
	for(auto &a : album.items)	 
		if (a.IsImage())
		{
			album.SetThumbnail(a);
			return;
		}
	// no image found so far: so only albums are in this album
	for (auto &a : album.items)
	{
		Album& alb = _albumMap[a];
		_AddAlbumThumbnail(alb, 0);
		if (alb.thumbnailId.Val())
		{
			album.SetThumbnail(alb.thumbnailId);
			return;
		}
	}
}

/*==========================================================================
* TASK:		reads whole album hierarchy from structure file
* EXPECTS: 'from' is the full path name of the file
* RETURNS: success
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
bool AlbumGenerator::_ReadStruct(QString fromFile)
{
	bool error = false;

	try
	{
		FileReader reader(fromFile);	// with ReadLine() discard empty lines, and trim lines
		if (!reader.Ok())
			throw BadStruct(reader.ReadCount(), FalconG::tr("Read error"));

		QString line = reader.NextLine(true);	// version line (comment format)

		if (line.left(versionStr.length()) != versionStr)
			throw BadStruct(reader.ReadCount(), FalconG::tr("Bad version string"));
		else
		{
			line = line.mid(versionStr.length());
			int pos = line.indexOf('.'), pos1 = line.lastIndexOf('.');
			if(pos < 0)
				throw BadStruct(reader.ReadCount(), FalconG::tr("Missing '.' from version"));

			config.majorStructVersion = line.left(pos).toInt();
			config.minorStructVersion = line.mid(pos+1, pos1 - pos - 1).toInt(); // if no sub version: all after pos

			_LanguageFromStruct(reader);  // throws when error
			emit SignalSetupLanguagesToUI();

			QString rline = reader.ReadLine();		// discard empty and comments

			if (!_ReadPathTable(reader))
				throw BadStruct(reader.ReadCount(), FalconG::tr("Invalid path table"));
			if (!_ReadOrphanTable(reader))
				throw BadStruct(reader.ReadCount(), FalconG::tr("Invalid orphan table"));

			// from now on we need the empty lines as well
			// root directory may also contain name and path
			rline = reader.l();
			if (rline[0] != '(' || rline[1] != 'A' && rline[1] != 'C')	// old type?
			{
				bool b;
				QStringList sl1 = __albumMapStructLineToList(rline, b);
				if(sl1.size() > 2 && sl1[1] != "1")
					throw BadStruct(reader.ReadCount(), FalconG::tr("Invalid / empty root album line"));
			}

					// recursive album read. there is only one top level album
					// with id == TOPMOST_ALBUM_ID (id == ALBUM_ID_FLAG is not used)
			_ReadAlbumFromStruct(reader, { ALBUM_ID_FLAG, NO_ID }, 0);	// parent ID is 0 for root album!
			_CleanupPathTable();
//#ifdef DEBUG
//			{
//				// DEBUG
//				QMap<QString, int> map;
//				QString s;
//
//				for (auto &a : _albumMap)
//				{
//					s = QString().setNum(a.pathId) + " : Path: ";
//					if (a.pathId == 1)
//						s += QString("(root)");
//					else
//						s += pathMap.Contains(a.pathId) ? pathMap[a.pathId] : QString("Invalid id:%1").arg(a.pathId);
//					s += QString(" for album %1 (id:%2)").arg(a.name).arg(a.ID.Val());
//					map.insert(s, 1);
//				}
//				QFile f("falconG-paths.dbg.txt");
//				f.open(QIODevice::WriteOnly);
//				QTextStream ofs(&f);
//				for (auto i = map.begin(); i != map.end(); ++i)
//					ofs << i.key() << "\n";
//				// /DEBUG
//			}
//			{
//				// DEBUG
//				QMap<QString, int> map;
//				QString s;
//
//				for (auto& a : _imageMap)
//				{
//					s = QString().setNum(a.pathId) + " : Path: ";
//					if (a.pathId == 1)
//						s += QString("(root)");
//					else
//						s += pathMap.Contains(a.pathId) ? pathMap[a.pathId] : QString("Invalid id:%1").arg(a.pathId);
//					s += QString(" for image %1 (id:%2)").arg(a.name).arg(a.ID.Val());
//					map.insert(s, 1);
//				}
//				QFile f("falconG-img-paths.dbg.txt");
//				f.open(QIODevice::WriteOnly);
//				QTextStream ofs(&f);
//				for (auto i = map.begin(); i != map.end(); ++i)
//					ofs << i.key() << "\n";
//				// /DEBUG
//			}
//#endif
			if (error)
				throw BadStruct(reader.ReadCount(),tr("Error"));
		}
		return true;
	}
	catch (BadStruct b)
	{
		QMessageBox(QMessageBox::Critical, FalconG::tr("falconG - Error"),
						FalconG::tr("Damaged structure file!\n"
										"Message: '") +
						b.msg +
						FalconG::tr("\n\n"
										"Processing aborted, because continuing\n"
										"could destroy your old .struct file!\n"
										"%1 lines read so far. ").arg(b.cnt),
							QMessageBox::Abort, frmMain).exec();
		b.cnt = 1;
		throw(b);
		return false;
	}
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
* TASK:	copies directory 'js' to gallery, except 'falconGGen.js'
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
		if(fi.fileName() != "falconGGen.js")
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
	emit SignalToCreateUplinkIcon(dest, "up-icon.png");
	return 0;
}

/*============================================================================
* TASK:		Writes whole directory structure into 'gallery.struct' in a separate thread
* EXPECTS:  wm   (def.  WriteMode::wmOnlyIfChanged)	: should write even when didn't change? 
			bm   (def. BackupMode::wmReplace)		: should replace backup file with original
* GLOBALS:
* REMARKS: Directory structure is written into file gallery.tmp, so if an
*			error occurs then the original structure file is still available.
*			After a successful write
*			if 'keep' is false
*				an existing backup file 'gallery.struct~'
*				is deleted,  then the gallery's *.struct file is renamed
*				to '*.struct~'
*			and the temporary file is renamed to 'gallery.struct'.
*			If 'keep' is true,
*				the old backup file is kept the  *.struct file is renamed
*				to '*.struct.tmp' and the temporary file is renamed
*				to 'gallery.struct'
*			This way the original state (before any changes) can be
*			restored (once only!) if required.
*--------------------------------------------------------------------------*/
int AlbumGenerator::WriteDirStruct(BackupMode bm, WriteMode wm)
{
	while (_structIsBeingWritten)		// before changing again wait for previous write to finish
		;

	if (wm == WriteMode::wmOnlyIfChanged && !_structFileChangeCount)
		return 0;

	_structIsBeingWritten = true;
	pWriteStructThread = new AlbumStructWriterThread(*this, bm == BackupMode::bmKeep);
	connect(pWriteStructThread, &AlbumStructWriterThread::resultReady, this, &AlbumGenerator::_WriteStructReady);
	connect(pWriteStructThread, &AlbumStructWriterThread::finished, pWriteStructThread, &QObject::deleteLater);
	_keepPreviousBackup = bm == BackupMode::bmKeep;

// DEBUG 	pWriteStructThread->start();
	pWriteStructThread->run();
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
	QDir dir;
	if (!dir.exists(QString(__CssDir.ToString())))
		CreateDir(QString(__CssDir.ToString()));

	QString tmpName = __CssDir.ToString() + "tmpfg.css";
	cssCreator.Create(tmpName,false);
	tmpName = BackupAndRename(__CssDir.ToString() + "falconG.css", tmpName);
	if (!tmpName.isEmpty())
	{
		ShowWarning(tmpName, frmMain);
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
	bool multipleLanguages = languages.LanguageCount() > 1;

	QString path, name, ext;						// see if name has a path, which may be either outside or inside of gallery root
	SeparateFileNamePath(base, path, name, &ext);
	if (ext.isEmpty())
		ext = ".html";
	if (path.isEmpty())
		base = config.dsGRoot.ToString() + name;
	else
		base = (config.dsGRoot + path).ToString() + name;
	if (multipleLanguages)
		base += (*languages["abbrev"]).at(language); // e.g. index_en.html

	if (toServerPath)
	{
		path = config.sServerAddress;
		int n = path.indexOf(':');	// may be 'http:/' or 'https:'
		if (n < 0)
			path = (config.bForceSSL ? "https://" :"http://") + path;

		if (path[path.length() - 1] != '/')
			path += '/';
		base = path + base;
	}
	return base + ext;
}

/*========================================================
 * TASK:	Process one image and thumbnail using
 *			converters
 * PARAMS:	im - actual source JPG image
 *			converter - common converter for images
 *			thumbConverter - same for thumbnails
 * GLOBALS:
 * RETURNS:	nothing
 * REMARKS:	- both source and generated images are JPGs
 *			- im may have its dimensions read from structure
 *				file, but it also may be a new image w.o. any
 *				size data read in
 *			- im may contain an image different from a
 *				previous process. It may be new, it may be
 *				of a different size or a newr date than the
 *				processed one, whose name is <im.ID>.jpg
 *-------------------------------------------------------*/
void AlbumGenerator::_ProcessOneImage(Image &im, ImageConverter &converter, std::atomic_int &cnt)
{
	int doProcess = 0;	// suppose neither image nor thumb must be (re)created
	QString src, dst, thumbName;

										// resize and copy and  watermark
	if (!im.ID.Val())	// noimage.jpg in dest folder
		src = PROGRAM_CONFIG::homePath + "sample/res/" + im.name;
	else
		src = (config.dsSrc + im.Path()).ToString() + im.name;   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)

	dst = config.LocalImageDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions); // e.g. imgs/123456789.jpg
	thumbName = config.LocalThumbnailDirectory().ToString() + im.LinkName(config.bLowerCaseImageExtensions);// e.g. thumbs/123456789.jpg
	// DEBUG
	//int calls = 0;
	//qDebug("%s - %d", im.name.toStdString().c_str(), ++calls);
	//if (src.indexOf("iOS") >= 0)
	//	doProcess = doProcess;
	// /DEBUG

	QFileInfo fiSrc(src), fiThumb(thumbName), fiDest(dst);						// test for source image
	bool srcExists = fiSrc.exists(),
		dstExists = fiDest.exists(dst),
		thumbExists = fiThumb.exists();

	if (!srcExists)	// then it might have been removed
	{
		im.ID.SetFlag(EXISTING_FLAG, false);
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

	im.GetResizedDimensions(dstSize);	// into 'im.rsize' and set 'im.bDestFileChangedOrMissing'

	if (imgSize.width() != im.osize.width() || imgSize.height() != im.osize. height())	// source changed rel. to data base: must re-create everything
	{
		im.osize = imgSize;

		im.GetResizedDimensions(dstSize);// must recalculate,
		im.bDestFileChangedOrMissing = true;		// this signals	dest. image changed
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

		if (config.bRegenerateAllImages || (im.bDestFileChangedOrMissing || (fiSize != im.fileSize) || !destIsNewer) )
			doProcess |= prImage;			// then do process

		if (destIsNewer)
			im.uploadDate = dtDestCreated.date();

		if (im.fileSize != fiSize)			// source image size changed: set new size in structure
			im.fileSize = fiSize;
	}
	else
		doProcess |= prImage;

	if (thumbExists)		// then test if this image was modified (width = 0: only added by name)
	{
		QDateTime dtThumb = fiThumb.lastModified();		  // date of creation of thumbnail image
		bool thumbIsNewer = dtThumb > dtSrc;

		if (config.bRegenerateAllImages || _MustRecreateThumbBasedOnImageDimensions(thumbName, im) || !thumbIsNewer)
			doProcess |= prThumb;			// then do process
	}
	else
		doProcess |= prThumb;			// then do process

	if (doProcess)
	{
		WaterMark *pwm = nullptr;
		if (im.ID.Val())				// else default NoImage.jpg
		{
			++_structFileChangeCount;	// when image changes structure must be re-saved

			if (config.waterMark.used)	
				pwm = &config.waterMark;
		}
		converter.flags = doProcess + (im.dontResize ? dontResize : 0) +
							(config.doNotEnlarge ? dontEnlarge : 0);

		imgReader.thumbSize = im.tsize;
		imgReader.imgSize = im.rsize;
		int convRes = converter.Process(imgReader, dst, thumbName, pwm);
		if (convRes < 0)		// converting error -1: can't overwrite, -2: read, -3: write
		{
			QString qs = QString(QMainWindow::tr("Conversion error:\n'%1'\nError code:%2\n")).arg(converter.ErrorText()).arg(convRes);
			ShowWarning(qs, frmMain);
		}
		else
		{
			QFileInfo fi(dst);
//			im.fileSize = fi.size();
			im.uploadDate = fi.lastModified().date();
			QImage dstImage(dst);

			im.dsize = dstImage.size();
			im.changed = im.ID.Val();			// so check in struct file before writing to disk
			_imageMap[im.ID] = im;
		}

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
	if (!_processing || config.dontRegenerateAnyImage)
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

	bool bSaveregenAll = config.bRegenerateAllImages;
	if (config.waterMark.WChanged()) // only  true if changed and the regenerate when changed checkbox is set
		config.bRegenerateAllImages = true;

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
	config.bRegenerateAllImages = bSaveregenAll;
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
	if (!_processing)
		return 0;

	// progress bar
	emit SignalToSetProgressParams(0, _ItemSize(), 0, 1); // phase = 1
	std::atomic_int cnt = TotalCount();	// count of images and videos copied so far

	for (auto &im : _videoMap)
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
		QString src = (config.dsSrc + im.Path()).ToString() + im.name,   // e.g. i:/images/alma.jpg (windows), /images/alma.jpg (linux)
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
QString AlbumGenerator::_IncludeFacebookLibrary() const
{
/*
    <script async defer crossorigin="anonymous"
            src="https://connect.facebook.net/en_US/sdk.js#xfbml=1&version=v8.0" nonce="ouGJwYtd">
    </script>
*/
	return QString(
		R"(
<!--get facebooks js code-->
	<script async defer crossorigin = "anonymous"
			src = "https://connect.facebook.net/)") + (*languages["countryCode"])[_actLanguage] +
					QString( R"(/sdk.js#xfbml=1&version=v8.0" nonce = "ouGJwYtd">
	</script> )");
}

void AlbumGenerator::_LightboxCodeIntoHtml(int nLightboxable)
{
	{
		/*
		* <div id='lightbox'>
			<div id='lb-container'>
				<button id='lb-close-btn' onclick="LightBoxFadeOut()">&#10005;</button>
				<div class="left-edge"></div>
				<button id="lb-prev-btn" onclick="PrevIMage()" >&#10094;</button>
				<img id="lightbox-img" src="" onclick="LightBoxFadeOut()" alt="Teljes méret/Full size">
				<div class="right-edge"></div>
				<button id="lb-next-btn" onclick="NextIMage()" >&#10095;</button>
				<p id="lightbox-caption" class="lightbox-caption"></p>
			</div>
		 </div>
		*/
		_ofs <<
			"<!--Lightbox for displaying full-size images-->\n\n"
			"   <div id='lightbox'>\n"
			"	 <div id='lb-container'>\n"				// vertical layout
			"       <button id='lb-close-btn' onclick='LightboxFadeOut()'> &#x2715; </button>\n";	// close button: always present	   
		if (nLightboxable > 1)			// no lightbox for folders				
		{					// next and prev. buttons only when more than 1 image
			_ofs << "		<div class=\"left-edge\" onclick='PrevImage()'></div>\n"
				"       <button id=\"lb-prev-btn\" onclick='PrevImage()'>&#10094;</button>\n"
				"		<div class=\"right-edge\" onclick='NextImage()'></div>\n"
				"       <button id=\"lb-next-btn\" onclick='NextImage()'>&#10095;</button>\n";
		}
		// image and caption are always present
		_ofs << "	  <div class='lb-image-wrapper'>\n"
			"	     <img id=\"lightbox-img\" src=\"\" onclick=\"LightboxFadeOut()\" alt=\"Teljes méret/Full size\">\n"
			"	  </div>\n"
			"	  <p id=\"lightbox-caption\"></p>\n"
			"	 </div>\n"	// lb-container
			"   </div>\n"	// for lightbox
			"<!--Lightbox  end-->\n\n";
	}
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
	QString updir = (ID == TOPMOST_ALBUM_ID ? "" : "../");
	QUrl url(linkName); // https://andreasfalco.com/albums/album1234.html\"

	_ofs << R"(<div class="fb-share-button" data-href=")"
		<< url.toEncoded()
		<< R"(" data-layout="button_count" data-size="small"><a target="_blank" href="https://www.facebook.com/sharer/sharer.php?u=)"
		<< url.toEncoded()
		<< R"(%2F&amp;src=sdkpreparse" class="fb-xfbml-parse-ignore">)"
		<< (*languages["share"])[_actLanguage]
		<< "</a></div>\n<div id=\"fb-root\"></div>";
}


/*=============================================================
 * TASK   :	write the start and <head> section for the web page
 * PARAMS :	album: actual album  for the web page
 * EXPECTS: 
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- there will be the following variables and tables in 
 *			a javascript block from which the web page will be 
 *			generated:
 * 
 *	const imd='<image dir>, thd='<thumbnail dir>, rsd='<resource dir>',
 *	const ald='<albumdir>, alb='<base name of albums>';
 *	const imgs=[<array of image objects>];
 *	const vids=[<array of video objects>];
 *	const albs=[<array of album objects>
 *			- both itms and albms may be empty
 * 			- image and video objects have the same structure:
 *					{i:'<id>',t:'<title text>',d:'<descr. text>};
 *			  album object also have a thumbnail field:
 *					{i:'<id>',t:'<title>',d:'<descr>,l:'<thumbnail id>'}
 *	const icnt=<count of images>,vcnt=<count of videos>,acnt=<count of albums>;	
 *		    - see :gengallery.js for usage
 *------------------------------------------------------------*/
QString AlbumGenerator::_PageHeadToString(const Album& album)
{
	QString supdir = (album.ID == TOPMOST_ALBUM_ID ? "" : "../"),
			sCssLink = QString("<link rel=\"stylesheet\" href=\"") +
					supdir +
					config.dsCssDir.ToString();

	QString s = QString("<!DOCTYPE html>\n"
		"<html lang = \"" + (*languages["abbrev"])[_actLanguage] + "\">\n"		  // e.g. en from en_GB
		"<head>\n");
	if (config.googleAnalyticsOn)
		s += _GoogleAnaliticsOn();

	s += QString("<meta charset=\"UTF-8\">\n"
		"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
		"<meta name=\"self\" content=\"%1\">\n"
		"<meta name=\"generator\" content=\"falconG\">\n"
		"<title>" + config.sGalleryTitle + "</title>\n").arg(album.ID.Val() );
	if (!config.sDescription.IsEmpty())
		s += QString("<meta name=\"description\" content=\"" + config.sDescription + "\"/>\n");
	if (!config.sKeywords.IsEmpty())
		s += QString("<meta name=\"keywords\" content=\"" + config.sKeywords + "\"/>\n");

	s += QString(sCssLink + "falconG.css\">\n");

	if (config.bFacebookLink)
		s += _IncludeFacebookLibrary();

	if (album.ID == RECENT_ALBUM_ID)
	{
		QString qs = "\n<script type=\"text/javascript\">\nconst lng='" + ((*languages["language"])[_actLanguage]) + "'\n"
			"const imd='../" + config.dsImageDir.ToString() + "'\n"
			"const thd='../" + config.dsThumbDir.ToString() + "'\n"
			"const ald=''\n</script>"			// no videos  or albums here
		;
		s += qs;

	}
	else if (album.items.size()) // const_cast<Album&>(album).ImageCount() + const_cast<Album&>(album).VideoCount() )
	{
		// the real web page is created in javascript in falconG.js
		// these are the data it needs
		s += QString("\n<script type=\"text/javascript\">\n"); 

		// important configuration
		QString qs = QString(	" const imd='%1%2',"		// image dir
								"thd='%3%4',"	// thumbnail dir
								"rsd='%5%6',"	// resource dir
								"ald='%7%8',"	// albumdir - only used for root and generated programs
								"alb='%9',"		// base name of albums
								"lng="			// argument added below
		).arg(supdir).arg(config.dsImageDir.ToString())
		 .arg(supdir).arg(config.dsThumbDir.ToString())
			.arg(supdir).arg("res/")
		 .arg(supdir).arg(config.dsAlbumDir.ToString())
		 .arg(config.sBaseName.ToString())
			;
									
		if ((*languages["language"]).size() > 1)	// 2 letter language abbrev,
			qs += QString("'%1';").arg((*languages["language"])[_actLanguage]);
		else
			qs += "'';";
		
		s += qs;

		int icnt=0, vcnt=0, acnt=0;	// count of images videos and albums

		// *********** arrays of all items **************
		// for albums with images or videos create a JS array with the name of the items in there
		// the array has 6 items for each image or video:
		//		file name (number), type (I: image, V: video), width (w), height (h), title (or ''), description (or '')

		IABase* pIa = nullptr;
		QString qsI, qsV, qsA, *pqs=nullptr;

		for (auto& a : album.items)
		{
			qs.clear();
			if (a.TestFlag(IMAGE_ID_FLAG))			// albums are entered when clicked
			{
				++icnt;
				pIa = &_imageMap[a];
				pqs = &qsI;
			}
			else if (a.TestFlag(VIDEO_ID_FLAG))
			{
				++vcnt;
				pIa = &_videoMap[a];
				pqs = &qsV;
			}
			else				// albums
			{
				++acnt;
				pIa = &_albumMap[a];
				pqs = &qsA;
			}
			if (pIa)
			{
				QSize size = pIa->ThumbSize();

				*pqs += QString("{i:'%1',w:%2,h:%3,").arg(pIa->ID.Val()).arg(size.width()).arg(size.height());
				LanguageTexts* plt = _textMap.Find(pIa->titleID);
				if (plt)
					*pqs += "t:'" + DecodeTextFor((*plt)[_actLanguage], DecodeTextTo::dtJavaScript) + "',";
				else
					*pqs += "t:'',";
				plt = _textMap.Find(pIa->descID);
				if (plt)
					*pqs += "d:'" + DecodeTextFor((*plt)[_actLanguage], DecodeTextTo::dtJavaScript) + "',";
				else
					*pqs += "d:'',";
				if (a.TestFlag(ALBUM_ID_FLAG))
					*pqs += QString("l:'%1'").arg(reinterpret_cast<const Album*>(pIa)->thumbnailId.Val());
				*pqs += "},\n";
			}		
		}
		s += QString("\n var imgs=[\n%1];\n").arg(qsI);		// can't be const, because we must assign to it
		s += QString("\n const vids=[\n%1];\n").arg(qsV);
		s += QString("\n const albs=[\n%1];\n").arg(qsA);

		qs = QString("  const icnt=%1,vcnt=%2,acnt=%3;\n").arg(icnt).arg(vcnt).arg(acnt);
		s += qs + QString("</script>\n");
	}
	s += "<script type=\"text/javascript\" src=\"" + supdir + "js/latestList"+ (*languages["abbrev"])[_actLanguage] +".js\"></script>\n" +
		 "<script type=\"text/javascript\" src=\"" + supdir + "js/falconG.js\"></script>";

	s += QString("\n</head>\n");

	return s;
}

/*=============================================================
 * TASK:	emit a nav bar for an about page
 * PARAMS:	lang: index of language in Languages
 * GLOBALS:	config, _ofs
 * RETURNS: none
 * REMARKS: - about.html file have a limited menu: to index_XX.html
 *				to home (embedding page to select any language)
 *				and for contact (if set)
 *------------------------------------------------------------*/
void AlbumGenerator::_OutputNavForAboutPage(int lang)
{
	auto outputMenuButton = [&](QString id, QString href, QString text, QString hint = QString())
	{
		_ofs << "    <a class = \"menu-item\" id=\"" << id << "\" href=\"" << href << "\">" << text << "</a>\n";
	};
	_ofs << "   <nav>\n";
	outputMenuButton("uplink", RootNameFromBase(config.sMainPage.ToString(), lang), "&nbsp;", (*languages["upOneLevel"])[lang]);
	outputMenuButton("home", config.homeLink, (*languages["toHomePage"])[lang]);
	if (config.bMenuToContact)
		outputMenuButton("contact", "mailto:" + config.sMailTo, (*languages["toContact"])[lang]);
	_ofs << "   </nav>\n";
}

/*=============================================================
 * TASK:	copy texts into about pages
 * PARAMS:	lang - actual language
 * GLOBALS:	config, _ofs
 * RETURNS:
 * REMARKS: - about texts are in files in the gallery source
 *			folder
 *			- about texts must be stored in utf-8 encoded
 *			files named
 *				about_text_XX.txt
 *			where _XX is the language abbreviation for more
 *			than one language and missing for one language
 *			- the about text files may contain html tags
 *			- Paragraphs in the text files are separated by empty lines
 *			and may contain more than one line of text.
 *			They need not be preceeded by <p> tags.
 *			-If they start with a '<X', where 'X' == 'p' or "h"
 *			they will not be enclosed between <p class="about">...</p>
 *			-class="about" is a max 600px wide
 *			centered text with space after each paragraph
 *------------------------------------------------------------*/
int AlbumGenerator::_OutputAboutText(int lang)
{
	bool b = languages.LanguageCount() != 1;
	QString name = config.dsSrc.ToString() + "about_text" + (b ? (*languages["abbrev"])[lang] : "") + ".txt";

	QFile f(name);
	if (!f.open(QIODevice::ReadOnly))
		return -1;

	QTextStream ifs(&f);
	ifs.setCodec("UTF-8");

	QString spara;
	bool inPara=false;	// inside paragraph?
	bool bp=false;	// paragraph's first line started with "<"?
	while (!(spara = ifs.readLine()).isNull())
	{
		spara = spara.trimmed();	// starting/ending white space
		if (spara.isEmpty())		// end of paragraph
		{
			inPara = false;
			if (bp)
			{
				_ofs << "</p>\n";
				bp = false;
			}
		}
		else if (!inPara)
		{
			bp = spara.left(2) != "<h" && spara.left(2) != "<p";
			if (bp)
			{
				spara = "<p class=about>" + spara;
				inPara = true;
			}
			_ofs << spara;

		}
		else
			_ofs << spara << "\n";
	}
	if(inPara)	// not yet closed paragraph
		_ofs << "</p>\n";
	f.close();
	return 0;
}

/*========================================================
 * TASK:	emit the sticky menu line at the top of the window
 * EXPECTS: album - album to be created
 *			uplink - page link if any
 * GLOBALS:	Languages, 'config'
 * RETURNS: nothing
 * REMARKS: - if no uplink is given in config no UP button is generated
 *-------------------------------------------------------*/
void AlbumGenerator::_OutputNav(Album &album, QString uplink)
{
	QString updir = (album.ID == TOPMOST_ALBUM_ID) ? "" : "../";

	auto outputMenuButton = [&](QString id, QString href, QString text, QString onClick=QString(), QString hint=QString())
	{
		QString s = (onClick.isEmpty() ? QString("") : QString(" onclick=\"%1\"").arg(onClick)) +
					(hint.isEmpty() ? QString("") : QString("alt= \"hint\"%1\""));

		s = QString("    <a class = \"menu-item\" id=\"%1\" href=\"%2\"%3>%4</a>\n").arg(id).arg(href).arg(s).arg(text);

		_ofs << s;
	};

	QString qsParent = album.parentId > RECENT_ALBUM_ID ? _albumMap[album.parentId].BareName() : config.sMainPage.ToString();
	qsParent = RootNameFromBase(qsParent, _actLanguage);
	if (album.parentId <= RECENT_ALBUM_ID)	// for uplinks to index you need to go up one directory, for others you do not
		qsParent = "../" + qsParent;
	// menu buttons
	_ofs << "   <nav>\n";
	if (!updir.isEmpty() && !uplink.isEmpty())
		outputMenuButton("uplink", uplink.isEmpty() ? qsParent : uplink, "&nbsp;", (*languages["upOneLevel"])[_actLanguage]);
//		<< "\"><img src=\"" + updir + "res/up-icon.png\" style=\"height:14px;\" title=\"" + (*languages["upOneLevel"])[_actLanguage] + "\" alt=\""
//		+ (*languages["upOneLevel"])[_actLanguage] + "\"></a>\n";			  // UP link
	outputMenuButton("home", updir + config.homeLink, (*languages["toHomePage"])[_actLanguage]);
	if (config.bMenuToAbout || !config.sAbout.IsEmpty())	// About menu is generated even when the checkbox is not checked but there's text in the field
	{
		QString s = config.sAbout;
		if (s.isEmpty())
			s = "about.html";
		outputMenuButton("about", updir + languages.FileNameForLanguage(s, _actLanguage), (*languages["toAboutPage"])[_actLanguage]);
	}
	if (config.bMenuToContact)
		outputMenuButton("contact", "mailto:" + config.sMailTo, (*languages["toContact"])[_actLanguage]);
	if (config.bMenuToDescriptions && album.DescCount() > 0)
		outputMenuButton("descriptions","#",(*languages["showDescriptions"])[_actLanguage], "javascript:ShowHide()");
	if (config.bMenuToToggleCaptions && (album.TitleCount() > 0 || album.ImageCount() > 0) && !(*languages["coupleCaptions"])[_actLanguage].isEmpty())
		outputMenuButton("captionss","#",(*languages["coupleCaptions"])[_actLanguage], "javascript:ShowHide()");
	if (album.SubAlbumCount() > 0  && album.ImageCount() > 0 &&  !(*languages["toAlbums"])[_actLanguage].isEmpty())	// when no images or no albums no need to jump to albums
		outputMenuButton("toAlbums", "#albums", (*languages["toAlbums"])[_actLanguage]);								  // to albums
	if (album.ID > RECENT_ALBUM_ID && config.bGenerateLatestUploads && _latestImages.list.size())	// if there are no images in this category, do not add link
	{
		QString qs = "latest";
		if (languages.LanguageCount() > 1)
			qs += (*languages["abbrev"])[_actLanguage];
		qs += ".html";
		outputMenuButton("latest", qs, (*languages["latestTitle"])[_actLanguage]);
	}
	//// debug
	//"<p style=\"margin-left:10px; font-size:8pt;font-family:Arial;\"id=\"felbontas\"></p>  <!--debug: display screen resolution-->
	_ofs << "&nbsp;&nbsp;";
	_WriteFacebookLink(album.LinkName(_actLanguage, true), TOPMOST_ALBUM_ID);	// only when required

	_ofs << "\n   </nav>\n";
}

/*============================================================================
* TASK:		emits <header> section (not the <head>) of a web page for a given album in a given language
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
			"    <a href=\"" << SiteLink(_actLanguage) << "\">\n"
			"	 <span class=\"falconG\">" << config.sGalleryTitle << "</span>"
			"</a>&nbsp; &nbsp;";

// _actLanguage switch texts
// TODO: when language count is larger than, say 3 use a combo box for selection
	for (int i = 0; i < languages.LanguageCount(); ++i)
		if (i != _actLanguage)
			_ofs << "     <a class=\"langs\" href=\"" + album.NameFromID(i) + "\">" << (*languages["name"])[i] << "</a>&nbsp;&nbsp\n";
	_ofs << "     <br><br><br>\n";
	if (album.ID == RECENT_ALBUM_ID)
	{
		QString qs = (*languages["latestTitle"])[_actLanguage];
		if(!qs.isEmpty())
			_ofs << "     <h2 class=\"gallery-title\">" << DecodeTextFor(qs, dtHtml) << "</h2><br><br>\n";
		qs = (*languages["latestDesc"])[_actLanguage];
		if(!qs.isEmpty())
			_ofs << "     <p class=\"gallery-desc\">"   << DecodeTextFor(qs, dtHtml) << "</p>\n";
	}
	else
	{
		QString title = album.titleID ? DecodeTextFor(_textMap[album.titleID][_actLanguage], dtHtml) : album.name;
			_ofs << "     <h2 class=\"gallery-title\">" << title << "</h2><br><br>\n";
		if (album.descID)
			_ofs << "     <p class=\"gallery-desc\">"   << DecodeTextFor(_textMap[album.descID][_actLanguage], dtHtml) << "</p>\n";
	}

	int nLightboxable = (album.ID == RECENT_ALBUM_ID) ?  config.nLatestCount : const_cast<Album&>(album).ImageCount() + const_cast<Album&>(album).VideoCount();
	if (nLightboxable)
		_LightboxCodeIntoHtml(nLightboxable);
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
		<< QString( (*languages["countOfImages"])[_actLanguage]).arg(album.ImageCount() + album.VideoCount()).arg(album.SubAlbumCount()) << "<br><br>\n"
		<< (*languages["falconG"])[_actLanguage] << "<br>\n"
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
* REMARKS:  - root albums (ID == TOPMOST_ALBUM_ID) are written into gallery root, others
*			  are put into directory 'albums'
*			- common for images and albums as both have thumbnail
*			- video section is written in _WriteVideoContainer()
*--------------------------------------------------------------------------*/
int AlbumGenerator::_WriteGalleryContainer(Album & album, uint8_t typeFlag, int idIndex)
{
	bool isAlbum = (typeFlag & ALBUM_ID_FLAG),
		 isImage = (typeFlag & IMAGE_ID_FLAG)/*,
		 isVideo = (typeFlag & VIDEO_ID_FLAG)*/;

	IdList& idList = album.items;
	if (idIndex >= 0 && idList.isEmpty())
		return isAlbum ? -1 : -2;

	ID_t id = idIndex >= 0 ? idList[idIndex] : RECENT_ALBUM_ID;
	if ((id.Flags() & typeFlag) == 0)	// not this type
		return 0;

	QString title, desc, sImageDir, sImagePath, sThumbnailDir, sThumbnailPath;

	QString sOneDirUp, sAlbumDir;
	//if (isAlbum && !album.Exists())
	//	return -1;

	if (album.ID == TOPMOST_ALBUM_ID )
	{
		sAlbumDir = config.dsAlbumDir.ToString();	// root album: all other albums are inside 'albums'
		sOneDirUp = "";								// and the img directory is here
	}
	else
	{
		sAlbumDir = "";			   // non root album: other albums are here
		sOneDirUp = "../";		   // and img directory here
	}

	sImageDir = (QDir::isAbsolutePath(config.dsImageDir.ToString())		? "" : sOneDirUp) + config.dsImageDir.ToString();
	sThumbnailDir = (QDir::isAbsolutePath(config.dsThumbDir.ToString()) ? "" : sOneDirUp) + config.dsThumbDir.ToString();

	Image *pImage = &_imageMap[NOIMAGE_ID];
	ID_t thumb;
	if (isAlbum)
	{

		if (idIndex >= 0)
			thumb = ThumbnailID(_albumMap[id], _albumMap);
		else
			if (_latestImages.list.size())
				thumb = _latestImages.list[QRandomGenerator::global()->generate() % (_latestImages.list.size())]; //with undeletableitemlist it was  .t; at the end

		if (thumb.Val())
		{
			if (_imageMap.contains(thumb))
				pImage = &_imageMap[thumb];
			else	  // the thumbnail image is not in data base, but
			{		  // it can be present on the disk so set paths from it
				sImagePath = sImageDir + QString("%1.jpg").arg(thumb.Val());
				sThumbnailPath = sThumbnailDir + QString("%1.jpg").arg(thumb.Val());
				if (!QFile::exists(sImagePath) || !QFile::exists(sThumbnailPath))
				{
					sImagePath.clear(); sThumbnailPath.clear();
				}
			}
		}
		else
			pImage = &_imageMap[NOIMAGE_ID];
	}
	else if(isImage)
	{
		pImage = &_imageMap[id];
		if (!pImage->Exists(CHECK))	
			pImage = &_imageMap[NOIMAGE_ID];
	}
	else		// video
	{
		pImage = &_imageMap[_videoMap[id].thumbnailId]; // video thumbnail is an image
		if (!pImage->Exists(CHECK))	
			pImage = &_imageMap[NOIMAGE_ID];
	}

	if (pImage)
	{
		if (pImage->tsize.width() <= 0)
			pImage->SetThumbSize();
	}
	if(idIndex < 0)
	{
		title = (*languages["latestTitle"])[_actLanguage];
		desc = (*languages["latestDesc"])[_actLanguage];
	}
	else
	{
		uint64_t tid = isAlbum ? _albumMap[id].titleID : _imageMap[id].titleID,
				 did = isAlbum ? _albumMap[id].descID : _imageMap[id].descID;

		title = DecodeTextFor(_textMap[tid][_actLanguage], dtHtml);
		desc = DecodeTextFor(_textMap[did][_actLanguage], dtHtml);
		if (isAlbum && title.isEmpty())	// no album title: use folder name
			title = _albumMap[id].name;
	}

	if (pImage && config.bDebugging)
		title += QString(" <br>%1<br>%2").arg(pImage->name).arg(pImage->ID.Val());
	if (title.isEmpty())
		title = "&nbsp;";

	if (sImagePath.isEmpty())		// otherwise name for image and thumbnail already set
	{
		sImagePath = (pImage->ID.Val() ? sImageDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName(config.bLowerCaseImageExtensions) : pImage->name);
		sThumbnailPath = (pImage->ID.Val()? sThumbnailDir : sOneDirUp + "res/") + (pImage->Valid() ? pImage->LinkName(config.bLowerCaseImageExtensions) : pImage->name);
	}

				   /*
				   *	<div class="img-container">
				   *		<div class="%1">							// imatte or amatte
				   *			<div id="%2" w=%3 h=%4 >				// %2:image or album id, %4: height, %3:width
				   *				<img class="%5" %6="%7" alt="%8" onclick="%9" %10>
				   *					// %5:"thumb" or "athumb", %6:"src=" or "data-src", %7:image link, %8:"alt text",%9:"ShowImage()" or album location %10: "style='something'" or ""
				   *			</div>
				   *		</div>
				   *		%11%										// %11:"<p class='desc' lang='language"> or nothing, %12:" or nothing %13 "></p>" or nothing
				   *		<a class="title" href="%9">%8</a>
				   *	</div>
				   */


	QString qsLoc;		// empty for non root albums/images
	if (isAlbum)
		qsLoc = "javascript:LoadAlbum('" +
					sAlbumDir + _albumMap[id].NameFromID(id, _actLanguage, false);
	else
		qsLoc = sImagePath.isEmpty() ? "#" : "javascript:ShowImage('" + sImagePath + "', '" + _EncodeTitle(title);
	qsLoc += +"')";

	QString tagPDesc;
	if (!desc.isEmpty())
		tagPDesc =	"<div class=\"desc\">\n"
					"			<p class='desc' lang='" + (*languages["language"])[_actLanguage] + "'>" + desc + "</p>\n"
					"		</div>\n";

	QString qsOptionalStyle;
	if (pImage && pImage->tsize.width() >= 700)
		qsOptionalStyle = " style=\"max-height:calc(99vw/" + QString().setNum((int)(pImage->Aspect())) + ")\"";

	QString qs = QString(R"(	<div class = "img-container">
		<div class="%1">
			<div )").arg((isAlbum ? "amatte" : "imatte"));							// 1
		if (pImage->ID.Val())
			qs += QString("id=\"%1\" ").arg(pImage->ID.Val());
	qs += QString(R"(>
				<img w=%1 h=%2 class="%3" %4="%5" %10alt="%6" onclick="%7" %8>
			</div>
		</div>
		%9
		<div class="links">
			<div class="title" onclick="%7">%6</div>
		</div>
	</div>

)") .arg(pImage->tsize.width())										// 1
	.arg(pImage->tsize.height())									// 2
	.arg(isAlbum ? "athumb" : "thumb")								// 3
	.arg(idIndex > 2 ? "data-src":"src")							// 4
	.arg(sThumbnailPath)											// 5
	.arg(title)														// 6
	.arg(qsLoc)														// 7
	.arg(qsOptionalStyle)											// 8
	.arg(tagPDesc)													// 9
	.arg(idIndex < 0 ? "id=\"latest\" " :"")						// 10
		;

	_ofs << qs;

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
	if (!(id.IsVideo()))
		return 0;

	QString title, desc, sVideoDir, sVideoPath, sThumbnailDir, sThumbnailPath;

	QString sOneDirUp, sAlbumDir;
	if (album.ID == TOPMOST_ALBUM_ID)
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
	sVideoPath = sVideoDir + ((album.VideoCount() > 0 ? pVideo->LinkName(config.bLowerCaseImageExtensions) : QString()));
	QString sVideoType = sVideoPath.right(3).toLower();
	_ofs << "		<div class=\"img-container\">\n"		//#1
			"			<div";								//#2
	if (pVideo)
		_ofs << " id=\"V" << pVideo->ID.Val() << "\" w=" << config.thumbWidth << " h=" << config.thumbHeight;
	_ofs << ">\n"
			"				<video width=\"" << config.thumbWidth << "\" height=\"" << (int)config.thumbWidth*1080.0/1920.0
		 << "\" onmouseover = \"this.controls=true\" onmouseout=\"this.controls=false\"";
	if (!config.bCanDownload)
		_ofs << "controlsset=\"nodownload\"";
	_ofs << ">\n"
		    "					<source src=\"" << sVideoPath << "\" type = \"video/" << sVideoType << "\">\n"
		 << "				</video>\n";

	title = DecodeTextFor(_textMap[pVideo->titleID][_actLanguage], dtHtml);
	desc = DecodeTextFor(_textMap[(pVideo->descID)][_actLanguage], dtHtml);
	_ofs << "					<div class=\"title\">\n";	//#3	// video in the video directory
	if (pVideo && config.bDebugging)
		title += QString(" <br>%1<br>%2").arg(pVideo->name).arg(pVideo->ID.Val());
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
* REMARKS: - root albums (ID == TOPMOST_ALBUM_ID) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreateOneHtmlAlbum(QFile &f, Album & album, int language, QString uplink, int & processedCount)
{
	if (album.ID.IsExcluded())
		return 0;

	if (!f.open(QIODevice::WriteOnly))
		return 16;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_remDsp.Update(processedCount);
	emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
	emit SignalProgressPos(++processedCount, _albumMap.size() * languages.LanguageCount());

	_ofs << _PageHeadToString(album)	// for root album set random thumbnail to most recent gallery
		 << " <body onload = \"falconGLoad(0)\" onbeforeunload=\"BeforeUnload()\"";
// was 		 << " <body onload = \"falconGLoad(" + QString(album.ID == TOPMOST_ALBUM_ID  && !config.bFixedLatestThumbnail && config.bGenerateLatestUploads ? "0":"0") + ")\" onbeforeunload=\"BeforeUnload()\"";
	if (config.bRightClickProtected)
		_ofs << " oncontextmenu=\"return false;\"";
	_ofs << ">\n"
			"  <div class=\"area\">\n";

	_OutputNav(album, uplink);	/* sticky menu line*/
	_WriteHeaderSection(album);

	_ofs << "<!-- Main section -->\n"
		"   <div class=\"main\" id=\"main\">\n";

	if (album.ID == TOPMOST_ALBUM_ID && config.bGenerateLatestUploads)	// root album and last uploaded?
	{
		_ofs << "<br><br><!-- section for latest uploads -->\n"
			<< "    <section id=\"latest-section\">\n";
		_WriteGalleryContainer(album, ALBUM_ID_FLAG, -1);		// -1: latest uploads> the thumbnail is set by javascript 'GetRandomLastImage()'

		_ofs << "    </section>\n<!-- end section for latest uploads -->\n";
	}
	// get number of images and sub-albums
	if (album.ImageCount(true) )		// force count calculation as images may have been added or removed in the GUI
	{
		_ofs << "<!--the images in this sub gallery-->\n"
			<< "    <div id=\"images\" class=\"fgsection\">" << (*languages["images"])[_actLanguage] << "</div>\n"
			<< "    <section id=\"images-section\">\n";
		// for the images first: content added in javascript (falconG.js)
		//// first the images
		//for (int i = 0; _processing && i < album.items.size(); ++i)
		//	if(album.items[i].IsImage() && !(album.items[i].IsExcluded()))
		//		_WriteGalleryContainer(album, IMAGE_ID_FLAG, i);
		_ofs << "    </section>\n<!-- end section Images -->\n";
	}

	if (album.VideoCount(true))		// force count calculation as images may have been added or removed in the GUI
	{
		_ofs << "<!--start section videos -->\n"
			 << "    <div id=\"videos\" class=\"fgsection\">" << (*languages["videos"])[_actLanguage] << "</div>\n"
			    "    <section id=\"videos-section\">\n";
		// videos are put here in javascript
		//for (int i = 0; _processing && i < album.items.size(); ++i)
		//	if (album.items[i].IsVideo() && !(album.items[i].IsExcluded()))
		//		_WriteVideoContainer(album, i);
		_ofs << "    </section>\n<!-- end section Videos -->\n";
	}

	if (album.SubAlbumCount(true))		// force count calculation as images may have been added or removed in the GUI
	{
		_ofs << "<!--start section albums -->\n"
			<< "    <div id=\"albums\" class=\"fgsection\">" << (*languages["albums"])[_actLanguage] << "</div>\n"
			"    <section id=\"albums-section\">\n";
		// filled in injavascript
		//for (int i = 0; _processing && i < album.items.size(); ++i)
		//{
		//	ID_t idSub = album.items[i];
		//	if ((idSub.IsAlbum()) && !(_albumMap[idSub].ID.IsExcluded()))
		//		_WriteGalleryContainer(album, ALBUM_ID_FLAG, i);
		//}
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
* EXPECTS:	album - create web page for this album
*			Language - index of actual language
*			uplink - uplink : one level up
*			processedCount - output parameter
* GLOBALS:	'_mustRecreateAllAlbums'
*			'config'
*				uplink - used as 'uplink' with the uplink button (unless top level)
*				homeLink - The 'home' button links to this page
*
* RETURNS: 0: OK, 16: error writing file
* REMARKS: - root albums (ID == TOPMOST_ALBUM_ID) are written into gallery root, all others in 'albums'
*			  so sub-album links must point into the 'albums' sub -directory EXCEPT for
*		   - non root: when it not changed and need not re-create all albums, just signals
*			 it processed the album
*			- uses _CreatePageInner (co-function) to recursively create sub albums
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreatePage(Album &album, int language, QString uplink, int &processedCount)
{
	if (album.ID.IsExcluded())
		return 0;

	_actLanguage = language;

	QString s;
	if ( album.ID == TOPMOST_ALBUM_ID)		// top level: use name from config
		s = config.dsGallery.ToString() + RootNameFromBase(config.sMainPage.ToString(), language);
	else		 // all non root albums are either in the same directory or in separate language directories inside it
		s = config.LocalAlbumDirectory().ToString() + album.NameFromID(language);

	QFile f(s);
	if (!_mustRecreateAllAlbums && f.exists() && !album.changed /*&& !_structFileChangeCount*/)
	{
		_remDsp.Update(processedCount);
		emit SignalToShowRemainingTime(_remDsp.tAct, _remDsp.tTot, _albumMap.size(), false);
		emit SignalProgressPos(++processedCount, _albumMap.size() * languages.LanguageCount());
	}
	else
		_CreateOneHtmlAlbum(f, album, language, uplink, processedCount);

	if (_processing) 		// create sub albums
	{
		if (album.SubAlbumCount() >0)
		{
			uplink = album.NameFromID(language);
			if (album.ID == TOPMOST_ALBUM_ID)
				uplink = QString("../") + uplink;

			for (int i = 0; _processing && i < album.items.size(); ++i)
			{
				if((album.items[i].IsAlbum()) )
					_CreatePage(_albumMap[album.items[i]], language, uplink, processedCount);
			}
		}
	}
	return 0;
}

/*=============================================================
 * TASK:	Creates "about" pages, as many as there are
 *			languages.
 * PARAMS:
 * GLOBALS: config
 * RETURNS:
 * REMARKS: Existing 'about pages' ARE overwritten!
 *------------------------------------------------------------*/
int AlbumGenerator::_CreateAboutPages()
{
	bool b = languages.LanguageCount() != 1;

	QString name = config.sAbout.ToString();
	if (name.isEmpty())
		name = "about.html";

	QString path, fn, fext;
	SeparateFileNamePath(name, path, fn, &fext);
	path = (config.dsGallery + config.dsGRoot).ToString();

	for (int i = 0; i < languages.LanguageCount(); ++i)
	{
		if (b)
			name = fn + (*languages["abbrev"])[i] + fext;
		name = path + name;

		QFile f(name);
		if (!f.open(QIODevice::WriteOnly))
			return -1;

		_ofs.setDevice(&f);
		_ofs.setCodec("UTF-8");

		_ofs << _PageHeadToString(_albumMap[TOPMOST_ALBUM_ID])
			<< "<body>\n";
		_ofs << "   <div class=\"header\">\n";

		_OutputNavForAboutPage(i);
		_ofs << "     <br><br>\n"
			"    <a href=\"" << SiteLink(i) << "\">"
			"<span class=\"falconG\">" << config.sGalleryTitle << "</span>"
			"</a>&nbsp; &nbsp;";
		if (b)
		{
			for (int j = 0; j < languages.LanguageCount(); ++j)
			{
				if (j != i)
				{
					_ofs << "&nbsp;&nbsp;<a class = \"langs\" href=\"";
					_ofs << path + fn + (*languages["abbrev"])[j] + fext << "\">" << (*languages["name"])[j];
					_ofs << "</a>\n  </div>\n";
				}
			}
		}

		if (_OutputAboutText(i) < 0)
			ShowWarning(QString(tr("Missing or unreadable \"about_text%1.txt\" file")).arg((*languages["abbrev"])[i])), frmMain;

		_ofs << "<footer class = \"footer\"><br>\n"
			<< (*languages["falconG"])[_actLanguage] << "<br>\n"
			"</footer>\n";

		_ofs << "</body>\n</html>\n";
		f.close();
	}
	return 0;
}

/*============================================================================
* TASK: Create a very basic home page
* EXPECTS:	config fields are set, albums are read in (_albumMap, etc)
* GLOBALS:	'config
* RETURNS: 0: OK, -1: error writing file
* REMARKS: file is named <main gallery page name> except when it is the same
*			as the config.sUplink, in which case it has an underscore at the front
*--------------------------------------------------------------------------*/
int AlbumGenerator::_CreateHomePage()
{
	if (languages.LanguageCount() == 1)
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

	_ofs << _PageHeadToString(_albumMap[TOPMOST_ALBUM_ID])
		<< "<body>\n";

// NO menus	_OutputNav(_albumMap[TOPMOST_ALBUM_ID], QString());	/* sticky menu line*/

	_ofs << "<!--Header section-->\n"
		<< "<header>\n<span class=\"falconG\">" << config.sGalleryTitle << "</span>&nbsp; &nbsp;";
	_WriteFacebookLink(_albumMap[TOPMOST_ALBUM_ID].LinkName(_actLanguage, true), TOPMOST_ALBUM_ID);	// when required
	_ofs << "<p class=\"menu-line\">\n";
	// languages links
	for (int i = 0; i < languages.LanguageCount(); ++i)
	{
		// menu buttons
		if (config.bMenuToAbout || !config.sAbout.IsEmpty())	// About menu is generated even when the checkbox is not checked but there's text in the field
		{
			QString qs = config.sAbout.IsEmpty() ? "about.html" : config.sAbout.ToString();
			_ofs << "<a href=\"";
			_ofs << languages.FileNameForLanguage(qs,i) << "\">" << (*languages["toAboutPage"])[i];
			_ofs << "</a>\n";
		}
		if (config.bMenuToContact)
			_ofs << "<a href=\"mailto:" << config.sMailTo << "\">" << (*languages["toContact"])[i] << "</a>\n";
		_ofs << "<br>\n";
	}
	_ofs << "</header>\n"
			"<!-- Main section -->\n"
			"<main id=\"main\">\n"
			"\t<div  class=\"fgsection\">\n";
	for (int i = 0; i < languages.LanguageCount() - 1; ++i)
		_ofs << QString("\t\t<a href=\"%1\">%2</a>\n").arg(languages.FileNameForLanguage(fn, i) + ".html").arg((*languages["name"])[i])
			 << "\t\t&nbsp;&nbsp;-&nbsp;&nbsp;\n";
	_ofs << QString("\t\t<a href=\"%1\">%2</a>\n").arg(languages.FileNameForLanguage(fn, languages.LanguageCount()-1) + ".html").arg((*languages["name"])[languages.LanguageCount()-1]);

	_ofs <<	" <!** ... TODO ... -->\n"
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

	emit SignalToSetProgressParams(0, _albumMap.size() * languages.LanguageCount(), 0, 2); // phase = 3

	_remDsp.Init(_albumMap.size() * languages.LanguageCount());


	QString uplink = config.sUplink.ToString();	 // points to embedding page, relative to html
											 // directory on server (inside e.g. public_html)
											 // e.g. single language:
											 //		index.html corresponds to https://your.site.com/index.html
											 // multiple languages:  as many pages as are languages:
											 // https://your.site.com/index_en.html, https://your.site.com/index_hu.html, etc)
											 // no default: can be empty or name w. or w.o extension: .htm, .html or .php
//	bool multipleLanguages = languages.LanguageCount() > 1;
	QString path, name;

	int cnt = 0;

	if (languages.LanguageCount() != 1 && config.bSeparateFoldersForLanguages)
	{
		int ask = config.doNotShowTheseDialogs;
		config.doNotShowTheseDialogs.v |= int(dboAskCreateDir);
		config.defaultAnswers[dboAskCreateDir] = QMessageBox::Yes;

		for (int lang = 0; _processing && lang < languages.LanguageCount(); ++lang)
			CreateDir((*languages["abbrev"])[lang]);

		config.doNotShowTheseDialogs = ask;
	}

	for (int lang = 0; _processing && lang < languages.LanguageCount(); ++lang)
	{
										// home of gallery root	 like 'index' or 'index.html'
		config.homeLink = uplink.isEmpty() ? config.sMainPage.ToString() : config.sUplink.ToString();

		_CreatePage(_albumMap[TOPMOST_ALBUM_ID], lang, uplink, cnt);	// all albums go into the same directory!
	}
	_CreateHomePage();

	emit SignalToSetProgressParams(0, 100, 100, 1);		// phase2

	if(config.bMenuToAbout)		// only create about pages when the checkbox is checked
		_CreateAboutPages();

	return 0;
}

/*=============================================================
 * TASK:	Cleans up destination gallery by removing all
 *			images and folders, which were removed from the source
 * PARAMS:
 * GLOBALS:
 * RETURNS:	0: success, 64 error
 * REMARKS:
 *------------------------------------------------------------*/
int AlbumGenerator::_CleanUpOutput()
{
	int result = 0;
	QDir dir;
	// cleanup albums
	dir.setPath((config.dsGallery + config.dsGRoot + config.dsAlbumDir).ToString());
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	dir.setSorting(QDir::Name);
	QFileInfoList lsFi = dir.entryInfoList();	// e.g albumXXXXX_en.html
	for (auto &fi : lsFi)
	{
		QString s = fi.fileName();
		s = s.mid(config.sBaseName.ToString().length());	// start of id number
		if (s.at(0).isNumber())
		{
			int posd = 0;
			while (s.at(posd).isNumber())
				++posd;
			ID_t id = {ALBUM_ID_FLAG, s.left(posd).toULongLong()};
			s = fi.filePath();
			if (!_albumMap.contains(id))
				if (!dir.remove(s))
					result = 64;
		}
	}
	// remove videos
	dir.setPath((config.dsGallery + config.dsGRoot + config.dsVideoDir).ToString());
	lsFi = dir.entryInfoList();
	for (auto &fi : lsFi)
	{
		QString s = fi.fileName();	// start of id number
		int posd = s.indexOf('.');
		ID_t id = { VIDEO_ID_FLAG, s.left(posd).toULongLong() };
		if (!_videoMap.contains(id))
			if (!dir.remove(fi.filePath()))
				result = 64;
	}
	// remove images
	dir.setPath((config.dsGallery + config.dsGRoot + config.dsImageDir).ToString());
	lsFi = dir.entryInfoList();
	for (auto &fi : lsFi)
	{
		QString s = fi.fileName();	// start of id number
		int posd = s.indexOf('.');
		ID_t id = {IMAGE_ID_FLAG, s.left(posd).toULongLong()} ;
		if (!_imageMap.contains(id))
		{
			if (!dir.remove(fi.filePath()))
				result = 64;
			s = (config.dsGallery + config.dsGRoot + config.dsThumbDir).ToString() + fi.fileName();
			if (!dir.remove(s))
				result = 64;
		}
	}
	return result;
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

	for (auto &a : _imageMap)
		if (a.ID.Val() && a.uploadDate >= dt)
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
		for (auto &a : _videoMap)
			if (a.ID.Val() && a.uploadDate >= dt)
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
 *			- real generation is set in javascript file falconG.js
 *------------------------------------------------------------*/
int AlbumGenerator::_DoLatestJs()
{
	// create array of latest uploads
	int n = _CollectLatestImagesAndVideos(_latestImages);	// limited # can be selected
	if (!n)
		return 0;

	// create one file for each languages named 'latestList_XX.js'

	for (int lang = 0; lang < languages.LanguageCount(); ++lang)
	{
		QString qsName = config.GalleryRoot().ToString() + "js/latestList" + (*languages["abbrev"])[lang] + ".js";

		QFile f(qsName);
		if (!f.open(QIODevice::WriteOnly))
			return 16;

		QTextStream ofjs(&f);
		ofjs.setCodec("UTF-8");

		ofjs << "// Copyright  András Sólyom (2018-" << PROGRAM_CONFIG::copyrightYear << 
			")\n// email:   solyom at andreasfalco dot com, andreasfalco at gmail dot com).\n"
			"\n"
			"// date of latest upload: " << _latestDateLimit.toString() << "\n"
			"// period: " << config.newUploadInterval << " days, count: " << n << ",max count : " << config.nLatestCount << "\n\n"
			"const ids =[\n";			// array for all max number of ID-s from which the imgs array is created

		for (auto &latest : _latestImages.list)
		{
			Image* pim;
			//Video* pvid;	 			no videos TODO?
			ID_t idt = (ID_t)latest;
			if (idt.IsImage())
			{
				pim = &_imageMap[idt];
				pim->SetThumbSize();
				ofjs << "{ i:" << (idt.Val()) << ",w:" << pim->tsize.width() << ",h:" << pim->tsize.height();
				if (pim->titleID)
				{
					QString s = DecodeTextFor(_textMap[pim->titleID][lang], dtJavaScript);
					ofjs << ",t:\"" << DecodeTextFor(_textMap[pim->titleID][lang], dtJavaScript) << "\"";
				}
				else
					ofjs << ",t:''";
				if (pim->descID)
					ofjs << ",d:\"" << DecodeTextFor(_textMap[pim->descID][lang], dtJavaScript) << "\"";
				else
					ofjs << ",d:''";
				ofjs << "},\n";
			}
			//			no videos TODO?
			//else if (idt.IsVideo())	
			//{
			//	pvid = &_videoMap[idt];
			//	ofjs << "{ id:" << (idt.Val()) << ",w:" << config.thumbWidth << ",h:" << config.thumbHeight;
			//	if (pvid->titleID)
			//		ofjs << ",t:\"" << DecodeTextFor(_textMap[pvid->titleID][lang], 2, true) << "\"";
			//	if (pvid->descID)
			//		ofjs << ",d:\"" << DecodeTextFor(_textMap[pvid->descID][lang], 2, true) << "\"";
			//	ofjs << "},\n";
			//}
		}
		ofjs << "];\n"
			<< "var cnt = " << (config.nLatestCount <= n ? config.nLatestCount.v : n) << ";\n"; // # of random images to show
		f.close();
	}
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

	if(languages.LanguageCount() > 1)
		postFix = (*languages["abbrev"])[lang];
	fileName = baseName + postFix + ".html";

	Album albumLatest;
	albumLatest.ID = RECENT_ALBUM_ID;

	QFile f(fileName);
	if (!f.open(QIODevice::WriteOnly))
		return 16;

	_ofs.setDevice(&f);
	_ofs.setCodec("UTF-8");

	_ofs << _PageHeadToString(_albumMap[RECENT_ALBUM_ID])
		 << " <body onload = \"falconGLoad(1)\"";
	if (config.bRightClickProtected)
		_ofs << " oncontextmenu=\"return false;\"";
	_ofs << ">\n"
		"  <div class=\"area\">\n";

		/* sticky menu line  */
	_OutputNav(albumLatest, "../index" + (*languages["abbrev"])[_actLanguage]+".html");

	_WriteFacebookLink(fileName, RECENT_ALBUM_ID);	// only when required

	_ofs << "</nav>\n";
	// - end of navigation menus -----

	_WriteHeaderSection(albumLatest);

	_ofs << "<!-- Main section -->\n"
			"<br><br>"
			"   <div class=\"main\" id=\"main\">\n"
			"		<section id=\"images-section\">\n"
            "		</section>\n"
			"	</div>\n"
			"</div>\n"
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

	for (int i = 0; i < languages.LanguageCount() && !res; ++i)
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
int AlbumGenerator::ProcessAndWrite()
{
	if (!_CreateBaseDirectories()) // from 'config'
		return 64;

	emit SignalAlbumStructWillChange();

	_ProcessImages();	// copy from from source into image directory
						// determine image dimensions and latest upload date
	_ProcessVideos();

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
		i |= _SaveFalconGCss();		// 0 | 8
	if (_processing)
		i |= _DoPages();			// 0 | 16
	emit SignalToSetProgressParams(0, 100, 100, 2);		// phase3 cleaning up
	if (_processing && config.bCleanupGalleryAfterGenerate)
	{
		emit SignalToSetProgressParams(0, 100, 100, 3);		// phase3 cleaning up
		i |= _CleanUpOutput();		// 0 | 64
	}
	emit SignalToSetProgressParams(0, 100, 0, 0);		// reset phase to 0
	_processing = false;

	// do not save after read of structure
	emit SignalAlbumStructChanged(_structFileChangeCount || !_slAlbumsModified.isEmpty());   // all album and image data

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
	_structIsBeingWritten = false;

	if (!s.isEmpty())		// then error message
	{
		QMessageBox(QMessageBox::Warning, tr("falconG - Generate"), s, QMessageBox::Close, frmMain).exec();
		return;
	}

	s = BackupAndRename(sStructPath, sStructTmp, _keepPreviousBackup);
	if (!s.isEmpty())
		QMessageBox(QMessageBox::Warning, tr("falconG - Generate"), s, QMessageBox::Close, frmMain).exec();
	//else
	//	SetChangesWritten();	// clear all 'changed' flag
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
		if (!cnt)
			++cnt;
		tTot = (time_t)((double)tAct / (double)cnt * max);
		tprev = tAct;
	}
}
Video* VideoMap::Find(ID_t id, bool useBase)
{
	uint64_t mask = useBase ? BASE_ID_MASK : 0xFFFFFFFFFFFFFFFFul;
	Video vid;
	uint64_t vidid = id.Val();
	for (auto i = begin(); i != end(); ++i)
		if ((i.key().Val() & mask) == (vidid & mask))
			return &i.value();
	return nullptr;
}

Video* VideoMap::Find(QString FullSourceName)
{
	Video vid;
	QString path;
	SeparateFileNamePath(FullSourceName, path, vid.name);

	Video::searchBy = Video::byFullSourceName;
	for (auto i = begin(); i != end(); ++i)
		if (i.value() == vid)
			return &i.value();
	return nullptr;
}

/*============================================================================
* TASK: Add a video to the list of all videos
* EXPECTS:  path - full or config.dsSrc relative path name of the video
*			added- output parameter: was this a new video?
* GLOBALS:
* RETURNS: id of video in map with the VIDEO_ID_FLAG
* REMARKS: - id stored in video map has no VIDEO_ID_FLAG set
* RETURNS: Id of image added + if it was added in 'added'
* REMARKS: - Id of image is the CRC32 of the file name only
*		   - If the same image ID is used and not added then
* 			 the usage Count is increased
*		   - when collisions occur adds a unique value above 0xFFFFFFFF to the ID
*		   - even non-existing videos are added to map
*		   - if an video with the same name, but with different path is found
*				and the one in the data base does not exist, replace the one
*				in the data base with the new video even when it also does not exist
*--------------------------------------------------------------------------*/
ID_t VideoMap::Add(QString path, bool& added)
{
	QString basePath;

	if (!QDir::isAbsolutePath(path))			// common part of path is not stored
		basePath = config.dsSrc.ToString();		// but we need it

	Video vid;
	QString lpath;
	path = CutSourceRootFrom(path);
	SeparateFileNamePath(path, lpath, vid.name);

	QDir dir;
	vid.ID.SetFlag(EXISTING_FLAG, dir.exists(basePath + lpath + vid.name));
	if (vid.Exists())
		vid.pathId = AlbumGenerator::lastUsedAlbumPathId;

	added = false;
	uint64_t id64 = CalcCrc(vid.name, false) ;	// just by name. CRC can but id can never be 0

	QFileInfo fi(path);

	Video* found = Find({ VIDEO_ID_FLAG, id64 }); // check if a file with this same base id is already in data base?
							  // can't use reference as we may want to modify 'found' below
	if (config.bKeepDuplicates)
	{
		do
			id64 += ID_INCREMENT;
		while (Find({ VIDEO_ID_FLAG, id64 }, false)); // full by ID
	}
	else
	{
		if (found->Valid())		  // yes an video with the same base ID as this video was found
		{						  // the ID can still be different from the id of the found video though
			do					  // loop thorugh all videos with the same base ID
			{
				if (found->name == vid.name)	// if name is the same too then same video
				{								// even when they are in different directories!
					if (found->ID.DoesExist())
						if (!vid.ID.DoesExist() || (vid.ID.DoesExist() && (found->uploadDate >= fi.lastModified().date() || found->fileSize >= fi.size())))
						{
							++found->usageCount;
							return found->ID;	  // do not change path as video already existed
						}
					id64 = found->ID.Val();		// else found video did not exist,
					break;				// replace it with this video but with the same ID as the old one
				}
				else						// same ID, different name - new video
					id64 += ID_INCREMENT;
				// repeat until a matching name with the new ID is found or no more videos
			} while ((found = Find({ VIDEO_ID_FLAG, id64 }, false))->Valid()); // full by ID
		}
	}
	// now I have a new video to add
	// new video:
	if (vid.ID.DoesExist())
	{
		vid.fileSize = fi.size();
		vid.uploadDate = fi.lastModified().date();
	}

	added = true;
	vid.ID = { VIDEO_ID_FLAG, id64 };
	insert(vid.ID, vid);
	return vid.ID;
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
		default: return pathId < i.pathId || (pathId == i.pathId && name < i.name);
	}
}

bool Video::operator==(const Video& i)
{
	switch (searchBy)
	{
		case byID: return ID == i.ID;
		case byBaseID: return (ID.Val() & BASE_ID_MASK) == (i.ID.Val() & BASE_ID_MASK) ? (name.toLower() == i.name.toLower() ? true : false) : false;
		case byName: return name == i.name;
		default: return pathId == i.pathId && name == i.name;
	}
}

QString Video::AsString(int width, int height)
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

QSize Video::ThumbSize() const
{
	return ThumbSizeFromId(thumbnailId);
}

/*=============================================================
 * TASK:   recursive source album, and image/video deletion
 *			from data base and conditionally from disk
 * PARAMS:	album			 - remove from this album's items
 *			ix:				 - index of item to remove from 'items'
 *			fromDisk		 - delete source files from disk too
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- if the item to be deleted is a folder and 'fromDisk'
 *				is specified all files inside it will be deleted, 
 *				even if they were not in the data base
 *			- tries to move files into trashcan/recycle bin first
 *			- only deletes images when no album contains them and
 *				they are not used as thumbnails either. Otherwise
 *				just decrement the counters
 *------------------------------------------------------------*/
void AlbumGenerator::_RemoveItem(Album& album, int ix, bool fromDisk)
{
	QString path; 
	ID_t id = album.items[ix];
	if (id.IsAlbum()) // this item is an album, so all of >>its<< items
	{						// must be removed
		Album& subAlbum = _albumMap[id];
		path = subAlbum.FullSourceName();
		_RemoveAllItemsFrom(id, fromDisk);
		if (album.thumbnailId.Val() && _imageMap.contains(album.thumbnailId))
		{
			Image* img = &_imageMap[album.thumbnailId];
			--img->thumbnailCount;
			if (!img->usageCount && !img->thumbnailCount)
			{
				_imageMap.remove(album.thumbnailId);
				if (fromDisk && !QFile::moveToTrash(path))
					QFile::remove(path);
			}
		}
		_albumMap.remove(id);
		// folders and files may be left in sub-folders
		// if they were not in the data base even after this!
		if (fromDisk && !QFile::moveToTrash(path))
			RemoveFolderRecursively(path);		// so those must also be deleted

		album.DecrementAlbumCount();
	}
	else 
	{
		if (id.IsImage())    // remove from 'album'
		{
			Image* img = &_imageMap[id];
			if (!--img->usageCount && (!img->thumbnailCount)) 		// nobody uses this?
			{	// if only used as thumbnail and the file is removed from disk, can't keep it
				path = img->FullSourceName();
				_imageMap.remove(id);
				if (fromDisk && (img->usageCount || img->thumbnailCount) && !QFile::moveToTrash(path))
					QFile::remove(path);
			}
			album.DecrementImageCount();
		}
		else if (id.IsVideo())    // remove from this album
		{
			Video* vid = &_videoMap[id];
			if (!--vid->usageCount)
			{
				path = vid->FullSourceName();
				_videoMap.remove(id);
				if (fromDisk && (vid->usageCount) && !QFile::moveToTrash(path))
					QFile::remove(path);
			}
			album.DecrementVideoCount();
		}
		// remove invalid item
		album.items.remove(ix);
	}
}

/*=============================================================
 * TASK:   recursive source album, and image/video deletion
 *			from data base and conditionally from disk
 * PARAMS: albumID			 - remove all of this album's items
 *			fromDisk		 - delete source files from disk too
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- if any item to be deleted is a folder all files inside
 *			  it will be deleted, even if they were not in the data base
 *			- tries to move files into trashcan^recycle bin first
 *------------------------------------------------------------*/
void AlbumGenerator::_RemoveAllItemsFrom(ID_t albumID, bool fromDisk)
{
	Album& album = _albumMap[albumID];
	QString path;
	for (int ix = 0; ix < album.items.count(); ++ix)
		_RemoveItem(album, ix, fromDisk);
	album.items.clear();
	albumgen.SetAlbumModified(album);
}
/*=============================================================
 * TASK:   recursive source album, and image/video deletion
 *			from data base and conditionally from disk
 * PARAMS: albumID			 - remove from this album's items
 *		   iconsForThisAlbum - remove also icons for items
 *		   ilx				 - list of indexes in album's items
 *								of items to be removed
 *			fromDisk		 - delete source files from disk too
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- if any item to be deleted is a folder all files inside
 *			  it will be deleted, even if they were not in the data base
 *			- tries to move files into trashcan/recycle bin first
 *------------------------------------------------------------*/
void AlbumGenerator::_RemoveItems(ID_t albumID, bool iconsForThisAlbum, IntList ilx, bool fromDisk)
{
	Album& album = _albumMap[albumID];
	QString path;
	for (auto ix : ilx)
		album.items[ix].SetFlag(DELETE_IT_FLAG, true);
	// now delete marked elements (this way ilx need not be ordered)
	for (int ix = album.items.size() - 1; ix >= 0; --ix)
	{
		if (album.items[ix].ShouldDelete())
		{
			_RemoveItem(album, ix, fromDisk);
			if(iconsForThisAlbum)
				fileIcons.Remove(ix);	// fileIcons in thumbnailView.cpp
		}
	}
	album.RecalcItemCounts();			// counts would reflect th number of items
	albumgen.SetAlbumModified(album);
}

/*=============================================================
 * TASK:	remove albums and others from structures
 *			and possibly from >>source<< gallery on disk too
 * PARAMS:	albumID - parent album, whose items are to be removed
 *			ilx - array of id-s to remove. Ids have type flag set
 *				this must be ordered from last item to first
 *			fromdisk:deletes it from disk too, but only after confirmation
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void AlbumGenerator::RemoveItems(ID_t albumID, IntList ilx, bool fromDisk, bool iconsForThisAlbum)
{
	if (fromDisk)
		if(QMessageBox::question(frmMain, tr("falconG - Warning"),
								tr(	"If a folder is removed all the files and folders inside it will be deleted too!\n\n"
									"This cannot be undone!\n\n"
									"Really delete the selected items from disk?")) != QMessageBox::Yes)
			return;
	emit SignalAlbumStructWillChange();
	_RemoveItems(albumID, iconsForThisAlbum, ilx, fromDisk);	// also from icon list for this album
	albumgen.WriteDirStruct(BackupMode::bmKeep, WriteMode::wmAlways);				// keep .struct~ file and create a new backup from the original
						
	emit SignalAlbumStructChanged(false);						// album structure changed and saved
}

void AlbumGenerator::SetAlbumModified(Album& album)
{
	if (_slAlbumsModified.indexOf(album.ID) < 0)
	{
		_slAlbumsModified << album.ID;
		album.changed = true;
	}
}
void AlbumGenerator::SetAlbumModified(ID_t albumId)		// albumId must be valid
{
	if (_slAlbumsModified.indexOf(albumId) < 0)
	{
		Album& album = _albumMap[albumId];
		_slAlbumsModified << albumId;
		album.changed = true;
	}
}
