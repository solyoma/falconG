/********************************************************************************
** Form generated from reading UI file 'sourcehistory.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCEHISTORY_H
#define UI_SOURCEHISTORY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
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
    QPushButton *btnAddDirectory;
    QSpacerItem *verticalSpacer_2;
    QPushButton *btnUndelete;
    QPushButton *btnDelete;
    QSpacerItem *verticalSpacer;
    QPushButton *btnSelect;
    QPushButton *btnCancel;

    void setupUi(QWidget *SourceHistoryClass)
    {
        if (SourceHistoryClass->objectName().isEmpty())
            SourceHistoryClass->setObjectName(QString::fromUtf8("SourceHistoryClass"));
        SourceHistoryClass->resize(556, 413);
        horizontalLayout_2 = new QHBoxLayout(SourceHistoryClass);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lvHistory = new QListView(SourceHistoryClass);
        lvHistory->setObjectName(QString::fromUtf8("lvHistory"));

        horizontalLayout->addWidget(lvHistory);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        btnAddDirectory = new QPushButton(SourceHistoryClass);
        btnAddDirectory->setObjectName(QString::fromUtf8("btnAddDirectory"));
        btnAddDirectory->setFlat(false);

        verticalLayout_2->addWidget(btnAddDirectory);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        btnUndelete = new QPushButton(SourceHistoryClass);
        btnUndelete->setObjectName(QString::fromUtf8("btnUndelete"));
        btnUndelete->setEnabled(false);

        verticalLayout_2->addWidget(btnUndelete);

        btnDelete = new QPushButton(SourceHistoryClass);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));
        btnDelete->setEnabled(false);

        verticalLayout_2->addWidget(btnDelete);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        btnSelect = new QPushButton(SourceHistoryClass);
        btnSelect->setObjectName(QString::fromUtf8("btnSelect"));

        verticalLayout_2->addWidget(btnSelect);

        btnCancel = new QPushButton(SourceHistoryClass);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
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
        btnAddDirectory->setText(QApplication::translate("SourceHistoryClass", "&Add", nullptr));
        btnUndelete->setText(QApplication::translate("SourceHistoryClass", "&Undelete", nullptr));
        btnDelete->setText(QApplication::translate("SourceHistoryClass", "&Delete", nullptr));
        btnSelect->setText(QApplication::translate("SourceHistoryClass", "&Ok", nullptr));
        btnCancel->setText(QApplication::translate("SourceHistoryClass", "&Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SourceHistoryClass: public Ui_SourceHistoryClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCEHISTORY_H
