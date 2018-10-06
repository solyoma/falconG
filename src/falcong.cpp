#include <QCloseEvent>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QDirIterator>
#include <QListView>

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"
#include "falcong.h"
#include "structEdit.h"
#include "sourcehistory.h"

FalconG *frmMain = nullptr;

/*------------------------------------- macros ------------------------------------*/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
static QString __ColorStyle(_CElem &elem)
{
	return QString("\n color: ") + ColorToStr(elem.color)
		+ ";\n background-color: " + ColorToStr(elem.background) + ';';
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
static QString __InvertColor(_CColor c)
{
	QString qs = c;
	int i = ~qs.toInt(0, 16);
	// DEBUG	qs = QString("#%1").arg((i & 0xFFFFFF), 0, 16);
	return QString("#%1").arg((i & 0xFFFFFF), 0, 16);
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
static QString __ContrastColorString(_CColor c)
{
	QColor qc = c.name;
	if (qc.isValid())
	{
		if (qc.red() + qc.green() + qc.blue() > 3*128)
			return "#000";
		else
			return "#fff";
	}
	else
		return "#000";
}

// DBUG
void ShowStyleOf(QWidget *pq) 
{
	QMessageBox msgb; msgb.setText(pq->styleSheet()); 
	msgb.setInformativeText("\nStyle for " + pq->objectName());
	msgb.setWindowTitle("falconG - Style for " + pq->objectName()); 
	msgb.exec();
}
// /DEBUG

//*************** helper *******************
static bool IsValidColor(QString str)
{
	if (str[0] != '#')
		str = '#' + str;
	QColor color(str);
	return color.isValid();
}

/*============================================================================
* TASK: format color QString into rgba when opacity is used, use original
*		 hex color QString if not. 
* EXPECTS: color - to transform
* RETURNS: color string starting w. '#'
*----------------------------------------------------------------------------*/
QString ColorToStr(_CColor color)
{
	QString &c = color.name;
	int n = c[0] == '#' ? 1 : 0;
	int opacity = color.opacity;
	if (opacity > 0)	// color name format: #AABBCC
		return QString("rgba(%1,%2,%3,%4)").arg(c.mid(n, 2).toInt(nullptr, 16)).arg(c.mid(n+2, 2).toInt(nullptr, 16)).arg(c.mid(n+4, 2).toInt(nullptr, 16)).arg(opacity / 100.0);

	return n ? c : '#' + c;
}

/*============================================================================
* TASK: format integer to hex color QString  with format #rrggbb
*		 
*----------------------------------------------------------------------------*/
QString IntToColorStr(int clr)
{
	return QString("#%1%2%3").arg(clr / 65536, 2, 16).arg((clr / 256) & 0xFF, 2, 16).arg(clr & 0xFF, 2, 16);
}


//**************************************** falconG *******************************************


/*============================================================================
* TASK:		construct a FalconG object, read its parameters, set up controls
*			and sample web page
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FalconG::FalconG(QWidget *parent)
	: QMainWindow(parent)
{
	config.dsApplication = QDir::current().absolutePath(); // before anybody changes the current directory

	ui.setupUi(this);
	ui.pnlProgress->setVisible(false);

	ui.trvAlbums->setHeaderHidden(true);
	ui.trvAlbums->setModel(new AlbumTreeModel());
	connect(ui.trvAlbums->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FalconG::_AlbumStructureSelectionChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SingleSelection, this, &FalconG::_TnvSelectionChanged);
	// connect with albumgen's 
	connect(this,	   &FalconG::CancelRun,						&albumgen, &AlbumGenerator::Cancelled);
	connect(&albumgen, &AlbumGenerator::SignalToSetProgressParams,		this, &FalconG::_SetProgressBar);
	connect(&albumgen, &AlbumGenerator::SignalProgressPos,			this, &FalconG::_SetProgressBarPos);
	connect(&albumgen, &AlbumGenerator::SignalSetLanguagesToUI,		this, &FalconG::_SetupLanguagesToUI);
	connect(&albumgen, &AlbumGenerator::SignalToEnableEditTab,		this, &FalconG::_EnableEditTab);
//	connect(&albumgen, &AlbumGenerator::SignalImageMapChanged,		this, &FalconG::_ImageMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalAlbumStructChanged,	this, &FalconG::_AlbumMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalToShowRemainingTime,	this, &FalconG::_ShowRemainingTime);
	connect(&albumgen, &AlbumGenerator::SetDirectoryCountTo,		this, &FalconG::_SetDirectoryCountTo);
	connect(ui.tnvImages, &ThumbnailWidget::SignalInProcessing,		this, &FalconG::_ThumbNailViewerIsLoading);
//	connect(ui.tnvImages, &ThumbnailWidget::SignalTitleChanged,		this, &FalconG::_TrvTitleChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SignalStatusChanged,	this, &FalconG::_TnvStatusChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SignalNewThumbnail,		this, &FalconG::_TnvNewThumbs);
	connect(ui.btnSaveChangedDescription, &QPushButton::clicked,  this, &FalconG::_SaveChangedTitleDescription);
	connect(ui.btnSaveChangedTitle,		  &QPushButton::clicked,  this, &FalconG::_SaveChangedTitleDescription);

	CONFIGS_USED::Read();
									  // if there was a last used directory read from it
	if ((CONFIGS_USED::indexOfLastUsed >= 0) && (CONFIGS_USED::lastConfigs.size() > CONFIGS_USED::indexOfLastUsed))
		config.Read(&CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed)); 
	else
		config.Read();				 // else read from program directory

	frmMain = this;
	_PopulateFromConfig();	// edit values from config

	ui.frmWeb->installEventFilter(this);

	_WebBackgroundClicked();	// default selection to config parameters
}

/*============================================================================
* TASK:		destructor
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FalconG::~FalconG()
{
}

/*============================================================================
* TASK:		prevent closing when qan operation is running
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::closeEvent(QCloseEvent * event)
{
	if (_running)		// operation in progress
	{
		QMessageBox(QMessageBox::Warning, "falconG - Warning",
			"An operation is in progress.\n Please stop it before exiting.",
			QMessageBox::Ok, this).exec();
		event->ignore();
	}

	if(_edited)
	{
		int res;
		res = QMessageBox(QMessageBox::Question, "falconG - albums edited",
			"There are unsaved changes in the albums / images\nDo you want to save them?",
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this).exec();
		if (res == QMessageBox::Save)
			albumgen.WriteDirStruct();
		else if (res == QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	if (config.changed)
	{
		int res;
		res  = QMessageBox(QMessageBox::Question, "falconG - Configuration",
			"There are unsaved changes in the configuration\nDo you want to save them?",
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this).exec();
		if (res == QMessageBox::Save)
			config.Write();
		else if (res == QMessageBox::Cancel)
			event->ignore();
	}
	else
		CONFIGS_USED::Write();						// save data in program directory
}

/*============================================================================
* TASK:		  display history of previous galleries and seelct from them
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSourceHistory_clicked()
{
	SourceHistory hist(CONFIGS_USED::lastConfigs, this);
	hist.exec();
	if (SourceHistory::Selected() >= 0 && SourceHistory::Selected() != CONFIGS_USED::indexOfLastUsed)
	{
		CONFIGS_USED::indexOfLastUsed = SourceHistory::Selected();
		config.Read(&CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed));

		_PopulateFromConfig();	// edit values from config

		QString s = ui.edtSourceGallery->text();
		if (ui.edtSourceGallery->text().isEmpty())
		{
			++_busy;
			ui.edtSourceGallery->setText(CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed));
			--_busy;
		}

		_WebBackgroundClicked();	// default selection to config parameters
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGenerate_clicked()
{
	if (_running)
	{
		if (!--_running)
			emit CancelRun();
	}
	else
	{
		CONFIGS_USED::Write();
		albumgen.RecreateWebPages(ui.chkGenerateAll->isChecked() || config.changed);

		++_running;
		_edited = false;

		ui.btnSaveStyleSheet->setEnabled(false);

		ui.btnSaveConfig->setEnabled(false);

		ui.btnExit->setEnabled(false);
		ui.actionExit->setEnabled(false);
		ui.btnPreview->setEnabled(false);

		ui.centralWidget->setCursor(Qt::WaitCursor);
		ui.btnGenerate->setText(tr("Cancel"));
		ui.pnlProgress->setVisible(true);

		ui.tabFalconG->setCurrentIndex(0);	// show front page
			   // when interrupted in phase1 (_phase == 0) total generation is used
		_phase = 0;
		if (!albumgen.Read())		// any error
			--_running;

		_TrvCountChanged();			// show in lblTotalCount (page: Edit)

		if (_running)
			ui.btnPreview->setEnabled(albumgen.Write() == 0);
				

		ui.progressBar->setValue(0);

		ui.pnlProgress->setVisible(false);
		ui.centralWidget->setCursor(Qt::ArrowCursor);
		ui.btnGenerate->setText(tr("Generate"));
		ui.btnSaveStyleSheet->setEnabled(true);

		ui.btnSaveConfig->setEnabled  (config.changed);

		ui.btnExit->setEnabled(true);
		ui.actionExit->setEnabled(true);

		--_running;
		ui.tabFalconG->setCurrentIndex(3);	// show 'Edit' page

		albumgen.RecreateWebPages(ui.chkGenerateAll->isChecked());	// not until relevant changes
	}

	ui.btnSaveStyleSheet->setEnabled(!_running);
}

/*============================================================================
* TASK:	  (Re)creates all albums.txt, meta.properties, comment.properties and
*		  *.info files for (possibly modified) image data into the same
*		 directory structure as JAlbum could use to generate its album
*		 in directory 'src' under the destination directory
* EXPECTS:	_albumMap, _imageMap and _textMap are either generated or read in
* GLOBALS: albumgen
* REMARKS:	- original files are not modified
*			- all new files are encoded as UTF-8
*			- .info files are copied from source
*			- albumfiles.txt links to original images in source directories
*--------------------------------------------------------------------------*/
//void FalconG::on_btnCreateJAlbum_clicked()
//{
//}


/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_EnableButtons()
{
	bool bEnable1 = !ui.edtSourceGallery->text().isEmpty() &&
					!ui.edtServerAddress->text().isEmpty(),
		 bEnable2 = !ui.edtDestGallery->text().isEmpty();

	if (bEnable1)
	{
		QDir qd(ui.edtSourceGallery->text());
		bEnable1 &= qd.exists();
	}

	ui.btnGenerate->setEnabled(bEnable1 && bEnable2);
	ui.btnSaveStyleSheet->setEnabled(bEnable2);
	ui.btnSaveConfig->setEnabled(config.changed);
}

/*============================================================================
* TASK:		set all characteristics of the sample gallery from 'config'
* EXPECTS: 'config' is set up
* GLOBALS:  'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_ConfigToSample()
{
	_EnableGbMenu(0);		// backup changed gradient colors into static variables and disable buttons

	++_busy;		// prevent page changes until last settings

	ui.btnBorderColor->setStyleSheet("QToolButton {\nbackground-color:" + config.imageBorder.color.name + ";\n}");
	ui.sbBorderWidth->setValue(config.imageBorder.width);
	ui.btnGradBorder->setStyleSheet("QToolButton {\n background-color:" + config.Menu.border.color.name + 
		                                          ";\n color:" + __ContrastColorString(config.Menu.border.color) + ";\n}");
	QString ss = "";
				  // style for menu buttons
	ss = "\n color:" + ColorToStr(config.Menu.color) +
		";\n padding: 4px 10px 3px" +
		";\n margin: 2px 3px" +
		";\n line-height: 18px" +
		';'+ _FontStyle(config.Menu);
	if (config.Menu.border.used)
		ss += QString("\nborder:1px solid %1;").arg(config.Menu.border.color.name);
	ss += "\n box-shadow: 0px 1px 1px rgba(0, 0, 0, 0.1);\n border-radius: 5px;\n}\n";
	if(config.Menu.gradient.used)
		ss = "QPushButton {\n background:" + _GradientStyleQt(config.Menu, false) + ';'+ ss +
			"QPushButton:pressed {\n background:" + _GradientStyleQt(config.Menu, true) + ';'+ ss;
	else
		ss = "QPushButton {\n background-color:" + config.Menu.background + ';' + ss +
		"QPushButton:pressed {\n background:" + __InvertColor(config.Menu.background) + ';' + ss;

	ui.btnMenu->setStyleSheet(ss);
	ui.btnUplink->setStyleSheet(ss);
	ui.btnCaption->setStyleSheet(ss);

					// style for other elements
#define SET_STYLE_TO(elem)	 ss = "QPushButton {\n" + __ColorStyle(config.elem) + "\n}\n";	\
							 ui.btn##elem->setStyleSheet(ss);

		// special for gallery title: not a button, but a panel
	ss = "QFrame {" + __ColorStyle(config.Web) + "\n}\n";
	ui.frmWeb->setStyleSheet(ss);
	ss = "QWidget" + ss.mid(6);
	//ss = "QWidget {\nbackground-color: " + ColorToStr(config.Web.background)\
	//	+ ";\n color: " + ColorToStr(config.Web.color) + ";\n}";
	ui.pnlGallery->setStyleSheet(ss);
	// DEBUG
	//	ShowStyleOf(ui.pnlGallery);

		// change font for these element
#define SET_STYLE_W_FONT_TO(elem) ss = "QPushButton {" + __ColorStyle(config.elem) + _FontStyle(config.elem) + "\n}";	\
									ui.btn##elem->setStyleSheet(ss);

	SET_STYLE_W_FONT_TO(GalleryTitle)
// DEBUG
// ShowStyleOf(ui.btnGalleryTitle);
	SET_STYLE_W_FONT_TO(GalleryDesc)
	SET_STYLE_W_FONT_TO(Lang)
	SET_STYLE_W_FONT_TO(AlbumTitle)
	SET_STYLE_W_FONT_TO(AlbumDesc)
	SET_STYLE_W_FONT_TO(Section)
	SET_STYLE_W_FONT_TO(ImageTitle)
	SET_STYLE_W_FONT_TO(ImageDesc)

	on_chkImageBorder_toggled(config.imageBorder.used);
	--_busy;
}

/*============================================================================
* TASK:		set all saved data into global controls	+ interface elements
* EXPECTS:	'config' is set up
* GLOBALS:	'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_PopulateFromConfig()
{
	++_busy;

	ui.edtSourceGallery->setText(QDir::toNativeSeparators(config.dsSrc.ToString()));
	ui.edtDestGallery->setText(QDir::toNativeSeparators(config.dsGallery.ToString()));

	ui.edtServerAddress->setText(config.sServerAddress);
	ui.edtGalleryRoot->setText(config.dsGRoot.str);
	ui.edtUplink->setText(config.sUplink);

	ui.edtGalleryTitle->setText(config.sGalleryTitle);
	ui.edtAlbumDir->setText(config.dsAlbumDir.str);
	ui.edtEmailTo->setText(config.sMailTo);
	ui.edtAbout->setText(config.sAbout);

	ui.edtImg->setText(config.dsImageDir.ToString());

	ui.chkFacebook->setChecked(config.bfacebookLink);
	ui.chkAddTitlesToAll->setChecked(config.bAddTitlesToAll);
	ui.chkAddDescToAll->setChecked(config.bAddDescriptionsToAll);
	ui.chkDoNotEnlarge->setChecked(config.doNotEnlarge);
	ui.chkDisregardStruct->setChecked(config.bDisregardStruct);
	ui.chkMenuToContact->setChecked(config.bMenuToContact);
	ui.chkMenuToAbout->setChecked(config.bMenuToAbout);
	ui.chkMenuToDescriptions->setChecked(config.bMenuToDescriptions);
	ui.chkMenuToToggleCaptions->setChecked(config.bMenuToToggleCaptions);

	ui.chkSourceRelativePerSign->setChecked(config.bSourceRelativePerSign);


	ui.chkButtonBorder->setChecked(config.Menu.border.used);
//	ui.btnGradBorder->setStyleSheet(QString("QToolButton {\n background-color:%1;\ncolor:%2;\n").arg(config.Menu.border.color.name).arg(__InvertColor(config.Menu.border.color)));
	ui.chkGradient->setChecked(config.Menu.gradient.used);
	ui.chkOvrImages->setChecked(config.bOvrImages);

	ui.btnGradStartColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[0].color + "}");
	ui.btnGradMiddleColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[1].color + "}");
	ui.btnGradStopColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[2].color + "}");

	ui.chkSetLatest->setChecked(config.generateLatestUploads);
	ui.sbNewDays->setValue(config.newUploadInterval);

	ui.chkUseGoogleAnalytics->setChecked(config.googleAnalyticsOn);
	ui.edtTrackingCode->setText(config.googleAnalTrackingCode);

	ui.sbImageWidth->setValue(config.imageWidth);
	ui.sbImageHeight->setValue(config.imageHeight);
	ui.btnLink->setChecked(config.imageSizesLinked);
	ui.chkDoNotEnlarge->setChecked(config.doNotEnlarge);

	int h = config.imageHeight ? config.imageHeight : 1;
	_aspect = (double)config.imageWidth / h;

							// Watermark
	ui.chkUseWM->setChecked(config.waterMark.used);
	ui.sbWmOpacity->setValue(config.waterMark.wm.opacity);

	ui.edtWatermark->setText(config.waterMark.wm.text);

	h = config.waterMark.wm.origin & 0xF;  // vertical position
	ui.cbWmVPosition->setCurrentIndex(h);
	h = (config.waterMark.wm.origin & 0xF0) >> 4; // horizontal position
	ui.cbWmHPosition->setCurrentIndex(h);
	
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.wm.marginY));
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.wm.marginX));

	ui.btnWmColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.color));
	ui.edtWmColor->setText(QString().setNum(config.waterMark.wm.color, 16));
	ui.btnWmShadowColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.shadowColor));

	ui.lblWaterMarkFont->setText(QString("%1 (%2,%3)").arg(config.waterMark.wm.font.family()).arg(config.waterMark.wm.font.pointSize()).arg(config.waterMark.wm.font.italic()));
	if (!config.waterMark.wm.text.isEmpty())
		ui.lblWmSample->setText(config.waterMark.wm.text);

	ui.lblWmSample->setFont(config.waterMark.wm.font);

	_ConfigToSample(); // to sample "WEB page"
	config.changed = false;

	--_busy;
}

/*============================================================================
 * TASK: Create non existing directories using names in global CONFIG:
 *		'dsGallery', dsGRoot, dsGRoot/css, dsGRoot/js, dsAlbumDir, dsAlbumDir/dsImageDir
 * EXPECTS:	Config is set up
 * RETURNS: true at success, false at failure
 * REMARKS: existing directories are left intact
 *---------------------------------------------------------------------------*/
bool FalconG::_CreateAlbumDirs()
{
	return true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtTrackingCode_textChanged()
{
	if (_busy)
		return;

	config.googleAnalTrackingCode = ui.edtTrackingCode->text();
	config.googleAnalTrackingCode.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtDestGallery_textChanged()
{
	if (_busy)
		return;

	config.dsGallery = ui.edtDestGallery->text();
	config.dsGallery.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtSourceGallery_textChanged()
{	
	if (_busy)
		return;

	if (!config.dsSrc.str.isEmpty() && config.dsSrc.str != ui.edtSourceGallery->text())
	{
		config.dsSrc = ui.edtSourceGallery->text();
		config.dsSrc.changed = true;
		for(int i = 0; i < CONFIGS_USED::lastConfigs.size(); ++i )
			if (config.dsSrc.str == CONFIGS_USED::lastConfigs[i])
			{
				CONFIGS_USED::indexOfLastUsed = i;
				break;
			}
		QFile f(config.dsSrc.str + falconG_ini);
		if (f.exists())
		{
			config.Read(&CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed));
			_PopulateFromConfig();	// edit values from config

			_WebBackgroundClicked();	// default selection to config parameters

			_EnableButtons();
		}
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtAlbumDir_textChanged()
{
	if (_busy)
		return;

	config.dsAlbumDir = ui.edtAlbumDir->text();
	config.dsAlbumDir.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtAbout_textChanged()
{
	if (_busy)
		return;

	config.sAbout = ui.edtAbout->text();
	config.sAbout.changed = true;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtGalleryRoot_textChanged()
{
	if (_busy)
		return;

	config.dsGRoot = ui. edtGalleryRoot->text();
	config.dsGRoot.changed = true;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtImg_textChanged()
{
	if (_busy)
		return;

	config.dsImageDir = ui. edtImg->text();
	config.dsImageDir.changed = true;
}

/*============================================================================
  * TASK:
  * EXcPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtServerAddress_textChanged()
{
	_EnableButtons();
	if (_busy)
		return;

	config.sServerAddress = ui. edtServerAddress->text();
	config.sServerAddress.changed = true;
}

/*============================================================================
* TASK:
* EXcPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtGalleryTitle_textChanged()
{
	if (_busy)
		return;

	config.sGalleryTitle = ui.edtGalleryTitle->text();
	config.sGalleryTitle.changed = true;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtUplink_textChanged()
{
	if (_busy)
		return;

	config.sUplink = ui. edtUplink->text();
	config.sUplink.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtMainPage_textChanged()
{
	if (_busy)
		return;

	config.sMainPage = ui.edtMainPage->text();
	config.sMainPage.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtEmailTo_textChanged()
{
	if (_busy)
		return;

	config.sMailTo = ui.edtEmailTo->text();
	config.sMailTo.changed = true;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtTextColor_textChanged()
{
	QString text = '#' + ui.edtTextColor->text();
	if (_busy || !IsValidColor(text))
		return;
	config.designChanged = true;
	++_busy;

	_SetWebColor();
	StyleHandler handler(ui.btnColor->styleSheet());
	handler.SetItem("QToolButton", "background-color", text);
	ui.btnColor->setStyleSheet(handler.StyleSheet());
	config.Web.color = text;
	config.Web.color.changed = true;

	--_busy;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtBackgroundColor_textChanged()
{
	QString text = '#' + ui.edtBackgroundColor->text();
	if (_busy || !IsValidColor(text))
		return;

	config.changed = config.designChanged = true;
	++_busy;

	StyleHandler handler(ui.btnBackground->styleSheet());
	handler.SetItem("QToolButton", "background-color", text);
	ui.btnBackground->setStyleSheet(handler.StyleSheet());

	_SetWebColor();
	--_busy;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtGradStartColor_textChanged()
{
	if (_busy || !IsValidColor(ui.edtTextColor->text()))
		return;

	ui.btnGradStartColor->setStyleSheet("QToolButton { background-color:#" + ui.edtGradStartColor->text()+"}");
	config.Menu.gradient.changed = true;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtGradMiddleColor_textChanged()
{
	if (_busy || !IsValidColor(ui.edtTextColor->text()))
		return;

	ui.btnGradMiddleColor->setStyleSheet("QToolButton { background-color:#" + ui.edtGradMiddleColor->text()+"}");
	config.Menu.gradient.changed = true;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtGradStopColor_textChanged()
{
	if (_busy || !IsValidColor(ui.edtTextColor->text()))
		return;

	ui.btnGradStopColor->setStyleSheet("QToolButton { background-color:#" + ui.edtGradStopColor->text() + ";}");
	config.Menu.gradient.changed = true;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWatermark_textChanged()
{
	if (_busy)
		return;
	config.waterMark.wm.SetText(ui.edtWatermark->text() );
	if(config.waterMark.wm.text.isEmpty())
		ui.lblWmSample->setText("Watermark Sample text");
	else
		ui.lblWmSample->setText(config.waterMark.wm.text);
	config.waterMark.changed = true;

	ui.btnSaveConfig->setEnabled(config.changed = true); 
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWmColor_textChanged()
{
	ui.btnSaveConfig->setEnabled(config.changed = true); 

	if (_busy)
		return;
	StyleHandler handler(ui.btnWmColor->styleSheet());
	handler.SetItem("QToolButton", "background-color", "#" + ui.edtWmColor->text());
	ui.btnWmColor->setStyleSheet(handler.StyleSheet());
	config.waterMark.wm.SetColor(ui.edtWmColor->text());
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWmHorizMargin_textChanged()
{
	if (_busy)
		return;
	config.waterMark.wm.marginX = ui.edtWmHorizMargin->text().toInt();
	config.waterMark.changed = true;
	ui.btnSaveConfig->setEnabled(true); 
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWmVertMargin_textChanged()
{
	if (_busy)
		return;
	config.waterMark.wm.marginY = ui.edtWmVertMargin->text().toInt();
	config.waterMark.changed = true;

	ui.btnSaveConfig->setEnabled(true);
}

/*============================================================================
  * TASK:
  * EXPECTS: h - new value (old is in config)
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_sbImageHeight_valueChanged(int h)
{
	if (_busy)
		return;
	++_busy;
	int w = config.imageWidth;

	if (ui.btnLink->isChecked())
	{
		w = h * _aspect;
		ui.sbImageWidth->setValue(w);
		config.imageWidth = w;
		config.imageWidth.changed = true;
		h = 0; // no change in aspect ratio when linked!
	}
	config.imageHeight = h;
	config.imageHeight.changed = true;
	if(h)
		_aspect = (double)w / (double)h;

	--_busy;
}

/*============================================================================
  * TASK:
  * EXPECTS: w - new value (old is in config)
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_sbImageWidth_valueChanged(int w)
{
	if (_busy)
		return;
	++_busy;
	int h = config.imageHeight;

	if (ui.btnLink->isChecked())
	{
		h = w / _aspect;
		ui.sbImageHeight->setValue(h);
		h = 0; // no change in aspect ratio when linked!
	}
	config.imageWidth = w;
	config.imageWidth.changed = true;
	if(h)	// else no change
		_aspect = (double)w / (double)h;
	--_busy;
}

/*============================================================================
* TASK:
* EXPECTS: h - new value (old is in config)
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbThumbnailHeight_valueChanged(int h)
{
	if (_busy)
		return;
	++_busy;
	int w = config.imageWidth;

	if (ui.btnLink->isChecked())
	{
		w = h * _aspect;
		ui.sbThumbnailWidth->setValue(w);
		config.thumbWidth = w;
		config.thumbWidth.changed = true;
		h = 0; // no change in aspect ratio when linked!
	}
	config.thumbHeight = h;
	config.thumbHeight.changed = true;
	if (h)
		_aspect = (double)w / (double)h;

	--_busy;
}

/*============================================================================
* TASK:
* EXPECTS: h - new value (old is in config)
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbThumbnailWidth_valueChanged(int w)
{
	if (_busy)
		return;
	++_busy;
	int h = config.imageHeight;

	if (ui.btnLink->isChecked())
	{
		h = w / _aspect;
		ui.sbThumbnailHeight->setValue(h);
		//config.imageWidth = w;
		//config.imageWidth.changed = true;
		h = 0; // no change in aspect ratio when linked!
	}
	config.thumbWidth = w;
	config.thumbWidth.changed = true;
	if (h)	// else no change
		_aspect = (double)w / (double)h;
	--_busy;
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_chkGenerateAll_toggled(bool on)
{
	if (_busy)
		return;
	if (config.bGenerateAll != on)
	{
		config.bGenerateAll = on;
		config.bGenerateAll.changed = true;
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkAddTitlesToAll_toggled(bool on)
{
	if (_busy)
		return;
	config.bAddTitlesToAll = on;
	config.bAddTitlesToAll.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkAddDescToAll_toggled(bool on)
{
	if (_busy)
		return;
	config.bAddDescriptionsToAll = on;
	config.bAddDescriptionsToAll.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkDisregardStruct_toggled(bool on)
{
	if (_busy)
		return;
	config.bDisregardStruct = on;
	config.bDisregardStruct.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkRightClickProtected_toggled(bool b)
{
	config.bRightClickProtected = b;
	config.bRightClickProtected.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkCanDownload_toggled(bool b)
{
	config.bCanDownload = b;
	config.bCanDownload.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkForceSSL_toggled(bool b)
{
	config.bForceSSL = b;
	config.bForceSSL.changed = true;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkUseGoogleAnalytics_toggled(bool b)
{
	config.googleAnalyticsOn = b; // ui.chkUseGoogleAnalytics->isChecked();
	config.googleAnalyticsOn.changed = true;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkSetLatest_toggled(bool on)
{
	config.generateLatestUploads = on;
	config.generateLatestUploads.changed = true;
	ui.btnSaveConfig->setEnabled(true); 
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_cbPointSize_currentTextChanged(const QString & txt)
{
	if (_busy)
		return;
	// test 'txt' for valid font size
	QRegularExpression rex("^\\d+\\w*$");
	if (txt.indexOf(rex) < 0)
		return;

	QString text = txt;
	QWidget *pb;
	_CElem *pElem;
	StyleHandler handler;

	QString qsClass;
	pElem = _SelectSampleElement(pb, qsClass);
	handler.Set(pb->styleSheet());
	handler.SetItem(qsClass, "font-size", text);
	pElem->font.changed = true;
	pElem->font.sizestr = txt;
	pb->setStyleSheet(handler.StyleSheet());
	config.changed = config.designChanged = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_edtFontFamily_textChanged()
{
	if (_busy)
		return;
	QWidget *pb;
	_CElem *pElem;
	StyleHandler handler;

	QString qsClass;
	pElem = _SelectSampleElement(pb, qsClass);
	handler.Set(pb->styleSheet());
	pElem->font.changed = true;
	pElem->font.family = ui.edtFontFamily->text();
	handler.SetItem(qsClass, "font-family", pElem->font.family);
	pb->setStyleSheet(handler.StyleSheet());
// DEBUG
//QString s = pb->styleSheet();
	config.changed = config.designChanged = true;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkImageBorder_toggled(bool on)
{
	StyleHandler handler;
	QString ss;
	handler.Set(ui.btnBorderColor->styleSheet());
	ss = handler.GetItem("QToolButton", "background-color");	// background
	ss = (on ? QString("border%1px").arg(ui.sbBorderWidth->value()): QString()) + "solid " + ss;
	ui.btnImage->setStyleSheet(handler.StyleSheet());
	config.changed = config.designChanged = true;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkIconText_toggled(bool)
{
	if (_busy)
		return;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkIconTop_toggled(bool)
{
	if (_busy)
		return;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkBold_toggled(bool)
{
	if (_busy)
		return;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkItalic_toggled(bool)
{
	if (_busy)
		return;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkStrikethrough_toggled(bool)
{
	if (_busy)
		return;
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkUnderline_toggled(bool)
{
	if (_busy)
		return;
	config.changed = config.designChanged = true;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkUseWM_toggled(bool on)
{
	if (_busy)
		return;
	config.waterMark.used = on;
	config.waterMark.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkOvrImages_toggled(bool on)
{
	if (_busy)
		return;
	config.bOvrImages = on;
	config.bOvrImages.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkDoNotEnlarge_toggled(bool on)
{
	if (_busy)
		return;
	config.doNotEnlarge = on;
	config.doNotEnlarge.changed = true;
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkSourceRelativePerSign_toggled(bool on)
{
	if (_busy)
		return;
	config.bSourceRelativePerSign = on;
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkTextOpacity_toggled(bool on)
{
	if (_busy)
		return;
	ui.sbTextOpacity->setEnabled(on);
	_SetChangedConfig();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkBackgroundOpacity_toggled(bool on)
{
	if (_busy)
		return;
	ui.sbBackgroundOpacity->setEnabled(on);
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkGradient_toggled(bool on)
{
	if (_busy)
		return;
	switch (_aeActiveElement)
	{
		case aeWebPage: break;
		case aeGalleryTitle: break;
		case aeGalleryDesc: break;
		case aeLangButton: break;
		case aeMenuButtons: config.Menu.gradient.changed = true;  
							config.Menu.gradient.used = on;
							if (on)
							{
								config.Menu.color.changed = true; // want to save these too when no gradient
								config.Menu.background.changed = true;
								config.Menu.border.changed = true;
							}
					break;
		case aeSmallGalleryTitle: break;
		case aeAlbumTitle: break;
		case aeAlbumDesc: break;
		case aeSection: break;
		case aeImageTitle: break;
		case aeImageDesc: break;
	}
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkButtonBorder_toggled(bool on)
{
	if (_busy)
		return;
	config.Menu.border.used = on;
	config.Menu.border.changed = true;
	StyleHandler handler(ui.btnMenu->styleSheet());
	if (on)
	{
		handler.SetItem("QPushButton", "border", "1px solid " + config.Menu.border.color.name);
		handler.SetItem("QPushButton:pressed", "border", "1px solid " + config.Menu.border.color.name);
	}
	else
	{
		handler.RemoveItem("QPushButton", "border");
		handler.RemoveItem("QPushButton:pressed", "border");
	}
	QString ss = handler.StyleSheet();
	ui.btnMenu->setStyleSheet(ss);
	ui.btnCaption->setStyleSheet(ss);
	ui.btnUplink->setStyleSheet(ss);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToContact_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToContact = on;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToAbout_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToAbout = on;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToDescriptions_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToDescriptions = on;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToToggleCaptions_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToToggleCaptions = on;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbNewDays_valueChanged(int val)
{
	if (_busy)
		return;
	config.newUploadInterval = val;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbLatestCount_valueChanged(int val)
{
	if (_busy)
		return;
	config.nLatestCount = val;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowHoriz_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowVert_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowBlur_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradStartPos_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradMiddlePos_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradStopPos_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowHoriz_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowVert_valueChanged(int val)
{
	_SetChangedConfig();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowBlur_valueChanged(int val)
{
	_SetChangedConfig();
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbBorderWidth_valueChanged(int val)
{
	if (_busy)
		return;

	if (!ui.chkImageBorder->isChecked())
		return;
	config.imageBorder.changed = config.designChanged = true;
	StyleHandler shButton(ui.btnBorderColor->styleSheet()),
		         shImage(ui.btnImage->styleSheet());
	shImage.SetItem("QToolButton", "border", QString("%1px solid ").arg(ui.sbBorderWidth->value()) + shButton.GetItem("QToolButton", "background-color"));
	ui.btnImage->setStyleSheet(shImage.StyleSheet());
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbTextOpacity_valueChanged(int val)
{
	_OpacityChanged(val, 1);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbBackgroundOpacity_valueChanged(int val)
{
	_OpacityChanged(val, 2);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmOpacity_valueChanged(int val)
{
	config.waterMark.wm.opacity = val;
	config.changed = config.waterMark.changed = true;

}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//void FalconG::on_btnLink_clicked()
//{
//}

/*============================================================================
* TASK:	Set parameters of sample (web) button clicked into common controls 
*			from 'config'
* EXPECTS:	'config' is OK
*			shElem - StyleHandler for 'pb'
*			pb	   - pointer to WEB button
*			elem   - elem in config (source of data)
* GLOBALS: config
* REMARKS: changes shElem
*			does not change 'config'
*--------------------------------------------------------------------------*/
void FalconG::_SetCommonControlsFromWebButton(StyleHandler &shElem, QWidget * pb, _CElem & elem)
{
	StyleHandler
		shColor(ui.btnColor->styleSheet()),				// common ui elements	
		shBackground(ui.btnBackground->styleSheet()),	//						
		shShadow(ui.btnShadowColor->styleSheet());		//						

	shElem.Set(pb->styleSheet());
	
	shColor.SetItem("QToolButton", "background-color", elem.color.name); 
	ui.btnColor->setStyleSheet(shColor.StyleSheet()); 
	
	shBackground.SetItem("QToolButton", "background-color", elem.background.name);
	ui.btnBackground->setStyleSheet(shBackground.StyleSheet()); 
	
	if(elem.shadow.useEver &&elem.shadow.use)
		shShadow.SetItem("QToolButton", "background-color", '#' + elem.shadow.color);

	ui.edtTextColor->setText(elem.color); 
	ui.edtBackgroundColor->setText(elem.background); 

	ui.gbTextShadow->setEnabled(true); 
	ui.chkShadowOn->setChecked(elem.shadow.useEver & elem.shadow.use); 
	ui.btnShadowColor->setStyleSheet(shShadow.StyleSheet()); 
	
	ui.sbShadowHoriz->setValue(elem.shadow.Horiz()); 
	ui.sbShadowVert->setValue(elem.shadow.Vert()); 
	ui.sbShadowBlur->setValue(elem.shadow.Blur()); 
	
	ui.chkTextOpacity->setChecked(elem.color.opacity > 0); 
	ui.chkBackgroundOpacity->setChecked(elem.background.opacity > 0); 
	ui.sbTextOpacity->setValue(abs(elem.color.opacity)); 
	ui.sbBackgroundOpacity->setValue(abs(elem.background.opacity)); 
	
	_EnableGbMenu(0);		// backup changed colors into static variables  and disable buttons

	ui.edtFontFamily->setText(elem.font.family); 
	ui.cbPointSize->setCurrentText(elem.font.sizestr); 
	ui.chkBold->setChecked(elem.font.features & _CFont::fBold); 
	ui.chkItalic->setChecked(elem.font.features & _CFont::fItalic); 
	ui.chkUnderline->setChecked(elem.font.features & _CFont::fUnderline); 
	ui.chkStrikethrough->setChecked(elem.font.features & _CFont::fStrikethrough);  
	
	ui.chkGradient->setChecked(elem.gradient.used);
}

/*============================================================================
  * TASK: set colors, opacities, fonts etc for any web element to 
  *			common UI controls from 'config'
  * EXPECTS: config is up to date with all data
  * GLOBALS:	 config
  * REMARKS:does not modify 'config'
*--------------------------------------------------------------------------*/
void FalconG::_WebColorButtonClicked()
{
	QString s, ss;

	++_busy;				// do not modify 'config' or set values back to button pressed
	StyleHandler shElem,
		shColor(ui.btnColor->styleSheet()),				// common ui element	
		shBackground(ui.btnBackground->styleSheet()),	//						
		shShadow(ui.btnShadowColor->styleSheet());		//						
	// macro to set all UI parameters from 'config'
	// button: name of button w.o. the 'btn'
	// name  : name of the configuration element from which the params are set
#define SET_ELEM_PARAMETERS(button, name) \
	_SetCommonControlsFromWebButton(shElem, ui.btn##button, config.name);
	//------------
	switch (_aeActiveElement)
	{
		case aeWebPage:   
			_SetCommonControlsFromWebButton(shElem, ui.frmWeb, config.Web);
			break;

		case aeMenuButtons:
			SET_ELEM_PARAMETERS(Menu, Menu)
			ui.edtGradStartColor->setText(config.Menu.gradient.gs[0].color);
			ui.edtGradMiddleColor->setText(config.Menu.gradient.gs[1].color);
			ui.edtGradStopColor->setText(config.Menu.gradient.gs[2].color);
			ui.btnGradStartColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[0].color + ";}");
			ui.btnGradMiddleColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[1].color + ";}");
			ui.btnGradStopColor->setStyleSheet("QToolButton { background-color:#" + config.Menu.gradient.gs[2].color + ";}");
// DEBUG
// ShowStyleOf(ui.btnGradStopColor);
			ui.sbGradStartPos->setValue(config.Menu.gradient.gs[0].percent);
			ui.sbGradMiddlePos->setValue(config.Menu.gradient.gs[1].percent);
			ui.sbGradStopPos->setValue(config.Menu.gradient.gs[2].percent);
			ui.chkGradient->setChecked(config.Menu.gradient.used);
			_EnableGbMenu(1);		// backup changed colors into static variables  and enable buttons
			break;
		case aeGalleryTitle:
				// color select buttons
			SET_ELEM_PARAMETERS(GalleryTitle, GalleryTitle)
			break;
		case aeSmallGalleryTitle:
				// color select buttons
			SET_ELEM_PARAMETERS(SmallGalleryTitle, SmallGalleryTitle)
			break;
		case aeGalleryDesc:
			// color select buttons
			SET_ELEM_PARAMETERS(GalleryDesc, GalleryDesc)
				break;
		case aeLangButton:
			// color select buttons
			SET_ELEM_PARAMETERS(Lang, Lang)
				break;
		case aeAlbumTitle:
			SET_ELEM_PARAMETERS(AlbumTitle, AlbumTitle)
			break;
		case aeAlbumDesc:
			SET_ELEM_PARAMETERS(AlbumDesc, AlbumDesc)
				break;
		case aeSection:
			SET_ELEM_PARAMETERS(Section, Section)
				break;
		case aeImageTitle:
			SET_ELEM_PARAMETERS(ImageTitle, ImageTitle)
			break;
		case aeImageDesc:
			SET_ELEM_PARAMETERS(ImageDesc, ImageDesc)
				break;
	}
// DEBUG 1 line
//	ShowStyleOf(ui.btnColor);
	--_busy;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_WebBackgroundClicked()
{
	_aeActiveElement = aeWebPage;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGalleryTitle_clicked()
{
	_aeActiveElement = aeGalleryTitle;
	_WebColorButtonClicked();
// DEBUG
//	ShowStyleOf(ui.pnlGallery);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGalleryDesc_clicked()
{
	_aeActiveElement = aeGalleryDesc;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSmallGalleryTitle_clicked()
{
	_aeActiveElement = aeSmallGalleryTitle;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnMenu_clicked()
{
	_aeActiveElement = aeMenuButtons;
// DEBUG
//ShowStyleOf(ui.btnMenu);
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradBorder_clicked()
{
	QColor qc(config.Menu.border.color.name);
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qc.isValid())
	{
		config.Menu.border.color = qc.name().mid(1);
		config.Menu.border.changed = true;
		StyleHandler handler(ui.btnBorderColor->styleSheet());
		handler.SetItem("QToolButton", "background-color", config.Menu.border.color.name);
		QString invc = __ContrastColorString(config.Menu.border.color);
		handler.SetItem("QToolButton", "color", invc);
		ui.btnGradBorder->setStyleSheet(handler.StyleSheet());
		_EnableButtons();
	}
	else
		return;

	if (!ui.chkButtonBorder->isChecked())
		return;

	StyleHandler handler(ui.btnMenu->styleSheet());
	handler.SetItem("QPushButton", "border", "1px solid " + config.Menu.border.color.name);
	handler.SetItem("QPushButton:pressed", "border", "1px solid " + config.Menu.border.color.name);
	ui.btnMenu->setStyleSheet(handler.StyleSheet());
	ui.btnCaption->setStyleSheet(handler.StyleSheet());
	ui.btnUplink->setStyleSheet(handler.StyleSheet());
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnLang_clicked()
{
	_aeActiveElement = aeLangButton;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnUplink_clicked()
{
	_aeActiveElement = aeMenuButtons;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnCaption_clicked()
{
	_aeActiveElement = aeMenuButtons;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnAlbumTitle_clicked()
{
	_aeActiveElement = aeAlbumTitle;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnAlbumDesc_clicked()
{
	_aeActiveElement = aeAlbumDesc;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSection_clicked()
{
	_aeActiveElement = aeSection;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnImageTitle_clicked()
{
	_aeActiveElement = aeImageTitle;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnImageDesc_clicked()
{
	_aeActiveElement = aeImageDesc;
	_WebColorButtonClicked();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSaveConfig_clicked()
{
	config.Write();
}


/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBrowseSource_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Select Source Directory", ui.edtSourceGallery->text().isEmpty() ? "": ui.edtSourceGallery->text());
	if (!s.isEmpty())
		ui.edtSourceGallery->setText(s);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBrowseDestination_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Select Destination Directory", ui.edtDestGallery->text().isEmpty() ? "": ui.edtDestGallery->text());
	if (!s.isEmpty())
		ui.edtDestGallery->setText(s);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnColor_clicked()
{
	if (_busy)
		return;

	QColor qc("#"+ui.edtTextColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qc.isValid())
	{
		ui.edtTextColor->setText(qc.name().mid(1));	// sets config, button color, web button style
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBackground_clicked()
{
	if (_busy)
		return;

	QColor qc("#" + ui.edtBackgroundColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	//	QString s = qc.name().mid(1);
	if (qc.isValid())
	{
		ui.edtBackgroundColor->setText(qc.name().mid(1));
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradStartColor_clicked()
{
	QColor qc("#" + ui.edtGradStartColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qc.isValid())
		ui.edtGradStartColor->setText(qc.name().mid(1));
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradMiddleColor_clicked()
{
	QColor qc("#" + ui.edtGradMiddleColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qc.isValid())
		ui.edtGradMiddleColor->setText(qc.name().mid(1));
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradStopColor_clicked()
{
	QColor qc("#" + ui.edtGradStopColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qc.isValid())
		ui.edtGradStopColor->setText(qc.name().mid(1));
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBorderColor_clicked()
{
	StyleHandler handler(ui.btnBorderColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color"));
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qc.isValid() && ui.chkImageBorder->isChecked())
	{
		handler.SetItem("QToolButton", "background-color", qc.name());
		ui.btnBorderColor->setStyleSheet(handler.StyleSheet());
		config.imageBorder.color = qc.name();
		on_sbBorderWidth_valueChanged(ui.sbBorderWidth->value());
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnShadowColor_clicked()
{
	if (_busy)
		return;

	QColor qc("#" + ui.edtTextColor->text());
	qc = QColorDialog::getColor(qc, this, tr("Select Color"));
	ui.btnShadowColor->setStyleSheet("QToolButton {\nbackground-color:" + qc.name()+";\n}\n");

	QWidget *pb;
	_CElem *pElem;
	StyleHandler handler;
	QString qsClass;
	pElem = _SelectSampleElement(pb, qsClass);
	handler.Set(pb->styleSheet());
	pElem->shadow.color = qc.name();
	pElem->shadow.changed = true;

	_SetChangedConfig();
	//handler.SetItem(qsClass, "text-shadow", ")
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnResetDesign_clicked()
{
	config.RestoreDesign();
	_PopulateFromConfig();		
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnDisplayHint_clicked()
{
	QMessageBox(QMessageBox::Information, "falconG - Hint",
				 "<html><head/><body><p>With certain user interface themes the color of some elements"
				 " on the right may differ from the ones you set on the left.<br>"
				 "A clear indication is a background mismatch between the color of the "
				 "<span style=\" font - style:italic;\">Gallery title</span> area on the top and the "
				 "main page area below it.</p></body></html>", QMessageBox::Ok, this).exec();
}

/*============================================================================
* TASK: texts for albums and images into into edtText and edtBaseText
* EXPECTS: 	'who'	- wctLangCombo or wctLangCombo: with no new album or image ID
					  wctSelection	('_AlbumStructureSelectionChanged' or
*										'_TnvSelectionChanged')
*			'_selection'  -  is set up with actual and new album 
*							 and image IDs
* GLOBALS:	_selection'
* REMARKS:	- if 'who' == wctLangCombo or wctBaseLangCombo: 
*					both 'newAlbum' and 'newImage' is 0
*					in this case save changed text (if any) and change language.
*					(if no actual album or image all edit boxes are cleared)
*			- if 'who' == wctSelection and 
*					if 'newAlbum' is not 0 then the source is
*						'_AlbumStructureSelectionChanged'
*						in this case no image is selected, set texts 
*						for the new album and set actAlbum = newAlbum
*					if 'newImage' is not 0 then the source is
*						'_TnvSelectionChanged'
*						in this case set texts for the selected image and 
*						_selectedImage = 'newImage' 'actImage
*					if both 'newAlbum' and 'newImage' is 0 then clear editors
*						and set actAlbum = a_selectedImage = 0
*--------------------------------------------------------------------------*/
void FalconG::_GetTextsForEditing(whoChangedTheText who)
{
	if (who == wctSelection && _selection.newImage == ID_t(-1))	// image selection cleared
	{
		_selection.newImage = 0;
		_selection.newAlbum = _selection.actAlbum;
	}

	bool readItFromDisk = _selection.newAlbum || _selection.newImage;

	++_busy;						// semaphore
	if (who == wctSelection && !readItFromDisk)	   // selection cleared: clear editing
	{
		ui.edtBaseTitle->clear();
		ui.edtBaseDescription->clear();
		ui.edtTitleText->clear();
		ui.edtDescriptionText->clear();

		_selection.actAlbum = _selection.selectedImage = 0;
		--_busy;
		return;
	}

	// indices for base and actual languages
	int bli = ui.cmbBaseLanguage->currentIndex(),	// text(s) in this language is the base
		li = ui.cmbLanguage->currentIndex();		// text(s) in this language may have been changed
	
	if (who == wctBaseLangCombo)		// then just change base text for given language
	{
		if (bli == li)								// then base text was changed to be the same as
			_SaveChangedTexts();					// actual text, which may be changed
		ui.edtBaseTitle->setPlainText(DecodeLF(_selection.title[bli]));
		ui.edtBaseDescription->setPlainText(DecodeLF(_selection.description[bli]));
		--_busy;
		return;
	}
	else if (who == wctLangCombo)		// then just change base text for given language
	{
		_SaveChangedTexts();					// actual text, which may be changed
		ui.edtTitleText->setPlainText(DecodeLF(_selection.title[li]));
		ui.edtDescriptionText->setPlainText(DecodeLF(_selection.description[li]));
		--_busy;
		return;
	}

		// must be wctSelection with either newAlbum ~= 0 or newImage != 0

	_SaveChangedTexts();				// for actAlbum/actImage but only when needed

	ID_t titleID = _selection.title.ID, 		// already the new IDs
		  descID = _selection.description.ID;

	Album *pActAlbum = nullptr;
	Image *pActImage = nullptr;
	if (_selection.newAlbum)	// then not new image -> get texts for album
	{
		pActAlbum = &albumgen.Albums()[_selection.newAlbum];
		titleID = pActAlbum->titleID;
		descID = pActAlbum->descID;
	}
	else if (_selection.newImage)
	{
		pActImage = &albumgen.Images()[_selection.newImage];
		titleID = pActImage->titleID;
		descID = pActImage->descID;
	}
	_selection.title.Clear();			// get rid of old text
	_selection.description.Clear();		// and get new texts

	if(titleID)
		_selection.title = albumgen.Texts()[titleID];		
	if(descID)
		_selection.description = albumgen.Texts()[descID];
			// set texts into edit boxes
	ui.edtBaseTitle->setPlainText(DecodeLF(_selection.title[bli]));
	ui.edtBaseDescription->setPlainText(DecodeLF(_selection.description[bli]));
	ui.edtTitleText->setPlainText(DecodeLF(_selection.title[li]));
	ui.edtDescriptionText->setPlainText(DecodeLF(_selection.description[li]));

	if(_selection.newAlbum || !_selection.newImage) // if no new image
		_selection.actAlbum = _selection.newAlbum;
		

	_selection.selectedImage = _selection.newImage;

	_selection.newAlbum = _selection.newImage = 0;
	--_busy;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
bool FalconG::eventFilter(QObject * o, QEvent * e)
{
	if (o == ui.frmWeb)
	{
		if (e->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent *me = (QMouseEvent*)e;
			if (me->button() == Qt::LeftButton)
				_WebBackgroundClicked();
		}
	}
	return false;
}

/*=============================================================
 * TASK:	   slot to handle album selection changes in 
 *				album tree view and load album title and 
 *				description texts into edit fields
 * EXPECTS:	current:  to this model index
 *			previous: from this model index
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_AlbumStructureSelectionChanged(const QItemSelection &current, const QItemSelection &previous)
{
	_SaveChangedTexts();

	ui.tnvImages->thumbNames.clear();
	ui.tnvImages->originalPaths.clear();
	ui.tnvImages->SetImageList(nullptr);

	int n = current.indexes().size();
	if (n == 1)		 // single selection and valid
	{
		// collect its images into list
		ui.tnvImages->thumbsDir = (config.dsGallery + config.dsGRoot + config.dsImageDir).ToString();

		ID_t id = ID_t(current.indexes()[0].internalPointer());	// store ID of last selected album

		_selection.newAlbum = id;	// only single selection is used
		_GetTextsForEditing(wctSelection);	 // reads new text into _currentTexts and _originalTexts for album
											 // before the image list is read
		// read image list
		IdList &idl = albumgen.Albums()[id].images;						
		ImageMap &im = albumgen.Images();

		for (int i = 0; i < idl.size(); ++i)
		{
			ui.tnvImages->thumbNames.push_back(im[idl[i]].LinkName());
			ui.tnvImages->originalPaths.push_back(im[idl[i]].FullName());
		}
		ui.tnvImages->SetImageList(&idl);
	}
	else
	{
		_selection.newAlbum = 0;			// nothing/multiple albums selected
		_GetTextsForEditing(wctSelection);	 // reads new text into _currentTexts and _originalTexts for album
	}

	ui.tnvImages->reLoad();
	_selection.selectedImage = _selection.newImage = 0;

}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: saves changed texts
*--------------------------------------------------------------------------*/
void FalconG::on_cbWmHPosition_currentIndexChanged(int index)
{
	if (_busy)
		return;
	config.waterMark.wm.origin &= 0xF;
	config.waterMark.wm.origin += index << 4;
	config.changed = config.waterMark.changed = true;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: saves changed texts
*--------------------------------------------------------------------------*/
void FalconG::on_cbWmVPosition_currentIndexChanged(int index)
{
	if (_busy)
		return;
	config.waterMark.wm.origin &= 0xF0;
	config.waterMark.wm.origin += index;
	config.changed = config.waterMark.changed = true;
}

/*=============================================================
 * TASK:		 language 1 index changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_cmbBaseLanguage_currentIndexChanged(int index)
{
	_GetTextsForEditing(wctBaseLangCombo);	// saves changes then get text for the other language
	_selection.baseLang = index;
}

/*=============================================================
 * TASK:		language index #2 changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_cmbLanguage_currentIndexChanged(int index)
{
	_GetTextsForEditing(wctLangCombo);	// saves changes then get text for the other language
	_selection.edtLang = index;
}

/*=============================================================
* TASK:	 title text for language 2 is changed
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
void FalconG::on_edtTitleText_textChanged()
{
	if (_busy)
		return;
	++_busy;
	_selection.changed |= FalconGEditTabSelection::fsTitle;
	if (ui.cmbBaseLanguage->currentIndex() == ui.cmbLanguage->currentIndex())
		ui.edtBaseTitle->setPlainText(ui.edtTitleText->toPlainText());
	ui.btnSaveChangedTitle->setEnabled(true);
	--_busy;
}

/*=============================================================
* TASK:	decsription text for language 2 changed
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
void FalconG::on_edtDescriptionText_textChanged()
{
	if (_busy)
		return;
	++_busy;
	_selection.changed |= FalconGEditTabSelection::fsDescription;
	if (ui.cmbBaseLanguage->currentIndex() == ui.cmbLanguage->currentIndex())
		ui.edtBaseDescription->setPlainText(ui.edtDescriptionText->toPlainText());
	ui.btnSaveChangedDescription->setEnabled(true);
	--_busy;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnPreview_clicked()
{
	QString qs = "file:///" +
		config.dsGallery.ToString() +
		AlbumGenerator::RootNameFromBase(config.homeLink, 0, false);
	if (!QDesktopServices::openUrl(QUrl(qs)))
	{
		QMessageBox(QMessageBox::Warning, QMainWindow::tr("falconG - Warning"),
						QMainWindow::tr("Cannot open\n'%1'").arg(qs),
							QMessageBox::Ok, frmMain).exec();
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString FalconG::_FontStyle(_CElem & elem)
{
	QString ss = "\n font-size: " + elem.font.sizestr;
	if (elem.font.bold)
		ss += ";\n font-weight: 800";
	if (elem.font.underline || elem.font.strikethrough)
	{
		ss += ";\n text-decoration: ";
		if (elem.font.underline)
			ss += " underline";
		else if (elem.font.strikethrough)
			ss += " strikethrough";
	}
	if (elem.font.italic)
		ss += ";\nfont-style:italic;";
	return ss + ';';
}

/*============================================================================
* TASK:		Creates QT stylesheet gradinet string
* EXPECTS:	elem - config element  (e.g. config.Menu)
*			invert - revese color order
* GLOBALS: nothing
* REMARKS:	 format: qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a90329, stop: 0.4  #8f0222, stop:1 #6d0019);
*--------------------------------------------------------------------------*/
QString FalconG::_GradientStyleQt(_CElem &elem, bool invert)
{
	QString s;
	if (!elem.gradient.used)
		s = '#' + elem.background;
	else if (invert)
		s = QString("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:%1 #%2, stop:%3 #%4 stop:%5 #%6)") 
				.arg(elem.gradient.gs[0].percent / 100.0).arg(elem.gradient.gs[2].color)
				.arg(elem.gradient.gs[1].percent / 100.0).arg(elem.gradient.gs[1].color)
				.arg(elem.gradient.gs[2].percent / 100.0).arg(elem.gradient.gs[0].color);
	else
		s = QString("qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:%1 #%2, stop:%3 #%4 stop:%5 #%6)") 
				.arg(elem.gradient.gs[0].percent / 100.0).arg(elem.gradient.gs[0].color)
				.arg(elem.gradient.gs[1].percent / 100.0).arg(elem.gradient.gs[1].color)
				.arg(elem.gradient.gs[2].percent / 100.0).arg(elem.gradient.gs[2].color);
	return s;
}

/*=============================================================
 * TASK:	Enable/disable gradient  edits, buttons, and positions
 * EXPECTS:	ena: 0: save and disable, 
 *				 1: save parameters and enable, 
 *				 2: restore parameters and enable
 * GLOBALS:
 * RETURNS:
 * REMARKS:	does not change config
 *------------------------------------------------------------*/
void FalconG::_EnableGbMenu(int ena)
{
	static QString ct, cm, cb;		// colors for buttons
	const QString ss = "QToolButton {background-color:#aaa;}";
	const QString css = "QToolButton {background-color:#";
	++_busy;
	if (ena != 2)		  // save
	{
		ct = ui.edtGradStartColor->text();
		cm = ui.edtGradMiddleColor->text();
		cb = ui.edtGradStopColor->text();
		if (!ena)		  // set disabled colors
		{
			ui.btnGradStartColor->setStyleSheet(ss);
			ui.btnGradMiddleColor->setStyleSheet(ss);
			ui.btnGradStopColor->setStyleSheet(ss);
		}
	}
	else				  // restore
	{
		ui.edtGradStartColor-> setText(ct);
		ui.edtGradMiddleColor->setText(cm);
		ui.edtGradStopColor->setText(cb);

		ui.btnGradStartColor->setStyleSheet(css + ct + ";}");
		ui.btnGradMiddleColor->setStyleSheet(css + cm + ";}");
		ui.btnGradStopColor->setStyleSheet(css + cb + ";}");
	}

	ui.btnGradStartColor->setEnabled(ena);
	ui.btnGradMiddleColor->setEnabled(ena);
	ui.btnGradStopColor->setEnabled(ena);

	ui.edtGradStartColor->setEnabled(ena);
	ui.edtGradMiddleColor->setEnabled(ena);
	ui.edtGradStopColor->setEnabled(ena);

	ui.sbGradStartPos->setEnabled(ena);
	ui.sbGradMiddlePos->setEnabled(ena);
	ui.sbGradStopPos->setEnabled(ena);
	--_busy;
}

// ***** MACRO ***********
/*============================================================================
  * TASK:set color & background opacities into config name: _CElem in config,
  * EXPECTS: elem - in config (e.g. config.Web)
  *			 which - 1: text, 2: background 3: both
  * GLOBALS: ui element
  * REMARKS: only modifies config when opacity is changed
 *--------------------------------------------------------------------------*/
void FalconG::_SetOpacityToConfig(_CElem & elem, int which)
{
	int val;
	if (which & 2)
	{
		val = (ui.chkBackgroundOpacity->isChecked() ? 1 : -1)*ui.sbBackgroundOpacity->value();
		if (elem.background.opacity != val)
		{
			elem.background.opacity = val;
			elem.background.changed = true;
		}
	}
	if(!which  & 1)
	{
		val = (ui.chkTextOpacity->isChecked() ? 1 : -1)*ui.sbTextOpacity->value();
		if (elem.color.opacity != val)
		{
			elem.color.opacity = val;
			elem.color.changed = true;
		}
	}
}

/*============================================================================
* TASK:	set	 color into config and the web element and sets config.designChanged
* EXPECTS: 
* GLOBALS: 'config'
* REMARKS: called when global controls changed
*--------------------------------------------------------------------------*/
void FalconG::_SetChangedConfig()
{
	if (_busy)
		return;

	++_busy;
	config.changed = config.designChanged = true;
	_EnableButtons();

	_SetWebColor();
	--_busy;
}

/*============================================================================
* TASK:	
* EXPECTS:
* GLOBALS: 'config'
* REMARKS:
*--------------------------------------------------------------------------*/
_CElem *FalconG::_SelectSampleElement(QWidget *&pb, QString &name)
{
	_CElem *pElem = nullptr;
	name = "QPushButton";
	switch (_aeActiveElement)
	{
		case aeWebPage:   pElem = &config.Web;
			pb = ui.frmWeb;
			name = "QFrame";
			break;
		case aeGalleryTitle:	pElem = &config.GalleryTitle;
			pb = ui.btnGalleryTitle;
			break;
		case aeSmallGalleryTitle: pElem = &config.SmallGalleryTitle;
			pb = ui.btnSmallGalleryTitle;
			break;
		case aeGalleryDesc:		pElem = &config.GalleryDesc;
			pb = ui.btnGalleryDesc;
			break;
		case aeLangButton:		pElem = &config.Lang;
			pb = ui.btnLang;
			break;
		case aeMenuButtons:		pElem = &config.Menu;
			pb = ui.btnMenu;
			break;
		case aeAlbumTitle:		pElem = &config.AlbumTitle;
			pb = ui.btnAlbumTitle;
			break;
		case aeAlbumDesc:		pElem = &config.AlbumDesc;
			pb = ui.btnAlbumDesc;
			break;
		case aeSection:			pElem = &config.Section;
			pb = ui.btnSection;
// DEBUG
//QString sdbg;
//sdbg = ui.btnSection->styleSheet();
			break;
		case aeImageTitle:		pElem = &config.ImageTitle;
			pb = ui.btnImageTitle;
			break;
		case aeImageDesc:		pElem = &config.ImageDesc;
			pb = ui.btnImageDesc;
			break;
	}
	return pElem;
}

/*============================================================================
  * TASK:	store changed opacity for text or background
  * EXPECTS: val - value of new opacity in percent
  *			 which: 1 -> text, 2: background	
  * GLOBALS:
  * REMARKS: val is not used
 *--------------------------------------------------------------------------*/
void FalconG::_OpacityChanged(int val, int which)
{
	QString ss;
	StyleHandler handler;
	QWidget *pb;
	_CElem *pElem;

	QString qsClass;
	pElem = _SelectSampleElement(pb, qsClass);
		;
	_SetOpacityToConfig(*pElem, which);
	handler.Set(pb->styleSheet());
	ss = ColorToStr(which == 1 ? pElem->color : pElem->background);
	handler.SetItem(qsClass, (which == 1 ? "color" : "background-color:"), ss);
	pb->setStyleSheet(handler.StyleSheet());

	switch (_aeActiveElement)
	{
		case aeWebPage:
			handler.Set(ui.pnlGallery->styleSheet());
			handler.SetItem("QWidget", (which == 1 ? "color" : "background-color:"), ss);
			ui.pnlGallery->setStyleSheet(handler.StyleSheet());
			break;
		case aeMenuButtons:    pElem = &config.Menu;
			ui.btnMenu->setStyleSheet(handler.StyleSheet());
			ui.btnUplink->setStyleSheet(handler.StyleSheet());
			ui.btnCaption->setStyleSheet(handler.StyleSheet());
			break;
		case aeLangButton: pElem = &config.Lang;
			ui.btnLang->setStyleSheet(handler.StyleSheet());
			break;
		default: break;
	}
}

/*============================================================================
  * TASK:		set configuration from the common controls
  * EXPECTS:	handler - style handler with actual settings, elem: _CElem from 'config'
  * GLOBALS:	config
  * REMARKS:	may change stylesheet data
 *--------------------------------------------------------------------------*/
void FalconG::_SetColorToConfig(StyleHandler &handler, _CElem &elem)
{
	int pmc1 = ui.chkTextOpacity->isChecked() ? 1 : -1,
		pmb1 = ui.chkBackgroundOpacity->isChecked() ? 1 : -1;
	QString qsClass;
	qsClass = elem.nameStr == "Web" ? "QFrame" : "QPushButton";


	if (elem.color != ui.edtTextColor->text() || elem.color.opacity != pmc1 * ui.sbTextOpacity->value()) 
	{	
		elem.color = ui.edtTextColor->text();	
		elem.color.opacity = pmc1 *ui.sbTextOpacity->value(); 
		elem.color.changed = true; 
		ui.btnSaveConfig->setEnabled(config.changed == true);  
		handler.SetItem(qsClass, "color", ColorToStr(elem.color));
	}	
	if (elem.background != ui.edtBackgroundColor->text() || elem.background.opacity != pmb1 *ui.sbTextOpacity->value()) 
	{	
		elem.background = ui.edtBackgroundColor->text();	
		elem.background.opacity = pmb1 * ui.sbBackgroundOpacity->value(); 
		elem.background.changed = true; 
		handler.SetItem(qsClass, "background-color", ColorToStr(elem.background));
		ui.btnSaveConfig->setEnabled(config.changed == true);
	}
	if(elem.gradient.useEver && elem.gradient.changed)
	{
		if (elem.gradient.gs[0].color != ui.edtGradStartColor->text())
			elem.gradient.gs[0].color = ui.edtGradStartColor->text();
		if (elem.gradient.gs[1].color != ui.edtGradMiddleColor->text())
			elem.gradient.gs[1].color = ui.edtGradMiddleColor->text();
		if (elem.gradient.gs[2].color != ui.edtGradStopColor->text())	// same as gradient border color
			elem.gradient.gs[2].color = ui.edtGradStopColor->text();
		if (elem.gradient.gs[0].percent != ui.sbGradStartPos->value())
			elem.gradient.gs[0].percent = ui.sbGradStartPos->value();
		if (elem.gradient.gs[1].percent != ui.sbGradMiddlePos->value())
			elem.gradient.gs[1].percent = ui.sbGradMiddlePos->value();
		if (elem.gradient.gs[2].percent != ui.sbGradStopPos->value())
			elem.gradient.gs[2].percent = ui.sbGradStopPos->value();

		if (elem.gradient.used)
		{
			handler.RemoveItem("QPushButton", "background-color");
			handler.RemoveItem("QPushButton:pressed", "background-color");
			handler.SetItem(qsClass, "background", _GradientStyleQt(elem, false));
			handler.SetItem("QPushButton:pressed", "background", _GradientStyleQt(elem, true));
		}
		else
		{
			handler.RemoveItem("QPushButton", "background");
			handler.RemoveItem("QPushButton:pressed", "background");
		}
		if (elem.border.changed)
		{
			if (elem.border.used != ui.chkButtonBorder->isChecked())
				elem.border.used = ui.chkButtonBorder->isChecked();
			if (elem.border.used)
			{
				handler.SetItem(qsClass, "border", "1px solid " + elem.border.color.name);
				handler.SetItem("QPushButton:pressed", "border", "1px solid %1" + elem.border.color.name);
			}
			else
			{
				handler.RemoveItem("QPushButton", "border");
				handler.RemoveItem("QPushButton:pressed", "border");
			}
		}
	}
	_EnableButtons();
}

/*============================================================================
  * TASK: sets colors and opacities from the common controls into config and 
  *			the web element on the UI
  * EXPECTS: 
  * GLOBALS:	 config
  * REMARKS: sets both color and background and if gradient is specified also
  *				sets background as a gradient
 *--------------------------------------------------------------------------*/
void FalconG::_SetWebColor()
{
	StyleHandler handler;
	QString ss;
	switch (_aeActiveElement)
	{
		case aeWebPage:   handler.Set(ui.frmWeb->styleSheet());
			_SetColorToConfig(handler, config.Web);		// and into 'handler'
			ss = handler.StyleSheet();
			ui.frmWeb->setStyleSheet(ss); 
			ss = "QWidget {" + ss.mid(11);
			ui.pnlGallery->setStyleSheet(ss);
// DEBUG
//ShowStyleOf(ui.pnlGallery); 	
			break;

		case aeGalleryTitle:   handler.Set(ui.btnGalleryTitle->styleSheet());
			_SetColorToConfig(handler, config.GalleryTitle);
			ui.btnGalleryTitle->setStyleSheet(handler.StyleSheet());
				break;
		case aeGalleryDesc:   handler.Set(ui.btnGalleryDesc->styleSheet());
			_SetColorToConfig(handler, config.GalleryDesc);
			ui.btnGalleryDesc->setStyleSheet(handler.StyleSheet());
				break;
		case aeLangButton:   handler.Set(ui.btnLang->styleSheet());
			_SetColorToConfig(handler, config.Lang);
			ui.btnLang->setStyleSheet(handler.StyleSheet());
				break;
		case aeSmallGalleryTitle:   handler.Set(ui.btnSmallGalleryTitle->styleSheet());
			_SetColorToConfig(handler, config.SmallGalleryTitle);
			ui.btnSmallGalleryTitle->setStyleSheet(handler.StyleSheet());
				break;
		case aeMenuButtons:    handler.Set(ui.btnMenu->styleSheet());
			_SetColorToConfig(handler, config.Menu);
			ss = handler.StyleSheet();
			ui.btnMenu->setStyleSheet(ss);
// DEBUG
// ShowStyleOf(ui.btnMenu); 	
//			ui.btnLang->setStyleSheet(ss);
			ui.btnUplink->setStyleSheet(ss);
			ui.btnCaption->setStyleSheet(ss);
			break;
		case aeAlbumTitle:	   handler.Set(ui.btnAlbumTitle->styleSheet());
			_SetColorToConfig(handler, config.AlbumTitle);
			ui.btnAlbumTitle->setStyleSheet(handler.StyleSheet());
			break;
		case aeAlbumDesc:	   handler.Set(ui.btnAlbumDesc->styleSheet());
			_SetColorToConfig(handler, config.AlbumDesc);
			ui.btnAlbumDesc->setStyleSheet(handler.StyleSheet());
			break;
		case aeSection:	   handler.Set(ui.btnSection->styleSheet());
			_SetColorToConfig(handler, config.Section);
			ui.btnSection->setStyleSheet(handler.StyleSheet());
			break;
		case aeImageTitle: 	   handler.Set(ui.btnImageTitle->styleSheet());
			_SetColorToConfig(handler, config.ImageTitle);
			ui.btnImageTitle->setStyleSheet(handler.StyleSheet());
			break;
		case aeImageDesc:	   handler.Set(ui.btnImageDesc->styleSheet());
			_SetColorToConfig(handler, config.ImageDesc);
			ui.btnImageDesc->setStyleSheet(handler.StyleSheet());
			break;
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_SetupWebButtonFromConfig()
{
	_CElem * from;
	QWidget * pb;
	// using _aeActiveElement

	QString qsClass;
	from = _SelectSampleElement(pb, qsClass);
	StyleHandler handler(pb->styleSheet());
			// colors
	if (!from->color.name.isEmpty() && (from->color.name != handler.GetItem("QPushButton", "color")))
		handler.SetItem(qsClass, "color", from->color);
	if (from->color.opacity)
		handler.SetItem(qsClass, "color", ColorToStr(from->color));	// also uses opacity
	if (!from->background.name.isEmpty() && (from->background.name != handler.GetItem("QPushButton", "background-color")))
		handler.SetItem(qsClass, "background-color", from->background);
	if (from->background.opacity)
		handler.SetItem(qsClass, "color", ColorToStr(from->color)); // also uses opacity
			// shadow
	if (from->shadow.use)
	{
		if (from->shadow.value)
			handler.SetItem(qsClass, "text-shadow",
				QString("%1 %2 %3 %4").arg(from->shadow.values[0]).
				arg(from->shadow.values[1]).
				arg(from->shadow.values[2]).
				arg(from->shadow.values[3]));
		else
			handler.RemoveItem("QPushButton", "text-shadow");
	}
	else
		handler.RemoveItem("QPushButton", "text-shadow");
			// gradient
	if (from->gradient.used)
	{
		handler.SetItem(qsClass, "background", _GradientStyleQt(*from));				// unpressed
		handler.SetItem("QPushButton:pressed", "background", _GradientStyleQt(*from, true));	// pressed
		if (from->border.used)
		{
			handler.SetItem(qsClass, "border", QString("1px solid %1").arg(from->border.color.name) );
			handler.SetItem("QPushButton:pressed", "border", QString("1px solid %1").arg(from->border.color.name));
		}
	}
	else
		handler.RemoveGroup("QPushButton:pressed");

	// font
	if(!from->font.family.isEmpty() && from->font.family != handler.GetItem("QPushButton", "font-family"))
		handler.SetItem(qsClass, "font-family", from->font.family);
	if (!from->font.sizestr.isEmpty() && from->font.sizestr != handler.GetItem("QPushButton", "font-size"))
		handler.SetItem(qsClass, "font-size", from->font.sizestr);
	QString sDecor = handler.GetItem("QPushButton", "text-decoration");

	unsigned char  bold = handler.GetItem("QPushButton", "font-weight").isEmpty() ? 0 : 1,
				   italic = handler.GetItem("QPushButton", "font-style").isEmpty() ? 0 : 1,
				   underline = sDecor.indexOf("underline") < 0  ? 0 : 1,
				   strikethrough = sDecor.indexOf("strikethrough") < 0 ? 0 : 1;
	if ((from->font.bold & bold) == 0) // one of them is not set
		if (from->font.bold == 0)
			handler.RemoveItem("QPushButton", "font-weight");
		else
			handler.SetItem(qsClass, "font-weight", "bold");
	if ((from->font.italic & italic) == 0) // one of them is not set
		if (from->font.italic == 0)
			handler.RemoveItem("QPushButton", "font-style");
		else
			handler.SetItem(qsClass, "font-style", "italic");
	sDecor.clear();
	if ((from->font.underline & underline) == 0) // one of them is not set
		if (from->font.underline != 0)
			sDecor += "underline ";
	if ((from->font.strikethrough & strikethrough) == 0) // one of them is not set
		if (from->font.strikethrough != 0)
			sDecor += "strikethrough";
	if(sDecor.isEmpty())
		handler.RemoveItem("QPushButton", "text-decoration");
	else
	{
		if (sDecor[sDecor.length() - 1] == ' ')
			sDecor.remove(sDecor.length() - 1);
		handler.SetItem(qsClass, "text-decoration", "underline");
	}
}

/*============================================================================
* TASK:	   Saves changed title and/or description
* EXPECTS: 
* GLOBALS: 
* REMARKS: MUST be called BEFORE _selection.actAlbum and selectedImage is changed
*--------------------------------------------------------------------------*/
void FalconG::_SaveChangedTexts()
{
	if (!_selection.changed)
		return;

	while (!albumgen.StructWritten())		// before changing again wait for previous write to finish
		;
// NO only _edited when structure changed and not yet written out	_edited = true;							// reset to false only when reading new album struct

	// get changed text into _selection
	ID_t origTitleID = _selection.title.ID,
		 origDescID = _selection.description.ID;

	if (_selection.changed & FalconGEditTabSelection::fsTitle)
	{
		_selection.title.SetTextForLanguageNoID(ui.edtTitleText->document()->toPlainText(), _selection.edtLang);
		_selection.title.CalcBaseID();		// new base ID for title
	}
	if (_selection.changed & FalconGEditTabSelection::fsDescription)
	{
		_selection.description.SetTextForLanguageNoID(ui.edtDescriptionText->document()->toPlainText(), _selection.edtLang);
		_selection.description.CalcBaseID();		// new base ID for description
	}

	bool added = false;
					// save changed texts to old and set texts from current
					// only need to modify album or image when ID is changed
	TextMap &tmap = albumgen.Texts();
	// first for title
					// but we only know the new base ID not the whole ID yet
	if( (_selection.title.ID == (origTitleID & BASE_ID_MASK)) &&	// if base ID ID is same as old
		origTitleID != 0 && 										// which was for existingh
		(_selection.title == tmap[origTitleID]))		// and the text itself is the same as the old one
				_selection.title.ID = origTitleID;					// then no change and get original ID back
	else if(_selection.title.ID)		// new ID  OR same ID, but new text 
	{			// which may nat have cahnged
		_selection.title.ID = tmap.Add(_selection.title, added); 				// change ID
		tmap.Remove(origTitleID);												// decrement usageCount

		if(_selection.selectedImage)
			albumgen.Images()[_selection.selectedImage].titleID = _selection.title.ID;	// Add new text
		else
			albumgen.Albums()[_selection.actAlbum].titleID = _selection.title.ID;	// Add new text

		if (ui.chkChangeTitleEverywhere->isChecked())
		{						// remove the old text and ID
								// replace the old ID with the new everywhere
			for (auto a : albumgen.Albums())
				if (a.titleID == origTitleID)
					a.titleID = _selection.title.ID, tmap.Remove(origTitleID);
			for (auto a : albumgen.Images())
				if (a.titleID == origTitleID)
					a.titleID = _selection.title.ID, tmap.Remove(origTitleID);
		}
	}
	// then for description
	if( (_selection.description.ID == (origDescID & BASE_ID_MASK))	&&
		origDescID &&
		(_selection.description == tmap[origDescID]))
				_selection.description.ID = origDescID;
	else if(_selection.description.ID)
	{
		_selection.description.ID = tmap.Add(_selection.description, added);	   // change ID
		tmap.Remove(origDescID);												   // decrement usageCount

		if (_selection.selectedImage)
			albumgen.Images()[_selection.selectedImage].descID = _selection.description.ID;	// Add new text
		else
			albumgen.Albums()[_selection.actAlbum].descID = _selection.description.ID; // Add new text

		if (ui.chkChangeDescriptionEverywhere->isChecked())
		{// remove the old text and ID
			tmap.Remove(origDescID);
			// replace the old ID with the new everywhere
			for (auto a : albumgen.Albums())
				if (a.descID == origDescID)
					a.descID = _selection.description.ID, tmap.Remove(origTitleID);
			for (auto a : albumgen.Images())
				if (a.descID == origDescID)
					a.descID = _selection.description.ID, tmap.Remove(origTitleID);
		}
	}

	_selection.changed = 0;				// all changes saved

	albumgen.WriteDirStruct(true);		// keep previous backup file
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_btnSaveStyleSheet_clicked()
{
	albumgen.SaveStyleSheets();
}


/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSelectWmFont_clicked()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, config.waterMark.wm.font, this, "falconG - select Watermark font");
	if (ok && config.waterMark.wm.font != font)
	{
		config.waterMark.wm.SetFont(font);
		config.changed = config.waterMark.changed = true;
		ui.lblWaterMarkFont->setText( QString("%1 (%2,%3)").arg(font.family()).arg(font.pointSize()).arg(font.italic()));
		if(!config.waterMark.wm.text.isEmpty())
			ui.lblWmSample->setText(config.waterMark.wm.text);
		else
			ui.lblWmSample->setText("Watermark sample text");

		ui.lblWmSample->setFont(font);
	}
}


/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnWmColor_clicked()
{
	StyleHandler handler(ui.btnWmColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color"));
	qc = QColorDialog::getColor(qc, this, tr("Select Watermark Color"));

	if (qc.isValid())
	{
		QString s = qc.name();
		handler.SetItem("QToolButton", "background-color", s);
		++_busy;
		ui.btnWmColor->setStyleSheet(handler.StyleSheet());
		s = s.mid(1);
		ui.edtWmColor->setText(s);
		config.waterMark.wm.SetColor(s);
		config.changed = config.waterMark.changed = true;
		--_busy;
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnWmShadowColor_clicked()
{
	StyleHandler handler(ui.btnWmColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color"));
	qc = QColorDialog::getColor(qc, this, tr("Select Watermark Shadow Color"));

	if (qc.isValid())
	{
		handler.SetItem("QToolButton", "background-color", qc.name());
		config.waterMark.wm.shadowColor = qc.name().mid(1).toInt(nullptr, 16);
		config.changed = config.waterMark.changed = true;
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_SetupLanguagesToUI()
{
	ui.cmbBaseLanguage->clear();
	ui.cmbLanguage->clear();
	for (int i = 0; i < Languages::Count(); ++i)
	{			 // edit texts page
		ui.cmbBaseLanguage->addItem(Languages::names[i] + " (" + Languages::abbrev[i] + ")");
		ui.cmbLanguage->addItem(Languages::names[i] + " (" + Languages::abbrev[i] + ")");
	}
	++_busy;
	if (Languages::Count() > 1)
	{
		ui.cmbBaseLanguage->setCurrentIndex(0);
		ui.cmbLanguage->setCurrentIndex(1);
	}
	--_busy;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_SetProgressBar(int minimum, int maximum, int pos, int phase)
{
	_phase = phase;
	ui.progressBar->setMinimum(minimum);
	ui.progressBar->setMaximum(maximum);
	ui.progressBar->setValue(pos);
	if (!phase)
	{
		ui.lblProgressTitle->setText("Phase 1: reading albums and images");
		ui.lblProgressDesc->setText("albums / images");
		ui.lblRemainingTime->setText("");
		ui.label_43->setVisible(false);
		ui.lblImagesPerSec->setVisible(false);
	}
	else if(phase == 1)
	{
		ui.label_43->setVisible(true);
		ui.lblImagesPerSec->setVisible(true);
		ui.lblImagesPerSec->setText("");
		ui.lblRemainingTime->setText("");
		ui.lblProgressTitle->setText("Phase 2: processing images");
		ui.lblProgressDesc->setText("images / total");
	}
	else
	{
		ui.lblProgressTitle->setText("Phase 3: Creating albums");
		ui.lblProgressDesc->setText("albums / total");
		ui.label_43->setVisible(false);
		ui.lblImagesPerSec->setVisible(false);
	}
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
int FalconG::GetProgressBarPos()
{
	return ui.progressBar->value();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::_SetProgressBarPos(int count, int maxCount)
{
	static int cnt = 0;
	if (++cnt == 10)
	{
		QApplication::processEvents();
		cnt = 0;
	}
	ui.lblProgress->setText(QString("%1 / %2").arg(count).arg(maxCount));
	ui.progressBar->setValue(count);
}

/*============================================================================
* TASK:	  show actual and remaining time  and image processing speed images/sec
* EXPECTS: actual: elapsed time, 
*		   total: total time
*		   count: how many images has been processed
*		   speed: should display speed too?
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_ShowRemainingTime(time_t actual, time_t total, int count, bool speed)
{
	ui.lblRemainingTime->setText(TimeToHMSStr(actual) + " / " + TimeToHMSStr(total-actual));
	if(speed)
		ui.lblImagesPerSec->setText(QString("%1").arg((double)count / (double)actual, 0,'f',1));
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_ThumbNailViewerIsLoading(bool yes)
{
	setCursor(yes ? Qt::WaitCursor : Qt::ArrowCursor);
	QApplication::processEvents();
}

void FalconG::_TrvCountChanged()
{
	ui.lblTotalCount->setText(QString("%1 Albums, %2 images").arg(albumgen.Albums().size()).arg(albumgen.Images().size()) );
}

void FalconG::_TnvStatusChanged(QString &s)
{
	ui.lblImageCount->setText(s);
}

/*=============================================================
 * TASK:   slot to receive name of selected thumbnal (in image dir)
 * EXPECTS:	 s: name of image (format: <ID>.<ext> for single image
 *				selection: empty if no selection
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_TnvSelectionChanged(QString s)
{
	_selection.newImage = s.isEmpty() ? ID_t(-1) : s.left(s.lastIndexOf('.')).toULongLong();
	_GetTextsForEditing(wctSelection);

}

/*============================================================================
 * TASK:
 * EXPECTS:
 * RETURNS:
 * GLOBALS:
 * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_TnvNewThumbs()
{
	if (_selection.actAlbum && _selection.selectedImage)
		albumgen.Albums()[_selection.actAlbum].thumbnail = _selection.selectedImage;
}

/*=============================================================
 * TASK:	  save the changed texts: slot for two buttons
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: save buttons are disabled until text changed and will be
 *			disabled after the save
 *------------------------------------------------------------*/
void FalconG::_SaveChangedTitleDescription()
{
	_SaveChangedTexts();
	ui.btnSaveChangedTitle->setEnabled(false);
	ui.btnSaveChangedDescription->setEnabled(false);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//void FalconG::_ImageMapChanged()
//{
//	reinterpret_cast<ImageMapModel*>(ui.lvImages->model())->ModelChanged();
//}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_AlbumMapChanged()
{
	reinterpret_cast<AlbumTreeModel*>(ui.trvAlbums->model())->ModelChanged();
	ui.trvAlbums->expandAll();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_EnableEditTab(bool enable)
{
	ui.tabEdit->setEnabled(enable);
	ui.tnvImages->setEnabled(enable);
}
