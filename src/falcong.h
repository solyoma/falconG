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

/*------------------------------------- macros ------------------------------------*/
#define AS_HEX_COLOR_STRING(n) QString("%1").arg(n, 6, 16, QChar('0'))
#define TO_COLOR_STRING(s,n) s = "#" + AS_HEX_COLOR_STRING(n)

QString ColorToStr(_CColor color);
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
	QStringList _styles;

	DesignProperty _whatChanged = dpNone;

	Semaphore	_busy,		// prevent recursive parameter changes
				_running;	// operation (e.g. web page generation) is running: do not close the program yet
	int _phase = 0;			// processing phase. set by 'SetProgressBar()'
	AlbumElement _aeActiveElement = aeWebPage;
	int _directoryCount = 0;
	double _aspect=1.0;		// image_width/image_height
			   // these fields are used when editing image/album texts
	FalconGEditTabSelection _selection;	// selection in tree view
	bool _edited = false;		// when image/album text etc modified in program
								// asks for save at exit
	QColor _lastUsedMenuForegroundColor = Qt::white;

	QStringList _slStyles;

	QTextStream ifs, ofs;

	QString _GradientStyleQt(_CElem &elem, bool invert = false);	// Creates QT stylesheet gradinet string
	void  _SetWidgetGradientQt(QWidget *pwidget, _CElem *pElem, bool invert = false);	// Creates QT stylesheet gradinet string
	_CElem* _PtrToElement(AlbumElement ae = aeUndefined);


	void _SetIconColor(QIcon &icon, _CElem &elem);
	QIcon _SetUplinkIcon(QString iconName = QString());

	void _EnableGradient(bool ena);
	void _EnableButtons();
	void _GlobalsToUi();
	void _ActualSampleParamsToUi();		// using _aeActiveElement
	void _PopulateFromConfig(); // into fields
	bool _CreateAlbumDirs();
	void _SetOpacityToConfig(_CElem & elem, int which = 3); // 1: text, 2: background, 3 both

			 // set CSS for sample 
	void _RunJavaScript(QString classMName, QString propertyStrings);	// propertyStrings may contain more than one separated by ';'
	void _SetCssProperty(_CElem *pElem, QString value,  QString subSelector = QString());
	void _SetColor(_CElem *pElem);
	void _SetBackground(_CElem* pElem);		// css 'background' color + url no other properties
	void _SetShadow(_CElem* pElem,int what);	// what: 0 block-shadow, 1:text-shadow
	void _SetBorder(_CElem* pElem);			// width, style, color or none
	void _SetLinearGradient(_CElem* pElem);
	void _SetFont(_CElem* pElem);			// all parameters for font
	void _SetDecoration(_CElem* pElem);
	void _SetTextAlign(_CElem *pElem);
	void _SetPageColor(_CElem *pElem);
	void _SetPageBackground(_CElem* pElem);	// possibly for all elements
	void _SetIcon();			// re-color it for the page
	void _SetWatermark();

	void _ElemToSample(AlbumElement ae = aeUndefined);	// with no argument uses _aeActElement

	void _ConfigToSample();	// for all elements
	
	
	void _OpacityChanged(int val, int which);	// which = 0 -> color, 1: background
	void _SaveChangedTexts();  // when texts are edited and changed

	void _SetLayoutMargins(skinStyle which);
	void _StyleTheProgram(skinStyle which);

	void _GetTextsForEditing(whoChangedTheText who); // using '_selection'

//	void _GetProperty(StyleIndex index);

//	bool eventFilter(QObject *o, QEvent *e) override;

private slots:
	void LinkClicked(QString s);		// from sample page
	void WebPageLoaded(bool ready);

	void _AlbumStructureSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
	void _SetProgressBar(int min, int max, int pos, int phase);
	void _SetProgressBarPos(int cnt1, int cnt2);
	void _SetupLanguagesToUI();
	void _EnableEditTab(bool on);
