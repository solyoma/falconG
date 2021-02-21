#include "config.h"
#include "support.h"
#include <QtWidgets>
#include <QtDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QTextStream>

//*****************************************																			//---------------------
void ShowWarning(QString qs, QWidget *parent)
{
	QFile f(config.dsApplication.ToString() + "falconG.warnings");
	f.open(QIODevice::WriteOnly | QIODevice::Append);
	if (f.isOpen())
	{
		QTextStream ofs(&f);
		ofs << QDateTime::currentDateTime().toString() << " - " << qs << "\n\n";
		f.close();
	}

	if (!config.bNoMoreWarnings)
	{
		QMessageBox msgBox(parent);
		msgBox.addButton(QMessageBox::Ok);
		msgBox.setWindowTitle(QMainWindow::tr("falconG - Warning"));
		QCheckBox *pchk = new QCheckBox(QMainWindow::tr("No more warnings for this album"));
		msgBox.setCheckBox(pchk);
		msgBox.setText(qs);
		msgBox.exec();
		config.bNoMoreWarnings = msgBox.checkBox()->isChecked();
	}
}


/*=============================================================
* TASK:	replaces LF character in string 's'
*			with the string \\n
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
QString EncodeLF(QString s)
{
	int pos;
	while ((pos = s.indexOf("\n")) >= 0)
		s = s.left(pos) + "\\n" +
		s.mid(pos + 1);
	return s;
}

/*=============================================================
* TASK:   Replaces encded \\n strings with character \n
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
QString DecodeLF(QString s, bool toHtml)
{
	int pos;
	QString LF = toHtml ? "<br>\n" : "\n";
	while ((pos = s.indexOf("\\n")) >= 0)
		s = s.left(pos) + LF + s.mid(pos + 2);
	return s;
}
#if 0
/*============================================================================
  * TASK:	Returns a string in which all '\n' sequences are replaced with "<br>
  * EXPECTS:	txt: string
  * RETURNS: possibly modified string
  * GLOBALS: none
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString __TextWithBR(QString &txt)
{
	if (txt.isEmpty())
		return txt;
	QString txtBR;
	int pos, posp = 0;
	while ((pos = txt.indexOf("\\n", posp)) >= 0)
	{
		txt += txt.mid(posp, (pos - posp)) + "<br>";
		posp = pos + 2;
	}
	txt += txt.mid(posp, -1);
	return txt;
}
#endif


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
  * TASK:	left trim _line
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: no right trim in Qt 5
 *--------------------------------------------------------------------------*/
void FileReader::_TrimLeft()
{
	int pos;
	for (pos = 0; pos < _line.length() && _line[pos].isSpace(); ++pos)
		;
	_line = _line.mid(pos);
}

/*============================================================================
  * TASK:	right trim _line
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: no right trim in Qt 5
 *--------------------------------------------------------------------------*/
void FileReader::_TrimRight()
{
	int pos;
	for (pos = _line.length()-1; pos >= 0 && _line[pos].isSpace(); --pos)
		;
	_line = _line.left(pos+1);
}

