#include <QMainWindow>
#include "enums.h"
using namespace Enums;
#include "support.h"
#include "languages.h"
#include "config.h"

/*--------------------- languages -------------------------*/
LangConstList Languages::abbrev;		// used instead of country code in names. e.g. set "_us" for "en_US" here
LangConstList Languages::albums;		// album section in HTML files
LangConstList Languages::countryCode;	// "en_US", "hu_HU", etc
LangConstList Languages::countOfImages;	// "%1 image(s) and %2 sub-album(s) in this album"
LangConstList Languages::coupleCaptions;  // 'Képcímek'
LangConstList Languages::falconG;		// "created by falconG"
LangConstList Languages::icons;			// icons to use instead of names
LangConstList Languages::images;		// image section in HTML files
LangConstList Languages::language;		// this is set in 'lang=XX' in HTML
LangConstList Languages::latestTitle;	// 'Kedvcsinálónak'
LangConstList Languages::latestDesc;	// 'Ebbe a könyvtárba...'
LangConstList Languages::names;			// used on menus to switchch language, eg. "Magyarul"
LangConstList Languages::share;			// 'Megosztás'
LangConstList Languages::showDescriptions;  // 'Képleírások'
LangConstList Languages::toHomePage;	// 'Kezdõlapra'
LangConstList Languages::toAboutPage;	// 'Rólam'
LangConstList Languages::toAlbums;		// jump to album section
LangConstList Languages::toContact;		// 'Kapcsolat'
LangConstList Languages::toTop;			// top of page
LangConstList Languages::upOneLevel;	// jump to parent
LangConstList Languages::videos;		// video section in HTML files

