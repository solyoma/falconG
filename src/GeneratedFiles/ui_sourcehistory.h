/********************************************************************************
** Form generated from reading UI file 'sourcehistory.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCEHISTORY_H
#define UI_SOURCEHISTORY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
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
    QSpacerItem *verticalSpacer_2;
    QPushButton *btnUndelete;
    QPushButton *btnDelete;
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
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        btnUndelete = new QPushButton(SourceHistoryClass);
        btnUndelete->setObjectName(QStringLiteral("btnUndelete"));
        btnUndelete->setEnabled(false);

        verticalLayout_2->addWidget(btnUndelete);

        btnDelete = new QPushButton(SourceHistoryClass);
        btnDelete->setObjectName(QStringLiteral("btnDelete"));
        btnDelete->setEnabled(false);

        verticalLayout_2->addWidget(btnDelete);

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
        btnUndelete->setText(QApplication::translate("SourceHistoryClass", "&Undelete", nullptr));
        btnDelete->setText(QApplication::translate("SourceHistoryClass", "&Delete", nullptr));
        btnSelect->setText(QApplication::translate("SourceHistoryClass", "&Select", nullptr));
        btnCancel->setText(QApplication::translate("SourceHistoryClass", "&Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SourceHistoryClass: public Ui_SourceHistoryClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCEHISTORY_H
