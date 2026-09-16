#pragma once

#include <QtCore>
#include<QImage>
#include<QImageReader>

struct ImageReader : public QImageReader
{
	QSize thumbSize = { 0, 0 },	// these must be set before processing thumbnails
		imgSize;
	QImage img;					// read scaled image into this
	bool isReady = false;
	bool read()
	{
		return isReady = QImageReader::read(&img); // && !img.isNull();
	}
	bool canRead() { return (isReady ? true : QImageReader::canRead()); }
	ImageReader(QIODevice* device, bool dontResize = false, const QByteArray& format = QByteArray()) : QImageReader(device, format)
	{
		setAutoTransform(true);  // auto rotate, flip and mirror during read() when portrait orientation is set in EXIF
	}
	ImageReader(const QString& fileName, bool dontResize = false, const QByteArray& format = QByteArray()) : QImageReader()
	{
		setAutoTransform(true);  // auto rotate, flip and mirror during read() when portrait orientation is set in EXIF
		setFileName(fileName);	 // creates and opens a read only QFile
		setFormat(format);		 // e.g. "jpg" (same as "JPG"), empty: cycle through supported formats until one found
	}
};