#include "ImageConverter.h"

#include <QMainWindow>
#include <QImageWriter>

#include "common.h"
#include "config.h"
#include "watermark.h"

/*============================================================================
  * TASK:	If the image reader can read the image sets original and new sizes
  * EXPECTS:	allowed maximum sizes set in constructor
  *				imgReader - reader for file name
  * RETURNS:	aspect ratio or 1.0 if any size is 0
  * GLOBALS:
  * REMARKS: - sizes will reflect the orientation:
  *				when EXIF rotation is used
  *			   image height and width  is swapped
  *			 - sets the scaled dimensions into the reader
  *			 - thumbnail scaling happens in writer
 *--------------------------------------------------------------------------*/
//double ImageConverter::CalcSizes(ImageReader &imgReader)
//{
//	if (!imgReader.canRead())
//		return 0;
//
//	newSize = oSize = imgReader.size();	// actual source image size on disk
//
//	QImageIOHandler::Transformations tr = imgReader.transformation();
//	if (tr & (QImageIOHandler::TransformationRotate90 | QImageIOHandler::TransformationMirrorAndRotate90))
//		oSize.transpose();
//
//	if (!oSize.width() || !oSize.height())
//		aspect =  1.0;
//	else
//	    aspect = (double)oSize.width() / (double)oSize.height(); // < 1: portrait, > 1 landscape orientation
//
//
//	if ((flags & dontResize) == 0)
//	{					// maxSize.x(),y() - new image width & height
//						// maxSize.width(),height() - thumbnail width & height
//		if (aspect >= 1)
//		{
//			if ((newSize.width() > maxSize.x()) || ((newSize.width() < maxSize.x()) && (flags & dontEnlarge) == 0))
//			{
//				newSize.setWidth(maxSize.x());
//				newSize.setHeight(maxSize.x() / aspect);
//			}
//			// thumbs always resized even when it means enlargement
//			_thumbSize.setWidth(maxSize.width());
//			_thumbSize.setHeight(maxSize.width() / aspect);
//		}
//		if (aspect <= 1)
//		{
//			if ((newSize.height() > maxSize.y()) || ((newSize.height() < maxSize.y()) && (flags & dontEnlarge) == 0))
//			{
//				newSize.setHeight(maxSize.y());
//				newSize.setWidth(aspect * maxSize.y());
//			}
//			// thumbs always resized even when it means enlargement
//			_thumbSize.setHeight(maxSize.height());
//			_thumbSize.setWidth(aspect * maxSize.height());
//		}
//	}
//	imgReader.setScaledSize(newSize);	// newSize used in read, _thumbSize used in write
//	return aspect;
//}

