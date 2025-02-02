#include <QObject>
#include <QtWidgets>
#include <QtDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QTextStream>
#include "support.h"
#include "config.h"
#include "falconG.h"

static void  WarningToFile(QString qs)
{
	QFile f(PROGRAM_CONFIG::homePath + "falconG.warnings");
	f.open(QIODevice::WriteOnly | QIODevice::Append);
	if (f.isOpen())
	{
		QTextStream ofs(&f);
		ofs << QDateTime::currentDateTime().toString() << " - " << qs << "\n";
		f.close();
	}
}

static QSplashScreen* splashScreen = nullptr;

void ShowSplashScreen(bool addMessage)
{
#ifndef DEBUG
	if (!splashScreen)
	{
		splashScreen = new QSplashScreen(QPixmap(":/icons/Resources/falconG-splash.png"));
		splashScreen->setWindowFlag(Qt::WindowStaysOnTopHint);
		splashScreen->show();
	}
	if (addMessage)
	{
		Qt::Alignment topRight = Qt::AlignHCenter | Qt::AlignBottom;
		splashScreen->showMessage(QObject::tr("falconG  - Setting up...\n\n\n"), topRight, Qt::black);
	}

	QApplication::processEvents();
#endif
}

void CloseSplashScreen()
{
	if (splashScreen)
	{
		splashScreen->finish(frmMain);
		delete splashScreen;
		splashScreen = nullptr;
	}
}

//*****************************************
void ShowWarning(QString qs, QWidget *parent)
{
	if (splashScreen)
		CloseSplashScreen();

	if (qs.isEmpty())
		qs = QMainWindow::tr("Unspecified message");

	WarningToFile(qs);

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

void InformationMessage(bool WarningAndNotInfo, QString title, QString text, int show, QString checkboxtext, QWidget* parent)
{
	if (WarningAndNotInfo)
		WarningToFile(text);

	if (config.doNotShowTheseDialogs.v & (1 << show))
		return;

	QMessageBox info(parent);
	info.setText(title);
	info.setIcon(WarningAndNotInfo ? QMessageBox::Warning : QMessageBox::Information);
	info.setInformativeText(text);
	info.setStandardButtons(QMessageBox::Ok);
	QCheckBox* checkBox = nullptr;
	if (!checkboxtext.isEmpty())
	{
		checkBox = new QCheckBox(checkboxtext);
		info.setCheckBox(checkBox);
	}

	int res = info.exec();

	if (checkBox && info.checkBox()->isChecked())
		config.doNotShowTheseDialogs.v |= (1 << show);
	config.defaultAnswers[show] = res;
}

/*=============================================================
 * TASK:	A question dialog with a checkbox to hide it in the
 *			future
 * PARAMS:	title, text parent, buttons: same as for QMessageBox
 *			show: integer between 1 and Common::dboMax, ordinal of
 *				dialog. See 'DialogBitsOrder' in support.h
 *				If 0 it behaves as a simple 
 *					QMessageBox::question would
 *			checkboxtext: if 'show' is not 0 and  adds a checkbox
 *				to the dialog with this text and stores
 *				its state when any button clicked
 * GLOBALS:	
 * RETURNS:	the same value as QMessageBox::exec() returns
 * REMARKS: sets the flags to not show this dialog again, but
 *			never clears them
 *------------------------------------------------------------*/
int QuestionDialog(QString title, QString text, Common::DialogBitsOrder show, QWidget* parent, QString checkboxtext, QMessageBox::StandardButtons buttons)
{
	if (show > Common::DialogBitsOrder::dboNone && config.doNotShowTheseDialogs.v & (1 << (int) show))
		return config.defaultAnswers[show];

	QMessageBox question(parent);
	question.setText(title);
	question.setIcon(QMessageBox::Question);
	question.setInformativeText(text);
	question.setStandardButtons(buttons);
	QCheckBox* checkBox = nullptr;
	if (show >= 0 && !checkboxtext.isEmpty())
	{
		checkBox = new QCheckBox(checkboxtext);
		question.setCheckBox(checkBox);
	}

	int res = question.exec();
	if (show > 0 && (res == QMessageBox::Yes || res == QMessageBox::Save) )
		config.defaultAnswers[show] = res;

	if (checkBox && question.checkBox()->isChecked())
		config.doNotShowTheseDialogs.v |= (1 << show);

	return res;
}

int DeleteOrRemoveConfirmationDialog(IntList &list, QWidget* parent)
{
	QString plurali = QObject::tr("images"), plurala = QObject::tr("albums");   // plural for image and album. May differ in other languages
	QString qs = QObject::tr("Do you want to delete selected %1 / %2 from disk \n(including items and sub-albums for album),\n or just to remove them from gallery?")
		.arg(list.size() > 1 ? plurali : QObject::tr("image"))
		.arg(list.size() > 1 ? plurala : QObject::tr("album"));
	QMessageBox msg;
	msg.setWindowTitle(QObject::tr("falconG - Delete Images and albums"));
	msg.setText(qs);
	msg.addButton(QObject::tr("Just remove"), QMessageBox::NoRole);               // 0
	msg.addButton(QObject::tr("From disk"), QMessageBox::YesRole);                // 1
	QPushButton *pbCancel = msg.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);                // 2
	msg.setDefaultButton(pbCancel);
	msg.setIcon(QMessageBox::Question);
	int res = msg.exec();

	if (res == 2)                   // Cancel
		return res;

	extern FalconG* frmMain;
	if (res == 1)    // delete from disk too
	{
		if (QMessageBox::question((QWidget*)frmMain, QObject::tr("falconG - Question"),
			QObject::tr("This will delete all selected images and folders from disk.\n"
				"This cannot be undone.\n\n"
				"Are you >>really<< sure you want to do this?")) != QMessageBox::Yes)
			return 2;
	}
	return res;
}

