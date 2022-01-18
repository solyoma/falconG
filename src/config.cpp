#include "config.h"

// helper	prepends a tab and adds a semicolon and LF after the string
// if s has more than one property (each should be in separate lines
// semicolons must already be present
void __AddSemi(QString& s, bool addSemicolon)
{
	if (!s.isEmpty())
	{
		if (addSemicolon && s.length() && s[s.length()-1] != QChar('\n'))
			s = "	" + s + ";";
		if (s.length() && s[s.length() - 1] != QChar('\n'))
			s += "\n";
	}
}


QString PROGRAM_CONFIG::homePath;
QString PROGRAM_CONFIG::samplePath;

int PROGRAM_CONFIG::maxSavedConfigs;		// last 10 configuration directory is stored
QStringList PROGRAM_CONFIG::lastConfigs;

int PROGRAM_CONFIG::indexOfLastUsed;		// this was the last one used
CONFIG *PROGRAM_CONFIG::parent = nullptr;

int PROGRAM_CONFIG::lang = -1;
QStringList PROGRAM_CONFIG::qslLangNames;
int PROGRAM_CONFIG::splitterLeft = 493;
int PROGRAM_CONFIG::splitterRight = 543;
int PROGRAM_CONFIG::schemeIndex = 0;

static bool __bClearChangedFlag = false;	// set to true to clear the changed flag after writing the configuration

					// global variables for configuration 
CONFIG config,		// must be here because _Changed uses it
	   configSave;

/*========================== CONFIG_USED ====================*/
void PROGRAM_CONFIG::Read()
{
	lastConfigs.clear();
	QSettings s(homePath+falconG_ini, QSettings::IniFormat);	// in user's local home directory

	lang = s.value("lang", -1).toInt();
	splitterLeft = s.value("sll", 493).toInt();
	splitterRight = s.value("slr", 543).toInt();
	schemeIndex = s.value("schemeIndex", 0).toInt();
	if (schemeIndex < 0) 
		schemeIndex = 0;

	s.beginGroup("config_save"); //--------------------------------

	maxSavedConfigs = s.value("maxSaveConfigs", 10).toInt();
	int numSaveConfigs = s.value("numSaveConfigs", 0).toInt();
	indexOfLastUsed = s.value("indexOfLastUsed", -1).toInt();
	QString str;
	for (int i = 0; i < numSaveConfigs; ++i)
	{
		str = s.value(QString().setNum(i), "").toString();
		if (!str.trimmed().isEmpty())
		{
#ifdef _MSC_VER
//			str = str.toLower();	// Windows: lC UC letters are the same in paths!
#endif
			lastConfigs.push_back(str);
		}
	}

	s.endGroup();				//------------------------------------
}

void PROGRAM_CONFIG::Write()
{
	QString sLast = parent->dsSrc.v.trimmed();
	if (sLast.isEmpty())		
		PROGRAM_CONFIG::indexOfLastUsed = -1;
	else
	{
		if (PROGRAM_CONFIG::indexOfLastUsed < 0)	// then search list for last item
		{
			int n;
			if ((n = PROGRAM_CONFIG::lastConfigs.indexOf(sLast, 0)) < 0)
			{
				if (PROGRAM_CONFIG::lastConfigs.size() == PROGRAM_CONFIG::maxSavedConfigs)
					PROGRAM_CONFIG::lastConfigs.removeLast();

				PROGRAM_CONFIG::lastConfigs.insert(0, sLast);
				PROGRAM_CONFIG::indexOfLastUsed = 0;
			}
			else
				PROGRAM_CONFIG::indexOfLastUsed = n;
		}
	}

	QSettings s(homePath+falconG_ini, QSettings::IniFormat);	// in program directory
	s.setValue("lang", PROGRAM_CONFIG::lang);
	s.setValue("schemeIndex", schemeIndex);
	s.setValue("sll", splitterLeft);
	s.setValue("slr", splitterRight);

	s.beginGroup("config_save");
		s.setValue("maxSaveConfigs", maxSavedConfigs);
		s.setValue("numSaveConfigs", lastConfigs.size());
		s.setValue("indexOfLastUsed", indexOfLastUsed);
		for (int i = 0; i < lastConfigs.size(); ++i)
			s.setValue(QString().setNum(i), lastConfigs.at(i));
	s.endGroup();
}


void PROGRAM_CONFIG::GetHomePath()
{
	homePath = QDir::homePath() +
#if defined (Q_OS_Linux)   || defined (Q_OS_Darwin) || defined(__linux__)
		"/.falconG/";
#elif defined(Q_OS_WIN)
		"/AppData/Local/FalconG/";
#endif
	if (!QDir(homePath).exists())
		QDir(homePath).mkdir(homePath);
			samplePath = homePath+"sample/";
}

/*========================================================
 * TASK: Determines file name of configuration for ini
 *		 or struct files
 *		 	
 * PARAMS:	forSave - name required for save or read
 *				for read: if file in source directory
 *							does not exist return default name
 *				for save: always returns a name in the source
 *							directory
 *			sExt - extension (.ini or .struct)
 *					defaults: 'falconG.ini' and 'gallery.struct'
 * GLOBALS: lastConfigs, indexOfLastUsed
 * RETURNS:	full path name of config file even if it doesn't exist
 * REMARKS: - call only when 'lastConfigs' and 'indexOfLastConfig'
 *				are correct
 *			- Config file name is the directory name
 *				followed by the extension sExt inside
 *				the original directory
 *				Example: directory name: '/in/this/dir/'
 *				File name '/in/this/dir/dir.ini'
 *-------------------------------------------------------*/
QString PROGRAM_CONFIG::NameForConfig(bool forSave, QString sExt)
{
	QString sDefault = homePath;	// set before calling this in Read()
	if (sExt == ".ini")
		sDefault += falconG_ini;
	else
		sDefault += "gallery.struct";

	QString sIniName, p, n, s;				  // if there was a last used directory read from it

	if (lastConfigs.isEmpty() || indexOfLastUsed < 0)
		return sDefault;

	s = lastConfigs[indexOfLastUsed];

	SeparateFileNamePath(s, p, n);	// cuts '/' from name
	if (s[s.length() - 1] != '/')
		s += "/";
	if (n.isEmpty())
		sIniName = sDefault;		 //	 'falconG.ini' (in user's folder)
	else
		sIniName = s + n + sExt;	 // = <path w.o. last folder name from 'falcong.ini'>/<folder>/<folder>.ini

	if(!forSave && !QFile::exists(sIniName))	 
	{
		QString sDefIniName = sDefault;
		if (QFile::exists(sDefIniName))
			return sDefIniName;
	}

	return sIniName;
}


/*========================================================
 * TASK:	generate string for delimited language names
 *			used for language translations for the
 *			program interface
 * PARAMS:
 * GLOBALS:
 * RETURNS: example: en_US:hu_HU
 * REMARKS: - only American English and Hungarian are used yet
 *-------------------------------------------------------*/
QString PROGRAM_CONFIG::LangNameListWDelim()
{
	QString qs = PROGRAM_CONFIG::qslLangNames[0].left(PROGRAM_CONFIG::qslLangNames[0].lastIndexOf('.'));
	for (int i = 1; i < PROGRAM_CONFIG::qslLangNames.size(); ++i)
		qs += ":" + PROGRAM_CONFIG::qslLangNames[i].left(PROGRAM_CONFIG::qslLangNames[i].lastIndexOf('.'));

	return qs;
}

void PROGRAM_CONFIG::GetTranslations()
{
	QDir dir(":/translations/translations");
	qslLangNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
	qslLangNames.sort();
}

void PROGRAM_CONFIG::MakeValidLastConfig()
{
	int i;
	QString s = config.dsSrc.ToString();
	i = lastConfigs.indexOf(s);
	if(i < 0)
	{
		indexOfLastUsed = lastConfigs.size();
		lastConfigs.append(s);
	}
	indexOfLastUsed = i;
}


