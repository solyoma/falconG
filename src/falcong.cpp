#include <QCloseEvent>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QDirIterator>
#include <QListView>
#include <QDesktopServices>
#include <QUrl>

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"
#include "falcong.h"
#include "structEdit.h"
#include "sourcehistory.h"


#define DEBUG_LOG(qs) \
{							 \
QFile f("debug.log");		 \
f.open(QIODevice::Append);	 \
QTextStream ofs(&f);		 \
ofs << qs << "\n";			 \
}


FalconG *frmMain = nullptr;

/*------------------------------------- macros ------------------------------------*/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//static QString __ColorStyle(_CElem &elem)
//{
//	return QString("\n color: ") + ColorToStr(elem.color)
//		+ ";\n background-color: " + ColorToStr(elem.background) + ';';
//}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//static QString __InvertColor(_CColor c)
//{
//	int i = ~c.Name(true).toInt(0, 16);
//	// DEBUG	qs = QString("#%1").arg((i & 0xFFFFFF), 0, 16);
//	return QString("#%1").arg((i & 0xFFFFFF), 0, 16);
//}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
//static QString __ContrastColorString(QString sc)
//{
//	QColor qc = QColor(sc);
//	if (qc.isValid())
//	{
//		if (qc.red() + qc.green() + qc.blue() > 3*128)
//			return "#000";
//		else
//			return "#fff";
//	}
//	else
//		return "#000";
//}

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
//static bool IsValidColor(QString str)
//{
//	if (str[0] != '#')
//		str = '#' + str;
//	QColor color(str);
//	return color.isValid();
//}

/*============================================================================
* TASK: format color QString into rgba when opacity is used, use original
*		 hex color QString if not. 
* EXPECTS: color - to transform
* RETURNS: color string starting w. '#'
*----------------------------------------------------------------------------*/
QString ColorToStr(_CColor color)
{
	QString c = color.Name();	// c starts with '#'
	int opacity = color.Opacity();
	if (opacity > 0)	// color name format: #AABBCC
		return QString("rgba(%1,%2,%3,%4)").arg(c.mid(1, 2).toInt(nullptr, 16)).arg(c.mid(1+2, 2).toInt(nullptr, 16)).arg(c.mid(1+4, 2).toInt(nullptr, 16)).arg((double)opacity / 100.0);

	return c;
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

	ui.sample->setPage(&_page);
	ui.sample->load(QUrl(QStringLiteral("file:///Resources/index.html")));

	connect(&_page, &WebEnginePage::LinkClickedSignal, this, &FalconG::LinkClicked);
	connect(&_page, &WebEnginePage::loadFinished, this, &FalconG::WebPageLoaded);

	ui.trvAlbums->setHeaderHidden(true);
	ui.trvAlbums->setModel(new AlbumTreeModel());
	connect(ui.trvAlbums->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FalconG::_AlbumStructureSelectionChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SingleSelection, this, &FalconG::_TnvSelectionChanged);
	// connect with albumgen's 
	connect(this,	   &FalconG::CancelRun,						&albumgen, &AlbumGenerator::Cancelled);
	connect(&albumgen, &AlbumGenerator::SignalToSetProgressParams,	this, &FalconG::_SetProgressBar);
	connect(&albumgen, &AlbumGenerator::SignalProgressPos,			this, &FalconG::_SetProgressBarPos);
	connect(&albumgen, &AlbumGenerator::SignalSetLanguagesToUI,		this, &FalconG::_SetupLanguagesToUI);
	connect(&albumgen, &AlbumGenerator::SignalToEnableEditTab,		this, &FalconG::_EnableEditTab);
//	connect(&albumgen, &AlbumGenerator::SignalImageMapChanged,		this, &FalconG::_ImageMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalAlbumStructChanged,	this, &FalconG::_AlbumMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalToShowRemainingTime,	this, &FalconG::_ShowRemainingTime);
	connect(&albumgen, &AlbumGenerator::SignalToCreateIcon,	this, &FalconG::_CreateUplinkIcon);
	connect(&albumgen, &AlbumGenerator::SetDirectoryCountTo,		this, &FalconG::_SetDirectoryCountTo);
	connect(ui.tnvImages, &ThumbnailWidget::SignalInProcessing,		this, &FalconG::_ThumbNailViewerIsLoading);
//	connect(ui.tnvImages, &ThumbnailWidget::SignalTitleChanged,		this, &FalconG::_TrvTitleChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SignalStatusChanged,	this, &FalconG::_TnvStatusChanged);
	connect(ui.tnvImages, &ThumbnailWidget::SignalNewThumbnail,		this, &FalconG::_TnvNewThumbs);
	connect(ui.btnSaveChangedDescription, &QPushButton::clicked,  this, &FalconG::_SaveChangedTitleDescription);
	connect(ui.btnSaveChangedTitle,		  &QPushButton::clicked,  this, &FalconG::_SaveChangedTitleDescription);

	QSettings s(falconG_ini, QSettings::IniFormat);	// in program directory

	restoreGeometry (s.value("wgeometry").toByteArray());
	restoreState(s.value("wstate").toByteArray());

	CONFIGS_USED::Read();		// to get last used configuration
	config.Read();				// to read config from

	frmMain = this;

	_PopulateFromConfig();

	int h = ui.tabEdit->height();
	ui.editSplitter->setSizes({h*70/100,h*30/100});// ({532,220 });

	_EnableButtons();
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

	if (config.Changed())
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

	QSettings s(falconG_ini, QSettings::IniFormat);	// in program directory
	s.setValue("wgeometry", saveGeometry());
	s.setValue("wstate", saveState());
}

void FalconG::on_toolBox_currentChanged(int newIndex)
{
	++_busy;
//	ui.cbActualItem->setCurrentIndex(newIndex);
	--_busy;
}

/*============================================================================
* TASK:		  display history of previous galleries and seelct from them
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSourceHistory_clicked()
{
	SourceHistory hist(CONFIGS_USED::maxSavedConfigs, CONFIGS_USED::lastConfigs, ui.edtSourceGallery->text(), this);

	hist.exec();
	CONFIGS_USED::maxSavedConfigs = hist.MaxSize();

	if (SourceHistory::Changed() || (SourceHistory::Selected() >= 0 &&
		SourceHistory::Selected() != CONFIGS_USED::indexOfLastUsed) )
	{
		if (SourceHistory::Selected() >= 0)
		{
			CONFIGS_USED::indexOfLastUsed = SourceHistory::Selected();
			config.Read();

			_PopulateFromConfig();	// edit values from config

			QString s = ui.edtSourceGallery->text().trimmed();
			if (ui.edtSourceGallery->text().isEmpty())
			{
				++_busy;
				ui.edtSourceGallery->setText(CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed));
				--_busy;
			}
		}
		if (SourceHistory::Changed())
			CONFIGS_USED::Write();
	}
}


/*============================================================================
  * TASK:	set the shortcut key sequence
  * EXPECTS: pw : button qs shoertcut as a string e.g. "F9" or "Ctrl+O"
  * RETURNS:
  * GLOBALS:
  * REMARKS: required as setText with a & in it changes the shortcut for the button
 *--------------------------------------------------------------------------*/
void __SetShortcut(QPushButton *pw, QString qs)
{
	pw->setShortcut(QKeySequence(qs));
}
// /DEBUG

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
		emit CancelRun();
	}
	else
	{
		CONFIGS_USED::Write();
		albumgen.SetRecrateAlbumFlag(config.bGenerateAll || config.Changed());

		++_running;
		_edited = false;

		ui.btnSaveStyleSheet->setEnabled(false);

		ui.btnSaveConfig->setEnabled(false);

		ui.btnExit->setEnabled(false);
		ui.actionExit->setEnabled(false);
		ui.btnPreview->setEnabled(false);

		ui.centralWidget->setCursor(Qt::WaitCursor);

		ui.btnGenerate->setText(tr("Cancel (F9}"));
		__SetShortcut(ui.btnGenerate, "F9");
		ui.pnlProgress->setVisible(true);

		ui.tabFalconG->setCurrentIndex(0);	// show front page
			   // when interrupted in phase1 (_phase == 0) total generation is used
		_phase = 0;
				// READ album
		if (!albumgen.Read())		// any error
			--_running, _phase = -1;

		_TrvCountChanged();			// show in lblTotalCount (page: Edit)

			// WRITE album
		if (_running)
			ui.btnPreview->setEnabled(albumgen.Write() == 0);

		ui.progressBar->setValue(0);

		ui.pnlProgress->setVisible(false);
		ui.centralWidget->setCursor(Qt::ArrowCursor);
		ui.btnGenerate->setText(tr("Generate (F9}"));
		__SetShortcut(ui.btnGenerate, "F9");
//		__SetShortcut(ui.btnPreview, "F12");
		ui.btnSaveStyleSheet->setEnabled(true);

		ui.btnSaveConfig->setEnabled  (config.Changed());

		ui.btnExit->setEnabled(true);
		ui.actionExit->setEnabled(true);

		--_running;
		if(_phase != -1)
			ui.tabFalconG->setCurrentIndex(2);	// show 'Edit' page

		albumgen.SetRecrateAlbumFlag(config.bGenerateAll);	// not until relevant changes
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
	if (_busy)
		return;

	bool bEnable1 = !ui.edtSourceGallery->text().isEmpty() &&
					!ui.edtServerAddress->text().isEmpty(),
		 bEnable2 = !ui.edtDestGallery->text().isEmpty();

	if (bEnable1)
	{
		QDir qd(ui.edtSourceGallery->text().trimmed());
		bEnable1 &= qd.exists();
	}

	ui.btnGenerate->setEnabled(bEnable1 && bEnable2);
	ui.btnSaveStyleSheet->setEnabled(bEnable2);
	ui.btnSaveConfig->setEnabled(config.Changed());
}

