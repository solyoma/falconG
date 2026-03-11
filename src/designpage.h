#pragma once
#include <QtWidgets/QWidget>

#include "clickablelabel.h"
#include "ui_designpage.h"

class DesignPage : public QWidget
{
	Q_OBJECT
public:
	DesignPage(QWidget* parent = Q_NULLPTR);
	~DesignPage() {}

protected:
	void mousePressEvent(QMouseEvent* pevent) override;
private:
	Ui::DesignPageClass ui;

	void _MyMousePressEvent(QMouseEvent* pevent);

	void _SetGlobalStyle();	// which index in combo box, each index may correspond to more than one item
									// see common.h for constants from 'nBtnDnUp' to ''nLblCopyright'
	void _SetUplinkButtonIcon();		// button w. icon
	void _SetMenuButtonStyle();		// all menu buttons

	void _SetSmallTitleStyle();						 // small title
	void _SetLangButtonStyle();						 // language

	void _SetLblDnGalleryStyle();					 // gallery title
	void _SetLblDnGalleryDescriptionStyle();
	void _SetLblDnPictureSectionLabelStyle();		 // 'Pictures'
	void _SetLblDnImagePStyle();					 // picture thumbnail image
	void _SetLblDnDescriptionPStyle();				 // image description
	void _SetLblDnTitlePStyle();					 // picture title
	void _SetLblDnTitlePOStyle();					 // picture title 2nd line
	void _SetLblDnAlbumSectionLabelStyle();			 // 'Albums'
	void _SetLblDnImageAStyle();					 // album thumbnail image
	void _SetLblDnDescriptionAStyle();				 // album description
	void _SetLblDnTitleAStyle();					 // album title
	void _SetLblDnTitleAOStyle();					 // album title 2nd line
	void _SetLblDnSummaryStyle();					 // 'this gallery contains...'
	void _SetLblDnCopyrightStyle();
	void _SetLblDnHeaderStyle();
	void _SetLblLightboxStyle();
	void _SetLblLightboxTitleStyle();

public slots:
	void SlotItemStyleChanged(int indexInCombobox); // callback from falcong.cpp, uses global 'config'

signals:
	void SignalClicked(int);	// which item see numbers above this
		// item id numbers must reflect the items in this window

private slots:									 // 0 page				 -> global settings tab
	void on_btnDnUp_clicked();					 // 1 btnDnUp			 ->	selected Item tab
	void on_btnDnAbout_clicked();				 // 2 btnDnAbout			 ->		-"-
	void on_btnDnContact_clicked();				 // 3 btnDnContact		 ->		-"-
	void on_btnDnCaptDesc_clicked();			 // 4 btnDnCaptDesc		 ->		-"-
	void on_btnDnDescription_clicked();			 // 5 btnDnDescription	 ->		-"-
	void on_btnDnToAlbums_clicked();			 // 6 btnDnToAlbums		 ->		-"-
	void on_lblDnName_clicked();				 // 7 lblDnName			 ->		-"-
	void on_lblDnLang_clicked();				 // 8 lblDnLang			 ->		-"-

	void on_lblDnGallery_clicked();				 //  9 lblDnGallery			   ->  -"-
	void on_lblDnGalleryDescription_clicked();	 // 10 lblDnGalleryDescription  ->  -"-	 
	void on_lblDnPictureSectionLabel_clicked();	 //	11 lblDnPictureSectionLabel
												 // 12 pnlDnSlideP - handled with callback of slideWidget
	void on_lblDnDescriptionP_clicked();		 // 13 lblDnDescriptionP ->  -"-
	void on_lblDnTitleP_clicked();				 // 14 lblDnTitleP		 ->  -"-
	void on_lblDnTitlePO_clicked();				 // 15 lblDnTitlePO		 ->  -"-
	void on_lblDnAlbumSectionLabel_clicked();	 //	16 lblDnAlbumSectionLabel
												 // 17 
	void on_lblDnDescriptionA_clicked();		 // 18 lblDnDescriptionA ->  -"-
	void on_lblDnTitleA_clicked();				 // 19 lblDnImageP		 ->  -"
	void on_lblDnTitleAO_clicked();				 // 20 lblDnImageA		 ->  -"-
	void on_lblDnSummary_clicked();				 // 21 lblDnSummary		 ->  -"-
	void on_lblDnCopyright_clicked();			 // 22 lblDnCopyright	 ->  -"-
};