/*===========================================================================
 * TASK: assign a QString to a CString
 * EXPECTS: s - QString
 * GLOBALS: config
 * RETURNS: the QString
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *--------------------------------------------------------------------------*/
QString &_CString::operator=(const QString s) 
{ 
	if (v != s) 
		v = s; 
	return v; 
}

/*===========================================================================
* TASK: assig a QString to a _CDirStr
* EXPECTS: s - QString
* GLOBALS: config
* RETURNS: the QString
* REMARKS:	- may modify the 'changed' field of global variable 'config'
*			- the stored string always ends with a '/'
*--------------------------------------------------------------------------*/
QString &_CDirStr::operator=(const QString s)
{
	 QString qs = QDir::cleanPath(s);
	 qs = AddSep(qs);
	 if (v == qs )
		 return v;

	 v = qs;
	return v;
}

/*===========================================================================
* TASK: Concatenate a subdirectory to this directory, unless it has an absolute path
* EXPECTS: subdir - absolute or relative directory name
* GLOBALS: config
* RETURNS: the result
* REMARKS: JAlbum quirk: if a link is relative to the JAlbuum root dir. the path to the
*		file name starts with a '/' instead of the full path. Therefore 
*		when config.bSourceRelativePerSign is set such paths are considered relative
*--------------------------------------------------------------------------*/
_CDirStr _CDirStr::operator+(const _CDirStr &subdir)
{
	QString dir = ToString(), 
			sub = subdir.ToString();
	if(dir.isEmpty() || dir[dir.length()-1] != '/')
		dir += '/';
	if( (sub.length() > 2 && sub[1] == ':') || !config.bSourceRelativePerSign) 
	{
		if (QDir::isAbsolutePath(sub))	// then it cannot be my sub directory
			return *const_cast<_CDirStr *>(&subdir);	// so the result will be it
	}
	if(!sub.isEmpty() && sub[0] == '/')
		sub = sub.mid(1);						// cut '/' from beginning

	sub = QDir::cleanPath(dir + sub);	// get rid of ./ ../, etc
												// but also ending '/'
	_CDirStr tmp;
	tmp.v = sub + '/';						// add back the '/'
	return tmp;
}

/*===========================================================================
* TASK: Concatenate a subdirectory to this directory, unless it has an absolute path
* EXPECTS: subdir - absolute or relative directory name
* GLOBALS: config.h
* RETURNS: the result
* REMARKS:
*--------------------------------------------------------------------------*/
_CDirStr _CDirStr::operator+(const QString subdir)
{
	_CDirStr path;
	path = subdir;
	return (*this) + path;
}

/*===========================================================================
 * TASK: assig a bool to a CBOOL
 * EXPECTS: s - bool
 * GLOBALS: config
 * RETURNS: the bool
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
//bool &_CBool::operator=(const bool s)
//{ 
//	return v = s; 
//}

/*===========================================================================
 * TASK: assign an into a CInt
 * EXPECTS: i - int
 * GLOBALS: config
 * RETURNS: the int
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
//int &_CInt::operator=(const int i)
//{
//	return v = i;
//}
// --------------------------- _CColor ------------------------------

/*===========================================================================
 * TASK: assign a QString to a _CColor and modify opacity
 * EXPECTS: str - QString: formats RGB or RRGGBB or #RGB or #RRGGBB, where
 *				 R,G,B are any hexadecimaL digit
 *		   opac - opacity in percent 0..100 or -1: no changes
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS:	- does not change if opacity is used
 *			- if string name is invalid does not set it
 *			- no symbolic color names
 *			- any combination of upper and lower case hexadecimal numbers
 *			- if the QString is #xyz, then it is the same as #xxyyzz
 *--------------------------------------------------------------------------*/
void _CColor::Set(QString str, int opac)
{ 
	if (!_ColorStringValid(str))
		return;

	_colorName = str; 
//	_opacityUsed = opac < 0;
	if(opac > 0)
		_opacity = opac;
	_Prepare();		// setup 'v'
}

QString _CColor::ForStyleSheet(bool addSemiColon, bool isBackground) const
{
	QString qs = (isBackground ? "background-color:" : "color:");
	
	if (_opacityUsed && _opacity != 255)
		qs += ToRgba();
	else
		qs += _colorName;

	__AddSemi(qs, addSemiColon);
	return qs;
}

/*===========================================================================
 * TASK: assign a QString to a CColor but do not modify opacity
 * EXPECTS: s - QString: formats xxx or xxxxxx or #xxx or #xxxxxx, where all
 *				 x's are any hexadecimaL DIGIT
 * GLOBALS: config
 * RETURNS: color QString: always starts with a '#'
 * REMARKS:	- if color is invalid does not change it
 *			- no symbolic color names
 *			- any combination of upper and lower case hexadecimal numbers
 *			- if the QString is #xyz, then it is the same as #xxyyzz
*--------------------------------------------------------------------------*/
QString _CColor::operator=(QString s)
{
	if (!_ColorStringValid(s))
		return QString();
	// here s starts with '#'
	_colorName = s.toLower();
	_Prepare();

	return _colorName;
}
_CColor& _CColor::operator=(_CColor c)
{
	v = c.v; v0 = c.v0;
	_Setup();
	return *this;
}
bool _CColor::operator==(const _CColor& o)
{
	bool res = _colorName == o._colorName;
	if (_opacity >= 0 && o._opacity >= 0)
		res &= _opacity == o._opacity;
	return res;
}
bool _CColor::operator!=(const _CColor& o)
{
	return !operator==(o);
}
/*===========================================================================
 * TASK: drop the leading '# from the name
 * EXPECTS: nothing
 * GLOBALS: -
 * RETURNS: name without the leading '#'
 * REMARKS: 
 *--------------------------------------------------------------------------*/
//_CColor::operator QString() const { return _colorName.mid(1); }	// drop the '#'

/*========================================================
 * TASK:	checks wether s is a valid color number string
 * PARAMS:	s color string
 * GLOBALS:
 * RETURNS: true or false
 * REMARKS: - accepted formats (x: lower or upper case 
 *				hexdecimal digit): xxx, #xxx, xxxxxx,#xxxxxx
 *			after it returns the string always starts with '#'
 *-------------------------------------------------------*/
bool _CColor::_ColorStringValid(QString &s)
{
	s = s.toLower();
	if (s.length() > 0 && s[0] != '#')
		s = QChar('#') + s;

	if (s.length() != 4 && s.length() != 7 && s.length() != 9 && s.length() != 10)	// #RGB, #RRGGBB, #AARRGGBB or #AARRGGBB-
		return false;
	char len = s.length() == 10 ? 9 : s.length(),
		 n;
	for (int i = 1; i < len; ++i)
	{
		n = s[i].unicode();
		if (n < '0' || (n > '9' && (n < 'a' || n > 'f')))
			return false;
	}
	return true;
}

// --------------------------- _CTextDecoration ------------------------------

QString _CTextDecoration::ForStyleSheet(bool addSemiColon) const
{
	QString qs, qsTmp;
	auto addProp = [&](QString name, QString value)
	{
		if (!value.isEmpty())
		{
			qsTmp = name + ":" + value;
			__AddSemi(qsTmp, addSemiColon);
			qs += qsTmp;
		}
	};
	addProp("text-decoration-line", TextDecorationLineStr());
	if(v != tdNone)
		addProp("text-decoration-style", TextDecorationStyleStr());
	return QString();
}

// --------------------------- _CFont ------------------------------

QString _CFont::ForStyleSheet(bool addSemiColon) const
{
	QString qs, qsTmp;
	auto addProp = [&](QString name, QString value)
	{
		if (!value.isEmpty())
		{
			qsTmp = name + ":" + value;
			__AddSemi(qsTmp, addSemiColon);
			qs += qsTmp;
		}
	};
	addProp("font-family", Family());
	addProp("font-size", SizeStr());
	addProp("line-height", LineHeightStr());
	if (Bold())
		addProp("font-weight", WeightStr());
	if (Italic())
		addProp("font-style", "italic");

	return qs;
}

