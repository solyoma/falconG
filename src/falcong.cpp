#include <QCloseEvent>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QDirIterator>
#include <QListView>
#include <QDesktopServices>
#include <QUrl>

#include <string>

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"
#include "falcong.h"
#include "structEdit.h"
#include "sourcehistory.h"
#include "csscreator.h"


#define DEBUG_LOG(qs) \
{							 \
	QFile f("debug.log");		 \
	f.open(QIODevice::Append);	 \
	QTextStream ofsdbg(&f);		 \
	ofsdbg << qs << "\n";			 \
}
#define DEBUG_LOG_NEW(qs) \
{							 \
	QFile f("debug.log");		 \
	f.open(QIODevice::WriteOnly);	 \
	QTextStream ofsdbg(&f);		 \
	ofsdbg << qs << "\n";			 \
}


FalconG *frmMain = nullptr;
FalconGScheme FSchemeVector::blue("Blue",
	"#3b5876",
	"#cccccc",
	"#747474",
	"#f0f0f0",
	"#8faed2",
	"#3b589b",
	"#1c3a55",
	"#3b584a",
	"#92b1d5",
	"#999999",
	"#697a8e",
	"#12273f",
	"#555555",
	"#555555",
	"#feb60e",
	"#f0a91f",
	"#e28308"
),
FSchemeVector::dark("Dark",
	"#282828",
	"#cccccc",
	"#4d4d4d",
	"#f0f0f0",
	"#383838",
	"#9B9B9B",
	"#454545",
	"#666666",
	"#c0c0c0",
	"#999999",
	"#555555",
	"#111111",
	"#555555",
	"#555555",
	"#feb60e",
	"#f0a91f",
	"#e28308"
), 							   
FSchemeVector::black("Black",
	"#191919",
	"#e1e1e1",
	"#323232",
	"#f0f0f0",
	"#282828",
	"#9b9b9b",
	"#323232",
	"#4a4a4a",
	"#a8a8a8",
	"#999999",
	"#191919",
	"#000000",
	"#323232",
	"#323232",
	"#feb60e",
	"#f0a91f",
	"#e28308"
);


/*------------------------------------- macros ------------------------------------*/

// DBUG
void ShowStyleOf(QWidget *pq) 
{
	QMessageBox msgb; msgb.setText(pq->styleSheet()); 
	msgb.setInformativeText("\nStyle for " + pq->objectName());
	msgb.setWindowTitle("falconG - Style for " + pq->objectName()); 
	msgb.exec();
}
// /DEBUG

