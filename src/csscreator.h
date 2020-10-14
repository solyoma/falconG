#pragma once

#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "config.h"


class CssFonts
{
	friend class CssCreator;
	QString _qsGoogleFonts, _qsFontDir, _qsDefaultFonts;
public:
	CssFonts() {}
	CssFonts(QString qsGoogleFonts, QString qsFontDir, QString qsDefaultFonts) :
		_qsGoogleFonts(qsGoogleFonts), _qsFontDir(qsFontDir), _qsDefaultFonts(qsDefaultFonts)
	{
		_qsGoogleFonts.replace(',','|');
		_qsFontDir.replace(',','|');
		_qsDefaultFonts.replace(',','|');
	}
};

class CssCreator
{
	CssFonts _fonts;
	QMap<QString, QString> _cssMap;
	QFile _f;
	QTextStream _ofs;

	bool _Open(QString name);
	void _Close() { if(_f.isOpen() ) _f.close(); }
	void _SaveHeader();

	void _CssForElement(QString classStr, _CElem &elem);	 // use after class name set. does not include "first-line" pseudo class
	void _CreateFirstLineForElement(QString what, _CElem* pElem);	// creates complete class for 'what'::first-line

	void _CreateGlobals();
	void _CreateForMenuButtons();
	void _CreateForLangButton();
	void _CreateForHeader();
	void _CreateForSmallTitle();
	void _CreateForGalleryTitle();
	void _CreateForGalleryDesc();
	void _CreateForSection();
	void _CreateForImages();
	void _CreateForImageTitle();
	void _CreateForImageDesc();
	void _CreateForLightboxTitle();
	void _CreateForLightboxDescription();
	void _CreateForFooter();
	void _CreateMediaQueries();

public:
	CssCreator(CssFonts fonts) : _fonts(fonts) {}

	void Create();
	bool Save(QString cssFileName);
};