/*============================================================================
* TASK:		reads language definitions from files in the
*				a) source 
*				b) destination directory or
*				c) program directory
*	in this order
* EXPECTS:	name of file to read in
* GLOBALS:  
* RETURNS: 0: error, actual number of languages read so far: success
* REMARKS:	- file names: <lang abbrev, eg. hu_HU>.lang	
*			- the program reads all such file from directory
*			- file formats:  
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
	int lang = names.size();

	abbrev.push_back(s);
	albums.push_back(s);
	countOfImages.push_back(s);
	countryCode.push_back(s);
	coupleCaptions.push_back(s);
	falconG.push_back(s);
	icons.push_back(s);
	images.push_back(s);
	language.push_back(s);
	latestDesc.push_back(s);
	latestTitle.push_back(s);
	names.push_back(s);
	share.push_back(s);
	showDescriptions.push_back(s);
	toAboutPage.push_back(s);
	toAlbums.push_back(s);
	toContact.push_back(s);
	toHomePage.push_back(s);
	toTop.push_back(s);			// title of icon to jump to top of page
	upOneLevel.push_back(s);	// title of icon jump to parent/up one level
	videos.push_back(s);

	QStringList sl;
	QString sn; // name in lowercase
	while (!(line = reader.ReadLine()).isEmpty())
	{
		sl = line.split('=');
		sn = sl[0].toLower();
		s = (sl.size() < 2 ? "" : sl[1]);
		if (sn == "")
			;
		else if (sn == "abbrev")
			abbrev[lang] = s;
		else if (sn == "about")
			toAboutPage[lang] = s;
		else if (sn == "albums")
			albums[lang] = s;
		else if (sn == "cdcaptions")
			coupleCaptions[lang] = s;
		else if (sn == "contact")
			toContact[lang] = s;
		else if (sn == "countofimages")
			countOfImages[lang] = s;
		else if (sn == "countrycode")
			countryCode[lang] = s;
		else if (sn == "descriptions")
			showDescriptions[lang] = s;
		else if (sn == "falcong")
		{
			QString qsolyom = ToUTF8(QString("Sólyom"));
			if (s.indexOf(qsolyom) < 0)
				s = sFalcongEnglishCopyright;
			falconG[lang] = s;
		}
		else if (sn == "homepage")
			toHomePage[lang] = s;
		else if (sn == "icon")
			icons[lang] = s;
		else if (sn == "images")
			images[lang] = s;
		else if (sn == "language")
			names[lang] = s;
		else if (sn == "latestdesc")
			latestDesc[lang] = s;
		else if (sn == "latesttitle")
			latestTitle[lang] = s;
		else if (sn == "name")
			names[lang] = s;
		else if (sn == "share")
			share[lang] = s;
		else if (sn == "toalbums")
			toAlbums[lang] = s;
		else if (sn == "totop")
			toTop[lang] = s;
		else if (sn == "uponelevel")
			upOneLevel[lang] = s;
		else if (sn == "videos")
			videos[lang] = s;
	}
	if (abbrev[lang].isEmpty())
		abbrev[lang] = countryCode[lang];
	return ++lang;
}

/*============================================================================
* TASK:		reads all language definition files in program directory
* EXPECTS:	nothing
* GLOBALS:
* RETURNS: the number of languages read
* REMARKS:	
*
*			- if no file exists sets a single languag: English
*	?????		- do not use when languages are set from GUI
*--------------------------------------------------------------------------*/
int Languages::Read()
{
	Clear();

	QDir dir;
	QFileInfoList list;
	QStringList nameList; nameList << "*.lang";

	QDir d = QDir::current();

	dir.setCurrent(config.dsSrc.ToString());
	list = dir.entryInfoList(nameList, QDir::Files);
	if (!list.size())
	{
		dir.setCurrent(config.dsGallery.ToString());
		list = dir.entryInfoList(nameList, QDir::Files);
	}
	if (!list.size())
	{
		// actual program directory
		list = d.entryInfoList(nameList, QDir::Files);
		dir.setCurrent(d.path());	// go back to program directory
	}

	if (list.size() == 0)	// no files: set default language: English
	{
		if (QMessageBox(QMessageBox::Warning,
			QMainWindow::tr("falconG - Warning"),
						//            1			2		  3			4		  5			6		  7			8
						//	 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 
			QMainWindow::tr("No language (*.lang) files are found.\n"
							"Do you want to cancel or continue with English as the single language?\n\n"
							"To avoid this message either create language files and put them into the\n"
							"    a) source\n"
							"    b) destination or\n"
							"    c) program directory\n"
							"or write the language texts into the '.struct' file\n\n"
							"A language file is a UTF-8 encoded text file starting with the line:\n"
							"  'falconG Language file'\n"
							"followed by definitions of the fixed texts used in the albums\n"
							" The format of the lines is 'name=text'. The names\n"
							"  and their texts are (the single quotes in the examples below are not used,"
							"  and the examples are in braces):\n"
							"  abbrev      = <language abbrev. in file names (e.g.'_hu' for`album123_hu.html`)>\n"
							"  albums      = <text header for the album section of the  actual album>\n"
							"  coupleCaptions = <text for captions-toggled-with-descriptions toggle>\n"
							"  countOfImages= %1 image(s) and %2 sub-album(s) in this album\n" //%1, %2 placeholders (videos are 'images')
							"  countryCode	= <like 'en_US'> (usually part of this is abbrev and language)\n"
							"  descriptions  =<text for the descriptionss toggle menu>\n"
							"  falconG   =<my copyright message. Please do not remove or change this, just translate>\n"
							"  icon      =<name of icon (.png) file for language> may be left empty\n"
							"  images    =<text header for the images section of the actual album>\n"
							"  language  =<text for'lang=XX' in HTML or 'Title-XX', 'Desc-XX' in '.struct'>"
							"  latestDesc=<description for album 'latest_en.html'>\n"
							"  latestTitle=<title on album 'latest_en.html'>\n"
							"  name	     =<language name to put on language switch menu>  ('to English')\n"
							"  share     =<text for the facebook share button>"
							"  toAboutPage = <text for the 'About' button>\n"
							"  toAlbums  = <menu text to jump to albums section>"
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
		abbrev.push_back("_en");
		albums.push_back("Albums");
		countOfImages.push_back("%1 image(s) and %2 sub-album(s) in this album");
		countryCode.push_back("en_US");
		coupleCaptions.push_back("Title&amp;Desc Coupled");	// captions and descriptions coupled
		falconG.push_back(sFalcongEnglishCopyright);
		icons.push_back("");
		images.push_back("Images");
		language.push_back("en");
		latestDesc.push_back("Randomly selected max " + QString().setNum(config.nLatestCount) + "Latest uploads");
		latestTitle.push_back("Latest uploads");
		names.push_back("English");
		share.push_back("Share");
		showDescriptions.push_back("Descriptions");
		toAboutPage.push_back("About");
		toAlbums.push_back("to Albums");
		toContact.push_back("Contact");
		toHomePage.push_back("Home");
		toTop.push_back("Jump to top of page");
		upOneLevel.push_back("Up one level");
		videos.push_back("Videos");
		return 1;
	}

	for (auto fi : list)
	{
		_Read(fi.fileName());
	}
	return falconG.size();
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
	abbrev.Prepare(newsize);
	albums.Prepare(newsize);
	countOfImages.Prepare(newsize);
	countryCode.Prepare(newsize);
	coupleCaptions.Prepare(newsize);
	falconG.Prepare(newsize);
	icons.Prepare(newsize);
	images.Prepare(newsize);
	language.Prepare(newsize);
	latestDesc.Prepare(newsize);
	latestTitle.Prepare(newsize);
	names.Prepare(newsize);
	share.Prepare(newsize);
	showDescriptions.Prepare(newsize);
	toAboutPage.Prepare(newsize);
	toAlbums.Prepare(newsize);
	toContact.Prepare(newsize);
	toHomePage.Prepare(newsize);
	toTop.Prepare(newsize);
	upOneLevel.Prepare(newsize);
	videos.Prepare(newsize);
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
	if (Languages::Count() == 1)
		return s;
	QString path, name, ext;
	SeparateFileNamePath(s, path, name, &ext);

	if (config.bSeparateFoldersForLanguages)
		name = Languages::abbrev[lang] + "/" + s;	// e.g. _en/album123.html
	else
		name += Languages::abbrev[lang];		// album123_en_US.html
	return path + name + ext;
}

