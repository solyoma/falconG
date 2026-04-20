#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QString>
#include <QMap>

enum DialogBitsOrder : int {
	dboNone,
	dboAskBeforeClosing,
	dboShowAfterSavingCss,
	dboSaveEdited,
	dboSaveConfig,
	dboAskCreateDir,
	dboAskSynchronize,
	dboAskToMoveIntoBaseAlbum,		// when drag & drop
	dboAskToMoveFromOrIntoBaseAlbum,
	dboShowVideoThumbnailProblem,
	dboAskIfToMoveIntoParentFolder,
	dboMax
};
enum _What : int { wNone, wColor, wBackground, wNoClosingBrace = 0x8000 };
enum Decoration {	   // _CTextDecoration
	tdNone = 0, tdUnderline = 1, tdLinethrough = 2, tdOverline = 4,
	tdSolid = 8, tdDotted = 16,
	tdDashed = 32, tdDouble = 64, tdWavy = 128
};
enum Align { alNone, alLeft, alRight, alCenter };			// _CTextAlign
enum Style : unsigned short { fNone = 0, fBold = 1, fItalic = 2, };	// _CtextStyle
enum FeatureOp { foClearAll, foSet, foUnset, foClearOthersAndSet };
enum ShadowPart { spUse, spHoriz, spVert, spBlurR, spSpread, spColorName };  // _CShadow
enum GradStop { gsStart, gsMiddle, gsStop };   // 0, 1, 2
enum BorderSide { sdNone = 0, sdTop = 1, sdRight = 2, sdBottom = 4, sdLeft = 8, sdAllSides = 15 };
enum BorderStyle { bsNone, bsSolid, bsDotted, bsDashed, bsDouble, bsGroove, bsRidge, bsInset, bsOutset };
enum BackgroundImageSizing {
	hNotUsed,
	hAuto, 			//	background-size: auto, background-style	 original size
	hCover,			//	background-size: cover,		whole visible screen (might stretch or crop)
	hContain,		//	background-size: contain,			whole image is inside
	hTile			//  background-size: horiz vert, background-repeat: repeat (default!)
};
// separate fonts, colors for all these, sync with falconG.js!
enum AlbumElement {
	aeUndefined = -1,
	aeWebPage,					// common  - color (nWebTextColor) & background (nWebBackgroundColor) + font
	aeHeader,					// header section -  color (SmallnGalleryTitleColor) + font
	aeMenuButtons,				// menu-items - buttons - color (nMenuButtonsTextColor), background (nMenuButtonsBackgroundColor) -only used when no gradient!
	//         + gradient colors (gradStartColor, gradMiddleColor, gradStopColor) and positions: (gradStartPos,gradMiddlePos,gradStopPos)
	//		   + font
	aeSmallTitle,				// small gallery title
	aeLangButton,				// languages - language buttons
	aeGalleryTitle,				// gallery title * color (nGalleryTitleColor) + font
	aeGalleryDesc,				// 		   decription
	aeSection,					// albums OR images - color (sectionTextColor) + text shadows() + font
	aeThumb,					// thumbnail image
	aeImageDesc,				//			  decription -	color (ImageDescColor) + font
	aeImageTitle,				// thumbnails title -		album title color (albumTitleColor) + font
	aeSummary,					// XXX images and YYY albums
	aeCopyright,				//
	aeLightboxTitle,			// lightbox - title
	aeLightboxDescription,		//			  description
	aeFooter					// footer
};

enum Existance {
	exNot,			// does not exist	
	exExists,		// present on disk
	exVirtual		// not on disk
};	// for albums