_CElem* FalconG::_PtrToElement(AlbumElement ae)
{
	if (ae == aeUndefined)
		ae = _aeActiveElement;
	switch (ae)		// same order as in the UI!
	{
		case aeWebPage:					 return &config.Web;
		case aeHeader:					 return &config.Header;
		case aeMenuButtons:				 return &config.Menu;
		case aeLangButton:				 return &config.Lang;
		case aeSmallTitle:				 return &config.SmallGalleryTitle;
		case aeGalleryTitle:			 return &config.GalleryTitle;
		case aeGalleryDesc:				 return &config.GalleryDesc;
		case aeSection:					 return &config.Section;
		case aeThumb:					 return &config.Thumb;
		case aeImageTitle:				 return &config.ImageTitle;
		case aeImageDesc:				 return &config.ImageDesc;
		case aeLightboxTitle:			 return &config.LightboxTitle;
		case aeLightboxDescription:		 return &config.LightboxDesc;
		case aeFooter:					 return &config.Footer;
		default: return  nullptr; break;
	}
}


//static QString __ColorToRGBAString(_CColor &cc)
//{
//	const QColor c(cc.ARGB());	// deals with #xyz #rrggbb and #aarrggbb
//
//	QString s = QString("(%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
//	if (c.alpha() != 0xff)
//		s = QString("rgba" + s + ",%1);").arg(c.alphaF());
//	else
//		s = "rgb" + s + ");";
//	return s;
//}

/*========================================================
 * TASK:	sets all characteristics of sample element
 *			from the actual given _CElem in confog
 * PARAMS:	ae - element index or aeUndefined when
 *				it uses _aeActiveElement
 * GLOBALS: config
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_ElemToSample(AlbumElement ae)
{
	if (ae == aeUndefined)
		ae = _aeActiveElement;

	_CElem* pElem = _PtrToElement(ae);

	if (ae == aeWebPage)
	{
		_SetPageColor(pElem);
		_SetPageBackground(pElem);
	}
	else
	{
		_SetColor(pElem);
		_SetBackground(pElem);
	}
	_SetFont(pElem);
	_SetDecoration(pElem);
	_SetShadow(pElem, 0);	// text
	_SetShadow(pElem, 1);	// box
	_SetLinearGradient(pElem);
	_SetBorder(pElem);
	_SetTextAlign(pElem);

	// menus: set uplink icon
	if (ae == aeMenuButtons)
	{
		QIcon icon = _SetUplinkIcon();		// from file (see around line# 2297)
		QString qs = "res/up-link.png";
		QFile::remove(qs);
		qs = "background-image: " + qs;
		icon.pixmap(64,64).save(qs);		// and save into "res" subdirectory
		_SetCssProperty(&config.Menu, qs,"#uplink");
	}
}

// helper
QString __ToolButtonBckStyleSheet(QString value)
{
	QString qs = QString("QToolButton { background-color:" + value + ";}");
	return qs;
}

void FalconG::_GlobalsToUi()
{
	++_busy;
	ui.btnPageColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Web.color.Name()));
	ui.btnPageBackground->setStyleSheet(__ToolButtonBckStyleSheet(config.Web.background.Name()));
	ui.edtBckImageName->setText(config.backgroundImage.fileName);
	switch (config.backgroundImage.v)
	{
		case hNotUsed: ui.rbNoBackgroundImage->setChecked(true); break;
		case hAuto:	  ui.rbCenterBckImage->setChecked(true); break;
		case hCover: ui.rbCoverBckImage->setChecked(true); break;
		case hTile: ui.rbTileBckImage->setChecked(true); break;
	}

	--_busy;
}

void FalconG::_ActualSampleParamsToUi()
{
	_CElem* pElem = _PtrToElement();

	++_busy;

	ui.btnForeground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->color.Name()));
	ui.chkTextOpacity->setChecked(pElem->color.Opacity() != -1);
	if(pElem->background.Valid())
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->background.Name()));	// sets btnBackground's color
	else // invalid: set from background
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(config.Web.background.Name()));	// sets btnBackground's color
	ui.chkBackgroundOpacity->setChecked(pElem->background.Opacity() != -1);

	_EnableGradient(false);					// disable gradient buttons

	ui.chkUseGradient->setChecked(pElem->gradient.used);	// re-enable gradient buttons when checked

	// set shadow colors and values
	ui.gbTextShadow->setEnabled(true);
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;	// 0:text shadow else box shadow
	bool b = pElem->shadow1[which].Used();
	QString sStyle = __ToolButtonBckStyleSheet(b ? pElem->shadow1[which].Color() : "#ddd");	// same color for shadow2
	ui.chkShadowOn->setChecked(b);
	ui.btnShadowColor->setStyleSheet(  sStyle );

	ui.sbShadowHoriz1->setValue(pElem->shadow1[which].Horiz());
	ui.sbShadowVert1->setValue (pElem->shadow1[which].Vert());
	ui.sbShadowBlur1->setValue (pElem->shadow1[which].Blur());

	ui.sbShadowHoriz2->setValue(pElem->shadow2[which].Horiz());
	ui.sbShadowVert2->setValue(pElem->shadow2[which].Vert());
	ui.sbShadowBlur2->setValue(pElem->shadow2[which].Blur());

	ui.chkTextOpacity->setChecked(pElem->color.Opacity() > 0);
	ui.chkBackgroundOpacity->setChecked(pElem->background.Opacity() > 0);
	ui.sbTextOpacity->setValue(pElem->color.Opacity() < 0 ? 100: pElem->color.Opacity());
	ui.sbTextOpacity->setEnabled(ui.chkTextOpacity->isChecked());
	ui.sbBackgroundOpacity->setValue(pElem->background.Opacity() < 0 ? 100 : pElem->background.Opacity());
	ui.sbBackgroundOpacity->setEnabled(ui.chkBackgroundOpacity->isChecked());

	ui.edtFontFamily->setText(pElem->font.Family());
	ui.cbPointSize->setCurrentText(pElem->font.SizeStr());
	ui.chkBold->setChecked(pElem->font.Bold());
	ui.chkItalic->setChecked(pElem->font.Italic());
	ui.chkTdUnderline->setChecked(pElem->decoration.IsUnderline());
	ui.chkTdLinethrough->setChecked(pElem->decoration.IsLineThrough());
	ui.chkDifferentFirstLine->setChecked(pElem->font.IsFirstLineDifferent());

	switch (pElem->alignment.v)
	{
		case alNone:  ui.rbTextAlignNone->setChecked(true); break;
		case alLeft: ui.rbTextLeft->setChecked(true); break;
		case alCenter: ui.rbTextCenter->setChecked(true); break;
		case alRight: ui.rbTextRight->setChecked(true); break;
	}

		// border
	ui.chkUseBorder->setChecked(pElem->border.Used());
	if (pElem->border.Used())
	{
		BorderSide side = (BorderSide)(ui.cbBorder->currentIndex()-1);

		ui.btnBorderColor->setStyleSheet(QString("QToolButton { background-color:" + pElem->border.ColorStr(side)+";\n color:"+ config.Web.background.Name()+";}\n"));
		ui.sbBorderRadius->setValue(pElem->border.Radius());
		int cnt = pElem->border.BorderCnt();
		ui.cbBorder->setCurrentIndex( cnt == 1 ? 0 : 1);
		ui.cbBorderStyle->setCurrentText(pElem->border.Style(side));
		ui.sbBorderWidth->setValue(pElem->border.Width(side));
	}

	ui.cbPointSizeFirstLine->setCurrentText(pElem->font.FirstLineFontSizeStr());


	--_busy;
}


/*============================================================================
* TASK:		set all characteristics of the sample gallery from 'config'
* EXPECTS: 'config' is set up
* GLOBALS:  'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_ConfigToSample()
{
	++_busy;		// prevent page changes until last settings

	_ElemToSample(aeWebPage);
	_ElemToSample(aeHeader);
	_ElemToSample(aeMenuButtons);
	_ElemToSample(aeLangButton);
	_ElemToSample(aeSmallTitle);
	_ElemToSample(aeGalleryTitle);
	_ElemToSample(aeGalleryDesc);
	_ElemToSample(aeSection);
	_ElemToSample(aeImageTitle);
	_ElemToSample(aeImageDesc);
	_ElemToSample(aeLightboxTitle);
	_ElemToSample(aeLightboxDescription);
	_ElemToSample(aeFooter);

	QString qs = QString("QToolButton {background-color:%1;color:%2;}").arg(config.imageBorder.ColorStr(sdAll)).arg(config.Web.background.Name());
	ui.btnImageBorderColor->setStyleSheet(qs);
	// qs = config.imageBorder.ForStyleSheet(false);
	qs = config.imageBorder.ForStyleSheetShort();
	_RunJavaScript(".thumb", qs);
	--_busy;
}

void FalconG::_SetConfigChanged(bool on)
{
	ui.btnSaveConfig->setEnabled(config.SetChanged(on));
}

/*============================================================================
* TASK:		set all saved data into global controls	+ interface elements
* EXPECTS:	'config' is set up
* GLOBALS:	'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_PopulateFromConfig()
{
	config.ClearChanged();

	++_busy;

	ui.edtAbout->setText(config.sAbout);
	ui.edtAlbumDir->setText(config.dsAlbumDir.ToString());
	ui.edtDefaultFonts->setText(config.sDefFonts.ToString());
	ui.edtDescription->setText(config.sDescription);
	ui.edtDestGallery->setText(QDir::toNativeSeparators(config.dsGallery.ToString()));
	ui.edtEmailTo->setText(config.sMailTo);
	ui.edtGalleryRoot->setText(config.dsGRoot.ToString());
	ui.edtGalleryTitle->setText(config.sGalleryTitle);
	QString s = config.sGoogleFonts.ToString();
	s.replace('|', ',');
	ui.edtGoogleFonts->setText(s);
	ui.edtKeywords->setText(config.sKeywords);
	ui.edtServerAddress->setText(config.sServerAddress);
	ui.edtSourceGallery->setText(QDir::toNativeSeparators(config.dsSrc.ToString()));
	ui.edtTrackingCode->setText(config.googleAnalTrackingCode);
	ui.edtUplink->setText(config.sUplink);
	ui.edtWmColor->setText(QString().setNum(config.waterMark.wm.Color(), 16));
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.wm.marginX));
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.wm.marginY));

	if (!config.dsAlbumDir.IsEmpty() && ui.edtAlbumDir->placeholderText() != config.dsAlbumDir.ToString())
		ui.edtAlbumDir->setText(config.dsAlbumDir.ToString());
	if (!config.sBaseName.IsEmpty() && ui.edtBaseName->placeholderText() != config.sBaseName.ToString())
		ui.edtBaseName->setText(config.sBaseName.ToString());
	if (!config.dsFontDir.IsEmpty() && ui.edtFontDir->placeholderText() != config.dsFontDir.ToString())
		ui.edtFontDir->setText(config.dsFontDir.ToString());
	if (!config.dsImageDir.IsEmpty() && ui.edtImg->placeholderText() != config.dsImageDir.ToString())
		ui.edtImg->setText(config.dsImageDir.ToString());
	if (!config.dsThumbDir.IsEmpty() && ui.edtThumb->placeholderText() != config.dsThumbDir.ToString())
		ui.edtThumb->setText(config.dsThumbDir.ToString());

	ui.chkAddDescToAll->setChecked(config.bAddDescriptionsToAll);
	ui.chkAddTitlesToAll->setChecked(config.bAddTitlesToAll);
	ui.chkButImages->setChecked(config.bButImages);
	ui.chkCanDownload->setChecked(config.bCanDownload);

// not saved/restored:
//	ui.chkDebugging->setChecked(config.bDebugging);
	--_busy;
	ui.chkCropThumbnails->setChecked(config.bCropThumbnails);
	ui.chkDistortThumbnails->setChecked(config.bDistrortThumbnails);
	++_busy;
    
	ui.chkDoNotEnlarge->setChecked(config.doNotEnlarge);
	ui.chkFacebook->setChecked(config.bFacebookLink);
	ui.chkGenerateAll->setChecked(config.bGenerateAll);
	ui.chkForceSSL->setChecked(config.bForceSSL);
	ui.chkImageBorder->setChecked(config.imageBorder.Used());
	ui.chkLowerCaseImageExtensions->setChecked(config.bLowerCaseImageExtensions);
	ui.chkMenuToAbout->setChecked(config.bMenuToAbout);
	ui.chkMenuToContact->setChecked(config.bMenuToContact);
	ui.chkMenuToDescriptions->setChecked(config.bMenuToDescriptions);
	ui.chkMenuToToggleCaptions->setChecked(config.bMenuToToggleCaptions);
	ui.chkOvrImages->setChecked(config.bOvrImages);
// not saved/restored:
//	ui.chkReadFromGallery->setChecked(config.bReadFromGallery);
//	ui.chkReadJAlbum->setChecked(config.bReadJAlbum);	  
	ui.chkRightClickProtected->setChecked(config.bRightClickProtected);
	ui.chkSetLatest->setChecked(config.generateLatestUploads);
	ui.chkSourceRelativePerSign->setChecked(config.bSourceRelativePerSign);
	ui.chkUseGoogleAnalytics->setChecked(config.googleAnalyticsOn);
	ui.chkUseGradient->setChecked(config.Menu.gradient.used);
	ui.chkUseWM->setChecked(config.waterMark.used);

	ui.btnGradMiddleColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[1].color));
	ui.btnGradStartColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[0].color));
	ui.btnGradStopColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[2].color));
	ui.btnLink->setChecked(config.imageSizesLinked);
	ui.btnWmColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.Color()));
	ui.btnWmShadowColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.shadowColor,6,16,QChar('0')));

	ui.sbNewDays->setValue(config.newUploadInterval);


	ui.sbImageWidth->setValue(config.imageWidth);
	ui.sbImageHeight->setValue(config.imageHeight);
	ui.sbThumbnailWidth->setValue(config .thumbWidth);
	ui.sbThumbnailHeight->setValue(config.thumbHeight);

	int h = config.imageHeight ? config.imageHeight : 1;
	_aspect = (double)config.imageWidth / h;

							// Watermark
	ui.sbWmOpacity->setValue(config.waterMark.wm.Opacity());

	ui.edtWatermark->setText(config.waterMark.wm.text);

	h = config.waterMark.wm.origin & 0xF;  // vertical position
	ui.cbWmVPosition->setCurrentIndex(h);
	h = (config.waterMark.wm.origin & 0xF0) >> 4; // horizontal position
	ui.cbWmHPosition->setCurrentIndex(h);
	

	ui.lblWaterMarkFont->setText(QString("%1 (%2,%3)").arg(config.waterMark.wm.font.family()).arg(config.waterMark.wm.font.pointSize()).arg(config.waterMark.wm.font.italic()));
	if (!config.waterMark.wm.text.isEmpty())
		ui.lblWmSample->setText(config.waterMark.wm.text);

	ui.lblWmSample->setFont(config.waterMark.wm.font);


	switch (config.styleIndex)
	{
		case 0:
		default: 
			ui.rbDefaultStyle->setChecked(true);
			break;
		case 1: ui.rbSystemStyle->setChecked(true);
			break;
		case 2:		// blue theme
			ui.rbBlueStyle->setChecked(true);
			break;
		case 3:		// dark theme
			ui.rbDarkStyle->setChecked(true);
			break;
		case 4:		// black theme
			ui.rbBlackStyle->setChecked(true);
			break;
	}
	ui.toolBox->setCurrentIndex(0);		// global page
	_GlobalsToUi();	
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

	config.googleAnalTrackingCode = ui.edtTrackingCode->text().trimmed();
	 _SetConfigChanged(config.googleAnalTrackingCode.Changed());
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

	config.dsGallery = ui.edtDestGallery->text().trimmed();
	_SetConfigChanged(config.dsGallery.Changed());
	_EnableButtons();
}

void FalconG::on_edtFontDir_textChanged()
{
	if (_busy)
		return;

	config.dsFontDir = ui.edtFontDir->text().trimmed();
	_SetConfigChanged(config.dsFontDir.Changed());
}

/*============================================================================
* TASK:	   Handle source gallery changes
* EXPECTS:
* GLOBALS:
* REMARKS: Although the first .ToString()uct file will be saved into the source 
*			directory
*--------------------------------------------------------------------------*/
void FalconG::on_edtSourceGallery_textChanged()
{	
	if (_busy)
		return;

	QString scr = QDir::cleanPath(ui.edtSourceGallery->text().trimmed()) + "/";
	if (!config.dsSrc.ToString().isEmpty() && config.dsSrc.ToString() != scr)
	{
		config.dsSrc = scr;
		for(int i = 0; i < CONFIGS_USED::lastConfigs.size(); ++i )
			if (config.dsSrc.ToString() == CONFIGS_USED::lastConfigs[i])
			{
				CONFIGS_USED::indexOfLastUsed = i;
				break;
			}
		QFile f(config.dsSrc.ToString() + falconG_ini);
		if (f.exists())
		{
			config.Read();
			_PopulateFromConfig();	// edit values from config

			_EnableButtons();
		}
	}
}