/*============================================================================
* TASK: format color QString into CSS rgba() when opacity is used, use original
*		 hex color QString if not. 
* EXPECTS: color - to transform
* RETURNS: color string starting w. '#'
*----------------------------------------------------------------------------*/
QString ColorToCss(_CColor color)
{
	QString c = color.Name();	// c starts with '#'
	int opacity = color.Opacity(true);	// in percent
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



/*========================================================
 * TASK:	copies file from resource to HD
 * PARAMS:	resPath   - path in resource data 
 *			name file - name to copy
 *			overwrite - existing files?
 * GLOBALS:
 * RETURNS:
 * REMARKS: - existing files kept
 *			- QFile::copy() can't copy from resource
 *			- files copied into folders inside 'sample'
 *				folder in program directory
 *-------------------------------------------------------*/
static bool _CopyResourceFileToSampleDir(QString resPath, QString name, bool overwrite=false)
{
	QString destDir;
	int pos = name.lastIndexOf('.') + 1;
	switch (name.at(pos).unicode())
	{
		case 'c': destDir = "css/"; break;
		case 'h': destDir = ""; break;		// html
		case 'j': destDir = name.at(++pos).unicode() == 's' ? "js/" : "res/"; break;	// .js or .jpg?
		default: destDir  = "res/"; break;
	}
	destDir = CONFIGS_USED::_samplePath + destDir;

	resPath += name;
	if ( ! QFile::exists(destDir + name))
	{
		QFile f(resPath);
		QFileInfo fi(f);
		if (!f.open(QIODevice::ReadOnly))
			goto ERR;
		QByteArray ba = f.read(fi.size());
		f.close();
		f.setFileName(destDir + name);
		if (f.open(QIODevice::WriteOnly) )
		{
			if (f.write(ba) < 0)
				goto ERR;
			return true;
		}
	}
	return true;	// not copied
ERR:
	QString serr = QObject::tr("Can't copy resource'%1' to folder '%2'\nPlease make sure the folder is writeable!\n\nExiting");
	QMessageBox::critical(nullptr, "falconG", serr.arg(name).arg(destDir));
	exit(1);
}

/*============================================================================
* TASK:		construct a FalconG object, read its parameters, set up controls
*			and sample web page
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FalconG::FalconG(QWidget *parent) : QMainWindow(parent)
{
	CONFIGS_USED::GetHomePath();

	config.dsApplication = QDir::current().absolutePath(); // before anybody changes the current directory

	// create directories for sample
	bool ask = false;
	CreateDir(CONFIGS_USED::_samplePath, ask);
	ask = false;
	CreateDir(CONFIGS_USED::_samplePath+"css", ask);
	ask = false;
	CreateDir(CONFIGS_USED::_samplePath+"js", ask);
	ask = false;
	CreateDir(CONFIGS_USED::_samplePath+"res", ask);

	QString resPPath = QStringLiteral(":/Preview/Resources/"),
			resIPath = QStringLiteral(":/icons/Resources/");
	_CopyResourceFileToSampleDir(resPPath, "index.html");
	_CopyResourceFileToSampleDir(resPPath, "falconG.css");
	_CopyResourceFileToSampleDir(resPPath, "falconG.js");
	_CopyResourceFileToSampleDir(resPPath, "placeholder.png");
	_CopyResourceFileToSampleDir(resPPath, "placeholder2.png");
	_CopyResourceFileToSampleDir(resPPath, "NoImage.jpg");

	_CopyResourceFileToSampleDir(resIPath, "up-icon.png");

	ui.setupUi(this);
	ui.pnlProgress->setVisible(false);

	connect(&_page, &WebEnginePage::LinkClickedSignal, this, &FalconG::LinkClicked);
	connect(&_page, &WebEnginePage::loadFinished, this, &FalconG::WebPageLoaded);


	ui.sample->setPage(&_page);

#ifdef __linux__	
	QString url = "file://" + CONFIGS_USED::_samplePath+"/index.html";
	_page.load(QUrl(url));
#else
	_page.load(QUrl("file:///"+ CONFIGS_USED::_samplePath+"index.html"));
#endif

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

	// read styles
	_schemes.ReadAndSetupStyles();

	// setup style change menus. _SlotForContextMenus sets up a signal mapping for style menus
	ui.tabGallery->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tabGallery, &QWidget::customContextMenuRequested, this, &FalconG::_SlotForContextMenu);
	ui.tabDesign->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tabDesign, &QWidget::customContextMenuRequested, this, &FalconG::_SlotForContextMenu);
	ui.tabEdit->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tabEdit, &QWidget::customContextMenuRequested, this, &FalconG::_SlotForContextMenu);
	ui.lblOptionsHelp->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.lblOptionsHelp, &QWidget::customContextMenuRequested, this, &FalconG::_SlotForContextMenu);
	ui.pnlSchemeEmulator->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.pnlSchemeEmulator, &QWidget::customContextMenuRequested, this, &FalconG::_SlotForContextMenu);


	_pCbColorSchemeEdit = ui.cbColorScheme->lineEdit();
	connect(_pCbColorSchemeEdit, &QLineEdit::editingFinished, this, &FalconG::_SlotForSchemeComboEditingFinished);

	QSettings s(CONFIGS_USED::_homePath+falconG_ini, QSettings::IniFormat);	// in program directory

	restoreGeometry (s.value("wgeometry").toByteArray());
	restoreState(s.value("wstate").toByteArray());
	CONFIGS_USED::Read();		// to get last used configuration

	config.Read();				// to read config from

	ui.designSplitter->setSizes({ config.splitterLeft, config.splitterRight } );

	_tmpScheme = _schemes[config.styleIndex];
	_tmpSchemeOrigName = _tmpScheme.MenuTitle;

	frmMain = this;
	// now that everything is ready
	_StyleTheProgram(config.styleIndex);
	_AddSchemeButtons();
	on_cbColorScheme_currentIndexChanged(0);

	_ModifyGoogleFontImport();

	_ConfigToUI();

	config.ClearChanged();

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
* TASK:		prevent closing when an operation is running
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
		return;
	}

	if (ui.chkCleanUp->isChecked())		// directory will be re-created at next program start
		RemoveDir(CONFIGS_USED::_homePath +"sample");

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

	QList<int> splitterSizes = ui.designSplitter->sizes();
	if (config.splitterLeft != splitterSizes.at(0))
	{
		config.splitterLeft = splitterSizes.at(0);
		config.splitterRight = splitterSizes.at(1);
		config.SetChanged(true);
	}

	if (config.Changed())
	{
		int res;
		res  = QMessageBox(QMessageBox::Question, "falconG - Configuration",
			"There are unsaved changes in the configuration\nDo you want to save them?",
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this).exec();
		if (res == QMessageBox::Save)
		{
			//CssCreator cssCreator;
			//cssCreator.Create("falconG.css", true);	// program library setting cursors too
			config.Write();
		}
		else if (res == QMessageBox::Cancel)
			event->ignore();
	}
	else
		CONFIGS_USED::Write();						// save data in program directory

	CssCreator cssCreator;
	cssCreator.Create(CONFIGS_USED::_samplePath+"css/falconG.css", true);	// program library

	QSettings s(CONFIGS_USED::_homePath+falconG_ini, QSettings::IniFormat);	// in program directory
	s.setValue("wgeometry", saveGeometry());
	s.setValue("wstate", saveState());
}

void FalconG::on_toolBox_currentChanged(int newIndex)
{
	if (_busy)
		return;
	++_busy;
	ui.cbActualItem->setCurrentIndex(newIndex);
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

			_ConfigToUI();	// edit values from config

			QString s = ui.edtSourceGallery->text().trimmed();
			if (ui.edtSourceGallery->text().isEmpty())
			{
				++_busy;
				ui.edtSourceGallery->setText(CONFIGS_USED::lastConfigs.at(CONFIGS_USED::indexOfLastUsed));
				--_busy;
			}
			_ConfigToSample();
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
		if (config.Changed())
		{
			CssCreator cssCreator;
			cssCreator.Create(CONFIGS_USED::_samplePath+"css/falconG.css", true);	// program library setting cursors too
		}

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


void FalconG::_SaveLinkIcon()
{
		QIcon icon = _SetUplinkIcon();		// from file (see around line# 2297)
		QString qs = "res/up-link.png";
		QFile::remove(qs);
		qs = "background-image: " + qs;
		icon.pixmap(64,64).save(qs);		// and save into "res" subdirectory
}

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
		_PageColorToSample();
		_PagebackgroundToSample();
	}
	else
	{
		_ColorToSample(pElem);
		_BackgroundToSample(pElem);
	}
	_FontToSample(pElem);
	_SpaceAfterToSample(pElem);
	_DecorationToSample(pElem);
	_ShadowToSample(pElem, 0);	// text
	_ShadowToSample(pElem, 1);	// box
	_SetLinearGradient(pElem);
	_BorderToSample(pElem);
	_TextAlignToSample(pElem);

	// menus: set uplink icon
	if (ae == aeMenuButtons)
	{	_SaveLinkIcon();

		_SetCssProperty(&config.Menu, "background-image:url(\"res/up-link.png\")", "#uplink");
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
		case hAuto:	   ui.rbCenterBckImage->setChecked(true); break;
		case hCover:   ui.rbCoverBckImage->setChecked(true); break;
		case hTile:    ui.rbTileBckImage->setChecked(true); break;
	}
	ui.hsImageSizeToShow->setValue(config.backgroundImage.size);
	if (config.backgroundImage.v != (int)hNotUsed)
	{
		_LoadBckImage(config.backgroundImage.fileName);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}

	--_busy;
}

void FalconG::_ActualSampleParamsToUi()
{
	_CElem* pElem = _PtrToElement();

	if (pElem->kind == aeThumb)		// set in _ConfigToUI and no need to update it
		return;

	++_busy;

	ui.btnForeground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->color.Name()));
	ui.chkTextOpacity->setChecked(pElem->color.Opacity(false) != -1);
	if(pElem->background.Valid())
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->background.Name()));	// sets btnBackground's color
	else // invalid: set from background
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(config.Web.background.Name()));	// sets btnBackground's color
	ui.chkBackgroundOpacity->setChecked(pElem->background.Opacity(false) != -1);

//	_EnableGradient(false);					// disable gradient buttons for background

	ui.chkUseGradient->setChecked(pElem->gradient.used);	// re-enable gradient buttons when checked

	// set shadow colors and values
	ui.gbTextShadow->setEnabled(true);
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;	// 0:text shadow else box shadow
	bool b = pElem->shadow1[which].Used();
	QString sStyle = __ToolButtonBckStyleSheet(b ? pElem->shadow1[which].Color() : "#ddd");	// same color for shadow2
	ui.chkShadowOn->setChecked(b);
	ui.btnShadowColor->setStyleSheet(  sStyle );

	_ShadowForElementToUI(pElem, ui.rbTextShadow->isChecked() ? 0 : 1);

	ui.chkTextOpacity->setChecked(pElem->color.Opacity(false) > 0);
	ui.sbTextOpacity->setValue(pElem->color.Opacity(false) < 0 ? 100: pElem->color.Opacity(true));
	ui.sbTextOpacity->setEnabled(ui.chkTextOpacity->isChecked());

	ui.chkBackgroundOpacity->setChecked(pElem->background.Opacity(false) > 0);
	ui.sbBackgroundOpacity->setValue(pElem->background.Opacity(false) < 0 ? 100 : pElem->background.Opacity(true));
	ui.sbBackgroundOpacity->setEnabled(ui.chkBackgroundOpacity->isChecked());

	ui.sbSpaceAfter->setValue(pElem->spaceAfter.v);

	ui.edtFontFamily->setText(pElem->font.Family());
	int ix = ui.cbFonts->findText(pElem->font.Family());
	if (ix < 0) ix = 0;	// not found -> empty
	ui.cbFonts->setCurrentIndex(ix);

	ui.cbPointSize->setCurrentText(pElem->font.SizeStr());
	ui.cbLineHeight->setCurrentText(pElem->font.LineHeightStr());
	ui.chkBold->setChecked(pElem->font.Bold());
	ui.chkItalic->setChecked(pElem->font.Italic());
	ui.chkTdUnderline->setChecked(pElem->decoration.IsUnderline());
	ui.chkTdOverline->setChecked(pElem->decoration.IsOverline());
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
	BorderSide side = (BorderSide)(ui.cbBorder->currentIndex()-1);

	ui.btnBorderColor->setStyleSheet(QString("QToolButton { background-color:" + pElem->border.ColorStr(side)+";\n color:"+ config.Web.background.Name()+";}\n"));
	ui.sbBorderRadius->setValue(pElem->border.Radius());
	int cnt = pElem->border.BorderCnt();
	ui.cbBorder->setCurrentIndex( cnt == 1 ? 0 : 1);
	ui.cbBorderStyle->setCurrentText(pElem->border.Style(side));
	ui.sbBorderWidth->setValue(pElem->border.Width(side));

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

	_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));

	QString qs = QString("QToolButton {background-color:%1;color:%2;}").arg(config.imageBorder.ColorStr(sdAll)).arg(config.Web.background.Name());
	ui.btnImageBorderColor->setStyleSheet(qs);
	qs = config.imageBorder.ForStyleSheetShort(false);
	_RunJavaScript(".thumb", qs);

	--_busy;
}

void FalconG::_SetConfigChanged(bool on)
{
	ui.btnSaveConfig->setEnabled(config.SetChanged(on));
}


/*========================================================
 * TASK:	set up controls on Design and Watermark page
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_DesignToUi()
{
	++_busy;
				// WaterMark
	StyleHandler handler(ui.btnWmColor->styleSheet());
	handler.SetItem("QToolButton", "background-color", QColor(config.waterMark.wm.Color()).name());
	handler.SetItem("QToolButton", "border", QString("1px solid %1").arg(config.waterMark.wm.BorderColor().name()) );
	ui.btnWmColor->setStyleSheet(handler.StyleSheet());
	ui.sbWmOpacity->setValue( (int)(config.waterMark.wm.Opacity(true)));
	//ui.btnWmColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.Color(),8,16,QChar('0')));
	if(config.waterMark.wm.shadowColor >= 0)
		ui.btnWmShadowColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.wm.shadowColor,6,16,QChar('0')));
		// others
	ui.edtWmHorizMargin->setText(QString().setNum(config.waterMark.wm.marginX));
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.wm.marginY));

	ui.chkImageBorder->setChecked(config.imageBorder.Used());
	ui.chkUseGradient->setChecked(config.Menu.gradient.used);
	ui.chkUseWM->setChecked(config.waterMark.used);

	ui.btnGradMiddleColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[1].color));
	ui.btnGradStartColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[0].color));
	ui.btnGradStopColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[2].color));

	ui.sbImageBorderWidth->setValue(config.imageBorder.Width(sdAll));
	ui.sbImagePadding->setValue(config.imagePadding);
	ui.sbSpaceAfter->setValue(0);

	_SettingUpFontsCombo();
	_GlobalsToUi();	
	--_busy;
}

void FalconG::_OtherToUi()
{
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
	s.replace('|', ',');s.replace('+', ' ');
	ui.edtGoogleFonts->setText(s);
	ui.edtKeywords->setText(config.sKeywords);
	ui.edtServerAddress->setText(config.sServerAddress);
	ui.edtSourceGallery->setText(QDir::toNativeSeparators(config.dsSrc.ToString()));
	ui.edtTrackingCode->setText(config.googleAnalTrackingCode);
	ui.edtUplink->setText(config.sUplink);
	if (!config.dsAlbumDir.IsEmpty() && ui.edtAlbumDir->placeholderText() != config.dsAlbumDir.ToString())
		ui.edtAlbumDir->setText(config.dsAlbumDir.ToString());
	if (!config.sBaseName.IsEmpty() && ui.edtBaseName->placeholderText() != config.sBaseName.ToString())
		ui.edtBaseName->setText(config.sBaseName.ToString());
	if (!config.dsFontDir.IsEmpty() && ui.edtFontDir->placeholderText() != config.dsFontDir.ToString())
		ui.edtFontDir->setText(config.dsFontDir.ToString());
	if (!config.dsImageDir.IsEmpty() && ui.edtImg->placeholderText() != config.dsImageDir.ToString())
		ui.edtImg->setText(config.dsImageDir.ToString());
	if (!config.dsVideoDir.IsEmpty() && ui.edtImg->placeholderText() != config.dsVideoDir.ToString())
		ui.edtVid->setText(config.dsVideoDir.ToString());
	if (!config.dsThumbDir.IsEmpty() && ui.edtThumb->placeholderText() != config.dsThumbDir.ToString())
		ui.edtThumb->setText(config.dsThumbDir.ToString());

	ui.edtWatermark->setText(config.waterMark.wm.text);

	ui.chkAddDescToAll->setChecked(config.bAddDescriptionsToAll);
	ui.chkAddTitlesToAll->setChecked(config.bAddTitlesToAll);
	ui.chkButImages->setChecked(config.bButImages);
	ui.chkCanDownload->setChecked(config.bCanDownload);

	ui.chkDoNotEnlarge->setChecked(config.doNotEnlarge);
	ui.chkFacebook->setChecked(config.bFacebookLink);
	ui.chkGenerateAll->setChecked(config.bGenerateAll);
	ui.chkForceSSL->setChecked(config.bForceSSL);
// not saved/restored:
//	ui.chkReadFromGallery->setChecked(config.bReadFromGallery);
//	ui.chkReadJAlbum->setChecked(config.bReadJAlbum);	  
	ui.chkRightClickProtected->setChecked(config.bRightClickProtected);
	ui.chkSetLatest->setChecked(config.generateLatestUploads);
	ui.chkSourceRelativePerSign->setChecked(config.bSourceRelativePerSign);
	ui.chkLowerCaseImageExtensions->setChecked(config.bLowerCaseImageExtensions);
	ui.chkMenuToAbout->setChecked(config.bMenuToAbout);
	ui.chkMenuToContact->setChecked(config.bMenuToContact);
	ui.chkMenuToDescriptions->setChecked(config.bMenuToDescriptions);
	ui.chkMenuToToggleCaptions->setChecked(config.bMenuToToggleCaptions);
	ui.chkOvrImages->setChecked(config.bOvrImages);
	ui.chkUseGoogleAnalytics->setChecked(config.googleAnalyticsOn);
	ui.btnLink->setChecked(config.imageSizesLinked);

	int h = (int)config.imageHeight ? (int)config.imageHeight : 1;
	_aspect = (double)config.imageWidth / h;

							// Watermark
	ui.sbWmOpacity->setValue(config.waterMark.wm.Opacity(true));
	ui.sbNewDays->setValue(config.newUploadInterval);
	ui.sbImageWidth->setValue(config.imageWidth);
	ui.sbImageHeight->setValue(config.imageHeight);
	ui.sbThumbnailWidth->setValue(config .thumbWidth);
	ui.sbThumbnailHeight->setValue(config.thumbHeight);

	h = config.waterMark.wm.origin & 0xF;  // vertical position
	ui.cbWmVPosition->setCurrentIndex(h);
	h = (config.waterMark.wm.origin & 0xF0) >> 4; // horizontal position
	ui.cbWmHPosition->setCurrentIndex(h);
	

	ui.lblWaterMarkFont->setText(QString("%1 (%2,%3)").arg(config.waterMark.wm.font.family()).arg(config.waterMark.wm.font.pointSize()).arg(config.waterMark.wm.font.italic()));
	if (!config.waterMark.wm.text.isEmpty())
		ui.lblWmSample->setText(config.waterMark.wm.text);

	ui.lblWmSample->setFont(config.waterMark.wm.font);

	h = config.imageQuality;
	ui.cbImageQuality->setCurrentIndex(h ? 11 - h/10 : 0);

	--_busy;

// not saved/restored:
//	ui.chkDebugging->setChecked(config.bDebugging);
	ui.chkCropThumbnails->setChecked(config.bCropThumbnails);
	ui.chkDistortThumbnails->setChecked(config.bDistrortThumbnails);
}

/*============================================================================
* TASK:		set all saved data into global controls	+ interface elements
* EXPECTS:	'config' is set up
* GLOBALS:	'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_ConfigToUI()
{
	_OtherToUi();
	_DesignToUi();
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
			_ConfigToUI();	// edit values from config

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

void FalconG::on_edtBckImageName_textChanged()
{
	if (_busy)
		return;

	QString name = QDir::fromNativeSeparators( ui.edtBckImageName->text());

	if (QFile::exists(name))
	{
		if (name != config.backgroundImage.fileName)		 // read only once
		{
			_LoadBckImage(name);
			config.backgroundImage.fileName = name;	// full path name for generator 
		}
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));	// show image
	}
	else
		_RunJavaScript("body", QString("background-image:"));	// show image

	config.SetChanged(true);				// file is copied to /res
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

void FalconG::on_edtVid_textChanged()
{
	if (_busy)
		return;

	config.dsVideoDir = ui.edtImg->text().trimmed();
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
//	QStringList qsl = s.split(QRegularExpression(R"([ ,]+(?=([^"\\]*(\\.|"([^"\\]*\\.)*[^"\\]*"))*[^"]*$))"));
	QStringList qsl = s.split(QRegularExpression(R"([,;] *)"));
//	s.replace(QRegularExpression(R"([ ,]+(?=([^"\\]*(\\.|"([^"\\]*\\.)*[^"\\]*"))*[^"]*$))"), "|");
	for (auto &qs : qsl)
	{
		qs.replace(' ', '+');
		qs.remove('"');
	}
	s = qsl.join('|');
	if (config.sGoogleFonts != s)
	{
		config.sGoogleFonts = s;
		_SetConfigChanged(true);
		_ModifyGoogleFontImport();
		_ConfigToUI();
	}
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
  * EXPECTS:mx, my new margines.  -1 : do not set this margin
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::_UpdateWatermarkMargins(int mx, int my)
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
		wm.marginY = my;	// set active and clear inactive
		_RunJavaScript(".thumb::after", wm.YMarginName(centered, false)+"");
		_RunJavaScript(".thumb::after", wm.YMarginName(centered, true)+ wm.OffsetYToStr() );
	}
	config.waterMark.wm = wm;

	_SetConfigChanged(true);
}
void FalconG::on_edtWmHorizMargin_textChanged()
{
	if (_busy)
		return;
	_UpdateWatermarkMargins(ui.edtWmHorizMargin->text().toInt(), -1);
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
	_UpdateWatermarkMargins(-1, ui.edtWmVertMargin->text().toInt());
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
	_CElem* pElem = _PtrToElement();
	pElem->font.SetSize(txt);
	if (!ui.chkDifferentFirstLine->isChecked())
	{
		ui.cbPointSizeFirstLine->setCurrentText(txt);
		pElem->font.SetDifferentFirstLine(true, txt);
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
	_FontToSample(pElem);
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
	qs = config.imageBorder.ForStyleSheetShort(false);
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
	_CElem* pElem = _PtrToElement();
	pElem->font.SetFeature(fBold, on);
	_SetConfigChanged(true);
	_FontToSample(pElem);
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
	_CElem* pElem = _PtrToElement();
	pElem->font.SetFeature(fItalic, on);
	_SetConfigChanged(true);
	_FontToSample(pElem);
}


void FalconG::_TextAlignToConfig(Align align, bool on)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	if (on)
	{
		pElem->alignment.v = align;
		_SetConfigChanged(true);
		_TextAlignToSample(pElem);	// clear decorations if neither checkbox is checked
	}
}

void FalconG::_SlotForContextMenu(const QPoint& pt)
{
	std::unique_ptr<QSignalMapper> _popupMapper{ new QSignalMapper(this) };
//	std::unique_ptr<QMenu> menu { new QMenu(this) };
	QMenu menu(this);
	QAction* pa = new QAction(tr("Styles"),this);
	pa->setEnabled(false);
	menu.addAction(pa);
	menu.addSeparator();
	for (int i = 0; i < _schemes.size(); ++i)
	{
		pa = menu.addAction(_schemes[i].MenuTitle, _popupMapper.get(), SLOT(map()));
		if (i == config.styleIndex)
		{
			pa->setCheckable(true);
			pa->setChecked(true);
		}
		_popupMapper->setMapping(pa, i);
	}
	connect(_popupMapper.get(), &QSignalMapper::mappedInt, this, &FalconG::_SlotForStyleChange);
	menu.exec(dynamic_cast<QWidget*>(sender())->mapToGlobal(pt));
}

void FalconG::_SlotForStyleChange(int which)
{
	_StyleTheProgram(which);
	_SetConfigChanged(true);
}

void FalconG::_EnableColorSchemeButtons()
{
	bool b = (config.styleIndex > 1) && _bSchemeChanged;
	ui.btnApplyColorScheme->setEnabled(b);
	ui.btnResetColorScheme->setEnabled(b);
	b = (config.styleIndex > 2); // - 2) != ui.cbColorScheme->currentIndex();
	ui.btnDeleteColorScheme->setEnabled(b);
	ui.btnMoveSchemeUp->setEnabled(ui.cbColorScheme->currentIndex() > 0);
}

void FalconG::_SlotForSchemeButtonClick(int which)
{
	StyleHandler sh( (*_pSchemeButtons[which]).styleSheet());
	QColor qc = sh.GetItem("", "background-color");
	QColor qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew == qc || !qcNew.isValid())
		return;
	sh.SetItem("", "background-color", qcNew.name());
	_pSchemeButtons[which]->setStyleSheet(sh.StyleSheet());
	if (!which)
		ui.pnlColorScheme->setStyleSheet("background-color:" + qcNew.name()+"; color:"+_tmpScheme.sTextColor);
	else if(which == 1)
		ui.pnlColorScheme->setStyleSheet("color:" + qcNew.name()+"; background-color:"+_tmpScheme.sBackground);
	_bSchemeChanged |= true;	// might have been changed already
	_tmpScheme[which] = qcNew.name();
	_EnableColorSchemeButtons();
}

/*========================================================
 * TASK:	set text decorations
 * PARAMS:	   decoration - enum (td...)
 *			   on - turn on or off
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_TextDecorationToConfig(Decoration decoration, bool on)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	pElem->decoration.SetDecoration(decoration, on);
	on |= ui.chkTdUnderline->isChecked() || ui.chkTdOverline->isChecked() || ui.chkTdLinethrough->isChecked();
	ui.gbDecorationStyle->setEnabled(on);
	if (on)
	{
		ui.rbTdSolid->setEnabled(on);
		ui.rbTdDotted->setEnabled(on);
		ui.rbTdDashed->setEnabled(on);
		ui.rbTdDouble->setEnabled(on);
		ui.rbTdWavy->setEnabled(on);
	}

	_DecorationToSample(pElem);	// clear decorations if neither checkbox is checked
	_SetConfigChanged(true);
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

void FalconG::on_rbTextShadow_toggled(bool b)
{
	_CElem* pElem = _PtrToElement();
	int n = b ? 0 : 1;	// text or box shadow
	_ShadowForElementToUI(pElem, n);
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
	pElem->color.SetOpacity(on ? ui.sbTextOpacity->value()*2.55 : -1, false);
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
	pElem->background.SetOpacity(on ? ui.sbBackgroundOpacity->value()*2.55 : -1, false);
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

	_SetLinearGradient(pElem);
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
	_RunJavaScript("menu-item#contact",QString("display:")+ (on ? "inline-block" : "none"));
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
	_RunJavaScript("menu-item#about",QString("display") + (on? "inline-block" : "none"));
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
	_RunJavaScript("menu-item#desc",QString("display") + (on? "inline-block" : "none") );
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
	_RunJavaScript("menu-item#captions", QString("display") + (on ? "inline-block" : "none"));
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
	_SetGradientLabelColors(pElem);
	_SetConfigChanged(true);
	_SetLinearGradient(pElem);
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
	_SetGradientLabelColors(pElem);
	_SetConfigChanged(true);
	_SetLinearGradient(pElem);
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
	_SetGradientLabelColors(pElem);
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
	QString qs = config.imageBorder.ForStyleSheetShort(false);
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
	config.waterMark.wm.SetOpacity(val, true);
	_SetWatermark();
	_SetConfigChanged(true);
	_RunJavaScript(".thumb::after","color"+ config.waterMark.wm.ColorToCss());
}

void FalconG::on_sbSpaceAfter_valueChanged(int val)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	pElem->spaceAfter = val;
	_SetConfigChanged(true);
	_SpaceAfterToSample(pElem);
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

void FalconG::on_hsImageSizeToShow_valueChanged(int val)
{
	if (_busy)
		return;
	config.backgroundImage.size = val;
	_RunJavaScript("body", config.backgroundImage.Size(false));
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
	_CElem* pElem = _PtrToElement();
	BorderSide side = (BorderSide )(ui.cbBorder->currentIndex()-1);
	QColor qc(pElem->border.ColorStr(side)),
			qcNew;
	qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew == qc || !qcNew.isValid())
		return;

	_SetConfigChanged(true);
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
 * TASK:	sets the uplink icon from resource
 *			colors it as set inconfig	for the uplink
 *			menu web button and returns the rec-colored icon
 *			to be saved
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
		iconName = ":/icons/Resources/up-icon.png";
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


/*========================================================
 * TASK:	When the css or html changes 
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_btnReload_clicked()
{
	_page.triggerAction(QWebEnginePage::Reload);
	_ConfigToUI();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnSaveConfig_clicked()
{
	QList<int> splitterSizes = ui.designSplitter->sizes();
	config.splitterLeft = splitterSizes.at(0);
	config.splitterRight = splitterSizes.at(1);
	config.Write();

	//CssCreator creator;
	//creator.Create("falconG.css", true);

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

	if(qcNew.isValid() )
	{
		if (qc != qcNew)
		{
			config.Web.color.SetColor(qcNew.name());
			ui.btnPageColor->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.color.Name()));
			_SetConfigChanged(true);
		}
		//AlbumElement ae = _aeActiveElement;
		//_aeActiveElement = aeWebPage;
		_PageColorToSample();
		//_aeActiveElement = ae;
	}
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

	if (qcNew.isValid())
	{
		if (qc != qcNew)
		{

			config.Web.background.SetColor(qcNew.name());
			ui.btnPageBackground->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.background.Name()));
			_SetConfigChanged(true);
		}
		//AlbumElement ae = _aeActiveElement;
		//_aeActiveElement = aeWebPage;
		_PagebackgroundToSample();
		//_aeActiveElement = ae;
	}
}

void FalconG::on_btnOpenBckImage_clicked()
{
	QString qs = QFileDialog::getOpenFileName(this, tr("falconG - Open Background Image"), QString(), tr("Image files (*.bmp *.gif *.jpg *.png)"));
	if (!qs.isEmpty())
		ui.edtBckImageName->setText(qs);
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
	_SetGradientLabelColors(pElem);

	_SetConfigChanged(true);
	_SetLinearGradient(pElem);
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
	_SetGradientLabelColors(pElem);

	_SetConfigChanged(true);
	_SetLinearGradient(pElem);
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
	_SetGradientLabelColors(pElem);

	_SetConfigChanged(true);
	_SetLinearGradient(pElem);
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
		BorderSide side = sdAll; //  (BorderSide)(ui.cbBorder->currentIndex() - 1);
		config.imageBorder.SetColor(side, qcNew.name());
		on_sbImageBorderWidth_valueChanged(ui.sbImageBorderWidth->value());
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
	_DesignToUi();

	_aeActiveElement = aeWebPage;
	ui.cbActualItem->setCurrentIndex(0);		// global page
	_ConfigToSample();
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


/*========================================================
 * TASK:	modifies the @import in the css file to include
 *			the actual Google fonts and default fonts
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - keeps a backup of the original css
 *			- if no Google fonts and no default fonts 
 *				is given removes teh @import line
 *-------------------------------------------------------*/
void FalconG::_ModifyGoogleFontImport()
{
	CssCreator cssCreator;
	cssCreator.Create(CONFIGS_USED::_samplePath+"css/falconG.css", true);	// program library setting cursors too
#if 0
	static QString name = CONFIGS_USED::_samplePath+"css/falconG.css",
		tmpName = name + ".tmp";
	QFile in(name),
		out(tmpName);
	in.open(QIODevice::ReadOnly),
		out.open(QIODevice::WriteOnly);

	QTextStream infs(&in), outfs(&out);
	QString line = infs.readLine(),	// first line may be the font import line
			savedLine;

	if (line.left(7) != "@import")	// did not start with font import: save line for after inserting @import later
		savedLine = line;
    
	line = config.sGoogleFonts.ToString();
	if (!line.isEmpty())
	{
		line.replace('"', "'");
		line = QString("@import url('https://fonts.googleapis.com/css?family=" + line + "&display=swap');");
		outfs << line << "\n";
	}
	line = config.sDefFonts.ToString();
	if (!line.isEmpty())
	{
	// TODO put fonts directory handling here
	}
	if (!savedLine.isEmpty())
		outfs << savedLine << "\n";
	while (!infs.atEnd())
	{
		line = infs.readLine();
		outfs << line << "\n";
	}
	in.close();
	out.close();
	BackupAndRename(name, tmpName);
#endif
	// reload page
	_page.triggerAction(QWebEnginePage::Reload);
	ui.cbActualItem->setCurrentIndex(0);
}


/*========================================================
 * TASK:	set up cbFonts from fonts in 
 *			config.sGoogleFonts and config.sDefFonts
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_SettingUpFontsCombo()
{
	QStringList qsl = config.sDefFonts.ToString().split('|') + config.sGoogleFonts.ToString().split('|');
	qsl.sort(Qt::CaseInsensitive);
	ui.cbFonts->clear();
	for (auto s : qsl)
	{
		s.replace('+', ' ');
		ui.cbFonts->addItem(s);
	}

	ui.cbFonts->setEnabled(qsl.size());
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
 * TASK:	Load image 'name' into label on UI
 * PARAMS:	name: path name of file to load
 * GLOBALS:	ui.lblBckImage, config
 * RETURNS: none
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_LoadBckImage(QString name)
{
	if (name.isEmpty())
		return;
	QSize size(308, 227);// = ui.lblbckImage->size(); label size is not the real one when page is loaded
	QImage image(name);
	QPixmap pm;
	pm = QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio));
	ui.lblbckImage->setPixmap(pm);
}

void FalconG::_ResetScheme()
{
}


/*========================================================
 * TASK:	Add all sacheme color selection buttons to 
 *			editor
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - to add a form layout to the widget in the
 *				Designer, there must be something already 
 *				on the widget, so I added two QLabels with
 *				no text and set the layout to form layout
 *			- Had I given the widget an explicit layout,
 *				to make that layout cover the whole widget
 *				I should have given a layout to the widget.
 *				(in that case it should have been used here
 *				 instead of ui.pnlColorScheme->layout() )
 *-------------------------------------------------------*/
void FalconG::_AddSchemeButtons()
{
	_pSchemeMapper = new QSignalMapper(this);

	// add defined schemes to combo box
	for (int i = 2; i < _schemes.size(); ++i)				// do not add default and system!
		ui.cbColorScheme->addItem(_schemes[i].MenuTitle);
	// add buttons to layout

	QFont font;
//	font.setFamily(QString::fromUtf8("Arial"));
	font.setPointSize(8);
	font.setBold(false);
	font.setWeight(50);

	QLabel *plabel = new QLabel(tr("Background color"));
	plabel->setFont(font);

	QFormLayout* pLayout = reinterpret_cast<QFormLayout*>(ui.pnlColorScheme->layout() );
	int i = 0;
	QPushButton* pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;

	plabel = new QLabel(tr("Foreground (Text) color") );
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Border color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Focused Input color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Hover color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Tab Border color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Input Background"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Selected Background"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Focused Border color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Disabled Foreground"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Disabled Background"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Image Background"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Button Pressed Bg"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Default Background"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Progressbar Chunk color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Warning color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(pButton , i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;
	plabel = new QLabel(tr("Groupbox Title Color"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton );
	_pSchemeMapper->setMapping(_pSchemeButtons[i], i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));
	++i;

	connect(_pSchemeMapper, &QSignalMapper::mappedInt, this, &FalconG::_SlotForSchemeButtonClick);
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
			// only check load once
	if (_isWebPageLoaded)
		return;
	_isWebPageLoaded = true;

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

void FalconG::on_cbFonts_currentIndexChanged(int index)
{
	if (_busy || index < 0)
		return;
	QString s = ui.cbFonts->currentText();
	ui.edtFontFamily->setText(s);
	//ui.cbFonts->setCurrentIndex(-1);
}

void FalconG::on_cbLineHeight_currentIndexChanged(int index)
{
	if (_busy || index < 0)
		return;
	_CElem* pElem = _PtrToElement(_aeActiveElement);
	pElem->font.SetLineHeight(ui.cbLineHeight->currentText());
	_FontToSample(pElem);	

}


/*========================================================
 * TASK:	when scheme is changed but not yet applied asks
 *				for apply
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_AskForApply()
{
	if (_busy )//|| !_bSchemeChanged)
		return;
	++_busy;
	if (QMessageBox::warning(this, tr("falconG - Color scheme changed"), tr("Changes were not applied.\nDo you want to apply changes>"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
		on_btnApplyColorScheme_clicked();
	--_busy;
}


/*========================================================
 * TASK:	when the index changes sets up _tmpScheme
 *			and color buttons
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_cbColorScheme_currentIndexChanged(int newIndex)
{
	if (_busy || newIndex < 0)
		return;
	if (newIndex +2 == _schemes.size())	// just the text changed and an ENTER is pressed?
		on_btnApplyColorScheme_clicked();
	else
	{
		if (_bNewSchemeName || _bSchemeChanged)
		_AskForApply();
		static const char *bckstr= "background-color:%1",
						  *bck_txt = "background-color:%1; color:%2";
		_tmpScheme = _schemes[newIndex+2];	// default and system are not changed
		_tmpSchemeOrigName = _tmpScheme.MenuTitle;
		ui.pnlColorScheme->setStyleSheet(QString(bck_txt).arg(_tmpScheme.sBackground).arg(_tmpScheme.sTextColor));
		for (int i = 0; i < _pSchemeButtons.size(); ++i)
			_pSchemeButtons[i]->setStyleSheet(QString(bckstr).arg(_tmpScheme[i]));
		_EnableColorSchemeButtons();
	}
}


/*========================================================
 * TASK:	enable buttons to signal there are new data to be added
 * PARAMS:	newText: changed text
 * GLOBALS:
 * RETURNS:
 * REMARKS: - if the title of the source text is not the same
 *				as the original (already existing) title: then  
 *				there were changes which can be applied or
 *				taken back
 *-------------------------------------------------------*/
void FalconG::on_cbColorScheme_currentTextChanged(const QString& newText)
{
	ui.btnApplyColorScheme->setEnabled(_schemes.IndexOf(newText) );
}

void FalconG::on_cbImageQuality_currentIndexChanged(int newIndex)
{
	if (_busy)
		return;
	config.imageQuality = newIndex ? (11 - newIndex)*10 : 0;
}


/*========================================================
 * TASK: when the user finishes editing the actual text of
 *			the scheme combo box creates a new scheme
 *	
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - called AFTER the currentTextChanged above
 *-------------------------------------------------------*/
void FalconG::_SlotForSchemeComboEditingFinished()
{
	if ( _schemes.IndexOf(ui.cbColorScheme->lineEdit()->text()) < 0 )
	{
		_bNewSchemeName = true;
		_tmpSchemeOrigName = ui.cbColorScheme->currentText();
//		_AskForApply();
		_EnableColorSchemeButtons();
	}
	else
		_bNewSchemeName = false;
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

//****************************************************************************

void FalconG::on_rbNoBackgroundImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hNotUsed;
		config.SetChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}

void FalconG::on_rbCenterBckImage_toggled(bool b)
{
	if (!_busy && b)
	{		
		config.backgroundImage.v = hAuto;
		config.SetChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}
void FalconG::on_rbCoverBckImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hCover;
		config.SetChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}
void FalconG::on_rbTileBckImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hTile;
		config.SetChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
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
							QMessageBox::Ok, this).exec();
	}
}