//	void _ImageMapChanged();
	void _AlbumMapChanged();
	void _ShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void _CreateUplinkIcon(QString destName);
	void _SetDirectoryCountTo(int cnt) { _directoryCount = cnt; }
	void _ThumbNailViewerIsLoading(bool yes);
	void _TrvCountChanged();
	void _TnvStatusChanged(QString &); // thumbnail view status changed
	void _TnvSelectionChanged(QString );		// name of selected item or "" for no ormultiple items
	void _TnvNewThumbs();		// previous seingle selection is the new thumbnail for actual album
	void _SaveChangedTitleDescription();

// auto connected slots
private slots:
	void on_toolBox_currentChanged(int newIndex);
	void on_btnSourceHistory_clicked();
	void on_btnGenerate_clicked();
	void on_btnSaveStyleSheet_clicked();
	void on_btnSelectWmFont_clicked();
	void on_btnWmColor_clicked();
	void on_btnWmShadowColor_clicked();

	void on_cbPointSize_currentTextChanged(const QString& txt);

	void on_btnPageColor_clicked();
	void on_btnPageBackground_clicked();

	void on_btnImageBorderColor_clicked();
	void on_btnSelectUplinkIcon_clicked();

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

	void on_cbActualItem_currentIndexChanged(int newIndex);

	void on_cbBorder_currentIndexChanged(int newIndex);			// all, top, right, bottom, left border
	void on_cbBorderStyle_currentIndexChanged(int newIndex);		// solid, dashed, etc
					//  watermark
	void on_cbWmVPosition_currentIndexChanged(int index);
	void on_cbWmHPosition_currentIndexChanged(int index);
					// edit text on structure page
	void on_cbBaseLanguage_currentIndexChanged(int index);
	void on_cbLanguage_currentIndexChanged(int index);

	void on_chkDebugging_toggled(bool);
	void on_chkDifferentFirstLine_toggled(bool b);
	void on_cbPointSizeFirstLine_currentTextChanged(const QString& txt);
	void on_edtFontFamily_textChanged();
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
	void _TextDecorationToConfig(Decoration dec, bool on);
	void on_chkTdLinethrough_toggled(bool);
	void on_chkTdUnderline_toggled(bool);
	void on_chkTdOverline_toggled(bool);
	void on_rbTdSolid_toggled(bool);
	void on_rbTdDotted_toggled(bool);
	void on_rbTdDashed_toggled(bool);
	void on_rbTdDouble_toggled(bool);
	void on_rbTdWavy_toggled(bool);
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

	void on_edtAlbumDir_textChanged();
	void on_edtAbout_textChanged();
	void on_edtBaseName_textChanged();
	void on_edtDestGallery_textChanged();
	void on_edtFontDir_textChanged();
	void on_edtGalleryRoot_textChanged();
	void on_edtEmailTo_textChanged();
	void on_edtImg_textChanged();
	void on_edtServerAddress_textChanged();
	void on_edtGalleryTitle_textChanged();
	void on_edtGalleryLanguages_textChanged();
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

	// option page
	void on_rbDefaultStyle_toggled(bool);
	void on_rbSystemStyle_toggled(bool);
	void on_rbDarkStyle_toggled(bool);
	void on_rbBlackStyle_toggled(bool);
	void on_rbBlueStyle_toggled(bool);

	void on_sbImageHeight_valueChanged(int h);			// image height
	void on_sbImageWidth_valueChanged(int w);				// image width
	void on_sbThumbnailHeight_valueChanged(int h);			// image height
	void on_sbThumbnailWidth_valueChanged(int w);				// image width
	void on_edtWmColor_textChanged();
	void UpdaetWatermarkMargins(int mx, int my);
	void on_edtWmHorizMargin_textChanged();
	void on_edtWmVertMargin_textChanged();
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

	void on_sbBorderWidth_valueChanged(int val);
	void on_sbBorderRadius_valueChanged(int val);
};

extern FalconG *frmMain;

