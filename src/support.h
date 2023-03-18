#pragma once
#include <QModelIndex>
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
#include <QTreeView>
//#include <QSet>
#include <time.h>

#include "enums.h"
using namespace Enums;

const QString versionStr = "# falconG Gallery Structure file ";

// versions: 1.0 - text ID not saved, collision saved using an '*' followed by the collision number
//           1.1 - full text ID is saved after and '*' Calculated if missing
//           1.2 - full text ID is saved after and '*' Calculated if missing # set for src and dest, 'C' for albumms !! for images

constexpr int majorStructVersion = 1,		// V 1.2.3 version string
			  minorStructVersion = 2,
			  subStructVersion   = 4;	

enum FileTypeImageVideo {ftUnknown, ftImage, ftVideo };

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
	FileReader(const QString s, int flags = frfTrim) : FileReader(s, QFlags<FrfFlags>(flags)) {}
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
	UsageCount() {}
	UsageCount(int cnt) : _cnt(cnt) {}		// cnt MUST be positive
	int operator++() { ++_cnt; return _cnt; }
	int operator--() { if (_cnt) --_cnt; return _cnt; }
	int operator+=(int cnt) { _cnt += cnt; return _cnt; }
	int operator-=(int cnt) { _cnt -= cnt; if (_cnt < 0) _cnt = 0;  return _cnt; }
	UsageCount &operator=(const UsageCount u) { if (&u != this) _cnt = u._cnt;  return *this; }
	UsageCount &operator=(int cnt) { if (cnt < 0) cnt = 0;  _cnt = cnt; return *this; }
	operator int() const { return _cnt; }
};

//*****************************************
class WaterMark
{
public:
	enum POSITION : int {
		LEFT = 0
		, HCENTER = 16
		, RIGHT = 32
		, TOP = 0
		, VCENTER = 1
		, BOTTOM = 2
	};
	using POS = int;

protected:
	bool _enabled = true;	// otherwise do not create image
	bool _changed = false;	// any changes
	bool _regenImagesWhenChanged = false;
private:
	//using POS = int;

	QString _text;
	int _origin = LEFT+TOP;	// 0,1,2		- index of vertical	position (top,center,bottom)
						// 0,0x10,0x20	- index of horizontal position  (left, center, right)

	QImage *_pmark = nullptr;				// watermark text as an image
	int _markWidth = 0, _markHeight = 0;	// image width and height

	int _marginX = 0, // >= 0, measured from nearest horizontal image edge in pixels
		_marginY = 0; // >= 0, measured from nearest vert. image edge

	unsigned _colorWOpacity = 0x00ffffff,		// AARRGGBB
			 _background = 0;			 
	bool	_useBackground = false;	

	bool _shadowOn = false;
	int _shadowHoriz, _shadowVert;
	unsigned _shadowBlur;
	unsigned _shadowColor = unsigned(-1); // color (rgb) -1: not used

	QFont _font;
public:

	QImage *SetupMark();
	WaterMark& operator=(const WaterMark& other);
	WaterMark& operator=(const WaterMark&& other);
	bool operator!=(const WaterMark& wm) const;

	QImage* PMarkImage() const { return _pmark; }
	QString Text() const { return _text; }
	int Origin() const { return _origin; }

	bool WChanged() const { return _changed && _regenImagesWhenChanged; }

	int Width() const  { return _markWidth; }
	int Height() const { return _markHeight; }
	int MarginX() const { return _marginX; }
	int MarginY() const { return _marginX; }
	QFont Font() const { return _font; }
	bool HasDropShadow(int* xoffset = nullptr, int* yoffset = nullptr, QColor* color = nullptr) const 
	{ 
		if (xoffset) *xoffset = _shadowHoriz;
		if (xoffset) *yoffset = _shadowVert;
		if (color) *color = _shadowColor == unsigned(-1) ? QColor(63, 63, 63, 180) : QColor(_shadowColor);

		return _shadowOn;
	}
	void GetMarkDimensions();	// into _markWidth and _markHeight using current font
	QString PositionToStyle(int width, int height, double ratio, POS newPosition = -1) const;