const char* StringToUtf8CString(QString qs)
{
	static QByteArray ba;
	ba = qs.toUtf8();
	return ba.data();
}


/*=============================================================
* TASK:	replaces LF character in string 's'	with the string \\n
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
QString EncodeText(const QString s)
{
	if (s.isEmpty() || s.indexOf('\n') < 0)
		return s;

	

	QString res = s;
	return res.replace("\n", "\\n");
	
	//
	//
	//
	//, st;
	//int n =0;
	//for (int i = 0; i < s.length(); ++i)
	//{
	//	switch (s[i].unicode())
	//	{
	//		case '\n': 
	//			res += "\\n"; 
	//			break;
	//			// '<' with '&lt;', '>' with '&gt;',
	//		//case '<': if(s.length() > i+1)
	//		//		  {
	//		//			if (s[i + 1].unicode() != ' ' && (s[i + 1].unicode() > '9' || s[i + 1].unicode() == '/') )
	//		//				res += "&lt;";
	//		//			else
	//		//				res += s[i];
	//		//		  }
	//		//		break;
	//		//case '>': if(i)
	//		//		  {
	//		//			if (s[i - 1].unicode() != ' ' && s[i - 1].unicode() > '9') 
	//		//				res += "&gt;";
	//		//			else
	//		//				res += s[i];
	//		//		  }
	//		//		break;
	//		case '&': st = s.mid(i, 8);
	//			n = 1;
	//			if (st.left(5) == "&amp;") n = 5; 
	//			else if (st.left(6) == "&lt;") n = 4; 
	//			else if (st.left(6) == "&gt;") n = 4; 
	//			else if (st.left(6) == "&nbsp;") n = 6; 
	//			else if (st.left(2) == "&#")					  // unicode constant
	//			{
	//				i += 2;
	//				while (i < s.length() && s[i].unicode() != ';')
	//					++n,++i;
	//				n += 2;
	//			}

	//			if(n == 1)
	//				res += "&amp;";
	//			else
	//			{
	//				res += st.left(n);
	//				i += --n;
	//			}
	//			break;
	//		default: res += s[i];
	//	}
	//}
	//return res;
}

/*=============================================================
* TASK:   Replaces encoded "\\n", "&amp;", "&lt;","&gt;" strings 
*			with one for the given purpose
* EXPECTS:	s		- string which may contain any of the above
*			purpose - dtPlain: plain text  
*							"\\n" => '\n', "&amp;" => '&',
*							"&lt;"=> '<', "&gt;"=> '>',
*					  dtHtml: HTML ->'\n', 
*							"\\n" => "<br>\n" 
*					  dtJS: JS -> '<br>'
*							"\\n" => "<br>" 
*							"&lt;"=> '<', "&gt;"=> '>',
* GLOBALS:
* RETURNS:	string after all replacements
* REMARKS:
*------------------------------------------------------------*/
QString DecodeTextFor(const QString s, DecodeTextTo purpose)
{
	if (s.isEmpty())
		return s;

	QString res = s;
	switch (purpose)
	{
		case dtPlain:
			res.replace("\\n", "\n");
			res.replace("&amp;", "&");
			res.replace("&lt;", "<");
			res.replace("&gt;", ">");
			break;
		case dtHtml:
			res.replace("\\n", "<br>\n");
			break;
		case dtJavaScript:
			if (res.indexOf('\n')>=0)
				res.replace("\n", "<br>");
			else if (res.indexOf("\\n") >= 0)
				res.replace("\\n", "<br>");
			res.replace("&lt;", "<");
			res.replace("&gt;", ">");
			res.replace('\'', 0x02);
			res.replace('\'', 0x03);
			res.replace('"' , 0x04);
			res.replace('\\', 0x05);
			break;
	}
	return res;

	//int pos = 0;
	//QString LF;
	//bool alsoQuotes = true;	// for Html and JavaScript when inside quotes
	//int quote = 0;
	//switch (purpose)
	//{
	//	default:
	//	case dtPlain:	LF = "\n"; alsoQuotes = false; break;
	//	case dtHtml:	LF = "<br>\n"; break;
	//	case dtJavaScript: LF = "<br>";break;
	//}
	//QString res;
	//for (; pos < s.length(); ++pos)
	//{
	//	if (s[pos] == QChar('\\') && pos < s.length() - 1 && s[pos + 1] == QChar('n'))
	//	{
	//		res += LF;
	//		++pos;
	//	}
	//	else if (s[pos].unicode() == '&')
	//	{
	//		if (s.mid(pos, 5) == "&amp;")
	//		{
	//			res += '&';
	//			pos += 4;
	//		}
	//		else if (s.mid(pos, 4) == "&lt;")
	//		{
	//			res += '<';
	//			pos += 3;
	//		}
	//		else if (s.mid(pos, 4) == "&gt;")
	//		{
	//			res += '>';
	//			pos += 3;
	//		}
	//		else
	//			res += s[pos];
	//	}
	//	else if (alsoQuotes && (s[pos].unicode() == '\'' || s[pos].unicode() == '\"') )
	//	{
	//		if (!quote)								// starting new quote?
	//			quote = s[pos].unicode();
	//		else 
	//		{
	//			if (quote == s[pos].unicode())		// same type -> ending quote
	//			{
	//				quote = 0;
	//				res += s[pos];
	//			}
	//			else								// other quote: 
	//				res += QChar('\\');
	//		}
	//	}
	//	else
	//		res += s[pos];
	//}

	//return res;
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
* TASK:	buffered read of a delimited UTF-8 encoded 
*		character string into '_line' from '_f' until EOF on the stream.
*		line delimiter is either LF or CR+LF. 
*		If the last character before the line delimiter is a backslash the
*		logical line does not ends at the delimiter, but continued in the
*		next physical line.	Continuation lines are concatenated into _line
*		with a single space separating the two parts.
*		
* EXPECTS:  
* GLOBALS:  _bytes	- input buffer it may contain more than one line
*           _bsize	- size of data in buffer '_bytes'
*			_bpos	- actual cursor position in buffer 
*			_bytesRead - count read from file
*			_line	- one line from _bytes or maybe from previous buffer
* RETURNS: nothing (_line and _ok are set)
* REMARKS:	- concatenates input lines ending in a backslash character
*			  with the next line using a single space between them
*			- result in _line is always Utf8, even if they were in the local
*				encoding
*			- at end of file or on read error _ok is set to false
*			- static '_lastCharRead' contains the last character read into 
*				buffer. If this character is an LF then the buffer ends with EOL 
*				otherwise the line continues in the next bufferfull of characters
*			- the character combination 'backslash + EOL' character is discarded
*				from _line
*--------------------------------------------------------------------------*/
void FileReader::_readBinaryLine()
{
	_line.clear();
	if (!_ok)
		return;

	static char __lastCharRead = 0;		   // can be any character including BS, LF or CR
	static bool __backslashAtEnd = false;  // partial line ended with a BS
										   // next partial line may start with CR or LF

	const char CR = '\r', LF = '\n', BS = '\\';
	bool EOL = false;	

	QByteArray ba;		// internal buffer: holds characters to be converted to string

	char ch = 0;
	// loop to read a complete line into 'ba'
	do
	{
		if (!_bsize)		// buffer is empty or all data in it were already used up
		{					// so wee need to read new data into '_bytes'
			_bpos = 0;	// new _bytes buffer read
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

			// EOL == false and the previous buffer ended with (| signals end of buffer)
			// . \  CR |			   (__backslashAtEnd == false)
			// . \  LF +-------------  line will continue here (__backslashAtEnd == false)
			// \ CR LF | 			   (__backslashAtEnd == false)
			// .  .  \ |			   (__backslashAtEnd == true)
			// .  .  . |			   (__backslashAtEnd == false)

			// modify _bpos if new buffer starts with an EOL character (CR or LF)
			if (__backslashAtEnd)	// of previous data in 'ba'. Check for continuation characters in this new data
			{
				if (_bytes[0] == LF)	// there is previous data ending with either CR (windows) or ordinary character (Macs,linux)
					_bpos = 1;
				else if (_bytes[0] == CR)  // MUST be a CR + LF combination (old Macs < OS X used CR as line end character, for them this does not work)
					_bpos = 2;			  
				else 
					ba += BS;			// other characters (e.g. '\\'|'n'): keep the \'
				EOL = false;
				// check if continuation line starts with spaces and previous part ended with it
				// and leave a single space here
				if (_bytes[_bpos] == ' ')
				{
					while (_bpos < _bsize && _bytes[_bpos] == ' ')
						++_bpos;
					if (ch != ' ')
						--_bpos;
				}
			}
			else if (__lastCharRead == CR && _bytes[0] == LF) // previous line ends here
			{
				_bpos = 1;	// in this case  next line starts at position 1
				EOL = true;
			}

			__lastCharRead = _bytes[_bsize - 1];	// store last character read into '_bytes'
			__backslashAtEnd = false;

			if (EOL)
				break;
		}

		// not EOL
		int epos = _bpos;	// actual check position

		// search for line end position up to the last byte
		// for continuation line checks (BS+CR+LF or BS+LF inside - no problem)
		while (!EOL && _bpos < _bsize)
		{		// get to end of line or end of one line section
				// or end of buffer
			while (epos < _bsize && _bytes[epos] != CR && _bytes[epos] != LF)
				++epos;

			// if CR or LF found we must check 2 or 3 characters starting from 
			//			'epos'  
			//	'|' character denotes end of buffer
			//	'.' means  any character except BS,CR.LF

			// when epos < _bsize -1, then CR or LF at epos
			// ('v' character denotes epos)
			//		epos
			//	      v			EOL   string ends at     _bpos after
			//----------------------------------------------------------
			//   .   CR  LF		yes	   epos					epos+2
			//   .   LF  .		yes	   epos					epos+1
			//   BS  CR  LF     no	   does not end			epos+2
			//   BS  LF  .		no	   does not end			epos+1
			//   .   |			no	   does not end			  0
			//   BS  |			no	   does not end			  0

			if (epos == _bsize)	// end of buffer and no CR or LF found
			{
				ch = _bytes[epos - 1];
				_bytes[epos - 1] = 0;
				ba += (_bytes + _bpos);
				if (ch == BS)
					__backslashAtEnd = true;
				else
					ba += ch;
				_bsize = 0;		// read new buffer of data into '_bytes'
				continue;
			}

			// when at buffer end ('epos'==_bsize-1) 
			//	#	_bytes | next buffer
			//	1		  .| .			=>concatenate with next buffer
			//	2	   . LF| .          => EOL found
			//	3	  BS LF| .          =>concatenate with next buffer
			//	4	     CR|LF			=>EOL found, skip first character in next buffer
			//	5	  BS CR|LF			=>concatenate with next buffer and skip first character there
			//  6        BS|.			=>BS character in line
			//  7        BS|LF			=>concatenate with next buffer and skip first character there
			//  8        BS|CR LF		=>concatenate with next buffer and skip first 2 characters there

			//  # in table above:		    1	         2          3		   4          5           6	           7	           8
			// end of string segment at  _bsize-1	 _bsize-1	 _bsize-2	_bsize-1   _bsize-2	   _bsize-1	    _bsize-1		_bsize-1
			// at epos at this point        any			LF			LF		   CR		  CR		  BS	     BS				   BS
			// epos after handling        _bsize	  _bsize	  _bsize	 _bsize		_bsize     _bsize	    _bsize			_bsize
			// EOL -"-				        no      	yes			no		   yes		  no        no		     no				   no

			ch = _bytes[epos];
			if (epos <= _bsize - 2)	// inside the buffer  and CR or LF found at _bytes[epos]
			{
				char chlast = epos > 1 ? _bytes[epos - 2] : ' ';	// if there was a BS this is the last character before it

				if (epos > 0 && _bytes[epos - 1] == BS) // BS+LF or BS+CR+LF
				{
					_bytes[epos - 1] = 0;		// end of string section
					__backslashAtEnd = true;	// temporary use
				}
				else // no backslash before CR or LF: EOL 
				{
					_bytes[epos] = 0;
					EOL = true;
				}
				++epos;			// next character after LF or CR 
				if (ch == CR)	// CR+LF pair
					++epos;		// after the LF this may set epos to be _bsize
				ba += (_bytes + _bpos);		// concatenate with buffer ending with a 0 byte
				if (!EOL && __backslashAtEnd)	// then drop more than one space
				{
					while (epos < _bsize && _bytes[epos] == ' ')
						++epos;
					if (chlast != ' ')
						--epos;
					__backslashAtEnd = false;	
				}
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
*			- lines ending in a backslash '\' are concatenated with text on next line
*				with only a single space separating them
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
*						- true: keep existing backup file delete any 'name.tmp'
*						        file, rename 'name' file to 'name.tmp' and then
*								rename 'tmpFile' to name. 
*						  false: an existing 
*							backup file will be deleted
* GLOBALS:	none
* RETURNS:  empty QString when backup and rename was successful, error message 
*			on error
* REMARKS:
*--------------------------------------------------------------------------*/
QString BackupAndRename(QString name, QString tmpName, bool keepPreviousBackup)
{
	QString qsTmp = tmpName + QString("~");
	QFile ft(name), ftmp(tmpName), ftmp1(qsTmp);
	QString qsErr;
	bool brt=true, brn;
	bool bx = ft.exists(),
		 bx1 = ftmp1.exists();
	if (keepPreviousBackup)
	{

		brt = bx1 ? ftmp1.remove() : true;
		brn = ft.rename(qsTmp);
		bool brn1 = ftmp.rename(name);
		if( (bx &&  !brt) ||  !brn ||!brn1)
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
			if (bx1)
				QFile::remove(qsTmp);
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

QString PrependSourcePathTo(const QString s)
{
	if (QDir::isAbsolutePath(s))
		return s;
	return config.dsSrc.ToString() + s;
}

QString CutSourceRootFrom(QString path)
{
	QString qs = config.dsSrc.ToString();
	int sl = qs.length();
	if (sl <= path.length() && path.left(sl) == qs)
		path = path.mid(sl);
	return path;
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
	return (s == ".jpg") || (s == ".jpeg") || (s == ".png") ? ftImage : (s == ".mp4") || (s == ".ogg") || (s == ".webm") ? ftVideo : ftUnknown;
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
	imgReader.setScaledSize(newSize);	// rescale to newSize when read, thumbSize used in write

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
	if(flags & prImage)
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
	if(flags & prThumb)
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
*		   - saves user seelction into static variable
*--------------------------------------------------------------------------*/
static bool __CancelCreate(QString s)
{
	if (config.doNotShowTheseDialogs.v & (int)dboAskCreateDir)
		return config.defaultAnswers[dboAskCreateDir] != QMessageBox::Yes;
			// can't use the one in falconG.cpp
	QMessageBox question;
	question.setText(QMainWindow::tr("falconG - Question"));
	question.setIcon(QMessageBox::Question);
	question.setInformativeText(QMainWindow::tr("Directory '%1' does not exist.\n\nCreate?").arg(s));
	question.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

	QCheckBox* checkBox = new QCheckBox(QMainWindow::tr("Don't ask again (use Options to re-enable)") );
	question.setCheckBox(checkBox);

	config.defaultAnswers[dboAskCreateDir] = question.exec();

	if (checkBox && question.checkBox()->isChecked())
		config.doNotShowTheseDialogs.v |= (int)dboAskCreateDir;

	return config.defaultAnswers[dboAskCreateDir] != QMessageBox::Yes;
}


/*==========================================================================
* TASK:		Recurvely creates directory and conditionally asks for confirmation
* EXPECTS:	sdir - directory to create
*			dirIndex - if not 0 appended to the name after a dash. Example: album-7
* RETURNS:  1: directory exists (or created successfully)
*			0: directory creation error
*			-1: cancelled
* REMARKS: 	 
*--------------------------------------------------------------------------*/
bool CreateDir(QString sdir, int dirIndex) // only create if needed
{										// ask - if does not exist ask what to do
	if (dirIndex)
		sdir += QString("-%1").arg(dirIndex);
	QDir folder;
	if (folder.exists(sdir))
		return true;

	if (__CancelCreate(sdir))
		return false;
	QStringList qsl = sdir.split('/');
	QString spath;
	bool res = true;
	for (auto &s : qsl)
	{
		spath += s;
		if (!s.isEmpty() && spath != "/")
		{
			if (!folder.exists(spath))
				res = folder.mkdir(spath);
			if (!res)
			{
				s = QMainWindow::tr("Can't create folder") + QString("'%1'").arg(spath);
				QMessageBox::warning(nullptr, QMainWindow::tr("falconG - Warning"), s);
				return false;
			}
			spath += "/";
		}
	}
	return true;
}

/*========================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - https://nachtimwald.com/2010/06/08/qt-remove-directory-and-its-contents/
 *-------------------------------------------------------*/
bool RemoveFolderRecursively(QString name, bool tryToTrash)
{
	bool result = false;
	QDir dir(name);

	if (dir.exists(name)) 
	{
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) 
		{
			if (info.isDir()) 
			{
				if (tryToTrash)
					result = QFile::moveToTrash(info.absoluteFilePath());
				if(!result)
					result = RemoveFolderRecursively(info.absoluteFilePath(), tryToTrash);
			}
			else 
			{
				if (tryToTrash)
					result = QFile::moveToTrash(info.absoluteFilePath());
				if(!result)
					result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) 
				return result;
			
		}
		if (tryToTrash)
			result = QFile::moveToTrash(name);
		if(!result)
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
bool RemoveDir(QString name, bool ask, bool tryToTrash)
{
	if (ask)
	{
		if (QMessageBox::question(nullptr, QMainWindow::tr("falconG"), QString(QMainWindow::tr("Really remove %1 and all of its content?")).arg(name)) != QMessageBox::Yes)
			return false;
	}
	return RemoveFolderRecursively(name, tryToTrash);
}

// ************* sturct Watermark ************

QString WaterMark::ColorToCss() const
{
	return QString("rgba(%1,%2,%3,%4)").arg((_colorWOpacity >> 16) & 0xFF).arg((_colorWOpacity >> 8) & 0xFF).arg(_colorWOpacity & 0xFF).arg(Opacity(false) / 255.0);
}
QColor WaterMark::BorderColor() const
{
	unsigned c = _colorWOpacity & 0xffffff;
	return (QColor(QString("#%1").arg(c, 6, 16, QChar(0))).value() < 170 ? "white" : "black");
}

/*=============================================================
 * TASK:	set positioning string for ".thumb::after"
 *			for the sample
 * EXPECTS:	width,height : for encllosing thumbnail
 *			ratio: thumbnail width / image width
 *			pos:	where to put it
 * GLOBALS: _marginX, _marginY
 * RETURNS:	2 line string for top and left set
 * REMARKS: ::after element would come after the main
 *			element, so both top and left will always be negative
 *------------------------------------------------------------*/
QString WaterMark::PositionToStyle(int width, int height, double ratio, POS pos) const// to set on .thumb::after
{
	if (pos < 0)
		pos = _origin;
	int left=-999999, top = -999999;	//impossible values
	switch (pos &0xF0)
	{
		case LEFT:		left = -width + _marginX; break;
		case HCENTER:	left = -(width+_markWidth)/2 + _marginX; break;
		case RIGHT:		left = -_markWidth - _marginX; break;
	}
	if (left < -width)
		left = -width;
	switch (pos & 0xF)
	{
		case TOP:		top =  -height + _marginY; break;
		case VCENTER:	top = -(height + _markHeight) / 2 + _marginY; break;
		case BOTTOM:	top = -_markHeight - _marginY; break;
	}
	if (top < -height)
		top = -height;

	QString qs = QString("top:%1px;\n\tleft:%2px;\n").arg(int(top*ratio)).arg(int(left*ratio));
	return qs;
}

unsigned WaterMark::Color() const 
{
	return _colorWOpacity; 
}
double WaterMark::Opacity(bool percent) const		// 0..255 (!percent) or 0..100 (percent)
{
	return ((_colorWOpacity >> 24) & 0xFF) * (percent ? 100.0 / 255.0 : 1.0);
}

	// setters
	// setters	: each regenerates watermark image and saves it into 'res/'
#define SET_WM_VALUE(a,b)\
			if(a != (b))	\
			{  \
				_changed = true;	\
				a = (b); \
			}
void WaterMark::SetFont(QFont& qfont) // use either SetText() or GetMarkDimension after font is modified
{
	SET_WM_VALUE(_font, qfont);
	_font.setStyleHint(QFont::AnyStyle, QFont::PreferAntialias);
	SetupMark();
}
void WaterMark::SetText(QString  qs)	// calls GetMarkDimensions
{
	SET_WM_VALUE(_text,qs);
	GetMarkDimensions();
	SetupMark();
}
void WaterMark::SetColorWithOpacity(unsigned c_colorWOpacity)
{
	SET_WM_VALUE(_colorWOpacity, c_colorWOpacity);
	SetupMark();
}
void WaterMark::SetColorWithOpacity(QString s_colorWOpacity)
{
	SET_WM_VALUE(_colorWOpacity, s_colorWOpacity.toUInt(nullptr, 16));
	SetupMark();
}
void WaterMark::SetOpacity(int val, bool percent) // val is in percent (0..100) or not(0..255)?
{
	if (percent)
		val *= 2.55;
	unsigned cwo = (((int)(val)) << 24) + (qRed(_colorWOpacity) << 16) + (qGreen(_colorWOpacity) << 8) + qBlue(_colorWOpacity);
	SET_WM_VALUE(_colorWOpacity, cwo);
	SetupMark();
}

bool WaterMark::operator!=(const WaterMark& wm) const
{
	return (_changed != wm._changed) 
		|| (_text != wm._text)
		|| (_origin != wm._origin)
		|| (_marginX != wm._marginX)
		|| (_marginY != wm._marginY)
		|| (_colorWOpacity != wm._colorWOpacity)
		|| (_background != wm._background)
		|| (_shadowColor != wm._shadowColor)
		|| (_font != wm._font);
}

void WaterMark::GetMarkDimensions()
{
	QFontMetrics fm(_font);
	_markWidth = fm.horizontalAdvance(_text);
	_markHeight = fm.height();
	if (_shadowOn)	// leave space for the shadow
	{
		_markWidth += _shadowHoriz;
		_markHeight += _shadowVert;
	}
}

/*=============================================================
 * TASK: generate watermark image for watermark text and 
 *			
 * EXPECTS:	font set, and correct text is also set,
 *			GetMarkDimensions() called at least once
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QImage* WaterMark::SetupMark()
{
	if (!_enabled)
		return nullptr;

	delete _pmark;
	_pmark = new QImage(_markWidth, _markHeight, QImage::Format_ARGB32);
	_pmark->fill(qRgba(0, 0, 0, 0));	// transparent image
	QPainter painter(_pmark);
	painter.setFont(_font);
	QColor c(qRed(_colorWOpacity), qGreen(_colorWOpacity), qBlue(_colorWOpacity), Opacity(false));
	// Debug 
	//QColor c(0xff,0,0,128);
	//QString n = c.rgba();
	QPen pen(c);
	painter.setPen(pen);

	int startX = 0, startY = 0;
	if (_shadowOn)
		startX = _shadowHoriz / 2, startY = _shadowVert / 2;
	painter.drawText(startX, startY, _markWidth, _markHeight, Qt::AlignCenter, _text);

	// DEBUG

	/*QFile fdbg("debug-watermark.txt");
	fdbg.open(QIODevice::WriteOnly);
	QTextStream odbg(&fdbg);
	odbg << "Font: " << font.family() << ", " << font.pointSize() << "pt, rgba:" << QString("#%1").arg(c.rgba(), 8, 16, QChar('0')) << ", penw:" << pen.width() << "\n";
	*/
	if (_shadowOn)
	{
		class MyShadow : public QGraphicsDropShadowEffect
		{
			public: 
				MyShadow(QObject* parent = nullptr) :QGraphicsDropShadowEffect(parent) {}
				void draw(QPainter* painter) override { drawSource(painter);  }
		}	shadow(nullptr);
		shadow.setXOffset(_shadowHoriz);
		shadow.setYOffset(_shadowVert);
		shadow.setBlurRadius(_shadowBlur);
		if(_shadowColor)
			shadow.setColor(_shadowColor);
		shadow.draw( &painter);
	}

	_pmark->save(PROGRAM_CONFIG::samplePath + "res/watermark.png");	// used on image
	return _pmark;
}

WaterMark& WaterMark::operator=(const WaterMark& other)
{
	_text = other._text;
	_origin = other._origin;

	delete _pmark;
	_pmark = nullptr;
	if (other._pmark)
		_pmark = new QImage(*other._pmark);

	_changed = other._changed;
	_markWidth = other._markWidth;
	_markHeight = other._markHeight;
	_colorWOpacity = other._colorWOpacity;
	_background = other._background;
	_useBackground = other._useBackground;
	_shadowOn = other._shadowOn;
	_shadowHoriz = other._shadowHoriz;
	_shadowVert = other._shadowVert;
	_shadowBlur = other._shadowBlur;
	_shadowColor = other._shadowColor;
	_font = other._font;
	return *this;
}

WaterMark& WaterMark::operator=(const WaterMark&& other)
{
	_text = other._text;
	_origin = other._origin;

	_pmark = other._pmark;

	_changed = other._changed;
	_markWidth = other._markWidth;
	_markHeight = other._markHeight;
	_colorWOpacity = other._colorWOpacity;
	_background = other._background;
	_useBackground = other._useBackground;
	_shadowOn = other._shadowOn;
	_shadowHoriz = other._shadowHoriz;
	_shadowVert = other._shadowVert;
	_shadowBlur = other._shadowBlur;
	_shadowColor = other._shadowColor;
	_font = other._font;
	return *this;
}

// =================================================================
QPixmap *MarkedIcon::folderThumbMark = nullptr;
QPixmap *MarkedIcon::noImageMark = nullptr;
QPixmap *MarkedIcon::noresizeMark = nullptr;
int MarkedIcon::thumbSize = THUMBNAIL_SIZE;		// named image is inside a (size x size) area this keeping aspect ratio
int MarkedIcon::borderWidth = thumbSize / THUMBNAIL_BORDER_FACTOR;				// in pixels portrait image: right and left, landscape image top and bottom
bool MarkedIcon::initted = false;				// images for icons read?


/*=============================================================
 * TASK:	reads an image into 'pxmp' member from file and
 *			shows it on a square pixmap with 'thumbSize' side
 *			on a background whose color depends on the type of
 *			the image (folder thumbnail or image thumbnail)
 * PARAMS:	name: file name to read image from
 *			is_folder: if this will be for a folder
 * GLOBALS:	static members are set
 * RETURNS:	if file read was successful
 * REMARKS: if read is unsuccessfull the pixmap still valid
 *------------------------------------------------------------*/
bool MarkedIcon::Read(QString fname, bool is_folder)
{
	name = fname;
	isFolder = is_folder;

	QImageReader reader(name);
	reader.setAutoTransform(true);
	QString colorname = (isFolder ? config.albumMatteColor : config.imageMatteColor).Name(true);
	QColor cbck = colorname;
	if (!cbck.isValid())
		return false;
	reader.setBackgroundColor(cbck);

	pxmp = QPixmap(thumbSize, thumbSize);
	pxmp.fill(cbck);

	exists = false;

	QSize osize = reader.size(),
		  dsize;
	if (!osize.isValid())
		return false;
		
	if (osize.width() >= osize.height())	// portrait
	{
		dsize.setWidth(thumbSize - 2*borderWidth);
		dsize.setHeight((double)(thumbSize - 2*borderWidth) / (double)(osize.width()) * osize.height());
	}
	else
	{
		dsize.setHeight(thumbSize - 2*borderWidth);
		dsize.setWidth((double)(thumbSize - 2*borderWidth) / (double)(osize.height()) * osize.width());
	}

	reader.setScaledSize(dsize);

	static QImage img;
	if(!reader.read(&img))		// maybe error display?
		return false;

	exists = true;

	QPainter painter(&pxmp);	// leave the border outside
	int xm =(pxmp.width() - dsize.width()) / 2, ym = (pxmp.height() - dsize.height()) / 2;
	painter.drawImage(xm, ym, img);

	return true;
}

/*=============================================================
 * TASK:	returns an icon with markers *folder Thumb an exists) on them
 * PARAMS:
 * GLOBALS:
 * RETURNS: an icon for image read. 
 * REMARKS: If no markers are to be set on this item returns just 
 *			the pixmap 'pxmp'
 *------------------------------------------------------------*/
QIcon MarkedIcon::ToIcon()
{
	if (exists && !isFolderThumb && !dontResize)	// no markers on image
		return QIcon(pxmp);

	QPixmap tmppxmp(thumbSize, thumbSize);
	QPainter painter(&tmppxmp);
	painter.drawPixmap(0,0, pxmp);		// image with border
	if (dontResize)
		painter.drawPixmap(borderWidth, borderWidth, *noresizeMark);
	if (isFolderThumb)
		painter.drawPixmap(thumbSize - folderThumbMark->width() - borderWidth, borderWidth, *folderThumbMark);
	if(!exists)
		painter.drawPixmap(thumbSize - 2*noImageMark->width() - borderWidth, borderWidth, *noImageMark);

	return QIcon(tmppxmp);
}

char* StringToCString(QString string)
{
	static QByteArray ba;
	ba = string.toLocal8Bit();
	return ba.data();
}

QString MakeRandomStringOfLength(int length)
{

	static constexpr const char array_str[] = "abcdefghijklmnopqrstuvwxyz_-+0123456789";
	static constexpr int array_size = sizeof(array_str);

	QString result;
	int idx = 0;
	for (int i = 0; i < length; ++i)
	{
		idx = QRandomGenerator::global()->bounded(array_size);
		QChar ch = array_str[idx];
		result.append(ch);
	}

	return result;
}