/*============================================================================
  * TASK:	trim _line
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void FileReader::_Trim()
{
	_line = _line.trimmed();
}

/*============================================================================
* TASK:	reads a *line delimiter* (LF or CR+LF) delimited UTF-8 encoded 
*		character string into '_line' from '_f' until EOF on the stream.
*		If the last character before the line delimiter is a backslash the line
*		does not ends at the delimiter, but it is a continuation line.
*		Continuation lines are concatenated _line
*		
* EXPECTS:  last read line may be overwritten
* GLOBALS:  _bpos - position in buffer to start of next line
*           _bsize - size of data in buffer
*			_bytesRead - from file
* RETURNS: nothing (_line and _ok are set)
* REMARKS:	- concatenates input lines ending in a backslash character
*			  with the next line
*			- result in _line is always Utf8, even if they were in the local
*				encoding
*			- at end of file or on read error _ok is set to false
*			- static '_lastCharRead' contains the last character read into 
*				buffer. If this character is an LF then the buffer ends with EOL 
*				otherwise the line continues in the next bufferfull of characters
*			- the character combination 'backslash + EOL' character is discarded
*--------------------------------------------------------------------------*/
void FileReader::_readBinaryLine()
{
	_line.clear();
	if (!_ok)
		return;

	static char __lastCharRead = 0;		   // can be any character BS or CR
	static bool __backslashAtEnd = false;  // partial line ended with a BS
										   // next partial line may start with CR or LF

	const char CR = '\r', LF = '\n', BS = '\\';
	bool EOL = false;	

	QByteArray ba;		// holds characters to be converted to string
						// never contains line continuation string BS+LF or BS+CR+LF
	char ch;
	do
	{
		if (!_bsize)		// must read new data into buffer
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

			// possible last characters in prev. buffer:
			// .  . CR				-  line will end with an LF	here  (EOL == false)
			// . CR LF	or .  . LF	-  line already ended (EOL == true)

			// EOL == false and the buffer ended with
			// . \  CR |			   (__backslashAtEnd == false)
			// . \  LF +-------------  line will continue here (__backslashAtEnd == false)
			// \ CR LF | 			   (__backslashAtEnd == false)
			// .  .  \ |			   (__backslashAtEnd == true)
			// .  .  . |			   (__backslashAtEnd == false)

			_bpos = 0;

			if (__backslashAtEnd)	// check for continuation characters
			{
				if (_bytes[0] == LF)
					_bpos = 1;
				else if (_bytes[0] == CR)
					_bpos = 2;			  // MUST be LF (old Macs < OS X does not work)
				else
					ba += BS;			// other characters: keep the \'
				EOL = false;
			}
			else if (__lastCharRead == CR && _bytes[0] == LF) // line ends here
			{
				_bpos = 1;	// in this case  current line starts at position 1
				EOL = true;
			}

			__lastCharRead = _bytes[_bsize - 1];	// store last character read
			__backslashAtEnd = false;

			if (EOL)
				break;
		}

		// not EOL
		int epos = _bpos;

		// search for line end position up to the last byte
		// for continuation line checks (BS+CR+LF or BS+LF inside - no problem)
		while (!EOL && _bpos < _bsize)
		{					// get end of line or one line section
			while (epos < _bsize && _bytes[epos] != CR && _bytes[epos] != LF)
				++epos;
			// if CR or LF found we must check 2 or 3 characters starting from 
			//			'epos'  
			//	'.' means  any character except BS,CR.LF

			// when epos < _bsize -1, then CR or LF at epos
			// ('v' character denotes epos)
			//	      v			EOL   string ends at     _bpos after
			//----------------------------------------------------------
			//   .   CR  LF		yes	   epos					epos+2
			//   .   LF  .		yes	   epos					epos+1
			//   BS  CR  LF     no	   epos-1				epos+2
			//   BS  LF  .		no	   epos-1				epos+1

			// when at buffer end ('|' character denotes end of buffer)
			//	#	buffer | next buffer
			//	1		  .| .			=>concatenate with next buffer
			//	2	   . LF| .           => EOL found
			//	3	  BS LF| .           =>concatenate with next buffer
			//	4	     CR|LF			=>EOL found, skip firstc character in next buffer
			//	5	  BS CR|LF			=>concatenate with next buffer and skip first character there
			//  6        BS|.			=>concatenate with next buffer 

			//  #						    1	         2          3		   4          5           6
			// end of string segment at  _bsize-1	 _bsize-1	 _bsize-2	_bsize-1   _bsize-2	   _bsize-1
			// epos at this point        _bsize-1	 _bsize-1	 _bsize-1	_bsize-1   _bsize-1    _bsize-1
			// epos after handling        _bsize	  _bsize	  _bsize	 _bsize		_bsize     _bsize
			// EOL -"-				        no      	yes			no		   yes		  no        no

			ch = _bytes[epos];
			if (epos < _bsize - 1)	// inside the buffer  => epos <= _bsize-2 and CR or LF found at _bytes[epos]
			{
				if (epos > 0 && _bytes[epos - 1] == BS) // BS+LF or BS+CR+LF
					_bytes[epos - 1] = 0;		// end of string
				else // no backslash before CR or LF: EOL 
				{
					_bytes[epos] = 0;
					EOL = true;
				}
				++epos;	// either after LF or to LF after CR
				if (ch == CR)
					++epos;			// after the LF
				ba += (_bytes + _bpos);		// concatenate with buffer 
				_bpos = epos;
			}
			else		  // end of buffer (epos == _bsize-1) -> to last character (__lastCharRead)
			{			  //  special handling
				if (__lastCharRead == BS)
				{		  // all bytes from buffer are read and next buffer must be concatenated with this
					__backslashAtEnd = true;	// anything may come in next buffer
					_bytes[epos] = 0;
				}
				else if (__lastCharRead == CR || __lastCharRead == LF) // then line end might have been found
				{
					if (epos > 0 && _bytes[epos - 1] == BS)		 // no it isn't: in this case no EOL
					{
						_bytes[epos - 1] = 0;
						if (__lastCharRead == CR)
							__backslashAtEnd = true;
					}
					else // depends on the last character
					{
						_bytes[epos] = 0;
						if(__lastCharRead == LF)	// else LF is in the next bufferfull of data
							EOL = true;
					}
				}
				else	// no CR or LF at end of buffer
					_bytes[_bsize] = 0;		// overflow area :)

				ba += (_bytes + _bpos);		// concatenate with buffer 
				_bpos = _bsize;	
			}
		}	// end of loop for concataneted lines, partial or complete line already in 'ba'

			// _bpos points to start of line section to be processed next
			// epos  points to 
			//					_bsize-1-th position when no CR or LF found
			//                  after the LF (max value is '_bsize' - outside the buffer
			//					when CR was found in the loop but it can be _bsize-1 when LF
			//                  was found in the loop
			// EOL - when LF or CR+LF is found without a preceeding BS

		if (_bpos == _bsize)		// read buffer again
			_bpos = _bsize = 0;		
	// line section is already in 'ba'
	} while (_ok && !EOL);
	
	if (_ok)			   // EOL: line in ba is OK, move it into _line
	{
		if (!ValidUtf8String(ba, ba.length()))
			_line = QString::fromLocal8Bit(ba, ba.length());
		else
			_line = ba;			// add to data


		if (_ok || ( (flags & frfEmptyLines) == 0 && !_line.isEmpty()) )
			++_readLineCount;
	}
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

	do
	{
		NextLine();		// read in line from file possibly convert it to UTF-8
						// and set '_ok'

		if (_flags & frfAllLines)			// no trimming, no exclusions
			break;

		if (_flags ^ frfCommentLines)		// if comments not allowed
			_DiscardComment();

		if (_flags & (frfNoWhiteSpaceLines | frfTrim))
			_Trim();
		else
		{
			if (_flags & frfLtrim)
				_TrimLeft();
			if (_flags & frfRtrim)
				_TrimRight();
		}
		if ((_flags & frfEmptyLines))		// then _line is always OK
			break;
	} while (_line.isEmpty() && _ok);
}