/*===========================================================================
 * TASK: setting font parameters
 * EXPECTS: fam - font family name(s). if more than one separated by commas 
 *				 and if any contains a space it must be quoted
 *		   siz - font size QString with units. E.g. 12pt or 20px
 *		   feat - ORed font feature flag bits (bold, etc, see 'feature' enum)
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *--------------------------------------------------------------------------*/
void _CFont::Set(QString fam, QString siz, QString slh, int feat, QString sFsSize)
{
	_details.clear();
	_details.push_back(fam);
	_details.push_back(siz);
	_details.push_back(slh);
	_details.push_back(QString().setNum(feat) );
	_details.push_back("0");			// default: no different first line
	_details.push_back(_details[fSiz]);	// first line same as others
	if(!sFsSize.isEmpty())	
	{
		_details[fDiffF] = "1";	// different first line
		_details[fFirstS] = sFsSize;
	}

	_Prepare();
}
void _CFont::SetFamily(QString fam)
{
	_details[fFam] = fam;
	_Prepare();
}

void _CFont::SetFeature(Style feat, FeatureOp op)
{
	int fs = _details[fFeat].toInt(),
		f = (int) feat;

	if (op == foClearAll || op == foClearOthersAndSet)
		fs = 0;
	else if (op == foUnset)
		fs &= ~f;
	if (op == foSet)
		fs |= f;

	_details[fFeat] = QString().setNum(fs);
	_Prepare();
}
void _CFont::SetFeature(Style feat, bool on)
{
	SetFeature(feat, on ? foSet: foUnset);
}

void _CFont::SetSize(int pt)
{
	SetSize(QString().setNum(pt));
}

void _CFont::SetSize(QString s)
{
	_details[fSiz] = s;
	_Prepare();
}

void _CFont::SetLineHeight(QString qslh)
{
	_details[fLineH] = qslh;
	_Prepare();
}

void _CFont::SetDifferentFirstLine(bool set, QString size)
{
	_details[4] = set ? "1" : "0";
	if (!size.isEmpty())
		_details[fFirstS] = size;
}

void _CFont::_Prepare()
{				  // only add first line size if used or different from font size
	int n = _details[fDiffF] == "1" && _details[fSiz] != _details[fFirstS] ? 6 : 4;
	v = _details[fFam];
	for (int i = 1; i < n; ++i)
		v += "|" + _details[i];
}

void _CFont::_Setup()
{
	_details = v.split(QChar('|'));		   
	switch (_details.size())		// No breaks!
	{											// element 0 is family
		case 1: _details.push_back("10pt");	  	// font-size string with unit
		case 2: _details.push_back(_details[1]);	// line-height default: same as font-size
		case 3: _details.push_back("0");	  	// style (bold:'1', italic:'2', bold & italic: '3')
		case 4: _details.push_back("0");	  	// first line is different?
		case 5: _details.push_back("10pt");	  	// font-size::first-line
		default:
			break;
	}
	_Prepare();
}

/*===========================================================================
 * TASK: rmove all features for font
 * EXPECTS: none
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
void _CFont::ClearFeatures()
{ 
	_details.pop_back();
	_Prepare();
}

//------------------------------------- _CShadow ------------------------------


/*========================================================
 * TASK:	sets up _details from v
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - text shadow does not have a spread, but it 
 *				still must be given in string if color is set
 *-------------------------------------------------------*/
// format: 
// <use>|<horiz>|<vert>[|<blur>][|spread][|color name starts with '#'>]
void _CShadow::_Setup()		
{							
	_details = v.split(QChar('|'));		// details size is always 6 even when no blur, spread or color is given

	switch (_details.size())			// no 'breaks'!
	{
		case 3:
			_details.push_back("0");	// blur radius
		case 4:
			_details.push_back("0");	// shadow spread
		case 5:
			_details.push_back("#000");	// color
		default:
			break;
	}
	_Prepare();
//	v0 = v;
}

void _CShadow::_Prepare()	// assumes: always 6 elements
{
	v.clear();
	QStringList sl = _details;
	if (sl[5] == "#000")		// no color was given
	{
		sl.pop_back();		
		if (sl.at(4) == '0')	// no spread
		{
			sl.pop_back();
			if (sl.at(3) == '0')	// no blur
				sl.pop_back();
		}
	}
	v = _details.join("|");
}

/*===========================================================================
 * TASK: set top,right,bttom or left shadow
 * EXPECTS: which - what part to set
 *		    val   - parameter of shadow in pixels or color as integer
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS:	synchronize with ShadowPart in enum.h !
 --------------------------------------------------------------------------*/
void _CShadow::Set(ShadowPart which, int val)	// which: 1: horiz, 2: vert, 3:blur, 4: spread !
{
	if(which == spColorName)
		_details[5] = QString("#%1").arg(_CInt::IntToHexString(val, 6));
	else
		_details[which] = QString().setNum(val);

	_Prepare();
}
/*===========================================================================
 * TASK: set all of the shadows
 * EXPECTS: top,ight,bottom,left - size of shadow in pixels
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
void _CShadow::Set(int horiz, int vert, int blur, int spread, QString clr, bool used)
{
	QString oldV = v;
	_details.clear();
	_details.push_back(used ? "1" : "0");
	_details.push_back(QString().setNum(horiz));
	_details.push_back(QString().setNum(vert));
	_details.push_back(QString().setNum(blur));
	_details.push_back(QString().setNum(spread));
	if (clr.isEmpty())
		clr = "#000";
	_details.push_back(clr);

	_Prepare();	// new v
}

QString _CShadow::ForStyleSheet(bool addSemiColon, bool first_line, int which) const
{
	{
		if (!IsSet() || !Used())
			return QString();

		QString res;
		if (first_line)
			res = which ? "text-shadow:" : "box-shadow:";
		else
			res = ", ";
		res += _details[1] + "px " + _details[2] + "px";
		if (which)			// box shadow has optional 'spread', text shadow doesn't
		{
			if (_details.at(4) != '0')
				res += " " + _details[3] + "px" + " " + _details[4] + "px";
		}
		else if(_details.at(3) != '0')
				res += " " + _details[3] + "px" + " " + _details[4] + "px";
			// always add color as 'Safari' on PC will not display the shadow otherwise (2020.11.16.)
		res += " " + _details[5];
		__AddSemi(res, addSemiColon);

		return res;
	}
}

/*===========================================================================
 * TASK:  set parameters of a gradient stop (may be vertical or horizontal)
 * EXPECTS:	pc - coordinate in percent relative to top or left
 *			clr - color QString for stop, format is #xxx or #xxxxxx
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
bool _CGradientStop::Set(int pc, QString clr) 
{ 
	percent = pc; 
	color = clr;
	return true; 
}


/*===========================================================================
 * TASK:  set parameters of a vertical gradient stop 
 * EXPECTS:	which - gsStart, gsMiddle, gsStop (0,1,2) - which of the 3 stops
 *			pc - coordinate in percent relative to top
 *			clr - color QString for stop, format is #xxx or #xxxxxx
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *--------------------------------------------------------------------------*/
void _CGradient::Set(GradStop which, int pc, QString clr) 
{ 
	gs[which].Set(pc, clr); 
	if (!_internal)
		_Prepare();
}

/*===========================================================================
 * TASK:  set parameters of all vertical gradient stops in a vertical gradient
 * EXPECTS:	topStop,middleStop and bottomStop - coordinate in percent relative to top
 *			topColor, middleColor and bottomColor - color QString for stop, format is #xxx or #xxxxxx
 *			used: this must be used (if used ever...)
 *			chg: is the value changed
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *--------------------------------------------------------------------------*/
void _CGradient::Set(bool usd, int topStop, QString topColor, int middleStop, QString middleColor, int bottomStop, QString bottomColor, bool chg)
{
	gs[0].Set(topStop, topColor);
	gs[1].Set(middleStop, middleColor);
	gs[2].Set(bottomStop, bottomColor);

	if (!_internal)
		_Prepare();
}

