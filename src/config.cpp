#include "config.h"
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
 * TASK: Reads configuration from ini or struct files
 * PARAMS:	sExt - extension (.ini or .struct)
 *					defaults: 'falconG.ini' and 'gallery.struct'
 * GLOBALS:
 * RETURNS:	full path name of config file
 * REMARKS: - if there was a last used directory read from it
 *				else read from program directory
 *-------------------------------------------------------*/
QString CONFIGS_USED::NameForConfig(QString sExt)
{
	QString sDefault;
	if (sExt == ".ini")
		sDefault = falconG_ini;
	else
		sDefault = "gallery.struct";

	QString sIniName, p, n, s;				  // if there was a last used directory read from it
	s = indexOfLastUsed >= 0 && indexOfLastUsed < lastConfigs.size() ? lastConfigs[indexOfLastUsed] : sDefault;
	SeparateFileNamePath(s, p, n);	// cuts '/' from name
	if (s[s.length() - 1] != '/')
		s += "/";
	if (n.isEmpty())
		sIniName = sDefault;		 //	 'falconG.ini' (in program folder)
	else
		sIniName = s + n + sExt;	 // = <path w.o. last folder name from 'falcong.ini'>/<folder>/<folder>.ini

	if (!QFile::exists(sIniName))	 
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
	if(dir[dir.length()-1] != '/')
		dir += '/';
	if( (sub.length() > 2 && sub[1] == ':') || !config.bSourceRelativePerSign) 
	{
		if (QDir::isAbsolutePath(sub))	// then it cannot be my sub directory
			return *const_cast<_CDirStr *>(&subdir);	// so the result will be it
	}
	if(sub[0] == '/')
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
 * TASK: assign a QString to a CColor and modify opacity
 * EXPECTS: str - QString: formats xxx or xxxxxx or #xxx or #xxxxxx, where all
 *				 x's are any hexadecimaL DIGIT
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
	if (_opacity != opac)
		_opacity = opac;
	_Prepare();
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
	if (_colorName.toLower() != s.toLower())
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
	if (s.length() > 0 && s[0] != '#')
		s = QChar('#') + s;
	if (s.length() != 4 && s.length() != 7)
		return false;
	int n;
	for (int i = 1; i < s.length(); ++i)
	{
		n = s[i].toLower().unicode();
		if (n < '0' || (n > '9' && (n < 'a' || n > 'f')))
			return false;
	}
	return true;
}

// --------------------------- _CFont ------------------------------

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