enum Changed { fsNothing = 0, fsTitle = 1, fsDescription = 2 }; // which text changed
enum skinStyle { stDefault, stSystem, stBlue, stDark, stBlack };
enum StyleIndex {   // C.f. index.html
	siColor, siBackground, siBackgroundColor, siBoxShadow, siBorder,
	siFontFamily, siFontFeatureSettings, siFontSize, siFontStyle,
	siFontWeight, siTextAlign, siTextDecoration, siTextShadow, siFirstLine
};
// properties that could have been changed (to set on web sample by _ChangesToSample)
enum DesignProperty {
	dpNone,		// nothing */
	dpColor, dpBackground,
	dpPageColor, dpPageBackground,
	dpBoxShadow, dpTextShadow,
	dpBorder,
	dpMenuGradient,
	dpFont,
	dptextAlign, dpImageBorder,
	dpIconToUplink,
	dpWatermark
};
enum whoChangedTheText {		// in the edit boxes
	wctSelection = 1,			// new thumbnail selected must save texts in database and .struct if changed
	wctBaseLangCombo = 2,		// base language combo index changed, no need to save the text
	wctLangCombo = 4			// language combo selection changed, must save text in selection
};
enum FrfFlags { frfNormal, frfAllLines, frfEmptyLines = 2, frfCommentLines = 4, frfLtrim = 8, frfRtrim = 8, frfTrim = 12, frfNoWhiteSpaceLines = 32, frfNeedUtf8 = 64 };
enum IcFlags :int { prImage = 1, prThumb = 2, dontEnlarge = 4, dontResize = 8 };
enum UserRoles {
	FileNameRole = Qt::UserRole + 1,		 // file name of generated image/video w.o. path, e.g. 12345.jpg 
	FilePathRole,							 // path of generated image directory ending in '/' relative to server root
	FullNameRole,							 // path name of generated image/video relative to server root
	SourcePathNameRole,						 // path name of image source either relative to source folder or full path
	SortRole,
	LoadedRole,
	TypeRole								// return type of item (none, image, video, album)
};
enum ElemSubPart {
	espAll,
	espColor,
	espBackground,
	espFont,
	espSpaceAfter,
	espTextDecor,
	espShadow1,
	espShadow2,
	espShadow,	// both shadow
	espGradient,
	espBorder,
	espTextAlign,
	espLinkIcon,
	espBackgroundImage
};

enum IconFlag {
	fiFolder = 1, fiThumb = 2, fiAlias = 4, fiDontResize = 8,	// 1..8: what markers to draw on icon image
	fiVideo = 16, fiImage = 32, fiNone = 0x00
};						// 16..32: what type of icon to draw

// for style sheets to make it more legible
constexpr const	bool forWebUse = true;
constexpr const bool forInternalUse = false;
constexpr const bool forBackground = true;
constexpr const bool notForBackground = false;
constexpr const bool addSemicolon = true;
constexpr const bool noSemicolon = false;
constexpr const bool onlyStyle = true;
constexpr const bool itemNameToo = true;
constexpr const bool noItemName = false;
constexpr const bool firstOne = true;
constexpr const bool notFirstOne = false;

constexpr const bool configChanged = true;
constexpr const bool configNotChanged = false;

using IconFlags = QFlags<IconFlag>;
// indices of clickable items on sample page
constexpr const int nGlobal = 0;
// menu
constexpr const int nBtnDnUp = 1;
constexpr const int nBtnDnAbout = 2;
constexpr const int nBtnDnContact = 3;
constexpr const int nBtnDnCaptDesc = 4;
constexpr const int nBtnDnDescription = 5;
constexpr const int nBtnDnToAlbums = 6;
// other
constexpr const int nLblDnName = 7;	   // small title
constexpr const int nLblDnLang = 8;	   // language

