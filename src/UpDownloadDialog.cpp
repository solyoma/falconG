#include "UpDownloadDialog.h"

UpDownloadDialog::UpDownloadDialog(UpDownloadDialog::Data data, QWidget* parent)
                                    : QDialog(parent), _data(data)
{
    ui.setupUi(this);
	ui.lblUrl->setText(data.qsServer);
	ui.lblPublicHtml->setText(data.qsRootFolder);
	ui.edtUser->setText(data.qsUser);
    ui.edtPassword->setText(data.qsPassword);
    rbProts[0] = ui.rbProtAuto;
    rbProts[1] = ui.rbProtSftp;
    rbProts[2] = ui.rbProtFtpsTls;
    rbProts[3] = ui.rbProtFtps;
    rbProts[4] = ui.rbProtFtp;
    int n = (int)data.protocol;
    if (n < 0 || n > 3)
        data.protocol = Protocol::Auto;
    else
        rbProts[n]->setChecked(true);
}

UpDownloadDialog::~UpDownloadDialog()
{
    for (int i = 0; i < 4; ++i)
        rbProts[i] = nullptr;
}

void UpDownloadDialog::on_tbtnShowPassword_pressed()
{
    ui.edtPassword->setEchoMode(QLineEdit::Normal);

}

void UpDownloadDialog::on_tbtnShowPassword_released()
{
    ui.edtPassword->setEchoMode(QLineEdit::Password);

}

void UpDownloadDialog::on_chkUnsecure_toggled(bool checked)
{
    ui.chkUnsecure->setEnabled(checked);
}