	QString ColorToCss() const;
	QColor BorderColor() const;
	unsigned Color() const;	// with opacity
	unsigned ShadowColor() const { return _shadowColor; };	// with opacity
	unsigned Background() const { return _background; }
	double Opacity(bool percent) const;		// 0..255 (!percent) or 0..100 (percent)

	void ClearChanges() { _changed = false; }
	void SetRegenerationMode(bool regen) { _regenImagesWhenChanged = regen; }
	// setters	: each regenerates watermark image and saves it into 'res/'
#define SET_WM_VALUE(a,b)\
			if(a != (b))	\
			{  \
				_changed = true;	\
				a = (b); \
			}
	void SetPositioning(int pos) { SET_WM_VALUE(_origin,pos) }
	void SetBackground(unsigned bck) { SET_WM_VALUE(_background, bck) }
	void SetColorWithOpacity(unsigned colorWOpacity);
	void SetColorWithOpacity(QString sColorWOpacity);
	void SetFont(QFont& qfont);
	void SetMarginX(int mx) { SET_WM_VALUE(_marginX, mx) }
	void SetMarginY(int my) { SET_WM_VALUE(_marginY, my) }
	void SetText(QString  qs);
	void SetOpacity(int val, bool percent); // val is in percent (0..100) or not(0..255)?
	void SetShadowColor(unsigned color) { SET_WM_VALUE(_shadowColor,color) }
	void SetShadowOn(bool on) { SET_WM_VALUE(_shadowOn,on) }
	void SetShadowBlur(unsigned blur) { SET_WM_VALUE(_shadowBlur,blur) }
#undef SET_WM_VALUE
};

//*****************************************																			
struct ImageReader : public QImageReader
{
	QSize thumbSize = { 0, 0 },	// these must be set before processing thumbnails
		  imgSize;
	QImage img;					// read scaled image into this
	bool isReady = false;
	bool read() 
	{ 
		return isReady = QImageReader::read(&img); // && !img.isNull();
	}
	bool canRead() { return (isReady ? true : QImageReader::canRead()); }
	ImageReader(QIODevice *device, bool dontResize = false, const QByteArray &format = QByteArray()) : QImageReader(device, format) 
	{
		setAutoTransform(true);  // auto rotate, flip and mirror during read() when portrait orientation is set in EXIF
	}
	ImageReader(const QString &fileName, bool dontResize = false, const QByteArray &format = QByteArray()) : QImageReader()
	{
		setAutoTransform(true);  // auto rotate, flip and mirror during read() when portrait orientation is set in EXIF
		setFileName(fileName);	 // creates and opens a read only QFile
		setFormat(format);		 // e.g. "jpg" (same as "JPG"), empty: cycle through supported formats until one found
	}
};
//*****************************************
// read an image from disk or resource, rotate it on read if needed, 
//  resize it to w and h keeping aspect ratio
//  add a square icon overlay at given postion from the right
struct MarkedIcon
{
	QString name;			// full path name of image for which we want an icon
	QPixmap pxmp;			// square pixmap contains image with a 'margin' wide border
	bool isFolder = false;			// different border
	bool isFolderThumb = false;		// if yes: mark the image
	bool dontResize = false;		// - " -
	bool exists = false;			// - " -
		// these ar used for each thumbnail
		// QPixmaps can only be initialized after the GUI initilize (QT quirk)
		// so we need pointers here
	static QPixmap *folderThumbMark;	// if folder thumbnail mark with this	
	static QPixmap *noImageMark;	// image does not exist
	static QPixmap *noresizeMark;	// do not resize image
	static int thumbSize;			// named image is inside a (size x size) area this keeping aspect ratio
	static int borderWidth;		
	static bool initted;

	MarkedIcon()
	{
		if (!initted)
			Init();
	}

