#pragma once

#include <QFlags>
#include <QString>
#include <QFile>

#include "support.h"

// **************** FileReader class ****************
//******************** text file reader ************
// file header line(s)
//   reads lines and drops comments and empty lines
class FileReader
{
public:		//	       0		     1				2			         4            8		      16              24=16+8 	        32					   64
	QFlags<FrfFlags> flags;
private:
	QFile _f;
	bool _ok;
	QString _line;
	int _flags = 0;
	int _readLineCount = 0;
	// for binary read (default)
	const qint64 _BUFFER_SIZE = 4096;
	char* _bytes = nullptr;		// array to read binary data into
	qint64 _fileSize = 0, _bytesRead = 0;
	int _bsize = 0,			// size of binary data read (at most _BUFFER_SIZE;)
		_bpos = 0;			// buffer position

	void _TrimLeft();
	void _TrimRight();
	void _Trim();
	void _readBinaryLine(); // and convert it to UTF8
	void _readLine();		// reads next line using _flags until EOF on the stream
	void _DiscardComment();	// part of line starting at a '#' character 
	// and the white spaces before that are discarded
public:
	explicit FileReader(const QString s, QFlags<FrfFlags> flags);
	FileReader(const QString s, int flags = frfTrim) : FileReader(s, QFlags<FrfFlags>(flags)) {}
	~FileReader() { delete[] _bytes; }
	void SetFlags(int flg) { _flags = flg; }
	bool Ok() const { return _ok; }
	QString ReadLine(int flags = frfNormal);	// normal: use internal flags, other use this
	QStringList ReadAndSplitLine(QChar sep);
	QString NextLine(bool doNotDiscardComment = false);	// reads and returns even empty or comment lines!

	inline QString l() const { return _line; }	 // last read line
	int ReadCount() const { return _readLineCount; }
};

