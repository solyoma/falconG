#pragma once

#include <QtCore>
#include <QtWidgets/QMainWindow>
#include <QTextStream>

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

// separate fonts, colors for all these
enum AlbumElements { 
	aeWebPage,				// color (nWebTextColor) & background (nWebBackgroundColor) + font
	aeGalleryTitle,				// color (nGalleryTitleColor) + font
	aeGalleryDesc,				// 
	aeLangButton,				// language buttons
	aeMenuButtons,				// buttons - color (nMenuButtonsTextColor), background (nMenuButtonsBackgroundColor) -only used when no gradient!
								//         + gradient colors (gradStartColor, gradMiddleColor, gradStopColor) and positions: (gradStartPos,gradMiddlePos,gradStopPos)
								//		   + font
	aeSmallGalleryTitle,		// color (SmallnGalleryTitleColor) + font
	aeAlbumTitle,				// album title color (albumTitleColor) + font
	aeAlbumDesc,				// color (albumDescColor) + font
	aeSection,					// color (sectionTextColor) + text shadows() + font
	aeImageTitle,				// color (nImageTitleColor)  + font
	aeImageDesc					// color (nImageDescTextColor)  + font
};

/*------------------------------------------------
* selection for tree views of albums
*-----------------------------------------------*/
struct FalconGEditTabSelection
{										// what can be changed:
	enum Changed { fsNothing = 0,  fsTitle = 1, fsDescription = 2 }; // which text changed
									    // these fields are used when editing image/album texts
	ID_t actAlbum = 0,				// actual selected album
		 newAlbum = 0;				// replace data from actAlbum with data from newAlbum
	ID_t selectedImage = 0,				// actual selected image
		 newImage = 0;					// new image
	QFlags<Changed> changed;			// title or description in either language has changed
	int baseLang = 0, edtLang = 0;		// index of actual languages before the text change
	LanguageTexts title, description;			// title or description texts for any languages
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

	Semaphore	_busy,		// prevent recursive parameter changes
				_running;	// operation (e.g. web page generation) is running: do not close the program yet
	int _phase = 0;			// processing phase. set by 'SetProgressBar()'
	AlbumElements _aeActiveElement = aeWebPage;
	int _directoryCount = 0;
	double _aspect=1.0;		// image_width/image_height
			   // these fields are used when editing image/album texts
	FalconGEditTabSelection _selection;	// selection in tree view
	bool _edited = false;		// when image/album text etc modified in program
								// asks for save at exit

	QTextStream ifs, ofs;

	_CElem *_SelectSampleElement(QWidget *&pb, QString &name);	 // for _aeActiveElement
	QString _FontStyle(_CElem &elem);
	QString _GradientStyleQt(_CElem &elem, bool invert = false);	// Creates QT stylesheet gradinet string
	void _EnableGbMenu(int ena);
	void _EnableButtons();
	void _SetCommonControlsFromWebButton(StyleHandler &shElem, QWidget *pb, _CElem &elem);
	void _WebColorButtonClicked();
	void _ConfigToSample();	// parameters
	void _PopulateFromConfig(); // into fields
	bool _CreateAlbumDirs();
	void _SetOpacityToConfig(_CElem & elem, int which = 3); // 1: text, 2: background, 3 both
	void _SetChangedConfig();
	void _OpacityChanged(int val, int which);	// which = 0 -> color, 1: background
	void _SetColorToConfig(StyleHandler &handler, _CElem &elem);
	void _SetWebColor();		// set WEB button colors
	void _SetupWebButtonFromConfig();
	void _SaveChangedTexts();  // when texts are edited and changed

	enum whoChangedTheText {wctSelection = 1, wctBaseLangCombo = 2, wctLangCombo = 4};
	void _GetTextsForEditing(whoChangedTheText who); // using '_selection'

	bool eventFilter(QObject *o, QEvent *e) override;

private slots:
	void _AlbumStructureSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
	void _SetProgressBar(int min, int max, int pos, int phase);
	void _SetProgressBarPos(int cnt1, int cnt2);
	void _SetupLanguagesToUI();
	void _EnableEditTab(bool on);
//	void _ImageMapChanged();
	void _AlbumMapChanged();
	void _ShowRemainingTime(time_t actual, time_t total, int count, bool speed);
	void _SetDirectoryCountTo(int cnt) { _directoryCount = cnt; }
	void _ThumbNailViewerIsLoading(bool yes);
	void _TrvCountChanged();
	void _TnvStatusChanged(QString &); // thumbnail view status changed
	void _TnvSelectionChanged(QString );		// name of selected item or "" for no ormultiple items
	void _TnvNewThumbs();		// previous seingle selection is the new thumbnail for actual album
	void _SaveChangedTitleDescription();

	void _WebBackgroundClicked();
// auto connected slots
private slots:
	void on_btnSourceHistory_clicked();

	void on_btnGenerate_clicked();

	void on_btnSaveStyleSheet_clicked();

	void on_btnSelectWmFont_clicked();
	void on_btnWmColor_clicked();
	void on_btnWmShadowColor_clicked();
	