QString _CGradient::ForStyleSheet(bool semi) const
{
	QString qs;
	if (!used) 
		qs = QString("background-image:none");
	else
		qs = QString("background-image:linear-gradient(%1 %2%,%3 %4%,%5 %6%)")
								.arg(gs[0].color).arg(gs[0].percent)
								.arg(gs[1].color).arg(gs[1].percent)
								.arg(gs[2].color).arg(gs[2].percent);
	__AddSemi(qs, semi);
	return qs;
}

QString _CGradient::ForQtStyleSheet(bool invert) const
{
	int n0, n1, n2;
	if (invert)
		n0 = 2, n1 = 1, n2 = 0;
	else
		n0 = 0, n1 = 1, n2 = 2;
	return QString("qlineargradient(x1:0, y1:0, x2:0,y2:1,stop:%1 %2,Stop:%3 %4, stop:%5 %6)")
								.arg(gs[n0].percent/100.0).arg(gs[n0].color)
								.arg(gs[n1].percent/100.0).arg(gs[n1].color)
								.arg(gs[n2].percent/100.0).arg(gs[n2].color);

}

void _CGradient::_Setup()	// what was read from settings into v
{
	QStringList qsl = v.split(QChar('|'));		//	<used>|<p Top>|<c Top>|<p Middle>|<c Middle>|<p Bottom>|<c bottom>
												// u: used 0,1, p = position (%), c = color

	if (qsl.size() != 7)		// not set
	{
		v = vd = "0|0|#A90329|40|#890222|100|#6d0019";		// default
		qsl = v.split(QChar('|'));
	}
							// index in qsl		  0			    1			   2			  3			        4				   5				6				
	_internal = true;
	used = qsl[0].toInt();
	for (int i = 1; i < qsl.size(); i += 2)
	{
		if (i + 1 < qsl.size())
			Set( (i-1)/2, qsl[i].toInt(), qsl[i + 1]);		// (i-1)/2 = 0,1,2
		else
			Set( (i-1)/2, 0, "#000000");
	}
	_internal = false;
}

void _CGradient::_Prepare()		// to store in settings
{
	v = QString("%1|%2|%3|%4|%5|%6|%7").arg(used)
									.arg(gs[0].percent).arg(gs[0].color)
									.arg(gs[1].percent).arg(gs[1].color)
									.arg(gs[2].percent).arg(gs[2].color);
}


// ------------------------------------- _CBorder ------------------------------------------------



/*========================================================
 * TASK:	Sets border styles
 * PARAMS:	semi: if set prepends a TAB and appends a semicolon
 *				after each line
 * GLOBALS:
 * RETURNS:
 * REMARKS: - returns border-top:, border-right, border-bottom
 *				border-left and border-radius
 *			- can't use different colors for 
 *				each side
 *			- may not use the shorthand as it only works if no
 *			  radius is given
 *-------------------------------------------------------*/
QString _CBorder::ForStyleSheet(bool semi) const		// w. radius
{
	if (!UsedSides())
	{
		QString qs = "border:none";
		__AddSemi(qs,semi);
		return qs;
	}

	QString res,res2,res3,res4;
	res = QString("border-top:%1px %2 %3").arg(_widths[sdTop]).arg(StyleStr(sdTop)).arg(ColorStr(sdTop));
	res2 = QString("border-right:%1px %2 %3").arg(_widths[sdRight]).arg(StyleStr(sdRight)).arg(ColorStr(sdRight));
	res3 = QString("border-bottom:%1px %2 %3").arg(_widths[sdBottom]).arg(StyleStr(sdBottom)).arg(ColorStr(sdBottom));
	res4 = QString("border-left:%1px %2 %3").arg(_widths[sdLeft]).arg(StyleStr(sdLeft)).arg(ColorStr(sdLeft));
	__AddSemi(res , semi);
	__AddSemi(res2, semi);
	__AddSemi(res3, semi);
	__AddSemi(res4, semi);
	res += res2 + res3 + res4;
		
	if (_radius)
	{
		res2 = QString("border-radius:%1px").arg(config.Menu.border.Radius());
		__AddSemi(res2, semi);
		res += res2;
	}

	return res;
}

QString _CBorder::ForStyleSheetShort(bool semicolonAtLineEnds) const
{
	QString res;
	if (!UsedSides())
		res = "border:none";
	else
	{
		if (_sizeWidths > 1)
			return ForStyleSheet(semicolonAtLineEnds);
		res = QString("border:%1px %2 %3").arg(_widths[0]).arg(StyleStr(sdAll)).arg(_colorNames[0]);
	}
	__AddSemi(res, semicolonAtLineEnds);
	return res;
}

void _CBorder::_CountWidths()
{
	_sizeWidths = 1;
	bool
		b12 = (_widths[0] == _widths[1]) && (_styleIndex[0] == _styleIndex[2]) && (_colorNames[0] == _colorNames[1]),				// top = right?
		b13 = (_widths[0] == _widths[2]) && (_styleIndex[0] == _styleIndex[2]) && (_colorNames[0] == _colorNames[2]),				// top = bottom?
		b24 = (_widths[1] == _widths[3]) && (_styleIndex[1] == _styleIndex[3]) && (_colorNames[1] == _colorNames[3]);				// right = left?
	if (b13 && b24)					// then either just top-left and bottom-right is given
		if (b12)					// or all equal
			_sizeWidths = 1;	// 1 width, style and color
		else
			_sizeWidths = 2;	// 2 widths, styles and colors
	else
		_sizeWidths = 4;		// 4 widths, styles and colors
}

void _CBorder::_Setup()
{
	QStringList qsl = v.split(QChar('|'));
	int siz = qsl.size();	// 13 or 14, 8 or 9, 4 or 5
	if (siz != 13 && siz != 14 && siz != 7 && siz != 8 && siz != 4 && siz != 5)		// bad string: use defaults
	{
		qsl.clear();
		qsl << "0" << "0" << "0" << "0" << "#000000";
	}
	// create for any of these: (<used> = 0..15)

	// siz == 4    0      1     2     3
	//			<used>|width|style|color
	// siz == 5    0      1		2	  3		4
	//			<used>|width|style|color|radius
	// siz == 7	   0			1					 2					 3					  4			 		 5					 6
	//			<used>|width top & bottom|width right and Left| style top & bottom|style right & left|color top & bottom|color right & left
	// siz == 8	   0			1					 2					 3					  4			 		 5					 6		   7
	//			<used>|width top & bottom|width right and Left| style top & bottom|style right & left|color top & bottom|color right & left|radius
	// siz == 13   0		1		   2			3			4		   5		 6		     7			  8			 9		  10		  11		   12
	//			<used>|width top|width right|width bottom|width Left|style top|style right|style bottom|style Left|color top|color right|color bottom|color Left|
	// siz == 14   0		1		   2			3			4		   5		 6		     7			  8			 9		  10		  11		   12       13
	//			<used>|width top|width right|width bottom|width Left|style top|style right|style bottom|style Left|color top|color right|color bottom|color Left|radius

		_used = qsl[0].toInt();
	switch (siz)
	{
		case 5:
			_radius = qsl[4].toInt();
		case 4:
			_widths[0] = _widths[1] = _widths[2] = _widths[3] = qsl[1].toInt();
			_styleIndex[0] = _styleIndex[1] = _styleIndex[2] = _styleIndex[3] = qsl[2].toInt();
			_colorNames[0] = _colorNames[1] = _colorNames[2] = _colorNames[3] = qsl[3];
			break;
		case 8:
			_radius = qsl[7].toInt();
		case 7:
			_widths[0] = _widths[2] = qsl[1].toInt(); _widths[1] = _widths[3] = qsl[2].toInt();
			_styleIndex[0] = _styleIndex[2] = qsl[3].toInt(); _styleIndex[1] = _styleIndex[3] = qsl[4].toInt();
			_colorNames[0] = _colorNames[2] = qsl[5];  _colorNames[1] = _colorNames[3] = qsl[6];
			break;
		case 14:
			_radius = qsl[13].toInt();
		case 13:
			_widths[0] = qsl[1].toInt();
			_widths[1] = qsl[2].toInt();
			_widths[2] = qsl[3].toInt();
			_widths[3] = qsl[4].toInt();
			_styleIndex[0] = qsl[5].toInt();
			_styleIndex[1] = qsl[6].toInt();
			_styleIndex[2] = qsl[7].toInt();
			_styleIndex[3] = qsl[8].toInt();
			_colorNames[0]= qsl[9];
			_colorNames[1]= qsl[10];
			_colorNames[2]= qsl[11];
			_colorNames[3]= qsl[12];
			break;
	}
	_CountWidths();
}

