#pragma once

#include <QtCore>
#include <QtWidgets/QMainWindow>
#include <QTextStream>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

#include "enums.h"
using namespace Enums;

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"
#include "thumbnailWidget.h"
#include "ui_falcong.h"

#include <memory>

/*------------------------------------- macros ------------------------------------*/
#define AS_HEX_COLOR_STRING(n) QString("%1").arg(n, 6, 16, QChar('0'))
#define TO_COLOR_STRING(s,n) s = "#" + AS_HEX_COLOR_STRING(n)

QString ColorToCss(_CColor color);
QString IntToColorStr(int clr);

typedef UsageCount Semaphore;

/*------------------------------------------------
* selection for tree views of albums
*-----------------------------------------------*/
struct FalconGEditTabSelection
{										// what can be changed:
									    // these fields are used when editing image/album texts
	ID_t actAlbum = 0,				// actual selected album
		 newAlbum = 0;				// replace data from actAlbum with data from newAlbum
	ID_t selectedImage = 0,				// actual selected image
		 newImage = 0;					// new image
	QFlags<Changed> changed;			// title or description in either language has changed
	int baseLang = 0, edtLang = 0;		// index of actual languages before the text change
	LanguageTexts title, description;			// title or description texts for any languages
};

//--------------------------------------------------------
class WebEnginePage : public QWebEnginePage
{
	Q_OBJECT

public:
	WebEnginePage(QWidget* parent = nullptr) : QWebEnginePage(parent) { }

	bool acceptNavigationRequest(const QUrl& url, NavigationType navType, bool isMainFrame) override
	{
		QString qs = url.scheme(),
			qsq = url.fileName();
		if (navType != QWebEnginePage::NavigationTypeRedirect)
			return true;

		emit LinkClickedSignal(url.fileName());

		return false;
	}
signals:
	void LinkClickedSignal(QString);
};

struct FalconGScheme
{
	QString
		MenuTitle,			// this will appear in the menu bar
		sBackground,
		sTextColor,
		sBorderColor,
		sFocusedInput,
		sHoverColor,
		sTabBorder,
		sInputBackground,
		sSelectedInputBgr,
		sFocusedBorder,
		sDisabledFg,
		sDisabledBg,
		sImageBackground,
		sPressedBg,
		sDefaultBg,
		sProgressBarChunk,
		sWarningColor,
		sBoldTitleColor;
	QString& operator[](int index) 
	{ 
		switch (index)
		{
			case 0:		  return sBackground;
			case 1:		  return sTextColor;
			case 2:		  return sBorderColor;
			case 3:		  return sFocusedInput;
			case 4:		  return sHoverColor;
			case 5:		  return sTabBorder;
			case 6:		  return sInputBackground;
			case 7:		  return sSelectedInputBgr;
			case 8:		  return sFocusedBorder;
			case 9:		  return sDisabledFg;
			case 10:	  return sDisabledBg;
			case 11:	  return sImageBackground;
			case 12:	  return sPressedBg;
			case 13:	  return sDefaultBg;
			case 14:	  return sProgressBarChunk;
			case 15:	  return sWarningColor;
			default: 
			case 16:	  return sBoldTitleColor;
		}
	}
	FalconGScheme() 
	{
	}
	FalconGScheme(const char* t,	 // menu title
				  const char* c0,	 // sBacground
				  const char* c1,	 // sTextColor	-	foreground
				  const char* c2,	 // sBorderColor
				  const char* c3,	 // sFocusedInput
				  const char* c4,	 // sHoverColor
				  const char* c5,	 // sTabBorder
				  const char* c6,	 // sInputBackground - editor backgrounds
				  const char* c7,	 // sSelectedInputBgr
				  const char* c8,	 // sFocusedBorder
				  const char* c9,	 // sDisabledFg
				  const char* c10,	 // sDisabledBg
				  const char* c11,	 // sImageBackground
				  const char* c12,	 // sPressedBg	-	button pressed
				  const char* c13,	 // sDefaultBg
				  const char* c14,	 // sProgressBarChunk
				  const char* c15,	 // sWarningColor
				  const char* c16	 // sBoldTitleColor - GroupBox title
									)
	{
		MenuTitle			= t;			// this will appear in the menu bar
		sBackground			= c0;
		sTextColor			= c1;
		sBorderColor		= c2;
		sFocusedInput		= c3;
		sHoverColor			= c4;
		sTabBorder			= c5;
		sInputBackground	= c6;
		sSelectedInputBgr	= c7;
		sFocusedBorder		= c8;
		sDisabledFg			= c9;
		sDisabledBg			= c10;
		sImageBackground	= c11;
		sPressedBg			= c12;
		sDefaultBg			= c13;
		sProgressBarChunk	= c14;
		sWarningColor		= c15;
		sBoldTitleColor		= c16;
	}

};								


