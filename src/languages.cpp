#include <QMainWindow>
#include "enums.h"
using namespace Enums;
#include "support.h"
#include "languages.h"
#include "config.h"

/*--------------------- languages -------------------------*/

Languages languages;

void Languages::SetTextFor(QString name, QString val,int lang)
{
	if (name == "")
		return;

	if (name == "falcong")	// special for copyright
	{
		QString qsolyom = ToUTF8(QString("Sólyom"));
		if (val.indexOf(qsolyom) < 0)
			val = sFalcongEnglishCopyright;
		
	}
	auto cit = constFind(name);
	if (cit != constEnd())
		(*cit.value())[lang] = val;
}

/*============================================================================
* TASK:		reads language definitions from files
* EXPECTS:	name of file to read in
* GLOBALS:  
* RETURNS: 0: error, actual number of languages read so far: success
* REMARKS:	- file names: <lang abbrev, eg. hu_HU>.lang	
*			- the program reads all such files from directory
*			- file format:  
*	   1st line		"falconG Language file"
*  about     =&lt;text for the &apos;About&apos; button&gt;
"  abbrev    =<text for 'countryCode' used in file names (def.:'countryCode')>\n"
*  albums    =&lt;text header for the album section of the  actual album&gt;
*  contact   =&lt;text for the &apos;contact&apos; button
*  countOfImages=%1 image(s) and %2 sub-album(s) in this album
*  countryCode	=&lt;like &apos;en_US&apos;&gt;
*  descriptions  =&lt;text for the descriptionss toggle menu&gt;
*  falconG   =&lt;Copyright message. Please do not remove or change this. just translate&gt;
*  homePage	 =&lt;text for menu to home page&gt;
*  icon      =&lt;name of icon (.png) file for language&gt; may be left empty
*  images    =&lt;text header for the images section of the actual album&gt;
*  latestDesc=&lt;description for album &apos;latest_en.html&apos;&gt;
*  latestTitle=&lt;title on album &apos;latest_en.html&apos;&gt;
*  name	     =&lt;language name to put on language switch menu&gt;  (&apos;to English&apos;)
*  share     =&lt;text for the facebook share button&gt;  captions  =&lt;text for the captions-descriptions couple toggle&gt;
*  toTop     =&lt;text for up arrow: jump to top of page&gt;
*  upOneLevel=&lt;text for up/left arrow: jump to parent dir&gt;
*  videos    =&lt;text header for the videos section of the actual album&gt;*			- if no file does not exist sets a single languag: English
**			- do not use when languages are set from GUI
*--------------------------------------------------------------------------*/
int Languages::_Read(QString name)
{
	FileReader reader(name, frfNeedUtf8 | frfTrim);
	QString line = reader.ReadLine();
	if (line != "falconG Language file")
		return 0;

	QString s;
	int lang = LanguageCount();	// this many languages has been read so far

	// before calling this first clear previous data !
	// reserve place for as many strings as there are files/languages

	for (auto it = begin(); it != end(); ++it)
		it.value()->push_back(s);

	QStringList sl;
	while (!(line = reader.ReadLine()).isEmpty())
	{
		sl = line.split('=');
		SetTextFor(sl[0], (sl.size() < 2 ? "" : sl[1]), lang);
	}
	return ++lang;
}