/*========================================================
 * TASK: prepare value to save in ini file
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - only stores element that are needed, so it
 *				may reduce the string
 *-------------------------------------------------------*/
void _CBorder::_Prepare()
{
	_CountWidths();
	switch (_sizeWidths)
	{
		case 1:		v = QString("%1|%2|%3|%4").arg(_used).arg(_widths[0]).arg(_styleIndex[0]).arg(_colorNames[0]); 
			break;
		case 2:		v = QString("%1|%2|%3|%4|%5|%6|%7").arg(_used).arg(_widths[0]).arg(_widths[1])
													.arg(_styleIndex[0]).arg(_styleIndex[1])
													.arg(_colorNames[0]).arg(_colorNames[1]); 
			break;
		default:	// case 4:
					v = QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12|%13").arg(_used)
									.arg(_widths[0]).arg(_widths[1]).arg(_widths[2]).arg(_widths[3])
									.arg(_styleIndex[0]).arg(_styleIndex[1]).arg(_styleIndex[2]).arg(_styleIndex[3])
									.arg(_colorNames[0]).arg(_colorNames[1]).arg(_colorNames[2]).arg(_colorNames[3]); 
					break;
	}
	if (_radius)
		v += QString("|%1").arg(_radius);
}

// -------------------------------------------------------------------------------------

QString _CElem::ColorsForStyleSheet(bool addSemicolon)
{
	return color.ForStyleSheet(addSemicolon, false) + 
			background.ForStyleSheet(addSemicolon, true);
}

QString _CElem::ForStyleSheet(bool semi)	// w.o. different first line for font with possibly semicolon
{											// because that must go to a different css class
	QString qs = color.ForStyleSheet(semi, false);
	if (ClassName() != "WEB" && !background.Name().isEmpty() && background.v != config.Web.background.v)
		qs += background.ForStyleSheet(semi, true);

	qs += gradient.ForStyleSheet(semi)
		 + font.ForStyleSheet(semi)
		 + decoration.ForStyleSheet(semi)
		 + alignment.ForStyleSheet(semi)
		 + shadow1[0].ForStyleSheet(semi, true,0)	// first shadow for text (right + down) 
		 + shadow2[0].ForStyleSheet(semi, true,1)	// 2nd shadow for text
		 + shadow1[1].ForStyleSheet(semi, false,0)	// 2nd shadow (left + up) 
		 + shadow2[1].ForStyleSheet(semi, false,1)
		 + border.ForStyleSheet(semi);

	return qs;
}

bool _CElem::Changed() const
{
	bool res = color.Changed() | background.Changed() | font.Changed();
	if(_bMayShadow)
	 	res |=  shadow1[0].Changed() | 
				shadow2[0].Changed() | 
				shadow1[1].Changed() | 
				shadow2[1].Changed() | 
				border.Changed();
	if(_bMayGradient)	 
		res |= gradient.Changed() ;
	return res;
}

void _CElem::ClearChanged()
{
	color.ClearChanged();
	background.ClearChanged();
	font.ClearChanged();
	shadow1[0].ClearChanged();
	shadow2[0].ClearChanged();
	shadow1[1].ClearChanged();
	shadow2[1].ClearChanged();
	gradient.ClearChanged();
	border.ClearChanged();
}

void _CElem::ColorsAndFontsFrom(const _CElem& other)
{
	_CFG_ITEM::operator=(other);
	color = other.color;				// does not copy the default!
		color.vd = color.v;				// and this will become the default
	background = other.background;
		background.vd = background.v;	// for when no value is given
	font = other.font;
		font.vd = font.v;
}

void _CElem::Write(QSettings& s, QString group)
{
	v = false;
	if (!group.isEmpty())
		s.beginGroup(group);
	s.beginGroup(itemName);
		color.Write(s);
		background.Write(s);
		font.Write(s);
		spaceAfter.Write(s);
		decoration.Write(s);
		alignment.Write(s);
		if(_bMayShadow)
		{	
			shadow1[0].Write(s);
			shadow2[0].Write(s);
			shadow1[1].Write(s);
			shadow2[1].Write(s);
		}
		if(_bMayGradient)
			gradient.Write(s);
		border.Write(s);
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
}

void _CElem::Read(QSettings& s, QString group)
{
	v = false;
	if (!group.isEmpty())
		s.beginGroup(group);
	s.beginGroup(itemName);
		color.Read(s);
		background.Read(s);
		font.Read(s);
		spaceAfter.Read(s);
		decoration.Read(s);
		alignment.Read(s);
		if (_bMayShadow)
		{
			shadow1[0].Read(s);
			shadow2[0].Read(s);
			shadow1[1].Read(s);
			shadow2[1].Read(s);
		}
		if (_bMayGradient)
			gradient.Read(s);
		border.Read(s);
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
}

_CElem& _CElem::operator=(const _CElem& o)
{
	v = o.v;
	v0 = o.v0;
	vd = o.vd;
	kind = o.kind;
	color = o.color;
	background = o.background;
	font = o.font;
	decoration = o.decoration;
	alignment = o.alignment;
	shadow1[0] = o.shadow1[0];
	shadow2[0] = o.shadow2[0];
	shadow1[1] = o.shadow1[1];
	shadow2[1] = o.shadow2[1];
	gradient = o.gradient;
	border = o.border;
	parent = o.parent;
	return *this;
}

/*===========================================================================
* TASK:  operator= for _CWaterMark
* EXPECTS:	cwm - the other _CWaterMark
* GLOBALS: config
* RETURNS: actual watermark
* REMARKS: sets design and other parameters
*--------------------------------------------------------------------------*/
_CWaterMark &_CWaterMark::operator=(const _CWaterMark &cwm)
{
	used = cwm.used;
	return *this;
}

void _CWaterMark::Write(QSettings& s, QString group)
{
	v = false;
	if (!group.isEmpty())
		s.beginGroup(group);

	QString sOrigin;
	s.beginGroup(itemName);
		s.setValue("used", used);
		s.setValue("text", Text());

		s.setValue("origin", Origin()); // low 4 bits: vertical, high 4 bits : horizontal

		s.setValue("mx", MarginX());
		s.setValue("my", MarginY());
		s.setValue("color", Color());
		s.setValue("background", Background());
		s.setValue("shadow", ShadowColor());

		s.setValue("family", Font().family());
		s.setValue("size",   Font().pointSize());
		s.setValue("weight", Font().weight());
		s.setValue("italic", Font().italic());
	s.endGroup();

	if (!group.isEmpty())
		s.endGroup();

}

void _CWaterMark::Read(QSettings& s, QString group)
{
	v = false;
	if (!group.isEmpty())
		s.beginGroup(group);


	s.beginGroup(itemName);
		_enabled = false;	// do not create new watermark image

		QFont font;		// must set font first to get text dimensions
		font.setFamily(s.value("family", "").toString());
		font.setPointSize(s.value("size", 16).toInt());
		font.setWeight(s.value("weight", QFont::Bold).toInt());
		font.setItalic(s.value("italic", false).toBool());
		SetFont(font);

		used = s.value("used", false).toBool();
		SetText( s.value("text", "Watermark").toString());	// this sets Watermark's extent


		SetPositioning( s.value("origin", 11).toInt() ); // low 4 bit: vertical next 4 bits: horizontal

		SetMarginX(s.value("mx", 0).toInt());
		SetMarginY(s.value("my", 0).toInt());
		SetColorWithOpacity( s.value("color", 0xffffffff).toInt() );
		SetBackground(s.value("background", 0).toInt());

		_enabled = true;				// end at last create watermark image
		SetShadowColor(s.value("shadow", 0).toInt());

	s.endGroup();


	if (!group.isEmpty())
		s.endGroup();
}
// ********************************* Background Image ********************
QString _CBackgroundImage::Url(bool addSemicolon) const
{

	if (fileName.isEmpty() || v == (int)hNotUsed)
		return QString();

	QString qsN = QString("background-image:");
	if (addSemicolon)	// then for WEb page
	{					// when image must be copied into /res
		QString p, n;
		SeparateFileNamePath(fileName, p, n);
		// must ide \" instead of ' because the whole string will be in ''
		// when _RunJavaScript is called in falconG.cpp
		qsN = QString("background-image:url(\"/res/%1\")").arg(n);
	}
	else				// for sample
		qsN = QString("background-image:url(\"file://%1\")").arg(fileName);
	__AddSemi(qsN, addSemicolon);
	return qsN;
}

