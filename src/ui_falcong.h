/********************************************************************************
** Form generated from reading UI file 'falcong.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FALCONG_H
#define UI_FALCONG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWebEngineWidgets/QWebEngineView>
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
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBox>
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
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabFalconG;
    QWidget *tabGallery;
    QVBoxLayout *tabGalleryLayout;
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
    QLabel *label_36;
    QLineEdit *edtDescription;
    QLabel *label_61;
    QLineEdit *edtKeywords;
    QHBoxLayout *horizontalLayout_16;
    QGroupBox *gbGallery;
    QGridLayout *gbGalleryLayout;
    QLineEdit *edtBackImage;
    QCheckBox *chkMenuToAbout;
    QLabel *label_47;
    QCheckBox *chkAllowLinkToImages;
    QLineEdit *edtGalleryLanguages;
    QLabel *label_42;
    QCheckBox *chkMenuToContact;
    QCheckBox *chkCanDownload;
    QCheckBox *chkFacebook;
    QLabel *label_62;
    QCheckBox *chkRightClickProtected;
    QLabel *label_50;
    QLineEdit *edtAbout;
    QCheckBox *chkMenuToDescriptions;
    QLineEdit *edtGalleryTitle;
    QCheckBox *chkForceSSL;
    QLineEdit *edtEmailTo;
    QCheckBox *chkMenuToToggleCaptions;
    QCheckBox *chkSeparateFoldersForLanguages;
    QVBoxLayout *verticalLayout_15;
    QGroupBox *gbLatest;
    QGridLayout *gbLatestLayout;
    QLabel *label_18;
    QLabel *label_30;
    QLabel *label_31;
    QSpinBox *sbLatestCount;
    QSpinBox *sbNewDays;
    QCheckBox *chkSetLatest;
    QGroupBox *gbGoogleAnalytics;
    QGridLayout *gridLayout_2;
    QLabel *label_9;
    QCheckBox *chkUseGoogleAnalytics;
    QLineEdit *edtTrackingCode;
    QSpacerItem *horizontalSpacer_12;
    QGroupBox *gbAdvanced;
    QGridLayout *gridLayout_17;
    QLabel *label_56;
    QLineEdit *edtImg;
    QLabel *label_23;
    QLineEdit *edtThumb;
    QLabel *label_8;
    QLineEdit *edtVid;
    QSpacerItem *horizontalSpacer_15;
    QLabel *label_22;
    QLineEdit *edtAlbumDir;
    QLabel *label_45;
    QLineEdit *edtBaseName;
    QLabel *label_67;
    QWidget *widget_9;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *edtGoogleFonts;
    QToolButton *btnGoToGoogleFontsPage;
    QLabel *label_44;
    QLineEdit *edtFontDir;
    QLabel *label_46;
    QLineEdit *edtDefaultFonts;
    QGridLayout *gridLayout_16;
    QCheckBox *chkReadFromGallery;
    QCheckBox *chkLowerCaseImageExtensions;
    QCheckBox *chkAddTitlesToAll;
    QCheckBox *chkAddDescToAll;
    QCheckBox *chkOvrImages;
    QCheckBox *chkDebugging;
    QCheckBox *chkReadJAlbum;
    QWidget *pnlProgress;
    QGridLayout *gridLayout_18;
    QLabel *lblProgressDesc;
    QLabel *lblProgressTitle;
    QLabel *lblProgress;
    QLabel *lblRemainingTimeTitle;
    QLabel *label_43;
    QLabel *lblRemainingTime;
    QSpacerItem *horizontalSpacer_2;
    QProgressBar *progressBar;
    QLabel *lblImagesPerSec;
    QSpacerItem *verticalSpacer;
    QWidget *tabDesign;
    QHBoxLayout *horizontalLayout_20;
    QWidget *widget_15;
    QHBoxLayout *horizontalLayout_19;
    QSplitter *designSplitter;
    QWidget *widget_16;
    QHBoxLayout *horizontalLayout_10;
    QVBoxLayout *verticalLayout_13;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_64;
    QComboBox *cbActualItem;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QToolBox *toolBox;
    QWidget *pgGlobal;
    QToolButton *btnSelectUplinkIcon;
    QGroupBox *gbGlobalColors;
    QGridLayout *gridLayout;
    QLabel *label_68;
    QToolButton *btnPageColor;
    QLabel *label_65;
    QToolButton *btnPageBackground;
    QCheckBox *chkSetAll;
    QGroupBox *gbBackgroundImage;
    QGridLayout *gridLayout_3;
    QGroupBox *gbBacgroundImagePosition;
    QVBoxLayout *verticalLayout;
    QRadioButton *rbNoBackgroundImage;
    QRadioButton *rbCenterBckImage;
    QRadioButton *rbCoverBckImage;
    QRadioButton *rbTileBckImage;
    QLabel *label_63;
    QLabel *lblbckImage;
    QToolButton *btnOpenBckImage;
    QLineEdit *edtBckImageName;
    QSlider *hsImageSizeToShow;
    QCheckBox *chkIconTop;
    QCheckBox *chkIconText;
    QWidget *pgSelected;
    QGridLayout *gridLayout_4;
    QGroupBox *gbGradient;
    QGridLayout *gridLayout_19;
    QCheckBox *chkUseGradient;
    QLabel *label_4;
    QLabel *label_24;
    QLabel *lblGradient;
    QToolButton *btnGradStartColor;
    QSpinBox *sbGradStartPos;
    QToolButton *btnGradMiddleColor;
    QSpinBox *sbGradMiddlePos;
    QToolButton *btnGradStopColor;
    QSpinBox *sbGradStopPos;
    QSpacerItem *verticalSpacer_3;
    QGroupBox *gbTextStyle;
    QVBoxLayout *verticalLayout_12;
    QWidget *widget_8;
    QHBoxLayout *horizontalLayout;
    QLabel *label_66;
    QRadioButton *rbTextAlignNone;
    QRadioButton *rbTextLeft;
    QRadioButton *rbTextCenter;
    QRadioButton *rbTextRight;
    QWidget *widget_10;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_69;
    QCheckBox *chkTdUnderline;
    QCheckBox *chkTdOverline;
    QCheckBox *chkTdLinethrough;
    QSpacerItem *horizontalSpacer_5;
    QGroupBox *gbDecorationStyle;
    QGridLayout *gridLayout_13;
    QRadioButton *rbTdWavy;
    QRadioButton *rbTdDouble;
    QRadioButton *rbTdSolid;
    QRadioButton *rbTdDotted;
    QRadioButton *rbTdDashed;
    QGroupBox *gbTextShadow;
    QGridLayout *gridLayout_15;
    QGridLayout *gridLayout_14;
    QLabel *label_85;
    QLabel *label_37;
    QSpinBox *sbShadowHoriz1;
    QLabel *label_38;
    QSpinBox *sbShadowVert1;
    QLabel *label_39;
    QSpinBox *sbShadowBlur1;
    QLabel *label_87;
    QSpinBox *sbShadowSpread1;
    QLabel *label_86;
    QLabel *label_83;
    QSpinBox *sbShadowHoriz2;
    QLabel *label_84;
    QSpinBox *sbShadowVert2;
    QLabel *label_82;
    QSpinBox *sbShadowBlur2;
    QLabel *label_88;
    QSpinBox *sbShadowSpread2;
    QRadioButton *rbBoxShadow;
    QCheckBox *chkShadowOn;
    QRadioButton *rbTextShadow;
    QToolButton *btnShadowColor;
    QGroupBox *gbFont;
    QGridLayout *gridLayout_9;
    QLabel *label_27;
    QLineEdit *edtFontFamily;
    QComboBox *cbFonts;
    QLabel *label_28;
    QComboBox *cbPointSize;
    QLabel *label_32;
    QComboBox *cbLineHeight;
    QLabel *label_6;
    QSpinBox *sbSpaceAfter;
    QSpacerItem *horizontalSpacer_13;
    QCheckBox *chkDifferentFirstLine;
    QComboBox *cbPointSizeFirstLine;
    QCheckBox *chkBold;
    QCheckBox *chkItalic;
    QWidget *gbColors;
    QGridLayout *gridLayout_7;
    QSpacerItem *horizontalSpacer_9;
    QLabel *label_26;
    QLabel *label_33;
    QSpinBox *sbBackgroundOpacity;
    QToolButton *btnForeground;
    QCheckBox *chkTextOpacity;
    QCheckBox *chkBackgroundOpacity;
    QToolButton *btnBackground;
    QSpinBox *sbTextOpacity;
    QGroupBox *gbBorder;
    QGridLayout *gridLayout_6;
    QLabel *label_57;
    QComboBox *cbBorderStyle;
    QToolButton *btnBorderColor;
    QLabel *label_58;
    QSpinBox *sbBorderWidth;
    QFrame *line;
    QLabel *label_70;
    QSpinBox *sbBorderRadius;
    QRadioButton *rbLeftBorder;
    QRadioButton *rbBottomBorder;
    QRadioButton *rbRightBorder;
    QRadioButton *rbTopBorder;
    QRadioButton *rbAllBorders;
    QWidget *pgImageDecor;
    QVBoxLayout *verticalLayout_10;
    QWidget *gbResizing;
    QGridLayout *gridLayout_12;
    QSpacerItem *horizontalSpacer_6;
    QLabel *label_2;
    QSpinBox *sbImageMargin;
    QLabel *label_15;
    QCheckBox *chkDistortThumbnails;
    QLabel *label_7;
    QSpinBox *sbImageHeight;
    QSpinBox *sbImageWidth;
    QToolButton *btnLink;
    QLabel *label_73;
    QComboBox *cbImageQuality;
    QLabel *label_59;
    QLabel *label_3;
    QCheckBox *chkCropThumbnails;
    QSpinBox *sbThumbnailHeight;
    QCheckBox *chkDoNotEnlarge;
    QSpinBox *sbThumbnailWidth;
    QLabel *label;
    QSpacerItem *horizontalSpacer_18;
    QGroupBox *gbImageDecoration;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_74;
    QComboBox *cbImageBorderStyle;
    QLabel *label_14;
    QSpinBox *sbImageBorderWidth;
    QLabel *label_75;
    QSpinBox *sbImageBorderRadius;
    QToolButton *btnImageBorderColor;
    QGroupBox *gbImageDecoration_2;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_25;
    QSpinBox *sbImageMatteWidth;
    QLabel *label_76;
    QToolButton *btnImageMatteColor;
    QLabel *label_96;
    QSpinBox *sbImageMatteRadius;
    QSpacerItem *horizontalSpacer_11;
    QGroupBox *gbImageDecoration_3;
    QHBoxLayout *horizontalLayout_24;
    QLabel *label_77;
    QSpinBox *sbAlbumMatteWidth;
    QLabel *label_78;
    QToolButton *btnAlbumMatteColor;
    QLabel *label_97;
    QSpinBox *sbAlbumMatteRadius;
    QSpacerItem *horizontalSpacer_19;
    QSpacerItem *verticalSpacer_2;
    QWidget *pgWatermark;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_3;
    QWidget *gbWatermark;
    QGridLayout *gridLayout_10;
    QLabel *label_34;
    QSpacerItem *horizontalSpacer_10;
    QCheckBox *chkUseWM;
    QLabel *label_29;
    QSpinBox *sbWmOpacity;
    QLabel *label_35;
    QToolButton *btnWmColor;
    QLineEdit *edtWatermark;
    QLabel *lblWmSample;
    QSpacerItem *verticalSpacer_4;
    QGroupBox *gbWatermarkFont;
    QHBoxLayout *horizontalLayout_3;
    QToolButton *btnSelectWmFont;
    QLabel *lblWaterMarkFont;
    QGroupBox *gbPosition;
    QHBoxLayout *horizontalLayout_12;
    QGridLayout *gridLayout_11;
    QLineEdit *edtWmVertMargin;
    QLabel *label_12;
    QLabel *label_13;
    QComboBox *cbWmVPosition;
    QGridLayout *gridLayout_21;
    QComboBox *cbWmHPosition;
    QLabel *label_60;
    QLineEdit *edtWmHorizMargin;
    QLabel *label_71;
    QGroupBox *gbWmTextShadow;
    QGridLayout *gridLayout_8;
    QCheckBox *chkUseWMShadow;
    QSpacerItem *horizontalSpacer;
    QLabel *label_16;
    QSpinBox *sbWmShadowHoriz;
    QLabel *label_40;
    QSpinBox *sbWmShadowVert;
    QLabel *label_41;
    QSpinBox *sbWmShadowBlur;
    QLabel *label_52;
    QToolButton *btnWmShadowColor;
    QLabel *label_48;
    QWidget *widget_5;
    QHBoxLayout *horizontalLayout_11;
    QToolButton *btnDisplayHint;
    QSpacerItem *horizontalSpacer_7;
    QPushButton *btnReload;
    QPushButton *btnResetDesign;
    QGroupBox *gbSample;
    QVBoxLayout *gbSampleLayout;
    QWebEngineView *sample;
    QWidget *tabEdit;
    QVBoxLayout *tabEditLayout;
    QSplitter *editSplitter;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout_19;
    QSplitter *splitter;
    QWidget *widget_4;
    QVBoxLayout *verticalLayout_9;
    QVBoxLayout *editAlbumsLayout;
    QLabel *label_51;
    QTreeView *trvAlbums;
    QLabel *lblTotalCount;
    QWidget *widget_6;
    QVBoxLayout *verticalLayout_11;
    QVBoxLayout *editImagesLayout;
    QLabel *label_53;
    ThumbnailWidget *tnvImages;
    QLabel *lblImageCount;
    QWidget *widget_7;
    QVBoxLayout *verticalLayout_18;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_5;
    QComboBox *cbBaseLanguage;
    QLabel *label_54;
    QTextEdit *edtBaseTitle;
    QVBoxLayout *verticalLayout_6;
    QComboBox *cbLanguage;
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
    QWidget *tabOptions;
    QVBoxLayout *verticalLayout_4;
    QLabel *lblOptionsHelp;
    QWidget *widget_11;
    QVBoxLayout *verticalLayout_14;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_23;
    QLabel *label_79;
    QRadioButton *rbMagyar;
    QRadioButton *rbEnglish;
    QSpacerItem *horizontalSpacer_17;
    QPushButton *btnResetDialogs;
    QGroupBox *gbColorScheme;
    QVBoxLayout *verticalLayout_16;
    QWidget *widget_14;
    QHBoxLayout *horizontalLayout_18;
    QWidget *pnlColorScheme;
    QFormLayout *formLayout_4;
    QLabel *label_49;
    QLabel *label_72;
    QGroupBox *pnlSchemeEmulator;
    QGridLayout *gridLayout_22;
    QSpacerItem *verticalSpacer_7;
    QToolButton *btnDeleteColorScheme;
    QListWidget *lwColorScheme;
    QPushButton *btnAddAndGenerateColorScheme;
    QPushButton *btnApplyColorScheme;
    QPushButton *btnResetColorScheme;
    QPushButton *btnMoveSchemeUp;
    QPushButton *btnMoveSchemeDown;
    QWidget *pnlButtons;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *btnGenerate;
    QCheckBox *chkGenerateAll;
    QCheckBox *chkButImages;
    QPushButton *btnPreview;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *btnSaveStyleSheet;
    QPushButton *btnSaveConfig;
    QCheckBox *chkCleanUp;
    QPushButton *btnExit;

    void setupUi(QMainWindow *falconGClass)
    {
        if (falconGClass->objectName().isEmpty())
            falconGClass->setObjectName(QStringLiteral("falconGClass"));
        falconGClass->setEnabled(true);
        falconGClass->resize(1066, 1029);
        falconGClass->setMinimumSize(QSize(1014, 0));
        falconGClass->setMaximumSize(QSize(16777215, 16777215));
        QFont font;
        font.setFamily(QStringLiteral("Arial"));
        falconGClass->setFont(font);
        falconGClass->setStyleSheet(QStringLiteral(""));
        actionOpen = new QAction(falconGClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionExit = new QAction(falconGClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        centralWidget = new QWidget(falconGClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setMinimumSize(QSize(955, 1029));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(2, 2, 2, 2);
        tabFalconG = new QTabWidget(centralWidget);
        tabFalconG->setObjectName(QStringLiteral("tabFalconG"));
        tabFalconG->setMinimumSize(QSize(0, 0));
        QFont font1;
        font1.setFamily(QStringLiteral("Arial"));
        font1.setBold(false);
        font1.setWeight(50);
        tabFalconG->setFont(font1);
        tabGallery = new QWidget();
        tabGallery->setObjectName(QStringLiteral("tabGallery"));
        tabGalleryLayout = new QVBoxLayout(tabGallery);
        tabGalleryLayout->setSpacing(2);
        tabGalleryLayout->setContentsMargins(11, 11, 11, 11);
        tabGalleryLayout->setObjectName(QStringLiteral("tabGalleryLayout"));
        tabGalleryLayout->setContentsMargins(2, 2, 2, 2);
        gbLocalMachine = new QGroupBox(tabGallery);
        gbLocalMachine->setObjectName(QStringLiteral("gbLocalMachine"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(gbLocalMachine->sizePolicy().hasHeightForWidth());
        gbLocalMachine->setSizePolicy(sizePolicy);
        QFont font2;
        font2.setFamily(QStringLiteral("Arial"));
        font2.setBold(true);
        font2.setWeight(75);
        gbLocalMachine->setFont(font2);
        gridLayout_5 = new QGridLayout(gbLocalMachine);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        chkSourceRelativePerSign = new QCheckBox(gbLocalMachine);
        chkSourceRelativePerSign->setObjectName(QStringLiteral("chkSourceRelativePerSign"));
        chkSourceRelativePerSign->setFont(font1);
        chkSourceRelativePerSign->setCheckable(true);
        chkSourceRelativePerSign->setChecked(true);

        gridLayout_5->addWidget(chkSourceRelativePerSign, 1, 0, 1, 3);

        edtDestGallery = new QLineEdit(gbLocalMachine);
        edtDestGallery->setObjectName(QStringLiteral("edtDestGallery"));
        edtDestGallery->setFont(font1);

        gridLayout_5->addWidget(edtDestGallery, 2, 2, 1, 1);

        edtSourceGallery = new QLineEdit(gbLocalMachine);
        edtSourceGallery->setObjectName(QStringLiteral("edtSourceGallery"));
        edtSourceGallery->setFont(font1);

        gridLayout_5->addWidget(edtSourceGallery, 0, 2, 1, 1);

        btnBrowseSource = new QToolButton(gbLocalMachine);
        btnBrowseSource->setObjectName(QStringLiteral("btnBrowseSource"));
        btnBrowseSource->setFont(font1);

        gridLayout_5->addWidget(btnBrowseSource, 0, 3, 1, 1);

        label_17 = new QLabel(gbLocalMachine);
        label_17->setObjectName(QStringLiteral("label_17"));
        label_17->setFont(font1);

        gridLayout_5->addWidget(label_17, 2, 0, 1, 1);

        btnBrowseDestination = new QToolButton(gbLocalMachine);
        btnBrowseDestination->setObjectName(QStringLiteral("btnBrowseDestination"));
        btnBrowseDestination->setFont(font1);

        gridLayout_5->addWidget(btnBrowseDestination, 2, 3, 1, 1);

        btnSourceHistory = new QPushButton(gbLocalMachine);
        btnSourceHistory->setObjectName(QStringLiteral("btnSourceHistory"));
        btnSourceHistory->setFont(font1);
        btnSourceHistory->setFlat(false);

        gridLayout_5->addWidget(btnSourceHistory, 0, 0, 1, 1);


        tabGalleryLayout->addWidget(gbLocalMachine);

        gbServer = new QGroupBox(tabGallery);
        gbServer->setObjectName(QStringLiteral("gbServer"));
        sizePolicy.setHeightForWidth(gbServer->sizePolicy().hasHeightForWidth());
        gbServer->setSizePolicy(sizePolicy);
        gbServer->setFont(font2);
        formLayout = new QFormLayout(gbServer);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label_19 = new QLabel(gbServer);
        label_19->setObjectName(QStringLiteral("label_19"));
        label_19->setFont(font1);

        formLayout->setWidget(0, QFormLayout::LabelRole, label_19);

        edtServerAddress = new QLineEdit(gbServer);
        edtServerAddress->setObjectName(QStringLiteral("edtServerAddress"));
        edtServerAddress->setFont(font1);

        formLayout->setWidget(0, QFormLayout::FieldRole, edtServerAddress);

        label_20 = new QLabel(gbServer);
        label_20->setObjectName(QStringLiteral("label_20"));
        label_20->setFont(font1);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_20);

        edtGalleryRoot = new QLineEdit(gbServer);
        edtGalleryRoot->setObjectName(QStringLiteral("edtGalleryRoot"));
        edtGalleryRoot->setFont(font1);

        formLayout->setWidget(1, QFormLayout::FieldRole, edtGalleryRoot);

        label_5 = new QLabel(gbServer);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setFont(font1);

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        edtUplink = new QLineEdit(gbServer);
        edtUplink->setObjectName(QStringLiteral("edtUplink"));
        edtUplink->setFont(font1);

        formLayout->setWidget(2, QFormLayout::FieldRole, edtUplink);

        label_21 = new QLabel(gbServer);
        label_21->setObjectName(QStringLiteral("label_21"));
        label_21->setFont(font1);

        formLayout->setWidget(3, QFormLayout::LabelRole, label_21);

        edtMainPage = new QLineEdit(gbServer);
        edtMainPage->setObjectName(QStringLiteral("edtMainPage"));
        edtMainPage->setFont(font1);

        formLayout->setWidget(3, QFormLayout::FieldRole, edtMainPage);

        label_36 = new QLabel(gbServer);
        label_36->setObjectName(QStringLiteral("label_36"));
        label_36->setFont(font1);

        formLayout->setWidget(4, QFormLayout::LabelRole, label_36);

        edtDescription = new QLineEdit(gbServer);
        edtDescription->setObjectName(QStringLiteral("edtDescription"));
        edtDescription->setFont(font1);

        formLayout->setWidget(4, QFormLayout::FieldRole, edtDescription);

        label_61 = new QLabel(gbServer);
        label_61->setObjectName(QStringLiteral("label_61"));
        label_61->setFont(font1);

        formLayout->setWidget(5, QFormLayout::LabelRole, label_61);

        edtKeywords = new QLineEdit(gbServer);
        edtKeywords->setObjectName(QStringLiteral("edtKeywords"));
        edtKeywords->setFont(font1);

        formLayout->setWidget(5, QFormLayout::FieldRole, edtKeywords);


        tabGalleryLayout->addWidget(gbServer);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setSpacing(6);
        horizontalLayout_16->setObjectName(QStringLiteral("horizontalLayout_16"));
        gbGallery = new QGroupBox(tabGallery);
        gbGallery->setObjectName(QStringLiteral("gbGallery"));
        sizePolicy.setHeightForWidth(gbGallery->sizePolicy().hasHeightForWidth());
        gbGallery->setSizePolicy(sizePolicy);
        gbGallery->setMinimumSize(QSize(0, 350));
        gbGallery->setFont(font2);
        gbGalleryLayout = new QGridLayout(gbGallery);
        gbGalleryLayout->setSpacing(6);
        gbGalleryLayout->setContentsMargins(11, 11, 11, 11);
        gbGalleryLayout->setObjectName(QStringLiteral("gbGalleryLayout"));
        edtBackImage = new QLineEdit(gbGallery);
        edtBackImage->setObjectName(QStringLiteral("edtBackImage"));
        edtBackImage->setFont(font1);

        gbGalleryLayout->addWidget(edtBackImage, 6, 2, 1, 2);

        chkMenuToAbout = new QCheckBox(gbGallery);
        chkMenuToAbout->setObjectName(QStringLiteral("chkMenuToAbout"));
        chkMenuToAbout->setFont(font1);

        gbGalleryLayout->addWidget(chkMenuToAbout, 3, 1, 1, 1);

        label_47 = new QLabel(gbGallery);
        label_47->setObjectName(QStringLiteral("label_47"));
        label_47->setFont(font1);

        gbGalleryLayout->addWidget(label_47, 0, 1, 1, 1);

        chkAllowLinkToImages = new QCheckBox(gbGallery);
        chkAllowLinkToImages->setObjectName(QStringLiteral("chkAllowLinkToImages"));
        chkAllowLinkToImages->setFont(font1);

        gbGalleryLayout->addWidget(chkAllowLinkToImages, 11, 1, 1, 2);

        edtGalleryLanguages = new QLineEdit(gbGallery);
        edtGalleryLanguages->setObjectName(QStringLiteral("edtGalleryLanguages"));
        edtGalleryLanguages->setFont(font1);

        gbGalleryLayout->addWidget(edtGalleryLanguages, 1, 2, 1, 2);

        label_42 = new QLabel(gbGallery);
        label_42->setObjectName(QStringLiteral("label_42"));
        label_42->setFont(font1);

        gbGalleryLayout->addWidget(label_42, 2, 1, 1, 1);

        chkMenuToContact = new QCheckBox(gbGallery);
        chkMenuToContact->setObjectName(QStringLiteral("chkMenuToContact"));
        chkMenuToContact->setFont(font1);

        gbGalleryLayout->addWidget(chkMenuToContact, 10, 3, 1, 1);

        chkCanDownload = new QCheckBox(gbGallery);
        chkCanDownload->setObjectName(QStringLiteral("chkCanDownload"));
        chkCanDownload->setFont(font1);

        gbGalleryLayout->addWidget(chkCanDownload, 12, 1, 1, 2);

        chkFacebook = new QCheckBox(gbGallery);
        chkFacebook->setObjectName(QStringLiteral("chkFacebook"));
        chkFacebook->setFont(font1);

        gbGalleryLayout->addWidget(chkFacebook, 14, 3, 1, 1);

        label_62 = new QLabel(gbGallery);
        label_62->setObjectName(QStringLiteral("label_62"));
        label_62->setFont(font1);

        gbGalleryLayout->addWidget(label_62, 1, 1, 1, 1);

        chkRightClickProtected = new QCheckBox(gbGallery);
        chkRightClickProtected->setObjectName(QStringLiteral("chkRightClickProtected"));
        chkRightClickProtected->setFont(font1);

        gbGalleryLayout->addWidget(chkRightClickProtected, 14, 1, 1, 1);

        label_50 = new QLabel(gbGallery);
        label_50->setObjectName(QStringLiteral("label_50"));
        label_50->setFont(font1);

        gbGalleryLayout->addWidget(label_50, 6, 1, 1, 1);

        edtAbout = new QLineEdit(gbGallery);
        edtAbout->setObjectName(QStringLiteral("edtAbout"));
        edtAbout->setFont(font1);

        gbGalleryLayout->addWidget(edtAbout, 3, 2, 1, 2);

        chkMenuToDescriptions = new QCheckBox(gbGallery);
        chkMenuToDescriptions->setObjectName(QStringLiteral("chkMenuToDescriptions"));
        chkMenuToDescriptions->setFont(font1);

        gbGalleryLayout->addWidget(chkMenuToDescriptions, 11, 3, 1, 1);

        edtGalleryTitle = new QLineEdit(gbGallery);
        edtGalleryTitle->setObjectName(QStringLiteral("edtGalleryTitle"));
        edtGalleryTitle->setFont(font1);

        gbGalleryLayout->addWidget(edtGalleryTitle, 0, 2, 1, 2);

        chkForceSSL = new QCheckBox(gbGallery);
        chkForceSSL->setObjectName(QStringLiteral("chkForceSSL"));
        chkForceSSL->setEnabled(true);
        chkForceSSL->setFont(font1);
        chkForceSSL->setChecked(true);

        gbGalleryLayout->addWidget(chkForceSSL, 10, 1, 1, 2);

        edtEmailTo = new QLineEdit(gbGallery);
        edtEmailTo->setObjectName(QStringLiteral("edtEmailTo"));
        edtEmailTo->setFont(font1);

        gbGalleryLayout->addWidget(edtEmailTo, 2, 2, 1, 2);

        chkMenuToToggleCaptions = new QCheckBox(gbGallery);
        chkMenuToToggleCaptions->setObjectName(QStringLiteral("chkMenuToToggleCaptions"));
        chkMenuToToggleCaptions->setFont(font1);

        gbGalleryLayout->addWidget(chkMenuToToggleCaptions, 12, 3, 1, 1);

        chkSeparateFoldersForLanguages = new QCheckBox(gbGallery);
        chkSeparateFoldersForLanguages->setObjectName(QStringLiteral("chkSeparateFoldersForLanguages"));
        chkSeparateFoldersForLanguages->setFont(font1);

        gbGalleryLayout->addWidget(chkSeparateFoldersForLanguages, 9, 2, 1, 1);


        horizontalLayout_16->addWidget(gbGallery);

        verticalLayout_15 = new QVBoxLayout();
        verticalLayout_15->setSpacing(6);
        verticalLayout_15->setObjectName(QStringLiteral("verticalLayout_15"));
        gbLatest = new QGroupBox(tabGallery);
        gbLatest->setObjectName(QStringLiteral("gbLatest"));
        gbLatest->setFont(font2);
        gbLatestLayout = new QGridLayout(gbLatest);
        gbLatestLayout->setSpacing(6);
        gbLatestLayout->setContentsMargins(11, 11, 11, 11);
        gbLatestLayout->setObjectName(QStringLiteral("gbLatestLayout"));
        label_18 = new QLabel(gbLatest);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setFont(font1);

        gbLatestLayout->addWidget(label_18, 0, 4, 1, 1);

        label_30 = new QLabel(gbLatest);
        label_30->setObjectName(QStringLiteral("label_30"));
        label_30->setFont(font1);

        gbLatestLayout->addWidget(label_30, 0, 1, 1, 1);

        label_31 = new QLabel(gbLatest);
        label_31->setObjectName(QStringLiteral("label_31"));
        label_31->setFont(font1);

        gbLatestLayout->addWidget(label_31, 0, 3, 1, 1);

        sbLatestCount = new QSpinBox(gbLatest);
        sbLatestCount->setObjectName(QStringLiteral("sbLatestCount"));
        sbLatestCount->setFont(font1);
        sbLatestCount->setMinimum(1);
        sbLatestCount->setValue(10);

        gbLatestLayout->addWidget(sbLatestCount, 0, 5, 1, 1);

        sbNewDays = new QSpinBox(gbLatest);
        sbNewDays->setObjectName(QStringLiteral("sbNewDays"));
        sbNewDays->setFont(font1);

        gbLatestLayout->addWidget(sbNewDays, 0, 2, 1, 1);

        chkSetLatest = new QCheckBox(gbLatest);
        chkSetLatest->setObjectName(QStringLiteral("chkSetLatest"));
        chkSetLatest->setFont(font1);

        gbLatestLayout->addWidget(chkSetLatest, 0, 0, 1, 1);


        verticalLayout_15->addWidget(gbLatest);

        gbGoogleAnalytics = new QGroupBox(tabGallery);
        gbGoogleAnalytics->setObjectName(QStringLiteral("gbGoogleAnalytics"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(gbGoogleAnalytics->sizePolicy().hasHeightForWidth());
        gbGoogleAnalytics->setSizePolicy(sizePolicy1);
        gbGoogleAnalytics->setFont(font2);
        gridLayout_2 = new QGridLayout(gbGoogleAnalytics);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_9 = new QLabel(gbGoogleAnalytics);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setFont(font1);

        gridLayout_2->addWidget(label_9, 0, 2, 1, 1);

        chkUseGoogleAnalytics = new QCheckBox(gbGoogleAnalytics);
        chkUseGoogleAnalytics->setObjectName(QStringLiteral("chkUseGoogleAnalytics"));
        chkUseGoogleAnalytics->setFont(font1);

        gridLayout_2->addWidget(chkUseGoogleAnalytics, 0, 0, 1, 1);

        edtTrackingCode = new QLineEdit(gbGoogleAnalytics);
        edtTrackingCode->setObjectName(QStringLiteral("edtTrackingCode"));
        edtTrackingCode->setFont(font1);

        gridLayout_2->addWidget(edtTrackingCode, 0, 3, 1, 1);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_12, 0, 1, 1, 1);


        verticalLayout_15->addWidget(gbGoogleAnalytics);

        gbAdvanced = new QGroupBox(tabGallery);
        gbAdvanced->setObjectName(QStringLiteral("gbAdvanced"));
        sizePolicy1.setHeightForWidth(gbAdvanced->sizePolicy().hasHeightForWidth());
        gbAdvanced->setSizePolicy(sizePolicy1);
        gbAdvanced->setMinimumSize(QSize(0, 210));
        gbAdvanced->setFont(font2);
        gridLayout_17 = new QGridLayout(gbAdvanced);
        gridLayout_17->setSpacing(6);
        gridLayout_17->setContentsMargins(11, 11, 11, 11);
        gridLayout_17->setObjectName(QStringLiteral("gridLayout_17"));
        label_56 = new QLabel(gbAdvanced);
        label_56->setObjectName(QStringLiteral("label_56"));
        label_56->setMinimumSize(QSize(93, 0));
        label_56->setFont(font1);

        gridLayout_17->addWidget(label_56, 0, 0, 1, 1);

        edtImg = new QLineEdit(gbAdvanced);
        edtImg->setObjectName(QStringLiteral("edtImg"));
        edtImg->setFont(font1);

        gridLayout_17->addWidget(edtImg, 0, 1, 1, 1);

        label_23 = new QLabel(gbAdvanced);
        label_23->setObjectName(QStringLiteral("label_23"));
        label_23->setMinimumSize(QSize(93, 0));
        label_23->setFont(font1);

        gridLayout_17->addWidget(label_23, 0, 2, 1, 1);

        edtThumb = new QLineEdit(gbAdvanced);
        edtThumb->setObjectName(QStringLiteral("edtThumb"));
        edtThumb->setFont(font1);

        gridLayout_17->addWidget(edtThumb, 0, 3, 1, 1);

        label_8 = new QLabel(gbAdvanced);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setFont(font1);

        gridLayout_17->addWidget(label_8, 1, 0, 1, 1);

        edtVid = new QLineEdit(gbAdvanced);
        edtVid->setObjectName(QStringLiteral("edtVid"));
        edtVid->setFont(font1);

        gridLayout_17->addWidget(edtVid, 1, 1, 1, 1);

        horizontalSpacer_15 = new QSpacerItem(87, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_17->addItem(horizontalSpacer_15, 1, 2, 1, 2);

        label_22 = new QLabel(gbAdvanced);
        label_22->setObjectName(QStringLiteral("label_22"));
        label_22->setFont(font1);

        gridLayout_17->addWidget(label_22, 2, 0, 1, 1);

        edtAlbumDir = new QLineEdit(gbAdvanced);
        edtAlbumDir->setObjectName(QStringLiteral("edtAlbumDir"));
        edtAlbumDir->setFont(font1);

        gridLayout_17->addWidget(edtAlbumDir, 2, 1, 1, 1);

        label_45 = new QLabel(gbAdvanced);
        label_45->setObjectName(QStringLiteral("label_45"));
        label_45->setFont(font1);

        gridLayout_17->addWidget(label_45, 2, 2, 1, 1);

        edtBaseName = new QLineEdit(gbAdvanced);
        edtBaseName->setObjectName(QStringLiteral("edtBaseName"));
        edtBaseName->setFont(font1);

        gridLayout_17->addWidget(edtBaseName, 2, 3, 1, 1);

        label_67 = new QLabel(gbAdvanced);
        label_67->setObjectName(QStringLiteral("label_67"));
        label_67->setFont(font1);

        gridLayout_17->addWidget(label_67, 3, 0, 1, 1);

        widget_9 = new QWidget(gbAdvanced);
        widget_9->setObjectName(QStringLiteral("widget_9"));
        widget_9->setMinimumSize(QSize(0, 0));
        widget_9->setMaximumSize(QSize(16777215, 28));
        widget_9->setFont(font1);
        horizontalLayout_4 = new QHBoxLayout(widget_9);
        horizontalLayout_4->setSpacing(2);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(2, 2, 2, 2);
        edtGoogleFonts = new QLineEdit(widget_9);
        edtGoogleFonts->setObjectName(QStringLiteral("edtGoogleFonts"));

        horizontalLayout_4->addWidget(edtGoogleFonts);

        btnGoToGoogleFontsPage = new QToolButton(widget_9);
        btnGoToGoogleFontsPage->setObjectName(QStringLiteral("btnGoToGoogleFontsPage"));

        horizontalLayout_4->addWidget(btnGoToGoogleFontsPage);


        gridLayout_17->addWidget(widget_9, 3, 1, 1, 3);

        label_44 = new QLabel(gbAdvanced);
        label_44->setObjectName(QStringLiteral("label_44"));
        label_44->setFont(font1);

        gridLayout_17->addWidget(label_44, 4, 0, 1, 1);

        edtFontDir = new QLineEdit(gbAdvanced);
        edtFontDir->setObjectName(QStringLiteral("edtFontDir"));
        edtFontDir->setFont(font1);

        gridLayout_17->addWidget(edtFontDir, 4, 1, 1, 3);

        label_46 = new QLabel(gbAdvanced);
        label_46->setObjectName(QStringLiteral("label_46"));
        label_46->setFont(font1);

        gridLayout_17->addWidget(label_46, 5, 0, 1, 1);

        edtDefaultFonts = new QLineEdit(gbAdvanced);
        edtDefaultFonts->setObjectName(QStringLiteral("edtDefaultFonts"));
        edtDefaultFonts->setFont(font1);

        gridLayout_17->addWidget(edtDefaultFonts, 5, 1, 1, 3);


        verticalLayout_15->addWidget(gbAdvanced);


        horizontalLayout_16->addLayout(verticalLayout_15);


        tabGalleryLayout->addLayout(horizontalLayout_16);

        gridLayout_16 = new QGridLayout();
        gridLayout_16->setSpacing(6);
        gridLayout_16->setObjectName(QStringLiteral("gridLayout_16"));
        gridLayout_16->setContentsMargins(9, 4, 9, 4);
        chkReadFromGallery = new QCheckBox(tabGallery);
        chkReadFromGallery->setObjectName(QStringLiteral("chkReadFromGallery"));
        chkReadFromGallery->setEnabled(false);

        gridLayout_16->addWidget(chkReadFromGallery, 0, 0, 1, 1);

        chkLowerCaseImageExtensions = new QCheckBox(tabGallery);
        chkLowerCaseImageExtensions->setObjectName(QStringLiteral("chkLowerCaseImageExtensions"));
        chkLowerCaseImageExtensions->setChecked(true);

        gridLayout_16->addWidget(chkLowerCaseImageExtensions, 1, 1, 1, 1);

        chkAddTitlesToAll = new QCheckBox(tabGallery);
        chkAddTitlesToAll->setObjectName(QStringLiteral("chkAddTitlesToAll"));
        chkAddTitlesToAll->setChecked(false);

        gridLayout_16->addWidget(chkAddTitlesToAll, 0, 2, 1, 1);

        chkAddDescToAll = new QCheckBox(tabGallery);
        chkAddDescToAll->setObjectName(QStringLiteral("chkAddDescToAll"));
        chkAddDescToAll->setChecked(false);

        gridLayout_16->addWidget(chkAddDescToAll, 1, 2, 1, 1);

        chkOvrImages = new QCheckBox(tabGallery);
        chkOvrImages->setObjectName(QStringLiteral("chkOvrImages"));
        chkOvrImages->setFont(font1);
        chkOvrImages->setChecked(true);

        gridLayout_16->addWidget(chkOvrImages, 0, 1, 1, 1);

        chkDebugging = new QCheckBox(tabGallery);
        chkDebugging->setObjectName(QStringLiteral("chkDebugging"));
        chkDebugging->setContextMenuPolicy(Qt::DefaultContextMenu);
        chkDebugging->setStyleSheet(QStringLiteral(""));

        gridLayout_16->addWidget(chkDebugging, 1, 3, 1, 1);

        chkReadJAlbum = new QCheckBox(tabGallery);
        chkReadJAlbum->setObjectName(QStringLiteral("chkReadJAlbum"));

        gridLayout_16->addWidget(chkReadJAlbum, 1, 0, 1, 1);

        gridLayout_16->setColumnStretch(0, 1);

        tabGalleryLayout->addLayout(gridLayout_16);

        pnlProgress = new QWidget(tabGallery);
        pnlProgress->setObjectName(QStringLiteral("pnlProgress"));
        pnlProgress->setMaximumSize(QSize(16777215, 16777215));
        gridLayout_18 = new QGridLayout(pnlProgress);
        gridLayout_18->setSpacing(6);
        gridLayout_18->setContentsMargins(11, 11, 11, 11);
        gridLayout_18->setObjectName(QStringLiteral("gridLayout_18"));
        lblProgressDesc = new QLabel(pnlProgress);
        lblProgressDesc->setObjectName(QStringLiteral("lblProgressDesc"));

        gridLayout_18->addWidget(lblProgressDesc, 0, 3, 1, 2);

        lblProgressTitle = new QLabel(pnlProgress);
        lblProgressTitle->setObjectName(QStringLiteral("lblProgressTitle"));

        gridLayout_18->addWidget(lblProgressTitle, 0, 0, 1, 2);

        lblProgress = new QLabel(pnlProgress);
        lblProgress->setObjectName(QStringLiteral("lblProgress"));
        lblProgress->setMinimumSize(QSize(60, 0));
        QFont font3;
        font3.setFamily(QStringLiteral("Arial"));
        font3.setPointSize(10);
        font3.setBold(false);
        font3.setWeight(50);
        lblProgress->setFont(font3);

        gridLayout_18->addWidget(lblProgress, 1, 3, 1, 1);

        lblRemainingTimeTitle = new QLabel(pnlProgress);
        lblRemainingTimeTitle->setObjectName(QStringLiteral("lblRemainingTimeTitle"));

        gridLayout_18->addWidget(lblRemainingTimeTitle, 2, 0, 1, 1);

        label_43 = new QLabel(pnlProgress);
        label_43->setObjectName(QStringLiteral("label_43"));

        gridLayout_18->addWidget(label_43, 2, 3, 1, 1);

        lblRemainingTime = new QLabel(pnlProgress);
        lblRemainingTime->setObjectName(QStringLiteral("lblRemainingTime"));

        gridLayout_18->addWidget(lblRemainingTime, 2, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(379, 13, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_18->addItem(horizontalSpacer_2, 2, 2, 1, 1);

        progressBar = new QProgressBar(pnlProgress);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setMaximumSize(QSize(16777215, 5));
        progressBar->setValue(0);
        progressBar->setTextVisible(false);

        gridLayout_18->addWidget(progressBar, 1, 0, 1, 3);

        lblImagesPerSec = new QLabel(pnlProgress);
        lblImagesPerSec->setObjectName(QStringLiteral("lblImagesPerSec"));

        gridLayout_18->addWidget(lblImagesPerSec, 2, 4, 1, 1);


        tabGalleryLayout->addWidget(pnlProgress);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        tabGalleryLayout->addItem(verticalSpacer);

        tabFalconG->addTab(tabGallery, QString());
        tabDesign = new QWidget();
        tabDesign->setObjectName(QStringLiteral("tabDesign"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(tabDesign->sizePolicy().hasHeightForWidth());
        tabDesign->setSizePolicy(sizePolicy3);
        tabDesign->setMinimumSize(QSize(1004, 0));
        horizontalLayout_20 = new QHBoxLayout(tabDesign);
        horizontalLayout_20->setSpacing(6);
        horizontalLayout_20->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_20->setObjectName(QStringLiteral("horizontalLayout_20"));
        horizontalLayout_20->setContentsMargins(2, 2, 2, 2);
        widget_15 = new QWidget(tabDesign);
        widget_15->setObjectName(QStringLiteral("widget_15"));
        horizontalLayout_19 = new QHBoxLayout(widget_15);
        horizontalLayout_19->setSpacing(1);
        horizontalLayout_19->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_19->setObjectName(QStringLiteral("horizontalLayout_19"));
        horizontalLayout_19->setContentsMargins(1, 1, 1, 1);
        designSplitter = new QSplitter(widget_15);
        designSplitter->setObjectName(QStringLiteral("designSplitter"));
        sizePolicy3.setHeightForWidth(designSplitter->sizePolicy().hasHeightForWidth());
        designSplitter->setSizePolicy(sizePolicy3);
        designSplitter->setOrientation(Qt::Horizontal);
        designSplitter->setChildrenCollapsible(false);
        widget_16 = new QWidget(designSplitter);
        widget_16->setObjectName(QStringLiteral("widget_16"));
        widget_16->setMinimumSize(QSize(439, 0));
        horizontalLayout_10 = new QHBoxLayout(widget_16);
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        horizontalLayout_10->setContentsMargins(1, 1, 1, 1);
        verticalLayout_13 = new QVBoxLayout();
        verticalLayout_13->setSpacing(6);
        verticalLayout_13->setObjectName(QStringLiteral("verticalLayout_13"));
        verticalLayout_13->setSizeConstraint(QLayout::SetFixedSize);
        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setSpacing(6);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(10, 0, 2, 0);
        label_64 = new QLabel(widget_16);
        label_64->setObjectName(QStringLiteral("label_64"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(1);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_64->sizePolicy().hasHeightForWidth());
        label_64->setSizePolicy(sizePolicy4);
        QFont font4;
        font4.setFamily(QStringLiteral("Arial"));
        font4.setPointSize(10);
        font4.setBold(true);
        font4.setWeight(75);
        label_64->setFont(font4);

        horizontalLayout_13->addWidget(label_64);

        cbActualItem = new QComboBox(widget_16);
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->addItem(QString());
        cbActualItem->setObjectName(QStringLiteral("cbActualItem"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(3);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(cbActualItem->sizePolicy().hasHeightForWidth());
        cbActualItem->setSizePolicy(sizePolicy5);

        horizontalLayout_13->addWidget(cbActualItem);


        verticalLayout_13->addLayout(horizontalLayout_13);

        widget_2 = new QWidget(widget_16);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(3);
        sizePolicy6.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy6);
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(2, 2, 2, 2);
        toolBox = new QToolBox(widget_2);
        toolBox->setObjectName(QStringLiteral("toolBox"));
        sizePolicy3.setHeightForWidth(toolBox->sizePolicy().hasHeightForWidth());
        toolBox->setSizePolicy(sizePolicy3);
        toolBox->setFont(font4);
        pgGlobal = new QWidget();
        pgGlobal->setObjectName(QStringLiteral("pgGlobal"));
        pgGlobal->setGeometry(QRect(0, 0, 431, 737));
        sizePolicy3.setHeightForWidth(pgGlobal->sizePolicy().hasHeightForWidth());
        pgGlobal->setSizePolicy(sizePolicy3);
        pgGlobal->setMinimumSize(QSize(0, 0));
        btnSelectUplinkIcon = new QToolButton(pgGlobal);
        btnSelectUplinkIcon->setObjectName(QStringLiteral("btnSelectUplinkIcon"));
        btnSelectUplinkIcon->setGeometry(QRect(10, 110, 99, 22));
        btnSelectUplinkIcon->setFont(font3);
        gbGlobalColors = new QGroupBox(pgGlobal);
        gbGlobalColors->setObjectName(QStringLiteral("gbGlobalColors"));
        gbGlobalColors->setGeometry(QRect(0, 10, 371, 89));
        sizePolicy.setHeightForWidth(gbGlobalColors->sizePolicy().hasHeightForWidth());
        gbGlobalColors->setSizePolicy(sizePolicy);
        gbGlobalColors->setFont(font4);
        gridLayout = new QGridLayout(gbGlobalColors);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_68 = new QLabel(gbGlobalColors);
        label_68->setObjectName(QStringLiteral("label_68"));
        label_68->setFont(font3);

        gridLayout->addWidget(label_68, 0, 0, 1, 1);

        btnPageColor = new QToolButton(gbGlobalColors);
        btnPageColor->setObjectName(QStringLiteral("btnPageColor"));
        btnPageColor->setStyleSheet(QStringLiteral("QToolButton {background-color:#d6e0f7;}"));

        gridLayout->addWidget(btnPageColor, 0, 1, 1, 1);

        label_65 = new QLabel(gbGlobalColors);
        label_65->setObjectName(QStringLiteral("label_65"));
        label_65->setFont(font3);

        gridLayout->addWidget(label_65, 1, 0, 1, 1);

        btnPageBackground = new QToolButton(gbGlobalColors);
        btnPageBackground->setObjectName(QStringLiteral("btnPageBackground"));
        btnPageBackground->setStyleSheet(QStringLiteral("QToolButton {background-color:#1e1f25;}"));

        gridLayout->addWidget(btnPageBackground, 1, 1, 1, 1);

        chkSetAll = new QCheckBox(gbGlobalColors);
        chkSetAll->setObjectName(QStringLiteral("chkSetAll"));
        chkSetAll->setFont(font3);
        chkSetAll->setChecked(true);

        gridLayout->addWidget(chkSetAll, 1, 2, 1, 1);

        gbBackgroundImage = new QGroupBox(pgGlobal);
        gbBackgroundImage->setObjectName(QStringLiteral("gbBackgroundImage"));
        gbBackgroundImage->setGeometry(QRect(0, 152, 421, 341));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(gbBackgroundImage->sizePolicy().hasHeightForWidth());
        gbBackgroundImage->setSizePolicy(sizePolicy7);
        gbBackgroundImage->setFont(font4);
        gridLayout_3 = new QGridLayout(gbBackgroundImage);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gbBacgroundImagePosition = new QGroupBox(gbBackgroundImage);
        gbBacgroundImagePosition->setObjectName(QStringLiteral("gbBacgroundImagePosition"));
        verticalLayout = new QVBoxLayout(gbBacgroundImagePosition);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        rbNoBackgroundImage = new QRadioButton(gbBacgroundImagePosition);
        rbNoBackgroundImage->setObjectName(QStringLiteral("rbNoBackgroundImage"));

        verticalLayout->addWidget(rbNoBackgroundImage);

        rbCenterBckImage = new QRadioButton(gbBacgroundImagePosition);
        rbCenterBckImage->setObjectName(QStringLiteral("rbCenterBckImage"));

        verticalLayout->addWidget(rbCenterBckImage);

        rbCoverBckImage = new QRadioButton(gbBacgroundImagePosition);
        rbCoverBckImage->setObjectName(QStringLiteral("rbCoverBckImage"));

        verticalLayout->addWidget(rbCoverBckImage);

        rbTileBckImage = new QRadioButton(gbBacgroundImagePosition);
        rbTileBckImage->setObjectName(QStringLiteral("rbTileBckImage"));

        verticalLayout->addWidget(rbTileBckImage);


        gridLayout_3->addWidget(gbBacgroundImagePosition, 0, 3, 2, 1);

        label_63 = new QLabel(gbBackgroundImage);
        label_63->setObjectName(QStringLiteral("label_63"));
        label_63->setFont(font3);

        gridLayout_3->addWidget(label_63, 0, 0, 1, 1);

        lblbckImage = new QLabel(gbBackgroundImage);
        lblbckImage->setObjectName(QStringLiteral("lblbckImage"));
        sizePolicy3.setHeightForWidth(lblbckImage->sizePolicy().hasHeightForWidth());
        lblbckImage->setSizePolicy(sizePolicy3);
        lblbckImage->setMinimumSize(QSize(0, 80));
        lblbckImage->setStyleSheet(QLatin1String("background:white;\n"
"/* :url(\"s:/photo/falconG/src/Resources/Lazise_02596-Edit\") white */"));
        lblbckImage->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(lblbckImage, 1, 0, 1, 3);

        btnOpenBckImage = new QToolButton(gbBackgroundImage);
        btnOpenBckImage->setObjectName(QStringLiteral("btnOpenBckImage"));

        gridLayout_3->addWidget(btnOpenBckImage, 0, 2, 1, 1);

        edtBckImageName = new QLineEdit(gbBackgroundImage);
        edtBckImageName->setObjectName(QStringLiteral("edtBckImageName"));
        edtBckImageName->setMinimumSize(QSize(0, 24));

        gridLayout_3->addWidget(edtBckImageName, 0, 1, 1, 1);

        hsImageSizeToShow = new QSlider(gbBackgroundImage);
        hsImageSizeToShow->setObjectName(QStringLiteral("hsImageSizeToShow"));
        hsImageSizeToShow->setMaximum(100);
        hsImageSizeToShow->setValue(100);
        hsImageSizeToShow->setOrientation(Qt::Horizontal);
        hsImageSizeToShow->setTickPosition(QSlider::TicksBothSides);
        hsImageSizeToShow->setTickInterval(10);

        gridLayout_3->addWidget(hsImageSizeToShow, 2, 0, 1, 4);

        chkIconTop = new QCheckBox(pgGlobal);
        chkIconTop->setObjectName(QStringLiteral("chkIconTop"));
        chkIconTop->setEnabled(false);
        chkIconTop->setGeometry(QRect(130, 110, 157, 20));
        chkIconTop->setFont(font3);
        chkIconTop->setChecked(false);
        chkIconText = new QCheckBox(pgGlobal);
        chkIconText->setObjectName(QStringLiteral("chkIconText"));
        chkIconText->setEnabled(false);
        chkIconText->setGeometry(QRect(130, 130, 160, 20));
        chkIconText->setFont(font3);
        chkIconText->setChecked(false);
        toolBox->addItem(pgGlobal, QStringLiteral("Global Settings for Page"));
        pgSelected = new QWidget();
        pgSelected->setObjectName(QStringLiteral("pgSelected"));
        pgSelected->setGeometry(QRect(0, 0, 431, 737));
        gridLayout_4 = new QGridLayout(pgSelected);
        gridLayout_4->setSpacing(2);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        gridLayout_4->setContentsMargins(2, 2, 2, 2);
        gbGradient = new QGroupBox(pgSelected);
        gbGradient->setObjectName(QStringLiteral("gbGradient"));
        gbGradient->setEnabled(false);
        gbGradient->setMinimumSize(QSize(0, 120));
        gbGradient->setFont(font4);
        gridLayout_19 = new QGridLayout(gbGradient);
        gridLayout_19->setSpacing(6);
        gridLayout_19->setContentsMargins(11, 11, 11, 11);
        gridLayout_19->setObjectName(QStringLiteral("gridLayout_19"));
        chkUseGradient = new QCheckBox(gbGradient);
        chkUseGradient->setObjectName(QStringLiteral("chkUseGradient"));
        chkUseGradient->setFont(font3);
        chkUseGradient->setChecked(true);

        gridLayout_19->addWidget(chkUseGradient, 0, 1, 1, 2);

        label_4 = new QLabel(gbGradient);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_19->addWidget(label_4, 1, 0, 1, 1);

        label_24 = new QLabel(gbGradient);
        label_24->setObjectName(QStringLiteral("label_24"));

        gridLayout_19->addWidget(label_24, 1, 2, 1, 1);

        lblGradient = new QLabel(gbGradient);
        lblGradient->setObjectName(QStringLiteral("lblGradient"));
        QSizePolicy sizePolicy8(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(lblGradient->sizePolicy().hasHeightForWidth());
        lblGradient->setSizePolicy(sizePolicy8);
        lblGradient->setMinimumSize(QSize(50, 0));
        lblGradient->setStyleSheet(QStringLiteral("background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #A90329, stop:0.4 #890222, stop:1 #6d0019); "));

        gridLayout_19->addWidget(lblGradient, 2, 0, 3, 1);

        btnGradStartColor = new QToolButton(gbGradient);
        btnGradStartColor->setObjectName(QStringLiteral("btnGradStartColor"));
        btnGradStartColor->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        btnGradStartColor->setArrowType(Qt::LeftArrow);

        gridLayout_19->addWidget(btnGradStartColor, 2, 1, 1, 1);

        sbGradStartPos = new QSpinBox(gbGradient);
        sbGradStartPos->setObjectName(QStringLiteral("sbGradStartPos"));
        QSizePolicy sizePolicy9(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy9.setHorizontalStretch(0);
        sizePolicy9.setVerticalStretch(0);
        sizePolicy9.setHeightForWidth(sbGradStartPos->sizePolicy().hasHeightForWidth());
        sbGradStartPos->setSizePolicy(sizePolicy9);
        sbGradStartPos->setMinimumSize(QSize(0, 24));
        sbGradStartPos->setMaximumSize(QSize(60, 16777215));
        sbGradStartPos->setFont(font3);

        gridLayout_19->addWidget(sbGradStartPos, 2, 2, 1, 1);

        btnGradMiddleColor = new QToolButton(gbGradient);
        btnGradMiddleColor->setObjectName(QStringLiteral("btnGradMiddleColor"));
        btnGradMiddleColor->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        btnGradMiddleColor->setArrowType(Qt::LeftArrow);

        gridLayout_19->addWidget(btnGradMiddleColor, 3, 1, 1, 1);

        sbGradMiddlePos = new QSpinBox(gbGradient);
        sbGradMiddlePos->setObjectName(QStringLiteral("sbGradMiddlePos"));
        sizePolicy9.setHeightForWidth(sbGradMiddlePos->sizePolicy().hasHeightForWidth());
        sbGradMiddlePos->setSizePolicy(sizePolicy9);
        sbGradMiddlePos->setMinimumSize(QSize(0, 24));
        sbGradMiddlePos->setFont(font3);
        sbGradMiddlePos->setMaximum(100);
        sbGradMiddlePos->setValue(40);

        gridLayout_19->addWidget(sbGradMiddlePos, 3, 2, 1, 1);

        btnGradStopColor = new QToolButton(gbGradient);
        btnGradStopColor->setObjectName(QStringLiteral("btnGradStopColor"));
        btnGradStopColor->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        btnGradStopColor->setArrowType(Qt::LeftArrow);

        gridLayout_19->addWidget(btnGradStopColor, 4, 1, 1, 1);

        sbGradStopPos = new QSpinBox(gbGradient);
        sbGradStopPos->setObjectName(QStringLiteral("sbGradStopPos"));
        sizePolicy9.setHeightForWidth(sbGradStopPos->sizePolicy().hasHeightForWidth());
        sbGradStopPos->setSizePolicy(sizePolicy9);
        sbGradStopPos->setMinimumSize(QSize(0, 24));
        sbGradStopPos->setFont(font3);
        sbGradStopPos->setMaximum(100);
        sbGradStopPos->setValue(100);

        gridLayout_19->addWidget(sbGradStopPos, 4, 2, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_19->addItem(verticalSpacer_3, 5, 0, 1, 1);


        gridLayout_4->addWidget(gbGradient, 3, 0, 1, 1);

        gbTextStyle = new QGroupBox(pgSelected);
        gbTextStyle->setObjectName(QStringLiteral("gbTextStyle"));
        gbTextStyle->setMaximumSize(QSize(16777215, 99999));
        verticalLayout_12 = new QVBoxLayout(gbTextStyle);
        verticalLayout_12->setSpacing(2);
        verticalLayout_12->setContentsMargins(11, 11, 11, 11);
        verticalLayout_12->setObjectName(QStringLiteral("verticalLayout_12"));
        widget_8 = new QWidget(gbTextStyle);
        widget_8->setObjectName(QStringLiteral("widget_8"));
        horizontalLayout = new QHBoxLayout(widget_8);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_66 = new QLabel(widget_8);
        label_66->setObjectName(QStringLiteral("label_66"));
        QFont font5;
        font5.setFamily(QStringLiteral("Arial"));
        font5.setPointSize(10);
        font5.setBold(false);
        font5.setItalic(true);
        font5.setWeight(50);
        label_66->setFont(font5);

        horizontalLayout->addWidget(label_66);

        rbTextAlignNone = new QRadioButton(widget_8);
        rbTextAlignNone->setObjectName(QStringLiteral("rbTextAlignNone"));
        rbTextAlignNone->setFont(font3);
        rbTextAlignNone->setChecked(false);

        horizontalLayout->addWidget(rbTextAlignNone);

        rbTextLeft = new QRadioButton(widget_8);
        rbTextLeft->setObjectName(QStringLiteral("rbTextLeft"));
        rbTextLeft->setFont(font3);

        horizontalLayout->addWidget(rbTextLeft);

        rbTextCenter = new QRadioButton(widget_8);
        rbTextCenter->setObjectName(QStringLiteral("rbTextCenter"));
        rbTextCenter->setFont(font3);
        rbTextCenter->setChecked(true);

        horizontalLayout->addWidget(rbTextCenter);

        rbTextRight = new QRadioButton(widget_8);
        rbTextRight->setObjectName(QStringLiteral("rbTextRight"));
        rbTextRight->setFont(font3);
        rbTextRight->setChecked(false);

        horizontalLayout->addWidget(rbTextRight);


        verticalLayout_12->addWidget(widget_8);

        widget_10 = new QWidget(gbTextStyle);
        widget_10->setObjectName(QStringLiteral("widget_10"));
        horizontalLayout_14 = new QHBoxLayout(widget_10);
        horizontalLayout_14->setSpacing(6);
        horizontalLayout_14->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        horizontalLayout_14->setContentsMargins(0, 0, 0, 0);
        label_69 = new QLabel(widget_10);
        label_69->setObjectName(QStringLiteral("label_69"));
        label_69->setFont(font5);

        horizontalLayout_14->addWidget(label_69);

        chkTdUnderline = new QCheckBox(widget_10);
        chkTdUnderline->setObjectName(QStringLiteral("chkTdUnderline"));
        chkTdUnderline->setFont(font3);

        horizontalLayout_14->addWidget(chkTdUnderline);

        chkTdOverline = new QCheckBox(widget_10);
        chkTdOverline->setObjectName(QStringLiteral("chkTdOverline"));
        chkTdOverline->setFont(font3);

        horizontalLayout_14->addWidget(chkTdOverline);

        chkTdLinethrough = new QCheckBox(widget_10);
        chkTdLinethrough->setObjectName(QStringLiteral("chkTdLinethrough"));
        chkTdLinethrough->setFont(font3);

        horizontalLayout_14->addWidget(chkTdLinethrough);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_5);


        verticalLayout_12->addWidget(widget_10);

        gbDecorationStyle = new QGroupBox(gbTextStyle);
        gbDecorationStyle->setObjectName(QStringLiteral("gbDecorationStyle"));
        gbDecorationStyle->setEnabled(false);
        gridLayout_13 = new QGridLayout(gbDecorationStyle);
        gridLayout_13->setSpacing(6);
        gridLayout_13->setContentsMargins(11, 11, 11, 11);
        gridLayout_13->setObjectName(QStringLiteral("gridLayout_13"));
        rbTdWavy = new QRadioButton(gbDecorationStyle);
        rbTdWavy->setObjectName(QStringLiteral("rbTdWavy"));
        rbTdWavy->setEnabled(false);
        rbTdWavy->setFont(font3);

        gridLayout_13->addWidget(rbTdWavy, 1, 1, 1, 1);

        rbTdDouble = new QRadioButton(gbDecorationStyle);
        rbTdDouble->setObjectName(QStringLiteral("rbTdDouble"));
        rbTdDouble->setEnabled(false);
        rbTdDouble->setFont(font3);

        gridLayout_13->addWidget(rbTdDouble, 1, 0, 1, 1);

        rbTdSolid = new QRadioButton(gbDecorationStyle);
        rbTdSolid->setObjectName(QStringLiteral("rbTdSolid"));
        rbTdSolid->setEnabled(false);
        rbTdSolid->setFont(font3);
        rbTdSolid->setChecked(true);

        gridLayout_13->addWidget(rbTdSolid, 0, 0, 1, 1);

        rbTdDotted = new QRadioButton(gbDecorationStyle);
        rbTdDotted->setObjectName(QStringLiteral("rbTdDotted"));
        rbTdDotted->setEnabled(false);
        rbTdDotted->setFont(font3);

        gridLayout_13->addWidget(rbTdDotted, 0, 1, 1, 1);

        rbTdDashed = new QRadioButton(gbDecorationStyle);
        rbTdDashed->setObjectName(QStringLiteral("rbTdDashed"));
        rbTdDashed->setEnabled(false);
        rbTdDashed->setFont(font3);

        gridLayout_13->addWidget(rbTdDashed, 0, 2, 1, 1);


        verticalLayout_12->addWidget(gbDecorationStyle);

        gbTextShadow = new QGroupBox(gbTextStyle);
        gbTextShadow->setObjectName(QStringLiteral("gbTextShadow"));
        gbTextShadow->setMinimumSize(QSize(0, 110));
        gbTextShadow->setFont(font5);
        gridLayout_15 = new QGridLayout(gbTextShadow);
        gridLayout_15->setSpacing(6);
        gridLayout_15->setContentsMargins(11, 11, 11, 11);
        gridLayout_15->setObjectName(QStringLiteral("gridLayout_15"));
        gridLayout_14 = new QGridLayout();
        gridLayout_14->setSpacing(6);
        gridLayout_14->setObjectName(QStringLiteral("gridLayout_14"));
        label_85 = new QLabel(gbTextShadow);
        label_85->setObjectName(QStringLiteral("label_85"));
        QFont font6;
        font6.setFamily(QStringLiteral("Arial"));
        font6.setPointSize(10);
        font6.setBold(false);
        font6.setItalic(false);
        font6.setWeight(50);
        label_85->setFont(font6);

        gridLayout_14->addWidget(label_85, 0, 0, 1, 1);

        label_37 = new QLabel(gbTextShadow);
        label_37->setObjectName(QStringLiteral("label_37"));
        QFont font7;
        font7.setFamily(QStringLiteral("Arial"));
        font7.setPointSize(8);
        font7.setBold(false);
        font7.setItalic(true);
        font7.setWeight(50);
        label_37->setFont(font7);

        gridLayout_14->addWidget(label_37, 0, 1, 1, 1);

        sbShadowHoriz1 = new QSpinBox(gbTextShadow);
        sbShadowHoriz1->setObjectName(QStringLiteral("sbShadowHoriz1"));
        sizePolicy9.setHeightForWidth(sbShadowHoriz1->sizePolicy().hasHeightForWidth());
        sbShadowHoriz1->setSizePolicy(sizePolicy9);
        sbShadowHoriz1->setFont(font7);

        gridLayout_14->addWidget(sbShadowHoriz1, 0, 2, 1, 1);

        label_38 = new QLabel(gbTextShadow);
        label_38->setObjectName(QStringLiteral("label_38"));
        label_38->setFont(font7);

        gridLayout_14->addWidget(label_38, 0, 3, 1, 1);

        sbShadowVert1 = new QSpinBox(gbTextShadow);
        sbShadowVert1->setObjectName(QStringLiteral("sbShadowVert1"));
        sizePolicy9.setHeightForWidth(sbShadowVert1->sizePolicy().hasHeightForWidth());
        sbShadowVert1->setSizePolicy(sizePolicy9);
        sbShadowVert1->setFont(font7);

        gridLayout_14->addWidget(sbShadowVert1, 0, 4, 1, 1);

        label_39 = new QLabel(gbTextShadow);
        label_39->setObjectName(QStringLiteral("label_39"));
        label_39->setFont(font7);

        gridLayout_14->addWidget(label_39, 0, 5, 1, 1);

        sbShadowBlur1 = new QSpinBox(gbTextShadow);
        sbShadowBlur1->setObjectName(QStringLiteral("sbShadowBlur1"));
        sizePolicy9.setHeightForWidth(sbShadowBlur1->sizePolicy().hasHeightForWidth());
        sbShadowBlur1->setSizePolicy(sizePolicy9);
        sbShadowBlur1->setFont(font7);

        gridLayout_14->addWidget(sbShadowBlur1, 0, 6, 1, 1);

        label_87 = new QLabel(gbTextShadow);
        label_87->setObjectName(QStringLiteral("label_87"));
        label_87->setFont(font7);

        gridLayout_14->addWidget(label_87, 0, 7, 1, 1);

        sbShadowSpread1 = new QSpinBox(gbTextShadow);
        sbShadowSpread1->setObjectName(QStringLiteral("sbShadowSpread1"));
        sizePolicy9.setHeightForWidth(sbShadowSpread1->sizePolicy().hasHeightForWidth());
        sbShadowSpread1->setSizePolicy(sizePolicy9);
        sbShadowSpread1->setFont(font7);

        gridLayout_14->addWidget(sbShadowSpread1, 0, 8, 1, 1);

        label_86 = new QLabel(gbTextShadow);
        label_86->setObjectName(QStringLiteral("label_86"));
        label_86->setFont(font6);

        gridLayout_14->addWidget(label_86, 1, 0, 1, 1);

        label_83 = new QLabel(gbTextShadow);
        label_83->setObjectName(QStringLiteral("label_83"));
        label_83->setFont(font7);

        gridLayout_14->addWidget(label_83, 1, 1, 1, 1);

        sbShadowHoriz2 = new QSpinBox(gbTextShadow);
        sbShadowHoriz2->setObjectName(QStringLiteral("sbShadowHoriz2"));
        sizePolicy9.setHeightForWidth(sbShadowHoriz2->sizePolicy().hasHeightForWidth());
        sbShadowHoriz2->setSizePolicy(sizePolicy9);
        sbShadowHoriz2->setFont(font7);
        sbShadowHoriz2->setMinimum(-99);
        sbShadowHoriz2->setMaximum(0);

        gridLayout_14->addWidget(sbShadowHoriz2, 1, 2, 1, 1);

        label_84 = new QLabel(gbTextShadow);
        label_84->setObjectName(QStringLiteral("label_84"));
        label_84->setFont(font7);

        gridLayout_14->addWidget(label_84, 1, 3, 1, 1);

        sbShadowVert2 = new QSpinBox(gbTextShadow);
        sbShadowVert2->setObjectName(QStringLiteral("sbShadowVert2"));
        sizePolicy9.setHeightForWidth(sbShadowVert2->sizePolicy().hasHeightForWidth());
        sbShadowVert2->setSizePolicy(sizePolicy9);
        sbShadowVert2->setFont(font7);
        sbShadowVert2->setMinimum(-99);
        sbShadowVert2->setMaximum(0);

        gridLayout_14->addWidget(sbShadowVert2, 1, 4, 1, 1);

        label_82 = new QLabel(gbTextShadow);
        label_82->setObjectName(QStringLiteral("label_82"));
        label_82->setFont(font7);

        gridLayout_14->addWidget(label_82, 1, 5, 1, 1);

        sbShadowBlur2 = new QSpinBox(gbTextShadow);
        sbShadowBlur2->setObjectName(QStringLiteral("sbShadowBlur2"));
        sizePolicy9.setHeightForWidth(sbShadowBlur2->sizePolicy().hasHeightForWidth());
        sbShadowBlur2->setSizePolicy(sizePolicy9);
        sbShadowBlur2->setFont(font7);

        gridLayout_14->addWidget(sbShadowBlur2, 1, 6, 1, 1);

        label_88 = new QLabel(gbTextShadow);
        label_88->setObjectName(QStringLiteral("label_88"));
        label_88->setFont(font7);

        gridLayout_14->addWidget(label_88, 1, 7, 1, 1);

        sbShadowSpread2 = new QSpinBox(gbTextShadow);
        sbShadowSpread2->setObjectName(QStringLiteral("sbShadowSpread2"));
        sizePolicy9.setHeightForWidth(sbShadowSpread2->sizePolicy().hasHeightForWidth());
        sbShadowSpread2->setSizePolicy(sizePolicy9);
        sbShadowSpread2->setFont(font7);

        gridLayout_14->addWidget(sbShadowSpread2, 1, 8, 1, 1);


        gridLayout_15->addLayout(gridLayout_14, 1, 0, 1, 3);

        rbBoxShadow = new QRadioButton(gbTextShadow);
        rbBoxShadow->setObjectName(QStringLiteral("rbBoxShadow"));
        rbBoxShadow->setChecked(true);

        gridLayout_15->addWidget(rbBoxShadow, 0, 1, 1, 1);

        chkShadowOn = new QCheckBox(gbTextShadow);
        chkShadowOn->setObjectName(QStringLiteral("chkShadowOn"));
        chkShadowOn->setFont(font7);

        gridLayout_15->addWidget(chkShadowOn, 0, 3, 1, 1);

        rbTextShadow = new QRadioButton(gbTextShadow);
        rbTextShadow->setObjectName(QStringLiteral("rbTextShadow"));

        gridLayout_15->addWidget(rbTextShadow, 0, 0, 1, 1);

        btnShadowColor = new QToolButton(gbTextShadow);
        btnShadowColor->setObjectName(QStringLiteral("btnShadowColor"));
        QFont font8;
        font8.setFamily(QStringLiteral("MS Shell Dlg 2"));
        font8.setPointSize(8);
        font8.setBold(false);
        font8.setItalic(false);
        font8.setWeight(50);
        btnShadowColor->setFont(font8);
        btnShadowColor->setStyleSheet(QStringLiteral("QToolButton {background-color:#ddd;}"));

        gridLayout_15->addWidget(btnShadowColor, 1, 3, 1, 1);

        chkShadowOn->raise();
        rbTextShadow->raise();
        rbBoxShadow->raise();
        btnShadowColor->raise();

        verticalLayout_12->addWidget(gbTextShadow);


        gridLayout_4->addWidget(gbTextStyle, 2, 0, 1, 2);

        gbFont = new QGroupBox(pgSelected);
        gbFont->setObjectName(QStringLiteral("gbFont"));
        gbFont->setMinimumSize(QSize(270, 130));
        gbFont->setFont(font4);
        gridLayout_9 = new QGridLayout(gbFont);
        gridLayout_9->setSpacing(6);
        gridLayout_9->setContentsMargins(11, 11, 11, 11);
        gridLayout_9->setObjectName(QStringLiteral("gridLayout_9"));
        label_27 = new QLabel(gbFont);
        label_27->setObjectName(QStringLiteral("label_27"));
        label_27->setFont(font3);

        gridLayout_9->addWidget(label_27, 0, 0, 1, 1);

        edtFontFamily = new QLineEdit(gbFont);
        edtFontFamily->setObjectName(QStringLiteral("edtFontFamily"));
        edtFontFamily->setMinimumSize(QSize(0, 24));
        edtFontFamily->setFont(font3);

        gridLayout_9->addWidget(edtFontFamily, 0, 1, 1, 3);

        cbFonts = new QComboBox(gbFont);
        cbFonts->setObjectName(QStringLiteral("cbFonts"));
        cbFonts->setEnabled(false);

        gridLayout_9->addWidget(cbFonts, 0, 4, 1, 2);

        label_28 = new QLabel(gbFont);
        label_28->setObjectName(QStringLiteral("label_28"));
        label_28->setFont(font3);

        gridLayout_9->addWidget(label_28, 1, 0, 1, 1);

        cbPointSize = new QComboBox(gbFont);
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
        cbPointSize->setObjectName(QStringLiteral("cbPointSize"));
        cbPointSize->setMinimumSize(QSize(0, 24));
        cbPointSize->setFont(font3);
        cbPointSize->setEditable(true);
        cbPointSize->setInsertPolicy(QComboBox::NoInsert);

        gridLayout_9->addWidget(cbPointSize, 1, 1, 1, 1);

        label_32 = new QLabel(gbFont);
        label_32->setObjectName(QStringLiteral("label_32"));
        label_32->setFont(font3);

        gridLayout_9->addWidget(label_32, 1, 2, 1, 1);

        cbLineHeight = new QComboBox(gbFont);
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->addItem(QString());
        cbLineHeight->setObjectName(QStringLiteral("cbLineHeight"));
        cbLineHeight->setFont(font3);
        cbLineHeight->setEditable(true);

        gridLayout_9->addWidget(cbLineHeight, 1, 3, 1, 1);

        label_6 = new QLabel(gbFont);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setFont(font3);

        gridLayout_9->addWidget(label_6, 1, 4, 1, 1);

        sbSpaceAfter = new QSpinBox(gbFont);
        sbSpaceAfter->setObjectName(QStringLiteral("sbSpaceAfter"));
        sbSpaceAfter->setFont(font3);
        sbSpaceAfter->setMaximum(100);

        gridLayout_9->addWidget(sbSpaceAfter, 1, 5, 1, 1);

        horizontalSpacer_13 = new QSpacerItem(53, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_9->addItem(horizontalSpacer_13, 2, 0, 1, 1);

        chkDifferentFirstLine = new QCheckBox(gbFont);
        chkDifferentFirstLine->setObjectName(QStringLiteral("chkDifferentFirstLine"));
        chkDifferentFirstLine->setFont(font3);

        gridLayout_9->addWidget(chkDifferentFirstLine, 2, 1, 1, 2);

        cbPointSizeFirstLine = new QComboBox(gbFont);
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->addItem(QString());
        cbPointSizeFirstLine->setObjectName(QStringLiteral("cbPointSizeFirstLine"));
        cbPointSizeFirstLine->setMinimumSize(QSize(0, 24));
        cbPointSizeFirstLine->setFont(font3);
        cbPointSizeFirstLine->setEditable(true);
        cbPointSizeFirstLine->setInsertPolicy(QComboBox::NoInsert);

        gridLayout_9->addWidget(cbPointSizeFirstLine, 2, 3, 1, 1);

        chkBold = new QCheckBox(gbFont);
        chkBold->setObjectName(QStringLiteral("chkBold"));
        chkBold->setFont(font3);

        gridLayout_9->addWidget(chkBold, 2, 4, 1, 1);

        chkItalic = new QCheckBox(gbFont);
        chkItalic->setObjectName(QStringLiteral("chkItalic"));
        chkItalic->setFont(font3);

        gridLayout_9->addWidget(chkItalic, 2, 5, 1, 1);


        gridLayout_4->addWidget(gbFont, 1, 0, 1, 2);

        gbColors = new QWidget(pgSelected);
        gbColors->setObjectName(QStringLiteral("gbColors"));
        sizePolicy.setHeightForWidth(gbColors->sizePolicy().hasHeightForWidth());
        gbColors->setSizePolicy(sizePolicy);
        gbColors->setMinimumSize(QSize(0, 92));
        gbColors->setFont(font4);
        gridLayout_7 = new QGridLayout(gbColors);
        gridLayout_7->setSpacing(6);
        gridLayout_7->setContentsMargins(11, 11, 11, 11);
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        gridLayout_7->setHorizontalSpacing(6);
        gridLayout_7->setContentsMargins(2, 2, 2, 2);
        horizontalSpacer_9 = new QSpacerItem(33, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_7->addItem(horizontalSpacer_9, 0, 4, 1, 1);

        label_26 = new QLabel(gbColors);
        label_26->setObjectName(QStringLiteral("label_26"));
        label_26->setFont(font3);

        gridLayout_7->addWidget(label_26, 0, 0, 1, 1);

        label_33 = new QLabel(gbColors);
        label_33->setObjectName(QStringLiteral("label_33"));
        label_33->setFont(font3);

        gridLayout_7->addWidget(label_33, 1, 0, 1, 1);

        sbBackgroundOpacity = new QSpinBox(gbColors);
        sbBackgroundOpacity->setObjectName(QStringLiteral("sbBackgroundOpacity"));
        sbBackgroundOpacity->setEnabled(false);
        sbBackgroundOpacity->setFont(font3);
        sbBackgroundOpacity->setMinimum(0);
        sbBackgroundOpacity->setMaximum(100);
        sbBackgroundOpacity->setSingleStep(10);
        sbBackgroundOpacity->setValue(100);

        gridLayout_7->addWidget(sbBackgroundOpacity, 1, 2, 1, 1);

        btnForeground = new QToolButton(gbColors);
        btnForeground->setObjectName(QStringLiteral("btnForeground"));
        sizePolicy9.setHeightForWidth(btnForeground->sizePolicy().hasHeightForWidth());
        btnForeground->setSizePolicy(sizePolicy9);
        QFont font9;
        font9.setFamily(QStringLiteral("MS Shell Dlg 2"));
        font9.setPointSize(10);
        font9.setBold(false);
        font9.setWeight(50);
        btnForeground->setFont(font9);
        btnForeground->setStyleSheet(QStringLiteral("QToolButton {background-color:#ddd;}"));

        gridLayout_7->addWidget(btnForeground, 0, 1, 1, 1);

        chkTextOpacity = new QCheckBox(gbColors);
        chkTextOpacity->setObjectName(QStringLiteral("chkTextOpacity"));
        chkTextOpacity->setFont(font3);

        gridLayout_7->addWidget(chkTextOpacity, 0, 3, 1, 1);

        chkBackgroundOpacity = new QCheckBox(gbColors);
        chkBackgroundOpacity->setObjectName(QStringLiteral("chkBackgroundOpacity"));
        chkBackgroundOpacity->setFont(font3);

        gridLayout_7->addWidget(chkBackgroundOpacity, 1, 3, 1, 1);

        btnBackground = new QToolButton(gbColors);
        btnBackground->setObjectName(QStringLiteral("btnBackground"));
        sizePolicy9.setHeightForWidth(btnBackground->sizePolicy().hasHeightForWidth());
        btnBackground->setSizePolicy(sizePolicy9);
        btnBackground->setStyleSheet(QStringLiteral("QToolButton {background-color:#a03020;}"));

        gridLayout_7->addWidget(btnBackground, 1, 1, 1, 1);

        sbTextOpacity = new QSpinBox(gbColors);
        sbTextOpacity->setObjectName(QStringLiteral("sbTextOpacity"));
        sbTextOpacity->setEnabled(false);
        sbTextOpacity->setFont(font3);
        sbTextOpacity->setMinimum(0);
        sbTextOpacity->setMaximum(100);
        sbTextOpacity->setSingleStep(10);
        sbTextOpacity->setValue(100);

        gridLayout_7->addWidget(sbTextOpacity, 0, 2, 1, 1);


        gridLayout_4->addWidget(gbColors, 0, 0, 1, 2);

        gbBorder = new QGroupBox(pgSelected);
        gbBorder->setObjectName(QStringLiteral("gbBorder"));
        gbBorder->setEnabled(false);
        gridLayout_6 = new QGridLayout(gbBorder);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        label_57 = new QLabel(gbBorder);
        label_57->setObjectName(QStringLiteral("label_57"));
        label_57->setFont(font3);

        gridLayout_6->addWidget(label_57, 2, 0, 1, 1);

        cbBorderStyle = new QComboBox(gbBorder);
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->addItem(QString());
        cbBorderStyle->setObjectName(QStringLiteral("cbBorderStyle"));
        cbBorderStyle->setFont(font3);

        gridLayout_6->addWidget(cbBorderStyle, 2, 1, 1, 3);

        btnBorderColor = new QToolButton(gbBorder);
        btnBorderColor->setObjectName(QStringLiteral("btnBorderColor"));
        sizePolicy9.setHeightForWidth(btnBorderColor->sizePolicy().hasHeightForWidth());
        btnBorderColor->setSizePolicy(sizePolicy9);
        QFont font10;
        font10.setFamily(QStringLiteral("MS Shell Dlg 2"));
        font10.setPointSize(10);
        font10.setBold(true);
        font10.setWeight(75);
        btnBorderColor->setFont(font10);
        btnBorderColor->setStyleSheet(QStringLiteral("QToolButton {background-color:#890222;color:#76fccc;}"));

        gridLayout_6->addWidget(btnBorderColor, 2, 4, 1, 1);

        label_58 = new QLabel(gbBorder);
        label_58->setObjectName(QStringLiteral("label_58"));
        label_58->setFont(font3);

        gridLayout_6->addWidget(label_58, 3, 0, 1, 1);

        sbBorderWidth = new QSpinBox(gbBorder);
        sbBorderWidth->setObjectName(QStringLiteral("sbBorderWidth"));
        sbBorderWidth->setFont(font3);
        sbBorderWidth->setMaximum(100);

        gridLayout_6->addWidget(sbBorderWidth, 3, 1, 1, 2);

        line = new QFrame(gbBorder);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_6->addWidget(line, 4, 0, 1, 5);

        label_70 = new QLabel(gbBorder);
        label_70->setObjectName(QStringLiteral("label_70"));
        label_70->setFont(font3);

        gridLayout_6->addWidget(label_70, 5, 0, 1, 2);

        sbBorderRadius = new QSpinBox(gbBorder);
        sbBorderRadius->setObjectName(QStringLiteral("sbBorderRadius"));
        sbBorderRadius->setFont(font3);

        gridLayout_6->addWidget(sbBorderRadius, 5, 2, 1, 2);

        rbLeftBorder = new QRadioButton(gbBorder);
        rbLeftBorder->setObjectName(QStringLiteral("rbLeftBorder"));
        rbLeftBorder->setFont(font3);

        gridLayout_6->addWidget(rbLeftBorder, 1, 3, 1, 2);

        rbBottomBorder = new QRadioButton(gbBorder);
        rbBottomBorder->setObjectName(QStringLiteral("rbBottomBorder"));
        rbBottomBorder->setFont(font3);

        gridLayout_6->addWidget(rbBottomBorder, 1, 1, 1, 2);

        rbRightBorder = new QRadioButton(gbBorder);
        rbRightBorder->setObjectName(QStringLiteral("rbRightBorder"));
        rbRightBorder->setFont(font3);

        gridLayout_6->addWidget(rbRightBorder, 0, 3, 1, 2);

        rbTopBorder = new QRadioButton(gbBorder);
        rbTopBorder->setObjectName(QStringLiteral("rbTopBorder"));
        rbTopBorder->setFont(font3);

        gridLayout_6->addWidget(rbTopBorder, 0, 1, 1, 2);

        rbAllBorders = new QRadioButton(gbBorder);
        rbAllBorders->setObjectName(QStringLiteral("rbAllBorders"));
        rbAllBorders->setFont(font3);

        gridLayout_6->addWidget(rbAllBorders, 0, 0, 1, 1);


        gridLayout_4->addWidget(gbBorder, 3, 1, 1, 1);

        gridLayout_4->setRowStretch(0, 1);
        toolBox->addItem(pgSelected, QStringLiteral("Selected item"));
        pgImageDecor = new QWidget();
        pgImageDecor->setObjectName(QStringLiteral("pgImageDecor"));
        pgImageDecor->setGeometry(QRect(0, 0, 431, 737));
        verticalLayout_10 = new QVBoxLayout(pgImageDecor);
        verticalLayout_10->setSpacing(6);
        verticalLayout_10->setContentsMargins(11, 11, 11, 11);
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        verticalLayout_10->setContentsMargins(3, 3, 3, 3);
        gbResizing = new QWidget(pgImageDecor);
        gbResizing->setObjectName(QStringLiteral("gbResizing"));
        sizePolicy1.setHeightForWidth(gbResizing->sizePolicy().hasHeightForWidth());
        gbResizing->setSizePolicy(sizePolicy1);
        gbResizing->setMinimumSize(QSize(0, 206));
        gbResizing->setFont(font4);
        gridLayout_12 = new QGridLayout(gbResizing);
        gridLayout_12->setSpacing(6);
        gridLayout_12->setContentsMargins(11, 11, 11, 11);
        gridLayout_12->setObjectName(QStringLiteral("gridLayout_12"));
        horizontalSpacer_6 = new QSpacerItem(94, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_12->addItem(horizontalSpacer_6, 1, 6, 1, 3);

        label_2 = new QLabel(gbResizing);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font3);

        gridLayout_12->addWidget(label_2, 0, 1, 1, 1);

        sbImageMargin = new QSpinBox(gbResizing);
        sbImageMargin->setObjectName(QStringLiteral("sbImageMargin"));
        sbImageMargin->setFont(font3);

        gridLayout_12->addWidget(sbImageMargin, 4, 8, 1, 1);

        label_15 = new QLabel(gbResizing);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setFont(font3);

        gridLayout_12->addWidget(label_15, 4, 7, 1, 1);

        chkDistortThumbnails = new QCheckBox(gbResizing);
        chkDistortThumbnails->setObjectName(QStringLiteral("chkDistortThumbnails"));
        sizePolicy9.setHeightForWidth(chkDistortThumbnails->sizePolicy().hasHeightForWidth());
        chkDistortThumbnails->setSizePolicy(sizePolicy9);
        chkDistortThumbnails->setFont(font3);

        gridLayout_12->addWidget(chkDistortThumbnails, 5, 0, 1, 5);

        label_7 = new QLabel(gbResizing);
        label_7->setObjectName(QStringLiteral("label_7"));
        QSizePolicy sizePolicy10(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy10.setHorizontalStretch(0);
        sizePolicy10.setVerticalStretch(0);
        sizePolicy10.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy10);
        label_7->setFont(font3);

        gridLayout_12->addWidget(label_7, 4, 0, 1, 1);

        sbImageHeight = new QSpinBox(gbResizing);
        sbImageHeight->setObjectName(QStringLiteral("sbImageHeight"));
        sizePolicy9.setHeightForWidth(sbImageHeight->sizePolicy().hasHeightForWidth());
        sbImageHeight->setSizePolicy(sizePolicy9);
        sbImageHeight->setFont(font3);
        sbImageHeight->setMinimum(50);
        sbImageHeight->setMaximum(8192);
        sbImageHeight->setSingleStep(10);
        sbImageHeight->setValue(1280);

        gridLayout_12->addWidget(sbImageHeight, 1, 4, 1, 1);

        sbImageWidth = new QSpinBox(gbResizing);
        sbImageWidth->setObjectName(QStringLiteral("sbImageWidth"));
        sizePolicy9.setHeightForWidth(sbImageWidth->sizePolicy().hasHeightForWidth());
        sbImageWidth->setSizePolicy(sizePolicy9);
        sbImageWidth->setFont(font3);
        sbImageWidth->setMinimum(50);
        sbImageWidth->setMaximum(8192);
        sbImageWidth->setSingleStep(10);
        sbImageWidth->setValue(1920);

        gridLayout_12->addWidget(sbImageWidth, 1, 1, 1, 2);

        btnLink = new QToolButton(gbResizing);
        btnLink->setObjectName(QStringLiteral("btnLink"));
        btnLink->setFont(font3);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/Resources/broken-link.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(QStringLiteral(":/icons/Resources/link.png"), QSize(), QIcon::Normal, QIcon::On);
        btnLink->setIcon(icon);
        btnLink->setIconSize(QSize(24, 24));
        btnLink->setCheckable(true);

        gridLayout_12->addWidget(btnLink, 1, 3, 1, 1);

        label_73 = new QLabel(gbResizing);
        label_73->setObjectName(QStringLiteral("label_73"));
        label_73->setFont(font3);

        gridLayout_12->addWidget(label_73, 3, 0, 1, 2);

        cbImageQuality = new QComboBox(gbResizing);
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->addItem(QString());
        cbImageQuality->setObjectName(QStringLiteral("cbImageQuality"));
        cbImageQuality->setFont(font3);

        gridLayout_12->addWidget(cbImageQuality, 3, 2, 1, 5);

        label_59 = new QLabel(gbResizing);
        label_59->setObjectName(QStringLiteral("label_59"));
        label_59->setFont(font3);
        label_59->setAlignment(Qt::AlignCenter);

        gridLayout_12->addWidget(label_59, 4, 3, 1, 1);

        label_3 = new QLabel(gbResizing);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setFont(font3);

        gridLayout_12->addWidget(label_3, 0, 4, 1, 1);

        chkCropThumbnails = new QCheckBox(gbResizing);
        chkCropThumbnails->setObjectName(QStringLiteral("chkCropThumbnails"));
        sizePolicy9.setHeightForWidth(chkCropThumbnails->sizePolicy().hasHeightForWidth());
        chkCropThumbnails->setSizePolicy(sizePolicy9);
        chkCropThumbnails->setFont(font3);

        gridLayout_12->addWidget(chkCropThumbnails, 6, 0, 1, 5);

        sbThumbnailHeight = new QSpinBox(gbResizing);
        sbThumbnailHeight->setObjectName(QStringLiteral("sbThumbnailHeight"));
        sizePolicy9.setHeightForWidth(sbThumbnailHeight->sizePolicy().hasHeightForWidth());
        sbThumbnailHeight->setSizePolicy(sizePolicy9);
        sbThumbnailHeight->setFont(font3);
        sbThumbnailHeight->setMinimum(50);
        sbThumbnailHeight->setMaximum(8192);
        sbThumbnailHeight->setSingleStep(10);
        sbThumbnailHeight->setValue(400);

        gridLayout_12->addWidget(sbThumbnailHeight, 4, 4, 1, 1);

        chkDoNotEnlarge = new QCheckBox(gbResizing);
        chkDoNotEnlarge->setObjectName(QStringLiteral("chkDoNotEnlarge"));
        sizePolicy9.setHeightForWidth(chkDoNotEnlarge->sizePolicy().hasHeightForWidth());
        chkDoNotEnlarge->setSizePolicy(sizePolicy9);
        chkDoNotEnlarge->setFont(font3);
        chkDoNotEnlarge->setChecked(true);

        gridLayout_12->addWidget(chkDoNotEnlarge, 2, 0, 1, 4);

        sbThumbnailWidth = new QSpinBox(gbResizing);
        sbThumbnailWidth->setObjectName(QStringLiteral("sbThumbnailWidth"));
        sbThumbnailWidth->setEnabled(false);
        sizePolicy9.setHeightForWidth(sbThumbnailWidth->sizePolicy().hasHeightForWidth());
        sbThumbnailWidth->setSizePolicy(sizePolicy9);
        sbThumbnailWidth->setFont(font3);
        sbThumbnailWidth->setMinimum(50);
        sbThumbnailWidth->setMaximum(8192);
        sbThumbnailWidth->setSingleStep(10);
        sbThumbnailWidth->setValue(600);

        gridLayout_12->addWidget(sbThumbnailWidth, 4, 1, 1, 2);

        label = new QLabel(gbResizing);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy10.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy10);
        label->setFont(font3);

        gridLayout_12->addWidget(label, 1, 0, 1, 1);

        horizontalSpacer_18 = new QSpacerItem(76, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_12->addItem(horizontalSpacer_18, 4, 5, 1, 1);


        verticalLayout_10->addWidget(gbResizing);

        gbImageDecoration = new QGroupBox(pgImageDecor);
        gbImageDecoration->setObjectName(QStringLiteral("gbImageDecoration"));
        gbImageDecoration->setMinimumSize(QSize(0, 70));
        gbImageDecoration->setFont(font4);
        horizontalLayout_15 = new QHBoxLayout(gbImageDecoration);
        horizontalLayout_15->setSpacing(6);
        horizontalLayout_15->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label_74 = new QLabel(gbImageDecoration);
        label_74->setObjectName(QStringLiteral("label_74"));
        label_74->setFont(font3);

        horizontalLayout_15->addWidget(label_74);

        cbImageBorderStyle = new QComboBox(gbImageDecoration);
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->addItem(QString());
        cbImageBorderStyle->setObjectName(QStringLiteral("cbImageBorderStyle"));
        cbImageBorderStyle->setFont(font3);

        horizontalLayout_15->addWidget(cbImageBorderStyle);

        label_14 = new QLabel(gbImageDecoration);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setFont(font3);

        horizontalLayout_15->addWidget(label_14);

        sbImageBorderWidth = new QSpinBox(gbImageDecoration);
        sbImageBorderWidth->setObjectName(QStringLiteral("sbImageBorderWidth"));
        sbImageBorderWidth->setMinimumSize(QSize(0, 24));
        sbImageBorderWidth->setFont(font3);
        sbImageBorderWidth->setMinimum(1);

        horizontalLayout_15->addWidget(sbImageBorderWidth);

        label_75 = new QLabel(gbImageDecoration);
        label_75->setObjectName(QStringLiteral("label_75"));
        label_75->setFont(font3);

        horizontalLayout_15->addWidget(label_75);

        sbImageBorderRadius = new QSpinBox(gbImageDecoration);
        sbImageBorderRadius->setObjectName(QStringLiteral("sbImageBorderRadius"));
        sbImageBorderRadius->setFont(font3);

        horizontalLayout_15->addWidget(sbImageBorderRadius);

        btnImageBorderColor = new QToolButton(gbImageDecoration);
        btnImageBorderColor->setObjectName(QStringLiteral("btnImageBorderColor"));
        QFont font11;
        font11.setFamily(QStringLiteral("MS Shell Dlg 2"));
        font11.setPointSize(8);
        font11.setBold(false);
        font11.setWeight(50);
        btnImageBorderColor->setFont(font11);
        btnImageBorderColor->setStyleSheet(QLatin1String("QToolButton { background-color:#ffff00;}\n"
"                         "));

        horizontalLayout_15->addWidget(btnImageBorderColor);


        verticalLayout_10->addWidget(gbImageDecoration);

        gbImageDecoration_2 = new QGroupBox(pgImageDecor);
        gbImageDecoration_2->setObjectName(QStringLiteral("gbImageDecoration_2"));
        gbImageDecoration_2->setMinimumSize(QSize(0, 70));
        horizontalLayout_17 = new QHBoxLayout(gbImageDecoration_2);
        horizontalLayout_17->setSpacing(6);
        horizontalLayout_17->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_17->setObjectName(QStringLiteral("horizontalLayout_17"));
        label_25 = new QLabel(gbImageDecoration_2);
        label_25->setObjectName(QStringLiteral("label_25"));
        label_25->setFont(font3);

        horizontalLayout_17->addWidget(label_25);

        sbImageMatteWidth = new QSpinBox(gbImageDecoration_2);
        sbImageMatteWidth->setObjectName(QStringLiteral("sbImageMatteWidth"));
        sbImageMatteWidth->setMinimumSize(QSize(0, 24));
        sbImageMatteWidth->setFont(font3);
        sbImageMatteWidth->setMinimum(0);
        sbImageMatteWidth->setValue(0);

        horizontalLayout_17->addWidget(sbImageMatteWidth);

        label_76 = new QLabel(gbImageDecoration_2);
        label_76->setObjectName(QStringLiteral("label_76"));
        label_76->setMinimumSize(QSize(38, 0));
        label_76->setFont(font3);

        horizontalLayout_17->addWidget(label_76);

        btnImageMatteColor = new QToolButton(gbImageDecoration_2);
        btnImageMatteColor->setObjectName(QStringLiteral("btnImageMatteColor"));
        btnImageMatteColor->setFont(font11);
        btnImageMatteColor->setStyleSheet(QLatin1String("QToolButton { background-color:#ddd;}\n"
"                         "));

        horizontalLayout_17->addWidget(btnImageMatteColor);

        label_96 = new QLabel(gbImageDecoration_2);
        label_96->setObjectName(QStringLiteral("label_96"));
        label_96->setFont(font3);

        horizontalLayout_17->addWidget(label_96);

        sbImageMatteRadius = new QSpinBox(gbImageDecoration_2);
        sbImageMatteRadius->setObjectName(QStringLiteral("sbImageMatteRadius"));
        sbImageMatteRadius->setFont(font3);

        horizontalLayout_17->addWidget(sbImageMatteRadius);

        horizontalSpacer_11 = new QSpacerItem(120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_17->addItem(horizontalSpacer_11);


        verticalLayout_10->addWidget(gbImageDecoration_2);

        gbImageDecoration_3 = new QGroupBox(pgImageDecor);
        gbImageDecoration_3->setObjectName(QStringLiteral("gbImageDecoration_3"));
        gbImageDecoration_3->setMinimumSize(QSize(0, 70));
        horizontalLayout_24 = new QHBoxLayout(gbImageDecoration_3);
        horizontalLayout_24->setSpacing(6);
        horizontalLayout_24->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_24->setObjectName(QStringLiteral("horizontalLayout_24"));
        label_77 = new QLabel(gbImageDecoration_3);
        label_77->setObjectName(QStringLiteral("label_77"));
        label_77->setFont(font3);

        horizontalLayout_24->addWidget(label_77);

        sbAlbumMatteWidth = new QSpinBox(gbImageDecoration_3);
        sbAlbumMatteWidth->setObjectName(QStringLiteral("sbAlbumMatteWidth"));
        sbAlbumMatteWidth->setMinimumSize(QSize(0, 24));
        sbAlbumMatteWidth->setFont(font3);
        sbAlbumMatteWidth->setMinimum(0);
        sbAlbumMatteWidth->setValue(0);

        horizontalLayout_24->addWidget(sbAlbumMatteWidth);

        label_78 = new QLabel(gbImageDecoration_3);
        label_78->setObjectName(QStringLiteral("label_78"));
        label_78->setMinimumSize(QSize(38, 0));
        label_78->setFont(font3);

        horizontalLayout_24->addWidget(label_78);

        btnAlbumMatteColor = new QToolButton(gbImageDecoration_3);
        btnAlbumMatteColor->setObjectName(QStringLiteral("btnAlbumMatteColor"));
        btnAlbumMatteColor->setFont(font11);
        btnAlbumMatteColor->setStyleSheet(QLatin1String("QToolButton { background-color:#ddd;}\n"
"                         "));

        horizontalLayout_24->addWidget(btnAlbumMatteColor);

        label_97 = new QLabel(gbImageDecoration_3);
        label_97->setObjectName(QStringLiteral("label_97"));
        label_97->setFont(font3);

        horizontalLayout_24->addWidget(label_97);

        sbAlbumMatteRadius = new QSpinBox(gbImageDecoration_3);
        sbAlbumMatteRadius->setObjectName(QStringLiteral("sbAlbumMatteRadius"));
        sbAlbumMatteRadius->setFont(font3);

        horizontalLayout_24->addWidget(sbAlbumMatteRadius);

        horizontalSpacer_19 = new QSpacerItem(120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_24->addItem(horizontalSpacer_19);


        verticalLayout_10->addWidget(gbImageDecoration_3);

        verticalSpacer_2 = new QSpacerItem(20, 608, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_2);

        toolBox->addItem(pgImageDecor, QStringLiteral("Images"));
        pgWatermark = new QWidget();
        pgWatermark->setObjectName(QStringLiteral("pgWatermark"));
        pgWatermark->setGeometry(QRect(0, 0, 100, 30));
        verticalLayoutWidget = new QWidget(pgWatermark);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 428, 474));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setSizeConstraint(QLayout::SetMinimumSize);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        gbWatermark = new QWidget(verticalLayoutWidget);
        gbWatermark->setObjectName(QStringLiteral("gbWatermark"));
        sizePolicy.setHeightForWidth(gbWatermark->sizePolicy().hasHeightForWidth());
        gbWatermark->setSizePolicy(sizePolicy);
        gbWatermark->setMinimumSize(QSize(0, 80));
        gbWatermark->setFont(font4);
        gridLayout_10 = new QGridLayout(gbWatermark);
        gridLayout_10->setSpacing(6);
        gridLayout_10->setContentsMargins(11, 11, 11, 11);
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));
        label_34 = new QLabel(gbWatermark);
        label_34->setObjectName(QStringLiteral("label_34"));
        label_34->setFont(font3);

        gridLayout_10->addWidget(label_34, 1, 0, 1, 1);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_10->addItem(horizontalSpacer_10, 1, 2, 1, 1);

        chkUseWM = new QCheckBox(gbWatermark);
        chkUseWM->setObjectName(QStringLiteral("chkUseWM"));
        chkUseWM->setFont(font3);

        gridLayout_10->addWidget(chkUseWM, 0, 0, 1, 1);

        label_29 = new QLabel(gbWatermark);
        label_29->setObjectName(QStringLiteral("label_29"));
        label_29->setFont(font3);

        gridLayout_10->addWidget(label_29, 1, 3, 1, 1);

        sbWmOpacity = new QSpinBox(gbWatermark);
        sbWmOpacity->setObjectName(QStringLiteral("sbWmOpacity"));
        sbWmOpacity->setFont(font3);
        sbWmOpacity->setMaximum(100);

        gridLayout_10->addWidget(sbWmOpacity, 1, 1, 1, 1);

        label_35 = new QLabel(gbWatermark);
        label_35->setObjectName(QStringLiteral("label_35"));
        label_35->setFont(font3);

        gridLayout_10->addWidget(label_35, 2, 0, 1, 1);

        btnWmColor = new QToolButton(gbWatermark);
        btnWmColor->setObjectName(QStringLiteral("btnWmColor"));
        btnWmColor->setCursor(QCursor(Qt::PointingHandCursor));
        btnWmColor->setStyleSheet(QStringLiteral("QToolButton {background-color:#fff}"));

        gridLayout_10->addWidget(btnWmColor, 1, 4, 1, 1);

        edtWatermark = new QLineEdit(gbWatermark);
        edtWatermark->setObjectName(QStringLiteral("edtWatermark"));
        edtWatermark->setMinimumSize(QSize(0, 24));
        edtWatermark->setFont(font3);

        gridLayout_10->addWidget(edtWatermark, 2, 1, 1, 4);


        verticalLayout_3->addWidget(gbWatermark);

        lblWmSample = new QLabel(verticalLayoutWidget);
        lblWmSample->setObjectName(QStringLiteral("lblWmSample"));
        sizePolicy.setHeightForWidth(lblWmSample->sizePolicy().hasHeightForWidth());
        lblWmSample->setSizePolicy(sizePolicy);
        QFont font12;
        font12.setFamily(QStringLiteral("Monotype Corsiva"));
        font12.setPointSize(28);
        font12.setBold(true);
        font12.setWeight(75);
        lblWmSample->setFont(font12);

        verticalLayout_3->addWidget(lblWmSample);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_4);

        gbWatermarkFont = new QGroupBox(verticalLayoutWidget);
        gbWatermarkFont->setObjectName(QStringLiteral("gbWatermarkFont"));
        gbWatermarkFont->setFont(font4);
        horizontalLayout_3 = new QHBoxLayout(gbWatermarkFont);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        btnSelectWmFont = new QToolButton(gbWatermarkFont);
        btnSelectWmFont->setObjectName(QStringLiteral("btnSelectWmFont"));
        QSizePolicy sizePolicy11(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy11.setHorizontalStretch(4);
        sizePolicy11.setVerticalStretch(0);
        sizePolicy11.setHeightForWidth(btnSelectWmFont->sizePolicy().hasHeightForWidth());
        btnSelectWmFont->setSizePolicy(sizePolicy11);

        horizontalLayout_3->addWidget(btnSelectWmFont);

        lblWaterMarkFont = new QLabel(gbWatermarkFont);
        lblWaterMarkFont->setObjectName(QStringLiteral("lblWaterMarkFont"));
        sizePolicy.setHeightForWidth(lblWaterMarkFont->sizePolicy().hasHeightForWidth());
        lblWaterMarkFont->setSizePolicy(sizePolicy);
        lblWaterMarkFont->setFont(font3);

        horizontalLayout_3->addWidget(lblWaterMarkFont);


        verticalLayout_3->addWidget(gbWatermarkFont);

        gbPosition = new QGroupBox(verticalLayoutWidget);
        gbPosition->setObjectName(QStringLiteral("gbPosition"));
        gbPosition->setMinimumSize(QSize(0, 110));
        gbPosition->setMaximumSize(QSize(430, 16777215));
        gbPosition->setFont(font4);
        horizontalLayout_12 = new QHBoxLayout(gbPosition);
        horizontalLayout_12->setSpacing(6);
        horizontalLayout_12->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
        gridLayout_11 = new QGridLayout();
        gridLayout_11->setSpacing(6);
        gridLayout_11->setObjectName(QStringLiteral("gridLayout_11"));
        edtWmVertMargin = new QLineEdit(gbPosition);
        edtWmVertMargin->setObjectName(QStringLiteral("edtWmVertMargin"));
        QSizePolicy sizePolicy12(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy12.setHorizontalStretch(0);
        sizePolicy12.setVerticalStretch(0);
        sizePolicy12.setHeightForWidth(edtWmVertMargin->sizePolicy().hasHeightForWidth());
        edtWmVertMargin->setSizePolicy(sizePolicy12);
        edtWmVertMargin->setMaximumSize(QSize(50, 16777215));
        edtWmVertMargin->setFont(font3);

        gridLayout_11->addWidget(edtWmVertMargin, 2, 1, 1, 1);

        label_12 = new QLabel(gbPosition);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setFont(font3);

        gridLayout_11->addWidget(label_12, 2, 0, 1, 1);

        label_13 = new QLabel(gbPosition);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setFont(font3);

        gridLayout_11->addWidget(label_13, 2, 2, 1, 1);

        cbWmVPosition = new QComboBox(gbPosition);
        cbWmVPosition->addItem(QString());
        cbWmVPosition->addItem(QString());
        cbWmVPosition->addItem(QString());
        cbWmVPosition->setObjectName(QStringLiteral("cbWmVPosition"));
        sizePolicy2.setHeightForWidth(cbWmVPosition->sizePolicy().hasHeightForWidth());
        cbWmVPosition->setSizePolicy(sizePolicy2);
        cbWmVPosition->setFont(font3);

        gridLayout_11->addWidget(cbWmVPosition, 0, 0, 1, 3);


        horizontalLayout_12->addLayout(gridLayout_11);

        gridLayout_21 = new QGridLayout();
        gridLayout_21->setSpacing(6);
        gridLayout_21->setObjectName(QStringLiteral("gridLayout_21"));
        cbWmHPosition = new QComboBox(gbPosition);
        cbWmHPosition->addItem(QString());
        cbWmHPosition->addItem(QString());
        cbWmHPosition->addItem(QString());
        cbWmHPosition->setObjectName(QStringLiteral("cbWmHPosition"));
        sizePolicy2.setHeightForWidth(cbWmHPosition->sizePolicy().hasHeightForWidth());
        cbWmHPosition->setSizePolicy(sizePolicy2);
        cbWmHPosition->setFont(font3);

        gridLayout_21->addWidget(cbWmHPosition, 0, 0, 1, 3);

        label_60 = new QLabel(gbPosition);
        label_60->setObjectName(QStringLiteral("label_60"));
        label_60->setFont(font3);

        gridLayout_21->addWidget(label_60, 1, 0, 1, 1);

        edtWmHorizMargin = new QLineEdit(gbPosition);
        edtWmHorizMargin->setObjectName(QStringLiteral("edtWmHorizMargin"));
        sizePolicy12.setHeightForWidth(edtWmHorizMargin->sizePolicy().hasHeightForWidth());
        edtWmHorizMargin->setSizePolicy(sizePolicy12);
        edtWmHorizMargin->setMaximumSize(QSize(50, 16777215));
        edtWmHorizMargin->setFont(font3);

        gridLayout_21->addWidget(edtWmHorizMargin, 1, 1, 1, 1);

        label_71 = new QLabel(gbPosition);
        label_71->setObjectName(QStringLiteral("label_71"));
        label_71->setFont(font3);

        gridLayout_21->addWidget(label_71, 1, 2, 1, 1);


        horizontalLayout_12->addLayout(gridLayout_21);


        verticalLayout_3->addWidget(gbPosition);

        gbWmTextShadow = new QGroupBox(verticalLayoutWidget);
        gbWmTextShadow->setObjectName(QStringLiteral("gbWmTextShadow"));
        gridLayout_8 = new QGridLayout(gbWmTextShadow);
        gridLayout_8->setSpacing(6);
        gridLayout_8->setContentsMargins(11, 11, 11, 11);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        chkUseWMShadow = new QCheckBox(gbWmTextShadow);
        chkUseWMShadow->setObjectName(QStringLiteral("chkUseWMShadow"));
        chkUseWMShadow->setFont(font3);

        gridLayout_8->addWidget(chkUseWMShadow, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(339, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_8->addItem(horizontalSpacer, 0, 1, 1, 7);

        label_16 = new QLabel(gbWmTextShadow);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setFont(font3);

        gridLayout_8->addWidget(label_16, 1, 0, 1, 1);

        sbWmShadowHoriz = new QSpinBox(gbWmTextShadow);
        sbWmShadowHoriz->setObjectName(QStringLiteral("sbWmShadowHoriz"));
        sizePolicy9.setHeightForWidth(sbWmShadowHoriz->sizePolicy().hasHeightForWidth());
        sbWmShadowHoriz->setSizePolicy(sizePolicy9);
        sbWmShadowHoriz->setMinimumSize(QSize(0, 24));
        sbWmShadowHoriz->setFont(font3);

        gridLayout_8->addWidget(sbWmShadowHoriz, 1, 1, 1, 1);

        label_40 = new QLabel(gbWmTextShadow);
        label_40->setObjectName(QStringLiteral("label_40"));
        label_40->setFont(font3);

        gridLayout_8->addWidget(label_40, 1, 2, 1, 1);

        sbWmShadowVert = new QSpinBox(gbWmTextShadow);
        sbWmShadowVert->setObjectName(QStringLiteral("sbWmShadowVert"));
        sizePolicy9.setHeightForWidth(sbWmShadowVert->sizePolicy().hasHeightForWidth());
        sbWmShadowVert->setSizePolicy(sizePolicy9);
        sbWmShadowVert->setMinimumSize(QSize(0, 24));
        sbWmShadowVert->setFont(font3);

        gridLayout_8->addWidget(sbWmShadowVert, 1, 3, 1, 1);

        label_41 = new QLabel(gbWmTextShadow);
        label_41->setObjectName(QStringLiteral("label_41"));
        label_41->setFont(font3);

        gridLayout_8->addWidget(label_41, 1, 4, 1, 1);

        sbWmShadowBlur = new QSpinBox(gbWmTextShadow);
        sbWmShadowBlur->setObjectName(QStringLiteral("sbWmShadowBlur"));
        sizePolicy9.setHeightForWidth(sbWmShadowBlur->sizePolicy().hasHeightForWidth());
        sbWmShadowBlur->setSizePolicy(sizePolicy9);
        sbWmShadowBlur->setMinimumSize(QSize(0, 24));
        sbWmShadowBlur->setFont(font3);

        gridLayout_8->addWidget(sbWmShadowBlur, 1, 5, 1, 1);

        label_52 = new QLabel(gbWmTextShadow);
        label_52->setObjectName(QStringLiteral("label_52"));
        label_52->setFont(font3);

        gridLayout_8->addWidget(label_52, 1, 6, 1, 1);

        btnWmShadowColor = new QToolButton(gbWmTextShadow);
        btnWmShadowColor->setObjectName(QStringLiteral("btnWmShadowColor"));
        btnWmShadowColor->setMinimumSize(QSize(0, 24));
        btnWmShadowColor->setFont(font9);
        btnWmShadowColor->setCursor(QCursor(Qt::PointingHandCursor));
        btnWmShadowColor->setStyleSheet(QStringLiteral("QToolButton {background-color:#ddd}"));

        gridLayout_8->addWidget(btnWmShadowColor, 1, 7, 1, 1);


        verticalLayout_3->addWidget(gbWmTextShadow);

        label_48 = new QLabel(pgWatermark);
        label_48->setObjectName(QStringLiteral("label_48"));
        label_48->setGeometry(QRect(20, 490, 391, 131));
        label_48->setFont(font3);
        toolBox->addItem(pgWatermark, QStringLiteral("Watermark"));

        horizontalLayout_2->addWidget(toolBox);


        verticalLayout_13->addWidget(widget_2);

        widget_5 = new QWidget(widget_16);
        widget_5->setObjectName(QStringLiteral("widget_5"));
        widget_5->setMinimumSize(QSize(0, 50));
        widget_5->setMaximumSize(QSize(16777215, 40));
        horizontalLayout_11 = new QHBoxLayout(widget_5);
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        btnDisplayHint = new QToolButton(widget_5);
        btnDisplayHint->setObjectName(QStringLiteral("btnDisplayHint"));

        horizontalLayout_11->addWidget(btnDisplayHint);

        horizontalSpacer_7 = new QSpacerItem(171, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_7);

        btnReload = new QPushButton(widget_5);
        btnReload->setObjectName(QStringLiteral("btnReload"));

        horizontalLayout_11->addWidget(btnReload);

        btnResetDesign = new QPushButton(widget_5);
        btnResetDesign->setObjectName(QStringLiteral("btnResetDesign"));

        horizontalLayout_11->addWidget(btnResetDesign);


        verticalLayout_13->addWidget(widget_5);


        horizontalLayout_10->addLayout(verticalLayout_13);

        designSplitter->addWidget(widget_16);
        gbSample = new QGroupBox(designSplitter);
        gbSample->setObjectName(QStringLiteral("gbSample"));
        QSizePolicy sizePolicy13(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy13.setHorizontalStretch(1);
        sizePolicy13.setVerticalStretch(0);
        sizePolicy13.setHeightForWidth(gbSample->sizePolicy().hasHeightForWidth());
        gbSample->setSizePolicy(sizePolicy13);
        gbSample->setMinimumSize(QSize(0, 689));
        gbSample->setFont(font2);
        gbSampleLayout = new QVBoxLayout(gbSample);
        gbSampleLayout->setSpacing(0);
        gbSampleLayout->setContentsMargins(11, 11, 11, 11);
        gbSampleLayout->setObjectName(QStringLiteral("gbSampleLayout"));
        gbSampleLayout->setContentsMargins(0, 0, 0, 0);
        sample = new QWebEngineView(gbSample);
        sample->setObjectName(QStringLiteral("sample"));
        sample->setUrl(QUrl(QStringLiteral("qrc:/Preview/Resources/index_en.html")));

        gbSampleLayout->addWidget(sample);

        designSplitter->addWidget(gbSample);

        horizontalLayout_19->addWidget(designSplitter);


        horizontalLayout_20->addWidget(widget_15);

        tabFalconG->addTab(tabDesign, QString());
        tabEdit = new QWidget();
        tabEdit->setObjectName(QStringLiteral("tabEdit"));
        sizePolicy.setHeightForWidth(tabEdit->sizePolicy().hasHeightForWidth());
        tabEdit->setSizePolicy(sizePolicy);
        tabEditLayout = new QVBoxLayout(tabEdit);
        tabEditLayout->setSpacing(0);
        tabEditLayout->setContentsMargins(11, 11, 11, 11);
        tabEditLayout->setObjectName(QStringLiteral("tabEditLayout"));
        tabEditLayout->setContentsMargins(0, 0, 0, 0);
        editSplitter = new QSplitter(tabEdit);
        editSplitter->setObjectName(QStringLiteral("editSplitter"));
        sizePolicy3.setHeightForWidth(editSplitter->sizePolicy().hasHeightForWidth());
        editSplitter->setSizePolicy(sizePolicy3);
        editSplitter->setOrientation(Qt::Vertical);
        editSplitter->setHandleWidth(10);
        widget_3 = new QWidget(editSplitter);
        widget_3->setObjectName(QStringLiteral("widget_3"));
        sizePolicy3.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy3);
        verticalLayout_19 = new QVBoxLayout(widget_3);
        verticalLayout_19->setSpacing(6);
        verticalLayout_19->setContentsMargins(11, 11, 11, 11);
        verticalLayout_19->setObjectName(QStringLiteral("verticalLayout_19"));
        verticalLayout_19->setContentsMargins(9, 0, 9, 0);
        splitter = new QSplitter(widget_3);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        widget_4 = new QWidget(splitter);
        widget_4->setObjectName(QStringLiteral("widget_4"));
        QSizePolicy sizePolicy14(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy14.setHorizontalStretch(0);
        sizePolicy14.setVerticalStretch(0);
        sizePolicy14.setHeightForWidth(widget_4->sizePolicy().hasHeightForWidth());
        widget_4->setSizePolicy(sizePolicy14);
        verticalLayout_9 = new QVBoxLayout(widget_4);
        verticalLayout_9->setSpacing(6);
        verticalLayout_9->setContentsMargins(11, 11, 11, 11);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(1, 1, 1, 1);
        editAlbumsLayout = new QVBoxLayout();
        editAlbumsLayout->setSpacing(6);
        editAlbumsLayout->setObjectName(QStringLiteral("editAlbumsLayout"));
        label_51 = new QLabel(widget_4);
        label_51->setObjectName(QStringLiteral("label_51"));
        sizePolicy.setHeightForWidth(label_51->sizePolicy().hasHeightForWidth());
        label_51->setSizePolicy(sizePolicy);
        label_51->setFont(font2);

        editAlbumsLayout->addWidget(label_51);

        trvAlbums = new QTreeView(widget_4);
        trvAlbums->setObjectName(QStringLiteral("trvAlbums"));

        editAlbumsLayout->addWidget(trvAlbums);

        lblTotalCount = new QLabel(widget_4);
        lblTotalCount->setObjectName(QStringLiteral("lblTotalCount"));

        editAlbumsLayout->addWidget(lblTotalCount);


        verticalLayout_9->addLayout(editAlbumsLayout);

        splitter->addWidget(widget_4);
        widget_6 = new QWidget(splitter);
        widget_6->setObjectName(QStringLiteral("widget_6"));
        sizePolicy4.setHeightForWidth(widget_6->sizePolicy().hasHeightForWidth());
        widget_6->setSizePolicy(sizePolicy4);
        verticalLayout_11 = new QVBoxLayout(widget_6);
        verticalLayout_11->setSpacing(6);
        verticalLayout_11->setContentsMargins(11, 11, 11, 11);
        verticalLayout_11->setObjectName(QStringLiteral("verticalLayout_11"));
        verticalLayout_11->setContentsMargins(1, 1, 1, 1);
        editImagesLayout = new QVBoxLayout();
        editImagesLayout->setSpacing(6);
        editImagesLayout->setObjectName(QStringLiteral("editImagesLayout"));
        label_53 = new QLabel(widget_6);
        label_53->setObjectName(QStringLiteral("label_53"));
        sizePolicy.setHeightForWidth(label_53->sizePolicy().hasHeightForWidth());
        label_53->setSizePolicy(sizePolicy);
        label_53->setFont(font2);

        editImagesLayout->addWidget(label_53);

        tnvImages = new ThumbnailWidget(widget_6);
        tnvImages->setObjectName(QStringLiteral("tnvImages"));
        tnvImages->setViewMode(QListView::IconMode);

        editImagesLayout->addWidget(tnvImages);

        lblImageCount = new QLabel(widget_6);
        lblImageCount->setObjectName(QStringLiteral("lblImageCount"));
        lblImageCount->setAlignment(Qt::AlignCenter);

        editImagesLayout->addWidget(lblImageCount);


        verticalLayout_11->addLayout(editImagesLayout);

        splitter->addWidget(widget_6);

        verticalLayout_19->addWidget(splitter);

        editSplitter->addWidget(widget_3);
        widget_7 = new QWidget(editSplitter);
        widget_7->setObjectName(QStringLiteral("widget_7"));
        sizePolicy3.setHeightForWidth(widget_7->sizePolicy().hasHeightForWidth());
        widget_7->setSizePolicy(sizePolicy3);
        verticalLayout_18 = new QVBoxLayout(widget_7);
        verticalLayout_18->setSpacing(6);
        verticalLayout_18->setContentsMargins(11, 11, 11, 11);
        verticalLayout_18->setObjectName(QStringLiteral("verticalLayout_18"));
        verticalLayout_18->setContentsMargins(9, 0, 9, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        cbBaseLanguage = new QComboBox(widget_7);
        cbBaseLanguage->setObjectName(QStringLiteral("cbBaseLanguage"));
        sizePolicy2.setHeightForWidth(cbBaseLanguage->sizePolicy().hasHeightForWidth());
        cbBaseLanguage->setSizePolicy(sizePolicy2);

        verticalLayout_5->addWidget(cbBaseLanguage);

        label_54 = new QLabel(widget_7);
        label_54->setObjectName(QStringLiteral("label_54"));
        sizePolicy1.setHeightForWidth(label_54->sizePolicy().hasHeightForWidth());
        label_54->setSizePolicy(sizePolicy1);
        label_54->setFont(font2);

        verticalLayout_5->addWidget(label_54);

        edtBaseTitle = new QTextEdit(widget_7);
        edtBaseTitle->setObjectName(QStringLiteral("edtBaseTitle"));
        edtBaseTitle->setEnabled(true);
        edtBaseTitle->setStyleSheet(QStringLiteral("QtextEdit { color: gray; background-color:light-grey}"));
        edtBaseTitle->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_5->addWidget(edtBaseTitle);


        horizontalLayout_6->addLayout(verticalLayout_5);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(1);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        cbLanguage = new QComboBox(widget_7);
        cbLanguage->setObjectName(QStringLiteral("cbLanguage"));
        sizePolicy2.setHeightForWidth(cbLanguage->sizePolicy().hasHeightForWidth());
        cbLanguage->setSizePolicy(sizePolicy2);

        verticalLayout_6->addWidget(cbLanguage);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_10 = new QLabel(widget_7);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setFont(font2);

        horizontalLayout_7->addWidget(label_10);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_3);

        btnSaveChangedTitle = new QPushButton(widget_7);
        btnSaveChangedTitle->setObjectName(QStringLiteral("btnSaveChangedTitle"));
        btnSaveChangedTitle->setEnabled(false);

        horizontalLayout_7->addWidget(btnSaveChangedTitle);

        chkChangeTitleEverywhere = new QCheckBox(widget_7);
        chkChangeTitleEverywhere->setObjectName(QStringLiteral("chkChangeTitleEverywhere"));

        horizontalLayout_7->addWidget(chkChangeTitleEverywhere);


        verticalLayout_6->addLayout(horizontalLayout_7);

        edtTitleText = new QTextEdit(widget_7);
        edtTitleText->setObjectName(QStringLiteral("edtTitleText"));

        verticalLayout_6->addWidget(edtTitleText);


        horizontalLayout_6->addLayout(verticalLayout_6);


        verticalLayout_18->addLayout(horizontalLayout_6);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        verticalLayout_7->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout_7->setContentsMargins(-1, 6, -1, -1);
        label_11 = new QLabel(widget_7);
        label_11->setObjectName(QStringLiteral("label_11"));
        sizePolicy1.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy1);
        label_11->setFont(font2);

        verticalLayout_7->addWidget(label_11);

        edtBaseDescription = new QTextEdit(widget_7);
        edtBaseDescription->setObjectName(QStringLiteral("edtBaseDescription"));
        edtBaseDescription->setEnabled(true);
        edtBaseDescription->setStyleSheet(QStringLiteral("QtextEdit { color: gray; background-color:light-grey}"));
        edtBaseDescription->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_7->addWidget(edtBaseDescription);


        horizontalLayout_8->addLayout(verticalLayout_7);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setSpacing(1);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        label_55 = new QLabel(widget_7);
        label_55->setObjectName(QStringLiteral("label_55"));
        sizePolicy1.setHeightForWidth(label_55->sizePolicy().hasHeightForWidth());
        label_55->setSizePolicy(sizePolicy1);
        label_55->setFont(font2);

        horizontalLayout_9->addWidget(label_55);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_8);

        btnSaveChangedDescription = new QPushButton(widget_7);
        btnSaveChangedDescription->setObjectName(QStringLiteral("btnSaveChangedDescription"));
        btnSaveChangedDescription->setEnabled(false);

        horizontalLayout_9->addWidget(btnSaveChangedDescription);

        chkChangeDescriptionEverywhere = new QCheckBox(widget_7);
        chkChangeDescriptionEverywhere->setObjectName(QStringLiteral("chkChangeDescriptionEverywhere"));

        horizontalLayout_9->addWidget(chkChangeDescriptionEverywhere);


        verticalLayout_8->addLayout(horizontalLayout_9);

        edtDescriptionText = new QTextEdit(widget_7);
        edtDescriptionText->setObjectName(QStringLiteral("edtDescriptionText"));

        verticalLayout_8->addWidget(edtDescriptionText);


        horizontalLayout_8->addLayout(verticalLayout_8);


        verticalLayout_18->addLayout(horizontalLayout_8);

        editSplitter->addWidget(widget_7);

        tabEditLayout->addWidget(editSplitter);

        tabFalconG->addTab(tabEdit, QString());
        tabOptions = new QWidget();
        tabOptions->setObjectName(QStringLiteral("tabOptions"));
        verticalLayout_4 = new QVBoxLayout(tabOptions);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        lblOptionsHelp = new QLabel(tabOptions);
        lblOptionsHelp->setObjectName(QStringLiteral("lblOptionsHelp"));

        verticalLayout_4->addWidget(lblOptionsHelp);

        widget_11 = new QWidget(tabOptions);
        widget_11->setObjectName(QStringLiteral("widget_11"));
        verticalLayout_14 = new QVBoxLayout(widget_11);
        verticalLayout_14->setSpacing(6);
        verticalLayout_14->setContentsMargins(11, 11, 11, 11);
        verticalLayout_14->setObjectName(QStringLiteral("verticalLayout_14"));
        groupBox = new QGroupBox(widget_11);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setMinimumSize(QSize(0, 60));
        groupBox->setFont(font2);
        horizontalLayout_23 = new QHBoxLayout(groupBox);
        horizontalLayout_23->setSpacing(6);
        horizontalLayout_23->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_23->setObjectName(QStringLiteral("horizontalLayout_23"));
        label_79 = new QLabel(groupBox);
        label_79->setObjectName(QStringLiteral("label_79"));
        label_79->setFont(font1);

        horizontalLayout_23->addWidget(label_79);

        rbMagyar = new QRadioButton(groupBox);
        rbMagyar->setObjectName(QStringLiteral("rbMagyar"));
        rbMagyar->setFont(font1);

        horizontalLayout_23->addWidget(rbMagyar);

        rbEnglish = new QRadioButton(groupBox);
        rbEnglish->setObjectName(QStringLiteral("rbEnglish"));
        rbEnglish->setMinimumSize(QSize(0, 0));
        rbEnglish->setFont(font1);
        rbEnglish->setChecked(true);

        horizontalLayout_23->addWidget(rbEnglish);

        horizontalSpacer_17 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_23->addItem(horizontalSpacer_17);

        btnResetDialogs = new QPushButton(groupBox);
        btnResetDialogs->setObjectName(QStringLiteral("btnResetDialogs"));
        btnResetDialogs->setMinimumSize(QSize(0, 23));
        btnResetDialogs->setFont(font1);

        horizontalLayout_23->addWidget(btnResetDialogs);


        verticalLayout_14->addWidget(groupBox);

        gbColorScheme = new QGroupBox(widget_11);
        gbColorScheme->setObjectName(QStringLiteral("gbColorScheme"));
        QFont font13;
        font13.setFamily(QStringLiteral("Arial"));
        font13.setPointSize(8);
        font13.setBold(true);
        font13.setWeight(75);
        gbColorScheme->setFont(font13);
        verticalLayout_16 = new QVBoxLayout(gbColorScheme);
        verticalLayout_16->setSpacing(2);
        verticalLayout_16->setContentsMargins(11, 11, 11, 11);
        verticalLayout_16->setObjectName(QStringLiteral("verticalLayout_16"));
        verticalLayout_16->setContentsMargins(6, -1, 6, 1);
        widget_14 = new QWidget(gbColorScheme);
        widget_14->setObjectName(QStringLiteral("widget_14"));
        horizontalLayout_18 = new QHBoxLayout(widget_14);
        horizontalLayout_18->setSpacing(6);
        horizontalLayout_18->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_18->setObjectName(QStringLiteral("horizontalLayout_18"));
        horizontalLayout_18->setContentsMargins(1, 1, 1, 1);
        pnlColorScheme = new QWidget(widget_14);
        pnlColorScheme->setObjectName(QStringLiteral("pnlColorScheme"));
        pnlColorScheme->setMaximumSize(QSize(16777215, 550));
        formLayout_4 = new QFormLayout(pnlColorScheme);
        formLayout_4->setSpacing(6);
        formLayout_4->setContentsMargins(11, 11, 11, 11);
        formLayout_4->setObjectName(QStringLiteral("formLayout_4"));
        label_49 = new QLabel(pnlColorScheme);
        label_49->setObjectName(QStringLiteral("label_49"));
        label_49->setMaximumSize(QSize(16777215, 3));
        QFont font14;
        font14.setFamily(QStringLiteral("Arial"));
        font14.setPointSize(12);
        font14.setBold(false);
        font14.setWeight(50);
        label_49->setFont(font14);

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_49);

        label_72 = new QLabel(pnlColorScheme);
        label_72->setObjectName(QStringLiteral("label_72"));
        label_72->setMaximumSize(QSize(16777215, 3));
        label_72->setFont(font14);

        formLayout_4->setWidget(0, QFormLayout::FieldRole, label_72);


        horizontalLayout_18->addWidget(pnlColorScheme);

        pnlSchemeEmulator = new QGroupBox(widget_14);
        pnlSchemeEmulator->setObjectName(QStringLiteral("pnlSchemeEmulator"));
        gridLayout_22 = new QGridLayout(pnlSchemeEmulator);
        gridLayout_22->setSpacing(6);
        gridLayout_22->setContentsMargins(11, 11, 11, 11);
        gridLayout_22->setObjectName(QStringLiteral("gridLayout_22"));
        verticalSpacer_7 = new QSpacerItem(20, 422, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_22->addItem(verticalSpacer_7, 4, 1, 1, 1);

        btnDeleteColorScheme = new QToolButton(pnlSchemeEmulator);
        btnDeleteColorScheme->setObjectName(QStringLiteral("btnDeleteColorScheme"));
        btnDeleteColorScheme->setEnabled(true);
        sizePolicy14.setHeightForWidth(btnDeleteColorScheme->sizePolicy().hasHeightForWidth());
        btnDeleteColorScheme->setSizePolicy(sizePolicy14);
        QFont font15;
        font15.setFamily(QStringLiteral("Arial"));
        font15.setPointSize(8);
        font15.setBold(false);
        font15.setWeight(50);
        btnDeleteColorScheme->setFont(font15);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/Resources/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnDeleteColorScheme->setIcon(icon1);
        btnDeleteColorScheme->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        gridLayout_22->addWidget(btnDeleteColorScheme, 2, 3, 1, 1);

        lwColorScheme = new QListWidget(pnlSchemeEmulator);
        lwColorScheme->setObjectName(QStringLiteral("lwColorScheme"));

        gridLayout_22->addWidget(lwColorScheme, 1, 0, 7, 1);

        btnAddAndGenerateColorScheme = new QPushButton(pnlSchemeEmulator);
        btnAddAndGenerateColorScheme->setObjectName(QStringLiteral("btnAddAndGenerateColorScheme"));
        btnAddAndGenerateColorScheme->setFont(font15);

        gridLayout_22->addWidget(btnAddAndGenerateColorScheme, 2, 1, 1, 1);

        btnApplyColorScheme = new QPushButton(pnlSchemeEmulator);
        btnApplyColorScheme->setObjectName(QStringLiteral("btnApplyColorScheme"));
        btnApplyColorScheme->setEnabled(false);
        btnApplyColorScheme->setFont(font13);

        gridLayout_22->addWidget(btnApplyColorScheme, 3, 1, 1, 1);

        btnResetColorScheme = new QPushButton(pnlSchemeEmulator);
        btnResetColorScheme->setObjectName(QStringLiteral("btnResetColorScheme"));
        btnResetColorScheme->setEnabled(false);
        btnResetColorScheme->setFont(font13);

        gridLayout_22->addWidget(btnResetColorScheme, 7, 3, 1, 1);

        btnMoveSchemeUp = new QPushButton(pnlSchemeEmulator);
        btnMoveSchemeUp->setObjectName(QStringLiteral("btnMoveSchemeUp"));
        btnMoveSchemeUp->setEnabled(false);
        btnMoveSchemeUp->setFont(font13);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/Resources/moveup-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveSchemeUp->setIcon(icon2);

        gridLayout_22->addWidget(btnMoveSchemeUp, 5, 1, 1, 1);

        btnMoveSchemeDown = new QPushButton(pnlSchemeEmulator);
        btnMoveSchemeDown->setObjectName(QStringLiteral("btnMoveSchemeDown"));
        btnMoveSchemeDown->setEnabled(false);
        btnMoveSchemeDown->setFont(font13);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/Resources/movedown-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveSchemeDown->setIcon(icon3);

        gridLayout_22->addWidget(btnMoveSchemeDown, 7, 1, 1, 1);


        horizontalLayout_18->addWidget(pnlSchemeEmulator);

        horizontalLayout_18->setStretch(0, 1);
        horizontalLayout_18->setStretch(1, 2);

        verticalLayout_16->addWidget(widget_14);

        verticalLayout_16->setStretch(0, 10);

        verticalLayout_14->addWidget(gbColorScheme);


        verticalLayout_4->addWidget(widget_11);

        verticalLayout_4->setStretch(0, 1);
        verticalLayout_4->setStretch(1, 5);
        tabFalconG->addTab(tabOptions, QString());

        verticalLayout_2->addWidget(tabFalconG);

        pnlButtons = new QWidget(centralWidget);
        pnlButtons->setObjectName(QStringLiteral("pnlButtons"));
        sizePolicy1.setHeightForWidth(pnlButtons->sizePolicy().hasHeightForWidth());
        pnlButtons->setSizePolicy(sizePolicy1);
        pnlButtons->setMinimumSize(QSize(0, 41));
        horizontalLayout_5 = new QHBoxLayout(pnlButtons);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        btnGenerate = new QPushButton(pnlButtons);
        btnGenerate->setObjectName(QStringLiteral("btnGenerate"));
        btnGenerate->setEnabled(false);

        horizontalLayout_5->addWidget(btnGenerate);

        chkGenerateAll = new QCheckBox(pnlButtons);
        chkGenerateAll->setObjectName(QStringLiteral("chkGenerateAll"));

        horizontalLayout_5->addWidget(chkGenerateAll);

        chkButImages = new QCheckBox(pnlButtons);
        chkButImages->setObjectName(QStringLiteral("chkButImages"));

        horizontalLayout_5->addWidget(chkButImages);

        btnPreview = new QPushButton(pnlButtons);
        btnPreview->setObjectName(QStringLiteral("btnPreview"));
        btnPreview->setEnabled(false);

        horizontalLayout_5->addWidget(btnPreview);

        horizontalSpacer_4 = new QSpacerItem(513, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);

        btnSaveStyleSheet = new QPushButton(pnlButtons);
        btnSaveStyleSheet->setObjectName(QStringLiteral("btnSaveStyleSheet"));
        btnSaveStyleSheet->setEnabled(false);

        horizontalLayout_5->addWidget(btnSaveStyleSheet);

        btnSaveConfig = new QPushButton(pnlButtons);
        btnSaveConfig->setObjectName(QStringLiteral("btnSaveConfig"));
        btnSaveConfig->setEnabled(false);

        horizontalLayout_5->addWidget(btnSaveConfig);

        chkCleanUp = new QCheckBox(pnlButtons);
        chkCleanUp->setObjectName(QStringLiteral("chkCleanUp"));

        horizontalLayout_5->addWidget(chkCleanUp);

        btnExit = new QPushButton(pnlButtons);
        btnExit->setObjectName(QStringLiteral("btnExit"));

        horizontalLayout_5->addWidget(btnExit);


        verticalLayout_2->addWidget(pnlButtons);

        falconGClass->setCentralWidget(centralWidget);
#ifndef QT_NO_SHORTCUT
        label_17->setBuddy(edtDestGallery);
        label_19->setBuddy(edtServerAddress);
        label_20->setBuddy(edtGalleryRoot);
        label_21->setBuddy(edtMainPage);
        label_42->setBuddy(edtEmailTo);
        label_30->setBuddy(sbNewDays);
        label_9->setBuddy(edtTrackingCode);
        label_56->setBuddy(edtThumb);
        label_23->setBuddy(edtThumb);
        label_22->setBuddy(edtAlbumDir);
        label_45->setBuddy(edtBaseName);
        label_44->setBuddy(edtFontDir);
        label_46->setBuddy(edtDefaultFonts);
        label_37->setBuddy(sbShadowHoriz1);
        label_38->setBuddy(sbShadowVert1);
        label_39->setBuddy(sbShadowBlur1);
        label_87->setBuddy(sbShadowBlur1);
        label_83->setBuddy(sbShadowHoriz1);
        label_84->setBuddy(sbShadowVert1);
        label_82->setBuddy(sbShadowBlur1);
        label_88->setBuddy(sbShadowBlur1);
        label_27->setBuddy(edtFontFamily);
        label_28->setBuddy(cbPointSize);
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
        QWidget::setTabOrder(edtMainPage, edtDescription);
        QWidget::setTabOrder(edtDescription, edtKeywords);
        QWidget::setTabOrder(edtKeywords, edtGalleryTitle);
        QWidget::setTabOrder(edtGalleryTitle, edtGalleryLanguages);
        QWidget::setTabOrder(edtGalleryLanguages, edtEmailTo);
        QWidget::setTabOrder(edtEmailTo, chkMenuToAbout);
        QWidget::setTabOrder(chkMenuToAbout, edtAbout);
        QWidget::setTabOrder(edtAbout, edtBackImage);
        QWidget::setTabOrder(edtBackImage, sbNewDays);
        QWidget::setTabOrder(sbNewDays, sbLatestCount);
        QWidget::setTabOrder(sbLatestCount, chkUseGoogleAnalytics);
        QWidget::setTabOrder(chkUseGoogleAnalytics, edtTrackingCode);
        QWidget::setTabOrder(edtTrackingCode, chkForceSSL);
        QWidget::setTabOrder(chkForceSSL, chkMenuToContact);
        QWidget::setTabOrder(chkMenuToContact, chkAllowLinkToImages);
        QWidget::setTabOrder(chkAllowLinkToImages, chkCanDownload);
        QWidget::setTabOrder(chkCanDownload, chkRightClickProtected);
        QWidget::setTabOrder(chkRightClickProtected, chkFacebook);
        QWidget::setTabOrder(chkFacebook, edtImg);
        QWidget::setTabOrder(edtImg, edtThumb);
        QWidget::setTabOrder(edtThumb, edtAlbumDir);
        QWidget::setTabOrder(edtAlbumDir, edtFontDir);
        QWidget::setTabOrder(edtFontDir, edtDefaultFonts);
        QWidget::setTabOrder(edtDefaultFonts, edtBaseName);
        QWidget::setTabOrder(edtBaseName, chkReadFromGallery);
        QWidget::setTabOrder(chkReadFromGallery, chkOvrImages);
        QWidget::setTabOrder(chkOvrImages, chkAddTitlesToAll);
        QWidget::setTabOrder(chkAddTitlesToAll, chkLowerCaseImageExtensions);
        QWidget::setTabOrder(chkLowerCaseImageExtensions, chkAddDescToAll);
        QWidget::setTabOrder(chkAddDescToAll, btnGenerate);
        QWidget::setTabOrder(btnGenerate, chkGenerateAll);
        QWidget::setTabOrder(chkGenerateAll, chkButImages);
        QWidget::setTabOrder(chkButImages, btnPreview);
        QWidget::setTabOrder(btnPreview, btnSaveStyleSheet);
        QWidget::setTabOrder(btnSaveStyleSheet, btnSaveConfig);
        QWidget::setTabOrder(btnSaveConfig, btnExit);
        QWidget::setTabOrder(btnExit, cbActualItem);
        QWidget::setTabOrder(cbActualItem, btnPageColor);
        QWidget::setTabOrder(btnPageColor, btnPageBackground);
        QWidget::setTabOrder(btnPageBackground, chkSetAll);
        QWidget::setTabOrder(chkSetAll, btnSelectUplinkIcon);
        QWidget::setTabOrder(btnSelectUplinkIcon, chkIconTop);
        QWidget::setTabOrder(chkIconTop, chkIconText);
        QWidget::setTabOrder(chkIconText, edtBckImageName);
        QWidget::setTabOrder(edtBckImageName, rbNoBackgroundImage);
        QWidget::setTabOrder(rbNoBackgroundImage, rbCenterBckImage);
        QWidget::setTabOrder(rbCenterBckImage, rbCoverBckImage);
        QWidget::setTabOrder(rbCoverBckImage, rbTileBckImage);
        QWidget::setTabOrder(rbTileBckImage, btnForeground);
        QWidget::setTabOrder(btnForeground, sbTextOpacity);
        QWidget::setTabOrder(sbTextOpacity, chkTextOpacity);
        QWidget::setTabOrder(chkTextOpacity, btnBackground);
        QWidget::setTabOrder(btnBackground, sbBackgroundOpacity);
        QWidget::setTabOrder(sbBackgroundOpacity, chkBackgroundOpacity);
        QWidget::setTabOrder(chkBackgroundOpacity, chkTdUnderline);
        QWidget::setTabOrder(chkTdUnderline, chkTdOverline);
        QWidget::setTabOrder(chkTdOverline, chkTdLinethrough);
        QWidget::setTabOrder(chkTdLinethrough, rbTdSolid);
        QWidget::setTabOrder(rbTdSolid, rbTdDotted);
        QWidget::setTabOrder(rbTdDotted, rbTdDashed);
        QWidget::setTabOrder(rbTdDashed, rbTdDouble);
        QWidget::setTabOrder(rbTdDouble, rbTdWavy);
        QWidget::setTabOrder(rbTdWavy, rbTextAlignNone);
        QWidget::setTabOrder(rbTextAlignNone, rbTextLeft);
        QWidget::setTabOrder(rbTextLeft, rbTextCenter);
        QWidget::setTabOrder(rbTextCenter, rbTextRight);
        QWidget::setTabOrder(rbTextRight, rbTextShadow);
        QWidget::setTabOrder(rbTextShadow, rbBoxShadow);
        QWidget::setTabOrder(rbBoxShadow, chkShadowOn);
        QWidget::setTabOrder(chkShadowOn, sbShadowHoriz1);
        QWidget::setTabOrder(sbShadowHoriz1, sbShadowVert1);
        QWidget::setTabOrder(sbShadowVert1, sbShadowBlur1);
        QWidget::setTabOrder(sbShadowBlur1, sbShadowSpread1);
        QWidget::setTabOrder(sbShadowSpread1, sbShadowHoriz2);
        QWidget::setTabOrder(sbShadowHoriz2, sbShadowVert2);
        QWidget::setTabOrder(sbShadowVert2, sbShadowBlur2);
        QWidget::setTabOrder(sbShadowBlur2, sbShadowSpread2);
        QWidget::setTabOrder(sbShadowSpread2, btnShadowColor);
        QWidget::setTabOrder(btnShadowColor, chkUseGradient);
        QWidget::setTabOrder(chkUseGradient, btnGradStartColor);
        QWidget::setTabOrder(btnGradStartColor, sbGradStartPos);
        QWidget::setTabOrder(sbGradStartPos, btnGradMiddleColor);
        QWidget::setTabOrder(btnGradMiddleColor, sbGradMiddlePos);
        QWidget::setTabOrder(sbGradMiddlePos, btnGradStopColor);
        QWidget::setTabOrder(btnGradStopColor, sbGradStopPos);
        QWidget::setTabOrder(sbGradStopPos, cbBorderStyle);
        QWidget::setTabOrder(cbBorderStyle, sbBorderRadius);
        QWidget::setTabOrder(sbBorderRadius, btnDisplayHint);
        QWidget::setTabOrder(btnDisplayHint, btnResetDesign);
        QWidget::setTabOrder(btnResetDesign, trvAlbums);
        QWidget::setTabOrder(trvAlbums, tnvImages);
        QWidget::setTabOrder(tnvImages, cbBaseLanguage);
        QWidget::setTabOrder(cbBaseLanguage, cbLanguage);
        QWidget::setTabOrder(cbLanguage, btnSaveChangedTitle);
        QWidget::setTabOrder(btnSaveChangedTitle, chkChangeTitleEverywhere);
        QWidget::setTabOrder(chkChangeTitleEverywhere, edtBaseTitle);
        QWidget::setTabOrder(edtBaseTitle, edtTitleText);
        QWidget::setTabOrder(edtTitleText, edtBaseDescription);
        QWidget::setTabOrder(edtBaseDescription, edtDescriptionText);
        QWidget::setTabOrder(edtDescriptionText, btnSaveChangedDescription);
        QWidget::setTabOrder(btnSaveChangedDescription, chkChangeDescriptionEverywhere);

        retranslateUi(falconGClass);
        QObject::connect(actionExit, SIGNAL(triggered()), falconGClass, SLOT(close()));
        QObject::connect(btnExit, SIGNAL(clicked()), falconGClass, SLOT(close()));

        tabFalconG->setCurrentIndex(0);
        toolBox->setCurrentIndex(0);
        cbPointSize->setCurrentIndex(6);
        cbLineHeight->setCurrentIndex(12);
        cbPointSizeFirstLine->setCurrentIndex(6);
        cbWmVPosition->setCurrentIndex(1);
        cbWmHPosition->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(falconGClass);
    } // setupUi

    void retranslateUi(QMainWindow *falconGClass)
    {
        falconGClass->setWindowTitle(QApplication::translate("falconGClass", "Falcon's Gallery Generator", nullptr));
        actionOpen->setText(QApplication::translate("falconGClass", "&Open", nullptr));
        actionExit->setText(QApplication::translate("falconGClass", "E&xit", nullptr));
#ifndef QT_NO_SHORTCUT
        actionExit->setShortcut(QApplication::translate("falconGClass", "Alt+X", nullptr));
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
        label_36->setText(QApplication::translate("falconGClass", "Site Description", nullptr));
        edtDescription->setPlaceholderText(QApplication::translate("falconGClass", "This will appear in search results as the description of your site. May be left empty. Maximum length is 275 characters", nullptr));
        label_61->setText(QApplication::translate("falconGClass", "Keywords", nullptr));
        edtKeywords->setPlaceholderText(QApplication::translate("falconGClass", "A list of keywords/expressions separated by commas. May be left empty.", nullptr));
        gbGallery->setTitle(QApplication::translate("falconGClass", "Gallery", nullptr));
#ifndef QT_NO_TOOLTIP
        edtBackImage->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Path name of  an image file, which will be set as the background image for the auto generated home page</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtBackImage->setPlaceholderText(QApplication::translate("falconGClass", "path to image file", nullptr));
        chkMenuToAbout->setText(QApplication::translate("falconGClass", "About page", nullptr));
        label_47->setText(QApplication::translate("falconGClass", "Title", nullptr));
        chkAllowLinkToImages->setText(QApplication::translate("falconGClass", "Allow  links to full size images", nullptr));
#ifndef QT_NO_TOOLTIP
        edtGalleryLanguages->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>List of language abbreviations (e.g. en_US or en_GB) separated by comma (,)</p><p>Texts for languages must either be in the .struct file or <br/>in files in either the program directory or in the<br/>actual gallery's source directory. Files must be named as</p><p><span style=\" font-style:italic;\">&lt;language abbreviation&gt;.text, </span></p><p>Example:<span style=\" font-style:italic;\"> en_US.text.</span></p><p>If left empty it uses all language text files it finds.</p><p>This is just the language used on the gallery pages and not the<br/>language of the program's interface!</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtGalleryLanguages->setPlaceholderText(QApplication::translate("falconGClass", " \"hu_HU,en_US\" to use hu_HU.lang and en_US.lang", nullptr));
        label_42->setText(QApplication::translate("falconGClass", "Send user emails to", nullptr));
#ifndef QT_NO_TOOLTIP
        chkMenuToContact->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Adds a <span style=\" font-style:italic;\">Contact </span> menu which opens mail to the value </p><p>in the user mail address field above.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkMenuToContact->setText(QApplication::translate("falconGClass", "Menu to Contact by email", nullptr));
        chkCanDownload->setText(QApplication::translate("falconGClass", "Allow the download of images", nullptr));
#ifndef QT_NO_TOOLTIP
        chkFacebook->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Add a link to like the albums on <span style=\" text-decoration: underline;\">facebook.</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkFacebook->setText(QApplication::translate("falconGClass", "Like on Facebook link", nullptr));
        label_62->setText(QApplication::translate("falconGClass", "Languages", nullptr));
#ifndef QT_NO_TOOLTIP
        chkRightClickProtected->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Try to protect page from downloads using the right mouse button (or Ctrl+click on Macs).</p><p><br/></p><p><span style=\" font-style:italic;\">Note: this only protects from the average users!</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkRightClickProtected->setText(QApplication::translate("falconGClass", "Right click protection", nullptr));
        label_50->setText(QApplication::translate("falconGClass", "Home page background", nullptr));
#ifndef QT_NO_TOOLTIP
        edtAbout->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Name of the <span style=\" font-weight:600;\">About</span> page. As many about pages are generated into the root, directory of the gallery as there are languages. For more than one language they will be named as &lt;base name&gt;_&lt;language abbrev&gt;.&lt;extension&gt;.</p><p>About menu is only generated when the checkbox is checked.</p><p>Example: default name 'about' then with two languages: English (en) and Hungarian (hu).File names will be <span style=\" font-style:italic;\">abut_en.html</span> and <span style=\" font-style:italic;\">about_hu.html.</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtAbout->setPlaceholderText(QApplication::translate("falconGClass", "about.html", nullptr));
#ifndef QT_NO_TOOLTIP
        chkMenuToDescriptions->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Adds menu to turn image descriptions on and off.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkMenuToDescriptions->setText(QApplication::translate("falconGClass", "Menu to toggle descriptions", nullptr));
#ifndef QT_NO_TOOLTIP
        edtGalleryTitle->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        edtGalleryTitle->setPlaceholderText(QApplication::translate("falconGClass", "Andreas Falco Photography", nullptr));
#ifndef QT_NO_TOOLTIP
        chkForceSSL->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Request the pages to be safe to access.</p><p>Without it links will not be not be protected.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkForceSSL->setText(QApplication::translate("falconGClass", "Force secure (HTTPS) access", nullptr));
#ifndef QT_NO_TOOLTIP
        edtEmailTo->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Enter your email address here and check the '<span style=\" font-style:italic;\">Menu to Contacts</span>' box below.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtEmailTo->setPlaceholderText(QApplication::translate("falconGClass", "Your Name <and@email.address>", nullptr));
#ifndef QT_NO_TOOLTIP
        chkMenuToToggleCaptions->setToolTip(QApplication::translate("falconGClass", "Adds menu to couple image captions with descriptions.", nullptr));
#endif // QT_NO_TOOLTIP
        chkMenuToToggleCaptions->setText(QApplication::translate("falconGClass", "Captions w. Descriptions", nullptr));
#ifndef QT_NO_TOOLTIP
        chkSeparateFoldersForLanguages->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>If checked HTML files for the different languages will be put into separate folders with names of the country codes.</p><p><br/></p><p>Example: two languages (en_US and hu_HU) then the name <span style=\" font-weight:600;\">album12345.html</span>  will be used in both folders <span style=\" font-weight:600;\">en_US</span> and <span style=\" font-weight:600;\">hu_HU </span>inside <span style=\" font-weight:600;\">albums</span>.</p><p><br/></p><p>If unchecked: each albums will be put in the same folder, but the country code will be used as a suffix.</p><p><br/></p><p>Example: <span style=\" font-weight:600;\">album12345-hu_HU.html</span> and <span style=\" font-weight:600;\">album12345-en_US.html </span>will be put into the same folder <span style=\" font-weight:600;\">albums</span>.</p><p><br/></p><p>We suggest, unless you have very many albums ( more than a few thousand) leave this unchecked.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkSeparateFoldersForLanguages->setText(QApplication::translate("falconGClass", "Separate Folders for Languages", nullptr));
#ifndef QT_NO_TOOLTIP
        gbLatest->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>When 'Generate' is checked creates a separate page for the latest uploads,</p><p>which will display <span style=\" font-style:italic;\">Max. # of images</span> randomly selected from a maximum of 100 images uploaded in the </p><p><span style=\" font-style:italic;\">Time period</span>  before (and including) the date of the latest upload.</p><p><br/></p><p>If you regenerate all images then all images will be &quot;latest&quot;!</p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        gbLatest->setTitle(QApplication::translate("falconGClass", "Latest Uploads Page", nullptr));
        label_18->setText(QApplication::translate("falconGClass", "Max. # of images: ", nullptr));
        label_30->setText(QApplication::translate("falconGClass", "Time period", nullptr));
        label_31->setText(QApplication::translate("falconGClass", "days", nullptr));
#ifndef QT_NO_TOOLTIP
        sbLatestCount->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Maximum number of images selected randomly from the latest photos</p><p>that will be displayed on the page of the latest uploads.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        sbNewDays->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Number of days before and including the date of the last generated photo<br/>that will be selected for the latest upload page.</p><p>A maximum of 100 photos will be selected as a sample of latest uploads<br/>but only <span style=\" font-style:italic;\">Max # of images</span>  images will be shown at the one time</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkSetLatest->setText(QApplication::translate("falconGClass", "Generate", nullptr));
        gbGoogleAnalytics->setTitle(QApplication::translate("falconGClass", "Google Analytics", nullptr));
        label_9->setText(QApplication::translate("falconGClass", "Trac&king Code", nullptr));
        chkUseGoogleAnalytics->setText(QApplication::translate("falconGClass", "Use", nullptr));
        gbAdvanced->setTitle(QApplication::translate("falconGClass", "Advanced (you may leave these alone)", nullptr));
        label_56->setText(QApplication::translate("falconGClass", "Image Directory", nullptr));
#ifndef QT_NO_TOOLTIP
        edtImg->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Directory for all images. </p><p>Inside gallery root if no absolute or relative path is given.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtImg->setPlaceholderText(QApplication::translate("falconGClass", "imgs", nullptr));
        label_23->setText(QApplication::translate("falconGClass", "Thumbnails into", nullptr));
#ifndef QT_NO_TOOLTIP
        edtThumb->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Directory for all images. </p><p><br/></p><p>Inside gallery root if no absolute or relative path is given.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtThumb->setPlaceholderText(QApplication::translate("falconGClass", "thumbs", nullptr));
        label_8->setText(QApplication::translate("falconGClass", "Video Directory", nullptr));
#ifndef QT_NO_TOOLTIP
        edtVid->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Directory for all videos. </p><p>Inside gallery root if no absolute or relative path is given.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtVid->setPlaceholderText(QApplication::translate("falconGClass", "vids", nullptr));
        label_22->setText(QApplication::translate("falconGClass", "Album Directory ", nullptr));
#ifndef QT_NO_TOOLTIP
        edtAlbumDir->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Directory for all albums. </p><p>Inside gallery root if no absolute or relative path is given.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtAlbumDir->setPlaceholderText(QApplication::translate("falconGClass", "albums", nullptr));
        label_45->setText(QApplication::translate("falconGClass", "Base link name", nullptr));
#ifndef QT_NO_TOOLTIP
        edtBaseName->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>All albums are referred as &lt;base link name&gt;&lt;index number&gt; <br/>not by their album name, which may contain any UTF-8 characters.</p><p>Example: the album <span style=\" font-style:italic;\">'\305\220rs\303\251g&quot;</span> may be referred as<span style=\" font-style:italic;\"> album123456.</span></p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtBaseName->setPlaceholderText(QApplication::translate("falconGClass", "album", nullptr));
        label_67->setText(QApplication::translate("falconGClass", "Google Fonts", nullptr));
#ifndef QT_NO_TOOLTIP
        edtGoogleFonts->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Comma separated font names (possibly with ':wght@&lt;font weight&gt;') <br/>without the Google URL.</p><p>If the name contains space(es) enclose it into quotes.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        edtGoogleFonts->setPlaceholderText(QApplication::translate("falconGClass", "comma separated font names on Google", nullptr));
#ifndef QT_NO_TOOLTIP
        btnGoToGoogleFontsPage->setToolTip(QApplication::translate("falconGClass", "Opens the Google fonts WEB page", nullptr));
#endif // QT_NO_TOOLTIP
        btnGoToGoogleFontsPage->setText(QApplication::translate("falconGClass", "F", nullptr));
        label_44->setText(QApplication::translate("falconGClass", "Fonts Directory", nullptr));
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
#ifndef QT_NO_TOOLTIP
        chkReadFromGallery->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Check this if you have an existing gallery in the folder</p><p>'Generate gallery into' and want to create the</p><p>.struct file in the source folder. That structure file</p><p>will not contain the original image names, and supposes</p><p>that the image files and albums of same names as in</p><p>the html files are present in the source directory.</p><p><br/></p><p>Pressing 'Generate'  when this option is checked</p><p>you loose all data in the .struct file!</p><p><br/></p><p>This setting will not be saved in the configuration.</p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkReadFromGallery->setText(QApplication::translate("falconGClass", "Create structure from existing HTML gallery", nullptr));
        chkLowerCaseImageExtensions->setText(QApplication::translate("falconGClass", "Lower case image extensions", nullptr));
#ifndef QT_NO_TOOLTIP
        chkAddTitlesToAll->setToolTip(QApplication::translate("falconGClass", "Add title fields for images in 'gallery struct', which doesn't have one", nullptr));
#endif // QT_NO_TOOLTIP
        chkAddTitlesToAll->setText(QApplication::translate("falconGClass", "Title fields for all", nullptr));
#ifndef QT_NO_TOOLTIP
        chkAddDescToAll->setToolTip(QApplication::translate("falconGClass", "Add description fields for images in 'gallery struct', which doesn't have one", nullptr));
#endif // QT_NO_TOOLTIP
        chkAddDescToAll->setText(QApplication::translate("falconGClass", "Description fields for all", nullptr));
        chkOvrImages->setText(QApplication::translate("falconGClass", "Overwrite existing images", nullptr));
#ifndef QT_NO_TOOLTIP
        chkDebugging->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p><span style=\" color:#ff0000;\">If checked each photo will have a title containing the original file name.</span></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        chkDebugging->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        chkDebugging->setText(QApplication::translate("falconGClass", "&Debugging", nullptr));
#ifndef QT_NO_TOOLTIP
        chkReadJAlbum->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Create or overwrite existing .struct file using</p><p>data from the source directory tree. </p><p>If that data contains files from a JAlbum Version &lt; 12,</p><p>that will be used to get image and album descriptions and titles.</p><p><br/></p><p>If no .struct file is present in the source directory then</p><p>that directory will automatically be processed as if</p><p>this option were selected.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkReadJAlbum->setText(QApplication::translate("falconGClass", "Read from directories/JAlbum", nullptr));
        lblProgressDesc->setText(QApplication::translate("falconGClass", "images/albums", nullptr));
        lblProgressTitle->setText(QApplication::translate("falconGClass", "Reading albums and images", nullptr));
        lblProgress->setText(QApplication::translate("falconGClass", "0/0", nullptr));
        lblRemainingTimeTitle->setText(QApplication::translate("falconGClass", "Elapsed / remaining time:", nullptr));
        label_43->setText(QApplication::translate("falconGClass", "Images/sec:", nullptr));
        lblRemainingTime->setText(QApplication::translate("falconGClass", "99:99 / 99:99", nullptr));
        progressBar->setFormat(QApplication::translate("falconGClass", "%p%", nullptr));
        lblImagesPerSec->setText(QApplication::translate("falconGClass", "999", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabGallery), QApplication::translate("falconGClass", "&Gallery", nullptr));
        label_64->setText(QApplication::translate("falconGClass", "Actual:", nullptr));
        cbActualItem->setItemText(0, QApplication::translate("falconGClass", "Use 'Global Settings for Page'", nullptr));
        cbActualItem->setItemText(1, QApplication::translate("falconGClass", "Header", nullptr));
        cbActualItem->setItemText(2, QApplication::translate("falconGClass", "Menu", nullptr));
        cbActualItem->setItemText(3, QApplication::translate("falconGClass", "Languages", nullptr));
        cbActualItem->setItemText(4, QApplication::translate("falconGClass", "Small Title", nullptr));
        cbActualItem->setItemText(5, QApplication::translate("falconGClass", "Gallery Title", nullptr));
        cbActualItem->setItemText(6, QApplication::translate("falconGClass", "Gallery Description", nullptr));
        cbActualItem->setItemText(7, QApplication::translate("falconGClass", "Section (albums, images) Title", nullptr));
        cbActualItem->setItemText(8, QApplication::translate("falconGClass", "Thumbnail image", nullptr));
        cbActualItem->setItemText(9, QApplication::translate("falconGClass", "Album / Image Title", nullptr));
        cbActualItem->setItemText(10, QApplication::translate("falconGClass", "Album / Image Description", nullptr));
        cbActualItem->setItemText(11, QApplication::translate("falconGClass", "Lighhtbox Title", nullptr));
        cbActualItem->setItemText(12, QApplication::translate("falconGClass", "Lightbox Description", nullptr));
        cbActualItem->setItemText(13, QApplication::translate("falconGClass", "Footer", nullptr));

        btnSelectUplinkIcon->setText(QApplication::translate("falconGClass", "Icon for up-link", nullptr));
        gbGlobalColors->setTitle(QApplication::translate("falconGClass", "Colors", nullptr));
        label_68->setText(QApplication::translate("falconGClass", "Page Foreground color:", nullptr));
        btnPageColor->setText(QString());
        label_65->setText(QApplication::translate("falconGClass", "Page Background color", nullptr));
        btnPageBackground->setText(QString());
        chkSetAll->setText(QApplication::translate("falconGClass", "Transparent others", nullptr));
        gbBackgroundImage->setTitle(QApplication::translate("falconGClass", "Background Image", nullptr));
        gbBacgroundImagePosition->setTitle(QApplication::translate("falconGClass", "Position", nullptr));
        rbNoBackgroundImage->setText(QApplication::translate("falconGClass", "None", nullptr));
        rbCenterBckImage->setText(QApplication::translate("falconGClass", "Center", nullptr));
        rbCoverBckImage->setText(QApplication::translate("falconGClass", "Scale", nullptr));
        rbTileBckImage->setText(QApplication::translate("falconGClass", "&Tile", nullptr));
        label_63->setText(QApplication::translate("falconGClass", "File:", nullptr));
        lblbckImage->setText(QApplication::translate("falconGClass", "background image", nullptr));
#ifndef QT_NO_TOOLTIP
        btnOpenBckImage->setToolTip(QApplication::translate("falconGClass", "Select background image", nullptr));
#endif // QT_NO_TOOLTIP
        btnOpenBckImage->setText(QApplication::translate("falconGClass", "...", nullptr));
        edtBckImageName->setPlaceholderText(QApplication::translate("falconGClass", "full path name of image file", nullptr));
#ifndef QT_NO_TOOLTIP
        hsImageSizeToShow->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Resizes the image for the preview only.</p><p>On the WEB page the image appears at</p><p>its original size.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkIconTop->setText(QApplication::translate("falconGClass", "Icon to top of page", nullptr));
        chkIconText->setText(QApplication::translate("falconGClass", "Icon to toggle info", nullptr));
        toolBox->setItemText(toolBox->indexOf(pgGlobal), QApplication::translate("falconGClass", "Global Settings for Page", nullptr));
        gbGradient->setTitle(QApplication::translate("falconGClass", "Gradient", nullptr));
        chkUseGradient->setText(QApplication::translate("falconGClass", "Use", nullptr));
        label_4->setText(QApplication::translate("falconGClass", "Color", nullptr));
        label_24->setText(QApplication::translate("falconGClass", "Stop", nullptr));
        lblGradient->setText(QString());
        btnGradStartColor->setText(QString());
        sbGradStartPos->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        btnGradMiddleColor->setText(QString());
        sbGradMiddlePos->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        btnGradStopColor->setText(QString());
        sbGradStopPos->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        gbTextStyle->setTitle(QApplication::translate("falconGClass", "Text Style", nullptr));
        label_66->setText(QApplication::translate("falconGClass", "Alignment:", nullptr));
        rbTextAlignNone->setText(QApplication::translate("falconGClass", "None", nullptr));
        rbTextLeft->setText(QApplication::translate("falconGClass", "Left", nullptr));
        rbTextCenter->setText(QApplication::translate("falconGClass", "Center", nullptr));
        rbTextRight->setText(QApplication::translate("falconGClass", "Right", nullptr));
        label_69->setText(QApplication::translate("falconGClass", "Decoration", nullptr));
        chkTdUnderline->setText(QApplication::translate("falconGClass", "Underline", nullptr));
        chkTdOverline->setText(QApplication::translate("falconGClass", "Overline", nullptr));
        chkTdLinethrough->setText(QApplication::translate("falconGClass", "Line-through", nullptr));
        rbTdWavy->setText(QApplication::translate("falconGClass", "wavy", nullptr));
        rbTdDouble->setText(QApplication::translate("falconGClass", "double", nullptr));
        rbTdSolid->setText(QApplication::translate("falconGClass", "solid", nullptr));
        rbTdDotted->setText(QApplication::translate("falconGClass", "dotted", nullptr));
        rbTdDashed->setText(QApplication::translate("falconGClass", "dashed", nullptr));
        gbTextShadow->setTitle(QApplication::translate("falconGClass", "Shadow", nullptr));
        label_85->setText(QApplication::translate("falconGClass", "#1", nullptr));
        label_37->setText(QApplication::translate("falconGClass", "H:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowHoriz1->setToolTip(QApplication::translate("falconGClass", "Horizontal displacement", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowHoriz1->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_38->setText(QApplication::translate("falconGClass", "&V:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowVert1->setToolTip(QApplication::translate("falconGClass", "Vertical displacement", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowVert1->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_39->setText(QApplication::translate("falconGClass", "B:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowBlur1->setToolTip(QApplication::translate("falconGClass", "Blur in pixels", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowBlur1->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_87->setText(QApplication::translate("falconGClass", "S:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowSpread1->setToolTip(QApplication::translate("falconGClass", "Spread in pixels", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowSpread1->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_86->setText(QApplication::translate("falconGClass", "#2", nullptr));
        label_83->setText(QApplication::translate("falconGClass", "H:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowHoriz2->setToolTip(QApplication::translate("falconGClass", "Horizontal displacement", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowHoriz2->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_84->setText(QApplication::translate("falconGClass", "&V:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowVert2->setToolTip(QApplication::translate("falconGClass", "Vertical displacement", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowVert2->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_82->setText(QApplication::translate("falconGClass", "B:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowBlur2->setToolTip(QApplication::translate("falconGClass", "Blur in pixels", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowBlur2->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_88->setText(QApplication::translate("falconGClass", "S:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbShadowSpread2->setToolTip(QApplication::translate("falconGClass", "Spread in pixels", nullptr));
#endif // QT_NO_TOOLTIP
        sbShadowSpread2->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        rbBoxShadow->setText(QApplication::translate("falconGClass", "Box Shadow", nullptr));
        chkShadowOn->setText(QApplication::translate("falconGClass", "Use", nullptr));
        rbTextShadow->setText(QApplication::translate("falconGClass", "Text Shadow", nullptr));
#ifndef QT_NO_TOOLTIP
        btnShadowColor->setToolTip(QApplication::translate("falconGClass", "Shadow color. Same for both", nullptr));
#endif // QT_NO_TOOLTIP
        btnShadowColor->setText(QString());
        gbFont->setTitle(QApplication::translate("falconGClass", "Font", nullptr));
        label_27->setText(QApplication::translate("falconGClass", "Family", nullptr));
        edtFontFamily->setPlaceholderText(QApplication::translate("falconGClass", "Example: \"Tms Rmn\",Arial", nullptr));
#ifndef QT_NO_TOOLTIP
        cbFonts->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>If you gave any default font names or </p><p>Google font names you can select them </p><p>too using this combo box.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        cbFonts->setPlaceholderText(QApplication::translate("falconGClass", "Select specified font", nullptr));
        label_28->setText(QApplication::translate("falconGClass", "Si&ze:", nullptr));
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

        label_32->setText(QApplication::translate("falconGClass", "Line height:", nullptr));
        cbLineHeight->setItemText(0, QApplication::translate("falconGClass", "8pt", nullptr));
        cbLineHeight->setItemText(1, QApplication::translate("falconGClass", "9pt", nullptr));
        cbLineHeight->setItemText(2, QApplication::translate("falconGClass", "10pt", nullptr));
        cbLineHeight->setItemText(3, QApplication::translate("falconGClass", "11pt", nullptr));
        cbLineHeight->setItemText(4, QApplication::translate("falconGClass", "12pt", nullptr));
        cbLineHeight->setItemText(5, QApplication::translate("falconGClass", "14pt", nullptr));
        cbLineHeight->setItemText(6, QApplication::translate("falconGClass", "16pt", nullptr));
        cbLineHeight->setItemText(7, QApplication::translate("falconGClass", "18pt", nullptr));
        cbLineHeight->setItemText(8, QApplication::translate("falconGClass", "20pt", nullptr));
        cbLineHeight->setItemText(9, QApplication::translate("falconGClass", "22pt", nullptr));
        cbLineHeight->setItemText(10, QApplication::translate("falconGClass", "24pt", nullptr));
        cbLineHeight->setItemText(11, QApplication::translate("falconGClass", "26pt", nullptr));
        cbLineHeight->setItemText(12, QApplication::translate("falconGClass", "28pt", nullptr));
        cbLineHeight->setItemText(13, QApplication::translate("falconGClass", "36pt", nullptr));
        cbLineHeight->setItemText(14, QApplication::translate("falconGClass", "48pt", nullptr));
        cbLineHeight->setItemText(15, QApplication::translate("falconGClass", "72pt", nullptr));

        label_6->setText(QApplication::translate("falconGClass", "Space below", nullptr));
        sbSpaceAfter->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        chkDifferentFirstLine->setText(QApplication::translate("falconGClass", "&Different 1st line:", nullptr));
        cbPointSizeFirstLine->setItemText(0, QApplication::translate("falconGClass", "8pt", nullptr));
        cbPointSizeFirstLine->setItemText(1, QApplication::translate("falconGClass", "9pt", nullptr));
        cbPointSizeFirstLine->setItemText(2, QApplication::translate("falconGClass", "10pt", nullptr));
        cbPointSizeFirstLine->setItemText(3, QApplication::translate("falconGClass", "11pt", nullptr));
        cbPointSizeFirstLine->setItemText(4, QApplication::translate("falconGClass", "12pt", nullptr));
        cbPointSizeFirstLine->setItemText(5, QApplication::translate("falconGClass", "14pt", nullptr));
        cbPointSizeFirstLine->setItemText(6, QApplication::translate("falconGClass", "16pt", nullptr));
        cbPointSizeFirstLine->setItemText(7, QApplication::translate("falconGClass", "18pt", nullptr));
        cbPointSizeFirstLine->setItemText(8, QApplication::translate("falconGClass", "20pt", nullptr));
        cbPointSizeFirstLine->setItemText(9, QApplication::translate("falconGClass", "22pt", nullptr));
        cbPointSizeFirstLine->setItemText(10, QApplication::translate("falconGClass", "24pt", nullptr));
        cbPointSizeFirstLine->setItemText(11, QApplication::translate("falconGClass", "26pt", nullptr));
        cbPointSizeFirstLine->setItemText(12, QApplication::translate("falconGClass", "28pt", nullptr));
        cbPointSizeFirstLine->setItemText(13, QApplication::translate("falconGClass", "36pt", nullptr));
        cbPointSizeFirstLine->setItemText(14, QApplication::translate("falconGClass", "48pt", nullptr));
        cbPointSizeFirstLine->setItemText(15, QApplication::translate("falconGClass", "72pt", nullptr));

        chkBold->setText(QApplication::translate("falconGClass", "Bold", nullptr));
        chkItalic->setText(QApplication::translate("falconGClass", "Italic", nullptr));
        label_26->setText(QApplication::translate("falconGClass", "Text", nullptr));
        label_33->setText(QApplication::translate("falconGClass", "Background", nullptr));
        sbBackgroundOpacity->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        btnForeground->setText(QString());
#ifndef QT_NO_TOOLTIP
        chkTextOpacity->setToolTip(QApplication::translate("falconGClass", "Turn opacity on or off", nullptr));
#endif // QT_NO_TOOLTIP
        chkTextOpacity->setText(QApplication::translate("falconGClass", "use opacity", nullptr));
#ifndef QT_NO_TOOLTIP
        chkBackgroundOpacity->setToolTip(QApplication::translate("falconGClass", "Turn opacity on or off", nullptr));
#endif // QT_NO_TOOLTIP
        chkBackgroundOpacity->setText(QApplication::translate("falconGClass", "use opacity", nullptr));
        btnBackground->setText(QString());
        sbTextOpacity->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        gbBorder->setTitle(QApplication::translate("falconGClass", "Border", nullptr));
        label_57->setText(QApplication::translate("falconGClass", "Style", nullptr));
        cbBorderStyle->setItemText(0, QApplication::translate("falconGClass", "none", nullptr));
        cbBorderStyle->setItemText(1, QApplication::translate("falconGClass", "solid", nullptr));
        cbBorderStyle->setItemText(2, QApplication::translate("falconGClass", "dotted", nullptr));
        cbBorderStyle->setItemText(3, QApplication::translate("falconGClass", "dashed", nullptr));
        cbBorderStyle->setItemText(4, QApplication::translate("falconGClass", "double", nullptr));
        cbBorderStyle->setItemText(5, QApplication::translate("falconGClass", "groove", nullptr));
        cbBorderStyle->setItemText(6, QApplication::translate("falconGClass", "ridge", nullptr));
        cbBorderStyle->setItemText(7, QApplication::translate("falconGClass", "inset", nullptr));
        cbBorderStyle->setItemText(8, QApplication::translate("falconGClass", "outset", nullptr));

        btnBorderColor->setText(QApplication::translate("falconGClass", "B", nullptr));
        label_58->setText(QApplication::translate("falconGClass", "Width:", nullptr));
        sbBorderWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_70->setText(QApplication::translate("falconGClass", "Border radius", nullptr));
        sbBorderRadius->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        rbLeftBorder->setText(QApplication::translate("falconGClass", "Left", nullptr));
        rbBottomBorder->setText(QApplication::translate("falconGClass", "Bottom", nullptr));
        rbRightBorder->setText(QApplication::translate("falconGClass", "Right", nullptr));
        rbTopBorder->setText(QApplication::translate("falconGClass", "Top", nullptr));
        rbAllBorders->setText(QApplication::translate("falconGClass", "All", nullptr));
        toolBox->setItemText(toolBox->indexOf(pgSelected), QApplication::translate("falconGClass", "Selected item", nullptr));
        label_2->setText(QApplication::translate("falconGClass", "Width", nullptr));
        sbImageMargin->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_15->setText(QApplication::translate("falconGClass", "Gap", nullptr));
#ifndef QT_NO_TOOLTIP
        chkDistortThumbnails->setToolTip(QApplication::translate("falconGClass", "Wide thumbnails will be squeezed into the \n"
"width given here. This distorsion is created\n"
"by the browser, so it may not be what you want.\n"
"\n"
"\n"
"Need only save the CSS files, no new generation\n"
"is required.", nullptr));
#endif // QT_NO_TOOLTIP
        chkDistortThumbnails->setText(QApplication::translate("falconGClass", "Modify aspect ratio for thumbnails", nullptr));
        label_7->setText(QApplication::translate("falconGClass", "Thumbnail", nullptr));
        sbImageHeight->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        sbImageWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
#ifndef QT_NO_TOOLTIP
        btnLink->setToolTip(QApplication::translate("falconGClass", "Links/unlinks image width and height,\n"
"so when one changes the other changes too.\n"
"", nullptr));
#endif // QT_NO_TOOLTIP
        btnLink->setText(QString());
        label_73->setText(QApplication::translate("falconGClass", "Image Quality:", nullptr));
        cbImageQuality->setItemText(0, QApplication::translate("falconGClass", "Same as original", nullptr));
        cbImageQuality->setItemText(1, QApplication::translate("falconGClass", "100% - best", nullptr));
        cbImageQuality->setItemText(2, QApplication::translate("falconGClass", "90%", nullptr));
        cbImageQuality->setItemText(3, QApplication::translate("falconGClass", "80% - high", nullptr));
        cbImageQuality->setItemText(4, QApplication::translate("falconGClass", "70%", nullptr));
        cbImageQuality->setItemText(5, QApplication::translate("falconGClass", "60%", nullptr));
        cbImageQuality->setItemText(6, QApplication::translate("falconGClass", "50% - medium", nullptr));
        cbImageQuality->setItemText(7, QApplication::translate("falconGClass", "40%", nullptr));
        cbImageQuality->setItemText(8, QApplication::translate("falconGClass", "30% - low", nullptr));
        cbImageQuality->setItemText(9, QApplication::translate("falconGClass", "20%", nullptr));
        cbImageQuality->setItemText(10, QApplication::translate("falconGClass", "10% - abysmal", nullptr));

#ifndef QT_NO_TOOLTIP
        cbImageQuality->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Select the quality of the images on the server.</p><p>The smaller the number the smaller the file size but </p><p>the more artifact will be present in the result.</p><p><br/></p><p>A huge space reduction is achieved, with a small</p><p>quality loss with a quality of 80%.</p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        label_59->setText(QApplication::translate("falconGClass", "x", nullptr));
        label_3->setText(QApplication::translate("falconGClass", "Height", nullptr));
#ifndef QT_NO_TOOLTIP
        chkCropThumbnails->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>All thumbnail images will have the same height.</p><p><br/></p><p>Check this button if you want thumbnails to have the</p><p>exact sizes given here. In this case the thumbnail</p><p>will be cropped from the top left corner of the</p><p>source image to this size. </p><p><br/></p><p>The thumbnail still may be narrower!</p><p><br/></p><p>Need only save the CSS files, no new generation</p><p>is required.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkCropThumbnails->setText(QApplication::translate("falconGClass", "Crop thumbnails to this size", nullptr));
        sbThumbnailHeight->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        chkDoNotEnlarge->setText(QApplication::translate("falconGClass", "Do not enlarge images", nullptr));
        sbThumbnailWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label->setText(QApplication::translate("falconGClass", "Image", nullptr));
        gbImageDecoration->setTitle(QApplication::translate("falconGClass", "Image Frame", nullptr));
        label_74->setText(QApplication::translate("falconGClass", "Frame style", nullptr));
        cbImageBorderStyle->setItemText(0, QApplication::translate("falconGClass", "no frame", nullptr));
        cbImageBorderStyle->setItemText(1, QApplication::translate("falconGClass", "solid", nullptr));
        cbImageBorderStyle->setItemText(2, QApplication::translate("falconGClass", "dotted", nullptr));
        cbImageBorderStyle->setItemText(3, QApplication::translate("falconGClass", "dashed", nullptr));
        cbImageBorderStyle->setItemText(4, QApplication::translate("falconGClass", "double", nullptr));
        cbImageBorderStyle->setItemText(5, QApplication::translate("falconGClass", "groove", nullptr));
        cbImageBorderStyle->setItemText(6, QApplication::translate("falconGClass", "ridge", nullptr));
        cbImageBorderStyle->setItemText(7, QApplication::translate("falconGClass", "inset", nullptr));
        cbImageBorderStyle->setItemText(8, QApplication::translate("falconGClass", "outset", nullptr));

        label_14->setText(QApplication::translate("falconGClass", "Width:", nullptr));
        sbImageBorderWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_75->setText(QApplication::translate("falconGClass", "Radius:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbImageBorderRadius->setToolTip(QApplication::translate("falconGClass", "Corner radius for a rounded rectangle. 0 means not rounded.", nullptr));
#endif // QT_NO_TOOLTIP
        sbImageBorderRadius->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        btnImageBorderColor->setText(QString());
#ifndef QT_NO_TOOLTIP
        gbImageDecoration_2->setToolTip(QApplication::translate("falconGClass", "\"Matte\" is the area around an image in the frame", nullptr));
#endif // QT_NO_TOOLTIP
        gbImageDecoration_2->setTitle(QApplication::translate("falconGClass", "Image Matte", nullptr));
        label_25->setText(QApplication::translate("falconGClass", "Width:", nullptr));
        sbImageMatteWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_76->setText(QApplication::translate("falconGClass", "Color:", nullptr));
        btnImageMatteColor->setText(QString());
        label_96->setText(QApplication::translate("falconGClass", "Radius:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbImageMatteRadius->setToolTip(QApplication::translate("falconGClass", "Corner radius for a rounded rectangle. 0 means not rounded.", nullptr));
#endif // QT_NO_TOOLTIP
        sbImageMatteRadius->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
#ifndef QT_NO_TOOLTIP
        gbImageDecoration_3->setToolTip(QApplication::translate("falconGClass", "\"Matte\" is the area around an image in the frame", nullptr));
#endif // QT_NO_TOOLTIP
        gbImageDecoration_3->setTitle(QApplication::translate("falconGClass", "Image Matte", nullptr));
        label_77->setText(QApplication::translate("falconGClass", "Width:", nullptr));
        sbAlbumMatteWidth->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_78->setText(QApplication::translate("falconGClass", "Color:", nullptr));
        btnAlbumMatteColor->setText(QString());
        label_97->setText(QApplication::translate("falconGClass", "Radius:", nullptr));
#ifndef QT_NO_TOOLTIP
        sbAlbumMatteRadius->setToolTip(QApplication::translate("falconGClass", "Corner radius for a rounded rectangle. 0 means not rounded.", nullptr));
#endif // QT_NO_TOOLTIP
        sbAlbumMatteRadius->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        toolBox->setItemText(toolBox->indexOf(pgImageDecor), QApplication::translate("falconGClass", "Images", nullptr));
        label_34->setText(QApplication::translate("falconGClass", "Opacity   ", nullptr));
        chkUseWM->setText(QApplication::translate("falconGClass", "use", nullptr));
        label_29->setText(QApplication::translate("falconGClass", "Color", nullptr));
        sbWmOpacity->setSuffix(QApplication::translate("falconGClass", "%", nullptr));
        label_35->setText(QApplication::translate("falconGClass", "Text", nullptr));
        btnWmColor->setText(QString());
        lblWmSample->setText(QApplication::translate("falconGClass", "Watermark text sample", nullptr));
        gbWatermarkFont->setTitle(QApplication::translate("falconGClass", "Font", nullptr));
        btnSelectWmFont->setText(QApplication::translate("falconGClass", "A", nullptr));
#ifndef QT_NO_TOOLTIP
        lblWaterMarkFont->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        lblWaterMarkFont->setText(QApplication::translate("falconGClass", "----------------------------------------------------------------------------------------------", nullptr));
        gbPosition->setTitle(QApplication::translate("falconGClass", "Position", nullptr));
#ifndef QT_NO_TOOLTIP
        edtWmVertMargin->setToolTip(QApplication::translate("falconGClass", "Relative to the nearest edge", nullptr));
#endif // QT_NO_TOOLTIP
        label_12->setText(QApplication::translate("falconGClass", "Vertical margin:", nullptr));
        label_13->setText(QApplication::translate("falconGClass", "px", nullptr));
        cbWmVPosition->setItemText(0, QApplication::translate("falconGClass", "Top", nullptr));
        cbWmVPosition->setItemText(1, QApplication::translate("falconGClass", "Center vertically", nullptr));
        cbWmVPosition->setItemText(2, QApplication::translate("falconGClass", "Bottom", nullptr));

        cbWmVPosition->setCurrentText(QApplication::translate("falconGClass", "Center vertically", nullptr));
        cbWmHPosition->setItemText(0, QApplication::translate("falconGClass", "Left", nullptr));
        cbWmHPosition->setItemText(1, QApplication::translate("falconGClass", "Center horizontally", nullptr));
        cbWmHPosition->setItemText(2, QApplication::translate("falconGClass", "Right", nullptr));

        label_60->setText(QApplication::translate("falconGClass", "Horizontal margin", nullptr));
#ifndef QT_NO_TOOLTIP
        edtWmHorizMargin->setToolTip(QApplication::translate("falconGClass", "Relative to the nearest edge", nullptr));
#endif // QT_NO_TOOLTIP
        label_71->setText(QApplication::translate("falconGClass", "px", nullptr));
        gbWmTextShadow->setTitle(QApplication::translate("falconGClass", "Text Shadow", nullptr));
        chkUseWMShadow->setText(QApplication::translate("falconGClass", "use", nullptr));
        label_16->setText(QApplication::translate("falconGClass", "Horizontal", nullptr));
        sbWmShadowHoriz->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_40->setText(QApplication::translate("falconGClass", "Vertical:", nullptr));
        sbWmShadowVert->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_41->setText(QApplication::translate("falconGClass", "Blur:", nullptr));
        sbWmShadowBlur->setSuffix(QApplication::translate("falconGClass", "px", nullptr));
        label_52->setText(QApplication::translate("falconGClass", "Color", nullptr));
        btnWmShadowColor->setText(QString());
        label_48->setText(QApplication::translate("falconGClass", "<html><head/><body><p>The watermark on the sample image is just an indication <br/>how they would look like on the page.</p></body></html>", nullptr));
        toolBox->setItemText(toolBox->indexOf(pgWatermark), QApplication::translate("falconGClass", "Watermark", nullptr));
        btnDisplayHint->setText(QApplication::translate("falconGClass", "?", nullptr));
#ifndef QT_NO_TOOLTIP
        btnReload->setToolTip(QApplication::translate("falconGClass", "Reset the WEB page to the state of\n"
"the last CSS save.", nullptr));
#endif // QT_NO_TOOLTIP
        btnReload->setText(QApplication::translate("falconGClass", "Reset Page", nullptr));
        btnResetDesign->setText(QApplication::translate("falconGClass", "Reset", nullptr));
        gbSample->setTitle(QApplication::translate("falconGClass", "Sample", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabDesign), QApplication::translate("falconGClass", "&Design", nullptr));
        label_51->setText(QApplication::translate("falconGClass", "Albums", nullptr));
        lblTotalCount->setText(QApplication::translate("falconGClass", "...", nullptr));
        label_53->setText(QApplication::translate("falconGClass", "Images", nullptr));
        lblImageCount->setText(QApplication::translate("falconGClass", "...", nullptr));
        label_54->setText(QApplication::translate("falconGClass", "Title (Language #1)", nullptr));
        label_10->setText(QApplication::translate("falconGClass", "Title (Language #2)", nullptr));
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
        tabFalconG->setTabText(tabFalconG->indexOf(tabEdit), QApplication::translate("falconGClass", "&Edit", nullptr));
        lblOptionsHelp->setText(QApplication::translate("falconGClass", "<html><head/><body><p><br/></p><p><br/></p><p align=\"center\"><span style=\" font-size:12pt;\">Right click </span></p><p align=\"center\">on the free area on any page to select the program scheme.</p><p align=\"center\">(e.g. dark)</p><p align=\"center\"><br/></p><p align=\"center\">You may not redefine schemes 'Default' and 'System colors'.</p></body></html>", nullptr));
        groupBox->setTitle(QApplication::translate("falconGClass", "Program Options", nullptr));
        label_79->setText(QApplication::translate("falconGClass", "Interface Language", nullptr));
        rbMagyar->setText(QApplication::translate("falconGClass", "Hungarian", nullptr));
        rbEnglish->setText(QApplication::translate("falconGClass", "English", nullptr));
#ifndef QT_NO_TOOLTIP
        btnResetDialogs->setToolTip(QApplication::translate("falconGClass", "Press this button to clear your saved choices in dialoges.", nullptr));
#endif // QT_NO_TOOLTIP
        btnResetDialogs->setText(QApplication::translate("falconGClass", "Reset dialogs", nullptr));
        gbColorScheme->setTitle(QApplication::translate("falconGClass", "Color Scheme Editor", nullptr));
        label_49->setText(QString());
        label_72->setText(QString());
        pnlSchemeEmulator->setTitle(QApplication::translate("falconGClass", "Color Scheme", nullptr));
        btnDeleteColorScheme->setText(QApplication::translate("falconGClass", "Delete", nullptr));
        btnAddAndGenerateColorScheme->setText(QApplication::translate("falconGClass", "Add ", nullptr));
        btnApplyColorScheme->setText(QApplication::translate("falconGClass", "Apply", nullptr));
        btnResetColorScheme->setText(QApplication::translate("falconGClass", "Reset", nullptr));
        btnMoveSchemeUp->setText(QApplication::translate("falconGClass", "Move Up      ", nullptr));
        btnMoveSchemeDown->setText(QApplication::translate("falconGClass", "Move Down", nullptr));
        tabFalconG->setTabText(tabFalconG->indexOf(tabOptions), QApplication::translate("falconGClass", "&Options", nullptr));
        btnGenerate->setText(QApplication::translate("falconGClass", "Generate (F9)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnGenerate->setShortcut(QApplication::translate("falconGClass", "F9", nullptr));
#endif // QT_NO_SHORTCUT
#ifndef QT_NO_TOOLTIP
        chkGenerateAll->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Re-generate all images and albums, regardless if they were already OK.</p><p>For images generated again the upload date will change to</p><p>today's date! This will change the list of the  latest uploads!</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkGenerateAll->setText(QApplication::translate("falconGClass", "&all", nullptr));
        chkButImages->setText(QApplication::translate("falconGClass", "except i&mages", nullptr));
        btnPreview->setText(QApplication::translate("falconGClass", "Preview (F12)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnPreview->setShortcut(QApplication::translate("falconGClass", "F12", nullptr));
#endif // QT_NO_SHORTCUT
        btnSaveStyleSheet->setText(QApplication::translate("falconGClass", "Save CSS", nullptr));
        btnSaveConfig->setText(QApplication::translate("falconGClass", "Save con&fig.", nullptr));
#ifndef QT_NO_SHORTCUT
        btnSaveConfig->setShortcut(QApplication::translate("falconGClass", "Ctrl+S", nullptr));
#endif // QT_NO_SHORTCUT
#ifndef QT_NO_TOOLTIP
        chkCleanUp->setToolTip(QApplication::translate("falconGClass", "<html><head/><body><p>Removes helper files created in </p><p>program directory (html,css,js,...)</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        chkCleanUp->setText(QApplication::translate("falconGClass", "Clean up and", nullptr));
        btnExit->setText(QApplication::translate("falconGClass", "E&xit (Alt+X)", nullptr));
#ifndef QT_NO_SHORTCUT
        btnExit->setShortcut(QApplication::translate("falconGClass", "Alt+X", nullptr));
#endif // QT_NO_SHORTCUT
    } // retranslateUi

};

namespace Ui {
    class falconGClass: public Ui_falconGClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FALCONG_H
