#include <QDir>
#include <QObject>
#include <QtWidgets>
#include <QtDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QTextStream>

#include "support.h"
#include "config.h"
#include "falcong.h"
#include "albums.h"

#include "logger.h"			  

/* --------------------------------- helper functions -----------------------*/
static void  WarningToFile(QString qs)
{
	QFile f(PROGRAM_CONFIG::homePath + "falconG.warnings");

	if (f.open(QIODevice::WriteOnly | QIODevice::Append))
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
		idx = QRandomGenerator::global()->bounded(array_size-1);
		QChar ch = array_str[idx];
		Q_ASSERT(ch.unicode());
		result.append(ch);
	}

	return result;
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
		msgBox.setWindowTitle(QMainWindow::tr(FG_WARNING));
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
		WarningToFile(text);	// save for reference

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
 *			show: integer between 1 and dboMax, ordinal of
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
int QuestionDialog(QString title, QString text, DialogBitsOrder show, QWidget* parent, QString checkboxtext, QMessageBox::StandardButtons buttons)
{
	if (show > DialogBitsOrder::dboNone && config.doNotShowTheseDialogs.v & (1 << (int) show))
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
	QString plurali = QObject::tr("images"), 
			plurala = QObject::tr("albums"),   // plural for image and album. May differ in other languages
			plurald = QObject::tr("them");      // plural for it, them, etc.plurald
	QString qs = QObject::tr("Do you want to delete selected %1 / %2 from disk,\n"
							 "including images/videos and sub-albums inside selected albums,\n"
							 "or just to remove %3 from gallery?")
		.arg(list.size() > 1 ? plurali : QObject::tr("image"))
		.arg(list.size() > 1 ? plurala : QObject::tr("album"))
		.arg(list.size() > 1 ? plurald: QObject::tr("it"));

	QMessageBox msg;
	msg.setWindowTitle(QObject::tr("falconG - Delete Images and albums"));
	msg.setText(qs);

	qs = QObject::tr("This operation cannot be undone!\n\n"
					 "Even selecting 'From disk' does not quaranty the selection\n"
		             "will be removed from disk. They may appear in other albums.\n"
					 "If a base - non alias - album, which has aliases is deleted,\n"
					 "its items are transferred into one of its aliases and the \n"
					 "folder and its items remain intact.");

	msg.addButton(QObject::tr("Just remove"), QMessageBox::NoRole);							// 0
	msg.addButton(QObject::tr("From disk"), QMessageBox::YesRole);							// 1
	QPushButton *pbCancel = msg.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);  // 2
	msg.setDefaultButton(pbCancel);
	msg.setIcon(QMessageBox::Question);
	int res = msg.exec();

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
*							"\n" => "<br>\n" 
*							"\\n" => "<br>\n" 
*							"\"" => "&quot;" 
*							"'"  => "&apos;" 
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
	bool inQuote = false;

	switch (purpose)
	{
		case dtPlain:
			res.replace("\\n", "\n");
			res.replace("<br>", "\n");
			res.replace("<br/>", "\n");
			res.replace("&amp;", "&");
			res.replace("&lt;", "<");
			res.replace("&gt;", ">");
			break;
		case dtDescription:
			inQuote = true;
			//[[fallthrough]]
		case dtHtml:
			res.replace("\\n","<br/>");
			res.replace("\n", "<br/>");
			if(inQuote)
			{
				res.replace("\"", "&quot;"); // but only inside quotes or apostrophes
				res.replace("\'", "&apos;"); // - " - 
			}
			break;
		case dtJavaScript:
			res.replace("\n", "<br>");
			res.replace("\\n", "<br>");
			res.replace("&lt;", "<");
			res.replace("&gt;", ">");
			res.replace(QChar('\''), QChar(0x02));  // STX
			res.replace(QChar('\''), QChar(0x03));  // ETX
			res.replace(QChar('"' ), QChar(0x04));  // EOT
			res.replace(QChar('\\'), QChar(0x05));  // ENQ
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

//*****************************************************

/*============================================================================
* TASK:		create backup of file 'name' as 'name~'
*			and rename file 'tmpName' to 'name'
* EXPECTS:	name		- name of existing file (e.g. fo.struct) to be renamed and backed up
*			tmpName		- name of yet temporary file to be renamed to 'name'
*							(e.g. fo.tmp)
*			keepPreviousBackup: 
*						- true: if a backup file named name~ exists, then
*								    keep it,  else proceed as if this was false.
*								1. delete any 'name.tmp' file
*						        2. rename 'name' file to 'name.tmp' and then
*								3. rename 'tmpFile' to name. 
* 							  As a result we will have 3 files named
*							  'name', 'name~' and 'name.tmp'
*						- false: 1. delete 'name.tmp' file if it exists
*								 2. delete name~ file if it exists,
* 								 3. rename 'name' file to 'name~' and then			
* 								 4. rename 'tmpFile' to name.
* 								 
* GLOBALS:	none
* RETURNS:  empty QString when backup and rename was successful, error message 
*			on error
* REMARKS:
*--------------------------------------------------------------------------*/
QString BackupAndRename(QString name, QString tmpName, bool keepPreviousBackup)
{
	QString qsBackup = name + QString("~"), 
			qsIntermed = name + ".tmp";
	QString qsErr;

	bool bFileExists = QFile::exists(name),
		 bBackupExists = QFile::exists(qsBackup),
		 bIntermExists = QFile::exists(qsIntermed);
									// remove .tmp file
	if(bIntermExists)
		if(!QFile::remove(qsIntermed) )		// delete any previous temporary file
			qsErr = QMainWindow::tr("Can't delete existing temporary file\n'%1'\n").arg(qsIntermed);

	if (!bBackupExists)
		keepPreviousBackup = false;	// no backup file exists, so we can't keep it

	if( keepPreviousBackup)
		qsBackup = qsIntermed;
	else if(bBackupExists)
		if(!QFile::remove(qsBackup))	// delete backup file if it exists
			qsErr += QMainWindow::tr("Can't delete existing backup file\n'%1'\n").arg(qsBackup);

	if(bFileExists )
		if( !QFile::rename(name, qsBackup) )	// rename original file to temporary name
				qsErr += QMainWindow::tr("Can't rename file\n'%1'\nto \n'%2'\n").arg(name).arg(qsBackup);
	if(!QFile::rename(tmpName, name))	// rename temporary file to original name
		qsErr += QMainWindow::tr("Can't rename file\n'%1'\nto \n'%2'").arg(tmpName).arg(name);

	return qsErr;
}

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

QString PrependSourcePathTo(QString s)
{
	if (!s.endsWith(QChar('/')))
		s += '/';
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
	if (!path.endsWith(QChar('/')))
		path += '/';
	return path;
}
/*============================================================================
* TASK:		check if file with name and info is an image OR a video file
* EXPECTS:	name - file name
*			fi	 - pointer to FileInfo structure
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
FileType FileTypeFromName(const QString &name, QFileInfo *fi)
{
	if (fi && fi->isDir())
		return ftFolder;
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

void SetFontWeight(QFont& font, int w)
{
	QFont::Weight weight = QFont::Normal;

	if (w <= 150) weight = QFont::Thin;
	else if (w <= 250) weight = QFont::ExtraLight;
	else if (w <= 350) weight = QFont::Light;
	else if (w <= 450) weight = QFont::Normal;
	else if (w <= 550) weight = QFont::Medium;
	else if (w <= 650) weight = QFont::DemiBold;
	else if (w <= 750) weight = QFont::Bold;
	else if (w <= 850) weight = QFont::ExtraBold;
	else weight = QFont::Black;

	font.setWeight(weight);
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
//			_thumbSize.setWidth(maxSize.width());
//			_thumbSize.setHeight(maxSize.width() / aspect);
//		}
//		if (aspect <= 1)
//		{
//			if ((newSize.height() > maxSize.y()) || ((newSize.height() < maxSize.y()) && (flags & dontEnlarge) == 0))
//			{
//				newSize.setHeight(maxSize.y());
//				newSize.setWidth(aspect * maxSize.y());
//			}
//			// thumbs always resized even when it means enlargement
//			_thumbSize.setHeight(maxSize.height());
//			_thumbSize.setWidth(aspect * maxSize.height());
//		}
//	}
//	imgReader.setScaledSize(newSize);	// newSize used in read, _thumbSize used in write
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
	imgReader.setScaledSize(newSize);	// rescale to newSize when read, _thumbSize used in write

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
	question.setText(QMainWindow::tr(FG_QUESTION));
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
*			ask - should ask the user to create the folder?
*			dirIndex - if not 0 appended to the name after a dash. Example: album-7
* RETURNS:  1: directory exists (or created successfully)
*			0: directory creation error
*			-1: cancelled
* REMARKS: 	 
*--------------------------------------------------------------------------*/
bool CreateDir(QString sdir, bool ask, int dirIndex) // only create if needed
{										// ask - if does not exist ask what to do
	if (dirIndex)
		sdir += QString("-%1").arg(dirIndex);
	QDir folder;
	if (folder.exists(sdir))
		return true;

	if (ask && __CancelCreate(sdir))
		return false;
	QDir dir(sdir);
	if(!dir.mkpath("."))  // create all path recursively if it doesn't exist
	{
		QString s = QMainWindow::tr("Can't create folder") + QString("'%1'").arg(sdir);
		QMessageBox::warning(nullptr, QMainWindow::tr(FG_WARNING), s);
		return false;
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

// ************* struct Watermark ************

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
QPixmap *MarkedIcon::_folderThumbMark = nullptr;
QPixmap *MarkedIcon::_aliasMark = nullptr;
QPixmap *MarkedIcon::_noImageMark = nullptr;
QPixmap *MarkedIcon::_noResizeMark = nullptr;
int  MarkedIcon::_thumbSize = THUMBNAIL_SIZE;		// named image is inside a (size x size) area this keeping aspect ratio
int  MarkedIcon::_borderWidth = _thumbSize / THUMBNAIL_BORDER_FACTOR;				// in pixels portrait image: right and left, landscape image top and bottom
bool MarkedIcon::_initted = false;				// images for icons read?


/*=============================================================
 * TASK:	reads an image into '_pxmp' member from file and
 *			shows it on a square pixmap with '_thumbSize' side
 *			on a background whose color depends on the type of
 *			the image (folder thumbnail or image thumbnail)
 * PARAMS:	name: file name to read image from
 *			iflags: icon flags may contain other flags than 
 *					image type
 * GLOBALS:	static members are set
 * RETURNS:	if file read was successful
 * REMARKS: if read is unsuccessfull the pixmap still valid
 *------------------------------------------------------------*/
bool MarkedIcon::Read(QString fname, IconFlags iflags)
{
	_name = fname;
	flags = iflags;

	QString colorname = (flags & fiFolder ? config.albumMatteColor : config.imageMatteColor).Name(true);
	QColor cbck = colorname;
	if (!cbck.isValid())
		return false;

	_pxmp = QPixmap(_thumbSize, _thumbSize);
	_pxmp.fill(cbck);

	QSize dsize;		// thumbnail size
	static QImage img;	// thumbnail image (for video: get from video file)

	auto setDSize = [&](QSize osize)  // get destinaton size for thumbnail image
		{
			if (osize.width() >= osize.height())	// portrait
			{
				dsize.setWidth(_thumbSize - 2 * _borderWidth);
				dsize.setHeight((double)(_thumbSize - 2 * _borderWidth) / (double)(osize.width()) * osize.height());
			}
			else
			{
				dsize.setHeight(_thumbSize - 2 * _borderWidth);
				dsize.setWidth((double)(_thumbSize - 2 * _borderWidth) / (double)(osize.height()) * osize.width());
			}							  
		};

	if (flags & fiVideo)	// then thumbnail is a frame from the video
	{
		Video* pvid = albumgen.Videos().Find(fname);
		if (!pvid)
			return false;	// no video found

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		fname.replace(fname.lastIndexOf('.'), 10, ".png");
#else
		if (!pvid->GetThumbnail(img, dsize, _thumbSize))
		{
			QMessageBox::warning(nullptr, QMainWindow::tr(FG_WARNING), 
				QMainWindow::tr("Can't get thumbnail for video file '%1'").arg(fname));
			return false;
		}
#endif
	}
	else   // image (may be a) folder thumbnail
	{
		QImageReader reader(fname);
		reader.setBackgroundColor(cbck);
		reader.setAutoTransform(true);

		_exists = false;

		QSize osize = reader.size();
		if (!osize.isValid())
			return false;
		setDSize(osize);

		reader.setScaledSize(dsize);

		if (!reader.read(&img))		// maybe error display?
			return false;

		_exists = true;
	}
	QPainter painter(&_pxmp);	// leave the border outside
	int xm =(_pxmp.width() - dsize.width()) / 2, ym = (_pxmp.height() - dsize.height()) / 2;
	painter.drawImage(xm, ym, img);

	return true;
}

/*=============================================================
 * TASK:	from _pxmp returns an icon with markers on it
 * PARAMS:
 * EXPECTS: _pxmp contains the already rendered background 
 *			(folders and others) and the image
 * GLOBALS:
 * RETURNS: an icon for image read. 
 * REMARKS: - If no markers are to be set on this item returns just 
 *			the pixmap '_pxmp'
 *			- thumbnail & alias markers are at top left, 
 *				fon't resize and missing flag at top right
 *------------------------------------------------------------*/
QIcon MarkedIcon::ToIcon() const
{
	static IconFlags __flags = { fiFolder, fiThumb, fiDontResize, fiAlias };
	if (_exists && (flags & __flags) ==0)	// no markers on images
		return QIcon(_pxmp);

	QPixmap tmp_pxmp(_thumbSize, _thumbSize);
	QPainter painter(&tmp_pxmp);
	painter.drawPixmap(0,0, _pxmp);		// image with border
	if (flags & fiThumb)	   // at top left position
			painter.drawPixmap(_borderWidth, _borderWidth, *_folderThumbMark);	 
	if (flags & fiAlias)
		painter.drawPixmap(_borderWidth + (flags & fiThumb ? _aliasMark->width() : 0), _borderWidth, *_aliasMark);

	if (flags & dontResize)		  // at top right
		painter.drawPixmap(_thumbSize - _folderThumbMark->width() - _borderWidth, _borderWidth, *_noResizeMark);
	if(!_exists)
		painter.drawPixmap(_thumbSize - 2*_noImageMark->width() - _borderWidth, _borderWidth, *_noImageMark);

	return QIcon(tmp_pxmp);
}

void AlbumPointers::SetupPointers()
{
	pSrc = albumgen.Albums().Find(srcAlbumId);			// actual source album or empty pointer if from external source
	pSrcAlbum = pSrc ? pSrc->BaseAlbum() : nullptr,	// if not an alias album pSrcAlbum == pSrc, including the case when pSrc is nullptr
	pDest = albumgen.Albums().Find(destAlbumId);		// actual destination album, which may be an alias
	pDestAlbum = pDest ? pDest->BaseAlbum() : nullptr;	 // drop to this album, no matter if pDest points to an alias
	srcBaseAlbumId = pSrcAlbum->ID.Val();
	destBaseAlbumId = pDestAlbum->ID.Val();
}
