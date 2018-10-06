#pragma once
#include <QApplication>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMainWindow>
#include <QFileInfo>
#include <QTextCodec>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <time.h>
// testing with STL
#include <stdio.h>

const QString versionStr = "# falconG Gallery Structure file 1.0";

//******************** text file reader ************
// file header line(s)
//   reads lines and drops comments and empty lines
class FileReader
{
public:		//	   0		 1			2			4 			6=2+4		  8		  16		 24=16+8 	   32
	enum Flags { normal, string,  emptyLines, commentLines= 4, allLines=6, ltrim =8, rtrim = 16, trim = 24, needUtf8 = 32 };
private:
	std::FILE *_if;  // test with STL
	bool _ok;
	QString _line;
	int _flags = 0;
	int _readLineCount = 0;
							// for binary read (default)
	const qint64 _BUFFER_SIZE = 4096;
	char *_bytes = nullptr;		// array to read binary data into
	qint64 _fileSize = 0, _bytesRead = 0;
	int _bsize = 0,			// size of binary data read (at most _BUFFER_SIZE;)
		_bpos = 0;			// buffer position

//	QTextCodec *codec;

	void _readBinaryLine(); // and convert it to UTF8
	void _readLine();		// reads next line using _flags until EOF on the stream
public:
	FileReader(const QString s, int flags = trim);
	~FileReader() { delete[] _bytes; }
	void SetFlags(int flg) { _flags = flg; }
	bool Ok() const { return _ok; }
	QString ReadLine(int flags = normal);	// normal: use internal flags, other use this
	QStringList ReadAndSplitLine(QChar sep);
	QString NextLine();	// reads and returns even empty or comment lines!
	inline QString l() const { return _line; }	 // last read line
	int ReadCount() const { return _readLineCount; }
};

//*****************************************																			//---------------------
class UsageCount
{
	int _cnt = 0;
public:
	int operator++() { ++_cnt; return _cnt; }
	int operator--() { if (_cnt) --_cnt; return _cnt; }
	operator int() { return _cnt; }
};

//*****************************************																			//---------------------
struct ImageConverter
{
	QString name;
	int owidth, oheight;			// original width and height
	int width, height;				// converted image size
	int maxwidth, maxheight;		// maximum allowed converted size
	bool dontEnlarge = true;
	double aspect = 0;				// width/height

	ImageConverter(int maxwidth, int maxheight, bool doNotEnlarge = true);

	double Process(QString src, QString dest);	// retuns aspect ratio (0: no src image)
};

QString TimeToHMSStr(time_t t);
bool BackupAndRename(QString name, QString tmpName, QWidget *parent = Q_NULLPTR);
int SeparateFileNamePath(QString & fullName, QString &path, QString& name);
bool IsImageFile(const QString &name, QFileInfo *fi = nullptr);
QString ToUTF8(QString &string);
QStringList ToUTF8(QStringList &string);

template<typename T> bool ValidUtf8String(const T& string, int len)
{
	int c, i, ix, n, j;
	for (i = 0, ix = len; i < ix; i++)
	{
		c = (unsigned char)( QChar(string.at(i)) .unicode());
		//if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
		if (0x00 <= c && c <= 0x7f) n = 0; // 0bbbbbbb
		else if ((c & 0xE0) == 0xC0) n = 1; // 110bbbbb
		else if (c == 0xed && i<(ix - 1) && ((unsigned char)(QChar(string.at(i + 1)).unicode()) & 0xa0) == 0xa0) return false; //U+d800 to U+dfff
		else if ((c & 0xF0) == 0xE0) n = 2; // 1110bbbb
		else if ((c & 0xF8) == 0xF0) n = 3; // 11110bbb
											//else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
											//else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
		else return false;
		for (j = 0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
			if ((++i == ix) || ( ( ((unsigned char)QChar(string.at(i)).unicode()) & 0xC0) != 0x80))
				return false;
		}
	}
	return true;

}

QImage LoadImage(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

QPixmap LoadPixmap(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);