QString _CBackgroundImage::Size(bool addSemicolon) const
{
	QString qs = "background-size:";
	switch (v)
	{
		case hNotUsed:
			return QString();
			
		case hAuto:
			qs +="auto";
			break;
		case hContain:
			qs += "contain";
			break;
		case hCover:
			qs += "cover";
			break;
		case hTile:
			if (size < 100)
				qs += QString("%1% auto").arg(size);
			else
				qs += "auto";
			break;
	}
	__AddSemi(qs, addSemicolon);
	return qs;
}
QString _CBackgroundImage::Position(bool addSemicolon) const
{
	QString qs = "background-position:";
	switch (v)
	{
		case hNotUsed:
			return QString();

		case hAuto:
			qs += "center";
			break;
		case hTile:
		case hContain:
		case hCover:
			qs += "left top";
			break;
	}
	__AddSemi(qs, addSemicolon);
	return qs;
}
QString _CBackgroundImage::Repeat(bool addSemicolon) const
{
	QString qs = "background-repeat:";
	switch (v)
	{
		case hNotUsed:
			return QString();
		case hAuto:
		case hCover:	
			qs += "no-repeat";
			break;
		case hTile:
			qs += "repeat";
			break;
	}
	__AddSemi(qs, addSemicolon);
	return qs;
}

QString _CBackgroundImage::ForStyleSheet(bool addSemicolon) const
{
	return Url(addSemicolon) + Position(addSemicolon) + Size(addSemicolon) + Repeat(addSemicolon);
}

void _CBackgroundImage::Write(QSettings& s, QString group)
{
	if (!group.isEmpty())
		s.beginGroup(group);

	s.beginGroup(itemName);
		s.setValue("mode", v);
		s.setValue("img", fileName);
		s.setValue("size", size);
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
}
void _CBackgroundImage::Read(QSettings& s, QString group)
{
	if (!group.isEmpty())
		s.beginGroup(group);
	s.beginGroup(itemName);
		v = s.value("mode", hNotUsed).toInt();
		fileName = s.value("img", QString()).toString();
		size = s.value("size", 100).toInt();
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
}


//*********************************** CONFIG *****************************
// not needed
void CONFIG::ClearChanged()
{
	_changed = false;
	dsSrc.ClearChanged();
	dsGallery.ClearChanged();
	dsGRoot.ClearChanged();
	dsAlbumDir.ClearChanged();

	sServerAddress.ClearChanged();
	sBaseName.ClearChanged();
	sMailTo.ClearChanged();
	sAbout.ClearChanged();
	sGalleryTitle.ClearChanged();
	sGalleryLanguages.ClearChanged();
	sUplink.ClearChanged();
	iconUplink.ClearChanged();
	sMainPage.ClearChanged();
	sDescription.ClearChanged();
	sKeywords.ClearChanged();

	bAddTitlesToAll.ClearChanged();
	bAddDescriptionsToAll.ClearChanged();
	bLowerCaseImageExtensions.ClearChanged();

	bMenuToContact.ClearChanged();
	bMenuToAbout.ClearChanged();
	bMenuToDescriptions.ClearChanged();
	bMenuToToggleCaptions.ClearChanged();

	bRightClickProtected.ClearChanged();
	bCanDownload.ClearChanged();
	bForceSSL.ClearChanged();
	bOvrImages.ClearChanged();
	bSourceRelativePerSign.ClearChanged();
	bFacebookLink.ClearChanged();

	dsImageDir.ClearChanged();
	dsVideoDir.ClearChanged();
	dsLastImageDir.ClearChanged();
	dsCssDir.ClearChanged();
	dsFontDir.ClearChanged();
	sDefFonts.ClearChanged();

	Web.ClearChanged();
	Menu.ClearChanged();
	SmallGalleryTitle.ClearChanged();
	GalleryTitle.ClearChanged();
	GalleryDesc.ClearChanged();
	Section.ClearChanged();
				// images
	imageWidth.ClearChanged();
	imageHeight.ClearChanged();
	imageQuality.ClearChanged();
	imageSizesLinked.ClearChanged();
	thumbWidth.ClearChanged();
	thumbHeight.ClearChanged();
	doNotEnlarge.ClearChanged();
	bCropThumbnails.ClearChanged();
	bDistrortThumbnails.ClearChanged();

	iconToTopOn.ClearChanged();
	iconInfoOn.ClearChanged();

	imageMargin.ClearChanged();
	imageBorder.ClearChanged();
	imageMatteWidth.ClearChanged();
	imageMatteRadius.ClearChanged();
	imageMatteColor.ClearChanged();
	albumMatteWidth.ClearChanged();
	albumMatteRadius.ClearChanged();
	albumMatteColor.ClearChanged();

	waterMark.ClearChanged();
	doNotShowTheseDialogs.ClearChanged();
	defaultAnswers.ClearChanged();
	googleAnalyticsOn.ClearChanged();
	googleAnalTrackingCode.ClearChanged();

	bGenerateLatestUploads.ClearChanged();
	newUploadInterval.ClearChanged();
	nLatestCount.ClearChanged();
}

