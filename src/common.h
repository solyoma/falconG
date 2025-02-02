#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QString>
#include <QMap>

namespace Common
{
	enum DialogBitsOrder : int {
		dboNone,
		dboAskBeforeClosing,
		dboShowAfterSavingCss,
		dboSaveEdited,
		dboSaveConfig,
		dboAskCreateDir,
		dboAskSynchronize,
		dboNeverMoveIntoFolder,		// when drag & drop
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
	enum BorderSide { sdAll=-1, sdTop, sdRight, sdBottom, sdLeft };  // index	-1, 0		1	   2		 3
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
		aeLangButton,				// languages - language buttons
		aeSmallTitle,				// small gallery title
		aeGalleryTitle,				// gallery title * color (nGalleryTitleColor) + font
		aeGalleryDesc,				// 		   decription
		aeSection,					// albums OR images - color (sectionTextColor) + text shadows() + font
		aeThumb,					// thumbnail image
		aeImageTitle,				// thumbnails title -		album title color (albumTitleColor) + font
		aeImageDesc,				//			  decription -	color (ImageDescColor) + font
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
}

// not ENUM, but common
const bool CHECK = true;
const bool DONTCHECK = false;
const uint64_t NO_ID = 0x00;	// for the album tree root as parent of the root and recent album
const uint64_t TOPMOST_PATH_ID	 = 0x01;

const uint64_t BASE_ID_MASK = 0x00000000FFFFFFFFull;	// values & BASE_ID_MASK = CRC
const uint64_t ID_INCREMENT = 0x0000000100000000ull;	// when id's clash add (possible a multiple) of this value to id 
const int TEXT_ID_COLLISION_FACTOR = 32;		// id >> TEXT_ID_COLLISION_FACTOR = overflow index

const uint64_t MAX_ID = 0x00FFFFFFFFFFFFFFull;
const uint64_t ID_MASK = 0x00FFFFFFFFFFFFFFull;	// so that we can combine _flags and ID and still get the ID back

// _flags for ID's
const uint8_t INVALID_ID_FLAG = 0x00;

const uint8_t IMAGE_ID_FLAG = 0x01;	// when set ID is for a image
const uint8_t VIDEO_ID_FLAG = 0x02;	// when set ID is for a video
const uint8_t ALBUM_ID_FLAG = 0x04;	// when set ID is for an album (used for albums as folder thumbnails)
// removed: thumbnailCount is used instead   const uint8_t THUMBNAIL_FLAG= 0x10;	// for images: this image is an album thumbnail, if other bits are unset: not in any album
const uint8_t EXCLUDED_FLAG = 0x10;
const uint8_t ORPHAN_FLAG	= 0x20;	// for images: this image is an album thumbnail, if other bits are unset: not in any album
const uint8_t EXISTING_FLAG = 0x40;	// on albums this signals a real folder on disk Otherwise it is a logical album
const uint8_t DELETE_IT_FLAG = 0x80;	// on albums this signals a real folder on disk Otherwise it is a logical album

const uint8_t TYPE_FLAGS	= 0x0F;

const uint NOT_SET = uint(-1);

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
	uint64_t _uval = 0;			// bits 56-63 is not used so _flags may be put there, 
	uint _dirIndex = NOT_SET;	// index of the directory this item is inside
	uint8_t _flags=0;			// types and other _flags 
public:
	constexpr ID_t() {}
	constexpr ID_t(const ID_t& o) : _uval(o._uval), _flags(o._flags) {}
	constexpr ID_t(uint8_t _flags, uint64_t id) : _uval(id),_flags(_flags) {}
	ID_t(QString idString, uint8_t flags) : _flags(flags)
	{
		if (idString.isEmpty())
			_uval = 0;
		else if (idString.lastIndexOf('i') > 0)
		{
			_uval = idString.left(idString.lastIndexOf('i')).toULongLong();
			_dirIndex = idString.mid(idString.lastIndexOf('i') + 1).toUInt();
		}
		else
			_uval = idString.toULongLong();
	}

