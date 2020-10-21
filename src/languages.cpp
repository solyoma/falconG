#include <QMainWindow>
#include "enums.h"
using namespace Enums;
#include "support.h"
#include "languages.h"
#include "config.h"

/*--------------------- languages -------------------------*/
LangConstList Languages::abbrev;		// abbreviations: "hu", "en", etc
LangConstList Languages::names;			// used on menus to switchch language, eg. "Magyarul"
LangConstList Languages::icons;			// icons to use instead of names
LangConstList Languages::Images;		// image section in HTML files
LangConstList Languages::Albums;		// album section in HTML files
LangConstList Languages::toAlbums;		// jump to album section
LangConstList Languages::toTop;			// top of page
LangConstList Languages::upOneLevel;	// jump to parent
LangConstList Languages::toHomePage;		// 'Kezdõlapra'
LangConstList Languages::toAboutPage;		// 'Rólam'
LangConstList Languages::toContact;		// á'Kapcsolat'
LangConstList Languages::showCaptions;    // 'Képcímek'
LangConstList Languages::showDescriptions;    // 'Képleírások'
LangConstList Languages::share;			// 'Megosztás'
LangConstList Languages::latestTitle;		// 'Kedvcsinálónak'
LangConstList Languages::latestDesc;		// 'Ebbe a könyvtárba...'
LangConstList Languages::countryCode;	// "en_US", "hu_HU", etc
LangConstList Languages::countOfImages;	// "%1 image(s) and %2 sub-album(s) in this album"
LangConstList Languages::falconG;		// "created by falconG"