/*========================================================
 * Style sheets (skins) for falconG
 * REMARKS: - default styles (default, system, blue, dark, black)
 *				are always present
 *			- styles are read from falconG.sty in program 
 *				directory
 *			- if a style there has the same Title as any 
 *				of the last 3 of the default they will be
 *				overwritten
 *			- styles default and system are always the first
 *				2 styles used, and if not redefined then
 *				blue, dark and black are the last ones
 *			- if a style requested which is no longer 
 *				present, the 'default' style is used instead
 *-------------------------------------------------------*/
class FSchemeVector : public  QVector<FalconGScheme>
{
	static FalconGScheme blue, dark, black;
public:
	FSchemeVector() 
	{
		reserve(5);		// for default, system, blue, dark, black 
		resize(2);		// default and system
		operator[](0).MenuTitle = "Default";
		operator[](1).MenuTitle = "System Colors";
		operator[](1).sBorderColor = "#747474";
	}
	void ReadAndSetupStyles();	// into menu items
	void Save();			// into "falconG.fsty"
	int IndexOf(FalconGScheme& fgst);
	int IndexOf(const QString &title);
};


//---------------
class FalconG : public QMainWindow
{
	Q_OBJECT

public:
	FalconG(QWidget *parent = Q_NULLPTR);
	~FalconG();

	void closeEvent(QCloseEvent *event);

	int GetProgressBarPos();
	int IsRunning() { return _running; }
	void Stop() { --_running; }

signals:
	void CancelRun();
private:

	Ui::falconGClass ui;

	WebEnginePage _page;
	// style (skin) selection
	FSchemeVector _schemes;		// default styles: default, system, blue, dark, black
	bool _bSchemeChanged = false,	// any of the colors changed
		 _bNewSchemeName = false;  // a new name of the scheme is added
	FalconGScheme _tmpScheme;	// used for editing
	QString _tmpSchemeOrigName;	// if not the same what was before it has been changed
	QList<QPushButton*> _pSchemeButtons;		// buttons added to options page
	QSignalMapper* _pSchemeMapper = nullptr;	// each button uses this
	QLineEdit* _pCbColorSchemeEdit;
	//std::unique_ptr<QSignalMapper> _popupMapper;
	// ---
	DesignProperty _whatChanged = dpNone;

	Semaphore	_busy,		// prevent recursive parameter changes
				_running;	// operation (e.g. web page generation) is running: do not close the program yet
	bool _isWebPageLoaded = false;
	int _phase = 0;			// processing phase. set by 'SetProgressBar()'
	AlbumElement _aeActiveElement = aeWebPage;
	int _directoryCount = 0;
	double _aspect=1.0;		// image_width/image_height
			   // these fields are used when editing image/album texts
	FalconGEditTabSelection _selection;	// selection in tree view
	bool _edited = false;		// when image/album text etc modified in program
								// asks for save at exit
	QColor _lastUsedMenuForegroundColor = Qt::white;

	QTextStream ifs, ofs;

	QString _GradientStyleQt(_CElem &elem, bool invert = false);	// Creates QT stylesheet gradinet string
	void  _SetGradientLabelColors(_CElem *pElem, bool invert = false);	// Creates QT stylesheet gradinet string
	_CElem* _PtrToElement(AlbumElement ae = aeUndefined);


	void _SetIconColor(QIcon &icon, _CElem &elem);
	QIcon _SetUplinkIcon(QString iconName = QString());

