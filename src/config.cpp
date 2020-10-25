#include "config.h"

bool savelItemsAlways;	// global !

// helper	prepends a tab and adds a semicolon and LF after the string
void __AddSemi(QString& s, bool addSemicolon)
{
	if (!s.isEmpty())
	{
		if (addSemicolon)
			s = "	" + s + ";";
		s += "\n";
	}
}


int CONFIGS_USED::maxSavedConfigs;		// last 10 configuration directory is stored
QStringList CONFIGS_USED::lastConfigs;

int CONFIGS_USED::indexOfLastUsed;		// this was the last one used
CONFIG *CONFIGS_USED::parent = nullptr;


static bool __bClearChangedFlag = false;	// set to true to clear the changed flag after writing the configuration

					// global variables for configuration 
CONFIG config,		// must be here because _Changed uses it
	   configSave;

/*========================== CONFIG_USED ====================*/
void CONFIGS_USED::Read()
{
	lastConfigs.clear();
	QSettings s(falconG_ini, QSettings::IniFormat);	// in program directory

	s.beginGroup("config_save"); //--------------------------------

	maxSavedConfigs = s.value("maxSaveConfigs", 10).toInt();
	int numSaveConfigs = s.value("numSaveConfigs", 0).toInt();
	indexOfLastUsed = s.value("indexOfLastUsed", 0).toInt();
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

void CONFIGS_USED::Write()
{
	QString sLast = parent->dsSrc.v.trimmed();
	if (sLast.isEmpty())		
		CONFIGS_USED::indexOfLastUsed = -1;
	else
	{
		if (CONFIGS_USED::indexOfLastUsed < 0)	// then search list for last item
		{
			int n;
			if ((n = CONFIGS_USED::lastConfigs.indexOf(sLast, 0)) < 0)
			{
				if (CONFIGS_USED::lastConfigs.size() == CONFIGS_USED::maxSavedConfigs)
					CONFIGS_USED::lastConfigs.removeLast();

				CONFIGS_USED::lastConfigs.insert(0, sLast);
				CONFIGS_USED::indexOfLastUsed = 0;
			}
			else
				CONFIGS_USED::indexOfLastUsed = n;
		}
	}

	QSettings s(falconG_ini, QSettings::IniFormat);	// in program directory
	s.beginGroup("config_save");
	s.setValue("maxSaveConfigs", maxSavedConfigs);
	s.setValue("numSaveConfigs", lastConfigs.size());
	s.setValue("indexOfLastUsed", indexOfLastUsed);
	for (int i = 0; i < lastConfigs.size(); ++i)
		s.setValue(QString().setNum(i), lastConfigs.at(i));
	s.endGroup();
}


/*========================================================
 * TASK: Determines file name of configuration from ini
*				 or struct files
 * PARAMS:	forSave - name required for save or read
			sExt - extension (.ini or .struct)
 *					defaults: 'falconG.ini' and 'gallery.struct'
 * GLOBALS: lastConfigs, indexOfLastUsed
 * RETURNS:	full path name of config file
 * REMARKS: - If a directory path is taken from 'lastConfig'
 *				returns a name of this directory without its
 *				path followed by the extension sExt inside
 *				the original directory
 *				Example: directory name: '/in/this/dir/'
 *				File name '/in/this/dir/dir.ini'
 *			- With no last used directory returns the
 *				default names with no directory path
 *-------------------------------------------------------*/
QString CONFIGS_USED::NameForConfig(bool forSave, QString sExt)
{
	QString sDefault;
	if (sExt == ".ini")
		sDefault = falconG_ini;
	else
		sDefault = "gallery.struct";

	QString sIniName, p, n, s;				  // if there was a last used directory read from it
	if (indexOfLastUsed < 0 || indexOfLastUsed >= lastConfigs.size())
		return sDefault;

	s = lastConfigs[indexOfLastUsed];
	SeparateFileNamePath(s, p, n);	// cuts '/' from name
	if (s[s.length() - 1] != '/')
		s += "/";
	if (n.isEmpty())
		sIniName = sDefault;		 //	 'falconG.ini' (in program folder)
	else
		sIniName = s + n + sExt;	 // = <path w.o. last folder name from 'falcong.ini'>/<folder>/<folder>.ini

	if(!forSave && !QFile::exists(sIniName))	 
	{
		sIniName = s + sDefault;
		if (!QFile::exists(sIniName))
			sIniName = sDefault;
	}

	return sIniName;
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
* REMARKS: JAlbum quirk: if a link is relatice to the JA;lbuum root the path to the
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
bool &_CBool::operator=(const bool s)
{ 
	return v = s; 
}

/*===========================================================================
 * TASK: assign an into a CInt
 * EXPECTS: i - int
 * GLOBALS: config
 * RETURNS: the int
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
int &_CInt::operator=(const int i)
{
	return v = i;
}
// --------------------------- _CColor ------------------------------

/*===========================================================================
 * TASK: assign a QString to a _CColor and modify opacity
 * EXPECTS: str - QString: formats RGB or RRGGBB or #RGB or #RRGGBB, where
 *				 R,G,B are any hexadecimaL digit
 *		   opac - opacity in percent 0..100
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS:	- if string name is invalid does not set it
 *			- no symbolic color names
 *			- any combination of upper and lower case hexadecimal numbers
 *			- if the QString is #xyz, then it is the same as #xxyyzz
 *--------------------------------------------------------------------------*/
void _CColor::Set(QString str, int opac)
{ 
	if (!_ColorStringValid(str))
		return;

	_colorName = str; 
	_opacity = opac;
	_Prepare();		// setup 'v'
}

QString _CColor::ForStyleSheet(bool addSemiColon, bool isBackground) const
{
	QString qs = (isBackground ? "background:" : "color:");
	if (_opacity != 100 && _opacity != -1)
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

	if (s.length() != 4 && s.length() != 7 && s.length() != 9)	// #RGB, #RRGGBB or #AARRGGBB
		return false;
	char n;
	for (int i = 1; i < s.length(); ++i)
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
	addProp("line-height", SizeStr());
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
void _CFont::Set(QString fam, QString siz, int feat, QString sFsSize)
{
	_details.clear();
	_details.push_back(fam);
	_details.push_back(siz);
	_details.push_back(QString().setNum(feat) );
	_details.push_back("0");			// default: no different first line
	_details.push_back(_details[1]);	// first line same as others
	if(!sFsSize.isEmpty())	
	{
		_details[3] = "1";	// different first line
		_details[4] = sFsSize;
	}

	_Prepare();
}
void _CFont::SetFamily(QString fam)
{
	_details[0] = fam;
	_Prepare();
}

void _CFont::SetFeature(Style feat, FeatureOp op)
{
	int fs = _details[2].toInt(),
		f = (int) feat;

	if (op == foClearAll || op == foClearOthersAndSet)
		fs = 0;
	else if (op == foUnset)
		fs &= ~f;
	if (op == foSet)
		fs |= f;

	_details[2] = QString().setNum(fs);
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
	_details[1] = s;
	_Prepare();
}

void _CFont::SetDifferentFirstLine(bool set, QString size)
{
	_details[3] = set ? "1" : "0";
	if (!size.isEmpty())
		_details[4] = size;
}

void _CFont::_Prepare()
{				  // only add first line size if used or different from font size
	int n = _details[3] == "1" && _details[1] != _details[4] ? 5 : 3;
	v = _details[0];
	for (int i = 1; i < n; ++i)
		v += "|" + _details[i];
}

void _CFont::_Setup()
{
	_details = v.split(QChar('|'));		   
	switch (_details.size())		// No breaks!
	{								// element 0 is family
		case 1: _details.push_back("10pt");	  // font-size string with unit
		case 2: _details.push_back("0");	  // style (bold:'1', italic:'2', bold & italic: '3')
		case 3: _details.push_back("0");	  // first line is different?
		case 4: _details.push_back("10pt");	  // font-size::first-line
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

void _CShadow::_Setup()
{
	_details = v.split(QChar('|'));
	while (_details.size() < 4)
		_details.push_back("0");
	if(_details.size() != 5)
		_details.push_back("#000000");

	if (_details[3].at(0).isDigit())
		_ixBlur = 3;
	else
		_ixBlur = 4;
	_ixColor = _ixBlur ^ 7;
	_Prepare();
//	v0 = v;
}

void _CShadow::_Prepare()
{
	v.clear();

	v = _details.join("|");
}

/*===========================================================================
 * TASK: set top,right,bttom or left shadow
 * EXPECTS: which - 0:   use, 1:horiz, 2: vert, 3 blur
 *		    val   - size of shadow in pixels
 *					but color may also be set as a number (see REMARKS)
 *			_ixBlur and _ixColor are set
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS:
 --------------------------------------------------------------------------*/
void _CShadow::Set(ShadowPart which, int val)	// which: 1: horiz, 2: vert, 3:blur !
{
	if((int)which < 3)		// these always exist
		_details[which] = QString().setNum(val);
	else   // 3 or 4   : blur or color may or may not exist
	{
		if ( (int) which == _ixBlur)
			_details[_ixBlur] = QString().setNum(val);
		else 
			_details[_ixColor] = QString("#%1").arg(_CInt::IntToHexString(val, 6));
	}

	_Prepare();
}
/*===========================================================================
 * TASK: set all of the shadows
 * EXPECTS: top,ight,bottom,left - size of shadow in pixels
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
void _CShadow::Set(int horiz, int vert, int blur, QString clr, bool used)
{
	QString oldV = v;
	_details.clear();
	_ixBlur = _ixColor = -1;

	_details.push_back(used ? "1" : "0");
	_details.push_back(QString().setNum(horiz));
	_details.push_back(QString().setNum(vert));
	_details.push_back(QString().setNum(blur));
	_ixBlur = 3;
	if (clr.isEmpty())
		clr = "#000000";
	_details.push_back(clr);
	_ixColor = 4;

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
		if (_ixBlur)
			res += " " + _details[_ixBlur] + "px";
		if (_ixColor)
			res += " " + _details[_ixColor];

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
	if (!used) 
		return QString();
	QString qs = QString("background-image:linear-gradient(%1,%2%,%3,%4%,%5,%6%)").arg(gs[0].color).arg(gs[0].percent)
														      .arg(gs[1].color).arg(gs[1].percent)
																.arg(gs[2].color).arg(gs[2].percent);
	__AddSemi(qs, semi);
	return qs;
}

QString _CGradient::ForQtStyleSheet(bool invert) const
{
	if (!used) 
		return QString();
	int n0, n1, n2;
	if (invert)
		n0 = 2, n1 = 1, n2 = 0;
	else
		n0 = 0, n1 = 1, n2 = 2;
	return QString("qlineargradient(x1:0, y1:0, x2:0,y2:1,,stop:%1 %2,Stop:%3 %4, stop:%5 %6)").arg(gs[n0].percent/100.0).arg(gs[n0].color)
														      .arg(gs[n1].percent/100.0).arg(gs[n1].color)
														      .arg(gs[n2].percent/100.0).arg(gs[n2].color);

}

void _CGradient::_Setup()	// what was read from settings into v
{
	QStringList qsl = v.split(QChar('|'));		//	<used>|<p Top>|<c Top>|<p Middle>|<c Middle>|<p Bottom>|<c bottom>
												// u: used 0,1, p = position (%), c = color

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
	v = QString("%1|%2|%3|%4|%5|%6").arg(used)
									.arg(gs[0].percent).arg(gs[0].color)
									.arg(gs[1].percent).arg(gs[1].color)
									.arg(gs[2].percent).arg(gs[2].color);
}


// -------------------------------------------------------------------------------------



/*========================================================
 * TASK:	Sets border styles
 * PARAMS:	semi: if set prepends a TAB and appends a semicolon
 *				after each line
 * GLOBALS:
 * RETURNS:
 * REMARKS: - returns border-top:, border-right, border-bottom
 *				border-left and border-radius
 *			- can't use different styles and colors for 
 *				each side
 *			- may not use the shorthand as it only works if no
 *			  radius is given
 *-------------------------------------------------------*/
QString _CBorder::ForStyleSheet(bool semi) const		// w. radius
{
	if(!Used())
		return QString();

	QString res,res2,res3,res4;
	res = QString("border-top:%1px %2 %3").arg(_widths[sdTop]).arg(Style(sdTop)).arg(ColorStr(sdTop));
	res2 = QString("border-right:%1px %2 %3").arg(_widths[sdRight]).arg(Style(sdRight)).arg(ColorStr(sdRight));
	res3 = QString("border-bottom:%1px %2 %3").arg(_widths[sdBottom]).arg(Style(sdBottom)).arg(ColorStr(sdBottom));
	res4 = QString("border-left:%1px %2 %3").arg(_widths[sdLeft]).arg(Style(sdLeft)).arg(ColorStr(sdLeft));
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

QString _CBorder::ForStyleSheetShort() const
{
	if (!Used())
		return QString("	border:none");

	if (_sizeWidths > 1)
		return ForStyleSheet(false);
	return QString("	border:%1px %2 %3").arg(_widths[0]).arg( Style(sdAll) ).arg(_colorNames[0]);
}

void _CBorder::_CountWidths()
{
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
		_sizeWidths = 4;		// 4 width, style and color
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
	// create as for this: <used 0 or 1>|top|right|bottom|left|style|<color>[|<radius in pixels>]
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
	switch (_sizeWidths)
	{
		case 1:		v = QString("%1|%2|%3|%4").arg(_used).arg(_widths[0]).arg(_styleIndex[0]).arg(_colorNames[0]); 
			break;
		case 2:		v = QString("%1|%2|%3|%4|%5|%6|%7").arg(_used).arg(_widths[0]).arg(_widths[1])
													.arg(_styleIndex[0]).arg(_styleIndex[1])
													.arg(_colorNames[0]).arg(_colorNames[1]); 
			break;
		default:	// case 4:
					v = QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12").arg(_used)
									.arg(_widths[0]).arg(_widths[1]).arg(_widths[2]).arg(_widths[3])
									.arg(_styleIndex[0]).arg(_styleIndex[10]).arg(_styleIndex[2]).arg(_styleIndex[3])
									.arg(_colorNames[0]).arg(_colorNames[20]).arg(_colorNames[2]).arg(_colorNames[3]); 
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

QString _CElem::ForStyleSheet(bool semi)	// w.o. different first line for font with possibli semicolon
{											// because it must go to a different css class
	QString qs = color.ForStyleSheet(semi, false);
	if (ClassName() != "WEB" && background.v != config.Web.background.v)
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
		decoration.Read(s);
		alignment.Read(s);
		if (_bMayShadow)
		{
			shadow1[0].Read(s);
			shadow2[0].Read(s);
			shadow1[1].Read(s);
			shadow2[1].Read(s);
		}
		if(_bMayGradient)
			gradient.Read(s);
		border.Read(s);
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
}

/*===========================================================================
* TASK:  inequality operator for _CWaterMark
* EXPECTS:	cwm - the other _CWaterMark
* GLOBALS: config
* RETURNS: nothing
* REMARKS: sets design and other parameters
*--------------------------------------------------------------------------*/
_CWaterMark &_CWaterMark::operator=(const _CWaterMark &cwm)
{
	v = ((WaterMark&)cwm.wm != (WaterMark&)wm);	//'const'
	wm = cwm.wm;
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
		s.setValue("text", wm.text);

		s.setValue("origin", wm.origin); // low 4 bits: vertical, high 4 bits : horizontal

		s.setValue("mx", wm.marginX);
		s.setValue("my", wm.marginY);
		s.setValue("color", wm.colorWOpacity);
		s.setValue("background", wm.background);
		s.setValue("shadow", wm.shadowColor);

		s.setValue("family", wm.font.family());
		s.setValue("size", wm.font.pointSize());
		s.setValue("weight", wm.font.weight());
		s.setValue("italic", wm.font.italic());
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
		used = s.value("used", false).toBool();
		wm.text = s.value("text", "Watermark").toString();

		wm.origin = s.value("origin", 11).toInt(); // low 4 bit: vertical next 4 bits: horizontal

		wm.marginX = s.value("mx", 0).toInt();
		wm.marginY = s.value("my", 0).toInt();
		wm.colorWOpacity = s.value("color", 0xffffff).toInt();
		wm.background = s.value("background", 0).toInt();
		wm.shadowColor = s.value("shadow", 0).toInt();
		wm.font.setFamily(s.value("family", "").toString());
		wm.font.setPointSize(s.value("size", 16).toInt());
		wm.font.setWeight(s.value("weight", QFont::Bold).toInt());
		wm.font.setItalic(s.value("italic", false).toBool());
	s.endGroup();


	if (!group.isEmpty())
		s.endGroup();
}
// ********************************* Background Image ********************

QString _CBackgroundImage::ForStyleSheet(bool addSemicolon)
{
	if(v == (int)hNotUsed)
		return QString();
		// ------- background image -----

	QString qsS = "auto",			// background-size
			qsR = "no-repeat";		// background-repeat

	if (config.backgroundImage.fileName.isEmpty())
		return QString();

	switch (config.backgroundImage.v)
	{
		case hAuto:		break;
		case hCover:	qsS = "cover"; break;
		case hContain:	qsS = "contain"; break;
		case hTile:		qsR = "repeat"; break;
	}

	QString qs = QString("background-image: %1;\n"
				   "background-size: %2;\n"
				   "background-repeat: %3;\n").arg(fileName).arg(qsS).arg(qsR);
	__AddSemi(qs, addSemicolon);
	return qs;
}

void _CBackgroundImage::Write(QSettings& s, QString group)
{
	if (!group.isEmpty())
		s.beginGroup(group);

	s.beginGroup(itemName);
		s.setValue("mode", v);
		s.setValue("img", fileName);
	s.endGroup();
	if (!group.isEmpty())
		s.endGroup();
	v = v0;
}
void _CBackgroundImage::Read(QSettings& s, QString group)
{
	if (!group.isEmpty())
		s.beginGroup(group);
	s.beginGroup(itemName);
		v = s.value("mode", hNotUsed).toInt();
		fileName = s.value("img", QString()).toString();
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
	imageSizesLinked.ClearChanged();
	thumbWidth.ClearChanged();
	thumbHeight.ClearChanged();
	doNotEnlarge.ClearChanged();
	bCropThumbnails.ClearChanged();
	bDistrortThumbnails.ClearChanged();

	imageBorder.ClearChanged();

	iconToTopOn.ClearChanged();
	iconInfoOn.ClearChanged();

	waterMark.ClearChanged();

	googleAnalyticsOn.ClearChanged();
	googleAnalTrackingCode.ClearChanged();

	generateLatestUploads.ClearChanged();
	newUploadInterval.ClearChanged();
	nLatestCount.ClearChanged();
	styleIndex.ClearChanged();
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
	bReadJAlbum = cfg.bReadJAlbum;
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

	generateLatestUploads = cfg.generateLatestUploads;
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

/*===========================================================================
 * TASK:  assign design configuration parameters
 * EXPECTS:	cfg - other config
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: 
*--------------------------------------------------------------------------*/
void CONFIG::FromDesign(const CONFIG &cfg)		// synchronize with Read!
{
	Web = cfg.Web;				// page
	Menu = cfg.Menu;				// buttons
	Lang = cfg.Lang;
											// special handling : include shadows
	GalleryTitle = cfg.GalleryTitle;		// "andreas falco photography"
	SmallGalleryTitle = cfg.SmallGalleryTitle;		// "andreas falco photography"
	GalleryDesc = cfg.GalleryDesc;		// "andreas falco photography"

	Section = cfg.Section;			// "Images" or "Albums"
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

	imageBorder = cfg.imageBorder;
	styleIndex = cfg.styleIndex;
}
/*============================================================================
* TASK: constructor
* EXPECTS: nothing
* RETURNS: nothing
* REMARKS: sets up names for _CElem's
*---------------------------------------------------------------------------*/
CONFIG::CONFIG()
{
	if(CONFIGS_USED::parent == nullptr)
			CONFIGS_USED::parent = this;
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
  * REMARKS: - read CONFIGS_USED before using this!
  *			 - Config element "Web" contains the default foreground and 
  *				background colors. These will be set into all other elements,
  *				but modified if the others changed
 *---------------------------------------------------------------------------*/
void CONFIG::Read()		// synchronize with Write!
{

	QString sIniName = CONFIGS_USED::NameForConfig(false, ".ini");

	QSettings s(sIniName, QSettings::IniFormat);
	s.setIniCodec("UTF-8");

	// directories
	dsSrc.Read(s);
	dsGallery.Read(s);
	dsGallery.Read(s);
	dsGRoot.Read(s);
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
											// sepcial data
	bAddTitlesToAll.Read(s);
	bAddDescriptionsToAll.Read(s);
	bLowerCaseImageExtensions.Read(s);
	bMenuToContact.Read(s);
	bMenuToAbout.Read(s);
	bMenuToDescriptions.Read(s);
	bMenuToToggleCaptions.Read(s);

	bRightClickProtected.Read(s);
	bCanDownload.Read(s);
	bForceSSL.Read(s);
	bOvrImages.Read(s);
	bSourceRelativePerSign.Read(s);
	bFacebookLink.Read(s);

	dsImageDir.Read(s);
	dsCssDir.Read(s);
	dsFontDir.Read(s);
	sDefFonts.Read(s);
				// images
	imageWidth.Read(s);
	imageHeight.Read(s);
	imageSizesLinked.Read(s);
	thumbWidth.Read(s);
	thumbHeight.Read(s);
	doNotEnlarge.Read(s);
	bCropThumbnails.Read(s);
	bDistrortThumbnails.Read(s);
	// image decoration
	imageBorder.Read(s);
	imagePadding.Read(s);

	iconToTopOn.Read(s);
	iconInfoOn.Read(s);
			// Google Analytycs
	googleAnalyticsOn.Read(s);
	googleAnalTrackingCode.Read(s);
			// latest uploads
	generateLatestUploads.Read(s);
	newUploadInterval.Read(s);
	nLatestCount.Read(s);
	styleIndex.Read(s);

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
	// Debug
	bDebugging.Read(s);

	if (CONFIGS_USED::indexOfLastUsed >= 0 && dsSrc.IsEmpty())
		dsSrc.v = CONFIGS_USED::lastConfigs[CONFIGS_USED::indexOfLastUsed];
	QString qs = CONFIGS_USED::NameForConfig(true, ".ini");
	if(!QFile::exists(qs))
	{
		savelItemsAlways = true;		// when the new file is created there are no 
		config._WriteIni(qs);					// already written peopwerties
		savelItemsAlways = false;
	}
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
											// sepcial data
	bAddTitlesToAll.Write(s);
	bAddDescriptionsToAll.Write(s);
	bLowerCaseImageExtensions.Write(s);
	bMenuToContact.Write(s);
	bMenuToAbout.Write(s);
	bMenuToDescriptions.Write(s);
	bMenuToToggleCaptions.Write(s);

	bRightClickProtected.Write(s);
	bCanDownload.Write(s);
	bForceSSL.Write(s);
	bOvrImages.Write(s);
	bSourceRelativePerSign.Write(s);
	bFacebookLink.Write(s);

	dsImageDir.Write(s);
	dsCssDir.Write(s);
	dsFontDir.Write(s);
	sDefFonts.Write(s);
				// images
	imageWidth.Write(s);
	imageHeight.Write(s);
	imageSizesLinked.Write(s);
	thumbWidth.Write(s);
	thumbHeight.Write(s);
	doNotEnlarge.Write(s);
	bCropThumbnails.Write(s);
	bDistrortThumbnails.Write(s);
	// image decoration
	imageBorder.Write(s);
	imagePadding.Write(s);
	
	iconToTopOn.Write(s);
	iconInfoOn.Write(s);
			// Google Analytycs
	googleAnalyticsOn.Write(s);
	googleAnalTrackingCode.Write(s);
			// latest uploads
	generateLatestUploads.Write(s);
	newUploadInterval.Write(s);
	nLatestCount.Write(s);
	styleIndex.Write(s);
	
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

	if (__bClearChangedFlag)
		ClearChanged();
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
	CONFIGS_USED::Write();		// save data in program directory
	_WriteIni(falconG_ini);		// last used data into there as well

	QString p, n;
	SeparateFileNamePath(dsSrc.ToString(), p, n);

	__bClearChangedFlag = true;				// mark all changes as saved
	_WriteIni(dsSrc.ToString() + n + ".ini");
	__bClearChangedFlag = false;

	ClearChanged();
}

QString _CTextAlign::ForStyleSheet(bool addSemiColon) const
{
	QString qs = v == alNone ? QString() : "	text-align:" + ActAlignStr();
	__AddSemi(qs, addSemiColon);
	return qs;
}
