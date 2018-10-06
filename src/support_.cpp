#include "support.h"
//#include "sys/stat.h"
#include "io.h"
#include <QtDebug>

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FileReader::FileReader(const QString s, int flags) : _flags(flags)
{
	//	QTextCodec *codec = QTextCodec::codecForName("ISO 8859-2");
	_ok = (_if = fopen(s.toStdString().c_str(), "rb")) != nullptr;
	if (_ok)
	{
		_fileSize = QFile(s).size();
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
			_bsize = std::fread(_bytes, 1, _bsize, _if);
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
}


/*============================================================================
* TASK:	reads next non empty non comment line until EOF on the stream
* EXPECTS: bool GetCommentLines - comment in line is not thrown away
* GLOBALS:
* REMARKS:	- in binary mode (default: when 'string' is not in _flags)
*				_line is always UTF-8 encoded
*			- when _flags contain 'string' line is converted only if
*				needUtf8 is specified
*			- sets _ok to false at EOF
*--------------------------------------------------------------------------*/
void FileReader::_readLine()
{
	_line.clear();
	if (!_ok)
		return;

	int pos;
	do
	{
		NextLine();

		if (_flags == allLines)							// no trimming, no exclusions
			break;

		pos = _line.indexOf('#');						// comment?
		if( (_flags & commentLines) == 0 && pos >= 0)	// if not allowed
			_line = _line.left(pos);					// delete it

		if (_flags & ltrim)
		{
			for (pos = 0; pos < _line.length() && _line[pos].isSpace(); ++pos)
				;
			if (pos)
				_line = _line.mid(pos);
		}
		else if (_flags & rtrim)
		{
			for (pos = _line.length()-1; pos >= 0 && _line[pos].isSpace(); --pos)
				;
			if (pos != _line.length()-1)
				_line = _line.mid(pos);
		}
		else if (_flags & trim)
			_line = _line.trimmed();

		if ((_flags & emptyLines))						// then all lines qualify
			break;

	} while (_line.isEmpty() && _ok);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString FileReader::ReadLine(int flags)
{
	if (!_ok)
		return QString();

	int saveFlags = normal;
	if (flags != normal)
	{
		saveFlags = _flags;
		_flags = flags;
	}

	_readLine();		
	if (flags != normal)
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
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QString FileReader::NextLine()
{
	_readBinaryLine();
	if (_ok)
		++_readLineCount;
	return _line;
}

//*****************************************************

/*============================================================================
* TASK:		renames file as backup (add a ~) and rename tmp file to original name
* EXPECTS:	name - name of existing file to be renamed
*			tmpName - name of yet temporary file to be renamed to 'name'
* GLOBALS:	none
* REMARKS:
*--------------------------------------------------------------------------*/
bool BackupAndRename(QString name, QString tmpName, QWidget *parent)
{
	QFile ft(name), f(tmpName);
	ft.remove(name + "~");
	if ((ft.exists() && !ft.rename(name + "~")) || !f.rename(name))
	{
		QString st = QMainWindow::tr("falconG error"),
			sx = QMainWindow::tr("Can't rename \n'%1'\n to \n'%1~'\nNew file remain named as \n'%2'").arg(name).arg(tmpName);
		QMessageBox(QMessageBox::Warning, st, sx, QMessageBox::Close, parent).exec();
		return false;
	}

	return true;
}

/* --------------------------------- helper functions -----------------------*/
/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int SeparateFileNamePath(QString & fullName, QString &path, QString& name)
{
	fullName = QDir::cleanPath(fullName);	// '/' s and . and .. resolved but not absolute path

	int pos = fullName.lastIndexOf('/');
	if (pos == fullName.length() - 1)	// ending with '/' it is a directory, we need to separate parent
	{
		fullName = fullName.left(pos);	// cut ending '/'
		pos = fullName.lastIndexOf('/');
	}
	if (pos < 0)
	{
		path = "";
		name = fullName;
		return 1;
	}
	path = fullName.left(pos + 1);	// include ending '/'
	name = fullName.mid(pos + 1);
	return 2;
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
QString ToUTF8(QString &string)
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

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
double ImageConverter::Process(QString src, QString dest)
{
	QImage img;
	if (!img.load(src))
		return 0;
	width = owidth = img.width(),
	height = oheight = img.height();
	if (!owidth || !oheight)
		return 1.0;

	aspect = (double)owidth / (double)oheight;

	if ( (owidth > maxwidth) || (!dontEnlarge && aspect >= 1) )
	{
		width = maxwidth;
		height = width	 / aspect;
	}
	if ( (height > maxheight) || (!dontEnlarge && aspect <= 1) )
	{
		height = maxheight;
		width = aspect * height;
	}

	if(width != owidth)
		img = img.scaledToHeight(height, Qt::SmoothTransformation);
	
	img.save(dest);

	return aspect;
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