constexpr const int nLblDnGallery = 9;	   // gallery title
constexpr const int nLblDnGalleryDescription = 10;
constexpr const int nLblDnPictureSectionLabel = 11;	   // 'Pictures'
constexpr const int nLblDnImageP = 12;	   // picture thumbnail image
constexpr const int nLblDnDescriptionP = 13;	   // image description
constexpr const int nLblDnTitleP = 14;	   // picture title
constexpr const int nLblDnTitlePO = 15;	   // picture title 2nd line
constexpr const int nLblDnAlbumSectionLabel = 16;	   // 'Albums'
constexpr const int nLblDnImageA = 17;	   // album thumbnail image
constexpr const int nLblDnDescriptionA = 18;    // album description
constexpr const int nLblDnTitleA = 19;	   // album title
constexpr const int nLblDnTitleAO = 20;	   // album title 2nd line
constexpr const int nLblDnFooter = 21;	   // 'this gallery contains...'+copyright message
constexpr const int nLblDnCopyright = 22;	   // sam as footer for now
constexpr const int nLblDnWatermark = 23;	   // sam as footer for now
constexpr const int nLblDnHeader = 24;	   // doesn't exist as button?
constexpr const int nLblLightbox = 25;	   // doesn't exist as button?
constexpr const int nLblLightboxDescription = 26;	   // doesn't exist as button?

constexpr const int SAMPLEINDEXSIZE = 27;	   // always one larger than the last one above

// design page combo box indices (change when UI 'falconG.ui' changes!)

constexpr const int nCbIxGlobal = 0;	   // global page
constexpr const int nCbIxHeader = 1;	   // ???
constexpr const int nCbIxMenu = 2;
constexpr const int nCbIxSmallTitle = 3;
constexpr const int nCbIxLanguage = 4;
constexpr const int nCbIxGalleryTitle = 5;
constexpr const int nCbIxGalleryDescr = 6;
constexpr const int nCbIxSection = 7;
constexpr const int nCbIxImageThumbnail = 8;
constexpr const int nCbIxAlbumThumbnail = 8;	// same as for the image
constexpr const int nCbIxIADescription = 9;	// same for image and album
constexpr const int nCbIxIATitle = 10;	// first line -"-
constexpr const int nCbIxIATitleO = 10;	// other lines: adjusted together
constexpr const int nCbIxWatermark = 11;
constexpr const int nCbIxFooter = 12;	// both item number and copyright
constexpr const int nCbIxLightbox = 13;	// can't visualize yet
constexpr const int nCbIxLightBoxDescription = 14;

constexpr const int UNIQUESIZE = 15;	// must be the count


//							n: index of item 	index in combo box
// 							on design page		 on the edit page
// global							nGlobal						nCbIxGlobal
// Menu	(incl. up icon button)		nBtnDnUp					nCbIxMenu
// 									nBtnDnAbout			
// 									nBtnDnContact		
// 									nBtnDnCaptDesc		
// 									nBtnDnDescription	
// 									nBtnDnToAlbums		
// Small Title						nLblDnName					nCbIxSmallTitle		
// Language							nLblDnLang					nCbIxLanguage		
// Gallery Title					nLblDnGallery			 	nCbIxGalleryTitle	
// Gallery description				nLblDnGalleryDescription 	nCbIxGalleryDescr	
// Pictures Section Title			nLblDnPictureSectionLabel	nCbIxSection		
// Thumbnail image					nLblDnImageP			 	nCbIxImageThumbnail	
// Image description				nLblDnDescriptionP		 	nCbIxIADescription	
// Image title 1st line				nLblDnTitleP		 	 	nCbIxIATitle	
// Image title 2nd line				nLblDnTitlePO			 	nCbIxIATitleO		
// Albums Section Title				nLblDnAlbumSectionLabel	 	nCbIxSection
// Album image						nLblDnImageA			 	nCbIxAlbumThumbnail
// Album description				nLblDnDescriptionA		 	nCbIxIADescription
// Album title 1st line				nLblDnTitleA		 	 	nCbIxIATitle
// Album title 2nd line				nLblDnTitleAO	 		 	nCbIxIATitleO
// Footer summary					nLblDnFooter			 	nCbIxFooter
// Footer copyright					nLblDnCopyright	 		 	nCbIxFooter
// Watermark text					nLblDnWatermark	 		 	nCbIxWatermark
// header							nLblDnHeader			 	nCbIxheader
// lightbox title					nLblLightbox			 	nCbLightBox
// Lightbox Description				nLblLightboxDescription	 	nCbLightBoxDescription