/*========================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_edtDescription_textChanged()
{
	if (_busy)
		return;
	config.sDescription = ui.edtDescription->text().trimmed();
	_SetConfigChanged(true);
}


/*========================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_edtKeywords_textChanged()
{
	if (_busy)
		return;
	config.sKeywords = ui.edtKeywords->text().trimmed();
	_SetConfigChanged(true);
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
	++_busy;
	config.dsAlbumDir = ui.edtAlbumDir->text().trimmed();
	_SetConfigChanged(true);
	--_busy;
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

	config.sAbout = ui.edtAbout->text().trimmed();
	_SetConfigChanged(true);
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

	config.dsGRoot = ui. edtGalleryRoot->text().trimmed();
	_SetConfigChanged(true);
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

	config.dsImageDir = ui. edtImg->text().trimmed();
	_SetConfigChanged(true);
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

	config.sServerAddress = ui. edtServerAddress->text().trimmed();
	_SetConfigChanged(true);
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

	config.sGalleryTitle = ui.edtGalleryTitle->text().trimmed();
	_SetConfigChanged(true);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtGalleryLanguages_textChanged()
{
	config.sGalleryLanguages = ui.edtGalleryLanguages->text().trimmed();
	_SetConfigChanged(true);
}

void FalconG::on_edtGoogleFonts_editingFinished()
{
	if (_busy)
		return;
	QString s = ui.edtGoogleFonts->text().trimmed();
// replace all spaces and commas NOT inside quotes
		//	https://stackoverflow.com/questions/6462578/regex-to-match-all-instances-not-inside-quotes
		// regualr expression for this with lookahead feature:
		//			[ ,]+(?=([^"\\]*(\\.|"([^"\\]*\\.)*[^"\\]*"))*[^"]*$)
		// simpler and much faster one (not working, why?):
		//			\\"|"(?:\\"|[^"])*"|([ ,]+)
//		s.replace(QRegularExpression(R"(\\"|"(?:\\"|[^"])*"|([ ,]+)"),"|");
	s.replace(QRegularExpression(R"([ ,]+(?=([^"\\]*(\\.|"([^"\\]*\\.)*[^"\\]*"))*[^"]*$))"), "|");
	config.sGoogleFonts = s;
	_SetConfigChanged(true);
	_ModifyGoogleFontImport();
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

	config.sUplink = ui. edtUplink->text().trimmed();
	_SetConfigChanged(true);
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

	config.sMainPage = ui.edtMainPage->text().trimmed();
	_SetConfigChanged(true);
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

	config.sMailTo = ui.edtEmailTo->text().trimmed();
	_SetConfigChanged(true);
}

void FalconG::on_edtThumb_textChanged()
{
	if (_busy)
		return;
	
	++_busy;
	config.dsThumbDir = ui.edtThumb->text().trimmed();
	--_busy;
	_SetConfigChanged(true);
}

void FalconG::on_edtBaseName_textChanged()
{
	if (_busy)
		return;

	config.dsGRoot = ui.edtGalleryRoot->text().trimmed();
	_SetConfigChanged(true);
}

void FalconG::on_edtDefaultFonts_textChanged()
{
	if (_busy)
		return;
	config.sDefFonts = ui.edtDefaultFonts->text().trimmed();
	_SetConfigChanged(true);
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
	
	_SetConfigChanged(true);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWmColor_textChanged()
{
	if (_busy)
		return;

	_SetConfigChanged(config.waterMark.v = true);

	StyleHandler handler(ui.btnWmColor->styleSheet());
	handler.SetItem("QToolButton", "background-color", "#" + ui.edtWmColor->text());
	ui.btnWmColor->setStyleSheet(handler.StyleSheet());
	config.waterMark.wm.SetColor("#" + QString().setNum((int)config.waterMark.wm.Opacity()*100, 16) + ui.edtWmColor->text().mid(1));
	config.waterMark.v = true;
	_RunJavaScript(".thumb::after","color: "+config.waterMark.wm.ColorToStr() );
}


/*============================================================================
  * TASK:
  * EXPECTS:mx, my new margines. -1 : do not set
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::_UpdaetWatermarkMargins(int mx, int my)
{
	WaterMark& wm = config.waterMark.wm;
	bool centered;
	if (mx >= 0)
	{
		wm.marginX = mx;	// set active and clear inactive
		_RunJavaScript(".thumb::after", wm.XMarginName(centered, false)+"");
		_RunJavaScript(".thumb::after", wm.XMarginName(centered, true)+ wm.OffsetXToStr() );
	}
	if (my >= 0)
	{
		wm.marginY = mx;	// set active and clear inactive
		_RunJavaScript(".thumb::after", wm.YMarginName(centered, false)+"");
		_RunJavaScript(".thumb::after", wm.YMarginName(centered, true)+ wm.OffsetYToStr() );
	}
	_SetConfigChanged(true);
}
void FalconG::on_edtWmHorizMargin_textChanged()
{
	if (_busy)
		return;
	_UpdaetWatermarkMargins(ui.edtWmHorizMargin->text().toInt(), -1);
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
	_UpdaetWatermarkMargins(-1, ui.edtWmVertMargin->text().toInt());
}

/*============================================================================
  * TASK:	change the image width in config to new width and
  *			for linked image sizes change the value in the height box
  * EXPECTS: val: new image width
  * GLOBALS:
  * REMARKS: - when linked the scpoect ratio will not change
  *			 - when not linked, the height will not change
  *			 - does not modify the thumbnail
 *--------------------------------------------------------------------------*/