/*============================================================================
* TASK:		reads all language definition files  in the
*				a) gallery source 
*				b) user directory or
*				c) program directory
*	in this order
* EXPECTS:	nothing
* GLOBALS: config, static Language strings
* RETURNS: the number of languages read
* REMARKS:	- language files have extension ;.lang'
*			- tries to read lnag files from the following directories:
*				config.dsSrc, destination directory, program directory
*			- searches the directory for language files to read them all
*			- if no language file exists sets a single language: English
*			- when languages are set in the GUI we will use only those
*--------------------------------------------------------------------------*/
int Languages::Read()
{
	Clear();

	QDir dir;
	QFileInfoList list;
	QStringList nameList; nameList << "*.lang";

	QDir dc = QDir::current();

	dir.setPath(config.dsSrc.ToString());					// gallery source
	list = dir.entryInfoList(nameList, QDir::Files);
	if (!list.size())
	{
		dir.setPath(PROGRAM_CONFIG::homePath);				// user folder
		list = dir.entryInfoList(nameList, QDir::Files);
		if (!list.size())										// current (program) folder
		{
			// actual program directory
			dir.setPath(dc.path());
			list = dc.entryInfoList(nameList, QDir::Files);
		}
	}

	if (!list.size())	// still no files: set single default language: English
	{
		if (QMessageBox(QMessageBox::Warning,
			QMainWindow::tr("falconG - Warning"),
						//            1			2		  3			4		  5			6		  7			8
						//	 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
			QMainWindow::tr("No language (*.lang) files are found.\n"
							"Do you want to cancel or continue with English as the single language?\n\n"
							"To avoid this message either create language files and put them into either\n"
							"    a) the gallery source folder\n"
							"    b) the user folder (Windows:Appdata/Local/falconG, linux: ~/.falconG) or\n"
							"    c) the program folder\n"
							"or write the language texts into the '.struct' file\n\n"
							"A language file is a UTF-8 encoded text file starting with the line:\n"
							"  'falconG Language file'\n"
							"followed by definitions of the fixed texts used in the albums\n"
							" The format of the lines is 'name=text'. The names\n"
							"  and their texts are (the single quotes in the examples below are not used,"
							"  and the examples are in braces):\n\n"
							"  abbrev      = <language abbrev. in file names if any (e.g.'_hu' for`album123_hu.html`)>\n"
							"  albums      = <text header for the album section of the  actual album>\n"
							"  countOfImages= %1 image(s) and %2 sub-album(s) in this album\n" //%1, %2 placeholders (videos are 'images')
							"  countryCode	= <like 'en_US'> (usually part of this is abbrev and language)\n"
							"  coupleCaptions = <text for captions-toggled-with-descriptions toggle>\n"
							"  falconG   =<my copyright message. Please do not remove or change this, just translate>\n"
							"  icon      =<name of icon (.png) file for language> may be left empty\n"
							"  images    =<text header for the images section of the actual album>\n"
							"  language  =<text for'lang=XX' in HTML or 'Title-XX', 'Desc-XX' in '.struct'>\n"
							"  latestDesc=<description for album 'latest_en.html'>\n"
							"  latestTitle=<title on album 'latest_en.html'>\n"
							"  name	     =<language name to put on language switch menu>  ('to English')\n"
							"  share     =<text for the facebook share button>\n"
							"  showDescriptions  =<text for the descriptionss toggle menu>\n"
							"  toAboutPage = <text for the 'About' button>\n"
							"  toAlbums  = <menu text to jump to albums section>\n"
							"  toContact   =<text for the 'contact' button\n"
							"  toHomePage	 =<text for menu to home page>\n"
							"  toTop     =<text for up arrow: jump to top of page>\n"
							"  upOneLevel=<text for up/left arrow: jump to parent dir>\n"
							"  videos    =<text header for the videos section of the actual album>\n"
							" \n"
							"When the language texts are put into the struct file, there must be as many\n"
							" language text blocks as languages. Each language text block is preceeded by\n"
							"  a line containing a single number: the ordinal of that language box,\n"
							"  followed by the same lines as in the .lang files\n"
							" The language text blocks are enclosed between lines\n"
							"			'[Language count:xx' and ']'\n"
							"	where 'xx' is the ordinal of the block minus one.\n"),
					QMessageBox::Ok | QMessageBox::Cancel, nullptr).exec() == QMessageBox::Cancel)
			return 0;

		QMap::operator[]("abbrev")->push_back(""); // no need for abbrev if only one language
		QMap::operator[]("albums")->push_back("Albums");
		QMap::operator[]("countOfImages")->push_back("%1 image(s) and %2 sub-album(s) in this album");
		QMap::operator[]("countryCode")->push_back("en_US");
		QMap::operator[]("coupleCaptions")->push_back("Title&amp;Captions Coupled");
		QMap::operator[]("falconG")->push_back(sFalcongEnglishCopyright);
		QMap::operator[]("icon")->push_back("");
		QMap::operator[]("images")->push_back("Images");
		QMap::operator[]("language")->push_back("en");
		QMap::operator[]("latestDesc")->push_back("Randomly selected max " + QString().setNum(config.nLatestCount) + "Latest uploads");
		QMap::operator[]("latestTitle")->push_back("Latest uploads");
		QMap::operator[]("name")->push_back("English");
		QMap::operator[]("share")->push_back("Share");
		QMap::operator[]("showDescriptions")->push_back("Descriptions");
		QMap::operator[]("toAboutPage")->push_back("About");
		QMap::operator[]("toAlbums")->push_back("to Albums");
		QMap::operator[]("toContact")->push_back("Contact");
		QMap::operator[]("toHomePage")->push_back("Home");
		QMap::operator[]("toTop")->push_back("Jump to top of page");
		QMap::operator[]("upOneLevel")->push_back("Up one level");
		QMap::operator[]("videos")->push_back("Videos");
		return 1;
	}

	for (auto fi : list)
	{
		_Read(fi.fileName());
	}
	return LanguageCount();
}