void FalconG::on_btnGoToGoogleFontsPage_clicked()
{
	QDesktopServices::openUrl(QUrl("https://fonts.google.com/"));
}


/*========================================================
 * TASK:	apply changes to color scheme
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_btnApplyColorScheme_clicked()
{
	_bSchemeChanged = _bNewSchemeName = false;
	int i = _schemes.IndexOf(_tmpSchemeOrigName);
	if (i >= 0)
		_schemes[i] = _tmpScheme;
	else
	{
		_tmpScheme.MenuTitle = _tmpSchemeOrigName;
		_schemes.push_back(_tmpScheme);
		ui.cbColorScheme->addItem(_tmpSchemeOrigName);
	}

	_schemes.Save();

	_EnableColorSchemeButtons();
	_StyleTheProgram(config.styleIndex);
}


/*========================================================
 * TASK:	undo change for actual color scheme
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - no reset after apply!
 *-------------------------------------------------------*/
void FalconG::on_btnResetColorScheme_clicked()
{
	_tmpScheme = _schemes[ui.cbColorScheme->currentIndex() + 2];
	_tmpSchemeOrigName = _tmpScheme.MenuTitle;
	ui.cbColorScheme->setCurrentText(_tmpSchemeOrigName);
	_bSchemeChanged = false;
	_EnableColorSchemeButtons();
	_StyleTheProgram(config.styleIndex);
}


