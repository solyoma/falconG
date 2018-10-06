#include "support.h"
#include <QtDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FileReader::FileReader(const QString s, QFlags<FrfFlags> flags) : _flags(flags)
{
	//	QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");

	_f.setFileName(s);
	_ok = _f.open(QIODevice::ReadOnly);
	if (_ok)
	{
		_fileSize = _f.size();
		_bytes = new char[_BUFFER_SIZE + 4];			// +4: place for ending 0
	}
}

/*============================================================================
* TASK:	reads next non empty non comment line int _line until EOF on the stream
*       and converts it to UTF8 if unless it is UTF8 already
* EXPECTS: nothing
* GLOBALS:
* REMARKS:	- result in _line is always Utf8
*			- at end of file or on read error _ok is set to false
*			- the purpose of '_lastCharRead' is that with 
*				windows text files using CR-LF pairs as line delimiter
*			    sometimes buffer ends with CR and LF is overflowed to the next buffer
*--------------------------------------------------------------------------*/
void FileReader::_readBinaryLine()		// 
{
	_line.clear();
	if (!_ok)
		return;

	QByteArray ba;

	static char _lastCharRead = 0;
	char ch;
	do
	{
		if (!_bsize)		// nothing in memory
		{
			_bsize = _BUFFER_SIZE;
			if (_bytesRead + _bsize > _fileSize)
				_bsize = _fileSize - _bytesRead;
			if (_bsize <= 0)
			{
				_ok = false;
				_line.clear();
				return;
			}
			_bsize = _f.read(_bytes, _bsize);
			if (_bsize > 0)
				_bytesRead += _bsize;
					// skip if first character is a continuation of the 
					// windows line ending sequence
			_bpos = _lastCharRead == 13 && _bytes[0] == 10 ?  1 :0;
			_lastCharRead = _bytes[_bsize - 1];	// store last char
		}
		int epos = _bpos;	// search for line end position
		while (epos < _bsize - 1 && _bytes[epos] != '\r' && _bytes[epos] != '\n')
			++epos;

		ch = _bytes[epos];
		if (ch != '\n' && ch != '\r')		// then end of buffer, but not end of line
		{
			++epos;				// to overflow bytes
			ch = _bytes[epos];
		}
		_bytes[epos] = 0;		// set EOS

		if( (!_bpos && !epos) || (epos != _bpos))	// when a previous buffer ended just before a CR LF pair
		{							// both _bpos and epos will be 0, but the line is not empty
			ba += (_bytes + _bpos);
			if ( (ch == '\n' || ch == '\r')	||		// then at end of line and not end of buffer
				 (_bytesRead == _fileSize) )		// at end of file a missing CR or LF is not a problem
			{
				if (!ValidUtf8String(ba, ba.length()))
					_line = QString::fromLocal8Bit(ba, ba.length());
				else
					_line = ba;			// add to data
			}
		}
		// skip line end character(s)
		if ((ch == '\n' && _bytes[epos + 1] == '\r') || (ch == '\r' && _bytes[epos + 1] == '\n'))
			epos += 2;  // CR LF or LF CR pair for line end, must skip both
		else
			++epos;		// LF or CR only: skip it
		_bpos = epos;		// possible start of next line, unless
		if (_bpos >= _bsize)	// we are at end of the buffer
			_bsize = 0;
	} while (ch != '\n' && ch != '\r');

	if (_ok || !_line.isEmpty())
		++_readLineCount;
}