/*============================================================================
* TASK:   creates new empty language text vectors
* EXPECTS: newsize - when 0 it just clears all and creates lists with no
*						elements
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void Languages::Clear(int newsize)
{
	QMap::operator[]("abbrev")->Prepare(newsize);
	QMap::operator[]("albums")->Prepare(newsize);
	QMap::operator[]("countOfImages")->Prepare(newsize);
	QMap::operator[]("countryCode")->Prepare(newsize);
	QMap::operator[]("coupleCaptions")->Prepare(newsize);
	QMap::operator[]("falconG")->Prepare(newsize);
	QMap::operator[]("icon")->Prepare(newsize);
	QMap::operator[]("images")->Prepare(newsize);
	QMap::operator[]("language")->Prepare(newsize);
	QMap::operator[]("latestDesc")->Prepare(newsize);
	QMap::operator[]("latestTitle")->Prepare(newsize);
	QMap::operator[]("name")->Prepare(newsize);
	QMap::operator[]("share")->Prepare(newsize);
	QMap::operator[]("showDescriptions")->Prepare(newsize);
	QMap::operator[]("toAboutPage")->Prepare(newsize);
	QMap::operator[]("toAlbums")->Prepare(newsize);
	QMap::operator[]("toContact")->Prepare(newsize);
	QMap::operator[]("toHomePage")->Prepare(newsize);
	QMap::operator[]("toTop")->Prepare(newsize);
	QMap::operator[]("upOneLevel")->Prepare(newsize);
	QMap::operator[]("videos")->Prepare(newsize);
}
/*============================================================================
  * TASK:	creates full file name for an HTML file for any language
  * EXPECTS:	s - file name (e.g. album123.html) w.o. languegs 
  * RETURNS:	name for the given language (see REMARKS)
  * GLOBALS:	config
  * REMARKS: - if there's one language only, then keeps name, else
  *				add the language code to the file: 
  *				e.g. 'album123-en_US.html', or 'en"US/album123.html'
  *			 - the caller must ensure that the generated name is used properly
  *				i.e. the folders exist, the file AND the link in that file to albums in the
  *				same language are set correctly!
 *--------------------------------------------------------------------------*/
QString Languages::FileNameForLanguage(QString s, int lang)
{
	if (languages.LanguageCount() == 1)
		return s;
	QString path, name, ext;
	SeparateFileNamePath(s, path, name, &ext);

	if (lang >= 0)
	{
		if (config.bSeparateFoldersForLanguages)
			name = (*operator[]("abbrev"))[lang] + "/" + s;	// e.g. _en/album123.html
		else
			name += (*operator[]("abbrev"))[lang];		// album123_en.html
		return path + name + ext;
	}
	else
	{
		return name;
	}
}

