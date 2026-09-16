#pragma once
#include <QModelIndex>
#include <QApplication>
#include <QFont>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMainWindow>
#include <QFileInfo>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	#include <QStringConverter>
#else
	#include <QTextCodec>
#endif
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QTreeView>
#include <QSplashScreen>
//#include <QSet>
#include <time.h>

#include "common.h"
using namespace Common;

const QString versionStr = "# falconG Gallery Structure file ";

// versions: 1.0 - text ID not saved, collision saved using an '*' followed by the collision number
//           1.1 - full text ID is saved after and '*' Calculated if missing
//           1.2 - full text ID is saved after and '*' Calculated if missing # set for src and dest, 'C' for albumms !! for images
//			 1.3 - source image paths have an ID and the file contains this ID instead of the path for the images and videos
//			 1.5 - album lines may contain a base album ID defining an alias to an existing album inside {}, before theclosing')'
//			 2.0 - can't read versions before 1.3 anymore use separate conversion program to update versions older than than 1.3

constexpr int majorProgramVersion = 2,		// V 2.0.1 version string
			  minorProgramVersion = 0,
			  subProgramVersion   = 1;

	// encoding and decoding text
enum DecodeTextTo { dtPlain, 			// from html or javascript to plain text (&amp;, &quot; &apos; &lg; &gt; <br>)
					dtDescription,		// similar to dtHTML, used in header for the description and title texts
					dtHtml,				// for ',",\n,\\n, <,>
					dtJavaScript 		// like HTML, but replaces with codes 2-5
				};

enum FileType : char {	ftUnknown = 0,
							ftImage = 'I',
							ftVideo = 'V',
							ftFolder= 'F',
							ftUnprocessedImage='i',
							ftnporocessedVideo = 'v' };
const char itemIsUnprocessedFlag = 0x20; // bit added to file type char to convert 'I' to 'i' and 'V' to 'v'

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
struct Album; // in albums.h

struct AlbumPointers
{
	IDVal_t srcAlbumId,		// source album id, == NOID if from external source, may be an alias
		    destAlbumId,	// destination album id, may be an alias IDs are stored as pointers may become
			srcBaseAlbumId,	// base album ID for source. May be equal to srcAlbumId
			destBaseAlbumId;// same for destination
	// invalid when albums are moved, so they must be re - fetched from the album map when needed
	Album* pDest,			// actual destination album. May be an alias album
		* pDestAlbum,	// either base album for destination album or pDest if it is not an alias
		* pSrc,			// actual source album, may be null for external source and may be an alias album for internal source
		* pSrcAlbum;		// real source album, which may be the base album for pSrc if it is an alias

	AlbumPointers() { Clear(); }
	AlbumPointers(IDVal_t srcAlbumId, IDVal_t destAlbumId) : srcAlbumId(srcAlbumId), destAlbumId(destAlbumId) { SetupPointers(); }
	AlbumPointers(const AlbumPointers& other) { *this = other; }
	void Clear() { srcAlbumId = destAlbumId = NO_ID; pDest = pDestAlbum = pSrc = pSrcAlbum = nullptr; }
	void SetupPointers();
	void Setup(IDVal_t srcId, IDVal_t destId)
	{
		srcAlbumId = srcId;
		destAlbumId = destId;
		SetupPointers();
	}
};

//*****************************************
//*****************************************


//*****************************************
//*****************************************
int SeparateFileNamePath(QString fullName, QString &path, QString& name, QString *pext = nullptr);
QString PrependSourcePathTo(const QString s);
QString CutSourceRootFrom(QString path);
QString TimeToHMSStr(time_t t);
QString BackupAndRename(QString name, QString tmpName, bool keepBackup = false);
FileType FileTypeFromName(const QString &name, QFileInfo *fi = nullptr);
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
//*****************************************
void ShowSplashScreen(bool addText=false);
void CloseSplashScreen();
void ShowWarning(QString qs, QWidget *parent = nullptr);
void InformationMessage(bool WarningAndNotInfo, QString title, QString text, int show = 0, QString checkboxtext = QString(), QWidget* parent = nullptr);
// QuestionDialog returns a single bit at position 'show', which, when 1 may be QMessageBox::Yes or QMessageBox::Save only!
int QuestionDialog(QString title, QString text, Common::DialogBitsOrder which = Common::DialogBitsOrder::dboNone, QWidget* parent = nullptr, QString checkboxtext = QString(), QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No);

int DeleteOrRemoveConfirmationDialog(IntList &list, QWidget* parent = nullptr); // returns 0: delete, 1: from disk too, 2: canceled

const char* StringToUtf8CString(QString qs);

QString EncodeText(const QString str);	// replaces '\n' with '\\n', '&' with '&amp;',
QString DecodeTextFor(const QString str, DecodeTextTo to = dtPlain);		// for dtPlain -  "\\n" -> '\n', for Html "\\n" to "<br>\n"
																			// for dtJavascript "\\n" -> "<br>" and quotos by escaped quotes \' or \"


QImage LoadImage(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

QPixmap LoadPixmap(QString path, int maxwidth, int maxheight, bool doNotEnlarge = true);

bool CopyOneFile(QString src, QString dest, bool overWrite = true);
bool CreateDir(QString sdir, bool ask = true, int dirIndex=0); // recursive creation of directories, returns 0: error, 1: created, -1: cancelled
											  // if dirIndex > 0: create a directory with a number appended to the name after a dash
bool RemoveFolderRecursively(QString name, bool tryToTrash=true);			// won't ask
bool RemoveDir(QString name, bool ask = false, bool tryToTrash=true);

char* StringToCString(QString string);
QString MakeRandomStringOfLength(int length);

//------------------------------

QFont::Weight IntToFontWeight(int w); 	// uses the Qt 5 weight values but the enum names are the same
										// in Qt 6, so i keep these names
int FontWeightToInt(QFont::Weight w);	// uses the Qt 5 weight values but the enum names are the same
										// in Qt 6, so i keep these names

