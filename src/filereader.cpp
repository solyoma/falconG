#include "filereader.h"  

// **************** FileReader class ****************

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
	for (pos = _line.length() - 1; pos >= 0 && _line[pos].isSpace(); --pos)
		;
	_line = _line.left(pos + 1);
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
						if (__lastCharRead == LF)	// else LF is in the next bufferfull of data
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


		if (_ok || ((flags & frfEmptyLines) == 0 && !_line.isEmpty()))
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
*				frfNormal - skip comment and empty lines and not trim it
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
			pos = _line.indexOf('#', pos + 1);
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
	if (!doNotDiscardComment)
		_DiscardComment();
	_TrimRight();

	return _line;
}
