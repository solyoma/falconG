#pragma once

#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "config.h"

class CssCreator
{
	QFile _f;
	QTextStream _ofs;
	bool _forFalconG;

	bool _Open(QString name);
	void _Close() { if(_f.isOpen() ) _f.close(); }

	void _CssForElement(QString classStr, _CElem &elem);	 // use after class name set. does not include "first-line" pseudo class

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

	void _Create();

public:
	CssCreator() {}

	bool Create(QString cssFileName, bool forFalconG = false);
};

