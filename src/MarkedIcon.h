#pragma once

#include <QString>
#include <QPixmap>

#include "common.h"

//*****************************************
// read an image from disk or resource, rotate it on read if needed,
//  resize it to w and h keeping aspect ratio
//  add a square icon overlay at given postion from the right
class MarkedIcon
{
	QString _name;			// full path name of image for which we want an icon
	QPixmap _pxmp;			// square pixmap contains image with a 'margin' wide border
	bool _exists = false;
		// these ar used for each thumbnail
		// QPixmaps can only be initialized after the GUI initialize (QT quirk)
		// so we need pointers here
	static QPixmap *_folderThumbMark;	// if folder thumbnail mark with this
	static QPixmap *_aliasMark;		// if folder is an alias
	static QPixmap *_noImageMark;	// image does not exist
	static QPixmap *_noResizeMark;	// do not resize image
	static int  _thumbSize;			// named image is inside a (size x size) area this keeping aspect ratio
	static int  _borderWidth;
	static bool _initted;
public:
	Common::IconFlags flags;		// flags: fiFolder, fiThumb, fiAlias, fiDontResize, fiVideo, fiImage
	MarkedIcon()
	{
		if (!_initted)
			Init();
	}

	MarkedIcon(const MarkedIcon& other)
	{
		(void) operator=(other);
	}

	MarkedIcon& operator=(const MarkedIcon& other)
	{
		flags = other.flags;
		_exists = other._exists;
		_name = other._name;
		_pxmp = other._pxmp;
		return *this;
	}

	~MarkedIcon()	{	}

	static void Init()
	{
		if (_initted)
			return;

		_folderThumbMark = new QPixmap(":/icons/Resources/folderIcon.png");
		_aliasMark		 = new QPixmap(":/icons/Resources/aliasIcon.png");
		_noImageMark	 = new QPixmap(":/icons/Resources/noImageMark.png");
		_noResizeMark	 = new QPixmap(":/icons/Resources/noResizeMark.png");
		_initted		 = true;
	}
	static void SetMaximumSizes(int size, int margin)
	{
		_thumbSize = size;
		_borderWidth = margin;
	}

	inline bool ToggleFolderThumbFlag()
	{
		if(flags.testFlag(Common::fiThumb))
		{	flags ^= Common::fiThumb;  // no clearFlag operation in flags
			return false;
		}
		else
		{
			flags.setFlag(Common::fiThumb);
			return true;
		}
	}
	void ToggleAliasFlag()
	{
		if(flags.testFlag(Common::fiAlias))
			flags ^= Common::fiAlias;  // no clearFlag operation in flags
		else // if(!setta)
			flags |= Common::fiAlias;	// set alias flag
	}
	void ToggleNoResizeFlag()
	{
		if(flags.testFlag(Common::fiDontResize))
			flags ^= Common::fiDontResize;  // no clearFlag operation in flags
		else // if(!noresize)
			flags.setFlag(Common::fiDontResize);
	}

	bool Read(QString name, Common::IconFlags flag);	// to _pxmp, transforms rotated image on read, sets 'exists'
	QIcon ToIcon() const;
};
//QImage ReadAndMarkImage(QString name, int w, int h, bool exists, QString icon, int pos);
