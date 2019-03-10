/********************************************************************************
** Form generated from reading UI file 'falcong.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FALCONG_H
#define UI_FALCONG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <thumbnailWidget.h>

QT_BEGIN_NAMESPACE

class Ui_falconGClass
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QWidget *centralWidget;
    QGridLayout *gridLayout_11;
    QWidget *widget_10;
    QHBoxLayout *horizontalLayout_1;
    QPushButton *btnGenerate;
    QCheckBox *chkGenerateAll;
    QCheckBox *chkButImages;
    QPushButton *btnPreview;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *btnSaveStyleSheet;
    QPushButton *btnSaveConfig;
    QPushButton *btnExit;
    QTabWidget *tabFalconG;
    QWidget *tabGallery;
    QVBoxLayout *verticalLayout_17;
    QGroupBox *gbLocalMachine;
    QGridLayout *gridLayout_5;
    QCheckBox *chkSourceRelativePerSign;
    QLineEdit *edtDestGallery;
    QLineEdit *edtSourceGallery;
    QToolButton *btnBrowseSource;
    QLabel *label_17;
    QToolButton *btnBrowseDestination;
    QPushButton *btnSourceHistory;
    QGroupBox *gbServer;
    QFormLayout *formLayout;
    QLabel *label_19;
    QLineEdit *edtServerAddress;
    QLabel *label_20;
    QLineEdit *edtGalleryRoot;
    QLabel *label_5;
    QLineEdit *edtUplink;
    QLabel *label_21;
    QLineEdit *edtMainPage;
    QHBoxLayout *horizontalLayout_16;
    QGroupBox *gbGallery;
    QGridLayout *gridLayout_17;
    QLabel *label_36;
    QLabel *label_47;
    QLineEdit *edtGalleryTitle;
    QLineEdit *edtAbout;
    QCheckBox *chkMenuToContact;
    QCheckBox *chkMenuToDescriptions;
    QCheckBox *chkRightClickProtected;
    QCheckBox *chkMenuToToggleCaptions;
    QLabel *label_50;
    QCheckBox *chkAllowLinkToImages;
    QLabel *label_42;
    QCheckBox *chkMenuToAbout;
    QCheckBox *chkCanDownload;
    QLineEdit *edtEmailTo;
    QLineEdit *edtBackImage;
    QCheckBox *chkForceSSL;
    QCheckBox *chkFacebook;
    QVBoxLayout *verticalLayout_15;
    QGroupBox *gbLatest;
    QGridLayout *gridLayout_6;
    QSpinBox *sbNewDays;
    QLabel *label_31;
    QLabel *label_30;
    QLabel *label_18;
    QSpinBox *sbLatestCount;
    QCheckBox *chkSetLatest;
    QGroupBox *gbGoogleAnalytics;
    QGridLayout *gridLayout_2;
    QCheckBox *chkUseGoogleAnalytics;
    QLabel *label_9;
    QLineEdit *edtTrackingCode;
    QGroupBox *gbAdvanced;
    QFormLayout *formLayout_2;
    QLabel *label_23;
    QLineEdit *edtThumb;
    QLabel *label_22;
    QLineEdit *edtAlbumDir;
    QLabel *label_44;
    QLineEdit *edtFontDir;
    QLabel *label_46;
    QLineEdit *edtDefaultFonts;
    QLabel *label_45;
    QLineEdit *edtBaseName;
    QLabel *label_56;
    QLineEdit *edtImg;
    QGridLayout *gridLayout_16;
    QCheckBox *chkReadFromGallery;
    QCheckBox *chkOvrImages;
    QCheckBox *chkAddDescToAll;
    QCheckBox *chkAddTitlesToAll;
    QCheckBox *chkreadJalbum;
    QWidget *pnlProgress;
    QGridLayout *gridLayout_18;
    QLabel *lblProgressTitle;
    QLabel *lblProgressDesc;
    QLabel *lblRemainingTimeTitle;
    QLabel *lblRemainingTime;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_43;
    QLabel *lblImagesPerSec;
    QProgressBar *progressBar;
    QLabel *lblProgress;
    QSpacerItem *verticalSpacer;
    QWidget *tabDesign;
    QVBoxLayout *verticalLayout_16;
    QHBoxLayout *horizontalLayout_10;
    QVBoxLayout *verticalLayout_13;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QCheckBox *chkBackgroundOpacity;
    QSpinBox *sbBackgroundOpacity;
    QLineEdit *edtBackgroundColor;
    QCheckBox *chkTextOpacity;
    QSpinBox *sbTextOpacity;
    QLabel *label_33;
    QToolButton *btnColor;
    QToolButton *btnBackground;
    QLabel *label_35;
    QLabel *label_34;
    QLabel *label_26;
    QLineEdit *edtTextColor;
    QLabel *label_16;
    QLabel *label_57;
    QGroupBox *gbTextShadow;
    QHBoxLayout *horizontalLayout;
    QLabel *label_37;
    QSpinBox *sbShadowHoriz;
    QLabel *label_38;
    QSpinBox *sbShadowVert;
    QLabel *label_39;
    QSpinBox *sbShadowBlur;
    QToolButton *btnShadowColor;
    QCheckBox *chkShadowOn;
    QGroupBox *gbMenu;
    QGridLayout *gridLayout_12;
    QLabel *label_24;
    QLabel *label_4;
    QToolButton *btnGradStartColor;
    QSpinBox *sbGradStartPos;
    QLineEdit *edtGradMiddleColor;
    QLineEdit *edtGradStartColor;
    QCheckBox *chkGradient;
    QToolButton *btnGradMiddleColor;
    QToolButton *btnGradStopColor;
    QToolButton *btnGradBorder;
    QSpinBox *sbGradStopPos;
    QCheckBox *chkButtonBorder;
    QSpinBox *sbGradMiddlePos;
    QLineEdit *edtGradStopColor;
    QLabel *label_58;
    QLabel *label_59;
    QLabel *label_60;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_13;
    QLabel *label_27;
    QLineEdit *edtFontFamily;
    QLabel *label_28;
    QComboBox *cbPointSize;
    QCheckBox *chkBold;
    QCheckBox *chkItalic;
    QCheckBox *chkUnderline;
    QCheckBox *chkStrikethrough;
    QGroupBox *gbDecoration;
    QGridLayout *gridLayout_10;
    QCheckBox *chkIconTop;
    QCheckBox *chkIconText;
    QCheckBox *chkImageBorder;
    QToolButton *btnBorderColor;
    QLabel *label_14;
    QSpinBox *sbBorderWidth;
    QLabel *label_15;
    QWidget *widget_5;
    QHBoxLayout *horizontalLayout_11;
    QToolButton *btnDisplayHint;
    QSpacerItem *horizontalSpacer_7;
    QPushButton *btnResetDesign;
    QSpacerItem *verticalSpacer_2;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_14;
    QWidget *pnlGallery;
    QVBoxLayout *verticalLayout;
    QPushButton *btnGalleryTitle;
    QPushButton *btnGalleryDesc;
    QFrame *frmWeb;
    QGridLayout *gridLayout_14;
    QPushButton *btnSection;
    QPushButton *btnAlbumDesc;
    QPushButton *btnAlbumTitle;
    QToolButton *btnImage;
    QHBoxLayout *horizontalLayout_13;
    QPushButton *btnUplink;
    QPushButton *btnMenu;
    QPushButton *btnCaption;
    QSpacerItem *horizontalSpacer_10;
    QHBoxLayout *horizontalLayout_12;
    QPushButton *btnSmallGalleryTitle;
    QPushButton *btnLang;
    QSpacerItem *horizontalSpacer_11;
    QHBoxLayout *horizontalLayout_15;
    QPushButton *btnIconToTop;
    QPushButton *btnImageTitle;
    QPushButton *btnDescription;
    QPushButton *btnImageDesc;
    QSpacerItem *verticalSpacer_5;
    QWidget *tabImages;
    QVBoxLayout *verticalLayout_1;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label;
    QCheckBox *chkDoNotEnlarge;
    QSpinBox *sbThumbnailWidth;
    QSpinBox *sbThumbnailHeight;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_9;
    QToolButton *btnLink;
    QSpinBox *sbImageWidth;
    QSpinBox *sbImageHeight;
    QWidget *widget;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *gbWatermark;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_4;
    QGridLayout *gridLayout_8;
    QSpinBox *sbWmOpacity;
    QLabel *label_32;
    QSpacerItem *horizontalSpacer_6;
    QLabel *label_12;
    QCheckBox *chkUseWM;
    QLabel *label_13;
    QLineEdit *edtWatermark;
    QHBoxLayout *horizontalLayout_4;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_3;
    QLineEdit *edtWmVertMargin;
    QLabel *label_8;
    QComboBox *cbWmVPosition;
    QLabel *label_6;
    QGroupBox *groupBox_9;
    QGridLayout *gridLayout_9;
    QLabel *label_48;
    QComboBox *cbWmHPosition;
    QLabel *label_49;
    QLineEdit *edtWmHorizMargin;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_25;
    QSpinBox *sbWmShadowHoriz;
    QLabel *label_40;
    QSpinBox *sbWmShadowVert;
    QLabel *label_41;
    QSpinBox *sbWmShadowBlur;
    QLabel *label_52;
    QToolButton *btnWmShadowColor;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_8;
    QGridLayout *gridLayout_7;
    QLineEdit *edtWmColor;
    QToolButton *btnWmColor;
    QToolButton *btnSelectWmFont;
    QLabel *label_29;
    QLabel *lblWaterMarkFont;
    QLabel *lblWmSample;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer_4;
    QWidget *tabEdit;
    QVBoxLayout *verticalLayout_20;
    QSplitter *editSplitter;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout_19;
    QSplitter *splitter;
    QWidget *widget_4;
    QVBoxLayout *verticalLayout_9;
    QVBoxLayout *verticalLayout_10;
    QLabel *label_51;
    QTreeView *trvAlbums;
    QLabel *lblTotalCount;
    QWidget *widget_6;
    QVBoxLayout *verticalLayout_11;
    QVBoxLayout *verticalLayout_12;
    QLabel *label_53;
    ThumbnailWidget *tnvImages;
    QLabel *lblImageCount;
    QWidget *widget_7;
    QVBoxLayout *verticalLayout_18;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_5;
    QComboBox *cmbBaseLanguage;
    QLabel *label_54;
    QTextEdit *edtBaseTitle;
    QVBoxLayout *verticalLayout_6;
    QComboBox *cmbLanguage;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_10;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *btnSaveChangedTitle;
    QCheckBox *chkChangeTitleEverywhere;
    QTextEdit *edtTitleText;
    QHBoxLayout *horizontalLayout_8;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_11;
    QTextEdit *edtBaseDescription;
    QVBoxLayout *verticalLayout_8;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_55;
    QSpacerItem *horizontalSpacer_8;
    QPushButton *btnSaveChangedDescription;
    QCheckBox *chkChangeDescriptionEverywhere;
    QTextEdit *edtDescriptionText;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *falconGClass)
    {
        if (falconGClass->objectName().isEmpty())
            falconGClass->setObjectName(QString::fromUtf8("falconGClass"));
        falconGClass->setEnabled(true);
        falconGClass->resize(753, 805);
        falconGClass->setMinimumSize(QSize(753, 0));
        falconGClass->setMaximumSize(QSize(16777215, 16777215));
        falconGClass->setStyleSheet(QString::fromUtf8(""));
        actionOpen = new QAction(falconGClass);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionExit = new QAction(falconGClass);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        centralWidget = new QWidget(falconGClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_11 = new QGridLayout(centralWidget);
        gridLayout_11->setSpacing(6);
        gridLayout_11->setContentsMargins(11, 11, 11, 11);
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        gridLayout_11->setContentsMargins(2, 2, 2, 2);
        widget_10 = new QWidget(centralWidget);
        widget_10->setObjectName(QString::fromUtf8("widget_10"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget_10->sizePolicy().hasHeightForWidth());
        widget_10->setSizePolicy(sizePolicy);
        widget_10->setMinimumSize(QSize(0, 0));
        horizontalLayout_1 = new QHBoxLayout(widget_10);
        horizontalLayout_1->setSpacing(6);
        horizontalLayout_1->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_1->setObjectName(QString::fromUtf8("horizontalLayout_1"));
        btnGenerate = new QPushButton(widget_10);
        btnGenerate->setObjectName(QString::fromUtf8("btnGenerate"));
        btnGenerate->setEnabled(false);

        horizontalLayout_1->addWidget(btnGenerate);

        chkGenerateAll = new QCheckBox(widget_10);
        chkGenerateAll->setObjectName(QString::fromUtf8("chkGenerateAll"));

        horizontalLayout_1->addWidget(chkGenerateAll);

        chkButImages = new QCheckBox(widget_10);
        chkButImages->setObjectName(QString::fromUtf8("chkButImages"));

        horizontalLayout_1->addWidget(chkButImages);

        btnPreview = new QPushButton(widget_10);
        btnPreview->setObjectName(QString::fromUtf8("btnPreview"));
        btnPreview->setEnabled(false);

        horizontalLayout_1->addWidget(btnPreview);

        horizontalSpacer_4 = new QSpacerItem(513, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_1->addItem(horizontalSpacer_4);

        btnSaveStyleSheet = new QPushButton(widget_10);
        btnSaveStyleSheet->setObjectName(QString::fromUtf8("btnSaveStyleSheet"));
        btnSaveStyleSheet->setEnabled(false);

        horizontalLayout_1->addWidget(btnSaveStyleSheet);

        btnSaveConfig = new QPushButton(widget_10);
        btnSaveConfig->setObjectName(QString::fromUtf8("btnSaveConfig"));
        btnSaveConfig->setEnabled(false);

        horizontalLayout_1->addWidget(btnSaveConfig);

        btnExit = new QPushButton(widget_10);
        btnExit->setObjectName(QString::fromUtf8("btnExit"));

        horizontalLayout_1->addWidget(btnExit);


        gridLayout_11->addWidget(widget_10, 1, 0, 1, 1);

        tabFalconG = new QTabWidget(centralWidget);
        tabFalconG->setObjectName(QString::fromUtf8("tabFalconG"));
        QFont font;
        font.setBold(false);
        font.setWeight(50);
        tabFalconG->setFont(font);
        tabGallery = new QWidget();
        tabGallery->setObjectName(QString::fromUtf8("tabGallery"));
        verticalLayout_17 = new QVBoxLayout(tabGallery);
        verticalLayout_17->setSpacing(2);
        verticalLayout_17->setContentsMargins(11, 11, 11, 11);
        verticalLayout_17->setObjectName(QString::fromUtf8("verticalLayout_17"));
        gbLocalMachine = new QGroupBox(tabGallery);
        gbLocalMachine->setObjectName(QString::fromUtf8("gbLocalMachine"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(gbLocalMachine->sizePolicy().hasHeightForWidth());
        gbLocalMachine->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        gbLocalMachine->setFont(font1);
        gridLayout_5 = new QGridLayout(gbLocalMachine);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        chkSourceRelativePerSign = new QCheckBox(gbLocalMachine);
        chkSourceRelativePerSign->setObjectName(QString::fromUtf8("chkSourceRelativePerSign"));
        chkSourceRelativePerSign->setFont(font);
        chkSourceRelativePerSign->setCheckable(true);
        chkSourceRelativePerSign->setChecked(true);

        gridLayout_5->addWidget(chkSourceRelativePerSign, 1, 0, 1, 3);

        edtDestGallery = new QLineEdit(gbLocalMachine);
        edtDestGallery->setObjectName(QString::fromUtf8("edtDestGallery"));
        edtDestGallery->setFont(font);

        gridLayout_5->addWidget(edtDestGallery, 2, 2, 1, 1);

        edtSourceGallery = new QLineEdit(gbLocalMachine);
        edtSourceGallery->setObjectName(QString::fromUtf8("edtSourceGallery"));
        edtSourceGallery->setFont(font);

        gridLayout_5->addWidget(edtSourceGallery, 0, 2, 1, 1);

        btnBrowseSource = new QToolButton(gbLocalMachine);
        btnBrowseSource->setObjectName(QString::fromUtf8("btnBrowseSource"));
        btnBrowseSource->setFont(font);

        gridLayout_5->addWidget(btnBrowseSource, 0, 3, 1, 1);

        label_17 = new QLabel(gbLocalMachine);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setFont(font);

        gridLayout_5->addWidget(label_17, 2, 0, 1, 1);

        btnBrowseDestination = new QToolButton(gbLocalMachine);
        btnBrowseDestination->setObjectName(QString::fromUtf8("btnBrowseDestination"));
        btnBrowseDestination->setFont(font);

        gridLayout_5->addWidget(btnBrowseDestination, 2, 3, 1, 1);

        btnSourceHistory = new QPushButton(gbLocalMachine);
        btnSourceHistory->setObjectName(QString::fromUtf8("btnSourceHistory"));
        btnSourceHistory->setFont(font);
        btnSourceHistory->setFlat(false);

        gridLayout_5->addWidget(btnSourceHistory, 0, 0, 1, 1);


        verticalLayout_17->addWidget(gbLocalMachine);

        gbServer = new QGroupBox(tabGallery);
        gbServer->setObjectName(QString::fromUtf8("gbServer"));
        sizePolicy1.setHeightForWidth(gbServer->sizePolicy().hasHeightForWidth());
        gbServer->setSizePolicy(sizePolicy1);
        gbServer->setFont(font1);
        formLayout = new QFormLayout(gbServer);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label_19 = new QLabel(gbServer);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setFont(font);

        formLayout->setWidget(0, QFormLayout::LabelRole, label_19);

        edtServerAddress = new QLineEdit(gbServer);
        edtServerAddress->setObjectName(QString::fromUtf8("edtServerAddress"));
        edtServerAddress->setFont(font);

        formLayout->setWidget(0, QFormLayout::FieldRole, edtServerAddress);

        label_20 = new QLabel(gbServer);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setFont(font);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_20);

        edtGalleryRoot = new QLineEdit(gbServer);
        edtGalleryRoot->setObjectName(QString::fromUtf8("edtGalleryRoot"));
        edtGalleryRoot->setFont(font);

        formLayout->setWidget(1, QFormLayout::FieldRole, edtGalleryRoot);

        label_5 = new QLabel(gbServer);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font);

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        edtUplink = new QLineEdit(gbServer);
        edtUplink->setObjectName(QString::fromUtf8("edtUplink"));
        edtUplink->setFont(font);

        formLayout->setWidget(2, QFormLayout::FieldRole, edtUplink);

        label_21 = new QLabel(gbServer);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setFont(font);

        formLayout->setWidget(3, QFormLayout::LabelRole, label_21);

        edtMainPage = new QLineEdit(gbServer);
        edtMainPage->setObjectName(QString::fromUtf8("edtMainPage"));
        edtMainPage->setFont(font);

        formLayout->setWidget(3, QFormLayout::FieldRole, edtMainPage);


        verticalLayout_17->addWidget(gbServer);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setSpacing(6);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        gbGallery = new QGroupBox(tabGallery);
        gbGallery->setObjectName(QString::fromUtf8("gbGallery"));
        gbGallery->setFont(font1);
        gridLayout_17 = new QGridLayout(gbGallery);
        gridLayout_17->setSpacing(6);
        gridLayout_17->setContentsMargins(11, 11, 11, 11);
        gridLayout_17->setObjectName(QString::fromUtf8("gridLayout_17"));
        label_36 = new QLabel(gbGallery);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        label_36->setFont(font);

        gridLayout_17->addWidget(label_36, 3, 0, 1, 1);

        label_47 = new QLabel(gbGallery);
        label_47->setObjectName(QString::fromUtf8("label_47"));
        label_47->setFont(font);

        gridLayout_17->addWidget(label_47, 0, 0, 1, 1);

        edtGalleryTitle = new QLineEdit(gbGallery);
        edtGalleryTitle->setObjectName(QString::fromUtf8("edtGalleryTitle"));
        edtGalleryTitle->setFont(font);

        gridLayout_17->addWidget(edtGalleryTitle, 0, 1, 1, 2);

        edtAbout = new QLineEdit(gbGallery);
        edtAbout->setObjectName(QString::fromUtf8("edtAbout"));
        edtAbout->setFont(font);

        gridLayout_17->addWidget(edtAbout, 3, 1, 1, 2);

        chkMenuToContact = new QCheckBox(gbGallery);
        chkMenuToContact->setObjectName(QString::fromUtf8("chkMenuToContact"));
        chkMenuToContact->setFont(font);

        gridLayout_17->addWidget(chkMenuToContact, 7, 2, 1, 1);

        chkMenuToDescriptions = new QCheckBox(gbGallery);
        chkMenuToDescriptions->setObjectName(QString::fromUtf8("chkMenuToDescriptions"));
        chkMenuToDescriptions->setFont(font);

        gridLayout_17->addWidget(chkMenuToDescriptions, 9, 2, 1, 1);

        chkRightClickProtected = new QCheckBox(gbGallery);
        chkRightClickProtected->setObjectName(QString::fromUtf8("chkRightClickProtected"));
        chkRightClickProtected->setFont(font);

        gridLayout_17->addWidget(chkRightClickProtected, 10, 0, 1, 1);

        chkMenuToToggleCaptions = new QCheckBox(gbGallery);
        chkMenuToToggleCaptions->setObjectName(QString::fromUtf8("chkMenuToToggleCaptions"));
        chkMenuToToggleCaptions->setFont(font);

        gridLayout_17->addWidget(chkMenuToToggleCaptions, 10, 2, 1, 1);

        label_50 = new QLabel(gbGallery);
        label_50->setObjectName(QString::fromUtf8("label_50"));
        label_50->setFont(font);

        gridLayout_17->addWidget(label_50, 4, 0, 1, 1);

        chkAllowLinkToImages = new QCheckBox(gbGallery);
        chkAllowLinkToImages->setObjectName(QString::fromUtf8("chkAllowLinkToImages"));
        chkAllowLinkToImages->setFont(font);

        gridLayout_17->addWidget(chkAllowLinkToImages, 8, 0, 1, 2);

        label_42 = new QLabel(gbGallery);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        label_42->setFont(font);

        gridLayout_17->addWidget(label_42, 2, 0, 1, 1);

        chkMenuToAbout = new QCheckBox(gbGallery);
        chkMenuToAbout->setObjectName(QString::fromUtf8("chkMenuToAbout"));
        chkMenuToAbout->setFont(font);

        gridLayout_17->addWidget(chkMenuToAbout, 8, 2, 1, 1);

        chkCanDownload = new QCheckBox(gbGallery);
        chkCanDownload->setObjectName(QString::fromUtf8("chkCanDownload"));
        chkCanDownload->setFont(font);

        gridLayout_17->addWidget(chkCanDownload, 9, 0, 1, 2);

        edtEmailTo = new QLineEdit(gbGallery);
        edtEmailTo->setObjectName(QString::fromUtf8("edtEmailTo"));
        edtEmailTo->setFont(font);

        gridLayout_17->addWidget(edtEmailTo, 2, 1, 1, 2);

        edtBackImage = new QLineEdit(gbGallery);
        edtBackImage->setObjectName(QString::fromUtf8("edtBackImage"));
        edtBackImage->setFont(font);

        gridLayout_17->addWidget(edtBackImage, 4, 1, 1, 2);

        chkForceSSL = new QCheckBox(gbGallery);
        chkForceSSL->setObjectName(QString::fromUtf8("chkForceSSL"));
        chkForceSSL->setEnabled(true);
        chkForceSSL->setFont(font);
        chkForceSSL->setChecked(true);

        gridLayout_17->addWidget(chkForceSSL, 7, 0, 1, 2);

        chkFacebook = new QCheckBox(gbGallery);
        chkFacebook->setObjectName(QString::fromUtf8("chkFacebook"));
        chkFacebook->setFont(font);

        gridLayout_17->addWidget(chkFacebook, 11, 0, 1, 1);


        horizontalLayout_16->addWidget(gbGallery);

        verticalLayout_15 = new QVBoxLayout();
        verticalLayout_15->setSpacing(6);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        gbLatest = new QGroupBox(tabGallery);
        gbLatest->setObjectName(QString::fromUtf8("gbLatest"));
        gbLatest->setFont(font1);
        gridLayout_6 = new QGridLayout(gbLatest);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        sbNewDays = new QSpinBox(gbLatest);
        sbNewDays->setObjectName(QString::fromUtf8("sbNewDays"));
        sbNewDays->setFont(font);

        gridLayout_6->addWidget(sbNewDays, 0, 1, 1, 1);

        label_31 = new QLabel(gbLatest);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        label_31->setFont(font);

        gridLayout_6->addWidget(label_31, 0, 2, 1, 1);

        label_30 = new QLabel(gbLatest);
        label_30->setObjectName(QString::fromUtf8("label_30"));
        label_30->setFont(font);

        gridLayout_6->addWidget(label_30, 0, 0, 1, 1);

        label_18 = new QLabel(gbLatest);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setFont(font);

        gridLayout_6->addWidget(label_18, 0, 3, 1, 1);

        sbLatestCount = new QSpinBox(gbLatest);
        sbLatestCount->setObjectName(QString::fromUtf8("sbLatestCount"));
        sbLatestCount->setFont(font);
        sbLatestCount->setMinimum(1);
        sbLatestCount->setValue(10);

        gridLayout_6->addWidget(sbLatestCount, 0, 4, 1, 1);

        chkSetLatest = new QCheckBox(gbLatest);
        chkSetLatest->setObjectName(QString::fromUtf8("chkSetLatest"));
        chkSetLatest->setFont(font);

        gridLayout_6->addWidget(chkSetLatest, 3, 0, 1, 2);


        verticalLayout_15->addWidget(gbLatest);

        gbGoogleAnalytics = new QGroupBox(tabGallery);
        gbGoogleAnalytics->setObjectName(QString::fromUtf8("gbGoogleAnalytics"));
        sizePolicy.setHeightForWidth(gbGoogleAnalytics->sizePolicy().hasHeightForWidth());
        gbGoogleAnalytics->setSizePolicy(sizePolicy);
        gbGoogleAnalytics->setFont(font1);
        gridLayout_2 = new QGridLayout(gbGoogleAnalytics);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        chkUseGoogleAnalytics = new QCheckBox(gbGoogleAnalytics);
        chkUseGoogleAnalytics->setObjectName(QString::fromUtf8("chkUseGoogleAnalytics"));
        chkUseGoogleAnalytics->setFont(font);

        gridLayout_2->addWidget(chkUseGoogleAnalytics, 0, 0, 1, 1);

        label_9 = new QLabel(gbGoogleAnalytics);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setFont(font);

        gridLayout_2->addWidget(label_9, 0, 1, 1, 1);

        edtTrackingCode = new QLineEdit(gbGoogleAnalytics);
        edtTrackingCode->setObjectName(QString::fromUtf8("edtTrackingCode"));
        edtTrackingCode->setFont(font);

        gridLayout_2->addWidget(edtTrackingCode, 0, 2, 1, 1);


        verticalLayout_15->addWidget(gbGoogleAnalytics);

        gbAdvanced = new QGroupBox(tabGallery);
        gbAdvanced->setObjectName(QString::fromUtf8("gbAdvanced"));
        sizePolicy.setHeightForWidth(gbAdvanced->sizePolicy().hasHeightForWidth());
        gbAdvanced->setSizePolicy(sizePolicy);
        gbAdvanced->setFont(font1);
        formLayout_2 = new QFormLayout(gbAdvanced);
        formLayout_2->setSpacing(6);
        formLayout_2->setContentsMargins(11, 11, 11, 11);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_23 = new QLabel(gbAdvanced);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setMinimumSize(QSize(93, 0));
        label_23->setFont(font);

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_23);

        edtThumb = new QLineEdit(gbAdvanced);
        edtThumb->setObjectName(QString::fromUtf8("edtThumb"));
        edtThumb->setFont(font);

        formLayout_2->setWidget(1, QFormLayout::FieldRole, edtThumb);

        label_22 = new QLabel(gbAdvanced);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setFont(font);

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_22);

        edtAlbumDir = new QLineEdit(gbAdvanced);
        edtAlbumDir->setObjectName(QString::fromUtf8("edtAlbumDir"));
        edtAlbumDir->setFont(font);

        formLayout_2->setWidget(2, QFormLayout::FieldRole, edtAlbumDir);

        label_44 = new QLabel(gbAdvanced);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        label_44->setFont(font);

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_44);

        edtFontDir = new QLineEdit(gbAdvanced);
        edtFontDir->setObjectName(QString::fromUtf8("edtFontDir"));
        edtFontDir->setFont(font);

        formLayout_2->setWidget(3, QFormLayout::FieldRole, edtFontDir);

        label_46 = new QLabel(gbAdvanced);
        label_46->setObjectName(QString::fromUtf8("label_46"));
        label_46->setFont(font);

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_46);

        edtDefaultFonts = new QLineEdit(gbAdvanced);
        edtDefaultFonts->setObjectName(QString::fromUtf8("edtDefaultFonts"));
        edtDefaultFonts->setFont(font);

        formLayout_2->setWidget(4, QFormLayout::FieldRole, edtDefaultFonts);

        label_45 = new QLabel(gbAdvanced);
        label_45->setObjectName(QString::fromUtf8("label_45"));
        label_45->setFont(font);

        formLayout_2->setWidget(5, QFormLayout::LabelRole, label_45);

        edtBaseName = new QLineEdit(gbAdvanced);
        edtBaseName->setObjectName(QString::fromUtf8("edtBaseName"));
        edtBaseName->setFont(font);

        formLayout_2->setWidget(5, QFormLayout::FieldRole, edtBaseName);

        label_56 = new QLabel(gbAdvanced);
        label_56->setObjectName(QString::fromUtf8("label_56"));
        label_56->setMinimumSize(QSize(93, 0));
        label_56->setFont(font);

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_56);

        edtImg = new QLineEdit(gbAdvanced);
        edtImg->setObjectName(QString::fromUtf8("edtImg"));
        edtImg->setFont(font);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, edtImg);


        verticalLayout_15->addWidget(gbAdvanced);


        horizontalLayout_16->addLayout(verticalLayout_15);


        verticalLayout_17->addLayout(horizontalLayout_16);

        gridLayout_16 = new QGridLayout();
        gridLayout_16->setSpacing(6);
        gridLayout_16->setObjectName(QString::fromUtf8("gridLayout_16"));
        gridLayout_16->setContentsMargins(9, 4, 9, 4);
        chkReadFromGallery = new QCheckBox(tabGallery);
        chkReadFromGallery->setObjectName(QString::fromUtf8("chkReadFromGallery"));

        gridLayout_16->addWidget(chkReadFromGallery, 0, 0, 1, 1);

        chkOvrImages = new QCheckBox(tabGallery);
        chkOvrImages->setObjectName(QString::fromUtf8("chkOvrImages"));
        chkOvrImages->setFont(font);
        chkOvrImages->setChecked(true);

        gridLayout_16->addWidget(chkOvrImages, 1, 0, 1, 1);

        chkAddDescToAll = new QCheckBox(tabGallery);
        chkAddDescToAll->setObjectName(QString::fromUtf8("chkAddDescToAll"));
        chkAddDescToAll->setChecked(false);

        gridLayout_16->addWidget(chkAddDescToAll, 1, 2, 1, 1);

        chkAddTitlesToAll = new QCheckBox(tabGallery);
        chkAddTitlesToAll->setObjectName(QString::fromUtf8("chkAddTitlesToAll"));
        chkAddTitlesToAll->setChecked(false);

        gridLayout_16->addWidget(chkAddTitlesToAll, 0, 2, 1, 1);

        chkreadJalbum = new QCheckBox(tabGallery);
        chkreadJalbum->setObjectName(QString::fromUtf8("chkreadJalbum"));

        gridLayout_16->addWidget(chkreadJalbum, 0, 1, 1, 1);

        gridLayout_16->setColumnStretch(0, 1);
        gridLayout_16->setColumnStretch(2, 1);

        verticalLayout_17->addLayout(gridLayout_16);

        pnlProgress = new QWidget(tabGallery);
        pnlProgress->setObjectName(QString::fromUtf8("pnlProgress"));
        pnlProgress->setMaximumSize(QSize(16777215, 16777215));
        gridLayout_18 = new QGridLayout(pnlProgress);
        gridLayout_18->setSpacing(6);
        gridLayout_18->setContentsMargins(11, 11, 11, 11);
        gridLayout_18->setObjectName(QString::fromUtf8("gridLayout_18"));
        lblProgressTitle = new QLabel(pnlProgress);
        lblProgressTitle->setObjectName(QString::fromUtf8("lblProgressTitle"));

        gridLayout_18->addWidget(lblProgressTitle, 0, 0, 1, 2);

        lblProgressDesc = new QLabel(pnlProgress);
        lblProgressDesc->setObjectName(QString::fromUtf8("lblProgressDesc"));

        gridLayout_18->addWidget(lblProgressDesc, 0, 3, 1, 2);

        lblRemainingTimeTitle = new QLabel(pnlProgress);
        lblRemainingTimeTitle->setObjectName(QString::fromUtf8("lblRemainingTimeTitle"));

        gridLayout_18->addWidget(lblRemainingTimeTitle, 2, 0, 1, 1);

        lblRemainingTime = new QLabel(pnlProgress);
        lblRemainingTime->setObjectName(QString::fromUtf8("lblRemainingTime"));

        gridLayout_18->addWidget(lblRemainingTime, 2, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(379, 13, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_18->addItem(horizontalSpacer_2, 2, 2, 1, 1);

        label_43 = new QLabel(pnlProgress);
        label_43->setObjectName(QString::fromUtf8("label_43"));

        gridLayout_18->addWidget(label_43, 2, 3, 1, 1);

        lblImagesPerSec = new QLabel(pnlProgress);
        lblImagesPerSec->setObjectName(QString::fromUtf8("lblImagesPerSec"));

        gridLayout_18->addWidget(lblImagesPerSec, 2, 4, 1, 1);

        progressBar = new QProgressBar(pnlProgress);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setMaximumSize(QSize(16777215, 5));
        progressBar->setValue(0);
        progressBar->setTextVisible(false);

        gridLayout_18->addWidget(progressBar, 1, 0, 1, 3);

        lblProgress = new QLabel(pnlProgress);
        lblProgress->setObjectName(QString::fromUtf8("lblProgress"));
        lblProgress->setMinimumSize(QSize(60, 0));
        QFont font2;
        font2.setPointSize(10);
        lblProgress->setFont(font2);

        gridLayout_18->addWidget(lblProgress, 1, 3, 1, 1);


        verticalLayout_17->addWidget(pnlProgress);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_17->addItem(verticalSpacer);

        tabFalconG->addTab(tabGallery, QString());
        tabDesign = new QWidget();
        tabDesign->setObjectName(QString::fromUtf8("tabDesign"));
        verticalLayout_16 = new QVBoxLayout(tabDesign);
        verticalLayout_16->setSpacing(6);
        verticalLayout_16->setContentsMargins(11, 11, 11, 11);
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        verticalLayout_13 = new QVBoxLayout();
        verticalLayout_13->setSpacing(6);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        groupBox_4 = new QGroupBox(tabDesign);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setFont(font1);
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        chkBackgroundOpacity = new QCheckBox(groupBox_4);
        chkBackgroundOpacity->setObjectName(QString::fromUtf8("chkBackgroundOpacity"));
        chkBackgroundOpacity->setFont(font);

        gridLayout_4->addWidget(chkBackgroundOpacity, 1, 6, 1, 1);

        sbBackgroundOpacity = new QSpinBox(groupBox_4);
        sbBackgroundOpacity->setObjectName(QString::fromUtf8("sbBackgroundOpacity"));
        sbBackgroundOpacity->setEnabled(false);
        sbBackgroundOpacity->setFont(font);
        sbBackgroundOpacity->setMinimum(0);
        sbBackgroundOpacity->setMaximum(100);
        sbBackgroundOpacity->setSingleStep(10);
        sbBackgroundOpacity->setValue(100);

        gridLayout_4->addWidget(sbBackgroundOpacity, 1, 4, 1, 1);

        edtBackgroundColor = new QLineEdit(groupBox_4);
        edtBackgroundColor->setObjectName(QString::fromUtf8("edtBackgroundColor"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(edtBackgroundColor->sizePolicy().hasHeightForWidth());
        edtBackgroundColor->setSizePolicy(sizePolicy3);
        edtBackgroundColor->setFont(font);

        gridLayout_4->addWidget(edtBackgroundColor, 1, 2, 1, 1);

        chkTextOpacity = new QCheckBox(groupBox_4);
        chkTextOpacity->setObjectName(QString::fromUtf8("chkTextOpacity"));
        chkTextOpacity->setFont(font);

        gridLayout_4->addWidget(chkTextOpacity, 0, 6, 1, 1);

        sbTextOpacity = new QSpinBox(groupBox_4);
        sbTextOpacity->setObjectName(QString::fromUtf8("sbTextOpacity"));
        sbTextOpacity->setEnabled(false);
        sbTextOpacity->setFont(font);
        sbTextOpacity->setMinimum(0);
        sbTextOpacity->setMaximum(100);
        sbTextOpacity->setSingleStep(10);
        sbTextOpacity->setValue(100);

        gridLayout_4->addWidget(sbTextOpacity, 0, 4, 1, 1);

        label_33 = new QLabel(groupBox_4);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        label_33->setFont(font);

        gridLayout_4->addWidget(label_33, 1, 0, 1, 1);

        btnColor = new QToolButton(groupBox_4);
        btnColor->setObjectName(QString::fromUtf8("btnColor"));
        sizePolicy3.setHeightForWidth(btnColor->sizePolicy().hasHeightForWidth());
        btnColor->setSizePolicy(sizePolicy3);
        btnColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#ddd;}"));

        gridLayout_4->addWidget(btnColor, 0, 3, 1, 1);

        btnBackground = new QToolButton(groupBox_4);
        btnBackground->setObjectName(QString::fromUtf8("btnBackground"));
        sizePolicy3.setHeightForWidth(btnBackground->sizePolicy().hasHeightForWidth());
        btnBackground->setSizePolicy(sizePolicy3);
        btnBackground->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#a03020;}"));

        gridLayout_4->addWidget(btnBackground, 1, 3, 1, 1);

        label_35 = new QLabel(groupBox_4);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        label_35->setFont(font);

        gridLayout_4->addWidget(label_35, 1, 5, 1, 1);

        label_34 = new QLabel(groupBox_4);
        label_34->setObjectName(QString::fromUtf8("label_34"));
        label_34->setFont(font);

        gridLayout_4->addWidget(label_34, 0, 5, 1, 1);

        label_26 = new QLabel(groupBox_4);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setFont(font);

        gridLayout_4->addWidget(label_26, 0, 0, 1, 1);

        edtTextColor = new QLineEdit(groupBox_4);
        edtTextColor->setObjectName(QString::fromUtf8("edtTextColor"));
        sizePolicy3.setHeightForWidth(edtTextColor->sizePolicy().hasHeightForWidth());
        edtTextColor->setSizePolicy(sizePolicy3);
        edtTextColor->setFont(font);

        gridLayout_4->addWidget(edtTextColor, 0, 2, 1, 1);

        label_16 = new QLabel(groupBox_4);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setFont(font);

        gridLayout_4->addWidget(label_16, 0, 1, 1, 1);

        label_57 = new QLabel(groupBox_4);
        label_57->setObjectName(QString::fromUtf8("label_57"));
        label_57->setFont(font);

        gridLayout_4->addWidget(label_57, 1, 1, 1, 1);


        verticalLayout_13->addWidget(groupBox_4);

        gbTextShadow = new QGroupBox(tabDesign);
        gbTextShadow->setObjectName(QString::fromUtf8("gbTextShadow"));
        horizontalLayout = new QHBoxLayout(gbTextShadow);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_37 = new QLabel(gbTextShadow);
        label_37->setObjectName(QString::fromUtf8("label_37"));

        horizontalLayout->addWidget(label_37);

        sbShadowHoriz = new QSpinBox(gbTextShadow);
        sbShadowHoriz->setObjectName(QString::fromUtf8("sbShadowHoriz"));
        sizePolicy3.setHeightForWidth(sbShadowHoriz->sizePolicy().hasHeightForWidth());
        sbShadowHoriz->setSizePolicy(sizePolicy3);
        sbShadowHoriz->setFont(font);

        horizontalLayout->addWidget(sbShadowHoriz);

        label_38 = new QLabel(gbTextShadow);
        label_38->setObjectName(QString::fromUtf8("label_38"));

        horizontalLayout->addWidget(label_38);

        sbShadowVert = new QSpinBox(gbTextShadow);
        sbShadowVert->setObjectName(QString::fromUtf8("sbShadowVert"));
        sizePolicy3.setHeightForWidth(sbShadowVert->sizePolicy().hasHeightForWidth());
        sbShadowVert->setSizePolicy(sizePolicy3);
        sbShadowVert->setFont(font);

        horizontalLayout->addWidget(sbShadowVert);

        label_39 = new QLabel(gbTextShadow);
        label_39->setObjectName(QString::fromUtf8("label_39"));

        horizontalLayout->addWidget(label_39);

        sbShadowBlur = new QSpinBox(gbTextShadow);
        sbShadowBlur->setObjectName(QString::fromUtf8("sbShadowBlur"));
        sizePolicy3.setHeightForWidth(sbShadowBlur->sizePolicy().hasHeightForWidth());
        sbShadowBlur->setSizePolicy(sizePolicy3);
        sbShadowBlur->setFont(font);

        horizontalLayout->addWidget(sbShadowBlur);

        btnShadowColor = new QToolButton(gbTextShadow);
        btnShadowColor->setObjectName(QString::fromUtf8("btnShadowColor"));
        btnShadowColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#ddd;}"));

        horizontalLayout->addWidget(btnShadowColor);

        chkShadowOn = new QCheckBox(gbTextShadow);
        chkShadowOn->setObjectName(QString::fromUtf8("chkShadowOn"));

        horizontalLayout->addWidget(chkShadowOn);

        label_38->raise();
        label_37->raise();
        label_39->raise();
        sbShadowHoriz->raise();
        sbShadowVert->raise();
        sbShadowBlur->raise();
        btnShadowColor->raise();
        chkShadowOn->raise();

        verticalLayout_13->addWidget(gbTextShadow);

        gbMenu = new QGroupBox(tabDesign);
        gbMenu->setObjectName(QString::fromUtf8("gbMenu"));
        gbMenu->setMinimumSize(QSize(270, 120));
        gbMenu->setFont(font1);
        gridLayout_12 = new QGridLayout(gbMenu);
        gridLayout_12->setSpacing(3);
        gridLayout_12->setContentsMargins(11, 11, 11, 11);
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        gridLayout_12->setContentsMargins(3, 3, 3, 3);
        label_24 = new QLabel(gbMenu);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        gridLayout_12->addWidget(label_24, 0, 3, 1, 1);

        label_4 = new QLabel(gbMenu);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_12->addWidget(label_4, 0, 1, 1, 1);

        btnGradStartColor = new QToolButton(gbMenu);
        btnGradStartColor->setObjectName(QString::fromUtf8("btnGradStartColor"));
        sizePolicy3.setHeightForWidth(btnGradStartColor->sizePolicy().hasHeightForWidth());
        btnGradStartColor->setSizePolicy(sizePolicy3);
        btnGradStartColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#A90329;}"));

        gridLayout_12->addWidget(btnGradStartColor, 1, 2, 1, 1);

        sbGradStartPos = new QSpinBox(gbMenu);
        sbGradStartPos->setObjectName(QString::fromUtf8("sbGradStartPos"));
        sbGradStartPos->setFont(font);

        gridLayout_12->addWidget(sbGradStartPos, 1, 3, 1, 1);

        edtGradMiddleColor = new QLineEdit(gbMenu);
        edtGradMiddleColor->setObjectName(QString::fromUtf8("edtGradMiddleColor"));
        sizePolicy3.setHeightForWidth(edtGradMiddleColor->sizePolicy().hasHeightForWidth());
        edtGradMiddleColor->setSizePolicy(sizePolicy3);
        edtGradMiddleColor->setFont(font);

        gridLayout_12->addWidget(edtGradMiddleColor, 2, 1, 1, 1);

        edtGradStartColor = new QLineEdit(gbMenu);
        edtGradStartColor->setObjectName(QString::fromUtf8("edtGradStartColor"));
        sizePolicy3.setHeightForWidth(edtGradStartColor->sizePolicy().hasHeightForWidth());
        edtGradStartColor->setSizePolicy(sizePolicy3);
        edtGradStartColor->setFont(font);

        gridLayout_12->addWidget(edtGradStartColor, 1, 1, 1, 1);

        chkGradient = new QCheckBox(gbMenu);
        chkGradient->setObjectName(QString::fromUtf8("chkGradient"));
        chkGradient->setFont(font);
        chkGradient->setChecked(true);

        gridLayout_12->addWidget(chkGradient, 3, 4, 1, 1);

        btnGradMiddleColor = new QToolButton(gbMenu);
        btnGradMiddleColor->setObjectName(QString::fromUtf8("btnGradMiddleColor"));
        sizePolicy3.setHeightForWidth(btnGradMiddleColor->sizePolicy().hasHeightForWidth());
        btnGradMiddleColor->setSizePolicy(sizePolicy3);
        btnGradMiddleColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#890222;}"));

        gridLayout_12->addWidget(btnGradMiddleColor, 2, 2, 1, 1);

        btnGradStopColor = new QToolButton(gbMenu);
        btnGradStopColor->setObjectName(QString::fromUtf8("btnGradStopColor"));
        sizePolicy3.setHeightForWidth(btnGradStopColor->sizePolicy().hasHeightForWidth());
        btnGradStopColor->setSizePolicy(sizePolicy3);
        btnGradStopColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#6d0019;}"));

        gridLayout_12->addWidget(btnGradStopColor, 3, 2, 1, 1);

        btnGradBorder = new QToolButton(gbMenu);
        btnGradBorder->setObjectName(QString::fromUtf8("btnGradBorder"));
        sizePolicy3.setHeightForWidth(btnGradBorder->sizePolicy().hasHeightForWidth());
        btnGradBorder->setSizePolicy(sizePolicy3);
        btnGradBorder->setFont(font1);
        btnGradBorder->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#890222;color:#76fccc;}"));

        gridLayout_12->addWidget(btnGradBorder, 2, 4, 1, 1);

        sbGradStopPos = new QSpinBox(gbMenu);
        sbGradStopPos->setObjectName(QString::fromUtf8("sbGradStopPos"));
        sbGradStopPos->setFont(font);
        sbGradStopPos->setMaximum(100);
        sbGradStopPos->setValue(100);

        gridLayout_12->addWidget(sbGradStopPos, 3, 3, 1, 1);

        chkButtonBorder = new QCheckBox(gbMenu);
        chkButtonBorder->setObjectName(QString::fromUtf8("chkButtonBorder"));
        chkButtonBorder->setFont(font);
        chkButtonBorder->setChecked(true);

        gridLayout_12->addWidget(chkButtonBorder, 1, 4, 1, 1);

        sbGradMiddlePos = new QSpinBox(gbMenu);
        sbGradMiddlePos->setObjectName(QString::fromUtf8("sbGradMiddlePos"));
        sbGradMiddlePos->setFont(font);
        sbGradMiddlePos->setMaximum(100);
        sbGradMiddlePos->setValue(40);

        gridLayout_12->addWidget(sbGradMiddlePos, 2, 3, 1, 1);

        edtGradStopColor = new QLineEdit(gbMenu);
        edtGradStopColor->setObjectName(QString::fromUtf8("edtGradStopColor"));
        sizePolicy3.setHeightForWidth(edtGradStopColor->sizePolicy().hasHeightForWidth());
        edtGradStopColor->setSizePolicy(sizePolicy3);
        edtGradStopColor->setFont(font);

        gridLayout_12->addWidget(edtGradStopColor, 3, 1, 1, 1);

        label_58 = new QLabel(gbMenu);
        label_58->setObjectName(QString::fromUtf8("label_58"));
        label_58->setFont(font);

        gridLayout_12->addWidget(label_58, 1, 0, 1, 1);

        label_59 = new QLabel(gbMenu);
        label_59->setObjectName(QString::fromUtf8("label_59"));
        label_59->setFont(font);

        gridLayout_12->addWidget(label_59, 2, 0, 1, 1);

        label_60 = new QLabel(gbMenu);
        label_60->setObjectName(QString::fromUtf8("label_60"));
        label_60->setFont(font);

        gridLayout_12->addWidget(label_60, 3, 0, 1, 1);

        gridLayout_12->setColumnStretch(1, 1);
        gridLayout_12->setColumnStretch(2, 1);
        gridLayout_12->setColumnStretch(3, 1);
        gridLayout_12->setColumnStretch(4, 1);

        verticalLayout_13->addWidget(gbMenu);

        groupBox_5 = new QGroupBox(tabDesign);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setMinimumSize(QSize(270, 100));
        groupBox_5->setFont(font1);
        gridLayout_13 = new QGridLayout(groupBox_5);
        gridLayout_13->setSpacing(3);
        gridLayout_13->setContentsMargins(11, 11, 11, 11);
        gridLayout_13->setObjectName(QString::fromUtf8("gridLayout_13"));
        gridLayout_13->setContentsMargins(3, 3, 3, 3);
        label_27 = new QLabel(groupBox_5);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setFont(font);

        gridLayout_13->addWidget(label_27, 0, 0, 1, 1);

        edtFontFamily = new QLineEdit(groupBox_5);
        edtFontFamily->setObjectName(QString::fromUtf8("edtFontFamily"));
        edtFontFamily->setFont(font);

        gridLayout_13->addWidget(edtFontFamily, 0, 1, 1, 2);

        label_28 = new QLabel(groupBox_5);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        label_28->setFont(font);

        gridLayout_13->addWidget(label_28, 1, 0, 1, 1);

        cbPointSize = new QComboBox(groupBox_5);
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->addItem(QString());
        cbPointSize->setObjectName(QString::fromUtf8("cbPointSize"));
        cbPointSize->setFont(font);
        cbPointSize->setEditable(true);
        cbPointSize->setInsertPolicy(QComboBox::NoInsert);

        gridLayout_13->addWidget(cbPointSize, 1, 1, 1, 2);

        chkBold = new QCheckBox(groupBox_5);
        chkBold->setObjectName(QString::fromUtf8("chkBold"));
        chkBold->setFont(font);

        gridLayout_13->addWidget(chkBold, 2, 0, 1, 1);

        chkItalic = new QCheckBox(groupBox_5);
        chkItalic->setObjectName(QString::fromUtf8("chkItalic"));
        chkItalic->setFont(font);

        gridLayout_13->addWidget(chkItalic, 2, 2, 1, 1);

        chkUnderline = new QCheckBox(groupBox_5);
        chkUnderline->setObjectName(QString::fromUtf8("chkUnderline"));
        chkUnderline->setFont(font);

        gridLayout_13->addWidget(chkUnderline, 3, 0, 1, 2);

        chkStrikethrough = new QCheckBox(groupBox_5);
        chkStrikethrough->setObjectName(QString::fromUtf8("chkStrikethrough"));
        chkStrikethrough->setFont(font);

        gridLayout_13->addWidget(chkStrikethrough, 3, 2, 1, 1);


        verticalLayout_13->addWidget(groupBox_5);

        gbDecoration = new QGroupBox(tabDesign);
        gbDecoration->setObjectName(QString::fromUtf8("gbDecoration"));
        gbDecoration->setMinimumSize(QSize(270, 0));
        gbDecoration->setFont(font1);
        gridLayout_10 = new QGridLayout(gbDecoration);
        gridLayout_10->setSpacing(3);
        gridLayout_10->setContentsMargins(11, 11, 11, 11);
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        gridLayout_10->setContentsMargins(3, 3, 3, 3);
        chkIconTop = new QCheckBox(gbDecoration);
        chkIconTop->setObjectName(QString::fromUtf8("chkIconTop"));
        chkIconTop->setFont(font);
        chkIconTop->setChecked(true);

        gridLayout_10->addWidget(chkIconTop, 5, 0, 1, 3);

        chkIconText = new QCheckBox(gbDecoration);
        chkIconText->setObjectName(QString::fromUtf8("chkIconText"));
        chkIconText->setFont(font);
        chkIconText->setChecked(true);

        gridLayout_10->addWidget(chkIconText, 6, 0, 1, 1);

        chkImageBorder = new QCheckBox(gbDecoration);
        chkImageBorder->setObjectName(QString::fromUtf8("chkImageBorder"));
        chkImageBorder->setFont(font);
        chkImageBorder->setChecked(true);

        gridLayout_10->addWidget(chkImageBorder, 0, 0, 1, 1);

        btnBorderColor = new QToolButton(gbDecoration);
        btnBorderColor->setObjectName(QString::fromUtf8("btnBorderColor"));
        btnBorderColor->setFont(font);
        btnBorderColor->setStyleSheet(QString::fromUtf8("QToolButton {\n"
"background-color:#ffff00;\n"
"}"));

        gridLayout_10->addWidget(btnBorderColor, 0, 1, 1, 1);

        label_14 = new QLabel(gbDecoration);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setFont(font);

        gridLayout_10->addWidget(label_14, 0, 2, 1, 1);

        sbBorderWidth = new QSpinBox(gbDecoration);
        sbBorderWidth->setObjectName(QString::fromUtf8("sbBorderWidth"));
        sbBorderWidth->setMinimumSize(QSize(0, 30));
        sbBorderWidth->setFont(font);
        sbBorderWidth->setMinimum(1);

        gridLayout_10->addWidget(sbBorderWidth, 0, 3, 1, 1);

        label_15 = new QLabel(gbDecoration);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setFont(font);

        gridLayout_10->addWidget(label_15, 0, 4, 1, 1);


        verticalLayout_13->addWidget(gbDecoration);

        widget_5 = new QWidget(tabDesign);
        widget_5->setObjectName(QString::fromUtf8("widget_5"));
        widget_5->setMinimumSize(QSize(0, 50));
        widget_5->setMaximumSize(QSize(16777215, 40));
        horizontalLayout_11 = new QHBoxLayout(widget_5);
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        btnDisplayHint = new QToolButton(widget_5);
        btnDisplayHint->setObjectName(QString::fromUtf8("btnDisplayHint"));

        horizontalLayout_11->addWidget(btnDisplayHint);

        horizontalSpacer_7 = new QSpacerItem(171, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_7);

        btnResetDesign = new QPushButton(widget_5);
        btnResetDesign->setObjectName(QString::fromUtf8("btnResetDesign"));

        horizontalLayout_11->addWidget(btnResetDesign);


        verticalLayout_13->addWidget(widget_5);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_13->addItem(verticalSpacer_2);


        horizontalLayout_10->addLayout(verticalLayout_13);

        groupBox_6 = new QGroupBox(tabDesign);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        groupBox_6->setMinimumSize(QSize(0, 689));
        groupBox_6->setFont(font1);
        verticalLayout_14 = new QVBoxLayout(groupBox_6);
        verticalLayout_14->setSpacing(6);
        verticalLayout_14->setContentsMargins(11, 11, 11, 11);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        pnlGallery = new QWidget(groupBox_6);
        pnlGallery->setObjectName(QString::fromUtf8("pnlGallery"));
        pnlGallery->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(pnlGallery);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        btnGalleryTitle = new QPushButton(pnlGallery);
        btnGalleryTitle->setObjectName(QString::fromUtf8("btnGalleryTitle"));
        QFont font3;
        font3.setFamily(QString::fromUtf8("Monotype Corsiva"));
        font3.setPointSize(16);
        font3.setBold(true);
        font3.setWeight(75);
        btnGalleryTitle->setFont(font3);
        btnGalleryTitle->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff;\n"
"font-family:\"Monotype Corsiva\";\n"
"font-size:16pt;\n"
"font-weight:bold;\n"
"}"));
        btnGalleryTitle->setFlat(true);

        verticalLayout->addWidget(btnGalleryTitle);

        btnGalleryDesc = new QPushButton(pnlGallery);
        btnGalleryDesc->setObjectName(QString::fromUtf8("btnGalleryDesc"));
        btnGalleryDesc->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff\n"
"}"));
        btnGalleryDesc->setFlat(true);

        verticalLayout->addWidget(btnGalleryDesc);


        verticalLayout_14->addWidget(pnlGallery);

        frmWeb = new QFrame(groupBox_6);
        frmWeb->setObjectName(QString::fromUtf8("frmWeb"));
        sizePolicy.setHeightForWidth(frmWeb->sizePolicy().hasHeightForWidth());
        frmWeb->setSizePolicy(sizePolicy);
        frmWeb->setMinimumSize(QSize(0, 549));
        frmWeb->setStyleSheet(QString::fromUtf8("QFrame {\n"
"background-color:#a03020;\n"
"}"));
        frmWeb->setFrameShape(QFrame::StyledPanel);
        frmWeb->setFrameShadow(QFrame::Raised);
        gridLayout_14 = new QGridLayout(frmWeb);
        gridLayout_14->setSpacing(6);
        gridLayout_14->setContentsMargins(11, 11, 11, 11);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        btnSection = new QPushButton(frmWeb);
        btnSection->setObjectName(QString::fromUtf8("btnSection"));
        QFont font4;
        font4.setFamily(QString::fromUtf8("Papyrus"));
        font4.setPointSize(22);
        font4.setBold(true);
        font4.setWeight(75);
        btnSection->setFont(font4);
        btnSection->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff;\n"
"font-family:Papyrus;\n"
"font-size:22pt;\n"
"font-weight:bold;\n"
"}"));
        btnSection->setFlat(true);

        gridLayout_14->addWidget(btnSection, 4, 1, 1, 1);

        btnAlbumDesc = new QPushButton(frmWeb);
        btnAlbumDesc->setObjectName(QString::fromUtf8("btnAlbumDesc"));
        btnAlbumDesc->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff\n"
"}"));
        btnAlbumDesc->setFlat(true);

        gridLayout_14->addWidget(btnAlbumDesc, 3, 1, 1, 1);

        btnAlbumTitle = new QPushButton(frmWeb);
        btnAlbumTitle->setObjectName(QString::fromUtf8("btnAlbumTitle"));
        btnAlbumTitle->setFont(font3);
        btnAlbumTitle->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff;\n"
"font-family:\"Monotype Corsiva\";\n"
"font-size:16pt;\n"
"font-weight:bold;\n"
"}"));
        btnAlbumTitle->setFlat(true);

        gridLayout_14->addWidget(btnAlbumTitle, 2, 1, 1, 1);

        btnImage = new QToolButton(frmWeb);
        btnImage->setObjectName(QString::fromUtf8("btnImage"));
        sizePolicy3.setHeightForWidth(btnImage->sizePolicy().hasHeightForWidth());
        btnImage->setSizePolicy(sizePolicy3);
        btnImage->setMinimumSize(QSize(275, 185));
        btnImage->setMaximumSize(QSize(275, 185));
        btnImage->setStyleSheet(QString::fromUtf8("QToolButton {\n"
" border: 1px solid #ffff00;\n"
"}"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/Resources/Lazise_02596-Edit.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnImage->setIcon(icon);
        btnImage->setIconSize(QSize(270, 180));

        gridLayout_14->addWidget(btnImage, 5, 1, 1, 1);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(6);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        btnUplink = new QPushButton(frmWeb);
        btnUplink->setObjectName(QString::fromUtf8("btnUplink"));
        btnUplink->setEnabled(true);
        btnUplink->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a90329, stop: 0.4  #8f0222, stop:1 #6d0019);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
"QPushButton:pressed {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a6d019, stop: 0.4  #8f0222, stop:1 #a90329);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/Resources/up-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnUplink->setIcon(icon1);
        btnUplink->setFlat(false);

        horizontalLayout_13->addWidget(btnUplink);

        btnMenu = new QPushButton(frmWeb);
        btnMenu->setObjectName(QString::fromUtf8("btnMenu"));
        btnMenu->setEnabled(true);
        btnMenu->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a90329, stop: 0.4  #8f0222, stop:1 #6d0019);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
"QPushButton:pressed {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a6d019, stop: 0.4  #8f0222, stop:1 #a90329);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
""));

        horizontalLayout_13->addWidget(btnMenu);

        btnCaption = new QPushButton(frmWeb);
        btnCaption->setObjectName(QString::fromUtf8("btnCaption"));
        btnCaption->setEnabled(true);
        btnCaption->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a90329, stop: 0.4  #8f0222, stop:1 #6d0019);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
"QPushButton:pressed {\n"
"background:qlineargradient(x1:0, y1:0, x2:0, y2:1,  stop:0 #a6d019, stop: 0.4  #8f0222, stop:1 #a90329);\n"
"color:#fff; \n"
"border: 1px solid #6d0019;\n"
"padding: 4px 10px 3px;\n"
"font-size: 13px;\n"
"line-height: 18px;\n"
"margin: 2px 3px;\n"
"font-weight: 800;\n"
"box-shadow: 0px 1px 1px rgba(0,0,0,0.1);\n"
"border-radius: 5px;\n"
"}\n"
""));

        horizontalLayout_13->addWidget(btnCaption);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_10);

        horizontalLayout_13->setStretch(1, 1);
        horizontalLayout_13->setStretch(2, 1);
        horizontalLayout_13->setStretch(3, 9999);

        gridLayout_14->addLayout(horizontalLayout_13, 1, 0, 1, 3);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setSpacing(6);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        btnSmallGalleryTitle = new QPushButton(frmWeb);
        btnSmallGalleryTitle->setObjectName(QString::fromUtf8("btnSmallGalleryTitle"));
        btnSmallGalleryTitle->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"	color:#fff; \n"
"	font-family:Constantia, Palatino, \"Palatino Linotype\", \"Palatino LT STD\", Georgia, serif;\n"
"	font-weight:400;\n"
"	font-size:10pt;\n"
"	text-align:left;\n"
"}"));
        btnSmallGalleryTitle->setFlat(true);

        horizontalLayout_12->addWidget(btnSmallGalleryTitle);

        btnLang = new QPushButton(frmWeb);
        btnLang->setObjectName(QString::fromUtf8("btnLang"));
        btnLang->setEnabled(true);
        btnLang->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"   color:#FFFF00;\n"
"background-color:#a03020;\n"
"   font-size: 8pt;\n"
" }"));
        btnLang->setFlat(true);

        horizontalLayout_12->addWidget(btnLang);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_12->addItem(horizontalSpacer_11);


        gridLayout_14->addLayout(horizontalLayout_12, 0, 0, 1, 3);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setSpacing(6);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        btnIconToTop = new QPushButton(frmWeb);
        btnIconToTop->setObjectName(QString::fromUtf8("btnIconToTop"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icon/Resources/up-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnIconToTop->setIcon(icon2);
        btnIconToTop->setFlat(true);

        horizontalLayout_15->addWidget(btnIconToTop);

        btnImageTitle = new QPushButton(frmWeb);
        btnImageTitle->setObjectName(QString::fromUtf8("btnImageTitle"));
        QFont font5;
        font5.setFamily(QString::fromUtf8("Rage Italic"));
        font5.setPointSize(14);
        font5.setBold(true);
        font5.setWeight(75);
        btnImageTitle->setFont(font5);
        btnImageTitle->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#ddd;\n"
"text-shadow: 0px 0px 20px #aaa;\n"
"font-family: \"Rage Italic\";\n"
"font-size:14pt;\n"
"font-weight: bold;\n"
"}"));
        btnImageTitle->setFlat(true);

        horizontalLayout_15->addWidget(btnImageTitle);

        btnDescription = new QPushButton(frmWeb);
        btnDescription->setObjectName(QString::fromUtf8("btnDescription"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icon/Resources/content-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnDescription->setIcon(icon3);
        btnDescription->setIconSize(QSize(32, 32));
        btnDescription->setFlat(true);

        horizontalLayout_15->addWidget(btnDescription);


        gridLayout_14->addLayout(horizontalLayout_15, 7, 1, 1, 1);

        btnImageDesc = new QPushButton(frmWeb);
        btnImageDesc->setObjectName(QString::fromUtf8("btnImageDesc"));
        btnImageDesc->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color:#fff\n"
"}"));
        btnImageDesc->setFlat(true);

        gridLayout_14->addWidget(btnImageDesc, 6, 1, 1, 1);

        btnImage->raise();
        btnAlbumDesc->raise();
        btnSection->raise();
        btnAlbumTitle->raise();
        btnImageDesc->raise();

        verticalLayout_14->addWidget(frmWeb);

        verticalLayout_14->setStretch(1, 1);

        horizontalLayout_10->addWidget(groupBox_6);

        horizontalLayout_10->setStretch(1, 1);

        verticalLayout_16->addLayout(horizontalLayout_10);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_16->addItem(verticalSpacer_5);

        tabFalconG->addTab(tabDesign, QString());
        tabImages = new QWidget();
        tabImages->setObjectName(QString::fromUtf8("tabImages"));
        verticalLayout_1 = new QVBoxLayout(tabImages);
        verticalLayout_1->setSpacing(6);
        verticalLayout_1->setContentsMargins(11, 11, 11, 11);
        verticalLayout_1->setObjectName(QString::fromUtf8("verticalLayout_1"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 6, -1, 6);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(tabImages);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        groupBox->setFont(font1);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font);

        gridLayout->addWidget(label_3, 0, 4, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        chkDoNotEnlarge = new QCheckBox(groupBox);
        chkDoNotEnlarge->setObjectName(QString::fromUtf8("chkDoNotEnlarge"));
        chkDoNotEnlarge->setFont(font);
        chkDoNotEnlarge->setChecked(true);

        gridLayout->addWidget(chkDoNotEnlarge, 1, 5, 1, 1);

        sbThumbnailWidth = new QSpinBox(groupBox);
        sbThumbnailWidth->setObjectName(QString::fromUtf8("sbThumbnailWidth"));
        sbThumbnailWidth->setFont(font);
        sbThumbnailWidth->setMinimum(50);
        sbThumbnailWidth->setMaximum(8192);
        sbThumbnailWidth->setSingleStep(10);
        sbThumbnailWidth->setValue(600);

        gridLayout->addWidget(sbThumbnailWidth, 2, 1, 1, 1);

        sbThumbnailHeight = new QSpinBox(groupBox);
        sbThumbnailHeight->setObjectName(QString::fromUtf8("sbThumbnailHeight"));
        sbThumbnailHeight->setFont(font);
        sbThumbnailHeight->setMinimum(50);
        sbThumbnailHeight->setMaximum(8192);
        sbThumbnailHeight->setSingleStep(10);
        sbThumbnailHeight->setValue(400);

        gridLayout->addWidget(sbThumbnailHeight, 2, 4, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setFont(font);

        gridLayout->addWidget(label_7, 2, 0, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_9, 1, 6, 1, 1);

        btnLink = new QToolButton(groupBox);
        btnLink->setObjectName(QString::fromUtf8("btnLink"));
        btnLink->setFont(font);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icon/Resources/link.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnLink->setIcon(icon4);
        btnLink->setCheckable(true);

        gridLayout->addWidget(btnLink, 1, 2, 2, 1);

        sbImageWidth = new QSpinBox(groupBox);
        sbImageWidth->setObjectName(QString::fromUtf8("sbImageWidth"));
        sbImageWidth->setFont(font);
        sbImageWidth->setMinimum(50);
        sbImageWidth->setMaximum(8192);
        sbImageWidth->setSingleStep(10);
        sbImageWidth->setValue(1920);

        gridLayout->addWidget(sbImageWidth, 1, 1, 1, 1);

        sbImageHeight = new QSpinBox(groupBox);
        sbImageHeight->setObjectName(QString::fromUtf8("sbImageHeight"));
        sbImageHeight->setFont(font);
        sbImageHeight->setMinimum(50);
        sbImageHeight->setMaximum(8192);
        sbImageHeight->setSingleStep(10);
        sbImageHeight->setValue(1280);

        gridLayout->addWidget(sbImageHeight, 1, 4, 1, 1);


        verticalLayout_2->addWidget(groupBox);


        horizontalLayout_2->addLayout(verticalLayout_2);


        verticalLayout_1->addLayout(horizontalLayout_2);

        widget = new QWidget(tabImages);
        widget->setObjectName(QString::fromUtf8("widget"));
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);

        verticalLayout_1->addWidget(widget);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        gbWatermark = new QGroupBox(tabImages);
        gbWatermark->setObjectName(QString::fromUtf8("gbWatermark"));
        sizePolicy.setHeightForWidth(gbWatermark->sizePolicy().hasHeightForWidth());
        gbWatermark->setSizePolicy(sizePolicy);
        gbWatermark->setFont(font1);
        horizontalLayout_3 = new QHBoxLayout(gbWatermark);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        gridLayout_8 = new QGridLayout();
        gridLayout_8->setSpacing(6);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        gridLayout_8->setVerticalSpacing(9);
        gridLayout_8->setContentsMargins(4, 4, 4, 9);
        sbWmOpacity = new QSpinBox(gbWatermark);
        sbWmOpacity->setObjectName(QString::fromUtf8("sbWmOpacity"));
        sbWmOpacity->setFont(font);

        gridLayout_8->addWidget(sbWmOpacity, 0, 2, 1, 1);

        label_32 = new QLabel(gbWatermark);
        label_32->setObjectName(QString::fromUtf8("label_32"));

        gridLayout_8->addWidget(label_32, 0, 3, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_8->addItem(horizontalSpacer_6, 0, 4, 1, 1);

        label_12 = new QLabel(gbWatermark);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setFont(font);

        gridLayout_8->addWidget(label_12, 0, 1, 1, 1);

        chkUseWM = new QCheckBox(gbWatermark);
        chkUseWM->setObjectName(QString::fromUtf8("chkUseWM"));
        chkUseWM->setFont(font);

        gridLayout_8->addWidget(chkUseWM, 0, 0, 1, 1);

        label_13 = new QLabel(gbWatermark);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setFont(font);

        gridLayout_8->addWidget(label_13, 1, 0, 1, 1);

        edtWatermark = new QLineEdit(gbWatermark);
        edtWatermark->setObjectName(QString::fromUtf8("edtWatermark"));
        edtWatermark->setFont(font);

        gridLayout_8->addWidget(edtWatermark, 1, 1, 1, 4);


        verticalLayout_4->addLayout(gridLayout_8);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        groupBox_7 = new QGroupBox(gbWatermark);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        groupBox_7->setFont(font1);
        gridLayout_3 = new QGridLayout(groupBox_7);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        edtWmVertMargin = new QLineEdit(groupBox_7);
        edtWmVertMargin->setObjectName(QString::fromUtf8("edtWmVertMargin"));
        edtWmVertMargin->setFont(font);

        gridLayout_3->addWidget(edtWmVertMargin, 2, 1, 1, 1);

        label_8 = new QLabel(groupBox_7);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setFont(font);

        gridLayout_3->addWidget(label_8, 2, 2, 1, 1);

        cbWmVPosition = new QComboBox(groupBox_7);
        cbWmVPosition->addItem(QString());
        cbWmVPosition->addItem(QString());
        cbWmVPosition->addItem(QString());
        cbWmVPosition->setObjectName(QString::fromUtf8("cbWmVPosition"));
        cbWmVPosition->setFont(font);

        gridLayout_3->addWidget(cbWmVPosition, 0, 0, 1, 2);

        label_6 = new QLabel(groupBox_7);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setFont(font);

        gridLayout_3->addWidget(label_6, 2, 0, 1, 1);


        horizontalLayout_4->addWidget(groupBox_7);

        groupBox_9 = new QGroupBox(gbWatermark);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        groupBox_9->setFont(font1);
        gridLayout_9 = new QGridLayout(groupBox_9);
        gridLayout_9->setSpacing(6);
        gridLayout_9->setContentsMargins(11, 11, 11, 11);
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        label_48 = new QLabel(groupBox_9);
        label_48->setObjectName(QString::fromUtf8("label_48"));
        label_48->setFont(font);

        gridLayout_9->addWidget(label_48, 2, 3, 1, 1);

        cbWmHPosition = new QComboBox(groupBox_9);
        cbWmHPosition->addItem(QString());
        cbWmHPosition->addItem(QString());
        cbWmHPosition->addItem(QString());
        cbWmHPosition->setObjectName(QString::fromUtf8("cbWmHPosition"));
        cbWmHPosition->setFont(font);

        gridLayout_9->addWidget(cbWmHPosition, 0, 0, 1, 3);

        label_49 = new QLabel(groupBox_9);
        label_49->setObjectName(QString::fromUtf8("label_49"));
        label_49->setFont(font);

        gridLayout_9->addWidget(label_49, 2, 0, 1, 2);

        edtWmHorizMargin = new QLineEdit(groupBox_9);
        edtWmHorizMargin->setObjectName(QString::fromUtf8("edtWmHorizMargin"));
        edtWmHorizMargin->setFont(font);

        gridLayout_9->addWidget(edtWmHorizMargin, 2, 2, 1, 1);


        horizontalLayout_4->addWidget(groupBox_9);


        verticalLayout_4->addLayout(horizontalLayout_4);

        widget_2 = new QWidget(gbWatermark);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        sizePolicy3.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy3);
        horizontalLayout_5 = new QHBoxLayout(widget_2);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_25 = new QLabel(widget_2);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        horizontalLayout_5->addWidget(label_25);

        sbWmShadowHoriz = new QSpinBox(widget_2);
        sbWmShadowHoriz->setObjectName(QString::fromUtf8("sbWmShadowHoriz"));
        sizePolicy3.setHeightForWidth(sbWmShadowHoriz->sizePolicy().hasHeightForWidth());
        sbWmShadowHoriz->setSizePolicy(sizePolicy3);
        sbWmShadowHoriz->setFont(font);

        horizontalLayout_5->addWidget(sbWmShadowHoriz);

        label_40 = new QLabel(widget_2);
        label_40->setObjectName(QString::fromUtf8("label_40"));

        horizontalLayout_5->addWidget(label_40);

        sbWmShadowVert = new QSpinBox(widget_2);
        sbWmShadowVert->setObjectName(QString::fromUtf8("sbWmShadowVert"));
        sizePolicy3.setHeightForWidth(sbWmShadowVert->sizePolicy().hasHeightForWidth());
        sbWmShadowVert->setSizePolicy(sizePolicy3);
        sbWmShadowVert->setFont(font);

        horizontalLayout_5->addWidget(sbWmShadowVert);

        label_41 = new QLabel(widget_2);
        label_41->setObjectName(QString::fromUtf8("label_41"));

        horizontalLayout_5->addWidget(label_41);

        sbWmShadowBlur = new QSpinBox(widget_2);
        sbWmShadowBlur->setObjectName(QString::fromUtf8("sbWmShadowBlur"));
        sizePolicy3.setHeightForWidth(sbWmShadowBlur->sizePolicy().hasHeightForWidth());
        sbWmShadowBlur->setSizePolicy(sizePolicy3);
        sbWmShadowBlur->setFont(font);

        horizontalLayout_5->addWidget(sbWmShadowBlur);

        label_52 = new QLabel(widget_2);
        label_52->setObjectName(QString::fromUtf8("label_52"));
        label_52->setFont(font);

        horizontalLayout_5->addWidget(label_52);

        btnWmShadowColor = new QToolButton(widget_2);
        btnWmShadowColor->setObjectName(QString::fromUtf8("btnWmShadowColor"));
        btnWmShadowColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#ddd}"));

        horizontalLayout_5->addWidget(btnWmShadowColor);

        horizontalSpacer = new QSpacerItem(107, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);


        verticalLayout_4->addWidget(widget_2);


        horizontalLayout_3->addLayout(verticalLayout_4);


        verticalLayout_3->addWidget(gbWatermark);


        verticalLayout_1->addLayout(verticalLayout_3);

        groupBox_8 = new QGroupBox(tabImages);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        groupBox_8->setFont(font1);
        gridLayout_7 = new QGridLayout(groupBox_8);
        gridLayout_7->setSpacing(6);
        gridLayout_7->setContentsMargins(11, 11, 11, 11);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        edtWmColor = new QLineEdit(groupBox_8);
        edtWmColor->setObjectName(QString::fromUtf8("edtWmColor"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(edtWmColor->sizePolicy().hasHeightForWidth());
        edtWmColor->setSizePolicy(sizePolicy4);
        edtWmColor->setFont(font);

        gridLayout_7->addWidget(edtWmColor, 0, 6, 1, 1);

        btnWmColor = new QToolButton(groupBox_8);
        btnWmColor->setObjectName(QString::fromUtf8("btnWmColor"));
        btnWmColor->setStyleSheet(QString::fromUtf8("QToolButton {background-color:#fff}"));

        gridLayout_7->addWidget(btnWmColor, 0, 7, 1, 1);

        btnSelectWmFont = new QToolButton(groupBox_8);
        btnSelectWmFont->setObjectName(QString::fromUtf8("btnSelectWmFont"));

        gridLayout_7->addWidget(btnSelectWmFont, 0, 0, 1, 1);

        label_29 = new QLabel(groupBox_8);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setFont(font);

        gridLayout_7->addWidget(label_29, 0, 5, 1, 1);

        lblWaterMarkFont = new QLabel(groupBox_8);
        lblWaterMarkFont->setObjectName(QString::fromUtf8("lblWaterMarkFont"));
        sizePolicy1.setHeightForWidth(lblWaterMarkFont->sizePolicy().hasHeightForWidth());
        lblWaterMarkFont->setSizePolicy(sizePolicy1);
        lblWaterMarkFont->setFont(font);

        gridLayout_7->addWidget(lblWaterMarkFont, 0, 2, 1, 3);

        lblWmSample = new QLabel(groupBox_8);
        lblWmSample->setObjectName(QString::fromUtf8("lblWmSample"));
        sizePolicy1.setHeightForWidth(lblWmSample->sizePolicy().hasHeightForWidth());
        lblWmSample->setSizePolicy(sizePolicy1);
        lblWmSample->setFont(font);

        gridLayout_7->addWidget(lblWmSample, 2, 0, 1, 8);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_7->addItem(horizontalSpacer_5, 0, 1, 1, 1);


        verticalLayout_1->addWidget(groupBox_8);

        verticalSpacer_4 = new QSpacerItem(20, 295, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_1->addItem(verticalSpacer_4);

        tabFalconG->addTab(tabImages, QString());
        tabEdit = new QWidget();
        tabEdit->setObjectName(QString::fromUtf8("tabEdit"));
        sizePolicy1.setHeightForWidth(tabEdit->sizePolicy().hasHeightForWidth());
        tabEdit->setSizePolicy(sizePolicy1);
        verticalLayout_20 = new QVBoxLayout(tabEdit);
        verticalLayout_20->setSpacing(0);
        verticalLayout_20->setContentsMargins(11, 11, 11, 11);
        verticalLayout_20->setObjectName(QString::fromUtf8("verticalLayout_20"));
        verticalLayout_20->setContentsMargins(0, 0, 0, 0);
        editSplitter = new QSplitter(tabEdit);
        editSplitter->setObjectName(QString::fromUtf8("editSplitter"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(editSplitter->sizePolicy().hasHeightForWidth());
        editSplitter->setSizePolicy(sizePolicy5);
        editSplitter->setOrientation(Qt::Vertical);
        editSplitter->setHandleWidth(10);
        widget_3 = new QWidget(editSplitter);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        sizePolicy5.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy5);
        verticalLayout_19 = new QVBoxLayout(widget_3);
        verticalLayout_19->setSpacing(6);
        verticalLayout_19->setContentsMargins(11, 11, 11, 11);
        verticalLayout_19->setObjectName(QString::fromUtf8("verticalLayout_19"));
        verticalLayout_19->setContentsMargins(9, 0, 9, 0);
        splitter = new QSplitter(widget_3);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        widget_4 = new QWidget(splitter);
        widget_4->setObjectName(QString::fromUtf8("widget_4"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(widget_4->sizePolicy().hasHeightForWidth());
        widget_4->setSizePolicy(sizePolicy6);
        verticalLayout_9 = new QVBoxLayout(widget_4);
        verticalLayout_9->setSpacing(6);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(1, 1, 1, 1);
        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setSpacing(6);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        label_51 = new QLabel(widget_4);
        label_51->setObjectName(QString::fromUtf8("label_51"));
        sizePolicy1.setHeightForWidth(label_51->sizePolicy().hasHeightForWidth());
        label_51->setSizePolicy(sizePolicy1);
        label_51->setFont(font1);

        verticalLayout_10->addWidget(label_51);

        trvAlbums = new QTreeView(widget_4);
        trvAlbums->setObjectName(QString::fromUtf8("trvAlbums"));

        verticalLayout_10->addWidget(trvAlbums);

        lblTotalCount = new QLabel(widget_4);
        lblTotalCount->setObjectName(QString::fromUtf8("lblTotalCount"));

        verticalLayout_10->addWidget(lblTotalCount);


        verticalLayout_9->addLayout(verticalLayout_10);

        splitter->addWidget(widget_4);
        widget_6 = new QWidget(splitter);
        widget_6->setObjectName(QString::fromUtf8("widget_6"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy7.setHorizontalStretch(1);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(widget_6->sizePolicy().hasHeightForWidth());
        widget_6->setSizePolicy(sizePolicy7);
        verticalLayout_11 = new QVBoxLayout(widget_6);
        verticalLayout_11->setSpacing(6);
        verticalLayout_11->setContentsMargins(11, 11, 11, 11);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        verticalLayout_11->setContentsMargins(1, 1, 1, 1);
        verticalLayout_12 = new QVBoxLayout();
        verticalLayout_12->setSpacing(6);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        label_53 = new QLabel(widget_6);
        label_53->setObjectName(QString::fromUtf8("label_53"));
        sizePolicy1.setHeightForWidth(label_53->sizePolicy().hasHeightForWidth());
        label_53->setSizePolicy(sizePolicy1);
        label_53->setFont(font1);

        verticalLayout_12->addWidget(label_53);

        tnvImages = new ThumbnailWidget(widget_6);
        tnvImages->setObjectName(QString::fromUtf8("tnvImages"));
        tnvImages->setViewMode(QListView::IconMode);

        verticalLayout_12->addWidget(tnvImages);

        lblImageCount = new QLabel(widget_6);
        lblImageCount->setObjectName(QString::fromUtf8("lblImageCount"));
        lblImageCount->setAlignment(Qt::AlignCenter);

        verticalLayout_12->addWidget(lblImageCount);


        verticalLayout_11->addLayout(verticalLayout_12);

        splitter->addWidget(widget_6);

        verticalLayout_19->addWidget(splitter);

        editSplitter->addWidget(widget_3);
        widget_7 = new QWidget(editSplitter);
        widget_7->setObjectName(QString::fromUtf8("widget_7"));
        sizePolicy5.setHeightForWidth(widget_7->sizePolicy().hasHeightForWidth());
        widget_7->setSizePolicy(sizePolicy5);
        verticalLayout_18 = new QVBoxLayout(widget_7);
        verticalLayout_18->setSpacing(6);
        verticalLayout_18->setContentsMargins(11, 11, 11, 11);
        verticalLayout_18->setObjectName(QString::fromUtf8("verticalLayout_18"));
        verticalLayout_18->setContentsMargins(9, 0, 9, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        cmbBaseLanguage = new QComboBox(widget_7);
        cmbBaseLanguage->setObjectName(QString::fromUtf8("cmbBaseLanguage"));
        sizePolicy2.setHeightForWidth(cmbBaseLanguage->sizePolicy().hasHeightForWidth());
        cmbBaseLanguage->setSizePolicy(sizePolicy2);

        verticalLayout_5->addWidget(cmbBaseLanguage);

        label_54 = new QLabel(widget_7);
        label_54->setObjectName(QString::fromUtf8("label_54"));
        sizePolicy.setHeightForWidth(label_54->sizePolicy().hasHeightForWidth());
        label_54->setSizePolicy(sizePolicy);
        label_54->setFont(font1);

        verticalLayout_5->addWidget(label_54);

        edtBaseTitle = new QTextEdit(widget_7);
        edtBaseTitle->setObjectName(QString::fromUtf8("edtBaseTitle"));
        edtBaseTitle->setEnabled(true);
        edtBaseTitle->setStyleSheet(QString::fromUtf8("QtextEdit { color: gray; background-color:light-grey}"));
        edtBaseTitle->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_5->addWidget(edtBaseTitle);


        horizontalLayout_6->addLayout(verticalLayout_5);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(1);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        cmbLanguage = new QComboBox(widget_7);
        cmbLanguage->setObjectName(QString::fromUtf8("cmbLanguage"));
        sizePolicy2.setHeightForWidth(cmbLanguage->sizePolicy().hasHeightForWidth());
        cmbLanguage->setSizePolicy(sizePolicy2);

        verticalLayout_6->addWidget(cmbLanguage);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_10 = new QLabel(widget_7);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setFont(font1);

        horizontalLayout_7->addWidget(label_10);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_3);

        btnSaveChangedTitle = new QPushButton(widget_7);
        btnSaveChangedTitle->setObjectName(QString::fromUtf8("btnSaveChangedTitle"));
        btnSaveChangedTitle->setEnabled(false);

        horizontalLayout_7->addWidget(btnSaveChangedTitle);

        chkChangeTitleEverywhere = new QCheckBox(widget_7);
        chkChangeTitleEverywhere->setObjectName(QString::fromUtf8("chkChangeTitleEverywhere"));

        horizontalLayout_7->addWidget(chkChangeTitleEverywhere);


        verticalLayout_6->addLayout(horizontalLayout_7);

        edtTitleText = new QTextEdit(widget_7);
        edtTitleText->setObjectName(QString::fromUtf8("edtTitleText"));

        verticalLayout_6->addWidget(edtTitleText);


        horizontalLayout_6->addLayout(verticalLayout_6);


        verticalLayout_18->addLayout(horizontalLayout_6);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout_7->setContentsMargins(-1, 6, -1, -1);
        label_11 = new QLabel(widget_7);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        sizePolicy.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy);
        label_11->setFont(font1);

        verticalLayout_7->addWidget(label_11);

        edtBaseDescription = new QTextEdit(widget_7);
        edtBaseDescription->setObjectName(QString::fromUtf8("edtBaseDescription"));
        edtBaseDescription->setEnabled(true);
        edtBaseDescription->setStyleSheet(QString::fromUtf8("QtextEdit { color: gray; background-color:light-grey}"));
        edtBaseDescription->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_7->addWidget(edtBaseDescription);


        horizontalLayout_8->addLayout(verticalLayout_7);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setSpacing(1);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        label_55 = new QLabel(widget_7);
        label_55->setObjectName(QString::fromUtf8("label_55"));
        sizePolicy.setHeightForWidth(label_55->sizePolicy().hasHeightForWidth());
        label_55->setSizePolicy(sizePolicy);
        label_55->setFont(font1);

        horizontalLayout_9->addWidget(label_55);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_8);

        btnSaveChangedDescription = new QPushButton(widget_7);
        btnSaveChangedDescription->setObjectName(QString::fromUtf8("btnSaveChangedDescription"));
        btnSaveChangedDescription->setEnabled(false);

        horizontalLayout_9->addWidget(btnSaveChangedDescription);

        chkChangeDescriptionEverywhere = new QCheckBox(widget_7);
        chkChangeDescriptionEverywhere->setObjectName(QString::fromUtf8("chkChangeDescriptionEverywhere"));

        horizontalLayout_9->addWidget(chkChangeDescriptionEverywhere);


        verticalLayout_8->addLayout(horizontalLayout_9);

        edtDescriptionText = new QTextEdit(widget_7);
        edtDescriptionText->setObjectName(QString::fromUtf8("edtDescriptionText"));

        verticalLayout_8->addWidget(edtDescriptionText);


        horizontalLayout_8->addLayout(verticalLayout_8);


        verticalLayout_18->addLayout(horizontalLayout_8);

        editSplitter->addWidget(widget_7);

        verticalLayout_20->addWidget(editSplitter);

        tabFalconG->addTab(tabEdit, QString());

        gridLayout_11->addWidget(tabFalconG, 0, 0, 1, 1);

        falconGClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(falconGClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        falconGClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
#ifndef QT_NO_SHORTCUT
        label_17->setBuddy(edtDestGallery);
        label_19->setBuddy(edtServerAddress);
        label_20->setBuddy(edtGalleryRoot);
        label_21->setBuddy(edtMainPage);
        label_42->setBuddy(edtEmailTo);
        label_30->setBuddy(sbNewDays);
        label_9->setBuddy(edtTrackingCode);
        label_23->setBuddy(edtThumb);
        label_22->setBuddy(edtAlbumDir);
        label_44->setBuddy(edtFontDir);
        label_46->setBuddy(edtDefaultFonts);
        label_45->setBuddy(edtBaseName);
        label_56->setBuddy(edtThumb);
        label_33->setBuddy(edtBackgroundColor);
        label_35->setBuddy(chkBackgroundOpacity);
        label_34->setBuddy(chkTextOpacity);
        label_26->setBuddy(edtTextColor);
        label_37->setBuddy(sbShadowHoriz);
        label_38->setBuddy(sbShadowVert);
        label_39->setBuddy(sbShadowBlur);
        label_27->setBuddy(edtFontFamily);
        label_28->setBuddy(cbPointSize);
        label_14->setBuddy(sbBorderWidth);
        label_12->setBuddy(sbWmOpacity);
        label_13->setBuddy(edtWatermark);
        label_6->setBuddy(edtWmVertMargin);
        label_25->setBuddy(sbWmShadowHoriz);
        label_40->setBuddy(sbWmShadowVert);
        label_41->setBuddy(sbWmShadowBlur);
        label_29->setBuddy(edtWmColor);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(btnSourceHistory, edtSourceGallery);
        QWidget::setTabOrder(edtSourceGallery, btnBrowseSource);
        QWidget::setTabOrder(btnBrowseSource, chkSourceRelativePerSign);
        QWidget::setTabOrder(chkSourceRelativePerSign, edtDestGallery);
        QWidget::setTabOrder(edtDestGallery, btnBrowseDestination);
        QWidget::setTabOrder(btnBrowseDestination, edtServerAddress);
        QWidget::setTabOrder(edtServerAddress, edtGalleryRoot);
        QWidget::setTabOrder(edtGalleryRoot, edtUplink);
        QWidget::setTabOrder(edtUplink, edtMainPage);
        QWidget::setTabOrder(edtMainPage, edtGalleryTitle);
        QWidget::setTabOrder(edtGalleryTitle, edtEmailTo);
        QWidget::setTabOrder(edtEmailTo, edtAbout);
        QWidget::setTabOrder(edtAbout, edtBackImage);
        QWidget::setTabOrder(edtBackImage, chkForceSSL);
        QWidget::setTabOrder(chkForceSSL, chkMenuToContact);
        QWidget::setTabOrder(chkMenuToContact, chkAllowLinkToImages);
        QWidget::setTabOrder(chkAllowLinkToImages, chkMenuToAbout);
        QWidget::setTabOrder(chkMenuToAbout, chkCanDownload);
        QWidget::setTabOrder(chkCanDownload, chkMenuToDescriptions);
        QWidget::setTabOrder(chkMenuToDescriptions, chkRightClickProtected);
        QWidget::setTabOrder(chkRightClickProtected, chkMenuToToggleCaptions);
        QWidget::setTabOrder(chkMenuToToggleCaptions, chkFacebook);
        QWidget::setTabOrder(chkFacebook, sbNewDays);
        QWidget::setTabOrder(sbNewDays, sbLatestCount);
        QWidget::setTabOrder(sbLatestCount, chkSetLatest);
        QWidget::setTabOrder(chkSetLatest, chkUseGoogleAnalytics);
        QWidget::setTabOrder(chkUseGoogleAnalytics, edtThumb);
        QWidget::setTabOrder(edtThumb, edtAlbumDir);
        QWidget::setTabOrder(edtAlbumDir, edtFontDir);
        QWidget::setTabOrder(edtFontDir, edtDefaultFonts);
        QWidget::setTabOrder(edtDefaultFonts, edtBaseName);
        QWidget::setTabOrder(edtBaseName, chkReadFromGallery);
        QWidget::setTabOrder(chkReadFromGallery, chkAddTitlesToAll);
        QWidget::setTabOrder(chkAddTitlesToAll, chkOvrImages);
        QWidget::setTabOrder(chkOvrImages, chkAddDescToAll);
        QWidget::setTabOrder(chkAddDescToAll, btnGenerate);
        QWidget::setTabOrder(btnGenerate, btnPreview);
        QWidget::setTabOrder(btnPreview, btnSaveConfig);
        QWidget::setTabOrder(btnSaveConfig, btnExit);
        QWidget::setTabOrder(btnExit, chkUseWM);
        QWidget::setTabOrder(chkUseWM, sbWmOpacity);
        QWidget::setTabOrder(sbWmOpacity, edtWatermark);
        QWidget::setTabOrder(edtWatermark, cbWmVPosition);
        QWidget::setTabOrder(cbWmVPosition, edtWmVertMargin);
        QWidget::setTabOrder(edtWmVertMargin, cbWmHPosition);
        QWidget::setTabOrder(cbWmHPosition, edtWmHorizMargin);
        QWidget::setTabOrder(edtWmHorizMargin, sbWmShadowHoriz);
        QWidget::setTabOrder(sbWmShadowHoriz, sbWmShadowVert);
        QWidget::setTabOrder(sbWmShadowVert, sbWmShadowBlur);
        QWidget::setTabOrder(sbWmShadowBlur, btnWmShadowColor);
        QWidget::setTabOrder(btnWmShadowColor, btnSelectWmFont);
        QWidget::setTabOrder(btnSelectWmFont, edtWmColor);
        QWidget::setTabOrder(edtWmColor, btnWmColor);
        QWidget::setTabOrder(btnWmColor, tabFalconG);

        retranslateUi(falconGClass);
        QObject::connect(actionExit, SIGNAL(triggered()), falconGClass, SLOT(close()));
        QObject::connect(btnExit, SIGNAL(clicked()), falconGClass, SLOT(close()));

        tabFalconG->setCurrentIndex(0);
        cbWmVPosition->setCurrentIndex(0);
        cbWmHPosition->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(falconGClass);
    } // setupUi

    void retranslateUi(QMainWindow *falconGClass)
    {
        falconGClass->setWindowTitle(QApplication::translate("falconGClass", "Falcon's gallery Generator", nullptr));
        actionOpen->setText(QApplication::translate("falconGClass", "&Open", nullptr));
        actionExit->setText(QApplication::translate("falconGClass", "E&xit", nullptr));
#ifndef QT_NO_SHORTCUT
        actionExit->setShortcut(QApplication::translate("falconGClass", "Alt+X", nullptr));
#endif // QT_NO_SHORTCUT
        btnGenerate->setText(QApplication::translate("falconGClass", "Generate (F9)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnGenerate->setShortcut(QApplication::translate("falconGClass", "F9", nullptr));
#endif // QT_NO_SHORTCUT
#ifndef QT_NO_TOOLTIP
        chkGenerateAll->setToolTip(QApplication::translate("falconGClass", "Re-generate all images and albums\n"
"regardless if they were already OK.", nullptr));
#endif // QT_NO_TOOLTIP
        chkGenerateAll->setText(QApplication::translate("falconGClass", "&all", nullptr));
        chkButImages->setText(QApplication::translate("falconGClass", "but images", nullptr));
        btnPreview->setText(QApplication::translate("falconGClass", "Preview (F2)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnPreview->setShortcut(QApplication::translate("falconGClass", "F2", nullptr));
#endif // QT_NO_SHORTCUT
        btnSaveStyleSheet->setText(QApplication::translate("falconGClass", "Save CSS", nullptr));
        btnSaveConfig->setText(QApplication::translate("falconGClass", "Save con&fig.", nullptr));
#ifndef QT_NO_SHORTCUT
        btnSaveConfig->setShortcut(QApplication::translate("falconGClass", "Ctrl+S", nullptr));
#endif // QT_NO_SHORTCUT
        btnExit->setText(QApplication::translate("falconGClass", "E&xit (Alt+X)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnExit->setShortcut(QApplication::translate("falconGClass", "Alt+X", nullptr));
#endif // QT_NO_SHORTCUT
        gbLocalMachine->setTitle(QApplication::translate("falconGClass", "Local machine", nullptr));
        chkSourceRelativePerSign->setText(QApplication::translate("falconGClass", "Paths starting with '/' are under source gallery", nullptr));
        edtDestGallery->setPlaceholderText(QApplication::translate("falconGClass", "( any folder on your machine other the source folder or its subfolders)", nullptr));
        edtSourceGallery->setPlaceholderText(QApplication::translate("falconGClass", "( directory for the hierarchy descriptions 'gallery.struct' or a 'jalbum' source gallery )", nullptr));
        btnBrowseSource->setText(QApplication::translate("falconGClass", "...", nullptr));
        label_17->setText(QApplication::translate("falconGClass", "Generate gallery into", nullptr));
        btnBrowseDestination->setText(QApplication::translate("falconGClass", "...", nullptr));
#ifndef QT_NO_TOOLTIP
        btnSourceHistory->setToolTip(QApplication::translate("falconGClass", "Click for history!", nullptr));
#endif // QT_NO_TOOLTIP
        btnSourceHistory->setText(QApplication::translate("falconGClass", "Source of Ga&llery", nullptr));
        gbServer->setTitle(QApplication::translate("falconGClass", "Gallery on Server", nullptr));
        label_19->setText(QApplication::translate("falconGClass", "Ser&ver address", nullptr));
        edtServerAddress->setPlaceholderText(QApplication::translate("falconGClass", "your.site.com (Setting the http:// or https://  prefix here overrides  'Force secure (HTTPS) access' below)", nullptr));
        label_20->setText(QApplication::translate("falconGClass", "Gallery root directory", nullptr));
#ifndef QT_NO_TOOLTIP
        edtGalleryRoot->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Any path entered into this field will be relative to the web root of the server, which on linux (or Unix) servers is usually 'public_html' in your home directory. This will be the directory of the top level Web page(s).  All of falconG's other files and directories will be inside this directory.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtGalleryRoot->setPlaceholderText(QApplication::translate("falconGClass", "( path of gallery root  on your site,. default: empty)", nullptr));
        label_5->setText(QApplication::translate("falconGClass", "Embed into page:", nullptr));
#ifndef QT_NO_TOOLTIP
        edtUplink->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>All sub-galleries of the main gallery has an uplink button (<span style=\" font-weight:600;\">up arrow</span>) and a <span style=\" font-weight:600;\">Home</span> button. The <span style=\" font-weight:600;\">Home</span> button takes the user to the home page on the site. This field either contains the link to the home page or it is left empty. </p><p>If left empty then the top level <span style=\" font-style:italic;\">Main gallery page </span>is used. That defaults to <span style=\" font-weight:600;\">index.html</span>. (see the hint for that field for details.)</p><p>If you want to embed your gallery into another WEB page then set this as the address of that page.</p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtUplink->setPlaceholderText(QApplication::translate("falconGClass", "(Empty (default) or the page to use when the Home menu is clicked.)", nullptr));
        label_21->setText(QApplication::translate("falconGClass", "Main gallery page      ", nullptr));
#ifndef QT_NO_TOOLTIP
        edtMainPage->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>This field sets the base name and extension of the home page and of the top level gallery page(s). Unless the field <span style=\" font-style:italic;\">Embed into page</span> is set the target for the <span style=\" font-weight:600;\">Home</span> button will be the page the name of which is set here. </p><p>As many top level gallery pages are generated into the root, directory of the gallery as there are languages. For more than one language they will be named as <span style=\" font-style:italic;\">&lt;base name&gt;_&lt;language abbrev&gt;.&lt;extension&gt;</span>. Example (default name,two languages: English and Hungarian with no embedding page): the two top level pages will be: <span style=\" font-style:italic;\">https://your.site.com/index_en.html</span> and <span style=\" font-style:italic;\">https://your.site.com/index_hu.html</span>. </p><p>For multiple language sites additionaly a very basic home page with this name is also generated. However when a file with the same name as of th"
                        "e embedding page the name of the generated home page will be preceeded by an underscore. </p><p>If this field is left empty the name <span style=\" font-weight:600; font-style:italic;\">index.html</span>'  is used, unless an embedding page with the same name exists, in which case thiis name is preceeded by and underscore. </p><p>Example #1: no name is set here and the name of the embedding page is set to <span style=\" font-style:italic;\">index.html</span>. The generated home page will be <span style=\" font-style:italic;\">_index.html.<br/>Example </span>#2: no name is set here and the name of the embedding page is empty.The name of the generated home page will be <span style=\" font-style:italic;\">index.html.</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtMainPage->setText(QString());
        edtMainPage->setPlaceholderText(QApplication::translate("falconGClass", "index.html         ( may set a root path or may add the extension too here. See tool tip for details)", nullptr));
        gbGallery->setTitle(QApplication::translate("falconGClass", "Gallery", nullptr));
        label_36->setText(QApplication::translate("falconGClass", "About page", nullptr));
        label_47->setText(QApplication::translate("falconGClass", "Title", nullptr));
#ifndef QT_NO_TOOLTIP
        edtGalleryTitle->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Series of language definition records, separated by commas.</p><p>Records are 3 comma separated fields. <br/>Field #1 -  2 lower case letter abbreviation (e.g. <span style=\" font-style:italic;\">en</span>)<br/>Field #2 -  Name to put to language selection menu (e.g. <span style=\" font-style:italic;\">in English</span>)<br/>Field #3 -  name of icon file for graphical image, may be left blank.</p><p>All texts in one language must be put into the file named<br/><span style=\" font-style:italic;\">&lt;abbreviation&gt;.text, </span>example:<span style=\" font-style:italic;\"> en.text.</span></p><p><span style=\" font-style:italic;\">See the User Guide for details of language files</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtGalleryTitle->setPlaceholderText(QApplication::translate("falconGClass", "Andreas Falco Photography", nullptr));
#ifndef QT_NO_TOOLTIP
        edtAbout->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Name of the <span style=\" font-weight:600;\">About</span> page. As many about pages are generated into the root, directory of the gallery as there are languages. For more than one language they will be named as &lt;base name&gt;_&lt;language abbrev&gt;.&lt;extension&gt;.</p><p>Example: default name 'about' is used Example with two languages: English (en) and Hungarian (hu).File names will be <span style=\" font-style:italic;\">abut_en.html</span> and <span style=\" font-style:italic;\">about_hu.html.</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtAbout->setPlaceholderText(QApplication::translate("falconGClass", "about.html", nullptr));
        chkMenuToContact->setText(QApplication::translate("falconGClass", "Menu to 'Contact' page", nullptr));
        chkMenuToDescriptions->setText(QApplication::translate("falconGClass", "Menu to toggle descriptionss", nullptr));
        chkRightClickProtected->setText(QApplication::translate("falconGClass", "Right click protection", nullptr));
        chkMenuToToggleCaptions->setText(QApplication::translate("falconGClass", "Captions", nullptr));
        label_50->setText(QApplication::translate("falconGClass", "Home page background", nullptr));
        chkAllowLinkToImages->setText(QApplication::translate("falconGClass", "Allow  links to full size images", nullptr));
        label_42->setText(QApplication::translate("falconGClass", "Send user emails to", nullptr));
        chkMenuToAbout->setText(QApplication::translate("falconGClass", "Menu to 'About' page", nullptr));
        chkCanDownload->setText(QApplication::translate("falconGClass", "Allow the download of images", nullptr));
        edtEmailTo->setPlaceholderText(QApplication::translate("falconGClass", "Your Name <and@email.address>", nullptr));
#ifndef QT_NO_TOOLTIP
        edtBackImage->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Path name of  an image file, which will be set as the background image for the auto generated home page</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtBackImage->setPlaceholderText(QApplication::translate("falconGClass", "path to image file", nullptr));
        chkForceSSL->setText(QApplication::translate("falconGClass", "Force secure (HTTPS) access", nullptr));
        chkFacebook->setText(QApplication::translate("falconGClass", "Like on Facebook link", nullptr));
        gbLatest->setTitle(QApplication::translate("falconGClass", "Latest uploads", nullptr));
        label_31->setText(QApplication::translate("falconGClass", "days", nullptr));
        label_30->setText(QApplication::translate("falconGClass", "Time period", nullptr));
        label_18->setText(QApplication::translate("falconGClass", "Max. # of images: ", nullptr));
        chkSetLatest->setText(QApplication::translate("falconGClass", "Generate 'Latest uploads' album", nullptr));
        gbGoogleAnalytics->setTitle(QApplication::translate("falconGClass", "Google Analytics", nullptr));
        chkUseGoogleAnalytics->setText(QApplication::translate("falconGClass", "Use", nullptr));
        label_9->setText(QApplication::translate("falconGClass", "Trac&king Code", nullptr));
        gbAdvanced->setTitle(QApplication::translate("falconGClass", "Advanced (you may leave theese alone)", nullptr));
        label_23->setText(QApplication::translate("falconGClass", "Thumbnails into", nullptr));
#ifndef QT_NO_TOOLTIP
        edtThumb->setToolTip(QApplication::translate("falconGClass", "Directory for all images. \n"
"Iinside gallery root if no absolute or relative path is given.", nullptr));
#endif // QT_NO_TOOLTIP
        edtThumb->setPlaceholderText(QApplication::translate("falconGClass", "thumb", nullptr));
        label_22->setText(QApplication::translate("falconGClass", "Album directory ", nullptr));
        edtAlbumDir->setPlaceholderText(QApplication::translate("falconGClass", "albums", nullptr));
        label_44->setText(QApplication::translate("falconGClass", "Fonts directory", nullptr));
#ifndef QT_NO_TOOLTIP
        edtFontDir->setToolTip(QApplication::translate("falconGClass", "Directory/directories for all special fonts. \n"
"Can be a WEB address too. \n"
"Will be put in stylesa.css", nullptr));
#endif // QT_NO_TOOLTIP
        edtFontDir->setPlaceholderText(QApplication::translate("falconGClass", "fonts", nullptr));
        label_46->setText(QApplication::translate("falconGClass", "Default fonts", nullptr));
#ifndef QT_NO_TOOLTIP
        edtDefaultFonts->setToolTip(QApplication::translate("falconGClass", "Font name separated by commas. If a name contains\n"
"spaces, put it into double quotes. No need to quote\n"
"other names.\n"
"Defaults:\n"
"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif", nullptr));
#endif // QT_NO_TOOLTIP
        edtDefaultFonts->setPlaceholderText(QApplication::translate("falconGClass", "(names separated by commas)", nullptr));
        label_45->setText(QApplication::translate("falconGClass", "Base link name", nullptr));
#ifndef QT_NO_TOOLTIP
        edtBaseName->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>All albums are referred as &lt;base link name&gt;&lt;index number&gt;</p><p>not by their albuml name, which may contain any UTF-8 characters.</p><p>Example: the album <span style=\" font-style:italic;\">'\305\220rs\303\251g&quot;</span> may be referred as<span style=\" font-style:italic;\"> album123456.</span></p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtBaseName->setPlaceholderText(QApplication::translate("falconGClass", "album", nullptr));
        label_56->setText(QApplication::translate("falconGClass", "Image directory", nullptr));
#ifndef QT_NO_TOOLTIP
        edtImg->setToolTip(QApplication::translate("falconGClass", "Directory for all images. \n"
"Iinside gallery root if no absolute or relative path is given.", nullptr));
#endif // QT_NO_TOOLTIP
        edtImg->setPlaceholderText(QApplication::translate("falconGClass", "img", nullptr));
#ifndef QT_NO_TOOLTIP
        chkReadFromGallery->setToolTip(QApplication::translate("falconGClass", "By checking this option and pressing 'Generate' \n"
"you loose all  changes you made to image and album \n"
"titles and descriptions!\n"
"\n"
"This setting will not be saved in the configuration.\n"
"", nullptr));
#endif // QT_NO_TOOLTIP
        chkReadFromGallery->setText(QApplication::translate("falconGClass", "Read From Gallery", nullptr));
        chkOvrImages->setText(QApplication::translate("falconGClass", "Overwrite existing images", nullptr));
        chkAddDescToAll->setText(QApplication::translate("falconGClass", "Add description fields for all images in 'gallery struct'", nullptr));
        chkAddTitlesToAll->setText(QApplication::translate("falconGClass", "Add title fields for all images in 'gallery struct'", nullptr));
        chkreadJalbum->setText(QApplication::translate("falconGClass", "Read JAlbum", nullptr));
        lblProgressTitle->setText(QApplication::translate("falconGClass", "Reading albums and images", nullptr));
        lblProgressDesc->setText(QApplication::translate("falconGClass", "images/albums", nullptr));
        lblRemainingTimeTitle->setText(QApplication::translate("falconGClass", "Elapsed / remaining time:", nullptr));
        lblRemainingTime->setText(QApplication::translate("falconGClass", "99:99 / 99:99", nullptr));
        label_43->setText(QApplication::translate("falconGClass", "Images/sec:", nullptr));
        lblImagesPerSec->setText(QApplication::translate("falconGClass", "999", nullptr));
        progressBar->setFormat(QApplication::translate("falconGClass", "%p%", nullptr));
        lblProgress->setText(QApplication::translate("falconGClass", "0/0", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabGallery), QApplication::translate("falconGClass", "Gallery", nullptr));
        groupBox_4->setTitle(QApplication::translate("falconGClass", "Colors", nullptr));
#ifndef QT_NO_TOOLTIP
        chkBackgroundOpacity->setToolTip(QApplication::translate("falconGClass", "Turn opacit on or off", nullptr));
#endif // QT_NO_TOOLTIP
        chkBackgroundOpacity->setText(QString());
        edtBackgroundColor->setText(QApplication::translate("falconGClass", "a03020", nullptr));
#ifndef QT_NO_TOOLTIP
        chkTextOpacity->setToolTip(QApplication::translate("falconGClass", "Turn opacit on or off", nullptr));
#endif // QT_NO_TOOLTIP
        chkTextOpacity->setText(QString());
        label_33->setText(QApplication::translate("falconGClass", "Bac&kground", nullptr));
        btnColor->setText(QString());
        btnBackground->setText(QString());
        label_35->setText(QApplication::translate("falconGClass", "%", nullptr));
        label_34->setText(QApplication::translate("falconGClass", "%", nullptr));
        label_26->setText(QApplication::translate("falconGClass", "Te&xt", nullptr));
        edtTextColor->setText(QApplication::translate("falconGClass", "ddd", nullptr));
        label_16->setText(QApplication::translate("falconGClass", "#", nullptr));
        label_57->setText(QApplication::translate("falconGClass", "#", nullptr));
        gbTextShadow->setTitle(QApplication::translate("falconGClass", "Text Shadow", nullptr));
        label_37->setText(QApplication::translate("falconGClass", "H:", nullptr));
        label_38->setText(QApplication::translate("falconGClass", "&V:", nullptr));
        label_39->setText(QApplication::translate("falconGClass", "S:", nullptr));
        btnShadowColor->setText(QString());
        chkShadowOn->setText(QApplication::translate("falconGClass", "On", nullptr));
        gbMenu->setTitle(QApplication::translate("falconGClass", "Menu", nullptr));
        label_24->setText(QApplication::translate("falconGClass", "Stop(%)", nullptr));
        label_4->setText(QApplication::translate("falconGClass", "Color", nullptr));
        btnGradStartColor->setText(QString());
        edtGradMiddleColor->setText(QApplication::translate("falconGClass", "890222", nullptr));
        edtGradStartColor->setText(QApplication::translate("falconGClass", "a90329", nullptr));
        chkGradient->setText(QApplication::translate("falconGClass", "Gradient", nullptr));
        btnGradMiddleColor->setText(QString());
        btnGradStopColor->setText(QString());
        btnGradBorder->setText(QApplication::translate("falconGClass", "B", nullptr));
        chkButtonBorder->setText(QApplication::translate("falconGClass", "Border:", nullptr));
        edtGradStopColor->setText(QApplication::translate("falconGClass", "6D0019", nullptr));
        label_58->setText(QApplication::translate("falconGClass", "#", nullptr));
        label_59->setText(QApplication::translate("falconGClass", "#", nullptr));
        label_60->setText(QApplication::translate("falconGClass", "#", nullptr));
        groupBox_5->setTitle(QApplication::translate("falconGClass", "Font", nullptr));
        label_27->setText(QApplication::translate("falconGClass", "Family", nullptr));
        label_28->setText(QApplication::translate("falconGClass", "Point si&ze:", nullptr));
        cbPointSize->setItemText(0, QApplication::translate("falconGClass", "8pt", nullptr));
        cbPointSize->setItemText(1, QApplication::translate("falconGClass", "9pt", nullptr));
        cbPointSize->setItemText(2, QApplication::translate("falconGClass", "10pt", nullptr));
        cbPointSize->setItemText(3, QApplication::translate("falconGClass", "11pt", nullptr));
        cbPointSize->setItemText(4, QApplication::translate("falconGClass", "12pt", nullptr));
        cbPointSize->setItemText(5, QApplication::translate("falconGClass", "14pt", nullptr));
        cbPointSize->setItemText(6, QApplication::translate("falconGClass", "16pt", nullptr));
        cbPointSize->setItemText(7, QApplication::translate("falconGClass", "18pt", nullptr));
        cbPointSize->setItemText(8, QApplication::translate("falconGClass", "20pt", nullptr));
        cbPointSize->setItemText(9, QApplication::translate("falconGClass", "22pt", nullptr));
        cbPointSize->setItemText(10, QApplication::translate("falconGClass", "24pt", nullptr));
        cbPointSize->setItemText(11, QApplication::translate("falconGClass", "26pt", nullptr));
        cbPointSize->setItemText(12, QApplication::translate("falconGClass", "28pt", nullptr));
        cbPointSize->setItemText(13, QApplication::translate("falconGClass", "36pt", nullptr));
        cbPointSize->setItemText(14, QApplication::translate("falconGClass", "48pt", nullptr));
        cbPointSize->setItemText(15, QApplication::translate("falconGClass", "72pt", nullptr));

        chkBold->setText(QApplication::translate("falconGClass", "Bold", nullptr));
        chkItalic->setText(QApplication::translate("falconGClass", "Italic", nullptr));
        chkUnderline->setText(QApplication::translate("falconGClass", "Underline", nullptr));
        chkStrikethrough->setText(QApplication::translate("falconGClass", "Strikethrough", nullptr));
        gbDecoration->setTitle(QApplication::translate("falconGClass", "Image Decoration", nullptr));
        chkIconTop->setText(QApplication::translate("falconGClass", "Icon to top of page", nullptr));
        chkIconText->setText(QApplication::translate("falconGClass", "Icon to toggle info", nullptr));
        chkImageBorder->setText(QApplication::translate("falconGClass", "Image Border", nullptr));
        btnBorderColor->setText(QString());
        label_14->setText(QApplication::translate("falconGClass", "Width", nullptr));
        label_15->setText(QApplication::translate("falconGClass", "px", nullptr));
        btnDisplayHint->setText(QApplication::translate("falconGClass", "?", nullptr));
        btnResetDesign->setText(QApplication::translate("falconGClass", "Reset", nullptr));
        groupBox_6->setTitle(QApplication::translate("falconGClass", "Sample", nullptr));
        btnGalleryTitle->setText(QApplication::translate("falconGClass", "Gallery title", nullptr));
        btnGalleryDesc->setText(QApplication::translate("falconGClass", "Lorem ipsum dolor sit amet consectetur adipisicing\n"
"elit. Placeat delectusfacilis dolorum dolores\n"
"distinctio...", nullptr));
        btnSection->setText(QApplication::translate("falconGClass", "Images", nullptr));
        btnAlbumDesc->setText(QApplication::translate("falconGClass", "Lorem ipsum dolor sit amet consectetur adipisicing\n"
"elit. Placeat delectus facilis dolorum dolores\n"
"distinctio, aperiam quam deserunt accusantium\n"
"possimus minima?", nullptr));
        btnAlbumTitle->setText(QApplication::translate("falconGClass", "Album Title", nullptr));
        btnImage->setText(QString());
        btnUplink->setText(QString());
        btnMenu->setText(QApplication::translate("falconGClass", "Home", nullptr));
        btnCaption->setText(QApplication::translate("falconGClass", "Captions", nullptr));
        btnSmallGalleryTitle->setText(QApplication::translate("falconGClass", "falconG Gallery Maker", nullptr));
        btnLang->setText(QApplication::translate("falconGClass", "Italiano", nullptr));
        btnIconToTop->setText(QString());
        btnImageTitle->setText(QApplication::translate("falconGClass", "IMAGE TiTLE", nullptr));
        btnDescription->setText(QString());
        btnImageDesc->setText(QApplication::translate("falconGClass", "Lorem ipsum dolor sit amet consectetur adipisicing\n"
"elit. Placeat delectus facilis dolorum dolores", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabDesign), QApplication::translate("falconGClass", "Design", nullptr));
        groupBox->setTitle(QApplication::translate("falconGClass", "Resizing", nullptr));
        label_3->setText(QApplication::translate("falconGClass", "Height", nullptr));
        label_2->setText(QApplication::translate("falconGClass", "Width", nullptr));
        label->setText(QApplication::translate("falconGClass", "Image", nullptr));
        chkDoNotEnlarge->setText(QApplication::translate("falconGClass", "Do not enlarge images", nullptr));
        label_7->setText(QApplication::translate("falconGClass", "Thumbnail", nullptr));
        btnLink->setText(QString());
        gbWatermark->setTitle(QApplication::translate("falconGClass", "Watermark", nullptr));
        label_32->setText(QApplication::translate("falconGClass", " %", nullptr));
        label_12->setText(QApplication::translate("falconGClass", "Opacity   ", nullptr));
        chkUseWM->setText(QApplication::translate("falconGClass", "use", nullptr));
        label_13->setText(QApplication::translate("falconGClass", "Te&xt", nullptr));
        groupBox_7->setTitle(QApplication::translate("falconGClass", "Vertical position", nullptr));
        label_8->setText(QApplication::translate("falconGClass", "px", nullptr));
        cbWmVPosition->setItemText(0, QApplication::translate("falconGClass", "Top", nullptr));
        cbWmVPosition->setItemText(1, QApplication::translate("falconGClass", "Center vertically", nullptr));
        cbWmVPosition->setItemText(2, QApplication::translate("falconGClass", "Bottom", nullptr));

        label_6->setText(QApplication::translate("falconGClass", "&Vertical margin:", nullptr));
        groupBox_9->setTitle(QApplication::translate("falconGClass", "Horizontal position", nullptr));
        label_48->setText(QApplication::translate("falconGClass", "px", nullptr));
        cbWmHPosition->setItemText(0, QApplication::translate("falconGClass", "Left", nullptr));
        cbWmHPosition->setItemText(1, QApplication::translate("falconGClass", "Center horizontally", nullptr));
        cbWmHPosition->setItemText(2, QApplication::translate("falconGClass", "Right", nullptr));

        label_49->setText(QApplication::translate("falconGClass", "Horizontal margin", nullptr));
        label_25->setText(QApplication::translate("falconGClass", "Shadow H:", nullptr));
        label_40->setText(QApplication::translate("falconGClass", "V:", nullptr));
        label_41->setText(QApplication::translate("falconGClass", "Blur:", nullptr));
        label_52->setText(QApplication::translate("falconGClass", "Color", nullptr));
        btnWmShadowColor->setText(QString());
        groupBox_8->setTitle(QApplication::translate("falconGClass", "Font", nullptr));
        edtWmColor->setText(QApplication::translate("falconGClass", "FFFFFF", nullptr));
        btnWmColor->setText(QString());
        btnSelectWmFont->setText(QApplication::translate("falconGClass", "Font", nullptr));
        label_29->setText(QApplication::translate("falconGClass", "Color", nullptr));
        lblWaterMarkFont->setText(QString());
        lblWmSample->setText(QApplication::translate("falconGClass", "Watermark text sample", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabImages), QApplication::translate("falconGClass", "Images", nullptr));
        label_51->setText(QApplication::translate("falconGClass", "Albums", nullptr));
        lblTotalCount->setText(QApplication::translate("falconGClass", "...", nullptr));
        label_53->setText(QApplication::translate("falconGClass", "Images", nullptr));
        lblImageCount->setText(QApplication::translate("falconGClass", "...", nullptr));
        label_54->setText(QApplication::translate("falconGClass", "Tiltle (Language #1)", nullptr));
        label_10->setText(QApplication::translate("falconGClass", "Tiltle (Language #2)", nullptr));
#ifndef QT_NO_TOOLTIP
        btnSaveChangedTitle->setToolTip(QApplication::translate("falconGClass", "Save changed title and description", nullptr));
#endif // QT_NO_TOOLTIP
        btnSaveChangedTitle->setText(QApplication::translate("falconGClass", "Replace", nullptr));
        chkChangeTitleEverywhere->setText(QApplication::translate("falconGClass", "Everywhere", nullptr));
        label_11->setText(QApplication::translate("falconGClass", "Description (Language #1)", nullptr));
        label_55->setText(QApplication::translate("falconGClass", "Description (Language #2)", nullptr));
#ifndef QT_NO_TOOLTIP
        btnSaveChangedDescription->setToolTip(QApplication::translate("falconGClass", "Save changed title and description", nullptr));
#endif // QT_NO_TOOLTIP
        btnSaveChangedDescription->setText(QApplication::translate("falconGClass", "Replace", nullptr));
        chkChangeDescriptionEverywhere->setText(QApplication::translate("falconGClass", "Everywhere", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabEdit), QApplication::translate("falconGClass", "Edit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class falconGClass: public Ui_falconGClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FALCONG_H