/*===========================================================================
 * TASK:  assignment operator for CONFIG
 * EXPECTS:	cfg - other config
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: sets design and other parameters
*--------------------------------------------------------------------------*/
CONFIG &CONFIG::operator=(const CONFIG &cfg)
{
	FromDesign(cfg);
	FromOther(cfg);
	return *this;
}
/*=============================================================
 * TASK:	remove common source directory path from path names
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QString CONFIG::RemoveSourceFromPath(QString s)
{
	QString sd = dsSrc.ToString();
	QString sp = s;

	// letter case doesn't matter in windows or Mac
#if defined(_MSC_VER) || defined(WIN32) || defined(WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
	sp = sp.toLower();
	sd = sd.toLower();
#elif defined()
#endif
	if (sp.left(sd.length()) == sd)
		s = s.mid(sd.length());
	return s;
}

QString CONFIG::AddSourceToPath(QString s)
{
	QString sd = dsSrc.ToString();
	QString sp = s;

	bool isAbsName = QDir::isAbsolutePath(s);
	// letter case doesn't matter in Windows or on a Mac
#if defined(Q_OS_WINDOWS) || defined (Q_OS_MACOS)
	sp = sp.toLower();
	sd = sd.toLower();
#endif
#if !defined Q_OS_WINDOWS
	// on linux or OS X abs. paths starts with '/'
	if (config.bSourceRelativePerSign)
		isAbsName = false;
#endif
	if (!isAbsName && sp.left(sd.length()) != sd)
		s = dsSrc.ToString() + s;
	return s;
}
/*===========================================================================
 * TASK:  assign other configuration parameters
 * EXPECTS:	cfg - other config
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: 
*--------------------------------------------------------------------------*/
void CONFIG::FromOther(const CONFIG &cfg)
// synchronize with Read!
{
	dsApplication = cfg.dsApplication;	// program directory

	dsSrc = cfg.dsSrc;					// source gallery
	dsGallery = cfg.dsGallery;			// destination directory
	dsGRoot = cfg.dsGRoot;				// name of root directory on server
	dsAlbumDir = cfg.dsAlbumDir;		// name of album directory (usually inside dsGRoot)
	dsCssDir = cfg.dsCssDir;
	dsFontDir = cfg.dsFontDir;
	dsImageDir = cfg.dsImageDir;
	dsVideoDir = cfg.dsVideoDir;
	dsThumbDir = cfg.dsThumbDir;

	sUplink = cfg.sUplink;				// if given uplink in gallery root goes here
	iconUplink = cfg.iconUplink;				// if given uplink in gallery root goes here
	sMainPage = cfg.sMainPage;
	sDescription = cfg.sDescription;
	sKeywords = cfg.sKeywords;

	bGenerateAll = cfg.bGenerateAll;
	bButImages = cfg.bButImages;
	bAddTitlesToAll = cfg.bAddTitlesToAll;
	bAddDescriptionsToAll = cfg.bAddDescriptionsToAll;
	bLowerCaseImageExtensions = cfg.bLowerCaseImageExtensions;
	bReadFromDirs = cfg.bReadFromDirs;
	bReadFromGallery = cfg.bReadFromGallery;
	bMenuToContact = cfg.bMenuToContact;
	bMenuToAbout = cfg.bMenuToAbout;
	bMenuToDescriptions = cfg.bMenuToDescriptions;
	bMenuToToggleCaptions = cfg.bMenuToToggleCaptions;

	bCanDownload = cfg.bCanDownload;
	bForceSSL = cfg.bForceSSL;
	bRightClickProtected = cfg.bRightClickProtected;
	bOvrImages = cfg.bOvrImages;

	sServerAddress = cfg.sServerAddress;
	sDefFonts = cfg.sDefFonts;
	sBaseName = cfg.sBaseName;
	sMailTo = cfg.sMailTo;
	sAbout = cfg.sAbout;

	bGenerateLatestUploads = cfg.bGenerateLatestUploads;
	newUploadInterval = cfg.newUploadInterval;
	nLatestCount = cfg.nLatestCount;

	googleAnalyticsOn = cfg.googleAnalyticsOn;
	googleAnalTrackingCode = cfg.googleAnalTrackingCode;

	sGalleryTitle = cfg.sGalleryTitle;
	sGalleryLanguages = cfg.sGalleryLanguages;
	bOvrImages = cfg.bOvrImages;
}

 void CONFIG::RestoreDesign()
 {
	 FromDesign(configSave);
 }

 void CONFIG::RestoreOther()
 {
	 FromOther(configSave);
 }

 void CONFIG::SaveDesign()
 {
	 return configSave.FromDesign(*this);
 }

void CONFIG::SaveOther()
 {
	 return configSave.FromOther(*this);
 }

/*===========================================================================
 * TASK:  assign design configuration parameters
 * EXPECTS:	cfg - other config
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: 
*--------------------------------------------------------------------------*/
void CONFIG::FromDesign(const CONFIG &cfg)		// synchronize with Read!
{			// _CElem's
	Web = cfg.Web;								// page
	Header = cfg.Header;
	Menu = cfg.Menu;							// buttons
	Lang = cfg.Lang;
											// special handling : include shadows
	SmallGalleryTitle = cfg.SmallGalleryTitle;		// "andreas falco photography"
	GalleryTitle = cfg.GalleryTitle;		
	GalleryDesc = cfg.GalleryDesc;		

	Section = cfg.Section;			// "Images" or "Albums"
	Thumb = cfg.Thumb;
	ImageTitle = cfg.ImageTitle;
	ImageDesc = cfg.ImageDesc;
	LightboxTitle = cfg.LightboxTitle;
	LightboxDesc = cfg.LightboxDesc;
	Footer = cfg.Footer;
	
	waterMark = cfg.waterMark;
	bFacebookLink = cfg.bFacebookLink;
	// images
	imageWidth = cfg.imageWidth;
	imageHeight = cfg.imageHeight;
	thumbWidth = cfg.thumbWidth;
	thumbHeight = cfg.thumbHeight;
	imageSizesLinked = cfg.imageSizesLinked;
	// image decoration
	iconToTopOn = cfg.iconToTopOn;
	iconInfoOn = cfg.iconInfoOn;


	imageMargin = cfg.imageMargin;
	imageBorder = cfg.imageBorder;
	imageMatteWidth = cfg.imageMatteWidth;
	imageMatteColor = cfg.imageMatteColor;
	imageMatteRadius = cfg.imageMatteRadius;

	albumMatteWidth = cfg.albumMatteWidth;
	albumMatteColor = cfg.albumMatteColor;
	albumMatteRadius = cfg.albumMatteRadius;
}
/*============================================================================
* TASK: constructor
* EXPECTS: nothing
* RETURNS: nothing
* REMARKS: sets up names for _CElem's
*---------------------------------------------------------------------------*/
CONFIG::CONFIG()
{
	if(PROGRAM_CONFIG::parent == nullptr)
			PROGRAM_CONFIG::parent = this;
	Header					.parent = &Web;
	Menu					.parent = &Web;
	Lang					.parent = &Web;
	SmallGalleryTitle 		.parent = &Web;
	GalleryTitle			.parent = &Web;
	GalleryDesc				.parent = &Web;
	Section					.parent = &Web;
	ImageTitle				.parent = &Web;
	ImageDesc				.parent = &Web;
	LightboxTitle			.parent = &Web;
	LightboxDesc			.parent = &Web;
	Footer					.parent = &Web;
}

/*============================================================================
  * TASK: read configuration settings or set default ones
  * EXPECTS: path (may be null) 
  *					if not null 
  *					reads data from ini file in '*path' named as last part of path
  *					with the ".ini" prefix if that exists. 
  *					Example: path = "this/is/my/path/" then  the file name is 
  *									"this/is/my/path/path.ini"
  *					If that doesn't doesn't tries  falcongG_ini in '*path' 
  *					and if that does not exist either then from "falconG.ini"
  *					in the program directory
  *					
  *	RETURNS: nothing
  * REMARKS: - read PROGRAM_CONFIG before using this!
  *			 - Config element "Web" contains the default foreground and 
  *				background colors. These will be set into all other elements,
  *				but modified if the others changed
 *---------------------------------------------------------------------------*/
