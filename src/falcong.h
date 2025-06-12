#pragma once

#include <QtCore>
#include <QtWidgets/QMainWindow>
#include <QTextStream>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

#include "common.h"
using namespace Common;

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"
#include "thumbnailView.h"
#include "schemes.h"
#include "ui_falcong.h"

#include <memory>


class ImageViewer;	// in imageviewer.h

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
	ID_t actAlbum = ID_t::Invalid();	// album in which the selected image or album is found
	ID_t selectedAlbum = ID_t::Invalid();	// actual selected album
	ID_t selectedImage = ID_t::Invalid();	// actual selected folder, image or video

	QFlags<Changed> changed;			// title or description in either language has changed

	int baseLang = 0, edtLang = 0;		// index of actual languages before the text change
	LanguageTexts title, description;	// title and description texts for each languages
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
// external
class AlbumTreeView;

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

	AlbumTreeView* GetTreeViewPointer() const;

signals:
	void SignalCancelRun();
	void SignalThumbSizeChanged(int newSize);
	void SignalToCloseAllViewers();
	void SignalActAlbumChanged(int row);
	void SignalBreadcrumbButtonPressed(void*);
	//void SignalToClearIconList();
private:

	Ui::falconGClass ui;

	WebEnginePage _page;
	// style (skin) selection
	bool _bSchemeChanged = false;	// any of the colors changed
	FalconGScheme _tmpScheme;	// used for editing
	QString _tmpSchemeOrigName;	// if not the same what was before it has been changed
	QList<QPushButton*> _pSchemeButtons;		// buttons added to options page
	QSignalMapper* _pSchemeMapper = nullptr;	// each button uses this
	//std::unique_ptr<QSignalMapper> _popupMapper;
	// ---
	DesignProperty _whatChanged = dpNone;

	Semaphore	_busy,		// prevent recursive parameter changes
				_running;	// operation (e.g. web page generation) is running: do not close the program yet
	QItemSelection _currentSelection;
	QModelIndex _currentTreeViewIndex;
	bool _bNewTreeViewSelection = true;	// when true _current contains single actual selection

	bool _isWebPageLoaded = false;
	int _phase = 0;			// processing phase. set by 'SetProgressBar()'
	AlbumElement _aeActiveElement = aeWebPage;
	int _directoryCount = 0;
	double _aspect=1.0;		// image_width/image_height
			   // these fields are used when editing image/album texts
	FalconGEditTabSelection _selection;	// selection in tree view
	QColor _lastUsedMenuForegroundColor = Qt::white;

	QTextStream ifs, ofs;

	QString _GradientStyleQt(_CElem &elem, bool invert = false);	// Creates QT stylesheet gradinet string
	void  _SetGradientLabelColors(_CElem *pElem, bool invert = false);	// Creates QT stylesheet gradinet string
	_CElem* _PtrToElement(AlbumElement ae = aeUndefined);

#if defined(_DEBUG)
	void keyPressEvent(QKeyEvent* event) override;
#endif


	void _SetIconColor(QIcon &icon, _CElem &elem) const;
	QIcon _SetUplinkIcon(QString iconName = QString());

	QString _SelectStructFileFromDir(QString& dirName);

	void _ReadLastAlbumStructure();	// use after config read

	void _EnableGradient(bool ena);
	void _EnableButtons();
	void _GlobalsToUi();
	void _ActualSampleParamsToUi();		// using _aeActiveElement
	void _DesignToUi();					// part of config used on design page
	void _OtherToUi();					// all others
	void _ConfigToUI(); 
	bool _CreateAlbumDirs();
