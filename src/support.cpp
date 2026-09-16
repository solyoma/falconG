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
#include "albums.h"

/* --------------------------------- helper functions -----------------------*/
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
			res.replace('\'', 0x02);  // STX
			res.replace('\'', 0x03);  // ETX
			res.replace('"' , 0x04);  // EOT
			res.replace('\\', 0x05);  // ENQ
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

QFont::Weight IntToFontWeight(int w) 	// uses the Qt 5 weight values but the enum names are the same
{									// in Qt 6, so i keep these names
	switch (w)
	{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case 0: return	QFont::Thin;
	case 12: return QFont::ExtraLight;
	case 25: return QFont::Light;
	default:					   // Default to Normal if unknown
	case 50: return QFont::Normal;
	case 57: return QFont::Medium;
	case 63: return QFont::DemiBold;
	case 75: return QFont::Bold;
	case 81: return QFont::ExtraBold;
	case 87: return QFont::Black;
#else
		case 100: return QFont::Thin;
		case 200: return QFont::ExtraLight;
		case 300: return QFont::Light;
	default:					   // Default to Normal if unknown
		case 400: return QFont::Normal;
		case 500: return QFont::Medium;
		case 600: return QFont::DemiBold;
		case 700: return QFont::Bold;
		case 900: return QFont::ExtraBold;
		case 900: return QFont::Black;
#endif
	}
}

int FontWeightToInt(QFont::Weight w)	// uses the Qt 5 weight values but the enum names are the same
{								// in Qt 6, so i keep these names
	switch (w)
	{
	case QFont::Thin: return 0;
	case QFont::ExtraLight: return 12;
	case QFont::Light: return 25;
	default:					   // Default to Normal if unknown
	case QFont::Normal: return 50;
	case QFont::Medium: return 57;
	case QFont::DemiBold: return 63;
	case QFont::Bold: return 75;
	case QFont::ExtraBold: return 81;
	case QFont::Black: return 87;
	}
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

