#pragma once

#include <QtCore>
#include <QtWidgets/QDialog>
#include <QPushButton> 
#include <QDialogButtonBox> 

#include "common.h"
using namespace Common;


#include "ui_exportCsv.h"

class CsvExportDialog : public QDialog
{
	Q_OBJECT
public:
	enum ExportOptions : int
	{
		eoNone, eoHeader = 1, eoRecursive = 2, eoOrig = 4, eoGenerated = 8
	};
	CsvExportDialog(QWidget* parent = nullptr) : QDialog(parent)
	{
		ui.setupUi(this);
		ui.edtFileName->setText(tr("falconGExport.csv"));
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false); // enable only when file name is set
	}
	~CsvExportDialog() { }
	QString GetFileName()     const { return ui.edtFileName->text().trimmed(); }
	bool IncludeHeader()    const { return ui.chkHeader->isChecked(); }
	bool IncludeAlbum()   const { return ui.chkRecursive->isChecked(); }
	bool IncludeOrigName()   const { return ui.chkOrigName->isChecked(); }
	bool IncludeGeneratedName() const { return ui.chkGenName->isChecked(); }

public:
	int options = 0; // a combination of ExportOptions

private:
	Ui::CsvExportDialogClass ui;  // pointer to the dialog UI
public slots:
//	int exec() override;
	void on_buttonBox_clicked(QAbstractButton* button);
};