// sample - combobox index mapping	
// MODIFY if count of nLblXXX or nCbYY changes!
class SCMap
{
	QMap<int, int> _sampleToComboboxIndex;
	QMap<int, int> _comboboxToSampleIndex;
	void _InitIndices();
public:
	SCMap() { _InitIndices(); }
	int SampleToCBIx(int sampleIndex);
	int CBIxToSample(int nCbIndex, int* pix);
	int UniqueSize() const { return _comboboxToSampleIndex.size(); }
	int SampleSize() const { return _sampleToComboboxIndex.size(); }
};

extern SCMap scMap; // sample index <-> combo box index indices

//--------------------------------------------------
constexpr const char* FG_WARNING = "falconG - Warning";
constexpr const char* FG_ERROR = "falconG - Error";
constexpr const char* FG_QUESTION = "falconG - Question";
constexpr const char* FG_IMG_VIEWER = "falconG - Image Viewer";

//--------------------------------------------------
// data for video
struct VideoData
{
	QSize frameSize = QSize(-1, -1);			// determined when thumbnail image is extracted
	qreal frameRate = 0.0;
	qint64 thumbPosition = 0;	                // position in video file where the thumbnail is taken from
};

using IDVal_t = uint64_t;	// ID_t is a composite of flags, IDVal_t and dir. index
using IDPath_t = uint64_t;	// IDPath_t is the type for path id's

// not ENUM, but common
const bool SIGNAL_ELAPSED_TIME = true, ADD_TO_ALBUM_ITEMS = true, IS_THUMBNAIL = true;
const bool CHECK = true;
const bool DONTCHECK = false;
const IDVal_t NO_ID = 0x00;	// for the album tree root as parent of the root and recent album
const IDVal_t TOPMOST_PATH_ID = 0x01;

const IDVal_t BASE_ID_MASK = 0x00000000FFFFFFFFull;	// values & BASE_ID_MASK = CRC
const IDVal_t ID_INCREMENT = 0x0000000100000000ull;	// when id's clash add (possible a multiple) of this value to id 
const int TEXT_ID_COLLISION_FACTOR = 32;		// id >> TEXT_ID_COLLISION_FACTOR = overflow index

const IDVal_t MAX_ID = 0x00FFFFFFFFFFFFFFull;
const IDVal_t ID_MASK = 0x00FFFFFFFFFFFFFFull;	// so that we can combine _flags and ID and still get the ID back

// flags for ID's
typedef uint8_t IDFlags;	// 8 bits for flags

const IDFlags INVALID_ID_FLAG = 0x00;
// type flags
constexpr IDFlags IMAGE_ID_FLAG = 0x01;	// when set ID is for a image
constexpr IDFlags VIDEO_ID_FLAG = 0x02;	// when set ID is for a video
constexpr IDFlags ALBUM_ID_FLAG = 0x04;	// when set ID is for an album (used for albums as folder thumbnails)
constexpr IDFlags ALBUM_LINK_FLAG = 0x08;	// when set ID is for an album already in the database
constexpr IDFlags TYPE_FLAGS = 0x0F;
// removed: thumbnailCount is used instead   const IDFlags THUMBNAIL_FLAG= 0x10;	// for images: this image is an album thumbnail, if other bits are unset: not in any album
			// other flags
constexpr IDFlags EXCLUDED_FLAG = 0x10;
constexpr IDFlags ORPHAN_FLAG = 0x20;	// for images: this image is an album thumbnail, if other bits are unset: not in any album
constexpr IDFlags EXISTING_FLAG = 0x40;	// on albums this signals a real folder on disk Otherwise it is a logical album
constexpr IDFlags DELETE_IT_FLAG = 0x80;	// on albums this signals a real folder on disk Otherwise it is a logical album


