#include "updownload.h"
// #include "curl/curl.h" in updownload.h

void UpDownload::SetUrlScheme(QUrl& url, UpDownloadProtocol protocol)
{
	switch (protocol)
	{
		case UpDownloadProtocol::Sftp:
			url.setScheme("sftp");
			break;
		case UpDownloadProtocol::FtpsImplicitTls:
			url.setScheme("ftps");  // TLS immediately on connect
			break;
		case UpDownloadProtocol::FtpsExplicitTls:
		case UpDownloadProtocol::Ftp:
			url.setScheme("ftp");
			break;
	}
}

UpDownload::UpDownload()
{
	int isSslOk = curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL);
	int isInitOk = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (isSslOk != CURLSSLSET_OK || isInitOk != CURLE_OK)
		_curl_status = CURLE_FAILED_INIT;
	else
		_curl_status = CURLE_OK;
}

UpDownload::~UpDownload()
{
	curl_global_cleanup();
}

QStringList  UpDownload::DownloadCatalog(const QUrl& url)
{
	return QStringList();
}

int UpDownload::DownloadFolder(const QUrl& fromUrl, const QString& toLocalFolder, bool recursively)
{
	return false;
}

int UpDownload::SetupTransfer(const UpDownloadParams& params)
{
	// InitCurl() and SetUrlScheme() must be called before calling this function

	bool isUpload = params.Direction() == TransferDirection::upload;
	bool justCheck = params.Direction() == TransferDirection::only_check_existence;

	CURLcode res = curl_easy_setopt(_pCurl, CURLOPT_URL, params.url.toEncoded().constData()); // suppose it will be the same for all subsequent calls
	curl_easy_setopt(_pCurl, CURLOPT_USERNAME, params.userName.toUtf8().constData());
	curl_easy_setopt(_pCurl, CURLOPT_PASSWORD, params.DecodedPassword().constData());
	curl_easy_setopt(_pCurl, CURLOPT_UPLOAD, isUpload ? 1L : 0L);
	if (isUpload)
	{
		curl_easy_setopt(_pCurl, CURLOPT_READDATA, _file);
		curl_easy_setopt(_pCurl, CURLOPT_INFILESIZE_LARGE,
			static_cast<curl_off_t>(_file->size()));
	}
	else if (justCheck)
	{
		curl_easy_setopt(_pCurl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(_pCurl, CURLOPT_FILETIME, 1L);
	}
	else
		curl_easy_setopt(_pCurl, CURLOPT_WRITEDATA, _file);


	if (res == CURLE_OK && params.protocol == UpDownloadProtocol::FtpsExplicitTls)
		res = curl_easy_setopt(_pCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

	curl_easy_setopt(_pCurl, CURLOPT_NOPROGRESS, justCheck ? 1L:0L);        // 0: we want progressCb reports
	curl_easy_setopt(_pCurl, CURLOPT_XFERINFODATA, nullptr);  // we don't use it
	curl_easy_setopt(_pCurl, CURLOPT_XFERINFOFUNCTION, justCheck ? nullptr : &params.progressCb);

	return _curl_status = res;
}

bool UpDownload::OpenLocalFileForTransfer(UpDownloadParams& params)
{
	bool isUpload = params.Direction() == TransferDirection::upload;
	_file = new QFile(params.filePath);
	if (!_file->open(isUpload ? QIODevice::ReadOnly : QIODevice::WriteOnly))
		return isUpload ? CURLE_READ_ERROR : CURLE_WRITE_ERROR;

	return true;
}

int UpDownload::DownloadFile(UpDownloadParams& params)
{
	if(!OpenLocalFileForTransfer(params))
		return _curl_status = CURLE_FAILED_INIT;

	if (SetupTransfer(params) != CURLE_OK)
	{
		_CloseLocalFile();
		return _curl_status = CURLE_FAILED_INIT;
	}

	_curl_status  = curl_easy_perform(_pCurl);

	return _curl_status;
}

int UpDownload::UploadFile(UpDownloadParams& params)
{
	if(!OpenLocalFileForTransfer(params))
		return _curl_status = CURLE_FAILED_INIT;

	if (SetupTransfer(params) != CURLE_OK)
	{
		_CloseLocalFile();
		return _curl_status = CURLE_FAILED_INIT;
	}

	return _curl_status = curl_easy_perform(_pCurl);
}

UpDownload::RemoteFileInfo UpDownload::GetRemoteFileInfo(UpDownloadParams& params)
{
	if (SetupTransfer(params) != CURLE_OK)
		return RemoteFileInfo();
	_curl_status = curl_easy_perform(_pCurl);
	if(_curl_status != CURLE_OK)
		return RemoteFileInfo();

	RemoteFileInfo nfo;
	nfo.exists = true;
	curl_off_t timestamp = -1;
	_curl_status = curl_easy_getinfo(_pCurl, CURLINFO_FILETIME_T, &timestamp);
	if(_curl_status != CURLE_OK)
		nfo.modifiedUtc = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);
	return nfo;
}


int UpDownload::Synchronize(const QUrl& withUrl, bool onlyUploadAndNoDeletionFromServer)
{
	return false;
}

void UpDownloadParams::_EncodePasswordFrom(const QString& pwd)
{
	_password = pwd;
}

void UpDownloadParams::_DecodePasswordTo(QString& pwd) const
{
	pwd = _password;
}