	void on_edtAlbumDir_textChanged();
	void on_edtAbout_textChanged();
	void on_edtBackgroundColor_textChanged();				// for many WEB element
	void on_edtDestGallery_textChanged();
	void on_edtGalleryRoot_textChanged();
	void on_edtGradStartColor_textChanged();			// gradient color #1 for many WEB element
	void on_edtGradMiddleColor_textChanged();			// gradient color #2 for many WEB element
	void on_edtGradStopColor_textChanged();			// gradient color #3 for many WEB element
	void on_edtEmailTo_textChanged();
	void on_edtImg_textChanged();
	void on_edtServerAddress_textChanged();
	void on_edtGalleryTitle_textChanged();
	void on_edtSourceGallery_textChanged();
	void on_edtTextColor_textChanged();				// for many WEB element
	void on_edtTrackingCode_textChanged();
	void on_edtUplink_textChanged();
	void on_edtMainPage_textChanged();
	void on_edtWatermark_textChanged();
	void on_sbImageHeight_valueChanged(int h);			// image height
	void on_sbImageWidth_valueChanged(int w);				// image width
	void on_sbThumbnailHeight_valueChanged(int h);			// image height
	void on_sbThumbnailWidth_valueChanged(int w);				// image width
	void on_edtWmColor_textChanged();
	void on_edtWmHorizMargin_textChanged();
	void on_edtWmVertMargin_textChanged();

	void on_cbPointSize_currentTextChanged(const QString &txt);
	void on_edtFontFamily_textChanged();

	void on_chkGenerateAll_toggled(bool);
	void on_chkAddTitlesToAll_toggled(bool);
	void on_chkAddDescToAll_toggled(bool);
	void on_chkDisregardStruct_toggled(bool);

	void on_chkRightClickProtected_toggled(bool);
	void on_chkCanDownload_toggled(bool);
	void on_chkForceSSL_toggled(bool);
	void on_chkUseGoogleAnalytics_toggled(bool);
	void on_chkSetLatest_toggled(bool);			// generate 'Latest upload'
	void on_chkTextOpacity_toggled(bool on);
	void on_chkBackgroundOpacity_toggled(bool on);
	void on_chkGradient_toggled(bool on);
	void on_chkShadowOn_toggled(bool on);
	void on_chkButtonBorder_toggled(bool on);
	void on_chkImageBorder_toggled(bool);		// image border
	void on_chkIconText_toggled(bool);
	void on_chkIconTop_toggled(bool);
	void on_chkBold_toggled(bool);
	void on_chkItalic_toggled(bool);
	void on_chkStrikethrough_toggled(bool);
	void on_chkUnderline_toggled(bool);
	void on_chkUseWM_toggled(bool);				// watermark
	void on_chkOvrImages_toggled(bool);		// used for image processing
	void on_chkDoNotEnlarge_toggled(bool);
	void on_chkSourceRelativePerSign_toggled(bool);
	void on_chkMenuToContact_toggled(bool);
	void on_chkMenuToAbout_toggled(bool);
	void on_chkMenuToDescriptions_toggled(bool);
	void on_chkMenuToToggleCaptions_toggled(bool);

	void on_sbNewDays_valueChanged(int val);
	void on_sbLatestCount_valueChanged(int val);
	void on_sbShadowHoriz_valueChanged(int val);
	void on_sbShadowVert_valueChanged(int val);
	void on_sbShadowBlur_valueChanged(int val);
	void on_sbGradStartPos_valueChanged(int val);
	void on_sbGradMiddlePos_valueChanged(int val);
	void on_sbGradStopPos_valueChanged(int val);
	void on_sbWmShadowVert_valueChanged(int val);
	void on_sbWmShadowHoriz_valueChanged(int val);
	void on_sbWmShadowBlur_valueChanged(int val);
	void on_sbBorderWidth_valueChanged(int val);
	void on_sbTextOpacity_valueChanged(int val);
	void on_sbBackgroundOpacity_valueChanged(int val);
	void on_sbWmOpacity_valueChanged(int val);

	void on_btnGalleryTitle_clicked();
	void on_btnGalleryDesc_clicked();
	void on_btnSmallGalleryTitle_clicked();
	void on_btnMenu_clicked();
	void on_btnGradBorder_clicked();
	void on_btnLang_clicked();
	void on_btnUplink_clicked();
	void on_btnCaption_clicked();
	void on_btnAlbumTitle_clicked();
	void on_btnAlbumDesc_clicked();
	void on_btnSection_clicked();
	void on_btnImageTitle_clicked();
	void on_btnImageDesc_clicked();

	void on_btnSaveConfig_clicked();

	void on_btnBrowseSource_clicked();
	void on_btnBrowseDestination_clicked();
	// color buttons
	void on_btnColor_clicked();	
	void on_btnBackground_clicked();
	void on_btnGradStartColor_clicked();
	void on_btnGradMiddleColor_clicked();
	void on_btnGradStopColor_clicked();
	void on_btnBorderColor_clicked();
	void on_btnShadowColor_clicked();

	void on_btnResetDesign_clicked();
	void on_btnDisplayHint_clicked();

	void on_cbWmVPosition_currentIndexChanged(int index);
	void on_cbWmHPosition_currentIndexChanged(int index);

					// edit text on structure page
	void on_cmbBaseLanguage_currentIndexChanged(int index);
//	void on_edtBaseTitle_textChanged();
//	void on_edtBaseDescription_textChanged();
	void on_cmbLanguage_currentIndexChanged(int index);
	void on_edtTitleText_textChanged();
	void on_edtDescriptionText_textChanged();

	void on_btnPreview_clicked();
};

extern FalconG *frmMain;