//	void _SetOpacityToConfig(_CElem & elem, int which = 3); // 1: text, 2: background, 3 both

			 // set CSS for sample 
	void _RunJavaScript(QString classMName, QString propertyStrings);	// propertyStrings may contain more than one separated by ';'
	void _SetCssProperty(_CElem *pElem, QString value,  QString subSelector = QString());
	void _RemoveCssProperty(_CElem *pElem, QString value, QString subSelector = QString());
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
	void _PageBackgroundToSample();	// possibly for all elements
	void _SetIcon();			// re-color it for the page
	void _WaterMarkToSample();
	void _UpdateWatermarkMargins(int mx, int my);

	void _SaveLinkIcon();
	void _ElemToSample(AlbumElement ae, ElemSubPart esp = espAll);	
	void _ElemToSample(ElemSubPart esp = espAll);					// with no argument uses _aeActElement

	void _ConfigToSample(ElemSubPart esp=espAll);	// for all elements

	void _SetConfigChanged(bool on);
	
	void _OpacityChanged(int val, int which, bool used);	// which = 0 -> color, 1: background

	void _SetLayoutMargins(int which);
	void _SetProgramScheme();

	void _ModifyGoogleFontImport();		// in CSS and re-load WEB page
	void _SettingUpFontsCombo();		// cbFonts set up from fonts in config.sGoogleFonts and config.sDefFonts

	void _GetBaseTexts(int index);				// from _selection into base text widgets
	void _GetLangTexts(int index);				// from _selection into actual text widgets
	void _SaveEditedTextsIntoSelection();		// from edtTitle and edtSiteDescription
	void _StoreLanguageTexts(LanguageTexts& texts);
	void _SaveChangedTexts();			// when texts in '_selection' are edited and changed
	void _GetTextsForEditing();			// using '_selection :FalconGEditTabSelection'
	void _LoadBckImage(QString name);	// from config

	void _AddSchemeButtons();
	void _ResetScheme();			 //	use _tmpScheme
	void _EnableColorSchemeButtons();

private slots:
	void _SlotAlbumChanged();	// e.g. image or album added to it, image name/path changed
	void _SlotAlbumStructChanged(bool yesitdid);
	void _SlotAlbumStructSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
	void _SlotAlbumStructWillChange();
	void _SlotAskForApply();	// when color scheme changed and not yet applied
	void _SlotBackgroundImageToSamplePage(BackgroundImageSizing sizing);
	void _SlotFolderChanged(int row);	// inside actual folder in ui.trvAlbums
	void _SlotCreateUplinkIcon(QString destPath, QString destName);
	bool _SlotDoOverWriteColorScheme(int i);
	void _SlotEnableEditTab(bool on);
	void _SlotForContextMenu(const QPoint& pt);
	void _SlotForEnableCloseAllViewers(bool enable);
	void _SlotForSchemeButtonClick(int which);
	void _SlotForSchemeChange(int which);
	bool _SlotLanguagesWarning();
	void _SlotLinkClicked(QString s);		// from sample page
	void _SlotLoadItemsToListView();	// uses _currentTreeViewIndex set in _SlotAlbumStructChanged
	void _SlotPropagatePageColor();	// to all items
	void _SlotRestartRequired();	// for language change
	void _SlotSaveChangedTitleDescription();
	void _SlotSetDirectoryCountTo(int cnt) { _directoryCount = cnt; }
	void _SlotSetProgressBar(int min, int max, int pos, int phase);
	void _SlotSetProgressBarPos(int cnt1, int cnt2);
	void _SlotSetupActualBorder(BorderSide side);
	void _SlotSetupLanguagesToUI();
	void _SlotShadowForElementToUI(_CElem* pElem, int which);	// which 0: text-shadow, 1: box-shadow
	void _SlotShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void _SlotTextAlignToConfig(Align align, bool on);
	void _SlotTextDecorationToConfig(Decoration dec, bool on);
	void _SlotThumbNailViewerIsLoading(bool yes);
	void _SlotTnvCountChanged();
	void _SlotTnvMultipleSelection(IdList, ID_t);	// list of ID of selected items for multiple items
	void _SlotTnvSelectionChanged(ID_t, ID_t);		// ID of selected item or ID of enclosing album for no items
	void _SlotTnvStatusChanged(QString &); // thumbnail view status changed
	void _SlotWebPageLoaded(bool ready);
	void _SlotTreePathChanged(const BreadcrumbVector&);

