#include <QString>
#include <QColor>

#include "common.h"
#include "csscreator.h"	 // includes config.h
//#include "config.h"

#include "slidewidget.h"
#include "stylehandler.h"
#include "designpage.h"

struct WidgetPointers
{
	QWidget* p = nullptr;
	void (DesignPage::*f)() = nullptr;
};
static WidgetPointers __wptr[SAMPLEINDEXSIZE];		// current # of widgets, index 0: scrollAreaWidgetContents

DesignPage::DesignPage(QWidget* parent)
{
	ui.setupUi(this);

	__wptr[nGlobal]					= {ui.scrollAreaWidgetContents	,&DesignPage::_SetGlobalStyle		};

	__wptr[nBtnDnUp]				= {ui.btnDnUp					,&DesignPage::_SetUplinkButtonIcon	};

	__wptr[nBtnDnAbout]				= {ui.btnDnAbout				,&DesignPage::_SetMenuButtonStyle	};
	__wptr[nBtnDnContact]			= {ui.btnDnContact				,&DesignPage::_SetMenuButtonStyle	};
	__wptr[nBtnDnCaptDesc]			= {ui.btnDnCaptDesc				,&DesignPage::_SetMenuButtonStyle	};
	__wptr[nBtnDnDescription]		= {ui.btnDnDescription			,&DesignPage::_SetMenuButtonStyle	};
	__wptr[nBtnDnToAlbums]			= {ui.btnDnToAlbums				,&DesignPage::_SetMenuButtonStyle	};

	__wptr[nLblDnName]				= {ui.lblDnName					,&DesignPage::_SetSmallTitleStyle	};
	__wptr[nLblDnLang]				= {ui.lblDnLang					,&DesignPage::_SetLangButtonStyle	};
	__wptr[nLblDnGallery]			= {ui.lblDnGallery				,&DesignPage::_SetLblDnGalleryStyle	};
	__wptr[nLblDnGalleryDescription]= {ui.lblDnGalleryDescription	,&DesignPage::_SetLblDnGalleryDescriptionStyle	};
	__wptr[nLblDnPictureSectionLabel]= {ui.lblDnPictureSectionLabel	,&DesignPage::_SetLblDnPictureSectionLabelStyle	};

	__wptr[nLblDnImageP]			= {ui.pnlDnSlideP				,&DesignPage::_SetLblDnImagePStyle };
	__wptr[nLblDnDescriptionP]		= {ui.lblDnDescriptionP			,&DesignPage::_SetLblDnDescriptionPStyle };
	__wptr[nLblDnTitleP]			= {ui.lblDnTitleP		 		,&DesignPage::_SetLblDnTitlePStyle	};
	__wptr[nLblDnTitlePO]			= {ui.lblDnTitlePO				,&DesignPage::_SetLblDnTitlePOStyle	};

	__wptr[nLblDnAlbumSectionLabel] = {ui.lblDnAlbumSectionLabel	,&DesignPage::_SetLblDnAlbumSectionLabelStyle	};
	__wptr[nLblDnImageA]			= {ui.pnlDnSlideA				,&DesignPage::_SetLblDnImageAStyle };
	__wptr[nLblDnDescriptionA]		= {ui.lblDnDescriptionA			,&DesignPage::_SetLblDnDescriptionAStyle };
	__wptr[nLblDnTitleA]			= {ui.lblDnTitleA				,&DesignPage::_SetLblDnTitleAStyle	};
	__wptr[nLblDnTitleAO]			= {ui.lblDnTitleAO		 		,&DesignPage::_SetLblDnTitleAOStyle	};

	__wptr[nLblDnFooter]			= {ui.lblDnSummary				,&DesignPage::_SetLblDnFooterStyle	};
	__wptr[nLblDnCopyright]			= {ui.lblDnCopyright	 		,&DesignPage::_SetLblDnFooterStyle };

	__wptr[nLblDnWatermark]			= {ui.pnlDnSlideA				,&DesignPage::_SetLblDnImagePStyle };

	ui.pnlDnSlideP->SetImage(QImage(":/Preview/Resources/placeholder.jpg"));
	ui.pnlDnSlideA->SetImage(QImage(":/Preview/Resources/placeholder2.jpg"));

	ui.pnlDnSlideP->SetMouseClickCallback([&](SlideWidget::SlideArea area, QMouseEvent* e) {emit SignalClicked(nLblDnImageP);});
	ui.pnlDnSlideA->SetMouseClickCallback([&](SlideWidget::SlideArea area, QMouseEvent* e) {emit SignalClicked(nLblDnImageA);});
}