	constexpr uint64_t Val() const { return _uval; }
	QString ValToString() const 
	{ 
		if (_dirIndex) 
			return QString("%1i%2").arg(_uval).arg(_dirIndex); 
		else 
			return QString().setNum(_uval); 
	}
	constexpr uint DirIndex() const { return _dirIndex; }
	constexpr uint8_t Flags() const { return _flags; }
	inline static const ID_t Invalid(uint64_t defarg = NO_ID) { return ID_t(INVALID_ID_FLAG, defarg); }
	constexpr inline bool IsInvalid() const { return !_uval || !_flags; }

	constexpr void SetValue(uint64_t val)
	{
		_uval = val;
	}
	constexpr void SetDirIndex(uint dirIndex)	// do not use this directly but through config.SetDirIndexFor()
	{
		_dirIndex = dirIndex;
	}
	constexpr uint8_t SetFlag(uint8_t which, bool setIt=true)	// which can be contain _flags ORed
	{														// returns resulting _flags
		_flags &= ~which;
		if (setIt)
			_flags |= which;
		return _flags;
	}
	constexpr bool TestFlag(uint8_t withTheseFlag) const
	{
		return _flags & withTheseFlag;
	}
	constexpr ID_t ClearNonTypeFlags(bool andModifyFlagsAsWell = false)
	{
		ID_t id = *this;
		id._flags =_flags & TYPE_FLAGS;
		if (andModifyFlagsAsWell)
			_flags = id._flags;
		return id;
	}

	constexpr inline ID_t& Increment(uint64_t inc) { _uval += inc; return *this; }

	constexpr inline ID_t& operator=(const ID_t& v) { _uval = v._uval; _flags = v._flags; _dirIndex = v._dirIndex; return *this; }
	//constexpr inline ID_t& operator=(const uint64_t v) { _uval = v; return *this; }

	//constexpr inline bool operator==(const uint64_t v) const { return _uval == v; }
	// in the following _dirIndex doesn't count
	inline bool operator==(const ID_t v) const { return _uval == v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	inline bool operator!=(const ID_t v) const { return _uval != v._uval || (_flags & TYPE_FLAGS) != (_flags & TYPE_FLAGS); }
	inline bool operator<(const ID_t v) const { return _uval < v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	inline bool operator<=(const ID_t v) const { return _uval <= v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	inline bool operator>(const ID_t v) const { return _uval > v._uval && (_flags & TYPE_FLAGS) == (v._flags & TYPE_FLAGS); }
	
	constexpr inline bool IsSameType(uint8_t type) const { return (Flags() & TYPE_FLAGS & type) != 0; }

	constexpr inline bool DoesExist() const		{ return _flags & EXISTING_FLAG; }
	constexpr inline bool IsAlbum() const		{ return _flags & ALBUM_ID_FLAG; }
	constexpr inline bool IsExcluded() const	{ return _flags & EXCLUDED_FLAG; }
	constexpr inline bool IsImage() const		{ return _flags & IMAGE_ID_FLAG; }
	constexpr inline bool IsVideo() const		{ return _flags & VIDEO_ID_FLAG; }
	constexpr inline bool IsDirIndexSet() const	{ return _dirIndex != NOT_SET;   }
	constexpr inline bool ShouldDelete() const	{ return _flags & DELETE_IT_FLAG; }
};
const ID_t INVALID_ALBUM_ID = { ALBUM_ID_FLAG, 0 };
const ID_t TOPMOST_ALBUM_ID = { ALBUM_ID_FLAG, 0x01};
const ID_t RECENT_ALBUM_ID	= { ALBUM_ID_FLAG, 0x02 };
const ID_t NOIMAGE_ID		= { IMAGE_ID_FLAG, 0 };


using IntList = QVector<int>;
using IdList = QVector<ID_t>;

#endif // COMMON_H