// auto connected slots
private slots:
	void on_btnAddAndGenerateColorScheme_clicked();					// generate color scheme
	void on_btnAlbumMatteColor_clicked();
	void on_btnApplyColorScheme_clicked();
	void on_btnBackground_clicked();
	void on_btnBorderColor_clicked();
	void on_btnBrowseDestination_clicked();
	void on_btnBrowseForBackgroundImage();
	void on_btnBrowseSource_clicked();
	void on_btnCloseAllViewers_clicked();
	void on_btnDeleteColorScheme_clicked();
	void on_btnDisplayHint_clicked();
	void on_btnForeground_clicked();
	void on_btnGenerate_clicked();
	void on_btnGoToGoogleFontsPage_clicked();
	void on_btnGradMiddleColor_clicked();
	void on_btnGradStartColor_clicked();
	void on_btnGradStopColor_clicked();
	void on_btnImageBorderColor_clicked();
	void on_btnImageMatteColor_clicked();
	void on_btnLink_clicked();
	void on_btnMoveSchemeDown_clicked();
	void on_btnMoveSchemeUp_clicked();
	void on_btnOpenBckImage_clicked();
	void on_btnPageBackground_clicked();
	void on_btnPageColor_clicked();
	void on_btnPreview_clicked();
	void on_btnReload_clicked();
	void on_btnResetColorScheme_clicked();
	void on_btnResetDesign_clicked();
	void on_btnResetDialogs_clicked();
	void on_btnSaveConfig_clicked();
	void on_btnSaveStyleSheet_clicked();
	void on_btnSelectUplinkIcon_clicked();
	void on_btnSelectWmFont_clicked();
	void on_btnShadowColor_clicked();
	void on_btnSelectSourceGallery_clicked();
	void on_btnWmColor_clicked();
	void on_btnWmShadowColor_clicked();
	void on_btnSaveStruct_clicked();
	
	void on_cbActualItem_currentIndexChanged(int newIndex);
	void on_cbBaseLanguage_currentIndexChanged(int index);
	void on_cbDefineLanguge_currentIndexChanged(int index);
	void on_cbBorderStyle_currentIndexChanged(int newIndex);		// none, solid, dashed, etc
	void on_cbFontSizeInPoints_currentTextChanged(const QString& txt);
	void on_cbFonts_currentIndexChanged(int index);
	void on_cbImageBorderStyle_currentIndexChanged(int newIndex);		// none, solid, dashed, etc
	void on_cbImageQuality_currentIndexChanged(int newIndex);
	void on_cbLanguageTextDef_currentTextChanged(QString text);
	void on_cbLanguage_currentIndexChanged(int index);
	void on_cbLineHeight_currentTextChanged(const QString& txt);
	void on_cbPointSizeFirstLine_currentTextChanged(const QString& txt);
	void on_cbWmHPosition_currentIndexChanged(int index);
	void on_cbWmVPosition_currentIndexChanged(int index);
	
	void on_chkAddDescToAll_toggled(bool);
	void on_chkAddTitlesToAll_toggled(bool);
	void on_chkBackgroundOpacity_toggled(bool on);
	void on_chkBold_toggled(bool);
	void on_chkCleanupGallery_toggled(bool on);
	void on_chkRegenAllImages_toggled(bool);
	void on_chkNoImages_toggled(bool);
	void on_chkCanDownload_toggled(bool);
	void on_chkCropThumbnails_toggled(bool);
	void on_chkDebugging_toggled(bool);
	void on_chkDifferentFirstLine_toggled(bool b);
	void on_chkDistortThumbnails_toggled(bool);
	void on_chkDoNotEnlarge_toggled(bool);
	void on_chkFacebook_toggled(bool);
	void on_chkFixedLatestThumbnail_toggled(bool);
	void on_chkForceSSL_toggled(bool);
	void on_chkGenerateAllPages_toggled(bool);
	void on_chkIconText_toggled(bool);
	void on_chkIconTop_toggled(bool);
	void on_chkItalic_toggled(bool);
	void on_chkKeepDuplicates_toggled(bool);
	void on_chkLowerCaseImageExtensions_toggled(bool);
	void on_chkUseMaxItemCountPerDir_toggled(bool);
	void on_chkMenuToAbout_toggled(bool);
	void on_chkMenuToContact_toggled(bool);
	void on_chkMenuToDescriptions_toggled(bool);
	void on_chkMenuToToggleCaptions_toggled(bool);
	void on_chkOvrImages_toggled(bool);		// used for image processing
	void on_chkReadFromGallery_toggled(bool);
	void on_chkRightClickProtected_toggled(bool);
	void on_chkSeparateFoldersForLanguages_toggled(bool);
	void on_chkSetAll_toggled(bool);			// background colors
	void on_chkSetLatest_toggled(bool);			// generate 'Latest upload'
	void on_chkShadowOn_toggled(bool on);
	void on_chkSourceRelativeForwardSlash_toggled(bool);
	void on_chkTdLinethrough_toggled(bool);
	void on_chkTdOverline_toggled(bool);
	void on_chkTdUnderline_toggled(bool);
	void on_chkTextOpacity_toggled(bool on);
	void on_chkUseGoogleAnalytics_toggled(bool);
	void on_chkUseGradient_toggled(bool on);
	void on_chkUseWMShadow_toggled(bool);				// watermark
	void on_chkUseWM_toggled(bool);				// watermark
	
	void on_edtAbout_textChanged();
	void on_edtAlbumDir_textChanged();
	void on_edtBaseName_textChanged();
	void on_edtBckImageName_textChanged();
	void on_edtBckPathOnServer_textChanged();
	void on_edtDefaultFonts_textChanged();
	void on_edtDescriptionText_textChanged();
	void on_edtSiteDescription_textChanged();
	void on_edtDestGallery_textChanged();
	void on_edtEmailTo_textChanged();
	void on_edtFontDir_textChanged();
	void on_edtFontFamily_textChanged();
	void on_edtGalleryLanguages_textChanged();
	void on_edtSourceGallery_textChanged();
	void on_edtGalleryTitle_textChanged();
	void on_edtGoogleFonts_editingFinished();
	void on_edtImg_textChanged();
	void on_edtKeywords_textChanged();
	void on_edtMainPage_textChanged();
	void on_edtServerAddress_textChanged();
	void on_edtSourceGallery_editingFinished();
	void on_edtThumb_textChanged();
	void on_edtTitleText_textChanged();
	void on_edtTrackingCode_textChanged();
	void on_edtUplink_textChanged();
	void on_edtVid_textChanged();
	void on_edtWatermark_textChanged();
	void on_edtWmHorizMargin_textChanged();
	void on_edtWmVertMargin_textChanged();
	
	void on_hsImageSizeToShow_valueChanged(int val);
	void on_lwColorScheme_currentRowChanged(int newIndex);
	
	void on_rbAllBorders_toggled(bool);
	void on_rbBottomBorder_toggled(bool);
	void on_rbCenterBckImage_toggled(bool);
	void on_rbCoverBckImage_toggled(bool);
	void on_rbEnglish_toggled(bool);
	void on_rbLeftBorder_toggled(bool);
	void on_rbMagyar_toggled(bool);
	void on_rbNoBackgroundImage_toggled(bool);
	void on_rbRightBorder_toggled(bool);
	void on_rbTdDashed_toggled(bool);
	void on_rbTdDotted_toggled(bool);
	void on_rbTdDouble_toggled(bool);
	void on_rbTdSolid_toggled(bool);
	void on_rbTdWavy_toggled(bool);
	void on_rbTextAlignNone_toggled(bool);
	void on_rbTextCenter_toggled(bool);
	void on_rbTextLeft_toggled(bool);
	void on_rbTextRight_toggled(bool);
	void on_rbTextShadow_toggled(bool);
	void on_rbTileBckImage_toggled(bool);
	void on_rbTopBorder_toggled(bool);

	void on_sbAlbumMatteRadius_valueChanged(int val);
	void on_sbAlbumMatteWidth_valueChanged(int val);
	void on_sbBackgroundOpacity_valueChanged(int val);
	void on_sbBorderRadius_valueChanged(int val);
	void on_sbBorderWidth_valueChanged(int val);
	void on_sbGradMiddlePos_valueChanged(int val);
	void on_sbGradStartPos_valueChanged(int val);
	void on_sbGradStopPos_valueChanged(int val);
	void on_sbImageBorderRadius_valueChanged(int h);	// border radius on both matte's outside border
	void on_sbImageBorderWidth_valueChanged(int val);
	void on_sbImageHeight_valueChanged(int h);			// image height
	void on_sbImageMargin_valueChanged(int h);			// image margin
	void on_sbImageMatteRadius_valueChanged(int w);		// border radius on image inside the matte
	void on_sbImageMatteWidth_valueChanged(int val);
	void on_sbImageWidth_valueChanged(int w);			// image width
	void on_sbMaxItemCountPerDir_valueChanged(int n);
	void on_sbLatestCount_valueChanged(int val);
	void on_sbNewDays_valueChanged(int val);
	void on_sbShadowBlur1_valueChanged(int val);
	void on_sbShadowBlur2_valueChanged(int val);
	void on_sbShadowHoriz1_valueChanged(int val);
	void on_sbShadowHoriz2_valueChanged(int val);
	void on_sbShadowSpread1_valueChanged(int val);
	void on_sbShadowSpread2_valueChanged(int val);
	void on_sbShadowVert1_valueChanged(int val);
	void on_sbShadowVert2_valueChanged(int val);
	void on_sbSpaceAfter_valueChanged(int val);
	void on_sbTextOpacity_valueChanged(int val);
	void on_sbThumbnailHeight_valueChanged(int h);			// image height
	void on_sbThumbnailWidth_valueChanged(int w);				// image width
	void on_sbWmOpacity_valueChanged(int val);
	void on_sbWmShadowBlur_valueChanged(int val);
	void on_sbWmShadowHoriz_valueChanged(int val);
	void on_sbWmShadowVert_valueChanged(int val);

	void on_sldIconSize_valueChanged(int newSize);


	void on_toolBox_currentChanged(int newIndex);
};

extern FalconG *frmMain;

