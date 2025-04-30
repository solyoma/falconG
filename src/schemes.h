#pragma once
#ifndef _SCHEMES_H
#define _SCHEMES_H

#include <QObject>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QStringList>

struct FalconGScheme
{
	QString
		MenuTitle,			// this will appear in the menu bar	it may contain a series of titles for all languages separated by commas
		sBackground,
		sTextColor,
		sBorderColor,
		sFocusedInput,
		sHoverColor,
		sTabBorder,				 
		sInputBackground,
		sSelectedInputBgr,
		sFocusedBorder,
		sDisabledFg,
		sDisabledBg,
		sImageBackground,
		sPressedBg,
		sDefaultBg,
		sProgressBarChunk,
		sWarningColor,
		sBoldTitleColor,
		sSelectionColor,
		sSelectionBackground,
		sSpacerColor;		// for drag & drop in tnvImages

	QString MenuTitleForLanguage(int lang)
	{
		if (MenuTitle.isEmpty())	// system colors
			MenuTitle = QString("Default:Alapértelmezett");		// modify when new user interface language added!
		QStringList sl = MenuTitle.split(':');
		if (lang < 0 || lang >= sl.size())
			lang = 0;
		return sl[lang];
	}

	QString& operator[](int index)
	{
		switch (index)
		{
		case 0:		return sBackground;
		case 1:		return sTextColor;
		case 2:		return sBorderColor;
		case 3:		return sFocusedInput;
		case 4:		return sHoverColor;
		case 5:		return sTabBorder;
		case 6:		return sInputBackground;
		case 7:		return sSelectedInputBgr;
		case 8:		return sFocusedBorder;
		case 9:		return sDisabledFg;
		case 10:	return sDisabledBg;
		case 11:	return sImageBackground;
		case 12:	return sPressedBg;
		case 13:	return sDefaultBg;
		case 14:	return sProgressBarChunk;
		case 15:	return sWarningColor;
		case 16:	return sBoldTitleColor;
		default:
		case 17:	return sSpacerColor;
		}
	}

	FalconGScheme()
	{
	}

	FalconGScheme(const char* t,	 // menu title
		const char* c0,	 // sBacground
		const char* c1,	 // sTextColor	-	foreground
		const char* c2,	 // sBorderColor
		const char* c3,	 // sFocusedInput
		const char* c4,	 // sHoverColor
		const char* c5,	 // sTabBorder
		const char* c6,	 // sInputBackground - editor backgrounds
		const char* c7,	 // sSelectedInputBgr
		const char* c8,	 // sFocusedBorder
		const char* c9,	 // sDisabledFg
		const char* c10, // sDisabledBg
		const char* c11, // sImageBackground
		const char* c12, // sPressedBg	-	button pressed
		const char* c13, // sDefaultBg
		const char* c14, // sProgressBarChunk
		const char* c15, // sWarningColor
		const char* c16, // sBoldTitleColo
		const char* c17, // SelectionColor
		const char* c18, // SelectionBackground

		const char* c19	 //sSpacerColor - spacer for drop operations
	)
	{
		MenuTitle = t;			// this will appear in the menu bar
		sBackground = c0;
		sTextColor = c1;
		sBorderColor = c2;
		sFocusedInput = c3;
		sHoverColor = c4;
		sTabBorder = c5;
		sInputBackground = c6;
		sSelectedInputBgr = c7;
		sFocusedBorder = c8;
		sDisabledFg = c9;
		sDisabledBg = c10;
		sImageBackground = c11;
		sPressedBg = c12;
		sDefaultBg = c13;
		sProgressBarChunk = c14;
		sWarningColor = c15;
		sBoldTitleColor = c16;
		sSelectionColor = c17;
		sSelectionBackground = c18;

		sSpacerColor = c19;
	}

};


/*========================================================
 * Style sheets (skins) for falconG
 * REMARKS: - default styles (default, system, blue, dark, black)
 *				are always present
 *			- styles are read from falconG.sty in program
 *				directory
 *			- if a style there has the same Title as any
 *				of the last 3 of the default they will be
 *				overwritten
 *			- styles default and system are always the first
 *				2 styles used, and if not redefined then
 *				blue, dark and black are the last ones
 *			- if a style requested which is no longer
 *				present, the 'default' style is used instead
 *-------------------------------------------------------*/
class FSchemeVector : public  QVector<FalconGScheme>
{
	static FalconGScheme blue, dark, black;
public:
	bool wasSaved = false; // set from outside

	FSchemeVector()
	{
		reserve(5);		// for default, system, blue, dark, black 
		resize(2);		// default and system
		operator[](0).MenuTitle = QMainWindow::tr("Default");
		operator[](1).MenuTitle = QMainWindow::tr("System Colors");
		operator[](1).sBorderColor = "#747474";
	}
	void ReadAndSetupSchemes();	// into menu items
	void Save();			// into "falconG.fsty"
	void push_back(const FalconGScheme& scheme)
	{
		wasSaved = false;
		QVector<FalconGScheme>::push_back(scheme);
	}
	int IndexOf(FalconGScheme& fgst);
	int IndexOf(const QString& title);
};

extern FSchemeVector schemes;		// default styles: default, system, blue, dark, black

#endif
