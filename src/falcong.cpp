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

// ************************ helper **********************
/*------------------------------------- macros ------------------------------------*/

// DEBUG
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
	if (color.IsOpacityUsed())	// color name format: #AABBCC
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
	destDir = PROGRAM_CONFIG::samplePath + destDir;

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
	QString serr = QMainWindow::tr("Can't copy resource'%1' to folder '%2'\nPlease make sure the folder is writeable!\n\nExiting");
	QMessageBox::critical(nullptr, "falconG", serr.arg(name).arg(destDir));
	exit(1);
}
// ============================================================================================================
/*============================================================================
* TASK:		construct a FalconG object, read its parameters, set up controls
*			and sample web page
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FalconG::FalconG(QWidget *parent) : QMainWindow(parent)
{
	PROGRAM_CONFIG::GetHomePath();

	config.dsApplication = QDir::current().absolutePath(); // before anybody changes the current directory

	// create directories for sample
	int ask = config.doNotShowTheseDialogs;
	config.doNotShowTheseDialogs.v |= int(dbAskCreateDir);
	config.defaultAnswers[dboAskCreateDir] = QMessageBox::Yes;

	CreateDir(PROGRAM_CONFIG::samplePath);
	CreateDir(PROGRAM_CONFIG::samplePath+"css");
	CreateDir(PROGRAM_CONFIG::samplePath+"js");
	CreateDir(PROGRAM_CONFIG::samplePath+"res");
	config.doNotShowTheseDialogs = ask;

	QString resPPath = QStringLiteral(":/Preview/Resources/"),
			resIPath = QStringLiteral(":/icons/Resources/");
	_CopyResourceFileToSampleDir(resPPath, "index_hu.html");
	_CopyResourceFileToSampleDir(resPPath, "index_en.html");
	_CopyResourceFileToSampleDir(resPPath, "falconGGen.js");
	_CopyResourceFileToSampleDir(resPPath, "falconG.js");
	_CopyResourceFileToSampleDir(resPPath, "latest.js");
	_CopyResourceFileToSampleDir(resPPath, "placeholder.jpg");
	_CopyResourceFileToSampleDir(resPPath, "placeholder2.jpg");
	_CopyResourceFileToSampleDir(resPPath, "NoImage.jpg");

	_CopyResourceFileToSampleDir(resIPath, "up-icon.png");

	schemes.ReadAndSetupSchemes();	// from user's directory

	ui.setupUi(this);
	ui.pnlProgress->setVisible(false);

#if defined Q_OS_WINDOWS
	ui.chkSourceRelativePerSign->setChecked(true);
	ui.chkSourceRelativePerSign->setEnabled(false);
#endif

	connect(&_page, &WebEnginePage::LinkClickedSignal, this, &FalconG::_LinkClicked);
	connect(&_page, &WebEnginePage::loadFinished, this, &FalconG::_WebPageLoaded);

	ui.edtWmHorizMargin->setValidator(new QIntValidator(0, 100, ui.edtWmHorizMargin));
	ui.edtWmVertMargin->setValidator(new QIntValidator(0, 100, ui.edtWmVertMargin));

	ui.sample->setPage(&_page);

	QString index_html = PROGRAM_CONFIG::samplePath + (PROGRAM_CONFIG::lang <= 0 ? "index_en.html" : "index_hu.html");
	_page.load(QUrl::fromLocalFile(index_html));

	ui.trvAlbums->setHeaderHidden(true);
	ui.trvAlbums->setModel(new AlbumTreeModel());

	connect(ui.trvAlbums->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FalconG::_AlbumStructureSelectionChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalSingleSelection, this, &FalconG::_TnvSelectionChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalMultipleSelection, this, &FalconG::_TnvMultipleSelection);

	// connect with albumgen's 
	connect(this,	   &FalconG::CancelRun,					&albumgen,	  &AlbumGenerator::Cancelled);
	connect(&albumgen, &AlbumGenerator::SignalToSetProgressParams,	this, &FalconG::_SetProgressBar);
	connect(&albumgen, &AlbumGenerator::SignalProgressPos,			this, &FalconG::_SetProgressBarPos);
	connect(&albumgen, &AlbumGenerator::SignalSetLanguagesToUI,		this, &FalconG::_SetupLanguagesToUI);
	connect(&albumgen, &AlbumGenerator::SignalToEnableEditTab,		this, &FalconG::_EnableEditTab);
//	connect(&albumgen, &AlbumGenerator::SignalImageMapChanged,		this, &FalconG::_ImageMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalAlbumStructChanged,	this, &FalconG::_AlbumMapChanged);
	connect(&albumgen, &AlbumGenerator::SignalToShowRemainingTime,	this, &FalconG::_ShowRemainingTime);
	connect(&albumgen, &AlbumGenerator::SignalToCreateIcon,			this, &FalconG::_CreateUplinkIcon);
	connect(&albumgen, &AlbumGenerator::SetDirectoryCountTo,		this, &FalconG::_SetDirectoryCountTo);

	connect(ui.tnvImages, &ThumbnailView::SignalAlbumStructChanged,	this, &FalconG::_AlbumMapChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalAlbumChanged,		this, &FalconG::_AlbumChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalInProcessing,		this, &FalconG::_ThumbNailViewerIsLoading);
//	connect(ui.tnvImages, &ThumbnailView::SignalTitleChanged,		this, &FalconG::_TrvTitleChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalStatusChanged,		this, &FalconG::_TnvStatusChanged);
	connect(ui.tnvImages, &ThumbnailView::SignalFolderChanged,		this, &FalconG::_SlotChangeToFolderAt);	

	connect(this, &FalconG::SignalThumbSizeChanged, ui.tnvImages, &ThumbnailView::ThumbnailSizeChanged);

	connect(ui.btnSaveChangedDescription, &QPushButton::clicked,	this, &FalconG::_SaveChangedTitleDescription);
	connect(ui.btnSaveChangedTitle,		  &QPushButton::clicked,	this, &FalconG::_SaveChangedTitleDescription);

	// read styles

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


	QSettings s(PROGRAM_CONFIG::homePath+falconG_ini, QSettings::IniFormat);	// in program directory

	restoreGeometry (s.value("wgeometry").toByteArray());
	restoreState(s.value("wstate").toByteArray());

	config.Read();				

	languages.Read();

	ui.designSplitter->setSizes({ PROGRAM_CONFIG::splitterLeft, PROGRAM_CONFIG::splitterRight } );
// DEBUG
//	qDebug("Splitter sizes: %d %d", ui.designSplitter->sizes().at(0), ui.designSplitter->sizes().at(1));

	_tmpScheme = schemes[PROGRAM_CONFIG::schemeIndex];
	_tmpSchemeOrigName = _tmpScheme.MenuTitle;

	frmMain = this;
	// now that everything is ready
	_SetProgramScheme();
	_AddSchemeButtons();
	++_busy;
	on_lwColorScheme_currentRowChanged(0);
	--_busy;

	_ModifyGoogleFontImport();

	_ConfigToUI();

	config.ClearChanged();

	int h = ui.tabEdit->height();
	ui.editSplitter->setSizes({h*70/100,h*30/100});// ({532,220 });

	ui.edtAboutText->Setup();

	_ReadLastAlbumStructure();

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
* TASK:		prevent closing when an operation is running and save changes to
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::closeEvent(QCloseEvent * event)
{
	if (_running)		// operation in progress
	{
		ShowWarning( tr("An operation is in progress.\n Please stop it before exiting."), this);
		event->ignore();
		return;
	}

	if (ui.chkCleanUp->isChecked())		// sample directory will be re-created at next program start
		RemoveDir(PROGRAM_CONFIG::homePath +"sample");

	if(_edited)
	{
		QMessageBox::StandardButtons resB = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
		int res = QuestionDialog( tr("falconG - albums edited"),
								tr("There are unsaved changes in the albums / images\nDo you want to save them?"),
								dbSaveEdited, 
								this,
								tr("Don't ask again (use Options to re-enable)")
							   );
		if (res == QMessageBox::Yes)
		{
			albumgen.WriteDirStruct();	// do not mark albums as not changed
			while (!albumgen.StructWritten())	// wait until write finished
				;
		}
		else if (res == QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}

	QList<int> splitterSizes = ui.designSplitter->sizes();
	if (PROGRAM_CONFIG::splitterLeft != splitterSizes.at(0) && splitterSizes.at(0) >= 360)	// splitter does not change the size if never was visible!
	{
		PROGRAM_CONFIG::splitterLeft = splitterSizes.at(0);
		PROGRAM_CONFIG::splitterRight = splitterSizes.at(1);
	}
	PROGRAM_CONFIG::Write();

	if (config.Changed())
	{				  
		if ( (config.doNotShowTheseDialogs & dbAskBeforeClosing) == 0)
		{
			QMessageBox::StandardButtons btns = QMessageBox::Yes | QMessageBox::No;
			int res = QuestionDialog( tr("falconG"),
							 tr("Do you really want to exit?"),
							 dbAskBeforeClosing,
							 frmMain,
							 tr("Don't ask again (use Options to re-enable))"),
							 btns
							);
			if (res == QMessageBox::No)
			{
				event->ignore();
				return;
			}
		}
		config.Write();
	}
	else
		PROGRAM_CONFIG::Write();	// save data in program directory

	CssCreator cssCreator;
	cssCreator.Create(PROGRAM_CONFIG::samplePath+"css/falconG.css", true);	// program library

	QSettings s(PROGRAM_CONFIG::homePath+falconG_ini, QSettings::IniFormat);	// in program directory
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
	SourceHistory hist(PROGRAM_CONFIG::maxSavedConfigs, PROGRAM_CONFIG::lastConfigs, ui.edtSourceGallery->text(), this);

	hist.exec();
	PROGRAM_CONFIG::maxSavedConfigs = hist.MaxSize();

	if (SourceHistory::Changed() || (SourceHistory::Selected() >= 0 &&
		SourceHistory::Selected() != PROGRAM_CONFIG::indexOfLastUsed) )
	{
		albumgen.Clear();	
		((AlbumTreeModel*)ui.trvAlbums->model())->ModelChanged();
		ui.tnvImages->Clear();

		if (SourceHistory::Selected() >= 0)
		{
			PROGRAM_CONFIG::indexOfLastUsed = SourceHistory::Selected();
			config.Read();

			_ConfigToUI();	// edit values from config

			QString s = ui.edtSourceGallery->text().trimmed();
			if (ui.edtSourceGallery->text().isEmpty())
			{
				++_busy;
				ui.edtSourceGallery->setText(PROGRAM_CONFIG::lastConfigs.at(PROGRAM_CONFIG::indexOfLastUsed));
				--_busy;
			}
			_ConfigToSample();

			_ReadLastAlbumStructure();

			Album *root = albumgen.AlbumForID(ROOT_ALBUM_ID);
			SeparateFileNamePath(config.dsSrc.ToString(), root->path, root->name);
		}
		if (SourceHistory::Changed())
			PROGRAM_CONFIG::Write();
	}
	_EnableButtons();
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
			cssCreator.Create(PROGRAM_CONFIG::samplePath+"css/falconG.css", true);	// program library setting cursors too
		}

		PROGRAM_CONFIG::Write();
		albumgen.SetRecrateAllAlbumFlag(config.bGenerateAll || config.Changed());

		++_running;

		ui.btnSaveStyleSheet->setEnabled(false);

		ui.btnSaveConfig->setEnabled(false);

		ui.btnExit->setEnabled(false);
		ui.actionExit->setEnabled(false);
		ui.btnPreview->setEnabled(false);

		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		ui.btnGenerate->setText(tr("Cancel (F9)"));
		__SetShortcut(ui.btnGenerate, "F9");
		ui.pnlProgress->setVisible(true);

		ui.tabFalconG->setCurrentIndex(0);	// show front page
			   // when interrupted in phase1 (_phase == 0) total generation is used
		_phase = 0;

				// READ album
		if (!albumgen.Read(false))		// any error
			--_running, _phase = -1;
		QGuiApplication::restoreOverrideCursor();
		_TnvCountChanged();			// show in lblTotalCount (page: Edit)

			// WRITE album
		if (_running)
		{
			QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			bool bAlbumWriteOK = albumgen.Write() == 0; // generate struct, pages, images, thumbs
			if (_edited)
			{
				albumgen.SetChangesWritten();	// nothing changed
				albumgen.WriteDirStruct();
			}
			_edited = false;

			ui.btnPreview->setEnabled(bAlbumWriteOK);
			if (!bAlbumWriteOK)		// no switch to edit TAB
				_phase = -1;
		}
		else
			_phase = -1;

		ui.progressBar->setValue(0);

		ui.pnlProgress->setVisible(false);
		QGuiApplication::restoreOverrideCursor();
		ui.btnGenerate->setText(tr("Generate (F9}"));
		__SetShortcut(ui.btnGenerate, "F9");
//		__SetShortcut(ui.btnPreview, "F12");
		ui.btnSaveStyleSheet->setEnabled(true);

		ui.btnSaveConfig->setEnabled  (config.Changed());

		ui.btnExit->setEnabled(true);
		ui.actionExit->setEnabled(true);

		--_running;
		if (_phase != -1)
		{
			ui.tabFalconG->setCurrentIndex(2);	// show 'Edit' page
//			ui.trvAlbums->setCurrentIndex(ui.trvAlbums->model()->index(0,0));
		}
		albumgen.SetRecrateAllAlbumFlag(config.bGenerateAll);	// not until relevant changes
	}

	ui.btnSaveStyleSheet->setEnabled(!_running);
	if (!_running)
		ui.chkReadJAlbum->setChecked(false);
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
		QString qs = "res/up-link.png";		// existing icon
		QFile::remove(qs);					// delete
		QIcon icon = _SetUplinkIcon();		// from original file w. new color (see around line# 2596)
		icon.pixmap(64,64).save(qs);		// and save back
//		qs = "background-image: " + qs;
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
void FalconG::_ElemToSample(AlbumElement ae, ElemSubPart esp)
{
	if (ae == aeUndefined)
		ae = _aeActiveElement;

	_CElem* pElem = _PtrToElement(ae);

	if (ae == aeWebPage)
	{
		if(esp == espAll || esp == espColor)
			_PageColorToSample();
		if(esp == espAll || esp == espBackground)
			_PageBackgroundToSample();
	}
	else
	{
		if(esp == espAll || esp == espColor)
			_ColorToSample(pElem);
		if(esp == espAll || esp == espBackground)
			_BackgroundToSample(pElem);
	}
	if(esp == espAll || esp == espFont)
		_FontToSample(pElem);
	if(esp == espAll || esp == espSpaceAfter)
		_SpaceAfterToSample(pElem);
	if(esp == espAll || esp == espTextDecor)
		_DecorationToSample(pElem);
	if(esp == espAll || esp == espShadow1 || esp == espShadow)
		_ShadowToSample(pElem, 0);	// text
	if(esp == espAll || esp == espShadow2 || esp == espShadow1)
		_ShadowToSample(pElem, 1);	// box
	if(esp == espAll || esp == espGradient)
		_SetLinearGradient(pElem);
	if(esp == espAll || esp == espBorder)
		_BorderToSample(pElem);
	if(esp == espAll || esp == espTextAlign)
		_TextAlignToSample(pElem);

	// menus: set uplink icon
	if (ae == aeMenuButtons)
	{	
		if(esp == espAll || esp == espLinkIcon)
			_SaveLinkIcon();
		if(esp == espAll || esp == espBackgroundImage)
			_SetCssProperty(&config.Menu, "background-image:url(\"res/up-link.png\")", "#uplink");
	}
}

void FalconG::_ElemToSample(ElemSubPart esp)
{
	_ElemToSample(_aeActiveElement, esp);
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

/*=============================================================
 * TASK:	fill in interface lements from stored value
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_ActualSampleParamsToUi()
{
	_CElem* pElem = _PtrToElement();

	if (pElem->kind == aeThumb)		// set in _ConfigToUI and no need to update it
		return;

	++_busy;

	ui.btnForeground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->color.Name()));
	ui.chkTextOpacity->setChecked(pElem->color.IsOpacityUsed() );
	if(pElem->background.Valid())
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(pElem->background.Name()));	// sets btnBackground's color
	else // invalid: set from background
		ui.btnBackground->setStyleSheet(__ToolButtonBckStyleSheet(config.Web.background.Name()));	// sets btnBackground's color
	ui.chkBackgroundOpacity->setChecked(pElem->background.IsOpacityUsed());

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

	ui.chkBackgroundOpacity->setChecked(pElem->background.IsOpacityUsed());
	ui.sbBackgroundOpacity->setValue(pElem->background.Opacity(true));
	ui.sbBackgroundOpacity->setEnabled(ui.chkBackgroundOpacity->isChecked());

	ui.sbSpaceAfter->setValue(pElem->spaceAfter.v);

	ui.edtFontFamily->setText(pElem->font.Family());
	int ix = ui.cbFonts->findText(pElem->font.Family());
	if (ix < 0) ix = 0;	// not found -> empty
	ui.cbFonts->setCurrentIndex(ix);

	ui.cbFontSizeInPoints->setCurrentText(pElem->font.SizeStr());
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

	ui.cbPointSizeFirstLine->setCurrentText(pElem->font.FirstLineFontSizeStr());

	--_busy;

		// border for actual element
	int used = pElem->border.UsedSides();
	BorderSide first = sdAll;
	if (!used)
	{
		++_busy;
		ui.rbAllBorders->setChecked(true);	// bits: 1:top,2:right,4:bottom,8:left
		--_busy;
	}
	else if(used == 15)	// something must be checked
		ui.rbAllBorders->setChecked(true);	// bits: 1:top,2:right,4:bottom,8:left
	else
	{
		if(used & 1)
			ui.rbTopBorder->setChecked(true), first = sdTop;
		else if (used & 2)
			ui.rbRightBorder->setChecked(true), first = sdRight;
		else if (used & 4)
			ui.rbBottomBorder->setChecked(true), first = sdBottom;
		else
			ui.rbLeftBorder->setChecked(true), first = sdLeft;
	}
	++_busy;
	ui.btnBorderColor->setStyleSheet(QString("QToolButton { background-color:" + pElem->border.ColorStr(first)+";\n color:"+ config.Web.background.Name()+";}\n"));
	ui.cbBorderStyle->setCurrentIndex(pElem->border.StyleIndex(first));
	ui.sbBorderWidth->setValue(pElem->border.Width(first));
	ui.sbBorderRadius->setValue(pElem->border.Radius());
	--_busy;
}


/*============================================================================
* TASK:		set all characteristics of the sample gallery from 'config'
* EXPECTS: 'config' is set up
* GLOBALS:  'config'
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::_ConfigToSample(ElemSubPart esp)
{
	++_busy;		// prevent page changes until last settings

	_ElemToSample(aeWebPage, esp);
	_ElemToSample(aeHeader, esp);
	_ElemToSample(aeMenuButtons, esp);
	_ElemToSample(aeLangButton, esp);
	_ElemToSample(aeSmallTitle, esp);
	_ElemToSample(aeGalleryTitle, esp);
	_ElemToSample(aeGalleryDesc, esp);
	_ElemToSample(aeSection, esp);
	_ElemToSample(aeImageTitle, esp);
	_ElemToSample(aeImageDesc, esp);
	_ElemToSample(aeLightboxTitle, esp);
	_ElemToSample(aeLightboxDescription, esp);
	_ElemToSample(aeFooter, esp);

	_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));

	QString qs = QString("QToolButton {background-color:%1;color:%2;}").arg(config.imageBorder.ColorStr(sdAll)).arg(config.Web.background.Name());
	ui.btnImageBorderColor->setStyleSheet(qs);
	qs = config.imageBorder.ForStyleSheetShort(false);
	_RunJavaScript("imatte", qs);
	_RunJavaScript("amatte", qs);

	--_busy;
}

void FalconG::_SetConfigChanged(bool on)
{
	ui.btnSaveConfig->setEnabled(config.SetChanged(on));
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
	QAction* pa = new QAction(tr("Styles"), this);
	pa->setEnabled(false);
	menu.addAction(pa);
	menu.addSeparator();
	for (int i = 0; i < schemes.size(); ++i)
	{
		pa = menu.addAction(schemes[i].MenuTitleForLanguage(PROGRAM_CONFIG::lang), _popupMapper.get(), SLOT(map()));
		if (i == PROGRAM_CONFIG::schemeIndex)
		{
			pa->setCheckable(true);
			pa->setChecked(true);
		}
		_popupMapper->setMapping(pa, i);
	}
	connect(_popupMapper.get(), &QSignalMapper::mappedInt, this, &FalconG::_SlotForSchemeChange);
	menu.exec(dynamic_cast<QWidget*>(sender())->mapToGlobal(pt));
}

void FalconG::_SlotForSchemeChange(int which)
{
	PROGRAM_CONFIG::schemeIndex = (which >= 0 && which < schemes.size()) ? which : 0;
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	_SetProgramScheme();
	_SetConfigChanged(true);
	QGuiApplication::restoreOverrideCursor();
}

void FalconG::_SlotChangeToFolderAt(int row)		// called from thumbnailView.cpp
{														// row: item index
	QModelIndex &cix = _currentTreeViewIndex;	// shorthand
	// get folder (album) index for row
	Album *album = albumgen.AlbumForID((ID_t)cix.internalPointer());
	int aix = 0;
	for (int i = 0; i < row; ++i)
		if (album->items[i] & ALBUM_ID_FLAG)
			++aix;

	QModelIndex mix = ((AlbumTreeModel*)ui.trvAlbums->model())->index(aix, 0, cix);
	ui.trvAlbums->selectionModel()->select(mix, QItemSelectionModel::ClearAndSelect);
}

void FalconG::_EnableColorSchemeButtons()
{
	bool b = _bSchemeChanged;
	ui.btnApplyColorScheme->setEnabled(b);
	ui.btnResetColorScheme->setEnabled(b);
	//	b = (PROGRAM_CONFIG::schemeIndex > 2); // - 2) != ui.lwColorScheme->currentRow();
	ui.btnDeleteColorScheme->setEnabled(schemes.size() > 2);
	int i = ui.lwColorScheme->currentRow(); // is 2 less than  the real index in the schemes array
	ui.btnMoveSchemeUp->setEnabled(i > 0);
	ui.btnMoveSchemeDown->setEnabled(i >= 0 && i < schemes.size() - 2 - 1);
}

void FalconG::_SlotForSchemeButtonClick(int which)
{
	if (_pSchemeButtons.isEmpty())
		return;

	StyleHandler sh((*_pSchemeButtons[which]).styleSheet());
	QColor qc = sh.GetItem("", "background-color");
	QColor qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew == qc || !qcNew.isValid())
		return;
	sh.SetItem("", "background-color", qcNew.name());
	_pSchemeButtons[which]->setStyleSheet(sh.StyleSheet());
	if (!which)
		ui.pnlColorScheme->setStyleSheet("background-color:" + qcNew.name() + "; color:" + _tmpScheme.sTextColor);
	else if (which == 1)
		ui.pnlColorScheme->setStyleSheet("color:" + qcNew.name() + "; background-color:" + _tmpScheme.sBackground);
	_bSchemeChanged |= true;	// might have been changed already
	_tmpScheme[which] = qcNew.name();
	_bSchemeChanged = true;
	_EnableColorSchemeButtons();
}


/*========================================================
 * TASK:	set up controls on 'Global settings for Page' 
 *			'Selected Item'and Watermark page
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_DesignToUi()
{
	++_busy;
				// globals
	_SettingUpFontsCombo();
	_GlobalsToUi();			
	
	StyleHandler handler;

	_ActualSampleParamsToUi();
		// selected item
	//_CElem* pElem = _PtrToElement(_aeActiveElement);

	//ui.chkUseGradient->setChecked(pElem->gradient.used);
	//ui.chkUseWM->setChecked(config.waterMark.used);

	//ui.btnGradMiddleColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[1].color));
	//ui.btnGradStartColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[0].color));
	//ui.btnGradStopColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[2].color));

	// images
	ui.cbImageBorderStyle->setCurrentIndex(config.imageBorder.StyleIndex(sdAll));
	ui.sbImageBorderWidth->setValue(config.imageBorder.Width(sdAll));
	ui.sbImageBorderRadius->setValue(config.imageBorder.Radius());
	ui.sbImageMatteWidth->setValue(config.imageMatteWidth);
	ui.sbImageMatteRadius->setValue(config.imageMatteRadius);

	ui.btnImageMatteColor->setStyleSheet(__ToolButtonBckStyleSheet(config.imageMatteColor.Name()));

	ui.sbAlbumMatteRadius->setValue(config.albumMatteRadius);
	ui.sbAlbumMatteWidth->setValue(config.albumMatteWidth);

	ui.btnAlbumMatteColor->setStyleSheet(__ToolButtonBckStyleSheet(config.albumMatteColor.Name()));

	ui.sbSpaceAfter->setValue(0);

				// WaterMark
	handler.Set(ui.btnWmColor->styleSheet());
	handler.SetItem("QToolButton", "background-color", QColor(config.waterMark.Color()).name());
	handler.SetItem("QToolButton", "border", QString("1px solid %1").arg(config.waterMark.BorderColor().name()) );
	ui.btnWmColor->setStyleSheet(handler.StyleSheet());
	ui.sbWmOpacity->setValue( (int)(config.waterMark.Opacity(true)));
	//ui.btnWmColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.Color(),8,16,QChar('0')));
	if(config.waterMark.ShadowColor() >= 0)
		ui.btnWmShadowColor->setStyleSheet(QString("QToolButton { background-color:#%1}").arg(config.waterMark.ShadowColor(),6,16,QChar('0')));
	ui.edtWmHorizMargin->setText(QString().setNum(config.waterMark.MarginX()));
	ui.edtWmVertMargin->setText(QString().setNum(config.waterMark.MarginY()));

	--_busy;
}

static void _SetEditText(QLineEdit* pe, _CDirStr cs, bool force=true)
{
	if (cs.Changed() || force)
		pe->setText(cs.ToString());
}

static inline bool _TestDirValue(QLineEdit* pe, _CString cs)
{
	return  (!cs.IsEmpty() && pe->placeholderText() != cs.ToString() && pe->placeholderText() + "/" != cs.ToString());
}
static inline bool _TestDirValue(QLineEdit* pe, _CDirStr cs)
{
	return  (!cs.IsEmpty() && pe->placeholderText() != cs.ToString() && pe->placeholderText() + "/" != cs.ToString());
}

void FalconG::_OtherToUi()
{
	++_busy;
	ui.edtAbout->setText(config.sAbout);
	_SetEditText(ui.edtAlbumDir, config.dsAlbumDir, _busy);
	ui.edtDefaultFonts->setText(config.sDefFonts.ToString());
	ui.edtDescription->setText(config.sDescription);
	ui.edtDestGallery->setText(QDir::toNativeSeparators(config.dsGallery.ToString()));
	ui.edtEmailTo->setText(config.sMailTo);
	_SetEditText(ui.edtGalleryRoot, config.dsGRoot, _busy);
	ui.edtGalleryTitle->setText(config.sGalleryTitle);
	QString s = config.sGoogleFonts.ToString();
	s.replace('|', ',');s.replace('+', ' ');
	ui.edtGoogleFonts->setText(s);
	ui.edtKeywords->setText(config.sKeywords);
	ui.edtServerAddress->setText(config.sServerAddress);
	ui.edtSourceGallery->setText(QDir::toNativeSeparators(config.dsSrc.ToString()));
	ui.edtTrackingCode->setText(config.googleAnalTrackingCode);
	ui.edtUplink->setText(config.sUplink);
	if (_TestDirValue(ui.edtAlbumDir, config.dsAlbumDir))
		ui.edtAlbumDir->setText(config.dsAlbumDir.ToString());
	if (_TestDirValue(ui.edtBaseName, config.sBaseName))
		ui.edtBaseName->setText(config.sBaseName);
	if (_TestDirValue(ui.edtFontDir, config.dsFontDir))
		ui.edtFontDir->setText(config.dsFontDir.ToString());
	if (_TestDirValue(ui.edtImg, config.dsImageDir))
		ui.edtImg->setText(config.dsImageDir.ToString());
	if (_TestDirValue(ui.edtVid, config.dsVideoDir))
		ui.edtVid->setText(config.dsVideoDir.ToString());
	if (_TestDirValue(ui.edtThumb, config.dsThumbDir))
		ui.edtThumb->setText(config.dsThumbDir.ToString());

	ui.edtWatermark->setText(config.waterMark.Text());

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
//	ui.chkReadJAlbum->setChecked(config.bReadFromDirs);	  
	ui.chkKeepDuplicates->setChecked(config.bKeepDuplicates);
	ui.chkRightClickProtected->setChecked(config.bRightClickProtected);
	ui.chkSetLatest->setChecked(config.bGenerateLatestUploads);
	ui.chkFixedLatestThumbnail->setChecked(config.bFixedLatestThumbnail);
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
	ui.sbWmOpacity->setValue(config.waterMark.Opacity(true));
	ui.sbNewDays->setValue(config.newUploadInterval);
	ui.sbImageWidth->setValue(config.imageWidth);
	ui.sbImageHeight->setValue(config.imageHeight);
	ui.sbThumbnailWidth->setValue(config .thumbWidth);
	ui.sbThumbnailHeight->setValue(config.thumbHeight);

	h = config.waterMark.Origin() & 0xF;  // vertical position
	ui.cbWmVPosition->setCurrentIndex(h);
	h = (config.waterMark.Origin() & 0xF0) >> 4; // horizontal position
	ui.cbWmHPosition->setCurrentIndex(h);
	

	ui.lblWaterMarkFont->setText(QString("%1 (%2,%3)").arg(config.waterMark.Font().family()).arg(config.waterMark.Font().pointSize()).arg(config.waterMark.Font().italic()));
	if (!config.waterMark.Text().isEmpty())
		ui.lblWmSample->setText(config.waterMark.Text());

	ui.lblWmSample->setFont(config.waterMark.Font());

	h = config.imageQuality;
	ui.cbImageQuality->setCurrentIndex(h ? 11 - h/10 : 0);
	if (PROGRAM_CONFIG::lang <= 0)
		ui.rbEnglish->setChecked(true);
	else if (PROGRAM_CONFIG::lang == 1)
		ui.rbMagyar->setChecked(true);


	ui.edtAboutText->SetupWebFonts(config.sDefFonts + "|"+ config.sGoogleFonts);

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
  * EXPECTS:mx, my new margins.  -1 : do not set this margin
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::_UpdateWatermarkMargins(int mx, int my)
{
	WaterMark& wm = config.waterMark;
	if (mx >= 0)
		wm.SetMarginX(mx);
	if (my >= 0)
		wm.SetMarginY(my);
	config.waterMark.SetupMark();
	_WaterMarkToSample();
	if (config.waterMark.used)
	{
		double ratio = (double)config.thumbWidth / (double)config.imageWidth;

		_RunJavaScript("thumb::after", wm.PositionToStyle(config.imageWidth, config.imageHeight, ratio, (WaterMark::POS)wm.Origin()));	// 2 images
		_RunJavaScript("athumb::after", wm.PositionToStyle(config.imageHeight, config.imageHeight, ratio, (WaterMark::POS)wm.Origin())); // album  square format image
	}
	_SetConfigChanged(true);
}
void FalconG::_SetupActualBorder(BorderSide side)
{
	_CElem* pElem = _PtrToElement();
	_CBorder &border = pElem->border;
	BorderSide oldSide = border.actSide;
	if ((border.actSide = side) == sdAll)	// then use last set values to all sides
	{
		border.SetStyleIndex(sdAll, border.StyleIndex(oldSide));
		border.SetWidth(sdAll, border.Width(oldSide));
		border.SetColor(sdAll, border.ColorStr(oldSide));
	}
	else	// change to data for actual side
	{
		++_busy;
		ui.cbBorderStyle->setCurrentIndex(border.StyleIndex(side));
		ui.sbBorderWidth->setValue(border.Width(side));
		ui.btnBorderColor->setStyleSheet(QString("QToolButton {background-color:%1;color:%2;}").arg(border.ColorStr(side)).arg(pElem->background.Name()));
		--_busy;
	}

	QString qs = pElem->border.ForStyleSheet(false);
	_SetCssProperty(pElem, qs);
	_EnableButtons();
}

void FalconG::_PropagatePageColor()
{
	config.Header.color = config.Web.color.Name();
	config.Menu.color = config.Web.color.Name(); 
	config.Lang.color = config.Web.color.Name(); 
	config.SmallGalleryTitle.color = config.Web.color.Name(); 
	config.GalleryTitle.color = config.Web.color.Name();
	config.GalleryDesc.color = config.Web.color.Name();
	config.Section.color = config.Web.color.Name(); 
	config.Thumb.color = config.Web.color.Name();
	config.ImageTitle.color = config.Web.color.Name();
	config.ImageDesc.color = config.Web.color.Name();
	config.LightboxTitle.color = config.Web.color.Name();
	config.LightboxDesc.color = config.Web.color.Name();
	config.Footer.color = config.Web.color.Name();
 
	_ConfigToSample(espColor);
}

//****************************************************************************

void FalconG::_RestartRequired()
{
	QMessageBox::warning(this, tr("falconG - Warning"), QString(tr("Please restart the program to change the language!")));
}

bool FalconG::_DoOverWriteColorScheme(int i)
{
	QString qs = tr("There is a scheme \n'%1'\nwith a title which at least partially\nmatches the modified title.\n"
		" Do you want to overwrite it?").arg(schemes[i].MenuTitle);
	return QMessageBox::question(this, tr("falconG - Question"),
		qs,
		QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

bool FalconG::_LanguagesWarning()
{
	return QMessageBox::question(this, tr("falconG - Question"),
		tr("No/not enough ':' in new name. The same name will be used for\n"
			"all program languages. Is this what you want?")) == QMessageBox::Yes;
}

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
	if (iconName.isEmpty())
		iconName = ":/icons/Resources/up-icon.png";
	QIcon icon(iconName);
	_lastUsedMenuForegroundColor = Qt::white;
	_SetIconColor(icon, config.Menu);
	return icon;
}

void FalconG::_ReadLastAlbumStructure()
{
	// read last album structure
	if (PROGRAM_CONFIG::indexOfLastUsed >= 0)
	{
		QString qs = PROGRAM_CONFIG::NameForConfig(false, ".struct");
		if (QFile::exists(qs) && albumgen.Read(true))
		{

			Album *root = albumgen.AlbumForID(ROOT_ALBUM_ID);
			SeparateFileNamePath(config.dsSrc.ToString(), root->path, root->name);

			_AlbumStructureSelectionChanged(QItemSelection(), QItemSelection());
			_TnvCountChanged();			// show in lblTotalCount (page: Edit)
			ui.trvAlbums->setCurrentIndex(ui.trvAlbums->model()->index(0, 0));
		}
	}
	_edited = false;
}


// ======================== Buttons

void FalconG::on_btnAddAndGenerateColorScheme_clicked()
{
	bool ok;
	QString qs = PROGRAM_CONFIG::LangNameListWDelim();
	qs = tr("New Scheme Name\n (for ") + qs + ")";
	QString sNewName = QInputDialog::getText(this, tr("falconG - Input"), qs, QLineEdit::Normal, QString(), &ok);
	if (!ok || sNewName.isEmpty())
		return;
	if (PROGRAM_CONFIG::qslLangNames.size() != sNewName.count(':') + 1)
	{
		if (!_LanguagesWarning())
			return;
		sNewName = sNewName + ":" + sNewName;
	}

	int i = schemes.IndexOf(sNewName);
	if (i >= 0)
	{
		if (i < 2)
			QMessageBox::warning(this, tr("falconG - Warning"), tr("Invalid new name. Please use another!"));
		else if (_DoOverWriteColorScheme(i))
		{
			schemes[i].MenuTitle = sNewName;
			schemes.Save();
			delete ui.lwColorScheme->takeItem(i - 2);
			ui.lwColorScheme->insertItem(i - 2, schemes[i].MenuTitleForLanguage(PROGRAM_CONFIG::lang));
			ui.lwColorScheme->setCurrentRow(i - 2);
		}
		return;
	}

	QColor qcBase = QColorDialog::getColor(qcBase, this, tr("Select background color."));
	if (qcBase.isValid())
	{
		QColor qc;
		_tmpScheme.MenuTitle = _tmpSchemeOrigName = sNewName;
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
		ui.lwColorScheme->addItem(_tmpScheme.MenuTitleForLanguage(PROGRAM_CONFIG::lang));
		schemes.push_back(_tmpScheme);
		schemes.Save();
		ui.lwColorScheme->setCurrentRow(schemes.size() - 2 - 1);	// 2 for default and system colors, 1: offset vs size
		ui.btnApplyColorScheme->setEnabled(true);
	}
}

void FalconG::on_btnAlbumMatteColor_clicked()
{
	StyleHandler handler(ui.btnAlbumMatteColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color")),
		qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew.isValid() && qc != qcNew)
	{
		handler.SetItem("QToolButton", "background-color", qcNew.name());
		handler.SetItem("QToolButton", "color", config.Web.background.Name());
		ui.btnAlbumMatteColor->setStyleSheet(handler.StyleSheet());
		config.albumMatteColor = qcNew.name();
		_RunJavaScript("amatte", QString("background-color:") + config.albumMatteColor.Name());
		_SetConfigChanged(true);
	}
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_btnBorderColor_clicked()
{
	_CElem* pElem = _PtrToElement();
	QColor qc(pElem->border.ColorStr(pElem->border.actSide)),
		qcNew;
	qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew == qc || !qcNew.isValid())
		return;

	_SetConfigChanged(true);
	pElem->border.SetColor(pElem->border.actSide, qcNew.name());
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
void FalconG::on_btnBrowseDestination_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Select Destination Directory", ui.edtDestGallery->text().isEmpty() ? "" : ui.edtDestGallery->text());
	if (!s.isEmpty())
		ui.edtDestGallery->setText(s);
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
void FalconG::on_btnBrowseSource_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Select Source Directory", ui.edtSourceGallery->text().isEmpty() ? "" : ui.edtSourceGallery->text());
	if (!s.isEmpty())
		ui.edtSourceGallery->setText(s);
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
 *			- the first tow shemes (Default and System Colors)
 *				cannot be deleted or edited!
 *-------------------------------------------------------*/