// other
constexpr uint NOT_SET = uint(-1);

/*=============================================================
 * Class to use as image/video/album ID
 *  '_dirIndex', 'config.bSeparateFoldersForLanguage'
 *	'config.bUseMaxItemsPerDir' and 'config.nMaxItemsPerDir'
 *	are used to determine which directory this item is to
 *	be stored
 *
 * '_dirIndex' may be non 0 or NOT_SET only when
 *  'config.bUseMaxItemsPerDir' is true
 *
 * When config.bSeparateFoldersForLanguages is true then
 * folders inside the ones shown below are used for items
 * Example: language code string is en_US
 *			'config.bSeparateFoldersForLanguage' is true
 *			and _dirIndex is 0 then English albums named
 *			albumXXXXX_en.html are stored
 *			in the 'albums/en_US/' directory, and when _dirIndex is 1
 *			in the 'albums-1/en_US/' directory.
 *
 * Using the default folder names below:
 * 	   when '_dirIndex' is NOT_SET or 0 use the base folders:
 *				albums are stored in: 'albums'
 *				images are stored in: 'imgs'
 *				thumbs are stored in: 'thumbs'
 *				videos are stored in: 'vids'
 *	  when '_dirIndex' is any other value:
 *				albums are stored in: albums-<dirIndex>
 *				images are stored in: imgs-<dirIndex>
 *				thumbs are stored in: thumbs-<dirIndex>
 *				videos are stored in: vids-<dirIndex>
 *		in this case these folders are created only when the number of items
 *		would exceed 'config.nMaxItemsPerDir'.
 *
 *	'_dirIndex' is also recorded in the .struct file after the ID and a
 *		trailing letter 'i'. Example: 123456i7.
 *
 * On album generation if the state of 'config.bUseMaxItemsPerDir' differs
 *	 from the value  in the '.ini' file, all existing 'XXX-<dirIndex> named folders
 * and files inside them are deleted from disk, and '_dirIndex' is recalculated
 * using the new value, as follows:	If the new value of 'config.bUseMaxItemsPerDir'
 *
 *	- is false then all '_dirIndex' values are set to 0
 *	- is true then all files inside the base folders
 *			are also deleted, the required number of non-base folders are created
 *			The number of folders created is determined by the count of items in maps
 *			albumMap, imageMap, videoMap and the value of 'config.nMaxItemsPerDir'
 *		'_dirIndex' values for each items in each maps are calculated and set
 *		In this case the state of chekckboxes 'chkGenerateAllPages',
 *		'chkRegenAllImages' and 'chkNoImages' are ignored and values 'false,fals,false'
 *		are used instead
 *
 *		When new items are added new directories may be created.
 *------------------------------------------------------------*/

class ID_t
{
	IDVal_t _uval = 0;			// bits 56-63 is not used so _flags may be put there, 
	IDFlags _flags = 0;			// types and other flags 
	uint _dirIndex = NOT_SET;	// index of the directory this item will be stored is inside
public:
	constexpr ID_t() {}
	constexpr ID_t(const ID_t& o) : _uval(o._uval), _flags(o._flags), _dirIndex(o._dirIndex) {}
	constexpr ID_t(uint8_t _flags, IDVal_t id) : _uval(id), _flags(_flags) {}
	constexpr ID_t(uint8_t _flags, IDVal_t id, uint di) : _uval(id), _flags(_flags), _dirIndex(di) {}
	ID_t(QString idString, uint8_t flags, uint _dirIndex = NOT_SET);

	constexpr IDVal_t Val()		 const { return _uval; }
	//constexpr operator IDVal_t() const { return _uval; }
	QString ValToString() const;
	constexpr uint DirIndex() const { return _dirIndex; }
	constexpr uint8_t Flags() const { return _flags; }
	static constexpr const ID_t Invalid(IDVal_t defarg = NO_ID) { return ID_t(INVALID_ID_FLAG, defarg); }
	constexpr inline bool IsInvalid() const { return !_uval || !_flags; }