	MarkedIcon(const MarkedIcon& other)
	{
		isFolder = other.isFolder;
		isFolderThumb = other.isFolderThumb;
		exists = other.exists;
		name = other.name;
		pxmp = other.pxmp;
	}

	~MarkedIcon()
	{

	}
	void SetAsFolderThumb(bool setth)
	{
		isFolderThumb = setth;
	}
	void SetNoResize(bool noresize) 
	{ 
		dontResize = noresize;  
	}
	static void Init()
	{
		if (initted)
			return;

		folderThumbMark = new QPixmap(":/icons/Resources/folderIcon.png");
		noImageMark		= new QPixmap(":/icons/Resources/noImageMark.png");
		noresizeMark	= new QPixmap(":/icons/Resources/noresizeMark.png");
		initted = true;
	}
	static void SetMaximumSizes(int size, int margin)
	{
		thumbSize = size; 
		borderWidth = margin;
	}

	bool Read(QString name, bool isFolder);	// to pxmp, transforms rotated image on read, sets 'exists'
	QIcon ToIcon();
};
//QImage ReadAndMarkImage(QString name, int w, int h, bool exists, QString icon, int pos);

//*****************************************
struct ImageConverter
{			  // if 'dontEnlarge' flag is set only shrinks image, but 
			  //	thumbnails may be enlarged
			  // dontResize may be used  for some images
	QString name;
	int flags;
	// TODO: bool keepAspectRatio = true;	// otherwise CROP image
	double aspect = 0;				// width/height: same for thumbnail unless square thumbnails required (TODO)
									// 0: not calculated

	ImageConverter(int flags = dontEnlarge) : flags(flags) { }

//	double CalcSizes(ImageReader &reader);	 // set original and new sizes
	int Process(ImageReader &reader, QString dest, QString thumb, WaterMark *pwm=nullptr);	// retuns aspect ratio (0: no src image)

	QString ErrorText() const { return _qsErrorMsg; }
private:
	QImage *_pImg = nullptr;
	QString _qsErrorMsg;

//	double _CalcSizes(bool thumb);			// using oSize, newSize, maxSize for image, newSize, thumbSize, maxThumbSize for thumbnail _
	void _AddWatermark(WaterMark &wm);		// to pImg using data from config
};

//*****************************************
int SeparateFileNamePath(QString fullName, QString &path, QString& name, QString *pext = nullptr);
QString CutSourceRoot(QString path);
QString TimeToHMSStr(time_t t);
QString BackupAndRename(QString name, QString tmpName, bool keepBackup = false);
FileTypeImageVideo IsImageOrVideoFile(const QString &name, QFileInfo *fi = nullptr);
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
void InformationMessage(bool WarningAndNotInfo, QString title, QString text, int show = 0, QString checkboxtext = QString(), QWidget* parent = nullptr);
// QuestionDialog returns a single bit at position 'show', which, when 1 may be QMessageBox::Yes or QMessageBox::Save only!
int QuestionDialog(QString title, QString text, int show = 0, QWidget* parent = nullptr, QString checkboxtext = QString(), QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No);

int DeleteOrRemoveConfirmationDialog(IntList &list, QWidget* parent = nullptr); // returns 0: delete, 1: from disk too, 2: canceled

const char* StringToUtf8CString(QString qs);

QString EncodeLF(QString str);
QString DecodeLF(QString str, int toHtmlOrJs = 0, bool alsoQuotes = false);		// string \\n to character \n or string<br>\n (html) or <br> (Javascript)


QImage LoadImage(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

QPixmap LoadPixmap(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

bool CopyOneFile(QString src, QString dest, bool overWrite = true);
bool CreateDir(QString sdir); // recursive creation of directories, returns 0: error, 1: created, -1: cancelled
bool RemoveFolderRecursively(QString name, bool tryToTrash=true);			// won't ask
bool RemoveDir(QString name, bool ask = false, bool tryToTrash=true);

char* StringToCString(QString string);