#pragma once
#include "curl\curl.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <functional>
#include <optional>

#include <QUrl>
#include <QFile>
#include <cstring>	// for memset
#include <xkeycheck.h>

#include "curl/curl.h"

enum class UpDownloadProtocol{ any, Sftp, FtpsExplicitTls, FtpsImplicitTls, Ftp };
enum class TransferDirection {upload, download, bidirect, only_check_existence, dir_listing};

/*struct ProgressData
{
	void* clientp; 		  // not used by curl, just transferred to the callback function  
	curl_off_t dltotal;   // expect this many bytes to download, upload only then = 0	  
	curl_off_t dlnow;	  // bytes downloaded so far, when upload only then = 0			  
	curl_off_t ultotal;	  // expect this many bytes to upload, when download only then = 0
	curl_off_t ulnow;	  // bytes uploaded so far, when download only then = 0			  

	ProgressData() { std::memset(this, 0, sizeof(*this)); }
	ProgressData(void* clientp,	curl_off_t dltotal,	
				   curl_off_t dlnow, curl_off_t ultotal,curl_off_t ulnow):
		clientp(clientp), dltotal(dltotal), dlnow(dlnow), ultotal(ultotal), ulnow(ulnow) {
	}
};
*/

// This is the transfer callback type that can be set with CURLOPT_XFERINFOFUNCTION
// which is called by curl frequently during transfer, rarely when idle
// Arguments: clientp - not used by curl, just transferred to the callback function
//						can point to anything, even to the callback function itself
//						or any data structure
//			  dltotal - expect this many bytes to download, 0 when upload only
//			  dlnow   - bytes downloaded so far, 0 when upload only
//			  ultotal - expect this many bytes to upload, 0 when download only
//			  ulnow   - bytes uploaded so far, 0 when download only
// Should return: 0 to continue, 1 to abort transfer
//				  or CURL_PROGRESSFUNC_CONTINUE so the transfer continues 
//				  executing the default progress function.
using ProgressCallback = std::function<int(void* clientp, curl_off_t dltotal,
				curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)>;

struct UpDownloadParams
{
	QUrl url;			// the url to download from or upload to, must be set before calling any transfer function
	QString filePath;	// source or destination
	QString userName;	// on server
	UpDownloadProtocol protocol=UpDownloadProtocol::any;  // any:try all. Otherwise port from config is used
	ProgressCallback progressCb;	// ProgressCallback();
								//   clientp set by CURLOPT_XFERINFODATA, not used, just passed along to the callback 'progressCb'
								//   dltotal - 
								//	 dlnow - 
								//	 ultotal,ulnow smilar but for upload


	UpDownloadParams() {}
	UpDownloadParams(const UpDownloadParams &params) 
	{
		Setup(params.url, params.filePath, params.userName, params._password, params.protocol, params.progressCb);
	}

	UpDownloadParams(const QUrl &url, const QString &file, const QString userName, 
					/* !const */ QString pwd = "*", const UpDownloadProtocol prot = UpDownloadProtocol::any, 
					ProgressCallback callback=nullptr)
	{
		Setup(url, file, userName, pwd, prot, callback);
	}

	constexpr void SetTransferDirection(TransferDirection dir)
	{
		_direction = dir;
	}
	constexpr TransferDirection Direction() const { return _direction; }

	void Setup(const QUrl& theUrl, const QString file, const QString user, /* !const */ QString pwd, const UpDownloadProtocol prot, ProgressCallback callback)
	{
		url = theUrl; filePath = file; userName = user; protocol = prot; progressCb = callback;
		SetPassword(pwd);
	}

	void SetPassword(QString &pwd)
	{
		_EncodePasswordFrom(pwd);
	}
	constexpr void SetDirection(TransferDirection dir)
	{
		_direction = dir;
	}

	const QString DecodedPassword()	const
	{
		QString qs;
		_DecodePasswordTo(qs);
		return qs.toUtf8();
	}

	void Clear() 
	{ 
		url.clear(); 
		filePath.clear(); 
		userName.clear(); 
		_password.clear(); 
		progressCb = nullptr;
		protocol = UpDownloadProtocol::any;
	}
private:
	QString _password;
	TransferDirection _direction = TransferDirection::download;

	void _EncodePasswordFrom(const QString &pwd);
	void _DecodePasswordTo(QString &pwd) const; 
};

class UpDownload
{
public:
	UpDownload();
	~UpDownload();
public:
	struct RemoteFileInfo
	{
		bool exists = false;	// unly use for single file test
		bool isDirectory = false;  // only used for folder listing
		QString name;			// only used for folder listing
		std::optional<QDateTime> modifiedUtc;  // to check if set: if(modifiedUtc) ...

	};
public:
	bool InitCurl() //call once before doing anything
	{
		_pCurl = curl_easy_init();
		if (!_pCurl)
			return _curl_status = CURLE_FAILED_INIT;
	}
	void CleanupCurl() //call after all transfers are finished
	{
		curl_easy_cleanup(_pCurl);
		_pCurl = nullptr;
	}
	constexpr int Status(bool clear = false)
	{
		int res = _curl_status;
		if (clear)
			_curl_status = CURLE_OK;
			
		return res;
	}
	constexpr bool StatusOk() const {return _curl_status == CURLE_OK;}

	int SetupTransfer(UpDownloadParams& params);	// before any transfer
	bool OpenLocalFileForTransfer(UpDownloadParams& params);
	int DownloadFile(UpDownloadParams &params);	// set url scheme and callback function into 'params' before calling this
	int UploadFile(UpDownloadParams& params);		// -"-
	RemoteFileInfo GetRemoteFileInfo(UpDownloadParams& params);
	int GetFolderListings(UpDownloadParams& params);

	void SetUrlScheme(QUrl &url, UpDownloadProtocol uproto);  // url is modified
	QStringList DownloadCatalog(const QUrl& fromUrl);		 // of files on server in folder given by url
	int DownloadFolder(const QUrl& fromUrl, const QString& toLocalFolder, bool recursively = false);
	int Synchronize(const QUrl& withUrl, bool onlyUploadAndNoDeletionFromServer = true);

private:
	int _curl_status = CURLE_OK;
	CURL* _pCurl = nullptr;
	QFile *_file = nullptr;
	std::optional<QDateTime> _modifiedUtc;
	QByteArray _dirList;			// only used for directory listing
	QList<RemoteFileInfo> _entries;	// only used for directory listing

	int _GetFolderListingForSftp(UpDownloadParams& params);

	void _CloseLocalFile() 
	{ 
		if (_file)
			_file->close();
		_file = nullptr;
	}
};