void CONFIG::Read()		// synchronize with Write!
{
	QString sIniName = PROGRAM_CONFIG::NameForConfig(false, ".ini"); // false: fallback to default if ini does not exist in source dir.

	QSettings s(sIniName, QSettings::IniFormat);
	s.setIniCodec("UTF-8");

	// directories
	dsSrc.Read(s);
	dsGallery.Read(s);
	dsGallery.Read(s);
	dsGRoot.Read(s);
	//if (dsGRoot.IsEmpty())
	//	dsGRoot = "/";
	//if(dsGRoot.ToString()[0] != QChar('/'))
	//	dsGRoot.v0 = dsGRoot.v = "/" + dsGRoot.v0;
	dsAlbumDir.Read(s);

	sBaseName.Read(s);
	sMailTo.Read(s);
	sAbout.Read(s);
	sGalleryTitle.Read(s);
	sGalleryLanguages.Read(s);
	sUplink.Read(s);
	iconUplink.Read(s);
	sMainPage.Read(s);
	sDescription.Read(s);
	sGoogleFonts.Read(s);
	sDefFonts.Read(s);
	sKeywords.Read(s);
											// special data
	bSeparateFoldersForLanguages.Read(s);
	bAddTitlesToAll.Read(s);
	bAddDescriptionsToAll.Read(s);
	bKeepDuplicates.Read(s);
	bLowerCaseImageExtensions.Read(s);
	bMenuToContact.Read(s);
	bMenuToAbout.Read(s);
	bMenuToDescriptions.Read(s);
	bMenuToToggleCaptions.Read(s);
	bFixedLatestThumbnail.Read(s);

	bRightClickProtected.Read(s);
	bCanDownload.Read(s);
	bForceSSL.Read(s);
	bOvrImages.Read(s);
	bSourceRelativePerSign.Read(s);
	bFacebookLink.Read(s);

	dsImageDir.Read(s);
	dsVideoDir.Read(s);
	dsLastImageDir.Read(s);
	dsCssDir.Read(s);
	dsFontDir.Read(s);
	sDefFonts.Read(s);
				// images
	imageWidth.Read(s);
	imageHeight.Read(s);
	imageQuality.Read(s);
	imageSizesLinked.Read(s);
	thumbWidth.Read(s);
	thumbHeight.Read(s);
	doNotEnlarge.Read(s);
	bCropThumbnails.Read(s);
	bDistrortThumbnails.Read(s);
	// image decoration
	imageMargin.Read(s);
	imageBorder.Read(s);
	imageMatteWidth.Read(s);
	imageMatteColor.Read(s);
	imageMatteRadius.Read(s);
	// album decoration
	albumMatteWidth.Read(s);
	imageMatteRadius.Read(s);
	albumMatteColor.Read(s);
	albumMatteRadius.Read(s);

	iconToTopOn.Read(s);
	iconInfoOn.Read(s);
			// Google Analytycs
	googleAnalyticsOn.Read(s);
	googleAnalTrackingCode.Read(s);
			// latest uploads
	bGenerateLatestUploads.Read(s);
	newUploadInterval.Read(s);
	nLatestCount.Read(s);

	sServerAddress.Read(s);

	Web.Read(s);		// sets common colors and fonts, which may be modified later
						// on read
	backgroundImage.Read(s);

	Menu.ColorsAndFontsFrom(Web);
	Lang.ColorsAndFontsFrom(Web);
	SmallGalleryTitle.ColorsAndFontsFrom(Web);
	GalleryTitle.ColorsAndFontsFrom(Web);
	GalleryDesc.ColorsAndFontsFrom(Web);
	Section.ColorsAndFontsFrom(Web);
	ImageTitle.ColorsAndFontsFrom(Web);
	ImageDesc.ColorsAndFontsFrom(Web);
	LightboxTitle.ColorsAndFontsFrom(Web);
	LightboxDesc.ColorsAndFontsFrom(Web);
	Footer.ColorsAndFontsFrom(Web);

	Menu.Read(s);
	Lang.Read(s);
	SmallGalleryTitle.Read(s);
	GalleryTitle.Read(s);
	GalleryDesc.Read(s);
	Section.Read(s);
	ImageTitle.Read(s);
	ImageDesc.Read(s);
	LightboxTitle.Read(s);
	LightboxDesc.Read(s);
	Footer.Read(s);

				// 	Watermarks
	waterMark.Read(s);
	// other
	doNotShowTheseDialogs.Read(s);
	defaultAnswers.Read(s);
	// Debug
	bDebugging.Read(s);

	int lastCfg = PROGRAM_CONFIG::indexOfLastUsed;		// check and correct bad parameter from file
	if (lastCfg >= PROGRAM_CONFIG::lastConfigs.size())
		PROGRAM_CONFIG::indexOfLastUsed = lastCfg = -1;

	if (lastCfg >= 0 && dsSrc.IsEmpty())
		dsSrc.v = PROGRAM_CONFIG::lastConfigs[lastCfg];
	QString qs = PROGRAM_CONFIG::NameForConfig(true, ".ini");
	if(!QFile::exists(qs))
		config._WriteIni(qs);			// already written properties

	SetChanged(false);
	configSave = *this;
}

/*=============================================================
* TASK: writes changed configuration settings
* EXPECTS: sIniName: name of file to write into
* RETURNS: nothing
* REMARKS: - does not modify the 'changed' flags
*		   - synchronize it with Read
*------------------------------------------------------------*/
void CONFIG::_WriteIni(QString sIniName)
{
	QSettings s(sIniName, QSettings::IniFormat);
	s.setIniCodec("UTF-8");

	// directories
	dsSrc.Write(s);
	dsGallery.Write(s);
	if (dsGRoot == "/")
		dsGRoot.Clear();
	dsGRoot.Write(s);
	dsAlbumDir.Write(s);

	sBaseName.Write(s);
	sMailTo.Write(s);
	sAbout.Write(s);
	sGalleryTitle.Write(s);
	sGalleryLanguages.Write(s);
	sUplink.Write(s);
	iconUplink.Write(s);
	sMainPage.Write(s);
	sDescription.Write(s);
	sGoogleFonts.Write(s);
	sDefFonts.Write(s);
	sKeywords.Write(s);
											// special data
	bSeparateFoldersForLanguages.Write(s);
	bAddTitlesToAll.Write(s);
	bAddDescriptionsToAll.Write(s);
	bKeepDuplicates.Write(s);
	bLowerCaseImageExtensions.Write(s);
	bMenuToContact.Write(s);
	bMenuToAbout.Write(s);
	bMenuToDescriptions.Write(s);
	bMenuToToggleCaptions.Write(s);
	bFixedLatestThumbnail.Write(s);

	bRightClickProtected.Write(s);
	bCanDownload.Write(s);
	bForceSSL.Write(s);
	bOvrImages.Write(s);
	bSourceRelativePerSign.Write(s);
	bFacebookLink.Write(s);

	dsImageDir.Write(s);
	dsVideoDir.Write(s);
	dsLastImageDir.Write(s);
	dsCssDir.Write(s);
	dsFontDir.Write(s);
	sDefFonts.Write(s);
				// images
	imageWidth.Write(s);
	imageHeight.Write(s);
	imageQuality.Write(s);
	imageSizesLinked.Write(s);
	thumbWidth.Write(s);
	thumbHeight.Write(s);
	doNotEnlarge.Write(s);
	bCropThumbnails.Write(s);
	bDistrortThumbnails.Write(s);
	// image decoration
	imageMargin.Write(s);
	imageBorder.Write(s);
	imageMatteWidth.Write(s);
	imageMatteColor.Write(s);
	// album decoration
	albumMatteWidth.Write(s);
	imageMatteRadius.Write(s);
	albumMatteColor.Write(s);
	albumMatteRadius.Write(s);
	
	iconToTopOn.Write(s);
	iconInfoOn.Write(s);
			// Google Analytycs
	googleAnalyticsOn.Write(s);
	googleAnalTrackingCode.Write(s);
			// latest uploads
	bGenerateLatestUploads.Write(s);
	newUploadInterval.Write(s);
	nLatestCount.Write(s);
	
	sServerAddress.Write(s);

	backgroundImage.Write(s);

	Web.Write(s);
	Menu.Write(s);
	Lang.Write(s);
	SmallGalleryTitle.Write(s);
	GalleryTitle.Write(s);
	GalleryDesc.Write(s);
	Section.Write(s);
	ImageTitle.Write(s);
	ImageDesc.Write(s);
	LightboxTitle.Write(s);
	LightboxDesc.Write(s);
	Footer.Write(s);
				// 	Watermarks
	waterMark.Write(s);

	// other
	doNotShowTheseDialogs.Write(s);
	defaultAnswers.Write(s);

	if (__bClearChangedFlag)
		ClearChanged();
	configSave = *this;
}
/*============================================================================
  * TASK: writes changed configuration settings
  * EXPECTS: nothing
  *	RETURNS: nothing
  * REMARKS: - saves config both in dsSrc directory in file named as
  *				dsSrc + <last part of dsSrc>+".ini
  *			 and into program directory in file 'falcong_ini'
  *			 if 'changed' is false it does not write anything
  *			 only changed fields are written out
  *			 synchronize it with Read
  *			 - sets all 'changed' flags to false
 *---------------------------------------------------------------------------*/
void CONFIG::Write()			// synchronize with Read!
{
	PROGRAM_CONFIG::Write();	// save data in user's directory
//	_WriteIni(falconG_ini);		// last used data into there as well

	QString p, n;
	SeparateFileNamePath(dsSrc.ToString(), p, n);

	__bClearChangedFlag = true;				// mark all changes as saved
	_WriteIni(dsSrc.ToString() + n + ".ini"); // save into album directory
	__bClearChangedFlag = false;

	ClearChanged();
}

QString _CTextAlign::ForStyleSheet(bool addSemiColon) const
{
	QString qs = v == alNone ? QString() : "text-align:" + ActAlignStr();
	__AddSemi(qs, addSemiColon);
	return qs;
}
