#include "MarkedIcon.h"

#include "thumbnailView.h"

// =================================================================
QPixmap *MarkedIcon::_folderThumbMark = nullptr;
QPixmap *MarkedIcon::_aliasMark = nullptr;
QPixmap *MarkedIcon::_noImageMark = nullptr;
QPixmap *MarkedIcon::_noResizeMark = nullptr;
int  MarkedIcon::_thumbSize = THUMBNAIL_SIZE;		// named image is inside a (size x size) area this keeping aspect ratio
int  MarkedIcon::_borderWidth = _thumbSize / THUMBNAIL_BORDER_FACTOR;				// in pixels portrait image: right and left, landscape image top and bottom
bool MarkedIcon::_initted = false;				// images for icons read?


/*=============================================================
 * TASK:	reads an image into '_pxmp' member from file and
 *			shows it on a square pixmap with '_thumbSize' side
 *			on a background whose color depends on the type of
 *			the image (folder thumbnail or image thumbnail)
 * PARAMS:	name: file name to read image from
 *			iflags: icon flags may contain other flags than
 *					image type
 * GLOBALS:	static members are set
 * RETURNS:	if file read was successful
 * REMARKS: if read is unsuccessfull the pixmap still valid
 *------------------------------------------------------------*/
bool MarkedIcon::Read(QString fname, IconFlags iflags)
{
	_name = fname;
	flags = iflags;

	QString colorname = (flags & fiFolder ? config.albumMatteColor : config.imageMatteColor).Name(true);
	QColor cbck = colorname;
	if (!cbck.isValid())
		return false;

	_pxmp = QPixmap(_thumbSize, _thumbSize);
	_pxmp.fill(cbck);

	QSize dsize;		// thumbnail size
	static QImage img;	// thumbnail image (for video: get from video file)

	auto setDSize = [&](QSize osize)  // get destinaton size for thumbnail image
		{
			if (osize.width() >= osize.height())	// portrait
			{
				dsize.setWidth(_thumbSize - 2 * _borderWidth);
				dsize.setHeight((double)(_thumbSize - 2 * _borderWidth) / (double)(osize.width()) * osize.height());
			}
			else
			{
				dsize.setHeight(_thumbSize - 2 * _borderWidth);
				dsize.setWidth((double)(_thumbSize - 2 * _borderWidth) / (double)(osize.height()) * osize.width());
			}
		};

	if (flags & fiVideo)	// then thumbnail is a frame from the video
	{
		Video* pvid = albumgen.Videos().Find(fname);
		if (!pvid)
			return false;	// no video found

		if (!pvid->GetThumbnail(img, dsize, _thumbSize))
		{
			QMessageBox::warning(nullptr, QMainWindow::tr("falconG - Warning"),
				QMainWindow::tr("Can't get thumbnail for video file '%1'").arg(fname));
			return false;
		}
	}
	else   // image (may be a) folder thumbnail
	{
		QImageReader reader(_name);
		reader.setBackgroundColor(cbck);
		reader.setAutoTransform(true);

		_exists = false;

		QSize osize = reader.size();
		if (!osize.isValid())
			return false;
		setDSize(osize);

		reader.setScaledSize(dsize);

		if (!reader.read(&img))		// maybe error display?
			return false;

		_exists = true;
	}
	QPainter painter(&_pxmp);	// leave the border outside
	int xm =(_pxmp.width() - dsize.width()) / 2, ym = (_pxmp.height() - dsize.height()) / 2;
	painter.drawImage(xm, ym, img);

	return true;
}

/*=============================================================
 * TASK:	from _pxmp returns an icon with markers on it
 * PARAMS:
 * EXPECTS: _pxmp contains the already rendered background
 *			(folders and others) and the image
 * GLOBALS:
 * RETURNS: an icon for image read.
 * REMARKS: - If no markers are to be set on this item returns just
 *			the pixmap '_pxmp'
 *			- thumbnail & alias markers are at top left,
 *				fon't resize and missing flag at top right
 *------------------------------------------------------------*/
QIcon MarkedIcon::ToIcon() const
{
	static IconFlags __flags = { fiFolder, fiThumb, fiDontResize, fiAlias };
	if (_exists && (flags & __flags) ==0)	// no markers on images
		return QIcon(_pxmp);

	QPixmap tmp_pxmp(_thumbSize, _thumbSize);
	QPainter painter(&tmp_pxmp);
	painter.drawPixmap(0,0, _pxmp);		// image with border
	if (flags & fiThumb)	   // at top left position
			painter.drawPixmap(_borderWidth, _borderWidth, *_folderThumbMark);
	if (flags & fiAlias)
		painter.drawPixmap(_borderWidth + (flags & fiThumb ? _aliasMark->width() : 0), _borderWidth, *_aliasMark);

	if (flags & fiDontResize)		  // at top right
		painter.drawPixmap(_thumbSize - _folderThumbMark->width() - _borderWidth, _borderWidth, *_noResizeMark);
	if(!_exists)
		painter.drawPixmap(_thumbSize - 2*_noImageMark->width() - _borderWidth, _borderWidth, *_noImageMark);

	return QIcon(tmp_pxmp);
}