void _CFont::SetFeature(int feat, bool single)
{
	if (single)
		feat = Features() | feat;

	_details[2] = QString().setNum(feat);
	_Prepare();
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
{				  // only add first line size if used or different then line size
	int n = _details[3] == "1" && _details[1] != _details[4] ? 5 : 4;
	v = _details[0];
	for (int i = 1; i < n; ++i)
		v += "|" + _details[i];
}

void _CFont::_Setup()
{
	_details = v.split(QChar('|'));		   
	switch (_details.size())		// No breaks!
	{								// element 0 is family
		case 1: _details.push_back("10pt");
		case 2: _details.push_back("0");
		case 3: _details.push_back("0");
		case 4: _details.push_back("10pt");
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

	v = _details[0];
	for (int i = 1; i < _details.size(); ++i)
		v += "|" + _details[i];
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
void _CShadow::Set(int which, int val)	// which: 1: horiz, 2: vert, 3:blur !
{
	if(which < 3)		// these always exist
		_details[which] = QString().setNum(val);
	else   // 3 or 4   : blur or color may or may not exist
	{
		if (which == _ixBlur)
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
 * EXPECTS:	which - 0..2 - which of the 3 stops
 *			pc - coordinate in percent relative to top
 *			clr - color QString for stop, format is #xxx or #xxxxxx
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *--------------------------------------------------------------------------*/
void _CGradient::Set(int which, int pc, QString clr) 
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

void _CGradient::_Setup()
{
	QStringList qsl = v.split(QChar('|'));	// <used = 1>|<positionTop(%)>|<color Top>|<positionMiddle(%)>|<color Middle>|<positionBottom(%)>|<color bottom>
							// index in qsl		  0			    1			   2			  3			        4				   5				6				
	_internal = true;
	used = qsl[0].toInt();
	for (int i = 1; i < 7; i += 2)
	{
		if (i + 1 < qsl.size())
			Set(i / 2, qsl[i].toInt(), qsl[i + 1]);
		else
			Set(i / 2, 0, "#000000");
	}
	_internal = false;
}

void _CGradient::_Prepare()
{
	v = QString("%1|%2|%3|%4|%5|%6").arg(used)
									.arg(gs[0].percent).arg(gs[0].color)
									.arg(gs[1].percent).arg(gs[1].color)
									.arg(gs[2].percent).arg(gs[2].color);
}


void _CBorder::_Setup()
{
	QStringList qsl = v.split(QChar('|'));
	used = qsl[0].at(0) == '1';
	color = qsl[1];
	if (color.at(0) != '#')
		color = "#" + color;
	width = qsl[2].toInt();
}

void _CBorder::_Prepare()
{
	v = QString("%1|%1|%1").arg(used ? 1 : 0).arg(color).arg(width);
}

bool _CElem::Changed() const
{
	bool res = color.Changed() | background.Changed() | font.Changed();
	if(_bMayShadow)
	 	res |= shadow.Changed() | border.Changed();
	if(_bMayGradient)	 
		res |= gradient.Changed() ;
	return res;
}

void _CElem::ClearChanged()
{
	color.ClearChanged();
	background.ClearChanged();
	font.ClearChanged();
	shadow.ClearChanged();
	gradient.ClearChanged();
	border.ClearChanged();
}

void _CElem::ColorsAndFontsFrom(const _CElem& other)
{
	_CFG_ITEM::operator=(other);
	color = other.color; 
		color.vd = color.v;				// and this will become the default
	background = other.background;
		background.vd = background.v;	// for when no value is giveb
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
		if(_bMayShadow)
		{	
			shadow.Write(s);
			border.Write(s);
		}
		if(_bMayGradient)
			gradient.Write(s);
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
		if (_bMayShadow)
		{
			shadow.Read(s);
			border.Read(s);
		}
		if(_bMayGradient)
			gradient.Read(s);
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
		s.setValue("color", wm.color);
		s.setValue("background", wm.background);
		s.setValue("shadow", wm.shadowColor);
		s.setValue("opacity", wm.opacity);

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
		wm.color = s.value("color", 0xffffff).toInt();
		wm.background = s.value("background", -1).toInt();
		wm.shadowColor = s.value("shadow", -1).toInt();
		wm.opacity = s.value("opacity", 100).toInt();
		wm.font.setFamily(s.value("family", "").toString());
		wm.font.setPointSize(s.value("size", 16).toInt());
		wm.font.setWeight(s.value("weight", QFont::Bold).toInt());
		wm.font.setItalic(s.value("italic", false).toBool());
	s.endGroup();


	if (!group.isEmpty())
		s.endGroup();
}

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
	AlbumTitle.ClearChanged();
	AlbumDesc.ClearChanged();
	Section.ClearChanged();
	ImageTitle.ClearChanged();
	ImageDesc.ClearChanged();
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

	AlbumTitle = cfg.AlbumTitle;		// "Hungary"
	AlbumDesc = cfg.AlbumDesc;			// "Hungary"
	Section = cfg.Section;			// "Images" or "Albums"
	ImageTitle = cfg.ImageTitle;		// not the image itself, just the texts!
	ImageDesc = cfg.ImageDesc;			// 
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
	CONFIGS_USED::parent = this;
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

	QString sIniName = CONFIGS_USED::NameForConfig(".ini");

	QSettings s(sIniName, QSettings::IniFormat);
	s.setIniCodec("UTF-8");

	// directories
	dsSrc.Read(s);
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
	Menu.ColorsAndFontsFrom(Web);
	Lang.ColorsAndFontsFrom(Web);
	SmallGalleryTitle.ColorsAndFontsFrom(Web);
	GalleryTitle.ColorsAndFontsFrom(Web);
	GalleryDesc.ColorsAndFontsFrom(Web);
	AlbumTitle.ColorsAndFontsFrom(Web);
	AlbumDesc.ColorsAndFontsFrom(Web);
	Section.ColorsAndFontsFrom(Web);
	ImageTitle.ColorsAndFontsFrom(Web);
	ImageDesc.ColorsAndFontsFrom(Web);


	Menu.Read(s);
	Lang.Read(s);
	SmallGalleryTitle.Read(s);
	GalleryTitle.Read(s);
	GalleryDesc.Read(s);
	AlbumTitle.Read(s);
	AlbumDesc.Read(s);
	Section.Read(s);
	ImageTitle.Read(s);
	ImageDesc.Read(s);
				// 	Watermarks
	waterMark.Read(s);
	// Debug
	bDebugging.Read(s);

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

	Web.Write(s);
	Menu.Write(s);
	Lang.Write(s);
	SmallGalleryTitle.Write(s);
	GalleryTitle.Write(s);
	GalleryDesc.Write(s);
	AlbumTitle.Write(s);
	AlbumDesc.Write(s);
	Section.Write(s);
	ImageTitle.Write(s);
	ImageDesc.Write(s);
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