	constexpr void SetValue(IDVal_t val)
	{
		_uval = val;
	}
	constexpr void SetDirIndex(uint dirIndex)	// do not use this directly but through config.SetDirIndexFor()
	{
		_dirIndex = dirIndex;
	}
	constexpr uint8_t SetFlag(uint8_t which, bool setIt = true)	// which can be contain _flags ORed
	{														// returns resulting _flags
		_flags &= ~which;
		if (setIt)
			_flags |= which;
		return _flags;
	}
	constexpr bool TestFlag(uint8_t withTheseFlags) const
	{
		return _flags & withTheseFlags;
	}
	constexpr ID_t ClearNonTypeFlags(bool andModifyFlagsAsWell = false)
	{
		ID_t id = *this;
		id._flags = _flags & TYPE_FLAGS;
		if (andModifyFlagsAsWell)
			_flags = id._flags;
		return id;
	}

	constexpr inline ID_t& Increment(IDVal_t inc) { _uval += inc; return *this; }

	constexpr inline ID_t& operator=(const ID_t& v) { _uval = v._uval; _flags = v._flags; _dirIndex = v._dirIndex; return *this; }
	//constexpr inline ID_t& operator=(const ID_t v) { _uval = v; return *this; }

	//constexpr inline bool operator==(const ID_t v) const { return _uval == v; }
	// in the following _dirIndex doesn't count
	constexpr inline bool operator==(const ID_t v) const { return _uval == v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	constexpr inline bool operator!=(const ID_t v) const { return _uval != v._uval || (_flags & TYPE_FLAGS) != (_flags & TYPE_FLAGS); }
	constexpr inline bool operator<(const ID_t v) const { return _uval < v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	constexpr inline bool operator<=(const ID_t v) const { return _uval <= v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	constexpr inline bool operator>(const ID_t v) const { return _uval > v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }

	constexpr inline bool IsSameType(uint8_t type) const { return (Flags() & TYPE_FLAGS & type) != 0; }

	constexpr inline bool DoesExist() const { return _flags & EXISTING_FLAG; }
	constexpr inline bool IsAlbum() const { return _flags & ALBUM_ID_FLAG; }
	constexpr inline bool IsAlbumLink() const { return _flags & ALBUM_LINK_FLAG; }
	constexpr inline bool IsExcluded() const { return _flags & EXCLUDED_FLAG; }
	constexpr inline bool IsImage() const { return _flags & IMAGE_ID_FLAG; }
	constexpr inline bool IsVideo() const { return _flags & VIDEO_ID_FLAG; }
	constexpr inline bool IsDirIndexSet() const { return _dirIndex != NOT_SET; }
	constexpr inline bool ShouldDelete() const { return _flags & DELETE_IT_FLAG; }
};

const IDVal_t INVALID_ALBUM_ID = 0;
const IDVal_t TOPMOST_ALBUM_ID = 0x01;
const IDVal_t RECENT_ALBUM_ID = 0x02;
const IDVal_t NOIMAGE_ID = 0;
//const ID_t INVALID_ALBUM_ID = { ALBUM_ID_FLAG, 0 };
//const ID_t TOPMOST_ALBUM_ID = { ALBUM_ID_FLAG, 0x01};
//const ID_t RECENT_ALBUM_ID	= { ALBUM_ID_FLAG, 0x02 };
//const ID_t NOIMAGE_ID		= { IMAGE_ID_FLAG, 0 };


using IntList = QVector<int>;
using IDValList = QVector<IDVal_t>;
using IdList = QVector<ID_t>;

const ID_t NO_ALBUM(ALBUM_ID_FLAG, NO_ID);	// used when no album is selected

#endif // COMMON_H