void FalconG::on_sbImageWidth_valueChanged(int val)
{
	if (_busy)
		return;
	++_busy;
	int h = config.imageHeight;

	if (ui.btnLink->isChecked())
	{
		h = val / _aspect;
		ui.sbImageHeight->setValue(h);
		config.imageHeight = h;
		
		h = 0; // no change in aspect ratio when linked!
	}
	config.imageWidth = val;
	config.waterMark.v = true;
	
	if(h)	// else no change
		_aspect = (double)val / (double)h;
	--_busy;
// ????	_ChangesToSample(dp);
}

/*============================================================================
  * TASK:	change the image height in config to new height and
  *			for linked image sizes change the value in the width box
  * EXPECTS: val - new image height
  * GLOBALS:
  * REMARKS: - when linked the scpoect ratio will not change
  *			 - when not linked, the width will not change
  *			 - does not modify the thumbnail
 *--------------------------------------------------------------------------*/
void FalconG::on_sbImageHeight_valueChanged(int val)
{
	if (_busy)
		return;
	++_busy;
	int w = config.imageWidth;

	if (ui.btnLink->isChecked())
	{
		w = val * _aspect;		// new width
		ui.sbImageWidth->setValue(w);
		config.imageWidth = w;
		
		w = 0; // no change in aspect ratio when linked!
	}
	config.imageHeight = val;
	
	if (w && val)
		_aspect = (double)w / (double)val;

	--_busy;
// ????	_ChangesToSample();
}

/*============================================================================
  * TASK:	change the thumbnail width in config to new width and
  *			for linked thumbnail sizes change the value in the height box
  * EXPECTS: val: new  thumbnail width
  * GLOBALS:
  * REMARKS: - when linked the aspect ratio will not change
  *			 - when not linked, the height will not change
  *			 - does not modify the thumbnail
 *--------------------------------------------------------------------------*/
void FalconG::on_sbThumbnailWidth_valueChanged(int val)
{
	if (_busy)
		return;
	++_busy;
	int h = config.thumbHeight;

	//if (ui.btnLink->isChecked())
	//{
	//	h = val / _aspect;
	//	ui.sbThumbnailHeight->setValue(h);
	//	config.thumbHeight = h;
	//	
	//	h = 0; // no change in aspect ratio when linked!
	//}
	config.thumbWidth = val;
	
	if (h)	// else no change
		_aspect = (double)val / (double)h;
	--_busy;
// ????	_ChangesToSample();
}

/*============================================================================
  * TASK:	change the thumbnail height in config to new height and
  *			for linked thumbnail sizes change the value in the width box
  * EXPECTS: val - new thumbnail height
  * GLOBALS:
  * REMARKS: - when linked the aspect ratio will not change
  *			 - when not linked, the width will not change
  *			 - does not modify the thumbnail
 *--------------------------------------------------------------------------*/
