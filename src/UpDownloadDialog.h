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
        Auto,
        Sftp,
        SftpTls,
        FtpS,
        Ftp
	};
    struct Data
    {
        QString qsServer;
        QString qsRootFolder;
        QString qsUser;
		QString qsPassword; // encoded password, never saved on disk, it resides in memory
		Protocol protocol;
        int port;
        bool rememberUser;
        bool isDownload;    // only true when downloading whole gallery
        bool isValid;
        Data() { Init(); }
        ~Data() {}
        void Init()
        {
            qsServer.clear();
            qsRootFolder.clear();
            qsUser.clear();
		    qsPassword.clear(); // encoded password, never saved on disk, it resides in memory
		    protocol = Protocol::Auto;
            port = 0;
            rememberUser = false;
            isDownload = false;  // only true when downloading gallery
            isValid = false;
        }
	};
public:
    explicit UpDownloadDialog(UpDownloadDialog::Data data, QWidget* parent = nullptr);
    ~UpDownloadDialog();

private:
    Ui::UpDownloadDialogClass ui;
	Data _data;
    QRadioButton* rbProts[3];


private slots:
	void on_tbtnShowPassword_pressed();
	void on_tbtnShowPassword_released();
	void on_chkUnsecure_toggled(bool checked);
};
