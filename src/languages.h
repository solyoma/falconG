#pragma once

#include <QString>
#include <QStringList>

const QString sFalcongEnglishCopyright = QString("Site generated by\n<strong>falconG</strong> - © A.Sólyom 2018-2020");

//*****************************************
class LangConstList : public QStringList
{
public:	
	LangConstList(int size = 0) : QStringList() { Prepare(size); }

	void Prepare(int languageCount) { clear();  for (int i = 0; i < languageCount; ++i) push_back(""); }
	void Clear() { for (int i = 0; i < size(); ++i) (*this)[i].clear(); }
	bool IsEmpty() {for (int i = 0; i < size(); ++i) if (!(*this)[i].isEmpty()) return false; return true;}
	void Fill(QString s) { for (int i = 0; i < size(); ++i) (*this)[i] = s; }

    LangConstList &operator=(const LangConstList & txt) { QStringList::operator=(dynamic_cast<const QStringList &>(txt)); return *this;}
    LangConstList &operator=(const QStringList &sl) { QStringList::operator=(sl);  return *this;}
};
//*****************************************

struct Languages	// each language has a text file named <text from 'languages'>.php
{					// or the languages are set in a .struct file
	static LangConstList abbrev;			// abbreviations: "hu", "en", etc
	static LangConstList names;			// used on menus to switch language, eg. "Magyarul"
	static LangConstList icons;			// icons to use instead of names
		// these are read from language definition files '<xxx>.lang in program directory
	static LangConstList Images;		// image section in HTML files 'Képek'
	static LangConstList Videos;		// video section in HTML files 'Videók'
	static LangConstList Albums;		// album section in HTML files 'Albumok'
	static LangConstList toAlbums;		// Jump to album section
	static LangConstList toTop;			// title of icon to jump to top of page
	static LangConstList upOneLevel;	// title of icon jump to parent/up one level
	static LangConstList toHomePage;		// 'Kezdõlapra'
	static LangConstList toAboutPage;	// 'Rólam'
	static LangConstList toContact;		// 'Kapcsolat'
	static LangConstList share;			// 'Megosztás'
	static LangConstList showCaptions;	// 'Képcimek'
	static LangConstList showDescriptions;	// 'Képleírások'
	static LangConstList latestTitle;	// 'Kedvcsinálónak'
	static LangConstList latestDesc;		// 'Ebbe a könyvtárba...'
	static LangConstList countryCode;	// "en_US", "hu_HU", etc
	static LangConstList countOfImages;	// "%1 image(s) and %2 sub-album(s) in this album"
	static LangConstList falconG;		// "created by "

	static int _Read(QString name);		// defult: single language, name: "English" abbrev: "en", no icon
	static int Read();					// reads all language files from program directory
	static void Clear(int newSize = 0);
	static int Count() { return abbrev.size(); }
	static QString FileNameForLanguage(QString name, int language);	// adds _<abbrev> to base name if more than one languages
};