/*============================================================================
* TASK:		resize images and add watermark
* EXPECTS:	imgReader - reader with image data set
*			dest - path of destination image
*			thumb - process a thumbnail?
*			ovr - overwrite image if it exists
*			pwm - pointer to watermark structure
*			parameters maxwidth, maxheight,dontEnlarge are set
* RETURNS:	- 0: OK
*			- load errors: -1
*			- if destination exists and it is not allowed to overwrite it: -2
*			- file write error: -3
* GLOBALS:
* REMARKS:	- path of source image must be set into imgReader before calling
*			- for thumbnails if the image was already loaded into imgReader
*				then scale image during save, else save the image as it is
*			- if there's an error sets _qsErrorMsg
*--------------------------------------------------------------------------*/
int ImageConverter::Process(ImageReader &imgReader, QString dest, QString thumb, WaterMark *pwm)
{
	if (QFile::exists(dest) && !config.bOvrImages)
	{
		_qsErrorMsg = QMainWindow::tr("Destination file") + QString(" ,%1' ").arg(dest) + QMainWindow::tr("exists and image override is not allowed!");
		return -1;
	}
	QImageIOHandler::Transformations trans = imgReader.transformation();
	QSize newSize = imgReader.imgSize;	// if 'trans' then it may already transposed sizes (from camera, not from PS/LR)
	if (trans & (QImageIOHandler::TransformationRotate90 | QImageIOHandler::TransformationMirrorAndRotate90))
		newSize.transpose();
	imgReader.setScaledSize(newSize);	// rescale to newSize when read, _thumbSize used in write

	if (!imgReader.isReady)			// not read yet
	{
		if (!imgReader.read())		// scaled and possibly rotated image
		{
			_qsErrorMsg = imgReader.errorString() + "\n'" + imgReader.fileName() + "'\n";
			return -2;
		}

		_pImg = &imgReader.img;		// must set here to be used in _AddWatermark
		if (pwm)
			_AddWatermark(*pwm);	// onto _pImg
	}

	// write scaled image into 'dest'
	if(flags & IcFlag::prImage)
	{
		QImageWriter imageWriter(dest);
		int quality = imgReader.quality();
		if (config.imageQuality > 0)
			quality = config.imageQuality;

		imageWriter.setQuality(quality);
		imageWriter.setFormat(imgReader.format());

		if (!imageWriter.write(imgReader.img))
		{
			_qsErrorMsg = imageWriter.errorString() + "\n'" + dest + "'\n";
			return -3;
		}
	}
	// write thumbnail image into 'thumb'
	// thumbnail image dimensions are never transposed
	// and all thumbnail images have the same height
	// which is set in 'config'
	if(flags & IcFlag::prThumb)
	{
		if (imgReader.thumbSize.width() <= 0 || imgReader.thumbSize.height() <= 0)
		{
			_qsErrorMsg = QMainWindow::tr("Invalid sizes for thumbnail") + QString(" \n'"+thumb + "'\n");
			return -aspect;
		}
		//	re-scale image for thumbnail
		imgReader.img = imgReader.img.scaled(imgReader.thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QImageWriter imageWriter(thumb);
		imageWriter.setQuality(imgReader.quality());
		imageWriter.setFormat(imgReader.format());
		if (!imageWriter.write(imgReader.img))
		{
			_qsErrorMsg += "'" + thumb + "'\n" + imageWriter.errorString();
			return -3;
		}
	}
	return 0;
}

/*============================================================================
* TASK:		add a watermark to the image '_pimg' points to
* EXPECTS: 	wm is a filled in watermark structure with valid mark image
*			_pImg points to existing image into which the watermark will
*			be printed
*			width, height, etc are set up
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void ImageConverter::_AddWatermark(WaterMark & wm)
{
	if (!wm.PMarkImage())		// no watermark image
		return;

	int markWidth = wm.Width(),
		markHeight = wm.Height();
	// combine the text with the image

	int x0=0, y0=0;		// origin on image for watermark
	int imgWidth = _pImg->width(), imgHeight = _pImg->height();

	switch ((wm.Origin() & 0xF0) >> 4)
	{
		case 0: x0 = wm.MarginX(); break;
		case 1: x0 = (imgWidth - markWidth) / 2; break;
		case 2: x0 = imgWidth - markWidth - wm.MarginY(); break;
	}
	if (x0 < 0)
		x0 = 0;
	switch ((wm.Origin() & 0xF))
	{
		case 0: y0 = wm.MarginY(); break;
		case 1: y0 = (imgHeight - markHeight) / 2; break;
		case 2: y0 = imgHeight - markHeight - wm.MarginY(); break;
	}
	if (y0 < 0)
		y0 = 0;

   // draw the watermark
	QPainter painter(_pImg);
	if (imgWidth < markWidth)
	{
		QRect	srect(0,0, markWidth, markHeight),
				drect(x0,y0, imgWidth, markHeight);	// source and destination

		painter.drawImage(drect, *wm.PMarkImage(), srect);
	}
	else
		painter.drawImage(QPoint(x0, y0), *wm.PMarkImage());

	// DEBUG
	//wm.PMarkImage()->save(PROGRAM_CONFIG::samplePath + "\\watermark.png");
	//QBrush brush(Qt::red);
	//painter.setPen(QPen(brush, 5));
	//painter.drawLine(x0, y0, x0+markWidth, y0);
	//painter.drawLine(x0+markWidth, y0,x0+markWidth, y0+markHeight);
	//painter.drawLine(x0, y0+markHeight,x0+markWidth, y0+markHeight);
	//painter.drawLine(x0, y0+markHeight,x0,y0);
	//_pImg->save(PROGRAM_CONFIG::samplePath + "\\watermarkedImage.png");
	// DEBUG
}