/*========================================================
 * TASK:	moves up the actual scheme
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - no need to change in display changes, 
 *				but need to s change the order of schemes
 *-------------------------------------------------------*/
void FalconG::on_btnMoveSchemeUp_clicked()
{
	int i = ui.cbColorScheme->currentIndex(), 
		j = i+2;							  // index in schemes (default and system can't be changed)

	if (j == config.styleIndex.v )
	{
		config.styleIndex = j - 1;
		config.SaveSchemeIndex();
	}
	else if(j == config.styleIndex.v+1 )
	{
		config.styleIndex = j + 1;
		config.SaveSchemeIndex();
	}

	FalconGScheme sty = _schemes[j];
	_schemes.replace(j, _schemes[j - 1]);
	_schemes.replace(--j, sty);

	++_busy;
	ui.cbColorScheme->removeItem(i--);
	ui.cbColorScheme->insertItem(i, _schemes[j].MenuTitle);
	ui.cbColorScheme->setCurrentIndex(i);
	if (!i)
		ui.btnMoveSchemeUp->setEnabled(false);
	--_busy;
	_schemes.Save();
}


/*========================================================
 * TASK:	delete scheme
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - the new actual scheme will be the actual (i.e. saved)
 *			- config.styleIndex may change if it was the one or 
 *				the actual one
 *			- when the actual selected scheme is deleted
 *				it still remains active, so you may
 *				restore it by writing the name into the combo box
 *-------------------------------------------------------*/