/*============================================================================
* TASK:	reads next line until EOF on the stream
* EXPECTS: '_flags; field is set up
* GLOBALS:
* RETURNS:  nothing
* REMARKS:	- reads line in binary mode 
*				_line read is always UTF-8 encoded
*			- '_flags' determine what to do with line
*				frfAllLines - keep all lines even that of only white spaces
*				frfEmptyLines - keep empty lines (white spaces are collapsed)
*				frfCommentLines - keep comment (starting with a '#' (in) lines
*				frfLTrim - trim white spaces from the left
*				frfRTrim - trim white spaces from the right
*				frfTrim  - trim white spaces from the left and right
*				frfNoWhiteSpaceLines - clear lines of only white spaces
*					not set: keep those lines intact
*				frfNeedUtf8 - convert lines to UTF-8 when needed
*			- sets _ok to false at EOF but there may be a partial line read in
*--------------------------------------------------------------------------*/
void FileReader::_readLine()
{
	_line.clear();
	if (!_ok)
		return;

	int pos, pos1;
	do
	{
		NextLine();		// read in line from file possibly convert it to UTF-8
						// and set '_ok'

		if (_flags & frfAllLines)			// no trimming, no exclusions
			break;

		if (_flags ^ frfCommentLines)		// if comments not allowed
		{
			pos = _line.indexOf('#');		
			if (pos >= 0)					// and there is one
				_line = _line.left(pos);	// delete it
		}

		pos = 0;						   // trim line from this
		pos1 = _line.length() - 1;		   // till this
		if (_flags & (frfNoWhiteSpaceLines | frfLtrim | frfTrim))
		{
			for (    ;  pos < _line.length() && _line[pos].isSpace(); ++pos)
				;
		}
		if (_flags & (frfNoWhiteSpaceLines | frfRtrim | frfTrim))
		{
			for (   ; pos1 >= 0 && _line[pos1].isSpace(); --pos1)
				;
		}
		if (_flags & frfNoWhiteSpaceLines  && pos > pos1)									// line with white spaces only
			_line.clear();

		if ((_flags & frfLtrim) == 0)
			pos = 0;
		if ((_flags & frfRtrim) == 0)
			pos1 = _line.length() - 1;

		if ( pos > 0 || pos1 < _line.length() - 1)
			_line = _line.mid(pos, pos1 - pos + 1);

		if ((_flags & frfEmptyLines))					// then _line is alwaysOK
			break;
	} while (_line.isEmpty() && _ok);
}

