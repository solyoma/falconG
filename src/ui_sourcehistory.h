/********************************************************************************
** Form generated from reading UI file 'sourcehistory.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCEHISTORY_H
#define UI_SOURCEHISTORY_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SourceHistoryClass
{
public:
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QListView *lvHistory;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QSpinBox *sbHistSize;
    QSpacerItem *verticalSpacer_3;
    QPushButton *btnAddDirectory;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_3;
    QToolButton *btnMoveUp;
    QToolButton *btnMoveDown;
    QPushButton *btnDelete;
    QPushButton *btnUndelete;
    QPushButton *btnSort;
    QSpacerItem *verticalSpacer;
    QPushButton *btnSelect;
    QPushButton *btnCancel;

    void setupUi(QWidget *SourceHistoryClass)
    {
        if (SourceHistoryClass->objectName().isEmpty())
            SourceHistoryClass->setObjectName(QStringLiteral("SourceHistoryClass"));
        SourceHistoryClass->resize(556, 413);
        horizontalLayout_2 = new QHBoxLayout(SourceHistoryClass);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lvHistory = new QListView(SourceHistoryClass);
        lvHistory->setObjectName(QStringLiteral("lvHistory"));

        horizontalLayout->addWidget(lvHistory);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        label = new QLabel(SourceHistoryClass);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout_2->addWidget(label);

        sbHistSize = new QSpinBox(SourceHistoryClass);
        sbHistSize->setObjectName(QStringLiteral("sbHistSize"));
        sbHistSize->setMinimum(1);
        sbHistSize->setValue(10);

        verticalLayout_2->addWidget(sbHistSize);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_3);

        btnAddDirectory = new QPushButton(SourceHistoryClass);
        btnAddDirectory->setObjectName(QStringLiteral("btnAddDirectory"));
        btnAddDirectory->setFlat(false);

        verticalLayout_2->addWidget(btnAddDirectory);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        btnMoveUp = new QToolButton(SourceHistoryClass);
        btnMoveUp->setObjectName(QStringLiteral("btnMoveUp"));
        btnMoveUp->setEnabled(false);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/Resources/moveup-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveUp->setIcon(icon);

        horizontalLayout_3->addWidget(btnMoveUp);

        btnMoveDown = new QToolButton(SourceHistoryClass);
        btnMoveDown->setObjectName(QStringLiteral("btnMoveDown"));
        btnMoveDown->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QStringLiteral("Resources/movedown-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveDown->setIcon(icon1);

        horizontalLayout_3->addWidget(btnMoveDown);


        verticalLayout_2->addLayout(horizontalLayout_3);

        btnDelete = new QPushButton(SourceHistoryClass);
        btnDelete->setObjectName(QStringLiteral("btnDelete"));
        btnDelete->setEnabled(false);

        verticalLayout_2->addWidget(btnDelete);

        btnUndelete = new QPushButton(SourceHistoryClass);
        btnUndelete->setObjectName(QStringLiteral("btnUndelete"));
        btnUndelete->setEnabled(false);

        verticalLayout_2->addWidget(btnUndelete);

        btnSort = new QPushButton(SourceHistoryClass);
        btnSort->setObjectName(QStringLiteral("btnSort"));
        btnSort->setEnabled(false);

        verticalLayout_2->addWidget(btnSort);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        btnSelect = new QPushButton(SourceHistoryClass);
        btnSelect->setObjectName(QStringLiteral("btnSelect"));

        verticalLayout_2->addWidget(btnSelect);

        btnCancel = new QPushButton(SourceHistoryClass);
        btnCancel->setObjectName(QStringLiteral("btnCancel"));
        btnCancel->setAutoDefault(true);

        verticalLayout_2->addWidget(btnCancel);


        horizontalLayout->addLayout(verticalLayout_2);


        horizontalLayout_2->addLayout(horizontalLayout);


        retranslateUi(SourceHistoryClass);

        btnCancel->setDefault(true);


        QMetaObject::connectSlotsByName(SourceHistoryClass);
    } // setupUi

    void retranslateUi(QWidget *SourceHistoryClass)
    {
        SourceHistoryClass->setWindowTitle(QApplication::translate("SourceHistoryClass", "falconG - Gallery Source Selection", nullptr));
        label->setText(QApplication::translate("SourceHistoryClass", "History size:", nullptr));
        btnAddDirectory->setText(QApplication::translate("SourceHistoryClass", "&Add", nullptr));
        btnMoveUp->setText(QApplication::translate("SourceHistoryClass", "...", nullptr));
        btnMoveDown->setText(QApplication::translate("SourceHistoryClass", "...", nullptr));
        btnDelete->setText(QApplication::translate("SourceHistoryClass", "&Delete", nullptr));
        btnUndelete->setText(QApplication::translate("SourceHistoryClass", "&Undelete", nullptr));
        btnSort->setText(QApplication::translate("SourceHistoryClass", "Sort", nullptr));
        btnSelect->setText(QApplication::translate("SourceHistoryClass", "&Ok", nullptr));
        btnCancel->setText(QApplication::translate("SourceHistoryClass", "&Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SourceHistoryClass: public Ui_SourceHistoryClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCEHISTORY_H
