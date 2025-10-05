#include "CsvExportDialog.h"

void CsvExportDialog::on_buttonBox_clicked(QAbstractButton* button) 
{
	if (ui.buttonBox->standardButton(button) == ui.buttonBox->QDialogButtonBox::Ok)
		; // accept();
};