void FalconG::on_sbThumbnailHeight_valueChanged(int val)
{
	if (_busy)
		return;
	++_busy;
	int w = config.thumbWidth;

	//if (ui.btnLink->isChecked())
	//{
	//	w = val * _aspect;		// new width
	//	ui.sbThumbnailWidth->setValue(w);
	//	config.thumbWidth = w;
	//	
	//	w = 0; // no change in aspect ratio when linked!
	//}
	config.thumbHeight = val;
	
	if (w && val)
		_aspect = (double)w / (double)val;

	--_busy;
// ??????	_ChangesToSample();
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
		
		ui.chkButImages->setChecked(false);
	}
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkButImages_toggled(bool on)
{
	if (_busy)
		return;
	if (config.bButImages != on)
	{
		config.bButImages = on;
		
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
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkLowerCaseImageExtensions_toggled(bool on)
{
	if (_busy)
		return;
	config.bLowerCaseImageExtensions = on;
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
	_SetConfigChanged(true);

	_EnableButtons();
}

/*============================================================================
* TASK:	toggle state of structure disregard gallery struct box
* EXPECTS:	on - new state
* GLOBALS:
* REMARKS:	this bool is not saved into the configuration
*--------------------------------------------------------------------------*/
void FalconG::on_chkReadJAlbum_toggled(bool on)
{
	if (_busy)
		return;
	config.bReadJAlbum = on;
	_SetConfigChanged(true);
	
	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkReadFromGallery_toggled(bool on )
{
	if (_busy)
		return;
	config.bReadFromGallery = on;
	_SetConfigChanged(true);
	
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
	if (_busy)
		return;
	config.bRightClickProtected = b;
	_SetConfigChanged(true);
	
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
	if (_busy)
		return;
	config.bCanDownload = b;
	_SetConfigChanged(true);
	
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
	if (_busy)
		return;
	config.bForceSSL = b;
	_SetConfigChanged(true);
	
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
	if (_busy)
		return;
	config.googleAnalyticsOn = b; // ui.chkUseGoogleAnalytics->isChecked();
	_SetConfigChanged(true);
	
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
	
	if (_busy)
		return;
	_SetConfigChanged(true);
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
	_PtrToElement()->font.SetSize(txt);
	if (!ui.chkDifferentFirstLine->isChecked())
	{
		ui.cbPointSizeFirstLine->setCurrentText(txt);
		_PtrToElement()->font.SetDifferentFirstLine(true, txt);
	}
	_SetConfigChanged(true);
	_ElemToSample();
}

void FalconG::on_chkDifferentFirstLine_toggled(bool b)
{
	if (_busy)
		return;
	_PtrToElement()->font.SetDifferentFirstLine(b);	// do not change first line font size
	_SetConfigChanged(true);
	_ElemToSample();
}

void FalconG::on_cbPointSizeFirstLine_currentTextChanged(const QString& txt)
{
	if (_busy)
		return;
	_CElem* pElem;

	QString qsClass;
	pElem = _PtrToElement();
	pElem->font.SetDifferentFirstLine(pElem->font.IsFirstLineDifferent(), txt);	// do not change first line font size
	_SetConfigChanged(true);
	_ElemToSample();
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
	_CElem *pElem;
	pElem = _PtrToElement();
	pElem->font.SetFamily(ui.edtFontFamily->text());
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkImageBorder_toggled(bool on)
{
	if (_busy)
		return;

	QString qs;
	config.imageBorder.SetUsed(on);
	qs = config.imageBorder.ForStyleSheet(false);
	_SetConfigChanged(true);
	_RunJavaScript(".thumb", qs);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS: disabled, not used
 *--------------------------------------------------------------------------*/
void FalconG::on_chkIconText_toggled(bool)
{
	if (_busy)
		return;
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS: disabled, not used
 *--------------------------------------------------------------------------*/
void FalconG::on_chkIconTop_toggled(bool)
{
	if (_busy)
		return;
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkBold_toggled(bool on)
{
	if (_busy)
		return;
	_PtrToElement()->font.SetFeature(fBold, on);
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkItalic_toggled(bool on)
{
	if (_busy)
		return;
	_PtrToElement()->font.SetFeature(fItalic, on);
	_SetConfigChanged(true);
	_ElemToSample();
}


void FalconG::_TextAlignToConfig(Align align, bool on)
{
	if (_busy)
		return;
	if(on)
		_PtrToElement()->alignment.v = align;
	_SetConfigChanged(true);
	_ElemToSample();	// clear decorations if neither checkbox is checked
}

/*========================================================
 * TASK:	set text decorations
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_TextDecorationToConfig(Decoration decoration, bool on)
{
	if (_busy)
		return;
	_PtrToElement()->decoration.SetDecoration(decoration, on);
	ui.gbDecorationStyle->setEnabled(ui.chkTdUnderline->isChecked() || ui.chkTdOverline->isChecked() || ui.chkTdLinethrough->isChecked());
	_SetConfigChanged(true);
	_ElemToSample();	// clear decorations if neither checkbox is checked
}

void FalconG::on_chkTdLinethrough_toggled(bool on) { 	_TextDecorationToConfig(tdLinethrough, on); }
void FalconG::on_chkTdUnderline_toggled(bool on) { _TextDecorationToConfig(tdUnderline, on); }
void FalconG::on_chkTdOverline_toggled(bool on) {	_TextDecorationToConfig(tdOverline, on);}
void FalconG::on_rbTdSolid_toggled  (bool on) { _TextDecorationToConfig(tdSolid, on); }
void FalconG::on_rbTdDotted_toggled(bool on) { _TextDecorationToConfig(tdDotted, on); }
void FalconG::on_rbTdDashed_toggled (bool on) { _TextDecorationToConfig(tdDashed, on); }
void FalconG::on_rbTdDouble_toggled (bool on) { _TextDecorationToConfig(tdDouble, on); }
void FalconG::on_rbTdWavy_toggled   (bool on) { _TextDecorationToConfig(tdWavy, on); }
void FalconG::on_rbTextAlignNone_toggled  (bool on) { _TextAlignToConfig(alNone, on); }
void FalconG::on_rbTextLeft_toggled  (bool on) { _TextAlignToConfig(alLeft, on); }
void FalconG::on_rbTextCenter_toggled(bool on) { _TextAlignToConfig(alCenter, on); }
void FalconG::on_rbTextRight_toggled( bool on) { _TextAlignToConfig(alRight, on); }


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
	_SetConfigChanged(true);
	
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
	_SetConfigChanged(true);
	
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
	_SetConfigChanged(true);

	_EnableButtons();
}

void FalconG::on_chkCropThumbnails_toggled(bool b)
{
	if (_busy)
		return;
	++_busy;
	config.bCropThumbnails = b;
	if (config.bDistrortThumbnails && b)
		config.bDistrortThumbnails = false;
	ui.chkDistortThumbnails->setChecked(config.bDistrortThumbnails);
	ui.sbThumbnailWidth->setEnabled(config.bDistrortThumbnails || b);
	--_busy;
	_SetConfigChanged(true);
}

void FalconG::on_chkDistortThumbnails_toggled(bool b)
{
	if (_busy)
		return;
	++_busy;
	config.bDistrortThumbnails = b;
	if (b && config.bCropThumbnails)
		config.bCropThumbnails = false;
	ui.chkCropThumbnails->setChecked(config.bCropThumbnails);
	ui.sbThumbnailWidth->setEnabled(b || config.bCropThumbnails);
	--_busy;
	_SetConfigChanged(true);
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
	_SetConfigChanged(true);
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
	_CElem* pElem = _PtrToElement();
	pElem->color.SetOpacity(on ? ui.sbTextOpacity->value()*2.55 : -1);
	_SetConfigChanged(true);
	_ElemToSample();
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
	_CElem* pElem = _PtrToElement();
	pElem->background.SetOpacity(on ? ui.sbBackgroundOpacity->value()*2.55 : -1);
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkUseGradient_toggled(bool on)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	pElem->gradient.used = on;
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkShadowOn_toggled(bool on)
{
	if (_busy)
		return;
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow1[which].Set(spUse, on);	// used flag
	_PtrToElement()->shadow2[which].Set(spUse, on);	// used flag
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_chkUseBorder_toggled(bool on)
{
	if (_busy)
		return;

	_CElem* pElem = _PtrToElement();
	pElem->border.SetUsed(on);
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
	_SetConfigChanged(true);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: on 'Gallery' page
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToContact_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToContact = on;
	_RunJavaScript(".menu-item#contact",QString("display:")+ (on ? "inline-block" : "none"));
	_SetConfigChanged(true);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: on 'Gallery' page
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToAbout_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToAbout = on;
	_RunJavaScript(".menu-item#about",QString("display") + (on? "inline-block" : "none"));
	_SetConfigChanged(true);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: on 'Gallery' page
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToDescriptions_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToDescriptions = on;
	_RunJavaScript(".menu-item#desc",QString("display") + (on? "inline-block" : "none") );
	_SetConfigChanged(true);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS: on 'Gallery' page
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToToggleCaptions_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToToggleCaptions = on;
	_RunJavaScript(".menu-item#captions", QString("display") + (on ? "inline-block" : "none"));
	_SetConfigChanged(true);
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
	_SetConfigChanged(true);
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
	_SetConfigChanged(true);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowHoriz1_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;

	_PtrToElement()->shadow1[which].Set(spHoriz, val);
	_ElemToSample();
}
/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowHoriz2_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;

	_PtrToElement()->shadow2[which].Set(spHoriz, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowVert1_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow1[which].Set(spVert, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowVert2_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow2[which].Set(spVert, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowBlur1_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow1[which].Set(spBlurR, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowBlur2_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow2[which].Set(spBlurR, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowSpread1_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow1[which].Set(spSpread, val);
	_ElemToSample();
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbShadowSpread2_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow2[which].Set(spSpread, val);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradStartPos_valueChanged(int val)
{
	_CElem* pElem = _PtrToElement();
	pElem->gradient.Set(gsStart, val, pElem->gradient.Color(gsStart));
	_SetWidgetGradientQt(ui.lblGradient, pElem);
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradMiddlePos_valueChanged(int val)
{
	_CElem* pElem = _PtrToElement();
	pElem->gradient.Set(gsMiddle, val, pElem->gradient.Color(gsMiddle));
	_SetWidgetGradientQt(ui.lblGradient, pElem);
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbGradStopPos_valueChanged(int val)
{
	_CElem* pElem = _PtrToElement();
	pElem->gradient.Set(gsStop, val, pElem->gradient.Color(gsStop));
	_SetWidgetGradientQt(ui.lblGradient, pElem);
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowHoriz_valueChanged(int val)
{
	config.waterMark.wm.shadowHoriz = val;
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowVert_valueChanged(int val)
{
	config.waterMark.wm.shadowVert = val;
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowBlur_valueChanged(int val)
{
	config.waterMark.wm.shadowBlur = val;
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbImageBorderWidth_valueChanged(int val)
{
	if (_busy)
		return;

	config.imageBorder.SetWidth(sdAll, val);
	QString qs = config.imageBorder.ForStyleSheetShort();
	qs = config.imageBorder.ForStyleSheetShort();
	_RunJavaScript(".thumb", qs);
	_SetConfigChanged(true);
}


/*========================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_sbImagePadding_valueChanged(int val)
{
	if (_busy || config.imagePadding == val )
		return;

	config.imagePadding = val;
	if (!config.imageBorder.Used())
		_RunJavaScript(".thumb",QString("padding:"));
	else	
		_RunJavaScript(".thumb",QString("padding: %1px").arg(val));

	_SetConfigChanged(true);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbTextOpacity_valueChanged(int val)
{
	if (_busy)
		return;
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
	if (_busy)
		return;
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
	if (_busy)
		return;
	config.waterMark.wm.SetOpacity(val);
	_SetConfigChanged(true);
	_RunJavaScript(".thumb::after","color:"+ config.waterMark.wm.ColorToStr());
}

void FalconG::on_sbBorderWidth_valueChanged(int val)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	BorderSide side = (BorderSide) (ui.cbBorder->currentIndex()-1);
	pElem->border.SetWidth(side, val);
	_SetConfigChanged(true);
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
}

void FalconG::on_sbBorderRadius_valueChanged(int val)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	pElem->border.SetRadius(ui.sbBorderRadius->value());
	_SetConfigChanged(true);
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//void FalconG::on_btnHome_clicked()
//{
//	_aeActiveElement = aeMenuButtons;
//	_ActualSampleParamsToUi();
//}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBorderColor_clicked()
{
	BorderSide side = (BorderSide )(ui.cbBorder->currentIndex()-1);
	QColor qc(config.Menu.border.ColorStr(side)),
			qcNew;
	qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew == qc || !qcNew.isValid())
		return;

	_SetConfigChanged(true);
	_CElem* pElem = _PtrToElement();
	pElem->border.SetColor(side, qcNew.name());
	ui.btnBorderColor->setStyleSheet(QString("QToolButton {background-color:%1;color:%2;}").arg(qcNew.name()).arg(config.Web.background.Name()));

	QString qs = pElem->border.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
	_EnableButtons();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//void FalconG::on_btnLang_clicked()
//{
//	_aeActiveElement = aeLangButton;
//	_ActualSampleParamsToUi();
//}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
//void FalconG::on_btnUplink_clicked()
//{
//	_aeActiveElement = aeMenuButtons;
//	_ActualSampleParamsToUi();
//}


/*========================================================
 * TASK:	sets the uplink icon from a file set in 
 *			config	in the correct color to the uplink
 *			menu web button
 * PARAMS:	iconName - path name of icon or empty
 *				when empty uses icon from resources
 * GLOBALS:	config
 * RETURNS: none, icon is set in btnUplink
 * REMARKS: - if no name given does not load any icon: uses
 *			  the one already set in the form file, just
 *			  changes its color
 *			- otherwise load file from the 'res' sub directory
 *			- icon file must be flat 32 bit png file with 
 *				transparent and white pixels. The color of 
 *				the white pixels will be changed to 
 *				config.Menu
 *-------------------------------------------------------*/
QIcon FalconG::_SetUplinkIcon(QString iconName)
{
	if (iconName.isEmpty() )
		iconName = "Resources/up-icon.png";
	QIcon icon(iconName);
	_lastUsedMenuForegroundColor = Qt::white;
	_SetIconColor(icon, config.Menu);
	return icon;
}

/*========================================================
 * TASK:	set icon for uplink from 'res' directory
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - icon must be a simple 2 color image
 *-------------------------------------------------------*/
void FalconG::on_btnSelectUplinkIcon_clicked()
{
	QString dir = config.dsApplication.ToString() + "res/";
	QString file = QFileDialog::getOpenFileName(this, "falconG - Select icon", dir, "*.png");
	if (!file.isEmpty())
	{
		QIcon icon(file);	 
		_lastUsedMenuForegroundColor = Qt::white;
		_SetIconColor(icon, config.Menu);
// ??? TODO
	}
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
	QString s	 = CONFIGS_USED::NameForConfig(true, ".ini"),
			sp = s.left(s.lastIndexOf('/'));	// path
	s = s.mid(s.lastIndexOf('/') + 1);			// name
	QMessageBox(QMessageBox::Information,
		QString("falconG"),
		QString(QMainWindow::tr("Saved configuration\n'%1'\n into folder \n'%2'").arg(s).arg(sp)),
		QMessageBox::Ok,
		this).exec();
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
void FalconG::on_btnForeground_clicked()
{
	if (_busy)
		return;

	_CElem* pElem = _PtrToElement();

	QColor qc(pElem->color.Name()),
		   qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;

	pElem->color.SetColor(qcNew.name());
	ui.btnForeground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->color.Name()));

	_SetConfigChanged(true);
	_ElemToSample();
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

	_CElem* pElem = _PtrToElement();

	QColor qc(pElem->background.Name()),
		   qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;

	pElem->background.SetColor(qcNew.name());
	ui.btnBackground->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(pElem->background.Name()));
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnPageColor_clicked()
{
	QColor qc(config.Web.color.Name()),
		qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;

	config.Web.color.SetColor(qcNew.name());
	ui.btnPageColor->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.color.Name()));
	_SetConfigChanged(true);
	AlbumElement ae = _aeActiveElement;
	_aeActiveElement = aeWebPage;
	_ElemToSample();
	_aeActiveElement = ae;
// DEBUG
//	ShowStyleOf(ui.btnPageColor);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnPageBackground_clicked()
{
	if (_busy)
		return;

	QColor qc(config.Web.background.Name()),
		qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;

	config.Web.background.SetColor(qcNew.name());
	ui.btnPageBackground->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.background.Name()));
	_SetConfigChanged(true);

	AlbumElement ae = _aeActiveElement;
	_aeActiveElement = aeWebPage;
	_ElemToSample();
	_aeActiveElement = ae;
}


/*========================================================
 * TASK: browse for background image
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - should svae last directory into config
 *-------------------------------------------------------*/
void FalconG::on_btnBrowseForBackgroundImage()
{
	static QString _qsLastBackgroundImagePath = "./";
	QString filename = QFileDialog::getOpenFileName(this,
													tr("falconG - Open background image"), 
													_qsLastBackgroundImagePath,
													"Image Files (*.png *.jpg)");
	if (!filename.isEmpty())
	{
		ui.edtBckImageName->setText(filename);
		QPixmap pm(filename);
		ui.lblbckImage->setPixmap(pm);
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
	_CElem* pElem = _PtrToElement();

	QColor qc(pElem->gradient.Color(gsStart)),
		   qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;
	pElem->gradient.Set(gsStart, ui.sbGradStartPos->value(), qcNew.name());
	_SetWidgetGradientQt(ui.lblGradient, pElem);

	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradMiddleColor_clicked()
{
	_CElem* pElem = _PtrToElement();

	QColor qc(pElem->gradient.Color(gsMiddle)),
		   qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;
	pElem->gradient.Set(gsMiddle, ui.sbGradMiddlePos->value(), qcNew.name());
	_SetWidgetGradientQt(ui.lblGradient, pElem);

	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnGradStopColor_clicked()
{
	_CElem* pElem = _PtrToElement();

	QColor qc(pElem->gradient.Color(gsStop)),
		   qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (!qcNew.isValid() || qc == qcNew)
		return;
	pElem->gradient.Set(gsStop, ui.sbGradStopPos->value(), qcNew.name());
	_SetWidgetGradientQt(ui.lblGradient, pElem);

	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnImageBorderColor_clicked()
{
	StyleHandler handler(ui.btnImageBorderColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color")),
			qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew.isValid() && qc != qcNew)
	{
		handler.SetItem("QToolButton", "background-color", qcNew.name());
		handler.SetItem("QToolButton", "color", config.Web.background.Name());
		ui.btnImageBorderColor->setStyleSheet(handler.StyleSheet());
		BorderSide side = (BorderSide)(ui.cbBorder->currentIndex()-1);
		config.imageBorder.SetColor(side, qcNew.name());
		on_sbImageBorderWidth_valueChanged(ui.sbBorderWidth->value());
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

	_CElem *pElem = _PtrToElement();
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	QColor qc = pElem->shadow1[which].Color(),
			qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qc != qcNew)
	{
		ui.btnShadowColor->setStyleSheet("QToolButton {\nbackground-color:" + qc.name() + ";\n}\n");
		_SetConfigChanged(true);
	}
	_ElemToSample();
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
				 "<html><head/><body><p>With certain linux themes the color of some elements"
				 " on the right may differ from the ones you set on the left.<br>"
				 "A clear indication is a background mismatch between the color of the "
				 "<span style=\" font - style:italic;\">Gallery title</span> area on the top and the "
				 "main page area below it.</p></body></html>", QMessageBox::Ok, this).exec();
}

void FalconG::_ModifyGoogleFontImport()
{
	static QString name = "Resources/falconG.css",
		tmpName = name + ".tmp";
	QFile in(name),
		out(tmpName);
	in.open(QIODevice::ReadOnly),
		out.open(QIODevice::WriteOnly);

	QTextStream ifs(&in), ofs(&out);
	QString line = ifs.readLine(),	// first line may be the font import line
			savedLine;

	if (line.left(7) != "@import")	// did not start with font import: save line for after inserting @import later
		savedLine = line;

	line = QString("@import url(\"https://fonts.googleapis.com/css?family=" + config.sGoogleFonts.ToString() + "\"");
	ofs << line << "\n";
	// TODO put fonts directory handling here
	if (!savedLine.isEmpty())
		ofs << savedLine << "\n";
	while (!(line = ifs.readLine()).isEmpty())
		ofs << line << "\n";
	in.close();
	out.close();
	BackupAndRename(name, tmpName);
	// reload page
	_page.triggerAction(QWebEnginePage::Reload);
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
	int bli = ui.cbBaseLanguage->currentIndex(),	// text(s) in this language is the base
		li = ui.cbLanguage->currentIndex();		// text(s) in this language may have been changed
	
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


/*========================================================
 * TASK:	slot for link clicked in sample (WebEnginePage)
 * PARAMS:	s -	link name a decimal number string
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::LinkClicked(QString s)
{
	if (!s.at(0).isDigit())
		return;

	QUrl url(s);
	int a = url.toString(QUrl::DecodeReserved).toInt();
	ui.cbActualItem->setCurrentIndex(a);
}


/*========================================================
 * TASK:	signal for web page loaded
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - before switching to page 'Design' the
 *				web page did not loaded therefore all of 
 *				our changes are lost
 *-------------------------------------------------------*/
void FalconG::WebPageLoaded(bool ready)
{
	static bool loaded = false;		// only check load once
	if (loaded)
		return;
	loaded = true;

	_ActualSampleParamsToUi();
	_ConfigToSample(); // to sample "WEB page"
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
	ui.edtWmHorizMargin->setEnabled(index != 1);
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
	ui.edtWmVertMargin->setEnabled(index != 1);
}

/*=============================================================
 * TASK:		 language 1 index changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_cbBaseLanguage_currentIndexChanged(int index)
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
void FalconG::on_cbLanguage_currentIndexChanged(int index)
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
	_selection.changed |= fsTitle;
	if (ui.cbBaseLanguage->currentIndex() == ui.cbLanguage->currentIndex())
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
	_selection.changed |= fsDescription;
	if (ui.cbBaseLanguage->currentIndex() == ui.cbLanguage->currentIndex())
		ui.edtBaseDescription->setPlainText(ui.edtDescriptionText->toPlainText());
	ui.btnSaveChangedDescription->setEnabled(true);
	--_busy;
}


/*========================================================
 * TASK:	the three function below change the program style
 * PARAMS:	if the given radio button is checked
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_rbDefaultStyle_toggled(bool on)
{
	if (on)
		frmMain->_StyleTheProgram(stDefault);
}
void FalconG::on_rbSystemStyle_toggled(bool on)
{
	if (on)
		frmMain->_StyleTheProgram(stSystem);
}
void FalconG::on_rbDarkStyle_toggled(bool on)
{
	if (on)
		frmMain->_StyleTheProgram(stDark);
}
void FalconG::on_rbBlackStyle_toggled(bool on)
{
	if (on)
		frmMain->_StyleTheProgram(stBlack);
}
void FalconG::on_rbBlueStyle_toggled(bool on)
{
	if (on)
		frmMain->_StyleTheProgram(stBlue);
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

void FalconG::on_btnGoToGoogleFontsPage_clicked()
{
	QDesktopServices::openUrl(QUrl("https://fonts.google.com/"));
}

void FalconG::on_cbActualItem_currentIndexChanged(int newIndex)
{
	_aeActiveElement = (AlbumElement)newIndex;
	ui.gbGradient->setEnabled(newIndex);
	ui.gbBorder->setEnabled(newIndex);
	int page = newIndex == aeThumb ? 2 : newIndex ? 1 : 0;
	ui.toolBox->setCurrentIndex(page);	// to settings
	_ActualSampleParamsToUi();
}

void FalconG::on_cbBorder_currentIndexChanged(int newIndex)
{
	_CElem* pElem = _PtrToElement();
	BorderSide side = (BorderSide) (newIndex -1);
	//if (!_busy)
	//	pElem->border.SetWidth(side, ui.sbBorderWidth->value());
	++_busy;
   ui.sbBorderWidth->setValue( pElem->border.Width(side) );
	--_busy;
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
}

void FalconG::on_cbBorderStyle_currentIndexChanged(int newIndex)
{
	_CElem* pElem = _PtrToElement();
	if (!_busy)
	{
		BorderSide side = (BorderSide)(ui.cbBorder->currentIndex()-1);
		pElem->border.SetStyleIndex(side, ui.cbBorderStyle->currentIndex());
	}
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
}

void FalconG::on_chkFacebook_toggled(bool on)
{
	config.bFacebookLink.v = on;
}

void FalconG::on_chkDebugging_toggled(bool b)
{
	config.bDebugging = b;
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
	if (!elem.gradient.used)
		return elem.background.Name();
	else
		return elem.gradient.ForQtStyleSheet(invert);
}

void FalconG::_SetWidgetGradientQt(QWidget* pwidget, _CElem* pElem, bool invert)
{
	pwidget->setStyleSheet(QString("background:%1;").arg(pElem->gradient.ForQtStyleSheet(invert)));
}


/*=============================================================
 * TASK:	Enable/disable gradient  edits, buttons, and positions
 * EXPECTS:	ena: 0: disable, 
 *				 1: enable, 
 * GLOBALS: config
 * RETURNS:
 * REMARKS:	does not change config
 *------------------------------------------------------------*/
void FalconG::_EnableGradient(bool ena)
{
	static QString ct, cm, cb;										// top, middle and bottom colors for gradient buttons
	const QString ss = "QToolButton {background-color:#aaa;}";		// disabled color
	const QString css = "QToolButton {background-color:";
	_CElem* pElem = _PtrToElement();

	++_busy;

	if (!ena)
	{
		ui.btnGradStartColor->setStyleSheet(ss);
		ui.btnGradMiddleColor->setStyleSheet(ss);
		ui.btnGradStopColor->setStyleSheet(ss);
		ui.gbGradient->setEnabled(false);
	}
	else if(pElem->gradient.used)			  // enable
	{
		ui.btnGradStartColor->setStyleSheet(css + pElem->gradient.gs[0].color + ";}");
		ui.btnGradMiddleColor->setStyleSheet(css + pElem->gradient.gs[1].color + ";}");
		ui.btnGradStopColor->setStyleSheet(css + pElem->gradient.gs[2].color + ";}");
		ui.sbGradStartPos->setValue(pElem->gradient.gs[0].percent);
		ui.sbGradMiddlePos->setValue(pElem->gradient.gs[0].percent);
		ui.sbGradStopPos->setValue(pElem->gradient.gs[0].percent);
		ui.gbGradient->setEnabled(true);
	}
	--_busy;
}

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
		elem.background.SetOpacity(val);
	}
	if((which & 1) == 0 )
	{
		val = (ui.chkTextOpacity->isChecked() ? 1 : -1)*ui.sbTextOpacity->value();
		elem.color.SetOpacity(val);
	}
}

//-------------------------------------------------------------------------------

/*========================================================
 * TASK: functions below set styles of sample from config
 * PARAMS:	parent: to check against. If nullptr use config.Web
 * GLOBALS:	config
 * RETURNS: nothing
 * REMARKS: - if any value is the same as for the parent
 *				it will not be set
 *-------------------------------------------------------*/

void FalconG::_SetColor(_CElem* pElem)
{
	QString qs;
	if (pElem == &config.Web || (pElem->parent && pElem->color != pElem->parent->color) ) // else clear color
		qs = pElem->color.ForStyleSheet(false, false);
	_SetCssProperty(pElem, qs);
}

/*========================================================
 * TASK:	The 9 functions below prepare the property
 *			 and value into sPropty amd sValue
 * PARAMS:	sPropty, sValue: string references
 * GLOBALS:	config
 * RETURNS:
 * REMARKS: - the changed parameters must already saved to config
 *			- to clear a parameter set sValue to empty string
 *-------------------------------------------------------*/
void FalconG::_SetBackground(_CElem* pElem)
{
	auto clearBackground = [&](AlbumElement what) 
	{
		_CElem* pe = _PtrToElement(what);
		_SetCssProperty(pe, "background-color:");
	};

	if (pElem->kind == aeWebPage)
	{
		if (ui.chkSetAll->isChecked())		// then remove all other background colors
		{
			clearBackground(aeHeader);
			clearBackground(aeMenuButtons);
			clearBackground(aeLangButton);
			clearBackground(aeGalleryTitle);
			clearBackground(aeGalleryDesc);
			clearBackground(aeSection);
			clearBackground(aeGalleryTitle);
			clearBackground(aeGalleryDesc);
			clearBackground(aeLightboxTitle);
			clearBackground(aeLightboxDescription);
			clearBackground(aeFooter);
		}
		_SetCssProperty(pElem, pElem->background.ForStyleSheet(false, true));
	}
	else
		_SetCssProperty(pElem,( (pElem->parent && pElem->parent->background != pElem->background) || !pElem->parent ? pElem->background.ForStyleSheet(false, true) : QString("background-color:") ) );
}

void FalconG::_SetShadow(_CElem* pElem,int what)
{
	QString sPropty, sValue;
	if (ui.chkShadowOn->isChecked())
		sValue = pElem->shadow1[what].ForStyleSheet(false, false, what) + pElem->shadow2[what].ForStyleSheet(false, true, what);
	_SetCssProperty(pElem, sPropty + sValue);
}

void FalconG::_SetBorder(_CElem* pElem)
{
	QString qs;
	if(pElem == &config.Web || !pElem->parent || pElem->parent->border != pElem->border)
		 qs = pElem->border.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
}

void FalconG::_SetLinearGradient(_CElem* pElem)
{
	QString qs;
	if(pElem == &config.Web || !pElem->parent || pElem->parent->gradient != pElem->gradient	)
		 qs = pElem->gradient.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
}

void FalconG::_SetFont(_CElem* pElem)
{
	QString qs;
	if (pElem == &config.Web || !pElem->parent || pElem->parent->font != pElem->font)
	{
		_SetCssProperty(pElem,  "font-family:" + pElem->font.Family() + "\n" +
								"font-size:"   + pElem->font.SizeStr() + "\n" +
								"font-weight:" + pElem->font.WeightStr() + "\n" +
								"font-style:" + pElem->font.ItalicStr());
//		 qs = pElem->font.IsFirstLineDifferent() ? pElem->font.FirstLineFontSizeStr() : pElem->font.SizeStr();
//		_SetCssProperty(pElem, "font-size:" + qs, "::first-line");	// parent = Web and ::first-line
	}
	else
	{
		_SetCssProperty(pElem,	"font-family:\n"
								"font-size:\n"
								"font-weight:\n"
								"font-style:");
//		qs = pElem->font.IsFirstLineDifferent() ? pElem->font.FirstLineFontSizeStr() : pElem->font.SizeStr();
//		_SetCssProperty(pElem, "font-size", qs, "::first-line");	// parent = Web and ::first-line
	}
}

void FalconG::_SetDecoration(_CElem* pElem)
{
	QString qs;
	if (!pElem->decoration.IsTextDecorationLine())
		_SetCssProperty(pElem, "text-decoration-line:");
	else if (pElem == &config.Web || !pElem->parent || pElem->parent->decoration != pElem->decoration)
	{
		_SetCssProperty(pElem, "text-decoration-line:"   +  pElem->decoration.TextDecorationLineStr() + "\n" + 
								"text-decoration-style:" +  pElem->decoration.TextDecorationStyleStr() + "");
	}
	else
	{
		_SetCssProperty(pElem,	"text-decoration-line:\n"
								"text-decoration-style:");
	}

}

void FalconG::_SetTextAlign(_CElem* pElem)
{
	QString qs;
	if (pElem == &config.Web || !pElem->parent || pElem->parent->alignment != pElem->alignment)
		qs = pElem->alignment.ActAlignStr();
	_SetCssProperty(pElem,"text-align:"+ qs);
}

void FalconG::_SetPageColor(_CElem* pElem)
{
	_SetCssProperty(pElem,"color:" + pElem->color.Name());
}

void FalconG::_SetPageBackground(_CElem* pElem)
{
	_SetCssProperty(pElem,"background:" + pElem->background.Name());
}

void FalconG::_SetIcon()		// only for 'menu-item#uplink' and the icon always has the same name
{								// but its color will change. Will it be visible without setting it again? And this way? TODO Check???
	_RunJavaScript("menu-item#uplink",QString("background-image:")+"url(file:///res/up-icon-sample.png)");
}


/*========================================================
 * TASK:	only used for falconG and not in production
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_SetWatermark()
{
			// set text
	_RunJavaScript(".thumb::after","content:" + config.waterMark.wm.text);
			// set offsets
	bool  centered;
	WaterMark &wm = config.waterMark.wm;
	QString qs = wm.XMarginName(centered, false);		// X offset  -clear unused property
	_RunJavaScript(".thumb::after", qs +  wm.OffsetXToStr());
	qs = wm.XMarginName(centered, true);				// set used property												   
	_RunJavaScript(".thumb::after",qs + wm.OffsetXToStr());

	qs = wm.YMarginName(centered, false);				// Y offset clear unused property
	_RunJavaScript(".thumb::after",qs + wm.OffsetYToStr());
	qs = wm.YMarginName(centered, true);				// set used property												   
	_RunJavaScript(".thumb::after",qs + wm.OffsetYToStr());
			// set color
	_RunJavaScript(".thumb::after","color:" + wm.ColorToStr());
}


/*============================================================================
  * TASK:	store changed opacity for text or background
  * EXPECTS: val - value of new opacity in percent
  *			 which: 1 -> text, 2: background	
  * GLOBALS:
  * REMARKS: 
 *--------------------------------------------------------------------------*/
void FalconG::_OpacityChanged(int val, int which)
{
	_CElem* pElem = _PtrToElement();
	if (which == 1)
		pElem->color.SetOpacity(val);
	else
		pElem->background.SetOpacity(val);
	_SetConfigChanged(true);
	_SetCssProperty(pElem, which == 1 ? pElem->color.ForStyleSheet(false,false) : pElem->background.ForStyleSheet(false, true));
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

	if (_selection.changed & fsTitle)
	{
		_selection.title.SetTextForLanguageNoID(ui.edtTitleText->document()->toPlainText(), _selection.edtLang);
		_selection.title.CalcBaseID();		// new base ID for title
	}
	if (_selection.changed & fsDescription)
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

	_selection.changed = fsNothing;				// all changes saved

	albumgen.WriteDirStruct(true);		// keep previous backup file
}

void FalconG::_SetLayoutMargins(skinStyle which)
{
	int layoutTop = 9;
	QMargins margins;

	auto modifyLayout = [&](QWidget* pw)
	{
		margins = pw->layout()->contentsMargins();
		margins.setTop(layoutTop);
		pw->layout()->setContentsMargins(margins);
	};

	switch (which)
	{
		case stDefault:
		default:
			layoutTop = 9;
			break;
		case stSystem:
		case stBlue:
		case stDark:
		case stBlack:
			layoutTop = 18;
			break;
	}
	modifyLayout(ui.gbLocalMachine);
	modifyLayout(ui.gbServer);
	modifyLayout(ui.gbAdvanced);
	modifyLayout(ui.gbGallery);
	modifyLayout(ui.gbLatest);

	modifyLayout(ui.gbGlobalColors);
	modifyLayout(ui.gbBackgroundImage);

	modifyLayout(ui.gbColors);
	modifyLayout(ui.gbFont);
	modifyLayout(ui.gbTextStyle);
	modifyLayout(ui.gbDecorationStyle);
	modifyLayout(ui.gbTextShadow);
	modifyLayout(ui.gbGradient);

	modifyLayout(ui.gbImageDecoration);
	modifyLayout(ui.gbResizing);

	modifyLayout(ui.gbWatermark);
	modifyLayout(ui.gbWatermarkFont);
	modifyLayout(ui.gbVerticalPosition);
	modifyLayout(ui.gbHorizontalPosition);
	modifyLayout(ui.gbWmTextShadow);

	modifyLayout(ui.gbSample);

}

void FalconG::_StyleTheProgram(skinStyle which)
{
	_SetLayoutMargins(which);

		// change these for new color set
							//			   def. system    blue		 dark		  black		 
	static QString sBackground[]		= { "", "",		"#3b5876", "#282828",	"#191919" }, // %1  background
				   sTextColor[]			= { "", "",		"#cccccc", "#cccccc",	"#e1e1e1" }, // %2  foreground
				   sBorderColor[]		= { "", "",		"#747474", "#4d4d4d",	"#323232" }, // %3  border
				   sFocusedInput[]		= { "", "",		"#f0f0f0", "#f0f0f0",	"#f0f0f0" }, // %4  focused text
				   sHoverColor[]		= { "", "",		"#8faed2", "#383838",	"#282828" }, // %5  hover over input
				   sTabBorder[]			= { "", "",		"#3b589b", "#9B9B9B",	"#9b9b9b" }, // %6
				   sInputBackground[]	= { "", "",		"#1c3a55", "#454545",	"#323232" }, // %7  editor backgrounds
				   sSelectedInputBgr[]	= { "", "",		"#3b584a", "#666666",	"#4a4a4a" }, // %8
				   sFocusedBorder[]		= { "", "",		"#92b1d5", "#c0c0c0",	"#a8a8a8" }, // %9
				   sDisabledBg[]		= { "", "",		"#697a8e", "#555555",	"#191919" }, // %10
				   sImageBackground[]	= { "", "",		"#12273f", "#111111",	"#000000" }, // %11
				   sPressedBg[]			= { "", "",		"#555555", "#555555",	"#323232" }, // %12 button pressed
				   sDefaultBg[]			= { "", "",		"#555555", "#555555",	"#323232" }, // %13
				   sProgressBarChunk[]	= { "", "",		"#e28308", "#e28308",	"#e28308" }, // %14
				   sWarningColor[]		= { "", "",		"#f0a91f", "#f0a91f",	"#f0a91f" }, // %15
				   sBoldTitleColor[]	= { "", "",		"#e28308", "#e28308",	"#e28308" }	 // %16	 GroupBox title
	;

	 // theme style string used only when not the default style is used
	static QString styles = {
	R"END(							 
* {
	background-color:%1;       /* %1 background */
	color:%2;                  /* %2 color */
}
        
/* ------------------- geometry ------------------*/        

QTabWidget::tab-bar {
    left: 5px; /* move to the right by 5px */
}
QTabBar::tab {
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
	min-width: 20ex;
	padding: 2px;
}        
QTabBar::tab:!selected {
    margin-top: 2px; /* make non-selected tabs look smaller */
}

QToolTip {
    border-radius: 4px;
}

QProgressBar,
QPushButton:flat {
	border:0;
}
QProgressBar::chunk{
	width: 10px;
}

QToolButton {
	border:0;
	border-radius:12px;
	height:24px;
	width:24px;
}
#btnSelectUplinkIcon {
	width:32em;
}

QTextEdit, 
QLineEdit,
QSpinBox {
    height:20px;
	padding: 0 8px;
}
QPushButton {
	padding:2px 4px;
}
QRadioButton::indicator,   
QCheckBox::indicator {
	width: 13px;
	height: 13px;
}

QRadioButton, QCheckBox {
	spacing: 5px;
}

QTextEdit, 
QLineEdit,
QSpinBox,
QPushButton,
QTreeView, 
QListView {
    border-radius: 10px;
}
        
/* ------------------ borders ----------------------*/   
QTabWidget:pane,     
QTabBar::tab, 
QToolTip,
QTextEdit, 
QLineEdit,
QGroupBox,
QSpinBox,
QPushButton,
QTreeView, 
QListView {
    border: 2px solid %3;          /* %3 border color */	
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QGroupBox::title {
	color:%16
}

QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%4;                      /* %4  focused */
}

#menuWidget {
	background-color:rgb(0,0,0,0);
}
QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;          /* %5 background hover + TAB selected*/
}
QTabBar::tab:selected {
	border-color:%6;              /* %6 border of selected TAB */
    border-bottom-color:%1;       /* %1 same as panel background*/
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%7;           /* %7 input background */
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%8; /* %8 selected text in inputs*/
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;               /* %9 border of focused */
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled {
	background:%10;                 /* %10 read only and disabled */
}

#lblBckImage,
#groupBox_6 {
	background-color:%11;           /* %11 sample area background and background image backround*/
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%12;           /* %12 button backgrounds */
}

QPusButton:default {
    background-color:%13;           /* %13 button backgrounds */
}

QProgressBar::chunk{
	background-color:%14;          /* %14 signal actual elem*/
}

#lblActualElem {                        /* %15 signal actual elem*/
	color:%15;
}
)END"
	};
	config.styleIndex = (int)which;
	if (which)
	{
		QString ss =
			QString(styles)
			.arg(sBackground[which])		// %1 
			.arg(sTextColor[which])			// %2 
			.arg(sBorderColor[which])		// %3 
			.arg(sFocusedInput[which])		// %4 
			.arg(sHoverColor[which])		// %5 
			.arg(sTabBorder[which])			// %6 
			.arg(sInputBackground[which])	// %7
			.arg(sSelectedInputBgr[which])	// %8 
			.arg(sFocusedBorder[which])		// %9 
			.arg(sDisabledBg[which])		// %10
			.arg(sImageBackground[which])	// %11
			.arg(sPressedBg[which])			// %12
			.arg(sDefaultBg[which])			// %13
			.arg(sProgressBarChunk[which])	// %14
			.arg(sWarningColor[which])		// %15
			.arg(sBoldTitleColor[which])	// %16
			;

		if (which == stBlue)		// blue
			ss += "QCheckBox::indicator:checked{"
				  "	  image: url(:/icons/Resources/blue-checked.png);"
			      "}";
// DEBUG
//			DEBUG_LOG(ss)

		frmMain->setStyleSheet(ss);

		//QFile f("style.txt");
		//f.open(QIODevice::WriteOnly);
		//QTextStream textstream(&f);
		//textstream << ss;

	}
	else
			frmMain->setStyleSheet("");

}

/*============================================================================
  * TASK:		changes the color of an icon to the one set in 'elem'
  * EXPECTS: icon - existing icon with only transparent and white pixels
  *			 elem - _CElem with the 'color' set
  * RETURNS: modified icon
  * GLOBALS: 
  * REMARKS: 
 *--------------------------------------------------------------------------*/
void FalconG::_SetIconColor(QIcon &icon, _CElem & elem)
{
	QPixmap pm;
	pm = icon.pixmap(64, 64);
	QBitmap mask = pm.createMaskFromColor(_lastUsedMenuForegroundColor, Qt::MaskOutColor);
	// save previous menu button color
	// do not change this _lastUsedMenuForegroundColor = elem.color.Name();
	pm.fill(elem.color.Name());
	pm.setMask(mask);
	icon = QIcon(pm);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_btnSaveStyleSheet_clicked()
{
	if (albumgen.SaveStyleSheets() == 0)
	{
		QString s = (config.dsGallery + config.dsGRoot + config.dsCssDir).ToString();
		QMessageBox(QMessageBox::Information,
					QString("falconG"),
					QString(QMainWindow::tr("Saved style sheet 'falconG.css'\ninto %1").arg(s)),
					QMessageBox::Ok,
					this).exec();
	}
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
	ui.cbBaseLanguage->clear();
	ui.cbLanguage->clear();
	for (int i = 0; i < Languages::Count(); ++i)
	{			 // edit texts page
		ui.cbBaseLanguage->addItem(Languages::names[i] + " (" + Languages::abbrev[i] + ")");
		ui.cbLanguage->addItem(Languages::names[i] + " (" + Languages::abbrev[i] + ")");
	}
	++_busy;
	if (Languages::Count() > 1)
	{
		ui.cbBaseLanguage->setCurrentIndex(0);
		ui.cbLanguage->setCurrentIndex(1);
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

/*============================================================================
  * TASK:
  * EXPECTS:  re-create icon file
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::_CreateUplinkIcon(QString destPath, QString destName)
{
	QString src = "res/"+destName;
	QPixmap pm(src) ;
	if (pm.isNull())
	{
		QMessageBox::warning(this, tr("falconG"), QString(tr("Warning\nCan't read icon\n'%1'")).arg(src));
		return;
	}
	QFile::remove(destPath + destName);
	QIcon icon(pm);
	_SetIconColor(icon, config.Menu);
	pm = icon.pixmap(64,64);
	pm.save(destPath + destName);
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


/*========================================================
 * TASK:	calls the javascript function to set one parameter
 * PARAMS:	className - css selector
 *			value - string of css styles separated by '\n'
 * GLOBALS:
 * RETURNS:
 * REMARKS: - 
 *-------------------------------------------------------*/
void FalconG::_RunJavaScript(QString className, QString value)
{
	QStringList qsl = value.split('\n');
	int pos;
	QString qs;	 

	auto runOneTag = [&](QString &s)
	{
		pos = s.indexOf(':');
		if (pos <= 0) 
			return;
		qs = QString("SetPropertyForClass('" + className + "','" + s.left(pos) + "','" + s.mid(pos+1) + "')");
		_page.runJavaScript(qs);
//		DEBUG_LOG(qs)
	};
	for (auto s : qsl)
		runOneTag(s);
}

void FalconG::_SetCssProperty(_CElem*pElem, QString value, QString subSelector)
{
	QString className = pElem->ClassName();
	if (!subSelector.isEmpty())
		className += subSelector;
	_RunJavaScript(className, value);
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