	void _EnableGradient(bool ena);
	void _EnableButtons();
	void _GlobalsToUi();
	void _ActualSampleParamsToUi();		// using _aeActiveElement
	void _DesignToUi();					// part of config used on design page
	void _OtherToUi();					// all others
	void _ConfigToUI(); 
	bool _CreateAlbumDirs();
	void _SetOpacityToConfig(_CElem & elem, int which = 3); // 1: text, 2: background, 3 both

			 // set CSS for sample 
	void _RunJavaScript(QString classMName, QString propertyStrings);	// propertyStrings may contain more than one separated by ';'
	void _SetCssProperty(_CElem *pElem, QString value,  QString subSelector = QString());
	void _ColorToSample(_CElem *pElem);
	void _BackgroundToSample(_CElem* pElem);		// css 'background' color + url no other properties
	void _ShadowToSample(_CElem* pElem,int what);	// what: 0 block-shadow, 1:text-shadow
	void _BorderToSample(_CElem* pElem);			// width, style, color or none
	void _SetLinearGradient(_CElem* pElem);
	void _FontToSample(_CElem* pElem);			// all parameters for font
	void _SpaceAfterToSample(_CElem* pElem);
	void _DecorationToSample(_CElem* pElem);
	void _TextAlignToSample(_CElem *pElem);
	void _PageColorToSample();
	void _PagebackgroundToSample();	// possibly for all elements
	void _SetIcon();			// re-color it for the page
	void _SetWatermark();
	void _UpdateWatermarkMargins(int mx, int my);

	void _SaveLinkIcon();
	void _ElemToSample(AlbumElement ae = aeUndefined);	// with no argument uses _aeActElement

	void _ConfigToSample();	// for all elements

	void _SetConfigChanged(bool on);
	
	void _OpacityChanged(int val, int which);	// which = 0 -> color, 1: background
	void _SaveChangedTexts();  // when texts are edited and changed

	void _SetLayoutMargins(int which);
	void _StyleTheProgram(int which);

	void _ModifyGoogleFontImport();		// in CSS and re-load WEB page
	void _SettingUpFontsCombo();		// cbFonts set up from fonts in config.sGoogleFonts and config.sDefFonts
	void _GetTextsForEditing(whoChangedTheText who); // using '_selection'
	void _LoadBckImage(QString name);	// from config

	void _AddSchemeButtons();
	void _ResetScheme();			 //	use _tmpScheme
	void _EnableColorSchemeButtons();

private slots:
	void LinkClicked(QString s);		// from sample page
	void WebPageLoaded(bool ready);

	void _AlbumStructureSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
	void _SetProgressBar(int min, int max, int pos, int phase);
	void _SetProgressBarPos(int cnt1, int cnt2);
	void _SetupLanguagesToUI();
	void _EnableEditTab(bool on);

	void _AlbumMapChanged();
	void _ShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void _CreateUplinkIcon(QString destPath, QString destName);
	void _SetDirectoryCountTo(int cnt) { _directoryCount = cnt; }
	void _ThumbNailViewerIsLoading(bool yes);
	void _TrvCountChanged();
	void _TnvStatusChanged(QString &); // thumbnail view status changed
	void _TnvSelectionChanged(QString );		// name of selected item or "" for no ormultiple items
	void _TnvNewThumbs();		// previous seingle selection is the new thumbnail for actual album
	void _SaveChangedTitleDescription();

	void _ShadowForElementToUI(_CElem* pElem, int which);	// which 0: text-shadow, 1: box-shadow

	void _TextDecorationToConfig(Decoration dec, bool on);
	void _TextAlignToConfig(Align align, bool on);
	void _SlotForContextMenu(const QPoint& pt);
	void _SlotForStyleChange(int which);

	void _SlotForSchemeButtonClick(int which);
	void _SlotForSchemeComboEditingFinished();
	void _AskForApply();	// when color scheme changed and not yet applied

// auto connected slots
private slots:
	void on_toolBox_currentChanged(int newIndex);
	void on_btnDeleteColorScheme_clicked();
	void on_btnSourceHistory_clicked();
	void on_btnGenerate_clicked();
	void on_btnSaveStyleSheet_clicked();
	void on_btnSelectWmFont_clicked();
	void on_btnWmColor_clicked();
	void on_btnWmShadowColor_clicked();