void FalconG::on_btnDeleteColorScheme_clicked()
{
	if (QMessageBox::warning(this, tr("falconG - Warning"),
		tr("Do you really want to delete this color scheme?"),
		QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
		return;

	int i = ui.lwColorScheme->currentRow();
	if (i + 2 < PROGRAM_CONFIG::schemeIndex)
	{
		PROGRAM_CONFIG::schemeIndex = PROGRAM_CONFIG::schemeIndex - 1;
		if (PROGRAM_CONFIG::schemeIndex < 0)
			PROGRAM_CONFIG::schemeIndex = 0;

		PROGRAM_CONFIG::Write();
	}
	delete ui.lwColorScheme->takeItem(i);
	schemes.remove(i + 2);
	ui.lwColorScheme->setCurrentRow(PROGRAM_CONFIG::schemeIndex);
	schemes.Save();
	_EnableColorSchemeButtons();
	_bSchemeChanged = false;
}

/*========================================================
 * TASK:	Return the original web page, throwing away the
 *			changes
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_btnReload_clicked()
{
	config.RestoreDesign();
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
	if (PROGRAM_CONFIG::splitterLeft != splitterSizes.at(0) && splitterSizes.at(0) >= 360)	// splitter does not change the size if never was visible!
	{
		PROGRAM_CONFIG::splitterLeft = splitterSizes.at(0);
		PROGRAM_CONFIG::splitterRight = splitterSizes.at(1);
	}
	config.Write();

	QString s = PROGRAM_CONFIG::NameForConfig(true, ".ini"),
		sp = s.left(s.lastIndexOf('/'));	// path
	s = s.mid(s.lastIndexOf('/') + 1);			// name
	InformationMessage(false,
		"falconG",
		tr("Saved configuration\n'%1'\n into folder \n'%2'").arg(s).arg(sp),
		dbSaveConfig,
		tr("Don't ask again (use Options to re-enable)"),
		this
	);
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

void FalconG::on_btnBackToParentAlbum_clicked()
{
	QModelIndex mix = ui.trvAlbums->currentIndex();
	if (!mix.isValid() || !mix.parent().isValid())
		return;

	ui.trvAlbums->setCurrentIndex(mix.parent()); // ???
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
void FalconG::on_btnPageBackground_clicked()
{
	if (_busy)
		return;

	QColor qc(config.Web.background.Name()),
		qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));

	if (qcNew.isValid())
	{
		config.Web.background.SetColor(qcNew.name());
		config.Web.background.SetOpacity(255, true, false);	// always 100% opacity
		ui.btnPageBackground->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.background.Name()));
		_SetConfigChanged(true);
		_PageBackgroundToSample();
	}
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

	if (qcNew.isValid())
	{
		if (qc != qcNew)
		{
			config.Web.color.SetColor(qcNew.name());
			ui.btnPageColor->setStyleSheet(QString("QToolButton {background-color:%1;}").arg(config.Web.color.Name()));
			_SetConfigChanged(true);
		}
		_PageColorToSample();
	}
	if (ui.chkSameForeground->isChecked())	// propagate color to all elements
	{
		_PropagatePageColor();
		ui.chkSameForeground->setChecked(false);
	}
	// DEBUG
	//	ShowStyleOf(ui.btnPageColor);
}

void FalconG::on_btnOpenBckImage_clicked()
{
	QString qs = QFileDialog::getOpenFileName(this, tr("falconG - Open Background Image"), QString(), tr("Image files (*.bmp *.gif *.jpg *.png)"));
	if (!qs.isEmpty())
		ui.edtBckImageName->setText(qs);
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
		_SetConfigChanged(true);
	}
}

void FalconG::on_btnImageMatteColor_clicked()
{
	StyleHandler handler(ui.btnImageMatteColor->styleSheet());

	QColor qc(handler.GetItem("QToolButton", "background-color")),
		qcNew = QColorDialog::getColor(qc, this, tr("Select Color"));
	if (qcNew.isValid() && qc != qcNew)
	{
		handler.SetItem("QToolButton", "background-color", qcNew.name());
		handler.SetItem("QToolButton", "color", config.Web.background.Name());
		ui.btnImageMatteColor->setStyleSheet(handler.StyleSheet());
		config.imageMatteColor = qcNew.name();
		_RunJavaScript("imatte", QString("background-color:") + config.imageMatteColor.Name());
		_SetConfigChanged(true);
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

	_CElem* pElem = _PtrToElement();
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

// ======================== Combo Boxes

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_cbFontSizeInPoints_currentTextChanged(const QString& txt)
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
	_ElemToSample(espFont);
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_cbLineHeight_currentTextChanged(const QString& txt)
{
	if (_busy || txt.isEmpty())
		return;
	_CElem* pElem = _PtrToElement(_aeActiveElement);
	pElem->font.SetLineHeight(txt);
	_FontToSample(pElem);
}

void FalconG::on_cbPointSizeFirstLine_currentTextChanged(const QString& txt)
{
	if (_busy || txt.isEmpty())
		return;
	_CElem* pElem;

	QString qsClass;
	pElem = _PtrToElement();
	pElem->font.SetDifferentFirstLine(pElem->font.IsFirstLineDifferent(), txt);	// do not change first line font size
	_SetConfigChanged(true);
	_ElemToSample(espFont);
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
	int o = config.waterMark.Origin();
	o &= 0xF;
	o += index << 4;
	config.waterMark.SetPositioning(o);
	config.waterMark.SetupMark();
	ui.edtWmHorizMargin->setEnabled(index != 1);
	_WaterMarkToSample();
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
	int o = config.waterMark.Origin();
	o &= 0xF0;
	o += index;
	config.waterMark.SetPositioning(o);
	config.waterMark.SetupMark();
	ui.edtWmVertMargin->setEnabled(index != 1);
	_WaterMarkToSample();
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
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_cbDefineLanguge_currentIndexChanged(int index)
{
}

void FalconG::on_cbLanguageTextDef_currentTextChanged(QString text)
{
	if (_busy || text.isEmpty())
		return;
	LangConstList* textList = languages[text];
	ui.edtLanguageTextdefinition->setText(textList->Definition());
	ui.edtLanguageTextText->setHtml((*textList)[ui.cbDefineLanguge->currentIndex()]);
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


/*========================================================
 * TASK:	when the index changes sets up _tmpScheme
 *			and color buttons
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_lwColorScheme_currentRowChanged(int newIndex)
{
	if (_busy || newIndex < 0)
		return;

	if (_bSchemeChanged)
		_AskForApply();
	static const char* bckstr = "background-color:%1",
		* bck_txt = "background-color:%1; color:%2";
	_tmpScheme = schemes[newIndex + 2];	// default and system are not changed
	_tmpSchemeOrigName = _tmpScheme.MenuTitle;
	ui.pnlColorScheme->setStyleSheet(QString(bck_txt).arg(_tmpScheme.sBackground).arg(_tmpScheme.sTextColor));
	for (int i = 0; i < _pSchemeButtons.size(); ++i)
		_pSchemeButtons[i]->setStyleSheet(QString(bckstr).arg(_tmpScheme[i]));
	_EnableColorSchemeButtons();
}


void FalconG::on_cbImageQuality_currentIndexChanged(int newIndex)
{
	if (_busy)
		return;
	config.imageQuality = newIndex ? (11 - newIndex) * 10 : 0;
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
		QMessageBox(QMessageBox::Warning, tr("falconG - Warning"),
			tr("Cannot open\n'%1'").arg(qs),
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

	int i = schemes.IndexOf(_tmpSchemeOrigName);
	if (i >= 0)	// then at least one language text of the title is matched
	{
		if (i < 2)
		{
			QMessageBox::warning(this, tr("falconG - Warning"), tr("Invalid new name. Please use another!"));
			return;
		}
		else if (schemes[i].MenuTitle == _tmpSchemeOrigName || _DoOverWriteColorScheme(i))// full or partial name matched
		{
			schemes[i] = _tmpScheme;
			_bSchemeChanged = true;
		}
	}
	else	// name did not exist save new
	{
		if (_tmpSchemeOrigName.indexOf(':') + 1 != PROGRAM_CONFIG::qslLangNames.size() && _LanguagesWarning())
		{
			_tmpScheme.MenuTitle = _tmpSchemeOrigName;
			schemes.push_back(_tmpScheme);
			ui.lwColorScheme->addItem(_tmpSchemeOrigName);
			ui.lwColorScheme->setCurrentRow(ui.lwColorScheme->count() - 1);
		}
	}
	if (_bSchemeChanged)
		schemes.Save();

	_bSchemeChanged = false;
	_EnableColorSchemeButtons();
	_SetProgramScheme();
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
	_tmpScheme = schemes[ui.lwColorScheme->currentRow() + 2];
	_tmpSchemeOrigName = _tmpScheme.MenuTitle;
	ui.lwColorScheme->setCurrentRow(0);
	_bSchemeChanged = false;
	_EnableColorSchemeButtons();
	_SetProgramScheme();
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
	int i = ui.lwColorScheme->currentRow(),  //0: corresponds to real index 2 in 'schemes'
		j = i + 2;							  // index in schemes (default and system can't be changed)
	if (!i)
		return;

	if (j == PROGRAM_CONFIG::schemeIndex)
	{
		PROGRAM_CONFIG::schemeIndex = j - 1;
		PROGRAM_CONFIG::Write();
	}
	else if (j == PROGRAM_CONFIG::schemeIndex + 1)
	{
		PROGRAM_CONFIG::schemeIndex = j + 1;
		PROGRAM_CONFIG::Write();
	}

	FalconGScheme sty = schemes[j];
	schemes.replace(j, schemes[j - 1]);
	schemes.replace(--j, sty);

	++_busy;
	QListWidgetItem* plwi = ui.lwColorScheme->takeItem(i);
	ui.lwColorScheme->insertItem(i - 1, plwi);
	ui.lwColorScheme->setCurrentRow(i - 1);
	--_busy;
	_EnableColorSchemeButtons();
	schemes.Save();
}

/*========================================================
 * TASK:	moves down the actual scheme
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - no need to change in display changes,
 *				but need to s change the order of schemes
 *-------------------------------------------------------*/
void FalconG::on_btnMoveSchemeDown_clicked()
{
	int i = ui.lwColorScheme->currentRow(),  //0: corresponds to real index 2 in 'schemes'
		j = i + 2;							 // index in schemes (default and system can't be changed)
	if (i == schemes.size() - 2 - 1)
		return;

	if (j == PROGRAM_CONFIG::schemeIndex)
	{
		PROGRAM_CONFIG::schemeIndex = j + 1;
		PROGRAM_CONFIG::Write();
	}
	else if (j == PROGRAM_CONFIG::schemeIndex - 1)
	{
		PROGRAM_CONFIG::schemeIndex = j - 1;
		PROGRAM_CONFIG::Write();
	}

	FalconGScheme sty = schemes[j];
	schemes.replace(j, schemes[j + 1]);
	schemes.replace(++j, sty);

	++_busy;
	QListWidgetItem* plwi = ui.lwColorScheme->takeItem(i);
	ui.lwColorScheme->insertItem(i + 1, plwi);
	ui.lwColorScheme->setCurrentRow(i + 1);
	--_busy;
	schemes.Save();
	_EnableColorSchemeButtons();
}

void FalconG::on_btnResetDialogs_clicked()
{
	if (QMessageBox::question(this, tr("falconG - Warning"), tr("This will reset all dialogs.\nDo you want to proceed?")) == QMessageBox::Yes)
		config.doNotShowTheseDialogs = 0;
}


// ======================== Checkboxes

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
void FalconG::on_chkBackgroundOpacity_toggled(bool on)
{
	if (_busy)
		return;
	ui.sbBackgroundOpacity->setEnabled(on);
	_CElem* pElem = _PtrToElement();
	int val = ui.sbBackgroundOpacity->value();
	pElem->background.SetOpacity(val, on, true);	// in percent
	_SetConfigChanged(true);
	_ElemToSample(espBackground);
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

void FalconG::on_cbBorderStyle_currentIndexChanged(int newIndex)
{
	if (_busy)
		return;

	_CElem* pElem = _PtrToElement();
	_CBorder& border = pElem->border;
	border.SetStyleIndex(pElem->border.actSide, newIndex);
	border.SetUsed(border.actSide, newIndex);

	_SetConfigChanged(true);
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
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
void FalconG::on_chkCanDownload_toggled(bool b)
{
	if (_busy)
		return;
	config.bCanDownload = b;
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

void FalconG::on_chkDebugging_toggled(bool b)
{
	config.bDebugging = b;
}

void FalconG::on_chkDifferentFirstLine_toggled(bool b)
{
	if (_busy)
		return;
	_PtrToElement()->font.SetDifferentFirstLine(b);	// do not change first line font size
	_SetConfigChanged(true);
	_ElemToSample(espFont);
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
void FalconG::on_chkDoNotEnlarge_toggled(bool on)
{
	if (_busy)
		return;
	config.doNotEnlarge = on;
	_SetConfigChanged(true);

	_EnableButtons();
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

void FalconG::on_chkFacebook_toggled(bool on)
{
	config.bFacebookLink.v = on;
}

/*=============================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_chkFixedLatestThumbnail_toggled(bool b)
{
	if (_busy)
		return;

	config.bFixedLatestThumbnail = b;
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
  * REMARKS: disabled, not used
 *--------------------------------------------------------------------------*/
void FalconG::on_chkIconText_toggled(bool)
{
	if (_busy)
		return;
	_SetConfigChanged(true);
	_ElemToSample(espLinkIcon);
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

void FalconG::on_cbImageBorderStyle_currentIndexChanged(int newIndex)
{
	if (!_busy)
	{
		config.imageBorder.SetUsed(sdAll, newIndex);
		config.imageBorder.SetStyleIndex(sdAll, newIndex);
		_SetConfigChanged(true);
		_RunJavaScript("imatte", config.imageBorder.ForStyleSheetShort(false));
	}
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

void FalconG::on_chkKeepDuplicates_toggled(bool yes)
{
	config.bKeepDuplicates = yes;
	_SetConfigChanged(true);
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
* REMARKS: on 'Gallery' page
*--------------------------------------------------------------------------*/
void FalconG::on_chkMenuToAbout_toggled(bool on)
{
	if (_busy)
		return;
	config.bMenuToAbout = on;
	_RunJavaScript("menu-item#about", QString("display") + (on ? "inline-block" : "none"));
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
	_RunJavaScript("menu-item#contact", QString("display:") + (on ? "inline-block" : "none"));
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
	_RunJavaScript("menu-item#desc", QString("display") + (on ? "inline-block" : "none"));
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
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkReadFromGallery_toggled(bool on)
{
	if (_busy)
		return;
	config.bReadFromGallery = on;
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
	config.bReadFromDirs = on;
	//	_SetConfigChanged(true);	no configuration change!

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

void FalconG::on_chkSeparateFoldersForLanguages_toggled(bool)
{
	config.bSeparateFoldersForLanguages = ui.chkSeparateFoldersForLanguages->isChecked();
}

void FalconG::on_chkSetAll_toggled(bool)
{
	_PageBackgroundToSample();
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
	_ElemToSample(espShadow);
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
	int val = ui.sbTextOpacity->value();
	pElem->color.SetOpacity(val, on, true);	// in percent
	_SetConfigChanged(true);
	_ElemToSample();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkSetLatest_toggled(bool on)
{

	if (_busy)
		return;
	config.bGenerateLatestUploads = on;
	_SetConfigChanged(true);
	_EnableButtons();
}

void FalconG::on_chkTdLinethrough_toggled(bool on) { _TextDecorationToConfig(tdLinethrough, on); }
void FalconG::on_chkTdUnderline_toggled(bool on) { _TextDecorationToConfig(tdUnderline, on); }
void FalconG::on_chkTdOverline_toggled(bool on) { _TextDecorationToConfig(tdOverline, on); }

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
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkUseWM_toggled(bool on)
{
	if (_busy)
		return;
	config.waterMark.used = on;
	_WaterMarkToSample();
	_SetConfigChanged(true);

	_EnableButtons();
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_chkUseWMShadow_toggled(bool on)
{
	if (_busy)
		return;
	config.waterMark.SetShadowOn(on);
	config.waterMark.SetupMark();
	_WaterMarkToSample();
	_SetConfigChanged(true);

	_EnableButtons();
}

// ======================== LineEdits

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
void FalconG::on_edtAlbumDir_textChanged()
{
	if (_busy)
		return;
	++_busy;
	config.dsAlbumDir = ui.edtAlbumDir->text().trimmed();
	_SetConfigChanged(true);
	--_busy;
}

void FalconG::on_edtBaseName_textChanged()
{
	if (_busy)
		return;

	config.dsGRoot = ui.edtGalleryRoot->text().trimmed();
	_SetConfigChanged(true);
}

/*========================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
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

	_SetConfigChanged(true);				// file is copied to /res
}

void FalconG::on_edtDefaultFonts_textChanged()
{
	if (_busy)
		return;
	config.sDefFonts = ui.edtDefaultFonts->text().trimmed();
	_SetConfigChanged(true);
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

void FalconG::on_edtDescription_textChanged()
{
	if (_busy)
		return;
	config.sDescription = ui.edtDescription->text().trimmed();
	_SetConfigChanged(true);
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

void FalconG::on_edtFontDir_textChanged()
{
	if (_busy)
		return;

	config.dsFontDir = ui.edtFontDir->text().trimmed();
	_SetConfigChanged(config.dsFontDir.Changed());
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
	_CElem* pElem;
	pElem = _PtrToElement();
	pElem->font.SetFamily(ui.edtFontFamily->text());
	_SetConfigChanged(true);
	_FontToSample(pElem);
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
void FalconG::on_edtImg_textChanged()
{
	if (_busy)
		return;

	config.dsImageDir = ui. edtImg->text().trimmed();
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
void FalconG::on_edtMainPage_textChanged()
{
	if (_busy)
		return;

	config.sMainPage = ui.edtMainPage->text().trimmed();
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
* TASK:	   Handle source gallery changes
* EXPECTS:
* GLOBALS:	PROGRAM_CONFIG
* REMARKS: Although the first .struct file will be saved into the source 
*			directory
*--------------------------------------------------------------------------*/
void FalconG::on_edtSourceGallery_textChanged()
{	
	if (_busy)
		return;

	QString src = QDir::cleanPath(ui.edtSourceGallery->text().trimmed()) + "/";
	if (!config.dsSrc.ToString().isEmpty() && config.dsSrc.ToString() != src)
	{
		config.dsSrc = src;
		PROGRAM_CONFIG::indexOfLastUsed = -1;	// suppose new path, not used yet
		for(int i = 0; i < PROGRAM_CONFIG::lastConfigs.size(); ++i )	// search if already exists
			if (src == PROGRAM_CONFIG::lastConfigs[i])
			{
				PROGRAM_CONFIG::indexOfLastUsed = i;	// found a config for this source
				break;							
			}
		QFile f(src + falconG_ini);
		if (f.exists())
		{
			config.Read();
			_ConfigToUI();	// edit values from config

		}
		else if(QFile::exists(src) )	// directory exists but no ini file inside yet
		{
			config.SetChanged(true);	// allow user to save config into this directory
			_EnableButtons();
		}
		Album* root = albumgen.AlbumForID(ROOT_ALBUM_ID);
		SeparateFileNamePath(config.dsSrc.ToString(), root->path, root->name);
	}
	albumgen.Clear();
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
void FalconG::on_edtUplink_textChanged()
{
	if (_busy)
		return;

	config.sUplink = ui. edtUplink->text().trimmed();
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
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_edtWatermark_textChanged()
{
	if (_busy)
		return;
	config.waterMark.SetText(ui.edtWatermark->text() );
	config.waterMark.SetupMark();
	if(config.waterMark.Text().isEmpty())
		ui.lblWmSample->setText("Watermark Sample text");
	else
		ui.lblWmSample->setText(config.waterMark.Text());
	_WaterMarkToSample();
	
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

// ========================  ???
void FalconG::on_hsImageSizeToShow_valueChanged(int val)
{
	if (_busy)
		return;
	config.backgroundImage.size = val;
	_RunJavaScript("body", config.backgroundImage.Size(false));
}

// ======================== Radio buttons

/*=============================================================
 * TASK:	the next 5 functions select the border of an element
 *			except images
 * EXPECTS:	on - if the radio button is activated
 * GLOBALS: config
 * RETURNS: none
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_rbAllBorders_toggled(bool on)
{
	if (!on || _busy)
		return;
	_SetupActualBorder(sdAll);
}

void FalconG::on_rbBottomBorder_toggled(bool on)
{
	if (!on || _busy)
		return;
	_SetupActualBorder(sdBottom);
}

void FalconG::on_rbCenterBckImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hAuto;
		_SetConfigChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}
void FalconG::on_rbCoverBckImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hCover;
		_SetConfigChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}
void FalconG::on_rbEnglish_toggled(bool b)
{
	if (_busy || !b)
		return;
	PROGRAM_CONFIG::lang = -1;	// English
	_RestartRequired();
}
void FalconG::on_rbLeftBorder_toggled(bool on)
{
	if (!on || _busy)
		return;
	_SetupActualBorder(sdLeft);
}
void FalconG::on_rbMagyar_toggled(bool b)
{
	if (_busy || !b)
		return;
	PROGRAM_CONFIG::lang = 1;	// Hungarian
	_RestartRequired();
}
void FalconG::on_rbNoBackgroundImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hNotUsed;
		_SetConfigChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}

void FalconG::on_rbRightBorder_toggled(bool on)
{
	if (!on || _busy)
		return;
	_SetupActualBorder(sdRight);
}

void FalconG::on_rbTdDashed_toggled(bool on) { _TextDecorationToConfig(tdDashed, on); }
void FalconG::on_rbTdDotted_toggled(bool on) { _TextDecorationToConfig(tdDotted, on); }
void FalconG::on_rbTdDouble_toggled(bool on) { _TextDecorationToConfig(tdDouble, on); }
void FalconG::on_rbTdSolid_toggled(bool on) { _TextDecorationToConfig(tdSolid, on); }
void FalconG::on_rbTdWavy_toggled(bool on) { _TextDecorationToConfig(tdWavy, on); }
void FalconG::on_rbTextAlignNone_toggled(bool on) { _TextAlignToConfig(alNone, on); }
void FalconG::on_rbTextCenter_toggled(bool on) { _TextAlignToConfig(alCenter, on); }
void FalconG::on_rbTextLeft_toggled(bool on) { _TextAlignToConfig(alLeft, on); }
void FalconG::on_rbTextRight_toggled(bool on) { _TextAlignToConfig(alRight, on); }

void FalconG::on_rbTextShadow_toggled(bool b)
{
	_CElem* pElem = _PtrToElement();
	int n = b ? 0 : 1;	// text or box shadow
	_ShadowForElementToUI(pElem, n);
	config.waterMark.SetupMark();
}
void FalconG::on_rbTileBckImage_toggled(bool b)
{
	if (!_busy && b)
	{
		config.backgroundImage.v = hTile;
		_SetConfigChanged(true);
		_RunJavaScript("body", config.backgroundImage.ForStyleSheet(false));
	}
}

void FalconG::on_rbTopBorder_toggled(bool on)
{
	if (!on || _busy)
		return;
	_SetupActualBorder(sdTop);
}

// ========================  Spin boxes

/*=============================================================
 * TASK:	changes radius of image border inside matte
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_sbAlbumMatteRadius_valueChanged(int w)
{
	if (_busy)
		return;
	config.albumMatteRadius = w;
	_SetConfigChanged(true);
	_RunJavaScript("athumb", QString("border-radius:%1px").arg(w) );
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbAlbumMatteWidth_valueChanged(int val)
{
	if (_busy)
		return;
	config.albumMatteWidth = val;
	_RunJavaScript("amatte", "background-color:" + config.albumMatteColor.Name());
	if(!val)
		_RunJavaScript("amatte", QString("padding:"));
	else
		_RunJavaScript("amatte", QString("padding: %1px").arg(val));
	_SetConfigChanged(true);
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
	_OpacityChanged(val, 2, ui.chkBackgroundOpacity->isChecked());
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

void FalconG::on_sbBorderWidth_valueChanged(int val)
{
	if (_busy)
		return;
	_CElem* pElem = _PtrToElement();
	pElem->border.SetWidth(pElem->border.actSide, val);
	_SetConfigChanged(true);
	_SetCssProperty(pElem, pElem->border.ForStyleSheet(false));
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
void FalconG::on_sbGradStopPos_valueChanged(int val)
{
	_CElem* pElem = _PtrToElement();
	
	pElem->gradient.Set(gsStop, val, pElem->gradient.Color(gsStop));
	_SetGradientLabelColors(pElem);
	_SetConfigChanged(true);
	_ElemToSample(espGradient);
}

/*=============================================================
 * TASK:	changes the border radius on the image and album matte!
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:	border is set on mattem not in image
 *------------------------------------------------------------*/
void FalconG::on_sbImageBorderRadius_valueChanged(int r)
{
	if (_busy)
		return;
	config.imageBorder.SetRadius(r);
	_SetConfigChanged(true);
	_RunJavaScript("imatte", QString("border-radius:%1px").arg(r));
	_RunJavaScript("amatte", QString("border-radius:%1px").arg(r));
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
	_RunJavaScript("imatte", qs);
	_RunJavaScript("amatte", qs);
	_SetConfigChanged(true);
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
	config.waterMark.GetMarkDimensions();
	config.waterMark.SetupMark();
	if (config.waterMark.v)
		_page.triggerAction(QWebEnginePage::Reload);

	if (w && val)
		_aspect = (double)w / (double)val;

	--_busy;
// ????	_ChangesToSample();
}

void FalconG::on_sbImageMargin_valueChanged(int m)
{
	if (_busy)
		return;
	config.imageMargin = m;
	_SetConfigChanged(true);
	_RunJavaScript("img-container", QString(m ? "margin: 0 %1;" : "margin:0;").arg(m));
}

/*=============================================================
 * TASK:	changes radius of image border inside matte
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::on_sbImageMatteRadius_valueChanged(int w)
{
	if (_busy)
		return;
	config.imageMatteRadius = w;
	_SetConfigChanged(true);
	_RunJavaScript("thumb", QString("border-radius:%1px").arg(w) );
}

/*========================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::on_sbImageMatteWidth_valueChanged(int val)
{
	if (_busy || config.imageMatteWidth == val )
		return;

	config.imageMatteWidth = val;
	_RunJavaScript("imatte", "background-color:" + config.imageMatteColor.Name());
	if (!val)
		_RunJavaScript("imatte",QString("padding:"));
	else	
		_RunJavaScript("imatte",QString("padding: %1px").arg(val));

	_SetConfigChanged(true);
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
	config.waterMark.GetMarkDimensions();
	config.waterMark.SetupMark();
	if (config.waterMark.v)
		_page.triggerAction(QWebEnginePage::Reload);
	
	if(h)	// else no change
		_aspect = (double)val / (double)h;
	--_busy;
// ????	_ChangesToSample(dp);
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
void FalconG::on_sbShadowBlur1_valueChanged(int val)
{
	int which = ui.rbTextShadow->isChecked() ? 0 : 1;
	_PtrToElement()->shadow1[which].Set(spBlurR, val);
	_ElemToSample(espShadow1);
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
	_ElemToSample(espShadow2);
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
	_ElemToSample(espShadow1);
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
	_ElemToSample(espShadow2);
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
	_ElemToSample(espShadow1);
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
	_ElemToSample(espShadow2);
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
	_ElemToSample(espShadow1);
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
	_ElemToSample(espShadow1);
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
	_OpacityChanged(val, 1, ui.chkTextOpacity->isChecked());
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

	config.thumbWidth = val;
	
	if (h)	// else no change
		_aspect = (double)val / (double)h;
	--_busy;
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

	config.thumbHeight = val;
	
	if (w && val)
		_aspect = (double)w / (double)val;

	--_busy;
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
	config.waterMark.SetOpacity(val, true);
	config.waterMark.SetupMark();
	_SetConfigChanged(true);
	_RunJavaScript("thumb::after","color"+ config.waterMark.ColorToCss());
	_WaterMarkToSample();
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowHoriz_valueChanged(int val)
{
	config.waterMark.SetShadowColor(val);
	config.waterMark.SetupMark();
	_SetConfigChanged(true);
	_ElemToSample(espGradient);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowVert_valueChanged(int val)
{
	config.waterMark.SetShadowColor(val);
	config.waterMark.SetupMark();
	_SetConfigChanged(true);
	_ElemToSample();
}

void FalconG::on_sldIconSize_valueChanged(int newSize)
{
	emit SignalThumbSizeChanged(newSize);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void FalconG::on_sbWmShadowBlur_valueChanged(int val)
{
	config.waterMark.SetShadowBlur(val);
	config.waterMark.SetupMark();
	_SetConfigChanged(true);
	_ElemToSample();
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
	cssCreator.Create(PROGRAM_CONFIG::samplePath+"css/falconG.css", true);	// program library setting cursors too
#if 0
	static QString name = PROGRAM_CONFIG::samplePath+"css/falconG.css",
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

	++_busy;						// semaphore
	if (who == wctSelection)
	{
		if (!_selection.newAlbum && !_selection.newImage)	   // selection cleared: show texts for album
		{
			ui.edtBaseTitle->clear();
			ui.edtBaseDescription->clear();
			ui.edtTitleText->clear();
			ui.edtDescriptionText->clear();

			_selection.actAlbum = _selection.selectedImage = 0;
			--_busy;
			return;
		}
		else
		{
			if (_selection.newAlbum)
			{
				Album& album = *albumgen.AlbumForID(_selection.newAlbum);
				_selection.title = albumgen.Texts()[album.titleID];
				_selection.description = albumgen.Texts()[album.descID];
			}
			else
			{
				Image& image = *albumgen.ImageAt(_selection.newImage);
				_selection.title = albumgen.Texts()[image.titleID];
				_selection.description = albumgen.Texts()[image.descID];
			}
		}
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
 * TASK:	Add all scheme color selection buttons to 
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
	for (int i = 2; i < schemes.size(); ++i)				// do not add default and system!
		ui.lwColorScheme->addItem(schemes[i].MenuTitleForLanguage(PROGRAM_CONFIG::lang));
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
	plabel = new QLabel(tr("Drag & Drop Insert Marker"));
	plabel->setFont(font);
	pButton = new QPushButton("");
	_pSchemeButtons.push_back(pButton);
	pLayout->addRow(plabel, pButton);
	_pSchemeMapper->setMapping(_pSchemeButtons[i], i);
	connect(pButton, SIGNAL(clicked()), _pSchemeMapper, SLOT(map()));

	connect(_pSchemeMapper, &QSignalMapper::mappedInt, this, &FalconG::_SlotForSchemeButtonClick);

	ui.lwColorScheme->setCurrentRow(0);
}


/*========================================================
 * TASK:	slot for link clicked in sample (WebEnginePage)
 * PARAMS:	s -	link name a decimal number string
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void FalconG::_LinkClicked(QString s)
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
void FalconG::_WebPageLoaded(bool ready)
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
 * GLOBALS:	_selection: FalconGEditTabSelection
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_AlbumStructureSelectionChanged(const QItemSelection &current, const QItemSelection &previous)
{
	_SaveChangedTexts();

	ui.tnvImages->Clear();

	int n = current.indexes().size();
	if (n == 1)		 // single selection and valid
	{
		// collect its images into list
//		ui.tnvImages->thumbsDir = (config.dsGallery + config.dsGRoot + config.dsImageDir).ToString();
		_currentTreeViewIndex = current.indexes()[0];
		ID_t id = ID_t(_currentTreeViewIndex.internalPointer());	// store ID of last selected album

		ui.tnvImages->Setup(id);
		_selection.newAlbum = id;	// only single selection is used
								    // before the image list is read
	}
	else
		_selection.newAlbum = 0;	// nothing/multiple albums selected


	ui.tnvImages->Load();
	ui.tnvImages->clearSelection();	// no selection
	_selection.selectedImage = _selection.newImage = 0;

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
	if (QMessageBox::warning(this, tr("falconG - Color scheme changed"), tr("Changes were not applied.\nDo you want to apply changes?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
		on_btnApplyColorScheme_clicked();
	--_busy;
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
	ui.btnGradMiddleColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[1].color));
	ui.btnGradStartColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[0].color));
	ui.btnGradStopColor->setStyleSheet(__ToolButtonBckStyleSheet(pElem->gradient.gs[2].color));
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
//void FalconG::_SetOpacityToConfig(_CElem & elem, int which)
//{
//	int val;
//	if (which & 2)
//	{
//		val = (ui.chkBackgroundOpacity->isChecked() ? 1 : -1)*ui.sbBackgroundOpacity->value(); // in %
//		elem.background.SetOpacity(val, true, true);
//	}
//	if((which & 1) == 0 )
//	{
//		val = (ui.chkTextOpacity->isChecked() ? 1 : -1)*ui.sbTextOpacity->value(); // in %
//		elem.color.SetOpacity(val, true, true);
//	}
//}

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
//	bool b = (pElem == &config.Web || (pElem->parent && pElem->color != pElem->parent->color)); // else clear color
//	_SetCssProperty(pElem,b ? pElem->color.ForStyleSheet(false, false) : QString("color:") );
	_SetCssProperty(pElem, pElem->color.ForStyleSheet(false, false));
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
	QString qs;
	qs = pElem->background.ForStyleSheet(false, true);
	_SetCssProperty(pElem, qs);
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

void FalconG::_PageBackgroundToSample()
{
	QString wbc = config.Web.background.Name();

	auto setSameBackground = [&](AlbumElement what)
	{		   // opacity does not change
		_CElem* pe = _PtrToElement(what);
		pe->background.Set(wbc, -1);
		_SetCssProperty(pe, pe->background.ForStyleSheet(false,true) );
	};

	if (ui.chkSetAll->isChecked())		// then remove all other background colors
	{
		setSameBackground(aeHeader);
		setSameBackground(aeMenuButtons);
		setSameBackground(aeLangButton);
		setSameBackground(aeSmallTitle);
		setSameBackground(aeGalleryTitle);
		setSameBackground(aeGalleryDesc);
		setSameBackground(aeSection);
		setSameBackground(aeImageTitle);
		setSameBackground(aeImageDesc);
		setSameBackground(aeLightboxTitle);
		setSameBackground(aeLightboxDescription);
		setSameBackground(aeFooter);
	}
	QString qs = config.Web.background.ForStyleSheet(false, true);
	_RunJavaScript("body", qs);
}

void FalconG::_SetIcon()		// only for 'menu-item#uplink' and the icon always has the same name
{								// but its color will change. Will it be visible without setting it again? And this way? TODO Check???
	_RunJavaScript("menu-item#uplink",QString("background-image:")+"url('file:///res/up-icon-sample.png')");
}


/*========================================================
 * TASK:	set watermark to sample page or remove it from there
 * PARAMS:
 * GLOBALS:	config.waterMark image is created
 * RETURNS:
 * REMARKS: - only used for falconG and not in production
 *-------------------------------------------------------*/
void FalconG::_WaterMarkToSample()
{
			// set text
	_CWaterMark &wm = config.waterMark;
	if (wm.used)
	{
		_RunJavaScript("athumb::after", "display:"); // remove "none"
		_RunJavaScript("athumb::after", "display:"); // remove "none"
	}
	else
	{
		_RunJavaScript("athumb::after", "display:none"); // remove "none"
		_RunJavaScript("athumb::after", "display:none"); // remove "none"
	}
	_page.triggerAction(QWebEnginePage::Reload);
}


/*============================================================================
  * TASK:	store changed opacity for text or background
  * EXPECTS: val - value of new opacity in percent
  *			 which: 1 -> text, 2: background	
  * GLOBALS:
  * REMARKS: 
 *--------------------------------------------------------------------------*/
void FalconG::_OpacityChanged(int val, int which, bool used)
{
	_CElem* pElem = _PtrToElement();
	if (which == 1)
		pElem->color.SetOpacity(val, used, true);
	else
		pElem->background.SetOpacity(val, used, true);
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

	albumgen.AlbumForID(_selection.actAlbum)->changed = true;
	_selection.changed = fsNothing;		// all changes saved

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
		layoutTop = 20;
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

void FalconG::_SetProgramScheme()
{	 
	int which = PROGRAM_CONFIG::schemeIndex;
	if (which >= schemes.size())
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
	background-color:%15;	   /* %15 progressbar chunk */
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
						/* these 2 do not work */
QTreeView::branch:open:has-children {
	color %2;

}
QTreeView::branch:closed:has-children {
	color:%2;
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
    border: 2px solid %3;	  /* %3   border color */
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QGroupBox::title {
	color:%17				/* %17 bold title color */
}

QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%4;				/* %4 focused input */
}

#menuWidget {
	background-color:rgb(0,0,0,0);
}
QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;	/* %5 - hover */
}
QTabBar::tab:selected {
	border-color:%6;		/* %6 - tab border */
    border-bottom-color:%1; /* %1 - background */
}

#pnlColorScheme {
	background-color:%1; /* %1 - background */
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%7;	/* %7 - input background */
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%8;	/* %8 */
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;		/* %9 - focused border */
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled,
QRadioVutton:disabled,
QCheckBox:disabled,
QSpinBox:disabled,
QMenu:disabled {
	color:%10;			/* %10 - disabled foreground */
	background:%11;		/* %11 - disabled background */
}

QLineEdit.disabled {
	color:%10;
	background:%11;	   /* %11 - disabled background */
}

#lblBckImage,
#groupBox_6 {
	background-color:%12; /* %12 -image background */
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%13; /* %13 - pressed button background */
}

QPusButton:default {
    background-color:%14; /* %14 - default background */
}

#lblActualElem {
	color:%16;			/* %16 - warning color */
}
#btnDeleteColorScheme {
	background-color:%9;	/* %9 focused border */
}
)END"
	};
	PROGRAM_CONFIG::schemeIndex = which;
	if (which)
	{
		QString ss =
			QString(styles)
			.arg(schemes[which].sBackground)		// %1 
			.arg(schemes[which].sTextColor)			// %2 
			.arg(schemes[which].sBorderColor)		// %3 
			.arg(schemes[which].sFocusedInput)		// %4 
			.arg(schemes[which].sHoverColor)		// %5 
			.arg(schemes[which].sTabBorder)			// %6 
			.arg(schemes[which].sInputBackground)	// %7
			.arg(schemes[which].sSelectedInputBgr)	// %8 
			.arg(schemes[which].sFocusedBorder)		// %9 
			.arg(schemes[which].sDisabledFg)		// %10
			.arg(schemes[which].sDisabledBg)		// %11
			.arg(schemes[which].sImageBackground)	// %12
			.arg(schemes[which].sPressedBg)			// %13
			.arg(schemes[which].sDefaultBg)			// %14
			.arg(schemes[which].sProgressBarChunk)	// %15
			.arg(schemes[which].sWarningColor)		// %16
			.arg(schemes[which].sBoldTitleColor)	// %17
//			.arg(schemes[which].sSpacerColor)		// %18
			;

		if (which == stBlue)		// blue
				ss += QString( R"(QCheckBox::indicator:checked {
		image: url(":/icons/Resources/blue-checked.png");
	}

QCheckBox::indicator:unchecked {
	image: url(":/icons/Resources/blue-unchecked.png");
}
)");
		// setStyleSheet(ss);	 // sometimes this does not cascade down to dialogs
		// use this instead:
		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
	else
		((QApplication*)(QApplication::instance()))->setStyleSheet("");

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
  * TASK:		saves the style sheet into both the gallery and the sample page
  * EXPECTS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FalconG::on_btnSaveStyleSheet_clicked()
{
	CssCreator cssCreator;
	cssCreator.Create(PROGRAM_CONFIG::samplePath + "css/falconG.css", true);	// program library
	config.SaveDesign();

	_SetConfigChanged(true);
	if (albumgen.SaveStyleSheets() != 0 || (config.doNotShowTheseDialogs & (int)dbShowAfterSavingCss) )
	{	
		QString s = (config.dsGallery + config.dsGRoot + config.dsCssDir).ToString();
		InformationMessage( false, tr("falconG"), 
							QString(tr("Saved style sheet 'falconG.css'\ninto %1").arg(s)),
							dbShowAfterSavingCss, tr("Don't show again (use Options to re-enable)"),
							this
						  );
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
	QFont font = QFontDialog::getFont(&ok, config.waterMark.Font(), this, "falconG - select Watermark font");
	if (ok && config.waterMark.Font() != font)
	{
		config.waterMark.SetFont(font);
		
		ui.lblWaterMarkFont->setText( QString("%1 (%2,%3)").arg(font.family()).arg(font.pointSize()).arg(font.italic()));
		if(!config.waterMark.Text().isEmpty())
			ui.lblWmSample->setText(config.waterMark.Text());
		else
			ui.lblWmSample->setText(tr("Watermark sample text"));

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

	QColor  qc = config.waterMark.Color() ,
			qcNew;
	qcNew = QColorDialog::getColor(qc, this, tr("Select Watermark Color"));

	if (qcNew.isValid() && qcNew != qc)
	{
		QString s = qcNew.name();
		handler.SetItem("QToolButton", "background-color", s);
		ui.btnWmColor->setStyleSheet(handler.StyleSheet());
		s = s.mid(1);	 // skip '#'
		config.waterMark.SetColorWithOpacity(s);
		_SetConfigChanged(config.waterMark.v = true);
		_RunJavaScript("thumb::after", "color: " + config.waterMark.ColorToCss());
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
		config.waterMark.SetShadowColor( qc.name().mid(1).toInt(nullptr, 16) );
		
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
	ui.cbBaseAboutLanguage->clear();
	ui.cbLanguage->clear();
	ui.cbTranslatedAboutLanguage->clear();

	for (int i = 0; i < languages.LanguageCount(); ++i)
	{			 // edit texts page
		ui.cbBaseLanguage->addItem((*languages["name"])[i] + " (" + (*languages["countryCode"])[i] + ")");
		ui.cbLanguage->addItem((*languages["name"])[i] + " (" + (*languages["countryCode"])[i] + ")");
		ui.cbBaseAboutLanguage->addItem((*languages["name"])[i] + " (" + (*languages["countryCode"])[i] + ")");
		ui.cbTranslatedAboutLanguage->addItem((*languages["name"])[i] + " (" + (*languages["countryCode"])[i] + ")");
	}
	++_busy;
	if (languages.LanguageCount() > 1)
	{
		ui.cbBaseLanguage->setCurrentIndex(0);
		ui.cbLanguage->setCurrentIndex(1);
		ui.cbBaseAboutLanguage->setCurrentIndex(0);
		ui.cbTranslatedAboutLanguage->setCurrentIndex(1);
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
		ui.lblProgressTitle->setText(tr("Phase 1: reading albums and images"));
		ui.lblProgressDesc->setText(tr("albums / images"));
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
		ui.lblProgressTitle->setText(tr("Phase 2: processing images"));
		ui.lblProgressDesc->setText(tr("images / total"));
	}
	else
	{
		ui.lblProgressTitle->setText(tr("Phase 3: Creating albums"));
		ui.lblProgressDesc->setText(tr("albums / total"));
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
	ui.lblProgress->setText(QString("%1 / %2").arg(count).arg(maxCount));
	ui.progressBar->setValue(count);

	static int cnt = 0;
	if (++cnt == 10)
	{
		QApplication::processEvents();
		cnt = 0;
	}
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
	QString src = PROGRAM_CONFIG::samplePath+"res/"+destName;
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
	if(yes)
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	else
		QGuiApplication::restoreOverrideCursor();
	QApplication::processEvents();
}

void FalconG::_TnvCountChanged()
{
	ui.lblTotalCount->setText(QString("%1 Albums, %2 images").arg(albumgen.Albums().size()).arg(albumgen.Images().size()) );
}

void FalconG::_TnvStatusChanged(QString &s)
{
	ui.lblImageCount->setText(s);
}

/*=============================================================
 * TASK:   slot to receive ID of selected thumbnal (in image dir)
 * EXPECTS:	 id: id of item for single selection: 
 *				ID of parent album if no selection
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_TnvSelectionChanged(ID_t id)
{
	if (id & IMAGE_ID_FLAG)
		_selection.newImage = id;
	else if(id)
		_selection.newAlbum = id;
	_GetTextsForEditing(wctSelection);
}

/*=============================================================
 * TASK:	
 * PARAMS:	list - list of selected items' IDs
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconG::_TnvMultipleSelection(IdList list)
{
//	int n = list.size();
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
 *			styles - string of css styles separated by '\n'
 * GLOBALS:
 * RETURNS:
 * REMARKS: - 
 *-------------------------------------------------------*/
void FalconG::_RunJavaScript(QString className, QString styles)
{
	if (!_isWebPageLoaded)		// only try to run after page looaded
		return;

	styles = styles.trimmed();

	static QStringList __qslRunThese;
	__qslRunThese = __qslRunThese + styles.split('\n');

	int pos;
	QString qs;	 

	auto runOneTag = [&](QString &s)
	{
		pos = s.indexOf(':');
		if (pos <= 0) 
			return;
		qs = QString("SetPropertyForSelector('" + className + "','" + s.left(pos) + "','" + s.mid(pos+1) + "')");
		_page.runJavaScript(qs);
//		DEBUG_LOG(qs)
	};
	for (auto s : __qslRunThese)
		runOneTag(s);

	__qslRunThese.clear();
}

void FalconG::_SetCssProperty(_CElem*pElem, QString value, QString subSelector)
{
	QString className = pElem->ClassName();
	if (!subSelector.isEmpty())
		className += subSelector;
	_RunJavaScript(className, value);
}

/*=============================================================
 * TASK:   slot for signal when album added/removed changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:- sets _edited to mark that unprocessed album 
 *				structure must be saved at exit
 *------------------------------------------------------------*/
void FalconG::_AlbumMapChanged()
{
	_edited = true;	// so that we save it at program end if not processed
	reinterpret_cast<AlbumTreeModel*>(ui.trvAlbums->model())->ModelChanged();
	ui.trvAlbums->expandToDepth(1);
	// ui.trvAlbums->expandAll();
}

/*=============================================================
 * TASK:   slot for signal when any album changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:- sets _edited to mark that unprocessed album 
 *				structure must be saved at exit
 *------------------------------------------------------------*/
void FalconG::_AlbumChanged()
{
	_edited = true;
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