void FalconG::on_btnDeleteColorScheme_clicked()
{
	if (QMessageBox::warning(this,	tr("falconG - Warning"), 
									tr("Do you really want to delete this color scheme?"), 
										QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
		return;

	int i = ui.cbColorScheme->currentIndex();
	if (i >= config.styleIndex.v - 2)
	{
		config.styleIndex = config.styleIndex - 1;
		config.SaveSchemeIndex();
	}
	ui.cbColorScheme->removeItem(i);
	_schemes.remove(i + 2);
	ui.cbColorScheme->setCurrentIndex(i == _schemes.size() - 2 ? --i : i);
	_schemes.Save();
	_EnableColorSchemeButtons();
	_bSchemeChanged = false;
}

void FalconG::on_btnGenerateScheme_clicked()
{
	QColor qcBase = QColorDialog::getColor(qcBase, this, tr("Select background color."));
	if (qcBase.isValid())
	{
		QColor qc;
		_tmpScheme.sBackground = qcBase.name();
/*
		_tmpScheme.sTextColor = ;
		_tmpScheme.sBorderColor = ;
		_tmpScheme.sFocusedInput = ;
		_tmpScheme.sHoverColor = ;
		_tmpScheme.sTabBorder = ;
		_tmpScheme.sInputBackground = ;
		_tmpScheme.sSelectedInputBgr = ;
		_tmpScheme.sFocusedBorder = ;
		_tmpScheme.sDisabledFg = ;
		_tmpScheme.sDisabledBg = ;
		_tmpScheme.sImageBackground = ;
		_tmpScheme.sPressedBg = ;
		_tmpScheme.sDefaultBg = ;
		_tmpScheme.sProgressBarChunk = ;
		_tmpScheme.sWarningColor = ;
		_tmpScheme.sBoldTitleColor = ;
*/
	}
}

//***************************************************************************
void FalconG::on_cbActualItem_currentIndexChanged(int newIndex)
{
	if (_busy)
		return;

	++_busy;
	_aeActiveElement = (AlbumElement)newIndex;
	ui.gbGradient->setEnabled(newIndex);
	ui.gbBorder->setEnabled(newIndex);
	int page = newIndex == aeThumb ? 2 : newIndex ? 1 : 0;
	ui.toolBox->setCurrentIndex(page);	// to settings
	_ActualSampleParamsToUi();
	--_busy;
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

void FalconG::_SetGradientLabelColors(_CElem* pElem, bool invert)
{
	ui.lblGradient->setStyleSheet(QString("background:%1;").arg(pElem->gradient.ForQtStyleSheet(invert)));
	ui.btnGradMiddleColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[1].color));
	ui.btnGradStartColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[0].color));
	ui.btnGradStopColor->setStyleSheet(__ToolButtonBckStyleSheet(config.Menu.gradient.gs[2].color));
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
		val = (ui.chkBackgroundOpacity->isChecked() ? 1 : -1)*ui.sbBackgroundOpacity->value(); // in %
		elem.background.SetOpacity(val, true);
	}
	if((which & 1) == 0 )
	{
		val = (ui.chkTextOpacity->isChecked() ? 1 : -1)*ui.sbTextOpacity->value(); // in %
		elem.color.SetOpacity(val, true);
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

void FalconG::_ColorToSample(_CElem* pElem)
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
void FalconG::_BackgroundToSample(_CElem* pElem)
{
	bool b = (pElem->parent && pElem->parent->background != pElem->background) || !pElem->parent;
	_SetCssProperty(pElem,b ? pElem->background.ForStyleSheet(false, true) : QString("background-color:") );
}

void FalconG::_ShadowToSample(_CElem* pElem,int what)
{
	QString sPropty, sValue;
	if (ui.chkShadowOn->isChecked())
		sValue = pElem->shadow1[what].ForStyleSheet(false, false, what) + pElem->shadow2[what].ForStyleSheet(false, true, what);
	_SetCssProperty(pElem, sPropty + sValue);
}

void FalconG::_BorderToSample(_CElem* pElem)
{
	QString qs;
	if(pElem == &config.Web || !pElem->parent || pElem->parent->border != pElem->border)
		 qs = pElem->border.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
}

void FalconG::_SetLinearGradient(_CElem* pElem)
{
	QString qs;
	if( pElem != &config.Web && (!pElem->parent || pElem->parent == &config.Web) )
		 qs = pElem->gradient.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
}

void FalconG::_FontToSample(_CElem* pElem)
{
	QString qs;
	if (pElem == &config.Web || !pElem->parent || pElem->parent->font != pElem->font)
	{
		_SetCssProperty(pElem,  "font-family:" + pElem->font.Family() + "\n" +
								"font-size:"   + pElem->font.SizeStr() + "\n" +
								"font-weight:" + pElem->font.WeightStr() + "\n" +
								"font-style:" + pElem->font.ItalicStr() + "\n" + 
								"line-height:" + pElem->font.LineHeightStr());
//		 qs = pElem->font.IsFirstLineDifferent() ? pElem->font.FirstLineFontSizeStr() : pElem->font.SizeStr();
//		_SetCssProperty(pElem, "font-size:" + qs, "::first-line");	// parent = Web and ::first-line
	}
	else
	{
		_SetCssProperty(pElem,	"font-family:\n"
								"font-size:\n"
								"font-weight:\n"
								"font-style:\n"
								"line-height:");
//		qs = pElem->font.IsFirstLineDifferent() ? pElem->font.FirstLineFontSizeStr() : pElem->font.SizeStr();
//		_SetCssProperty(pElem, "font-size", qs, "::first-line");	// parent = Web and ::first-line
	}
}

void FalconG::_SpaceAfterToSample(_CElem* pElem)
{
	QString qs = "margin-bottom:" + (pElem->spaceAfter ? QString("%1px").arg(pElem->spaceAfter) : QString());
	_SetCssProperty(pElem, qs);
}

void FalconG::_DecorationToSample(_CElem* pElem)
{
	QString qs;
	if (!pElem->decoration.IsTextDecorationLine())
		_SetCssProperty(pElem, "text-decoration-line:");
	else if (pElem == &config.Web || !pElem->parent || pElem->parent == &config.Web || pElem->parent->decoration != pElem->decoration)
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

void FalconG::_TextAlignToSample(_CElem* pElem)
{
	QString qs;
	if (pElem == &config.Web || !pElem->parent || pElem->parent->alignment != pElem->alignment)
		qs = pElem->alignment.ActAlignStr();
	_SetCssProperty(pElem,"text-align:"+ qs);
}

void FalconG::_PageColorToSample()
{
	_SetCssProperty(&config.Web,"color:" + config.Web.color.Name());
}

void FalconG::_PagebackgroundToSample()
{
	QString wbc = config.Web.background.Name();

	auto clearBackground = [&](AlbumElement what)
	{
		_CElem* pe = _PtrToElement(what);
		pe->background.Set(wbc, -100);
		_SetCssProperty(pe, "background-color:");
	};

	if (ui.chkSetAll->isChecked())		// then remove all other background colors
	{
		clearBackground(aeHeader);
		clearBackground(aeMenuButtons);
		clearBackground(aeLangButton);
		clearBackground(aeSmallTitle);
		clearBackground(aeGalleryTitle);
		clearBackground(aeGalleryDesc);
		clearBackground(aeSection);
		clearBackground(aeImageTitle);
		clearBackground(aeImageDesc);
		clearBackground(aeLightboxTitle);
		clearBackground(aeLightboxDescription);
		clearBackground(aeFooter);
	}
	QString qs = config.Web.background.ForStyleSheet(false, true);
	_RunJavaScript("body", qs);
}

void FalconG::_SetIcon()		// only for 'menu-item#uplink' and the icon always has the same name
{								// but its color will change. Will it be visible without setting it again? And this way? TODO Check???
	_RunJavaScript("menu-item#uplink",QString("background-image:")+"url('file:///res/up-icon-sample.png')");
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
	WaterMark &wm = config.waterMark.wm;
	_RunJavaScript(".thumb::after","content:" + wm.text);
			// set offsets
	bool  centered;
	QString 
	qs = wm.XMarginName(centered, false);		// X offset  -clear unused property
	_RunJavaScript(".thumb::after", qs +  wm.OffsetXToStr());
	qs = wm.XMarginName(centered, true);				// set used property												   
	_RunJavaScript(".thumb::after",qs + wm.OffsetXToStr());

	qs = wm.YMarginName(centered, false);				// Y offset clear unused property
	_RunJavaScript(".thumb::after",qs + wm.OffsetYToStr());
	qs = wm.YMarginName(centered, true);				// set used property												   
	_RunJavaScript(".thumb::after",qs + wm.OffsetYToStr());
			// set color
	_RunJavaScript(".thumb::after","color:" + wm.ColorToCss());
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
		pElem->color.SetOpacity(val,true);
	else
		pElem->background.SetOpacity(val, true);
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

void FalconG::_SetLayoutMargins(int which)
{
	int layoutTop = 9;
	QMargins margins;

	auto modifyLayout = [&](QWidget* pw)
	{
		margins = pw->layout()->contentsMargins();
		margins.setTop(layoutTop);
		pw->layout()->setContentsMargins(margins);
	};

	if(which)
		layoutTop = 18;
	else
		layoutTop = 9;

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
	modifyLayout(ui.gbPosition);
	modifyLayout(ui.gbWmTextShadow);

	modifyLayout(ui.gbSample);

}

void FalconG::_StyleTheProgram(int which)
{
	if (which >= _schemes.size())
		which = 0;

	_SetLayoutMargins(which);

		// change these for new color set
							//			   def. system    blue		 dark		  black		 
	 // theme style string used only when not the default style is used
	static QString styles = {
	R"END(
* {
	background-color:%1;       /* %1 background */
	color:%2;                  /* %2 color */
}
        
/* ------------------- geometry ------------------*/        

QTabWidget::tab-bar {
    left: 5px;
}
QTabBar::tab {
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
	min-width: 20ex;
	padding: 2px;
}        
QTabBar::tab:!selected {
    margin-top: 2px; 
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
	background-color:%15;
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
	position: absolute;
	top:-4px;
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
    border: 2px solid %3;
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QGroupBox::title {
	color:%17
}

QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%4;
}

#menuWidget {
	background-color:rgb(0,0,0,0);
}
QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;
}
QTabBar::tab:selected {
	border-color:%6;
    border-bottom-color:%1;
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%7;
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%8;
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled,
QRadioVutton:disabled,
QCheckBox:disabled,
QSpinBox:disabled,
QMenu:disabled {
	color:%10;
	background:%11;
}

QLineEdit.disabled {
	color:%10;
	background:%11;
}

#lblBckImage,
#groupBox_6 {
	background-color:%12;
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%13;
}

QPusButton:default {
    background-color:%14;
}

#lblActualElem {
	color:%16;
}
#btnDeleteColorScheme {
	background-color:%9;
}
)END"
	};
	config.styleIndex = which;
	if (which)
	{
		QString ss =
			QString(styles)
			.arg(_schemes[which].sBackground)		// %1 
			.arg(_schemes[which].sTextColor)		// %2 
			.arg(_schemes[which].sBorderColor)		// %3 
			.arg(_schemes[which].sFocusedInput)		// %4 
			.arg(_schemes[which].sHoverColor)		// %5 
			.arg(_schemes[which].sTabBorder)		// %6 
			.arg(_schemes[which].sInputBackground)	// %7
			.arg(_schemes[which].sSelectedInputBgr)	// %8 
			.arg(_schemes[which].sFocusedBorder)	// %9 
			.arg(_schemes[which].sDisabledFg)		// %10
			.arg(_schemes[which].sDisabledBg)		// %11
			.arg(_schemes[which].sImageBackground)	// %12
			.arg(_schemes[which].sPressedBg)		// %13
			.arg(_schemes[which].sDefaultBg)		// %14
			.arg(_schemes[which].sProgressBarChunk)	// %15
			.arg(_schemes[which].sWarningColor)		// %16
			.arg(_schemes[which].sBoldTitleColor)	// %17
			;

		if (which == stBlue)		// blue
				ss += QString( R"(QCheckBox::indicator:checked {
		image: url(":/icons/Resources/blue-checked.png");
	}

QCheckBox::indicator:unchecked {
	image: url(":/icons/Resources/blue-unchecked.png");
}
)");
// DEBUG
//			DEBUG_LOG_NEW(ss)

		// setStyleSheet(ss);	 // sometimes this does not cascade down to dialogs
		// use this instead:
		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);

		//QFile f("style.txt");
		//f.open(QIODevice::WriteOnly);
		//QTextStream textstream(&f);
		//textstream << ss;

	}
	else
		setStyleSheet("");

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

	QColor  qc = config.waterMark.wm.Color() ,
			qcNew;
	qcNew = QColorDialog::getColor(qc, this, tr("Select Watermark Color"));

	if (qcNew.isValid() && qcNew != qc)
	{
		QString s = qcNew.name();
		handler.SetItem("QToolButton", "background-color", s);
		ui.btnWmColor->setStyleSheet(handler.StyleSheet());
		s = s.mid(1);	 // skip '#'
		config.waterMark.wm.SetColor(s);
		_SetConfigChanged(config.waterMark.v = true);
		_RunJavaScript(".thumb::after", "color: " + config.waterMark.wm.ColorToCss());
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
	QString src = CONFIGS_USED::_samplePath+"res/"+destName;
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

void FalconG::_ShadowForElementToUI(_CElem* pElem, int which)
{
	++_busy;
	ui.sbShadowSpread1->setEnabled(which);
	ui.sbShadowSpread2->setEnabled(which);

	if (which)
	{
		ui.sbShadowSpread1->setValue(pElem->shadow1[which].Spread());
		ui.sbShadowSpread2->setValue(pElem->shadow2[which].Spread());
	}

	ui.chkShadowOn->setChecked(pElem->shadow1[which].Used());

	ui.sbShadowHoriz1->setValue(pElem->shadow1[which].Horiz());
	ui.sbShadowVert1->setValue(pElem->shadow1[which].Vert());
	ui.sbShadowBlur1->setValue(pElem->shadow1[which].Blur());
	ui.sbShadowHoriz2->setValue(pElem->shadow2[which].Horiz());
	ui.sbShadowVert2->setValue(pElem->shadow2[which].Vert());
	ui.sbShadowBlur2->setValue(pElem->shadow2[which].Blur());

	QString qc = pElem->shadow1[which].Color();
	if (qc.isEmpty())	// not set
		qc = "#ddd";	
	ui.btnShadowColor->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(qc));
	--_busy;

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
	static QStringList __qslRunThese;
	__qslRunThese = __qslRunThese + value.split('\n');

	if (!_isWebPageLoaded)		// only try to run after page looaded
		return;

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
	for (auto s : __qslRunThese)
		runOneTag(s);

	__qslRunThese.clear();
}

