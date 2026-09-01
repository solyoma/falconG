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

int UpDownload::SetupTransfer(UpDownloadParams& params)
{
	// InitCurl() and SetUrlScheme() must be called before calling this function

	bool isUpload = params.Direction() == TransferDirection::upload;
	bool justCheckFile = params.Direction() == TransferDirection::only_check_existence;
	bool getDirList = params.Direction() == TransferDirection::dir_listing;

		// Ensure the URL points at a directory, e.g. ftp://host/incoming/
	if(getDirList && !params.url.path().endsWith('/'))
			params.url.setPath(params.url.path() + '/');

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
	else if (justCheckFile)
	{
		curl_easy_setopt(_pCurl, CURLOPT_NOBODY, 1L);
		curl_easy_setopt(_pCurl, CURLOPT_FILETIME, 1L);
	}
	else if(getDirList)
	{
		curl_easy_setopt(_pCurl, CURLOPT_WRITEDATA, &_dirList);
		curl_easy_setopt(_pCurl, CURLOPT_WRITEFUNCTION,
			+[](char* data, size_t, size_t count, void* out) -> size_t {
				static_cast<QByteArray*>(out)->append(data, qsizetype(count));
				return count;
			});
		if (params.protocol == UpDownloadProtocol::Sftp)	
			curl_easy_setopt(_pCurl, CURLOPT_DIRLISTONLY, 1L); // for sftp only the directory list is standardized, not the sizes and times
		else
			curl_easy_setopt(_pCurl, CURLOPT_CUSTOMREQUEST, "MLSD"); // FTP / FTPS only

		// An MLSD response typically looks like: type=file;size=120;modify=20260829143015; report.pdf
		//  or type=dir;modify=20260828090000; archive
	}
	else
		curl_easy_setopt(_pCurl, CURLOPT_WRITEDATA, _file);


	if (res == CURLE_OK && params.protocol == UpDownloadProtocol::FtpsExplicitTls)
		res = curl_easy_setopt(_pCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

	curl_easy_setopt(_pCurl, CURLOPT_NOPROGRESS, justCheckFile || getDirList ? 1L:0L);        // 0: we want progressCb reports
	curl_easy_setopt(_pCurl, CURLOPT_XFERINFODATA, nullptr);  // we don't use it
	curl_easy_setopt(_pCurl, CURLOPT_XFERINFOFUNCTION, justCheckFile || getDirList ? nullptr : &params.progressCb);

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
	if(_curl_status != CURLE_OK)	// e.g. CURLE_REMOTE_FILE_NOT_FOUND
		return RemoteFileInfo();

	RemoteFileInfo nfo;
	nfo.exists = true;
	curl_off_t timestamp = -1;
	_curl_status = curl_easy_getinfo(_pCurl, CURLINFO_FILETIME_T, &timestamp);
	if(_curl_status != CURLE_OK)
		nfo.modifiedUtc = QDateTime::fromSecsSinceEpoch(timestamp, Qt::UTC);
	return nfo;
}

int UpDownload::_GetFolderListingForSftp(UpDownloadParams& params)
{
	return 0;
}

int UpDownload::GetFolderListings(UpDownloadParams& params)
{
	_entries.clear();

	params.SetDirection(TransferDirection::dir_listing);
	if (params.protocol == UpDownloadProtocol::Sftp)
		return _GetFolderListingForSftp(params);

// FTP FTPS
	if (SetupTransfer(params) != CURLE_OK)
		return 0;
	_curl_status = curl_easy_perform(_pCurl);
	QList<RemoteFileInfo> fi;
	for (const QByteArray& line : _dirList.split('\n')) 
	{
		const int separator = line.indexOf(' ');
		if (separator < 0)
			continue;

		const QList<QByteArray> facts = line.left(separator).split(';');
		RemoteFileInfo entry;
		entry.name = QString::fromUtf8(line.mid(separator + 1)).trimmed();

		for (const QByteArray& fact : facts) 
		{
			const int equals = fact.indexOf('=');
			if (equals < 0) continue;

			const QByteArray key = fact.left(equals).toLower();
			const QByteArray value = fact.mid(equals + 1);

			if (key == "type")
				entry.isDirectory = (value == "dir");
			else if (key == "modify") {
				entry.modifiedUtc = QDateTime::fromString(
					QString::fromLatin1(value.left(14)),
					QStringLiteral("yyyyMMddhhmmss"));
				entry.modifiedUtc->setTimeSpec(Qt::UTC);
			}
		}

		if (!entry.name.isEmpty())
			_entries.append(entry);
	}
	return CURLE_OK;
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
