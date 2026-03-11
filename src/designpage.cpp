
#include <QString>
#include <QColor>

#include "common.h"
#include "config.h"

#include "slidewidget.h"
#include "stylehandler.h"
#include "designpage.h"

struct WidgetPointers
{
	QWidget* p = nullptr;
	void (DesignPage::*f)() = nullptr;
};
static WidgetPointers __wptr[cboxSize];		// current # of widgets, index 0: scrollAreaWidgetContents

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

	__wptr[nLblDnSummary]			= {ui.lblDnSummary				,&DesignPage::_SetLblDnSummaryStyle	};
	__wptr[nLblDnCopyright]			= {ui.lblDnCopyright	 		,&DesignPage::_SetLblDnCopyrightStyle };

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
	if (indexFrmCmbBx >= cboxSize)
		return;

	auto applyStyle = [&]()
		{
			int ix[cboxSize] = {};
			int cnt = CBToDesign(indexFrmCmbBx, ix, cboxSize);	// get items with same style into ix
			for (int i = 0; i < cnt; i++)					// and set style of each of them
				if(__wptr[ix[i]].f != nullptr)					// nullptr for nLblDnheader nLblDnLightBox and nLblDnLightboxTitle
					(this->*(__wptr[ix[i]].f))();
		};

	if (indexFrmCmbBx < 0)		  // then apply all styles from config
	{
		for(indexFrmCmbBx = 0; indexFrmCmbBx < actualItemsSize; ++indexFrmCmbBx)	 // 'actualItemsSize' see common.h
			applyStyle();
	}
	else 
		applyStyle();

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
/*	ui.pnlAlbumSectionHeader->setStyleSheet(qs);
	ui.pnlDnAlbum->setStyleSheet(qs);
	ui.pnlDnGalleryDescription->setStyleSheet(qs);
	ui.pnlDnGalleryTitle->setStyleSheet(qs);
	ui.pnlDnPicture->setStyleSheet(qs);
	ui.pnlDnPictureSectionHeader->setStyleSheet(qs);
	ui.pnlDnSlideA->setStyleSheet(qs);
	ui.pnlDnSlideP->setStyleSheet(qs);
*/
}

void DesignPage::_SetUplinkButtonIcon()		// UP menu button with icon
{
	const QIcon &icon = config.SetUplinkIcon(QString());		// empty name: default up-icon.png in resources
	ui.btnDnUp->setIcon(icon);
}
void DesignPage::_SetMenuButtonStyle()		
{
	static const QString buttons[] = { "#btnDnUp", "#btnDnAbout", "#btnDnContact", "#btnDnCaptDesc", "#btnDnDescription", "#btnDnToAlbums" };
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
	QString style = //"#lblDnName {\n " +
		config.SmallGalleryTitle.ForStyleSheet(addSemicolon);// +"\n}\n";
	ui.lblDnName->setStyleSheet(style);
}
void DesignPage::_SetLangButtonStyle()
{
	QString style = "#lblDnLang\n{\n " +
					config.Lang.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnLang->setStyleSheet(style);
}
void DesignPage::_SetLblDnGalleryStyle()
{
	QString style = "#lblDnGallery {\n " +
				config.GalleryTitle.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnGallery->setStyleSheet(style);
}
void DesignPage::_SetLblDnGalleryDescriptionStyle()
{
	QString style = "#lblDnGalleryDescription {\n " +
		config.GalleryDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnGalleryDescription->setStyleSheet(style);
}
void DesignPage::_SetLblDnPictureSectionLabelStyle()
{
	QString style = "#lblDnPictureSectionLabel {\n " +
		config.Section.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnPictureSectionLabel->setStyleSheet(style);
}
void DesignPage::_SetLblDnImagePStyle()			// image thumbnail image
{

}
void DesignPage::_SetLblDnDescriptionPStyle()
{
	QString style = "#lblDnDescriptionP {\n " +
		config.ImageDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnDescriptionP->setStyleSheet(style);
}
void DesignPage::_SetLblDnTitlePStyle()			// image title first line
{
	ui.lblDnTitleP->setStyleSheet( "#lblDnTitleP {\n " +
		config.ImageTitle.ForStyleSheet(config.ImageTitle.differentFirstLine, true) + "\n}\n" );
}
void DesignPage::_SetLblDnTitlePOStyle()		// when first line differs then image title all other lines
{
	ui.lblDnTitleP->setStyleSheet( "#lblDnTitlePO {\n " +
		config.ImageTitle.ForStyleSheet(notFirstOne, addSemicolon) + "\n}\n" );
}
void DesignPage::_SetLblDnAlbumSectionLabelStyle()	// label styles are the same for albums and pictures
{
	QString style = "#lblDnAlbumSectionLabel {\n " +
		config.Section.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnPictureSectionLabel->setStyleSheet(style);
}
void DesignPage::_SetLblDnImageAStyle()			// album thumbnail image
{

}
void DesignPage::_SetLblDnDescriptionAStyle()
{
	QString style = "#lblDnDescriptionA {\n " +
		config.ImageDesc.ForStyleSheet(addSemicolon) + "\n}\n";
	ui.lblDnDescriptionP->setStyleSheet(style);
}
void DesignPage::_SetLblDnTitleAStyle()
{
	ui.lblDnTitleP->setStyleSheet( "#lblDnTitleA {\n " +
		config.ImageTitle.ForStyleSheet(config.ImageTitle.differentFirstLine, true) + "\n}\n" );
}
void DesignPage::_SetLblDnTitleAOStyle()
{
	ui.lblDnTitleP->setStyleSheet("#lblDnTitleAO {\n " +
		config.ImageTitle.ForStyleSheet(notFirstOne, addSemicolon) + "\n}\n");
}
void DesignPage::_SetLblDnSummaryStyle()
{
	ui.lblDnSummary->setStyleSheet("#lblDnSummary {\n " +
		config.Footer.ForStyleSheet(addSemicolon) + "\n}\n");
}
void DesignPage::_SetLblDnCopyrightStyle()
{
	ui.lblDnSummary->setStyleSheet("#lblDnCopyright {\n " +
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

void DesignPage::on_lblDnSummary_clicked() 				  { qDebug("nLblDnSummary");			emit SignalClicked(nLblDnSummary				);}
void DesignPage::on_lblDnCopyright_clicked() 			  { qDebug("nLblDnCopyright");			emit SignalClicked(nLblDnCopyright	 			);}