/*=============================================================
 * TASK:   part of line starting at a '#' character
 *			and the white spaces before that are discarded
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FileReader::_DiscardComment()
{
	int pos = _line.indexOf('#');
	while (pos >= 0)					// and there is one
	{
		if (pos > 0 && (_line[pos - 1] == '\\' || _line[pos - 1] == '&'))		// \#  or &# is NOT start of a comment
		{
			pos = _line.indexOf('#', pos+1);
			continue;
		}
		while (pos >= 0 && _line[pos].isSpace())
			--pos;
		_line = _line.left(pos);	// delete includinga and after the '#'
		pos = -1;
	}


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
* TASK:		Read line even when it is empty or a comment
* EXPECTS:
* GLOBALS:
* REMARKS: 	- comments are not discarded from line
*			- lines are always right trimmed
*			- lines of whitespace(s) only will be cleared
*--------------------------------------------------------------------------*/
QString FileReader::NextLine(bool doNotDiscardComment)
{
	_flags |= frfNoWhiteSpaceLines | frfCommentLines;
	_readBinaryLine();
	if(!doNotDiscardComment)
		_DiscardComment();
	_TrimRight();
	
	return _line;
}

//*****************************************************

/*============================================================================
* TASK:		create backup of file 'name' by renaming it to 'name~'
*			and rename the new file 'tmpName' to original 'name'
* EXPECTS:	name		- name of existing file to be renamed and backed up
*			tmpName		- name of yet temporary file to be renamed to 'name'
*			keepPreviousBackup: 
*						- true: keep existing backup file and simply delete 'name'
*							before renaming 'tmpFile'. 
*						  false: an existing 
*							backup file will be deleted
* GLOBALS:	none
* RETURNS:  empty QString when backup and rename was successfull, error message 
*			on error
* REMARKS:
*--------------------------------------------------------------------------*/
QString BackupAndRename(QString name, QString tmpName, bool keepPreviousBackup)
{
	QFile ft(name), ftmp(tmpName);
	QString qsErr;
	bool brt=true, brn;
	bool bx = ft.exists();
	if (keepPreviousBackup)
	{

		brt = bx ? ft.remove() : true;
		brn = ftmp.rename(name);
		if( (bx &&  !brt) ||  !brn)
		{
			qsErr = QMainWindow::tr("Either can't delete \n'%1'\n"
								 " or can't rename '%2' to '%1'\n"
								 "Modified file remains named as \n'%2'").arg(name).arg(tmpName);
		}
	}
	else
	{	
		if (bx)
		{
			QFile::remove(name + "~");
			brt = ft.rename(name + "~");
		}

		brn = ftmp.rename(name);
//		if ((ft.exists() && !ft.rename(name + "~")) || !f.rename(name))
		if( (bx && !brt) || !brn)
		{
			qsErr = QMainWindow::tr("Can't create backup file\n'%1~'\n"
								 "Temporary file\n'%2'\nwas not renamed").arg(name).arg(tmpName);
		}
	}
	//if (bErr)
	//{
	//	QMessageBox(QMessageBox::Warning, st, sx, QMessageBox::Close, parent).exec();
	//	return false;
	//}
	return qsErr;
}