/*============================================================================
* TASK:	   read any line depending on 'flags'(even empty and comment lines) 
* EXPECTS: flags: any combination of FrfFlags or frfNormal(0)
* RETURNS: 
* GLOBALS:
* REMARKS:  - default flags is frfNormal (i.e. use internal _flags)
*			- does not change '_flags'
*--------------------------------------------------------------------------*/
QString FileReader::ReadLine(int thisflags)
{
	if (!_ok)
		return QString();

	int saveFlags = frfNormal;		// means: keep all flags in _flags
	if (thisflags != frfNormal)		// then use 'thisflags' instead
	{
		saveFlags = _flags;
		_flags = thisflags;
	}

	_readLine();		
	if (flags != frfNormal)
		_flags = saveFlags;

	return _line;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QStringList FileReader::ReadAndSplitLine(QChar sep)
{
	ReadLine();
	if (_ok && !_line.isEmpty())
		return _line.split(sep);

	return QStringList();
}

/*============================================================================
* TASK:		Read line even when it is empty
* EXPECTS:
* GLOBALS:
* REMARKS: lines containing whitespace(s) only	will be cleared of it
*--------------------------------------------------------------------------*/
QString FileReader::NextLine()
{
	_flags |= frfNoWhiteSpaceLines;
	_readBinaryLine();
	if (!_line.isEmpty() && _line.trimmed().isEmpty())
		_line.clear();
	return _line;
}

//*****************************************************

/*============================================================================
* TASK:		renames file as backup (add a ~) and rename tmp file to original name
* EXPECTS:	name - name of existing file to be renamed
*			tmpName - name of yet temporary file to be renamed to 'name'
*			parent: pointer to widget used for QMessageBox
*			keepbackup: if true do not rename 'name' to 'name~' but delete it
* GLOBALS:	none
* REMARKS:
*--------------------------------------------------------------------------*/
bool BackupAndRename(QString name, QString tmpName, QWidget *parent, bool keepBackup)
{
	QFile ft(name), f(tmpName);
	bool bErr = false;
	QString st = QMainWindow::tr("falconG error"), sx;
	if (keepBackup)
	{
		if(! ft.remove(name) || !f.rename(name) )
		{
			sx = QMainWindow::tr("Either can't delete \n'%1'\n"
								 " or can't rename '%2'\n"
								 "Modified file remain named as \n'%2'").arg(name).arg(tmpName);
			bErr = true;
		}
	}
	else
	{
		ft.remove(name + "~");
		if ((ft.exists() && !ft.rename(name + "~")) || !f.rename(name))
		{
			sx = QMainWindow::tr("Can't rename \n'%1'\n to \n'%1~'\n"
								  "New file remain named as \n'%2'").arg(name).arg(tmpName);
			bErr = true;
		}
	}
	if (bErr)
	{
		QMessageBox(QMessageBox::Warning, st, sx, QMessageBox::Close, parent).exec();
		return false;
	}
	return true;
}

/* --------------------------------- helper functions -----------------------*/
/*============================================================================
* TASK:		 separates an absolute or relative path file or directory path 
*			 into its components
* EXPECTS:	fullName -  file or directory path (IN) Directories must end in '/'
*			path     -  file or parent directory path (OUT) empty or ends with '/'
*			name	 -  (OUT) if pext is null name with extension, otherwise 
*						just the name w.o. extension
*			pext	 - null pointer or pointer to extension variable (OUT)
* GLOBALS:
* REATURNS: number of parts created: 
*						1 just name, 
*						2: path and name or name and extension
*						3: all three parts
* REMARKS:	path and *pext (when not nullptr) may be empty, name always 
*			has a value
*			for directoy names: the ending '/' is stripped of the name
*--------------------------------------------------------------------------*/
int SeparateFileNamePath(QString fullName, QString &path, QString& name, QString *pext)
{
	fullName = QDir::cleanPath(fullName);	// '/' s and . and .. resolved but not absolute path

	int pos = fullName.lastIndexOf('/');
	if (pos == fullName.length() - 1)	// ending with '/' it is a directory, we need to separate parent
	{
		fullName = fullName.left(pos);	// cut ending '/'
		pos = fullName.lastIndexOf('/');
	}

	int res;
	if (pos < 0)
	{
		path = "";
		name = fullName;
		res = 1;
	}
	else
	{
		path = fullName.left(pos + 1);	// include ending '/'
		name = fullName.mid(pos + 1);
		res = 2;
	}
	if (pext)
	{
		pos = name.indexOf('.');	// full extension including the dot, e.g. .tar.gz
		*pext = pos < 0 ? "" : name.mid(pos);
		if (pos >= 0)
			name = name.left(pos); // w.o. extension
		if (pos > 0)	// then name + extension
			++res;
	}
	return res;
}
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
bool IsImageFile(const QString &name, QFileInfo *fi)
{
	if (fi && fi->isDir())
		return false;
	int pos = name.lastIndexOf('.');
	QString s = name.mid(pos).toLower();
	return (s == ".jpg") || (s == ".jpeg") || (s == ".gif") || (s == ".png");
}


/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString ToUTF8(QString string)
{
	if (string.isEmpty())
		return string;
	if (!ValidUtf8String(string, string.length()))
	{
		string = string.toUtf8();
//		QByteArray byteArray = string.toUtf8();
//		string.clear(); char ch;
//		string = byteArray;
		//for (int i = 0; i < byteArray.size(); ++i)
		//{
		//	ch = byteArray[i];
		//	string += ch;
		//}
		//QByteArray byteArray = string.toUtf8();
		//QTextCodec::ConverterState state;
		//QTextCodec *codec = QTextCodec::codecForName("UTF-8");
		//const QString text = codec->toUnicode(byteArray.constData(), byteArray.size(), &state);
		//if (state.invalidChars > 0) {
		//	qDebug() << "Not a valid UTF-8 sequence.";
		//return text;
		//}
	}
	return string;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QStringList ToUTF8(QStringList & sl)
{
	for (int i = 0; i < sl.size(); ++i)
		sl[i] = ToUTF8(sl[i]);
	return sl;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QImage LoadImage(QString path, int maxwidth, int maxheight, bool doNotEnlarge)
{
	QImage img;
	if(!img.load(path))
		return QImage();
	int w = img.width(),
		h = img.height();
	if (!w || !h)
		return QImage();

	double aspect = (double)w/ (double)h;

	if ((w > maxwidth) || (!doNotEnlarge && aspect > 1))
	{
			w = maxwidth;
			h = w / aspect;
	}
	if ((h > maxheight) || (!doNotEnlarge && aspect < 1))
	{
		h = maxheight;
		w = aspect * h;
	}

	if(h != img.height())
		return img.scaledToWidth(w, Qt::SmoothTransformation);
	
	return img;
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QPixmap LoadPixmap(QString path, int maxwidth, int maxheight, bool doNotEnlarge)
{
	return QPixmap::fromImage(LoadImage(path, maxwidth, maxheight,doNotEnlarge));
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
ImageConverter::ImageConverter(int maxwidth, int maxheight, bool doNotEnlarge) :
	maxwidth(maxwidth), maxheight(maxheight), dontEnlarge(doNotEnlarge)
{

}

double ImageConverter::GetSizes(QImageReader &imgReader)
{
	if (!imgReader.canRead())
		return 0;

	QSize osize = imgReader.size();

	width = owidth = osize.width();
	height = oheight = osize.height();
	if (!owidth || !oheight)
		return 1.0;

	aspect = (double)owidth / (double)oheight;

	if ((owidth > maxwidth) || (!dontEnlarge && aspect >= 1))
	{
		width = maxwidth;
		height = width / aspect;
	}
	if ((height > maxheight) || (!dontEnlarge && aspect <= 1))
	{
		height = maxheight;
		width = aspect * height;
	}
	if (width != owidth)	// if either width or height needed changing both changed
	{
		osize = QSize(width, height);
		imgReader.setScaledSize(osize);
	}
	return aspect;
}

/*============================================================================
* TASK:		resize and watermark images 
* EXPECTS:	 
*			 dest - path of destination image
*			 ovr - overwrite image if it exists
*			 pwm - pointer to watermark structure
*			parameters maxwidth, maxheight,dontEnlarge are set
* RETURNS: aspect ratio or 0 for load errors and -1.0 if destination exists and 
*			it is not allowed to overwrite it
* GLOBALS: 
* REMARKS:	path of source image must be set into imgReader before calling
*--------------------------------------------------------------------------*/
double ImageConverter::Process(QImageReader &imgReader, QString dest, bool ovr, WaterMark *pwm)
{
	if (!ovr)	// file MUST exists (checked before coming here)  && QFile::exists(dest))
		return -1.0;

	if ((aspect = GetSizes(imgReader)) == 0)
		return 0;

	static QImage img;
	imgReader.read(&img);

	_pImg = &img;
	if (pwm)
		_AddWatermark(*pwm);

	QImageWriter imageWriter(dest);
	imageWriter.setQuality(imgReader.quality());
	imageWriter.setFormat(imgReader.format());
	if (!imageWriter.write(img))
	{
		QMessageBox(QMessageBox::Warning, QMainWindow::tr("falconG - Warning"),
			imageWriter.errorString(), QMessageBox::Ok).exec();
	}
	return aspect;
}

/*============================================================================
* TASK:		add a watermark to the image '_pimg' points to
* EXPECTS: 	wm is a filled in watermark structure with valid mark image
*			_pimg [oints to existing image
*			width, height, etc are set up
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void ImageConverter::_AddWatermark(WaterMark & wm)
{
	if (!wm.mark)		// no watermark image
		return;

	int markWidth = wm.mark->width(),
		markHeight = wm.mark->height();
	// combine the text with the image

	int x0=0, y0=0;		// origin on image for watermark
	int imgWidth = _pImg->width(), imgHeight = _pImg->height();

	switch ((wm.origin & 0xF0) >> 4)
	{
		case 0: x0 = wm.marginX; break;
		case 1: x0 = (imgWidth - markWidth) / 2; break;
		case 2: x0 = imgWidth - markWidth - wm.marginX; break;
	}
	if (x0 < 0)
		x0 = 0;
	switch ((wm.origin & 0xF))
	{
		case 0: y0 = wm.marginY; break;
		case 1: y0 = (imgHeight - markHeight) / 2; break;
		case 2: y0 = imgHeight - markHeight - wm.marginY; break;
	}
	if (y0 < 0)
		y0 = 0;

   // draw the watermark
	QPainter painter(_pImg);
	if (imgWidth < markWidth)
	{
		QRect	srect(0,0, markWidth, markHeight), 
				drect(x0,y0, imgWidth, markHeight);	// source and destination

		painter.drawImage(drect, *wm.mark, srect);
	}
	else
		painter.drawImage(QPoint(x0, y0), *wm.mark);

}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString TimeToHMSStr(time_t t)
{
	QString s; QChar O('0');
	int sec = t % 60, m = (t / 60) % 60, h = t / 3600;
	if (h)
		s = QString("%1:%2:%3").arg(h, 0, 10, O).arg(m, 2, 10, O).arg(sec, 2, 10, O);
	else if (m)
		s = QString("%1:%2").arg(m, 2, 10, O).arg(sec, 2, 10, O);
	else
		s = QString("%1 s").arg(sec, 0, 10, O);
	return s;
}
