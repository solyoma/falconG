#pragma once
#include <QtCore>
#include <QtWidgets/QWidget>
#include <QDialog>

#include "ui_selectStruct.h"

class  SelectStructDialog : public QDialog
{
	Q_OBJECT
public:
	SelectStructDialog(QWidget* parent, QString title, QStringList files) : QDialog(parent)
	{
		ui.setupUi(this);
		ui.lblFolderName->setText(title);
		ui.lwStructs->addItems(files);
	}
	QString SelectedStruct() { return _res; }
private:
	Ui::SelectStructDialogClass ui;
	int _exitCode = QDialog::Rejected;
	QString _res;

public slots:
	void on_lwStructs_currentItemChanged()
	{
		ui.btnOk->setEnabled(ui.lwStructs->currentItem() >= 0);

	}
	void on_btnOk_clicked() 
	{ 
		_exitCode = QDialog::Accepted; 
		QListWidgetItem* plwi = ui.lwStructs->currentItem();
		_res = plwi->text();
	}
};
