#pragma once

#include <QString>
#include <QStringList>

#if !defined(ID_T_DEFINED)
using ID_t = int64_t;
#endif

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

//------------------------------------------
class TextMap;			// in albums.h, used in text for ID calculation

/*============================================================================
* LanguageTexts and TextMap
*	- LanguageTexts objects contains the concatenated texts for all languages
*	- Texts are represented by their ID, which may not be 0
*	- text IDs are calculated using all language texts. This is the base ID
*	- two or more texts may have the same base ID but different QStrings in
*		them (text ID collision)
*	  in which case the ID of the new text (the one that was added later on)
*		is modified using the 'collision' member, which is
*			1 + the number of the other texts with the same base ID
*	- the usage count of each text is the number of Albums and Images that
*		use it. If 'usageCount' is 0 thex is not used anywhere
*	- More than one Album or Image can use the same text, or more accurately the
*	  same text ID
*
*----------------------------------------------------------------------------*/

struct LanguageTexts				// both Album and Image uses this
{
	ID_t ID = 0;		// same for all translations of this in all languages (0: invalid)
	int collision = -1;	// (-1: not set) set when this text collided with an other one: ID = base id + (collision << ID_COLLISION_FACTOR)
	QVector<int> lenghts;	// set to have as many elements as there are languages, lang ID is index in text
	QString textsForAllLanguages; //concatenated language texts
	UsageCount usageCount; // how many times this same text is added to list

	LanguageTexts(int languageCount = 0) { Clear(languageCount); }
	LanguageTexts(const QStringList& texts);
	LanguageTexts(const LanguageTexts& t) : ID(t.ID), collision(t.collision), lenghts(t.lenghts), textsForAllLanguages(t.textsForAllLanguages), usageCount(t.usageCount) {}

	LanguageTexts& operator=(const LanguageTexts& t);
	LanguageTexts& operator=(const QStringList& txts);
	bool operator==(const LanguageTexts& t);		// compare first ID then all texts
	bool operator!=(const LanguageTexts& t);		// compare first ID then lang (unless lang < 0, when only ID is used)
	int operator<(const LanguageTexts& t);

	const QString operator[](int index) const			   // ***  can't change text using this! Use SetText() instead
	{
		static QString dummy;
		if (textsForAllLanguages.isEmpty())
			return dummy;
		if (index == 0)
			return textsForAllLanguages.left(lenghts[0]);
		return (index > 0 && index < lenghts.size()) ? textsForAllLanguages.mid(lenghts[index - 1], lenghts[index]) : dummy;
	}
	void SetTextForLanguageNoID(const QString str, int lang);		 // just set

	bool Valid() const { return ID > 0; }
	ID_t CalcBaseID();			// changes 'ID' but does not change 'collision'
	ID_t CalcID(TextMap& map);	// changes 'ID' and 'collision' unless already OK (read in)

	void Clear(int newSize = 0);  // 0: does not change size of 'lenghts'
	bool IsEmpty() { return textsForAllLanguages.isEmpty(); }
};