/*=============================================================
 * TASK   :
 * PARAMS :
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void DesignPage::mousePressEvent(QMouseEvent* event)
{
	_MyMousePressEvent(event);
}

void DesignPage::_MyMousePressEvent(QMouseEvent* event)
{
	emit SignalClicked(nGlobal);
}

/*=============================================================
 * TASK   :	sets up the style for the items in
 * PARAMS :
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void DesignPage::SlotItemStyleChanged(int indexFrmCmbBx)
{
	if (indexFrmCmbBx < UNIQUESIZE)  // -1: all items
	{
		int ix[UNIQUESIZE] = {};
		int cnt = scMap.CBIxToSample(indexFrmCmbBx, ix);	// get items with same style into ix
		for (int i = 0; i < cnt; i++)				// and set style of each of them
			if (__wptr[ix[i]].f != nullptr)			// nullptr for nLblDnheader nLblDnLightBox and nLblDnLightboxTitle
				(this->*(__wptr[ix[i]].f))();
	}
}
void DesignPage::_SetGlobalStyle()		// from global page: color, background color and background image
{
	QString qs1 = config.backgroundImage.ForStyleSheet(forInternalUse, forBackground, itemNameToo),
			qs2 = config.Web.background.ForStyleSheet(addSemicolon, forBackground, itemNameToo);	
	QString qs = //"#scrollAreaWidgetContents \n{\n" + 
				 qs1 + qs2 +
				 config.Web.color.ForStyleSheet(addSemicolon, notForBackground, itemNameToo) + "\n"; // "}\n";
	ui.scrollAreaWidgetContents->setStyleSheet(qs);
	// set background to separate menu panel to the global style
	StyleHandler styleHandler(ui.pnlMenu->styleSheet());
	styleHandler.RemoveItem(QString(), "background");
	styleHandler.RemoveItem(QString(), "background-color");
	styleHandler.SetItem(QString(), "background", qs1);
	styleHandler.SetItem(QString(), "background-color", qs2);
	ui.pnlMenu->setStyleSheet(qs);
}

void DesignPage::_SetUplinkButtonIcon()		// UP menu button with icon
{
	const QIcon &icon = config.SetUplinkIcon(QString());		// empty name: default up-icon.png in resources
	ui.btnDnUp->setIcon(icon);
}
void DesignPage::_SetMenuButtonStyle()		
{
	const QString buttons[] = { "#btnDnUp", "#btnDnAbout", "#btnDnContact", "#btnDnCaptDesc", "#btnDnDescription", "#btnDnToAlbums" };
	QString qss = config.Menu.ForStyleSheet(addSemicolon);

	auto setStyle = [&](int i) -> QString
		{
			return buttons[i] + " {\n" + qss + "\n}\n" + buttons[i] + ":hover {\n background-color:gray;\n}";
		};
	
	ui.btnDnAbout->setStyleSheet(setStyle(1));
	ui.btnDnContact->setStyleSheet(setStyle(2));
	ui.btnDnCaptDesc->setStyleSheet(setStyle(3));
	ui.btnDnDescription->setStyleSheet(setStyle(4));
	ui.btnDnToAlbums->setStyleSheet(setStyle(5));
	// separate as paddigng must be changed
	qss = qss.replace("padding:4px 10px 3px", "padding:6px");
	ui.btnDnUp->setStyleSheet(setStyle(0));
}
void DesignPage::_SetSmallTitleStyle()
{
	// small titles: name and language
	QString style = config.SmallGalleryTitle.ForStyleSheet(addSemicolon);
	_ApplyStyleToQtLabel(ui.lblDnName, style);
}
void DesignPage::_SetLangButtonStyle()
{
	QString style = "#lblDnLang\n{\n " +
					config.Lang.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnLang,style);
}
void DesignPage::_SetLblDnGalleryStyle()
{
	QString style = "#lblDnGallery {\n " +
				config.GalleryTitle.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnGallery,style);
}
void DesignPage::_SetLblDnGalleryDescriptionStyle()
{
	QString style = "#lblDnGalleryDescription {\n " +
		config.GalleryDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnGalleryDescription,style);
}
void DesignPage::_SetLblDnPictureSectionLabelStyle()
{
	QString style = "#lblDnPictureSectionLabel {\n " +
		config.Section.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnPictureSectionLabel,style);
	// and set here for the album too
	_SetLblDnAlbumSectionLabelStyle();
}
void DesignPage::_SetLblDnImagePStyle()		// image thumbnail 
{											// no different values of sides
	QString style = CssCreator::StyleSheetForSampleImageOrAlbum(true, ".imatte", true);
	// style may contain non-css rules, like 'matte-radius:'
	StyleHandler styHandler(style);
	ui.pnlDnSlideA->SetMatteColor(QColor(styHandler.GetItem(".imatte", "background-color")));
	ui.pnlDnSlideA->SetMatteRadius(styHandler.GetItem(".imatte", "matte-radius"));
	ui.pnlDnSlideA->SetAllBordersFromString(styHandler.GetItem(".imatte", "border"));  // <width> <style> <color>
	ui.pnlDnSlideA->SetMatteRadius(styHandler.GetItem("", "border", "matte-radius"));


	_SetLblDnImageAStyle();					// same values for albums
}
void DesignPage::_SetLblDnDescriptionPStyle()
{
	QString style = "#lblDnDescriptionP {\n " +
		config.ImageDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnDescriptionP,style);
	// and set here for the album too
	_SetLblDnDescriptionAStyle();
}
void DesignPage::_SetLblDnTitlePStyle()			// image + album title first line
{	
	QString style = "#lblDnTitleP {\n " +
		config.ImageTitle.ForStyleSheet(config.ImageTitle.font.IsFirstLineDifferent(), true) + "}\n";
	_ApplyStyleToQtLabel(ui.lblDnTitleP, style );
	// and set here for the album too
	_SetLblDnTitlePOStyle();		// image + album title other lines
	_SetLblDnTitleAStyle();			// album title first line
}
void DesignPage::_SetLblDnTitlePOStyle()		// when first line differs then image title all other lines
{								// image title first line
	QString style = "#lblDnTitlePO {\n " +
		config.ImageTitle.ForStyleSheet(notFirstOne, addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnTitlePO, style);
	// and set here for the album too
	_SetLblDnTitleAOStyle();	// album title first line		
}
void DesignPage::_SetLblDnAlbumSectionLabelStyle()	// label styles are the same for albums and pictures
{
	QString style = "#lblDnAlbumSectionLabel {\n " +
		config.Section.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnAlbumSectionLabel,style);
}
void DesignPage::_SetLblDnImageAStyle()			// album thumbnail image
{

}
void DesignPage::_SetLblDnDescriptionAStyle()
{
	QString style = "#lblDnDescriptionA {\n " +
		config.ImageDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnDescriptionA,style);
}
void DesignPage::_SetLblDnTitleAStyle()
{
	QString style = "#lblDnTitleA {\n " +
		config.ImageTitle.ForStyleSheet(config.ImageTitle.font.IsFirstLineDifferent(), true) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnTitleA, style );
	_SetLblDnTitleAOStyle();
}
void DesignPage::_SetLblDnTitleAOStyle()
{
	QString style = "#lblDnTitleAO {\n " +
		config.ImageTitle.ForStyleSheet(notFirstOne, addSemicolon) + "\n}\n";
	_ApplyStyleToQtLabel(ui.lblDnTitleAO, style);
}
void DesignPage::_SetLblDnFooterStyle()
{
	_ApplyStyleToQtLabel(ui.lblDnSummary,"#lblDnSummary {\n " +
		config.Footer.ForStyleSheet(addSemicolon) + "\n}\n");
	_SetLblDnCopyrightStyle();	// for now
}
void DesignPage::_SetLblDnCopyrightStyle()
{
	_ApplyStyleToQtLabel(ui.lblDnCopyright,"#lblDnCopyright {\n " +
		config.Footer.ForStyleSheet(addSemicolon) + "\n}\n");
}
void DesignPage::_SetLblDnHeaderStyle()
{
	; // ???
}
void DesignPage::_SetLblLightboxStyle()
{
	; // ???
}
void DesignPage::_SetLblLightboxTitleStyle()
{
	; // ???
}

static Qt::Alignment __GetAlignment(const QString& str)
{
	Qt::Alignment a;
	int ix = str.indexOf("text-align:");
	if (ix > 0)
	{
		QStringRef ref(& str, ix + 11, str.length() - ix - 11);
		if (ref.left(4) == "left")
			a.setFlag(Qt::AlignLeft);
		else if (ref.left(6) == "center")
			a .setFlag(Qt::AlignHCenter);
		else if (ref.left(5) == "right")
			a .setFlag(Qt::AlignRight);
	}
	return a;
}
static void __ChangeFont(QFont& font, const QString& str)
{
	StyleHandler sh(str);
	QString qs = sh.GetItem(QString(), "font-family");
	font.setFamily(qs);
	qs = sh.GetItem(QString(), "font-size");
	font.setPointSize(qs.left(qs.length()-2).toInt());
	qs = sh.GetItem(QString(), "font-style");
	font.setItalic(false);
	font.setBold(false);
	if (!qs.isEmpty())
	{
		font.setItalic(qs.indexOf("italic") >= 0);
		font.setBold(qs.indexOf("bold") >= 0);
	}
	qs = sh.GetItem(QString(), "font-weight");	// may override 'Bold'
	if (qs.isEmpty())
		font.setWeight(QFont::Normal);
	else
		font.setWeight(QFont::Bold);		// = 75 would be 900 for CSS
}

void DesignPage::_ApplyStyleToQtLabel(ClickableLabel *toLabel, const QString &thisStyleString)
{
	Qt::Alignment a = __GetAlignment(thisStyleString);
	if(a)
		toLabel->setAlignment(a);
	QFont font = toLabel->font();
	__ChangeFont(font, thisStyleString);
	toLabel->setFont(font);

	toLabel->setStyleSheet(thisStyleString);
}

														 
void DesignPage::on_btnDnUp_clicked()					  { qDebug("nBtnDnUp");					emit SignalClicked(nBtnDnUp							);}
void DesignPage::on_btnDnAbout_clicked()				  { qDebug("nBtnDnAbout");				emit SignalClicked(nBtnDnAbout					);}
void DesignPage::on_btnDnContact_clicked()				  { qDebug("nBtnDnContact");			emit SignalClicked(nBtnDnContact				);}
void DesignPage::on_btnDnCaptDesc_clicked()				  { qDebug("nBtnDnCaptDesc");			emit SignalClicked(nBtnDnCaptDesc				);}
void DesignPage::on_btnDnDescription_clicked() 			  { qDebug("nBtnDnDescription");		emit SignalClicked(nBtnDnDescription			);}
void DesignPage::on_btnDnToAlbums_clicked() 			  { qDebug("nBtnDnToAlbums");			emit SignalClicked(nBtnDnToAlbums				);}

void DesignPage::on_lblDnName_clicked() 				  { qDebug("nLblDnName");				emit SignalClicked(nLblDnName					);}
void DesignPage::on_lblDnLang_clicked() 				  { qDebug("nLblDnLang");				emit SignalClicked(nLblDnLang					);}

void DesignPage::on_lblDnGallery_clicked() 				  { qDebug("nLblDnGallery");			emit SignalClicked(nLblDnGallery				);}
void DesignPage::on_lblDnGalleryDescription_clicked() 	  { qDebug("nLblDnGalleryDescription"); emit SignalClicked(nLblDnGalleryDescription		);}

void DesignPage::on_lblDnPictureSectionLabel_clicked()	  { qDebug("nLblDnPictureSectionLabel"); emit SignalClicked(nLblDnPictureSectionLabel	);}
void DesignPage::on_lblDnDescriptionP_clicked() 		  { qDebug("nLblDnDescriptionP");		emit SignalClicked(nLblDnDescriptionP			);}
void DesignPage::on_lblDnTitleP_clicked() 				  { qDebug("nLblDnTitleP");				emit SignalClicked(nLblDnTitleP		 			);}
void DesignPage::on_lblDnTitlePO_clicked() 				  { qDebug("nLblDnTitlePO");			emit SignalClicked(nLblDnTitlePO				);}
															
void DesignPage::on_lblDnAlbumSectionLabel_clicked()	  { qDebug("nLblDnAlbumSectionLabel"); emit SignalClicked(nLblDnAlbumSectionLabel       );}
void DesignPage::on_lblDnDescriptionA_clicked() 		  { qDebug("nLblDnDescriptionA");		emit SignalClicked(nLblDnDescriptionA			);}
void DesignPage::on_lblDnTitleA_clicked() 				  { qDebug("nLblDnTitleA");				emit SignalClicked(nLblDnTitleA		 			);}
void DesignPage::on_lblDnTitleAO_clicked() 				  { qDebug("nLblDnTitleAO");			emit SignalClicked(nLblDnTitleAO	 			);}

void DesignPage::on_lblDnSummary_clicked() 				  { qDebug("nLblDnSummary");			emit SignalClicked(nLblDnFooter					);}
void DesignPage::on_lblDnCopyright_clicked() 			  { qDebug("nLblDnCopyright");			emit SignalClicked(nLblDnFooter		 			);}
