#include "UpDownloadDialog.h"

UpDownloadDialog::UpDownloadDialog(UpDownloadDialog::Data &data, QWidget* parent)
                                    : QDialog(parent), _data(data)
{
    ui.setupUi(this);

    _busy = true;

    if (_data.isDownload)
    {
        ui.lblUpDownload->setText(tr("Download from"));
        ui.lblToFromDir->setText(tr("From folder"));
    }

	ui.lblUrl->setText(_data.qsServer);
	ui.lblPublicHtml->setText(_data.qsRootFolder);
	ui.edtUser->setText(_data.qsUser);
    ui.edtPassword->setText(_data.qsPassword);
    ui.chkUnsecure->setChecked(_data.protocol == Protocol::Ftp);
    rbProts[0] = ui.rbProtAuto;
    rbProts[1] = ui.rbProtSftp;
    rbProts[2] = ui.rbProtFtpsTls;
    rbProts[3] = ui.rbProtFtps;
    rbProts[4] = ui.rbProtFtp;

    sbPorts[0] = nullptr;  // auto
    sbPorts[1] = ui.sbPortSftp;  
    sbPorts[2] = ui.sbPortFtpsTls;
    sbPorts[3] = ui.sbPortFtps;
    sbPorts[4] = ui.sbPortFtp;


    int n = (int)_data.protocol;
    if (n < 0 || n > 4)
        _data.protocol = Protocol::Auto;
    else
        rbProts[n]->setChecked(true);

    for (int i = 1; i < 5; ++i)
        sbPorts[i]->setValue(_data.ports[i]);

    _busy = false;
}

UpDownloadDialog::~UpDownloadDialog()
{
    for (int i = 0; i < 4; ++i)
    {
        rbProts[i] = nullptr;
        sbPorts[i] = nullptr;
    }
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
	ui.rbProtFtp->setEnabled(checked);
	if (!checked && ui.rbProtFtp->isChecked())
		ui.rbProtAuto->setChecked(true);
}

void UpDownloadDialog::on_edtUser_textChanged(const QString& txt)
{
    if (_busy)
        return;
    _data.qsUser = ui.edtUser->text();
}
void UpDownloadDialog::on_edtPassword_textChanged(const QString& txt)
{
    if (_busy)
        return;
    _data.qsPassword = ui.edtPassword->text();
}
// ports
void UpDownloadDialog::on_rbProtAuto_toggled(bool on)
{
    if (_busy)
        return;
    if (on)
        _data.protocol = Protocol::Auto;
}
void UpDownloadDialog::on_rbProtSftp_toggled(bool on)
{
    if (_busy)
        return;
    if (on)
        _data.protocol = Protocol::Sftp;
}
void UpDownloadDialog::on_rbProtFtpsTls_toggled(bool on)
{
    if (_busy)
        return;
    if (on)
        _data.protocol = Protocol::FtpsTls;
}
void UpDownloadDialog::on_rbProtFtps_toggled(bool on)
{
    if (_busy)
        return;
    if (on)
        _data.protocol = Protocol::Ftps;
}
void UpDownloadDialog::on_rbProtFtp_toggled(bool on)
{
    if (_busy)
        return;
    if (on)
        _data.protocol = Protocol::Ftp;
}

void UpDownloadDialog::on_sbPortSftp_valueChanged(int value)
{
    if (_busy)
        return;
    _data.PortsAsString();
}
void UpDownloadDialog::on_sbPortFtps_valueChanged(int value)
{
    if (_busy)
        return;
    _data.PortsAsString();
}

void UpDownloadDialog::on_sbPortFtpsTls_valueChanged(int value)
{
    if (_busy)
        return;
    _data.PortsAsString();
}

void UpDownloadDialog::on_sbPortFtp_valueChanged(int value)
{
    if (_busy)
        return;
    _data.PortsAsString();
}

