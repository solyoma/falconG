#include "schemes.h"
#include "config.h"
#include <QSettings>
#include <QFile>



FalconGScheme FSchemeVector::blue("Blue:Kék",
	"#3b5876",
	"#cccccc",
	"#747474",
	"#f0f0f0",
	"#8faed2",
	"#3b589b",
	"#1c3a55",
	"#3b584a",
	"#92b1d5",
	"#999999",
	"#697a8e",
	"#12273f",
	"#555555",
	"#555555",
	"#feb60e",
	"#f0a91f",
	"#e28308",
	"#aaaaaa"
),
FSchemeVector::dark("Dark:Sötét",
	"#282828",
	"#cccccc",
	"#4d4d4d",
	"#f0f0f0",
	"#383838",
	"#9B9B9B",
	"#454545",
	"#666666",
	"#c0c0c0",
	"#999999",
	"#555555",
	"#111111",
	"#555555",
	"#555555",
	"#feb60e",
	"#f0a91f",
	"#e28308",
	"#aaaaaa"
),
FSchemeVector::black("Black:Fekete",
	"#191919",
	"#e1e1e1",
	"#323232",
	"#f0f0f0",
	"#282828",
	"#9b9b9b",
	"#323232",
	"#4a4a4a",
	"#a8a8a8",
	"#999999",
	"#191919",
	"#000000",
	"#323232",
	"#323232",
	"#feb60e",
	"#f0a91f",
	"#e28308",
	"#aaaaaa"
);

FSchemeVector schemes;		// default styles: default, system, blue, dark, black

// ================================ FSchemeVector ============================
static const QString __def = "Default:Alapértelmezett";
static const QString __blue = "Blue:Kék";
static const QString __dark = "Dark:Sötét";
static const QString __blck = "Black:Fekete";


void FSchemeVector::ReadAndSetupSchemes()
{
	bool bl = false, da = false, bk = false;	// blue, dark, black set
	if (QFile::exists(PROGRAM_CONFIG::homePath + "falconG.fsty"))
	{
		QSettings s(PROGRAM_CONFIG::homePath + "falconG.fsty", QSettings::IniFormat);	// in program directory;
		s.setIniCodec("UTF-8");
		QStringList keys = s.childGroups();	// get all top level keys with subkeys
		FalconGScheme fgst;
		for (auto k : keys)
		{
			s.beginGroup(k);
			fgst.MenuTitle = s.value("Title", "").toString();			// this will appear in the menu bar
			fgst.sBackground = s.value("Background", "").toString();
			fgst.sTextColor = s.value("TextColor", "").toString();
			fgst.sBorderColor = s.value("BorderColor", "").toString();
			fgst.sFocusedInput = s.value("FocusedInput", "").toString();
			fgst.sHoverColor = s.value("HoverColor", "").toString();
			fgst.sTabBorder = s.value("TabBorder", "").toString();
			fgst.sInputBackground = s.value("InputBackground", "").toString();
			fgst.sSelectedInputBgr = s.value("SelectedInputBgr", "").toString();
			fgst.sFocusedBorder = s.value("FocusedBorder", "").toString();
			fgst.sDisabledFg = s.value("DisabledFg", "").toString();
			fgst.sDisabledBg = s.value("DisabledBg", "").toString();
			fgst.sImageBackground = s.value("ImageBackground", "").toString();
			fgst.sPressedBg = s.value("PressedBg", "").toString();
			fgst.sDefaultBg = s.value("DefaultBg", "").toString();
			fgst.sProgressBarChunk = s.value("ProgressBarChunk", "").toString();
			fgst.sWarningColor = s.value("WarningColor", "").toString();
			fgst.sBoldTitleColor = s.value("BoldTitleColor", "").toString();
			fgst.sSpacerColor = s.value("SpacerColor", "").toString();
			s.endGroup();
			push_back(fgst);
			if (fgst.MenuTitle == __blue)
				bl = true;
			if (fgst.MenuTitle == __dark)
				da = true;
			if (fgst.MenuTitle == __blck)
				bk = true;
		}
	}
	// set defaults if not set already
	if (!bl)
		push_back(blue);
	if (!da)
		push_back(dark);
	if (!bk)
		push_back(black);
}

void FSchemeVector::Save()
{
	if (QFile::exists(PROGRAM_CONFIG::homePath + "falconG.fsty"))
		QFile::remove(PROGRAM_CONFIG::homePath + "falconG.fsty");

	QSettings s(PROGRAM_CONFIG::homePath + "/falconG.fsty", QSettings::IniFormat);	// in program directory;
	for (int i = 2; i < size(); ++i)	// do not save default and system
	{
		char st[] = "0"; st[0] += i - 1;
		FalconGScheme& fgst = operator[](i);
		s.beginGroup(st);
		s.setValue("Title", fgst.MenuTitle);			// this will appear in the menu bar
		s.setValue("Background", fgst.sBackground);
		s.setValue("TextColor", fgst.sTextColor);
		s.setValue("BorderColor", fgst.sBorderColor);
		s.setValue("FocusedInput", fgst.sFocusedInput);
		s.setValue("HoverColor", fgst.sHoverColor);
		s.setValue("TabBorder", fgst.sTabBorder);
		s.setValue("InputBackground", fgst.sInputBackground);
		s.setValue("SelectedInputBgr", fgst.sSelectedInputBgr);
		s.setValue("FocusedBorder", fgst.sFocusedBorder);
		s.setValue("DisabledFg", fgst.sDisabledFg);
		s.setValue("DisabledBg", fgst.sDisabledBg);
		s.setValue("ImageBackground", fgst.sImageBackground);
		s.setValue("PressedBg", fgst.sPressedBg);
		s.setValue("DefaultBg", fgst.sDefaultBg);
		s.setValue("ProgressBarChunk", fgst.sProgressBarChunk);
		s.setValue("WarningColor", fgst.sWarningColor);
		s.setValue("BoldTitleColor", fgst.sBoldTitleColor);
		s.endGroup();
	}
}

int FSchemeVector::IndexOf(FalconGScheme& fgst)
{
	return IndexOf(fgst.MenuTitle);
}

int FSchemeVector::IndexOf(const QString& title)
{
	for (int i = 2; i < size(); ++i)
		if (operator[](i).MenuTitleForLanguage(PROGRAM_CONFIG::lang) == title)
			return i;
	return -1;
}