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
#include <QImageReader>
#include <QPainter>
//#include <QSet>
#include <time.h>

const QString versionStr = "# falconG Gallery Structure file ";

// versions: 1.0 - text ID not saved, collision saved using an '*' followed by the collision number
//           1.1 - full text ID is saved after and '*' Calculated if missing
//           1.2 - full text ID is saved after and '*' Calculated if missing # set for src and dest, 'C' for albumms !! for images

const int majorStructVersion = 1,
		  minorStructVersion = 2;


//******************** text file reader ************
// file header line(s)
//   reads lines and drops comments and empty lines
class FileReader
{
public:		//	       0		     1				2			         4            8		      16              24=16+8 	        32					   64
	enum FrfFlags { frfNormal, frfAllLines, frfEmptyLines = 2, frfCommentLines= 4, frfLtrim = 8, frfRtrim = 8, frfTrim = 12, frfNoWhiteSpaceLines = 32, frfNeedUtf8 = 64 };
	QFlags<FrfFlags> flags;
private:
	QFile _f;
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

	void _TrimLeft();
	void _TrimRight();
	void _Trim();
	void _readBinaryLine(); // and convert it to UTF8
	void _readLine();		// reads next line using _flags until EOF on the stream
	void _DiscardComment();	// part of line starting at a '#' character 
							// and the white spaces before that are discarded
public:
	explicit FileReader(const QString s, QFlags<FrfFlags> flags);
	FileReader(const QString s, int flags = frfTrim) : FileReader(s, QFlags<FileReader::FrfFlags>(flags)) {}
	~FileReader() { delete[] _bytes; }
	void SetFlags(int flg) { _flags = flg; }
	bool Ok() const { return _ok; }
	QString ReadLine(int flags = frfNormal);	// normal: use internal flags, other use this
	QStringList ReadAndSplitLine(QChar sep);
	QString NextLine(bool doNotDiscardComment=false);	// reads and returns even empty or comment lines!
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
	int operator+=(int cnt) { _cnt += cnt; return _cnt; }
	int operator-=(int cnt) { _cnt -= cnt; if (_cnt < 0) _cnt = 0;  return _cnt; }
	UsageCount &operator=(const UsageCount u) { if (&u != this) _cnt = u._cnt;  return *this; }
	UsageCount &operator=(int cnt) { if (cnt < 0) cnt = 0;  _cnt = cnt; return *this; }
	operator int() const { return _cnt; }
};

//*****************************************																			//---------------------
struct WaterMark
{
	QString text;
	int origin = 0+0;	// 0,1,2: index of vertical 0,0x10,0x20 - horizontal
	int marginX = 0, // measured from nearest horizontal image edge 
		marginY = 0; // measured from nearest vert. image edge
	int color = 0,		// rgb
		background = -1, // rgb -1: not used
		shadowColor = -1,// color (rgb) -1: not used
		opacity = 0;	// in percent
	QFont font;
	QImage *mark = nullptr;		// watermark text
	void SetFont(QFont & qfont) { font = qfont; SetupMark(); }
	void SetText(QString  qs) { text = qs; SetupMark(); }
	void SetColor(int ccolor) { color = ccolor; SetupMark();}
	void SetColor(QString scolor) { color = scolor.toInt(nullptr, 16); SetupMark(); }

	bool operator!=(const WaterMark &wm)
	{
		return (text != wm.text)
			|| (origin != wm.origin)
			|| (marginX != wm.marginX)
			|| (marginY != wm.marginY)
			|| (color != wm.color)
			|| (background != wm.background)
			|| (shadowColor != wm.shadowColor)
			|| (opacity != wm.opacity)
			|| (font != wm.font);
	}

	void SetupMark()
	{
		QFontMetrics fm(font);

		int markWidth = fm.width(text),
			markHeight = fm.height();
		delete mark;
		mark = new QImage(markWidth, markHeight, QImage::Format_ARGB32);
		mark->fill(qRgba(0, 0, 0, 0) );	// transparent image
		QPainter painter(mark);
		painter.setFont(font);
		QColor c(qRed(color), qGreen(color), qBlue(color), opacity / 100.0 * 255);
		QPen pen(c);
		painter.setPen(pen);

		painter.drawText(0, 0, markWidth, markHeight, Qt::AlignCenter, text);
	}
};

//*****************************************																			
struct ImageReader : public QImageReader
{
	QImage img;			// read scaled image into this
	bool isReady = false;
	bool read() 
	{ 
		return isReady = QImageReader::read(&img) && !img.isNull(); 
	}
	bool canRead() { return (isReady ? true : QImageReader::canRead()); }
	ImageReader(QIODevice *device, bool dontResize = false, const QByteArray &format = QByteArray()) : QImageReader(device, format) 
	{
		setAutoTransform(true);  // auto rotate	when portrait orientation is set in EXIF
	}
	ImageReader(const QString &fileName, bool dontResize = false, const QByteArray &format = QByteArray()) : QImageReader()
	{
		setAutoTransform(true);  // auto rotate when portrait orientation is set in EXIF
		setFileName(fileName);
		setFormat(format);
	}
};

//*****************************************
struct ImageConverter
{			  // only shrink image thumbnails may be enlarged(?)
			  // dontResize may be used  for some images
	enum IcFlags :int { prImage = 1, prThumb = 2, dontEnlarge = 4, dontResize = 8 };
	QString name;
	QSize oSize,					// original width and height for image
		newSize, 					// calculated new dimensions for image
		thumbSize;					// calculated thumbnail sizes

	QRect maxSize;					// maximum allowed converted dimensions for image
									// in x and y, and thumbnail size in width and height
	int flags;
	// TODO: bool keepAspectRatio = true;	// otherwise CROP image
	double aspect = 0;				// width/height: same for thumbnail unless square thumbnails required (TODO)
									// 0: not calculated

	ImageConverter(QRect maxSize, int flags = dontEnlarge);		// create converter

	double CalcSizes(ImageReader &reader);	 // set original and new sizes
	double Process(ImageReader &reader, QString dest, QString thumb, bool ovr, WaterMark *pwm=nullptr);	// retuns aspect ratio (0: no src image)

	QString ErrorText() const { ; }
private:
	QImage *_pImg = nullptr;
	QString _qsErrorMsg;

	double _CalcSizes(bool thumb);			// using oSize, newSize, maxSize for image, newSize, thumbSize, maxThumbSize for thumbnail _
	void _AddWatermark(WaterMark &wm);		// to pImg using data from config
};

QString TimeToHMSStr(time_t t);
QString BackupAndRename(QString name, QString tmpName, bool keepBackup = false);
int SeparateFileNamePath(QString fullName, QString &path, QString& name, QString *pext = nullptr);
bool IsImageFile(const QString &name, QFileInfo *fi = nullptr);
QString ToUTF8(QString string);
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
//*****************************************																			//---------------------
void ShowWarning(QString qs, QWidget *parent = nullptr);

QString EncodeLF(QString str);
QString DecodeLF(QString str, bool toHtml = false);		// string \\n to character \n or string<br>\n


QImage LoadImage(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

QPixmap LoadPixmap(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);