/* --------------------------------- helper functions -----------------------*/
/*============================================================================
* TASK:		 separates an absolute or relative path file or directory path 
*			 into its components. For directories (fullName ends with '/'
*			 sets the name to be the directory name and the path its parent
*			 directory
* EXPECTS:	fullName -  (IN) file or directory path 
*							Directory names must end with '/'
*			path     -  (OUT) file or parent directory path,
*							empty or ends with '/'
*			name	 -  (OUT) if pext is null name with extension, 
*						otherwise just the name w.o. extension
*			pext	 -  (IN) null pointer or pointer to extension variable
*						*pext used when not nullptr(OUT)
* GLOBALS:
* RETURNS: number of parts created: 
*						1: just name, 
*						2: path and name or name and extension
*						3: all three parts
* REMARKS:	- path and *pext (when pext is not nullptr) may be empty,
*				name always has a value
*			- for directoy names: the ending '/' is stripped of the name
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
* TASK:		check if file with name and info is an image OR a video file
* EXPECTS:	name - file name
*			fi	 - pointer to FileInfo structure
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FileTypeImageVideo IsImageOrVideoFile(const QString &name, QFileInfo *fi)
{
	if (fi && fi->isDir())
		return ftUnknown;
	int pos = name.lastIndexOf('.');
	QString s = name.mid(pos).toLower();
	return (s == ".jpg") || (s == ".jpeg") || (s == ".gif") || (s == ".png") ? ftImage : (s == "*.mp4") || (s == "*.ogg") || (s == "*.webm") ? ftVideo : ftUnknown;
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
		string = string.toUtf8();

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
* TASK:		loadn a rescaled (enlarged or shrinked) image into memory
* EXPECTS:	path:	image path
*			maxwidth, maxheight: maximum dimensions
*			doNotEnlarge: only shrink or leave it
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
//			thumbSize.setWidth(maxSize.width());
//			thumbSize.setHeight(maxSize.width() / aspect);
//		}
//		if (aspect <= 1)
//		{
//			if ((newSize.height() > maxSize.y()) || ((newSize.height() < maxSize.y()) && (flags & dontEnlarge) == 0))
//			{
//				newSize.setHeight(maxSize.y());
//				newSize.setWidth(aspect * maxSize.y());
//			}
//			// thumbs always resized even when it means enlargement
//			thumbSize.setHeight(maxSize.height());
//			thumbSize.setWidth(aspect * maxSize.height());
//		}
//	}
//	imgReader.setScaledSize(newSize);	// newSize used in read, thumbSize used in write
//	return aspect;
//}

/*============================================================================
* TASK:		resize images and add watermark
* EXPECTS:	 
*			 dest - path of destination image
*			thumb - process a thumbnail?
*			 ovr - overwrite image if it exists
*			 pwm - pointer to watermark structure
*			parameters maxwidth, maxheight,dontEnlarge are set
* RETURNS:	- normal exit: aspect ratio 
*			- load errors: 0.0
*			- if destination exists and it is not allowed to overwrite it: -1.0
*			- file write error: -2.0
* GLOBALS: 
* REMARKS:	- path of source image must be set into imgReader before calling
*			- for thumbnails if the image was already loaded into imgReader
*				then scale image during save, else save the image as it is
*--------------------------------------------------------------------------*/
double ImageConverter::Process(ImageReader &imgReader, QString dest, QString thumb, WaterMark *pwm)
{
	if (!config.bOvrImages)	// file MUST exist (checked before coming here)  && QFile::exists(dest))
		return -1.0;

	QImageIOHandler::Transformations tr = imgReader.transformation();
	QSize newSize = imgReader.imgSize;	// if tr then it may already transposed sizes (from camera, not from PS/LR)
	if (tr & (QImageIOHandler::TransformationRotate90 | QImageIOHandler::TransformationMirrorAndRotate90))
		newSize.transpose();
	imgReader.setScaledSize(newSize);	// newSize used in read, thumbSize used in write

	if (!imgReader.isReady)			// not read yet
	{								
		imgReader.read();			// scaled and possibly rotated image

		_pImg = &imgReader.img;		// must set here to be used in _AddWatermark
		if (pwm)
			_AddWatermark(*pwm);
	}
		
	QString qsErr;
	// write scaled image into 'dest'
	if(flags & prImage)
	{
		QImageWriter imageWriter(dest);
		imageWriter.setQuality(imgReader.quality());
		imageWriter.setFormat(imgReader.format());

		if (!imageWriter.write(imgReader.img))
			qsErr = imageWriter.errorString() + "\n'" + dest + "'\n";
	}
	// write thumbnail image into 'thumb'
	// thumbnail image dimensions are never transposed 
	// and all thumbnail images have the same height
	// which is set in 'config'
	if(flags & prThumb)
	{
		if (imgReader.thumbSize.width() <= 0 || imgReader.thumbSize.height() <= 0)
		{
			ShowWarning("Invalid sizes for thumbnail \n'"+thumb + "'\n");
			return aspect;
		}
		//	re-scale image for thumbnail
		imgReader.img = imgReader.img.scaled(imgReader.thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QImageWriter imageWriter(thumb);
		imageWriter.setQuality(imgReader.quality());
		imageWriter.setFormat(imgReader.format());
		if (!imageWriter.write(imgReader.img))
			qsErr += "'"+thumb + "'\n" +imageWriter.errorString();
	}
	if (!qsErr.isEmpty())
		ShowWarning(qsErr);
	return aspect;
}

/*============================================================================
* TASK:		add a watermark to the image '_pimg' points to
* EXPECTS: 	wm is a filled in watermark structure with valid mark image
*			_pImg points to existing image
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


bool CopyOneFile(QString src, QString dest, bool overWrite)
{
	if (QFile::exists(dest))
		if (overWrite)
			QFile::remove(dest);
	return QFile::copy(src, dest);	// true:copy OK, false: copy error
}


/*==========================================================================
* TASK:		ask if to cancel directory creation
* EXPECTS: s - name of directory to display in message box
* RETURNS: true: ok to create, false: cancel create
* REMARKS: - prepares _root album and recursively processes all levels
*--------------------------------------------------------------------------*/
static bool __CancelCreate(QString s)
{
	return  QMessageBox(QMessageBox::Question, QMainWindow::tr("falconG"),
						QMainWindow::tr("Directory '%1' does not exist.\n\nCreate?").arg(s),
						QMessageBox::Yes | QMessageBox::Cancel).exec() == QMessageBox::Cancel;
}


/*==========================================================================
* TASK:		Creates directory and conditionally asks for confirmation
* EXPECTS: sdir - directory to create
*			aks	- if directory does not exist then ask permission to proceed?
* RETURNS:  true: directory either exists or created successfully
* REMARKS: 	 ask set to true if dir. existed and false when it did not
*--------------------------------------------------------------------------*/
bool CreateDir(QString sdir, bool& ask) // only create if needed
{										// ask - if does not exist ask what to do
	QDir folder;
	if (folder.exists(sdir))
		return ask = true;

	bool b = ask;
	ask = false;

	if (b && __CancelCreate(sdir))
		return false;
	return folder.mkdir(sdir);
}

/*========================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - https://nachtimwald.com/2010/06/08/qt-remove-directory-and-its-contents/
 *-------------------------------------------------------*/
static bool __RemoveFolder(QString name)
{
	bool result = true;
	QDir dir(name);

	if (dir.exists(name)) 
	{
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = __RemoveFolder(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = dir.rmdir(name);
	}

	return result;
}


/*========================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - NO NEED: QSir.removeRecursively() does this
 *				except asking for it
 *-------------------------------------------------------*/
bool RemoveDir(QString name, bool ask)
{
	if (ask)
	{
		if (QMessageBox::question(nullptr, QObject::tr("falconG"), QString(QObject::tr("Really remove %1 and all of its content?")).arg(name)) != QMessageBox::Yes)
			return false;
	}
	return __RemoveFolder(name);
}