	void on_btnPageColor_clicked();
	void on_btnPageBackground_clicked();
	void on_btnOpenBckImage_clicked();

	void on_btnImageBorderColor_clicked();
	void on_btnSelectUplinkIcon_clicked();

	void on_btnReload_clicked();
	void on_btnSaveConfig_clicked();

	void on_btnBrowseSource_clicked();
	void on_btnBrowseDestination_clicked();
	// color buttons
	void on_btnForeground_clicked();
	void on_btnBackground_clicked();
	void on_btnBrowseForBackgroundImage();
	void on_btnGradStartColor_clicked();
	void on_btnGradMiddleColor_clicked();
	void on_btnGradStopColor_clicked();
	void on_btnBorderColor_clicked();
	void on_btnShadowColor_clicked();

	void on_btnResetDesign_clicked();
	void on_btnDisplayHint_clicked();
	void on_btnPreview_clicked();

	void on_btnGoToGoogleFontsPage_clicked();

	void on_btnApplyColorScheme_clicked();
	void on_btnResetColorScheme_clicked();
	void on_btnMoveSchemeUp_clicked();

	void on_cbActualItem_currentIndexChanged(int newIndex);


	// generate color scheme
	void on_btnGenerateScheme_clicked();

	void on_cbBorder_currentIndexChanged(int newIndex);			// all, top, right, bottom, left border
	void on_cbBorderStyle_currentIndexChanged(int newIndex);		// solid, dashed, etc

	void on_cbColorScheme_currentIndexChanged(int newIndex);
	void on_cbColorScheme_currentTextChanged(const QString& newText);
					// image
	void on_cbImageQuality_currentIndexChanged(int newIndex);
					//  watermark
	void on_cbWmVPosition_currentIndexChanged(int index);
	void on_cbWmHPosition_currentIndexChanged(int index);
					// edit text on structure page
	void on_cbBaseLanguage_currentIndexChanged(int index);
	void on_cbLanguage_currentIndexChanged(int index);
	void on_cbFonts_currentIndexChanged(int index);
	void on_cbLineHeight_currentIndexChanged(int index);

	void on_cbPointSize_currentTextChanged(const QString& txt);

	void on_chkFacebook_toggled(bool);
	void on_chkDebugging_toggled(bool);
	void on_chkDifferentFirstLine_toggled(bool b);
	void on_cbPointSizeFirstLine_currentTextChanged(const QString& txt);
	void on_chkGenerateAll_toggled(bool);
	void on_chkButImages_toggled(bool);
	void on_chkAddTitlesToAll_toggled(bool);
	void on_chkAddDescToAll_toggled(bool);
	void on_chkLowerCaseImageExtensions_toggled(bool);
	void on_chkReadJAlbum_toggled(bool);
	void on_chkReadFromGallery_toggled(bool);
	void on_chkRightClickProtected_toggled(bool);
	void on_chkCanDownload_toggled(bool);
	void on_chkForceSSL_toggled(bool);
	void on_chkUseGoogleAnalytics_toggled(bool);
	void on_chkSetLatest_toggled(bool);			// generate 'Latest upload'
	void on_chkTextOpacity_toggled(bool on);
	void on_chkBackgroundOpacity_toggled(bool on);
	void on_chkUseGradient_toggled(bool on);
	void on_chkShadowOn_toggled(bool on);
	void on_chkUseBorder_toggled(bool on);
	void on_chkImageBorder_toggled(bool);		// image border
	void on_chkIconText_toggled(bool);
	void on_chkIconTop_toggled(bool);
	void on_chkBold_toggled(bool);
	void on_chkItalic_toggled(bool);
	void on_chkTdLinethrough_toggled(bool);
	void on_chkTdUnderline_toggled(bool);
	void on_chkTdOverline_toggled(bool);
	void on_chkUseWM_toggled(bool);				// watermark
	void on_chkOvrImages_toggled(bool);		// used for image processing
	void on_chkDoNotEnlarge_toggled(bool);
	void on_chkCropThumbnails_toggled(bool);
	void on_chkDistortThumbnails_toggled(bool);
	void on_chkSourceRelativePerSign_toggled(bool);
	void on_chkMenuToContact_toggled(bool);
	void on_chkMenuToAbout_toggled(bool);
	void on_chkMenuToDescriptions_toggled(bool);
	void on_chkMenuToToggleCaptions_toggled(bool);

