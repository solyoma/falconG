#pragma once
#include <QDialog>
#include <QRadioButton>
#include <QString>

#include "ui_UpDownloadDialog.h"

class UpDownloadDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Protocol
    {
        Auto,      // 0
        Sftp,      // 1  (22)
        FtpsTls,   // 2  (21)
        Ftps,      // 3  (990)
        Ftp        // 4  (21)
	};
    struct Data
    {
        QString qsServer;
        QString qsRootFolder;
        QString qsUser;
		QString qsPassword; // encoded password, never saved on disk, it resides in memory
		Protocol protocol;  // active protocol, gives the port inde too
        int ports[5]; // defaults { 0, 22,21,990,21 };
        bool isDownload;    // only true when downloading whole gallery
        bool isValid;
        Data() { Init(); }
        ~Data() {}
        void SetPortsFromString(QString s)
        {
            _SetDefPorts();
            QStringList sl = s.split(',');
            for (int i = 0; i < sl.size() && i < 4; ++i)
                ports[i+1] = sl[i].toInt();
        }
        QString PortsAsString() const
        {
            return QString("%1,%2,%3,%4").arg(ports[1]).arg(ports[2]).arg(ports[3]).arg(ports[4]);
        }
        void Init()
        {
            qsServer.clear();
            qsRootFolder.clear();
            qsUser.clear();
		    qsPassword.clear(); // encoded password, never saved on disk, it resides in memory
		    protocol = Protocol::Auto;
            isDownload = false;  // only true when downloading gallery
            isValid = false;
            _SetDefPorts();
        }
    private:
        void _SetDefPorts()
        {
            ports[0] = 0;    // auto
            ports[1] = 22;   // sftp
            ports[2] = 21;   // ftps+tls
            ports[3] = 990;  // ftps
            ports[4] = 21;  // ftp
        }
	};
public:
    explicit UpDownloadDialog(UpDownloadDialog::Data &data, QWidget* parent = nullptr);
    ~UpDownloadDialog();

private:
    Ui::UpDownloadDialogClass ui;
	Data &_data;
    QRadioButton* rbProts[5];
    QSpinBox* sbPorts[5];
    bool _busy;

private slots:
	void on_tbtnShowPassword_pressed();
	void on_tbtnShowPassword_released();

    void on_edtUser_textChanged(const QString& txt);
    void on_edtPassword_textChanged(const QString& txt);

    void on_rbProtAuto_toggled(bool on);
    void on_rbProtSftp_toggled(bool on);
    void on_rbProtFtps_toggled(bool on);
    void on_rbProtFtpsTls_toggled(bool on);
    void on_rbProtFtp_toggled(bool on);

    void on_sbPortSftp_valueChanged(int value);
    void on_sbPortFtps_valueChanged(int value);
    void on_sbPortFtpsTls_valueChanged(int value);
    void on_sbPortFtp_valueChanged(int value);

	void on_chkUnsecure_toggled(bool checked);
};
