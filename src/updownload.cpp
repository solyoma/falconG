#include "updownload.h"

UpDownload::UpDownload()
{
	if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) 
		_curl_status = false;
	else
		_curl_status = true;
}

UpDownload::~UpDownload()
{
	curl_global_cleanup();
}

bool UpDownload::DownloadFile(const QString& url, const QString& outputPath)
{
	return false;
}

bool UpDownload::UploadFile(const QString& url, const QString& filePath)
{
	return false;
}
