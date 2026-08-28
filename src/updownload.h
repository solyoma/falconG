#pragma once
#include "curl\curl.h"

#include <QString>

class UpDownload
{
public:
	UpDownload();
	~UpDownload();
	bool DownloadFile(const QString& url, const QString& outputPath);
	bool UploadFile(const QString& url, const QString& filePath);
private:
	bool _curl_status = false;
};