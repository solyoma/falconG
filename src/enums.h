#pragma once

namespace Enums
{
	enum DialogBits : int {			// sync with DialogBitsOrder below!
			dbAskBeforeClosing=1, 	
			dbShowAfterSavingCss = 2, 
			dbSaveEdited=4, 
			dbSaveConfig=8,  
			dbAskCreateDir=16
	};
	enum DialogBitsOrder : int {
		dboAskBeforeClosing,
		dboShowAfterSavingCss,
		dboSaveEdited,
		dboSaveConfig,
		dboAskCreateDir
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
using IntList = QVector<int>;