void FalconG::_SetCssProperty(_CElem*pElem, QString value, QString subSelector)
{
	if (value.isEmpty())
		return;

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

// ================================ FSchemeVector ============================
void FSchemeVector::ReadAndSetupStyles()
{
	bool bl = false, da = false, bk = false;	// blue, dark, black set
	if (QFile::exists("falconG.fsty")) 
	{
		QSettings s("falconG.fsty", QSettings::IniFormat);	// in program directory;
		QStringList keys = s.childGroups();	// get all top level keys with subkeys
		FalconGScheme fgst;
		for (auto k : keys)
		{
			s.beginGroup(k);
			fgst.MenuTitle			= s.value("Title"			, "").toString();			// this will appear in the menu bar
			fgst.sBackground		= s.value("Background"		, "").toString();
			fgst.sTextColor			= s.value("TextColor"		, "").toString();
			fgst.sBorderColor		= s.value("BorderColor"		, "").toString();
			fgst.sFocusedInput		= s.value("FocusedInput"	, "").toString();
			fgst.sHoverColor		= s.value("HoverColor"		, "").toString();
			fgst.sTabBorder			= s.value("TabBorder"		, "").toString();
			fgst.sInputBackground	= s.value("InputBackground"	, "").toString();
			fgst.sSelectedInputBgr	= s.value("SelectedInputBgr", "").toString();
			fgst.sFocusedBorder		= s.value("FocusedBorder"	, "").toString();
			fgst.sDisabledFg		= s.value("DisabledFg"		, "").toString();
			fgst.sDisabledBg		= s.value("DisabledBg"		, "").toString();
			fgst.sImageBackground	= s.value("ImageBackground"	, "").toString();
			fgst.sPressedBg			= s.value("PressedBg"		, "").toString();
			fgst.sDefaultBg			= s.value("DefaultBg"		, "").toString();
			fgst.sProgressBarChunk	= s.value("ProgressBarChunk", "").toString();
			fgst.sWarningColor		= s.value("WarningColor"	, "").toString();
			fgst.sBoldTitleColor	= s.value("BoldTitleColor"	, "").toString();
			s.endGroup();
			push_back(fgst);
			if (fgst.MenuTitle == "Blue")
				bl = true;
			if (fgst.MenuTitle == "Dark")
				da = true;
			if (fgst.MenuTitle == "Black")
				bk = true;
		}
	}
	// set defaults if not set already
	if(!bl)
		push_back(blue);
	if (!da)
		push_back(dark);
	if (!bk)
		push_back(black);
}

void FSchemeVector::Save()
{
	if (QFile::exists("falconG.fsty"))
		QFile::remove("falconG.fsty");

	QSettings s("falconG.fsty", QSettings::IniFormat);	// in program directory;
	for (int i = 2; i <  size(); ++i)	// do not save default and system
	{
		char st[] = "0"; st[0] += i-1;
		FalconGScheme& fgst = operator[](i);
		s.beginGroup(st);
			s.setValue("Title",fgst.MenuTitle);			// this will appear in the menu bar
			s.setValue("Background",fgst.sBackground);
			s.setValue("TextColor",fgst.sTextColor);
			s.setValue("BorderColor",fgst.sBorderColor);
			s.setValue("FocusedInput",fgst.sFocusedInput);
			s.setValue("HoverColor",fgst.sHoverColor);
			s.setValue("TabBorder",fgst.sTabBorder);
			s.setValue("InputBackground",fgst.sInputBackground);
			s.setValue("SelectedInputBgr",fgst.sSelectedInputBgr);
			s.setValue("FocusedBorder",fgst.sFocusedBorder);
			s.setValue("DisabledFg",fgst.sDisabledFg);
			s.setValue("DisabledBg",fgst.sDisabledBg);
			s.setValue("ImageBackground",fgst.sImageBackground);
			s.setValue("PressedBg",fgst.sPressedBg);
			s.setValue("DefaultBg",fgst.sDefaultBg);
			s.setValue("ProgressBarChunk",fgst.sProgressBarChunk);
			s.setValue("WarningColor",fgst.sWarningColor);
			s.setValue("BoldTitleColor",fgst.sBoldTitleColor);
		s.endGroup();
	}
}

int FSchemeVector::IndexOf(FalconGScheme& fgst)
{
	return IndexOf(fgst.MenuTitle);
}

int FSchemeVector::IndexOf(const QString &title)
{
	for (int i = 2; i < size(); ++i)
		if (operator[](i).MenuTitle == title)
			return i;
	return -1;
}