	void on_edtFontFamily_textChanged();
	void on_edtAlbumDir_textChanged();
	void on_edtAbout_textChanged();
	void on_edtBckImageName_textChanged();
	void on_edtBaseName_textChanged();
	void on_edtDefaultFonts_textChanged();
	void on_edtDestGallery_textChanged();
	void on_edtFontDir_textChanged();
	void on_edtGalleryRoot_textChanged();
	void on_edtEmailTo_textChanged();
	void on_edtImg_textChanged();
	void on_edtVid_textChanged();
	void on_edtServerAddress_textChanged();
	void on_edtGalleryTitle_textChanged();
	void on_edtGalleryLanguages_textChanged();
	void on_edtGoogleFonts_editingFinished();
	void on_edtSourceGallery_textChanged();
	void on_edtDescription_textChanged();
	void on_edtKeywords_textChanged();
	void on_edtThumb_textChanged();
	void on_edtTrackingCode_textChanged();
	void on_edtUplink_textChanged();
	void on_edtMainPage_textChanged();
	void on_edtWatermark_textChanged();
	void on_edtTitleText_textChanged();
	void on_edtDescriptionText_textChanged();
	void on_edtWmHorizMargin_textChanged();
	void on_edtWmVertMargin_textChanged();

	void on_rbNoBackgroundImage_toggled(bool);
	void on_rbCenterBckImage_toggled(bool);
	void on_rbCoverBckImage_toggled(bool);
	void on_rbTileBckImage_toggled(bool);
	void on_rbTdSolid_toggled(bool);
	void on_rbTdDotted_toggled(bool);
	void on_rbTdDashed_toggled(bool);
	void on_rbTdDouble_toggled(bool);
	void on_rbTdWavy_toggled(bool);
	void on_rbTextAlignNone_toggled(bool);
	void on_rbTextLeft_toggled(bool);
	void on_rbTextCenter_toggled(bool);
	void on_rbTextRight_toggled(bool);
	void on_rbTextShadow_toggled(bool);

	void on_sbImageHeight_valueChanged(int h);			// image height
	void on_sbImageWidth_valueChanged(int w);				// image width
	void on_sbThumbnailHeight_valueChanged(int h);			// image height
	void on_sbThumbnailWidth_valueChanged(int w);				// image width
	void on_sbNewDays_valueChanged(int val);
	void on_sbLatestCount_valueChanged(int val);
	void on_sbShadowHoriz1_valueChanged(int val);
	void on_sbShadowVert1_valueChanged(int val);
	void on_sbShadowBlur1_valueChanged(int val);
	void on_sbShadowSpread1_valueChanged(int val);
	void on_sbShadowHoriz2_valueChanged(int val);
	void on_sbShadowVert2_valueChanged(int val);
	void on_sbShadowBlur2_valueChanged(int val);
	void on_sbShadowSpread2_valueChanged(int val);
	void on_sbGradStartPos_valueChanged(int val);
	void on_sbGradMiddlePos_valueChanged(int val);
	void on_sbGradStopPos_valueChanged(int val);
	void on_sbWmShadowVert_valueChanged(int val);
	void on_sbWmShadowHoriz_valueChanged(int val);
	void on_sbWmShadowBlur_valueChanged(int val);
	void on_sbImageBorderWidth_valueChanged(int val);
	void on_sbImagePadding_valueChanged(int val);
	void on_sbTextOpacity_valueChanged(int val);
	void on_sbBackgroundOpacity_valueChanged(int val);
	void on_sbWmOpacity_valueChanged(int val);
	void on_sbSpaceAfter_valueChanged(int val);

	void on_sbBorderWidth_valueChanged(int val);
	void on_sbBorderRadius_valueChanged(int val);

	void on_hsImageSizeToShow_valueChanged(int val);
};

extern FalconG *frmMain;

