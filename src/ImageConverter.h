#pragma once

#include <QtCore>
#include <QString>
#include <QImage>

#include "common.h"
#include "ImageReader.h"
#include "watermark.h"


struct ImageConverter
{			  // if 'dontEnlarge' flag is set only shrinks image, but
			  //	thumbnails may be enlarged
			  // dontResize may be used  for some images
	QString name;
	Common::IcFlags flags;
	// TODO: bool keepAspectRatio = true;	// otherwise CROP image
	double aspect = 0;				// width/height: same for thumbnail unless square thumbnails required (TODO)
									// 0: not calculated

	ImageConverter(Common::IcFlags flags = Common::IcFlag::dontEnlarge) : flags(flags) { }

//	double CalcSizes(ImageReader &reader);	 // set original and new sizes
	int Process(ImageReader &reader, QString dest, QString thumb, WaterMark *pwm=nullptr);	// retuns aspect ratio (0: no src image)

	QString ErrorText() const { return _qsErrorMsg; }
private:
	QImage *_pImg = nullptr;
	QString _qsErrorMsg;

//	double _CalcSizes(bool thumb);			// using oSize, newSize, maxSize for image, newSize, thumbSize, maxThumbSize for thumbnail _
	void _AddWatermark(WaterMark &wm);		// to pImg using data from config
};