/*============================================================================
* TASK:		reads language definitions from files in the
*				a) source 
*				b) destination directory or
*				c) program directory
*	in this order
* EXPECTS:	name of file to read in
* GLOBALS:  
* RETURNS: 0: error, actual number of languages read so far: success
* REMARKS:	- file names: <lang abbrev>.lang	
*			- the program reads all such file from directory
*			- file formats:  
*			   1st line		"falconG Language file"
*				others may follow in any order and the right hand side may be empty
*				abbrev=<2 letter abbrev>
*				name=<text to put buttons or lists>
*				icon=<icon (png) file name>
*				Images=<text for Images section>
*				Albums=<text for Albums section>
*				HomePage=<e.g. Home>		
*				About=<about>		
*				Contact=<contact>
*				captions=<text to show descriptions";
*            	share=<text for sharing links>
*			- if no file does not exist sets a single languag: English
*			- do not use when languages are set from GUI
*--------------------------------------------------------------------------*/
int Languages::_Read(QString name)
{
	FileReader reader(name, frfNeedUtf8 | frfTrim);
	QString line = reader.ReadLine();
	if (line != "falconG Language file")
		return 0;

	QString s;
	int next = abbrev.size();
	abbrev.push_back(s);
	names.push_back(s);
	icons.push_back(s);
	Images.push_back(s);
	Albums.push_back(s);
	toAlbums.push_back(s);
	toHomePage.push_back(s);
	toAboutPage.push_back(s);
	toContact.push_back(s);
	showDescriptions.push_back(s);
	share.push_back(s);
	latestTitle.push_back(s);
	latestDesc.push_back(s);
	countryCode.push_back(s);
	countOfImages.push_back(s);
	falconG.push_back(s);

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
			abbrev[next] = s;
		else if (sn == "name")
			names[next] = s;
		else if (sn == "icon")
			icons[next] = s;
		else if (sn == "images")
			Images[next] = s;
		else if (sn == "albums")
			Albums[next] = s;
		else if (sn == "toalbums")
			toAlbums[next] = s;
		else if (sn == "totop")
			toAlbums[next] = s;
		else if (sn == "upOneLevel")
			toAlbums[next] = s;
		else if (sn == "homepage")
			toHomePage[next] = s;
		else if (sn == "about")
			toAboutPage[next] = s;
		else if (sn == "contact")
			toContact[next] = s;
		else if (sn == "descriptions")
			showDescriptions[next] = s;
		else if (sn == "share")
			share[next] = s;
		else if (sn == "latesttitle")
			latestTitle[next] = s;
		else if (sn == "latestdesc")
			latestDesc[next] = s;
		else if (sn == "countrycode")
			countryCode[next] = s;
		else if (sn == "countofimages")
			countOfImages[next] = s;
		else if (sn == "falcong")
		{
			QString qsolyom = ToUTF8(QString("Sólyom"));
			if (s.indexOf(qsolyom) < 0)
				s = sFalcongEnglishCopyright;
			falconG[next] = s;
		}
	}
	return ++next;
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
							"    b) destination directory or\n"
							"    c) program directory\n"
							"or write the language texts into the '.struct' file\n\n"
							"A language file is a UTF-8 coded text file starting with line:\n"
							"  'falconG Language file'\n"
							"followed by definitions of the fixed texts used in the albums (language text\n"
							" blocks).\n"
							"When the language texts are put into the struct file, there must be as many\n"
							" language text blocks as languages. Each language text block is preceeded by\n"
							"  a line containing a single number: the ordinal of that language box.\n"
							" The language text blocks are enclosed between lines\n"
							"			'[Language count:xx' and ']'\n"
							"	where 'xx' is the ordinal of the block minus one.\n"
							" A language text block is made up from lines in the form 'name=text'. The names\n"
							"  and their texts are (the single quotes in the examples below are not used):\n"
							"  abbrev    =<2 letter language abbreviation> e.g. 'en'\n"
							"  name	     =<language name to put on language switch menu> e.g.  'to English'\n"
							"  icon      =<name of icon (.png) file for language> may be left empty\n"
							"  images    =<text header for the images section of the actual album>\n"
							"  toTop     =<text for up arrow: jump to top of page>"
							"  upOneLevel=<text for up/left arrow: jump to parent dir>"
							"  albums    =<text header for the album section of the  actual album>>\n"
							"  homePage	 =<text for menu to home page>\n"
							"  about     =<text for the 'About' button>\n"
							"  contact   =<text for the 'contact' button\n"
							"  share     =<text for the facebook share button>"
							"  latestTitle=<title for album latest.html>"
							"  latestDesc=<description for album latest.html>\n"
							"  countryCode	=<like en_US>\n"
							"  countOfImages=% 1 image(s) and %2 sub-album(s) in this album\n"
							"  falconG   =<copyright message. Please do not remove or change. just translate"
			),
					QMessageBox::Ok | QMessageBox::Cancel, nullptr).exec() == QMessageBox::Cancel)
			return 0;
		abbrev.push_back("en");
		names.push_back("English");
		icons.push_back("");
		Images.push_back("images");
		Albums.push_back("Albums");
		toAlbums.push_back("to Albums");
		toHomePage.push_back("Home");
		toTop.push_back("Jump to top of page");
		upOneLevel.push_back("Up one level");
		toAboutPage.push_back("About");
		toContact.push_back("Contact");
		showDescriptions.push_back("Captions");
		share.push_back("Share");
		latestTitle.push_back("Latest uploads");
		latestDesc.push_back("Randomly selected max " + QString().setNum(config.nLatestCount) + "Latest uploads");
		countryCode.push_back("en_US");
		countOfImages.push_back("% 1 image(s) and %2 sub-album(s) in this album");
		falconG.push_back(sFalcongEnglishCopyright);
		return 1;
	}

	for (auto fi : list)
	{
		_Read(fi.fileName());
	}
	return abbrev.size();
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
	names.Prepare(newsize);
	icons.Prepare(newsize);
	Images.Prepare(newsize);
	Albums.Prepare(newsize);
	toAlbums.Prepare(newsize);
	toTop.Prepare(newsize);
	upOneLevel.Prepare(newsize);
	toHomePage.Prepare(newsize);
	toAboutPage.Prepare(newsize);
	toContact.Prepare(newsize);
	share.Prepare(newsize);
	showDescriptions.Prepare(newsize);
	latestTitle.Prepare(newsize);
	latestDesc.Prepare(newsize);
	countryCode.Prepare(newsize);
	countOfImages.Prepare(newsize);
	falconG.Prepare(newsize);
}
/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
QString Languages::FileNameForLanguage(QString s, int language)
{
	QString path, name, ext;
	SeparateFileNamePath(s, path, name, &ext);

	if (Languages::Count() > 1)
		name += "_" + Languages::abbrev[language];
	return path + name + ext;
}

