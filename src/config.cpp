#include "config.h"
int CONFIGS_USED::maxSavedConfigs;		// last 10 configuration directory is stored
QStringList CONFIGS_USED::lastConfigs;

int CONFIGS_USED::indexOfLastUsed;		// this was the last one used
CONFIG *CONFIGS_USED::parent = nullptr;

static bool __bClearChangedFlag = false;	// set to true to clear the changed flag after writing the configuration

					// global variables for configuration 
CONFIG config,		// must be here because _Changed uses it
	   configSave;

// new ACONFIG based configuration
QString AconfigKindToString(ACONFIG_KIND kind)
{
	switch (kind)
	{
		case ackBool: return "bool field";
		case ackInt:  return "int field";
		case ackReal: return "real field";
		case ackText: return "text field";
		case ackNone:
		default:		return "undefined";
	}
}


void BOOL_FIELD::Store(QSettings &s)
{
	if (value != defVal)
	{
		s.beginGroup(name);
			s.setValue("kind", "b");
			s.setValue("value",value);
			s.setValue("default", defVal);
		s.endGroup();
	}
	changed = false;
}

void BOOL_FIELD::Retrieve(QSettings & s)
{
	s.beginGroup(name);
		if(s.value("kind").toString() != "b")
			throw "bool-field type mismatch";
		defVal = s.value("default", false).toBool();
		value = s.value("value", defVal).toBool();
	s.endGroup();

	changed = false;
}


void INT_FIELD::Store(QSettings &s)
{
	if (value != defVal)
	{
		s.beginGroup(name);
			s.setValue("kind", "i");
			s.setValue("value", value);
			s.setValue("default", defVal);
		s.endGroup();
	}
	changed = false;
}

void INT_FIELD::Retrieve(QSettings & s)
{
	s.beginGroup(name);
		if (s.value("kind").toString() != "i")
			throw "int-field type mismatch";
		defVal = s.value("default", 0).toInt();
		value = s.value("value", defVal).toInt();
	s.endGroup();

	changed = false;
}

void REAL_FIELD::Store(QSettings &s)
{
	if (value != defVal)
	{
		s.beginGroup(name);
			s.setValue("kind", "r");
			s.setValue("value", value);
			s.setValue("default:", defVal);
		s.endGroup();
	}
	changed = false;
}

void REAL_FIELD::Retrieve(QSettings & s)
{
	s.beginGroup(name);
		if (s.value("kind").toString() != "r")
			throw "real-field type mismatch";
		defVal = s.value("default", 0.0).toDouble();
		value = s.value("value", defVal). toDouble();
	s.endGroup();

	changed = false;
}

void TEXT_FIELD::Store(QSettings &s)
{
	if (value != defVal)
	{
		s.beginGroup(name);
			s.setValue("kind", "t");
			s.setValue("value", value);
			s.setValue("default:", defVal);
		s.endGroup();
	}
	changed = false;
}

void TEXT_FIELD::Retrieve(QSettings & s)
{
	s.beginGroup(name);
		if (s.value("kind").toString() != "t")
			throw "text-field type mismatch";
		defVal = s.value("default", false).toString();
		value  = s.value("value",  defVal).toString();
	s.endGroup();

	changed = false;
}

void COMPOUND_FIELD::Store(QSettings &s)
{
	BOOL_FIELD *pb;

	s.beginGroup(name);
		s.setValue("kind", "c");
		s.setValue("value", value);
		s.setValue("default:", defVal);
		for (auto pf : _fields)
		{
			if(pf->kind != ackNone)
				pf->Store(s);
		}
	s.endGroup();

	changed = false;
}

void COMPOUND_FIELD::Retrieve(QSettings & s)
{
	s.beginGroup(name);
		if (s.value("kind").toString() != "b")
			throw "compound-field type mismatch";
		defVal = s.value("default", false).toString();
		value = s.value("value", defVal).toString();



	s.endGroup();

	changed = false;
}



BOOL_FIELD& BOOL_FIELD::operator=(bool newVal)
{
	if (changed = (newVal != value))
	{
		value = newVal;
		if (parent)
			static_cast<COMPOUND_FIELD *>(parent)->SetChanged(true);
	}
	return *this;
}


INT_FIELD& INT_FIELD::operator=(int newVal)
{
	if (changed = (newVal != value))
	{
		value = newVal;
		if (parent)
			parent->SetChanged(true);
	}
	return *this;
}

REAL_FIELD& REAL_FIELD::operator=(double newVal)
{
	if (changed = (newVal != value))
	{
		value = newVal;
		if (parent)
			parent->SetChanged(true);
	}
	return *this;
}

TEXT_FIELD& TEXT_FIELD::operator=(QString newVal)
{
	if (changed = (newVal != value))
	{
		value = newVal;
		if (parent)
			parent->SetChanged(true);
	}
	return *this;
}

void COMPOUND_FIELD::_CopyLists(const COMPOUND_FIELD &other)
{
	changed = other.changed;
	_intList = other._intList;
	_boolList = other._boolList;
	_realList = other._realList;
	_textList = other._textList;
	_compList = other._compList;

	_fields.clear();
	// set up new _fields
	for (auto a : _boolList)
		_fields[a.name] = &a;
	for (auto a : _intList)
		_fields[a.name] = &a;
	for (auto a : _realList)
		_fields[a.name] = &a;
	for (auto a : _textList)
		_fields[a.name] = &a;
	for (auto a : _compList)
		_fields[a.name] = &a;

}
COMPOUND_FIELD &COMPOUND_FIELD::operator=(const COMPOUND_FIELD &other)
{
	TEXT_FIELD::operator=(other);
	_CopyLists(other);

	return *this;
}

void COMPOUND_FIELD::AddBoolField(QString name, bool defVal, bool val)
{
	BOOL_FIELD intf(name, defVal, val, this);
	_boolList.push_back(intf);
	if (_fields.count(name) && _fields[name]->kind != ackBool)
		throw "type mismatch";
	_fields[name] = &_boolList.back();
}
void COMPOUND_FIELD::AddIntField(QString name, int defVal, int val)
{
	INT_FIELD intf(name, defVal, val, this);
	_intList.push_back(intf);
	if (_fields.count(name) && _fields[name]->kind != ackInt)
		throw "type mismatch";
	_fields[name] = &_intList.back();
}
void COMPOUND_FIELD::AddRealField(QString name, double defVal, double val)
{
	REAL_FIELD intf(name, defVal, val, this);
	_realList.push_back(intf);
	if (_fields.count(name) && _fields[name]->kind != ackReal)
		throw "type mismatch";
	_fields[name] = &_realList.back();
}
void COMPOUND_FIELD::AddTextField(QString name, QString defVal, QString val)
{
	TEXT_FIELD textf(name, defVal, val, this);
	size_t n = _textList.size();
	_textList.push_back(textf);
	if (_fields.count(name) && _fields[name]->kind != ackText)
		throw "type mismatch";
	_fields[name] = &_textList.back();
}
void COMPOUND_FIELD::AddCompundField(QString name, QString defVal, QString val)
{
	COMPOUND_FIELD cmpf(name, defVal, val, this);
	size_t n = _textList.size();
}

size_t COMPOUND_FIELD::Size(ACONFIG_KIND kind) const
{
	switch (kind)
	{
		case ackBool:	return _boolList.size();
		case ackInt:	return _intList.size();
		case ackReal:	return _realList.size();
		case ackText:	return _textList.size();
		case ackComp:	return _compList.size();
		default:		return _fields.size();
	}
}

// ************* ACONFIG **************

void ACONFIG::Load(QString iniName)	// from ini file
{
	QSettings s(iniName, )
	if( !qf.open(QFile::ReadOnly) )
		throw "can't open";
	_ifs.setDevice(&qf);

	QString qs;
	QStringList sl;
	while (!_ifs.atEnd())
	{
		_ifs.readLineInto(&qs);
		sl = qs.split(',');

		if (sl.size() != 4) // <name>,<type>,<value>,<default>
			throw "bad string";
		switch (sl[1][0].unicode())
		{
		case 'b': AddBoolField(sl[0], sl[2].toInt(), sl[3].toInt()); break;
		case 'i': AddIntField( sl[0], sl[2].toInt(), sl[3].toInt()); break;
		case 'r': AddRealField(sl[0], sl[2].toDouble(), sl[3].toDouble()); break;
		case 't': AddTextField(sl[0], sl[2], sl[3]);
		default: break;
		}
	}
	qf.close();

	_changed = false;
}

void ACONFIG::Store(QString fname)
{
	QSettings stt(falconG_ini, QSettings::IniFormat);	// in program directory

	for (auto p : _fields)
		p->Store(stt);

	_changed = false;
}

FIELD_BASE *COMPOUND_FIELD::operator[](QString fieldn)
{

	if (_fields.count(fieldn))
		return _fields[fieldn];
	return nullptr;
}

// DEBUG
void COMPOUND_FIELD::DumpFields(ACONFIG_KIND kind, QString file)
{
	QFile qfs;
	QTextStream *pts, ofs;

	if (!file.isEmpty())			// then print in file
	{
		qfs.setFileName(file);
		if (qfs.open(QFile::WriteOnly))
		{
			ofs.setDevice(&qfs);
			pts = &ofs;
		}
	}
	else
		pts = &QTextStream(stdin);

	FIELD_BASE *pf;
	for (auto a : _fields)
	{
		pf = nullptr;
		switch (kind)
		{
		case ackBool: if (a->kind == ackBool) pf = a; break;
		case ackInt:  if (a->kind == ackInt)  pf = a; break;
		case ackReal: if (a->kind == ackReal) pf = a; break;
		case ackText: if (a->kind == ackText) pf = a; break;
		case ackNone: pf = a; break;
		default: break;
		}
		*pts << "name: " << pf->name << ", kind: " << AconfigKindToString(pf->kind)
			<< ", changed: " << (pf->changed ? "yes" : "no") << ", default: ";
		switch (pf->kind)
		{
		case ackBool: *pts << static_cast<BOOL_FIELD *>(pf)->defVal << ", value: " << static_cast<BOOL_FIELD *>(pf)->value << "\n"; break;
		case ackInt:  *pts << static_cast<INT_FIELD *> (pf)->defVal << ", value: " << static_cast<INT_FIELD *>(pf)->value << "\n"; break;
		case ackReal: *pts << static_cast<REAL_FIELD *>(pf)->defVal << ", value: " << static_cast<INT_FIELD *>(pf)->value << "\n"; break;
		case ackText: *pts << static_cast<TEXT_FIELD *>(pf)->defVal << ", value: " << static_cast<TEXT_FIELD *>(pf)->value << "\n"; break;
		}
	}
	if (file.isEmpty())	// wrote to stdout
		delete pts;		// ???
}

//--------------------------------------------------------------
// writes field into open file in format
// <name>,<kind (one LC letter)>,<default>,<actual value>

void ACONFIG::_Write(FIELD_BASE * pf)
{
	pf->Store(_ofs);
	pf->changed = false;
}







/*=====================_Changed flags ====================================================*/
bool _Changed::operator=(bool val) 
{
	if (val != _changed)
	{
		_changed = val; 
		if(val)
			config.changed = true;
	} 
	return val;
}

bool _Changed::operator!=(bool val)
{
	return (val != _changed);
}

bool _Changed::operator==(bool val)
{
	return (val == _changed);
}

bool _Changed::operator|=(bool val)
{
	val |= _changed;
	return operator=(val);
}

bool _Changed::operator&=(bool val)
{
	val &= _changed;
	return operator=(val);
}

/*========================== CONFIG_USED ====================*/
void CONFIGS_USED::Read()
{
	lastConfigs.clear();
	QSettings s(falconG_ini, QSettings::IniFormat);	// in program directory
	s.beginGroup("config_save");
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
	s.endGroup();
}

void CONFIGS_USED::Write()
{
	QString sLast = parent->dsSrc.str.trimmed();
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
		sIniName = sDefault;
	else
		sIniName = s + n + sExt;

	if (!QFile::exists(sIniName))
	{
		sIniName = s + sDefault;
		if (!QFile::exists(sIniName))
			sIniName = sDefault;
	}

	return sIniName;
}



// ! If you add any new field to CONFIG you need to add the name, 
//		the default value the read and write calls and make changes
//		to PopulateFromConfig() in 'falconG.cpp'

bool CONFIG::_ChangedFlag::operator=(bool flag)
{
	_state = flag;
	if (!flag)		// not changed: clear all 'changed' flags
	{
		_parent->dsApplication.changed = false;
		_parent->dsSrc.changed = false;
		_parent->dsGallery.changed = false;
		_parent->sGalleryLanguages = false;
		_parent->dsGRoot.changed = false;
		_parent->dsAlbumDir.changed = false;
		_parent->dsCssDir.changed = false;
		_parent->dsFontDir.changed = false;
		_parent->dsImageDir.changed = false;
		_parent->dsThumbDir.changed = false;

		_parent->sUplink.changed = false;
		_parent->iconUplink.changed = false;
		_parent->sMainPage.changed = false;
		_parent->sDescription.changed = false;
		_parent->sKeywords.changed = false;
		
		_parent->bGenerateAll.changed = false;
		_parent->bButImages.changed = false;
		_parent->bAddTitlesToAll.changed = false;
		_parent->bAddDescriptionsToAll.changed = false;
		_parent->bLowerCaseImageExtensions.changed = false;
		_parent->bReadJAlbum.changed = false;
		_parent->bMenuToContact.changed = false;
		_parent->bMenuToAbout.changed = false;
		_parent->bMenuToDescriptions.changed = false;
		_parent->bMenuToToggleCaptions.changed = false;
		_parent->bFacebookLink.changed = false;
		_parent->doNotEnlarge.changed = false;
		_parent->bCropThumbnails.changed = false;
		_parent->bDistrortThumbnails.changed = false;
		_parent->imageBorder.changed = false;

		_parent->bCanDownload.changed = false;
		_parent->bForceSSL.changed = false;
		_parent->bRightClickProtected.changed = false;
		_parent->bOvrImages.changed = false;
		_parent->bSourceRelativePerSign.changed = false;

		_parent->sServerAddress.changed = false;
		_parent->sDefFonts.changed = false;
		_parent->sBaseName.changed = false;
		_parent->sMailTo.changed = false;
		_parent->sAbout.changed = false;

		_parent->generateLatestUploads.changed = false;
		_parent->newUploadInterval.changed = false;
		_parent->nLatestCount.changed = false;

		_parent->googleAnalyticsOn.changed = false;
		_parent->googleAnalTrackingCode.changed = false;

		_parent->sGalleryTitle.changed = false;
		_parent->sGalleryLanguages.changed = false;
		_parent->GalleryTitle.ClearChanged();
		_parent->GalleryDesc.ClearChanged();

		_parent->Web.ClearChanged();
		_parent->Lang.ClearChanged();
		_parent->Menu.ClearChanged();

		_parent->SmallGalleryTitle.ClearChanged();
		_parent->AlbumTitle.ClearChanged();
		_parent->AlbumDesc.ClearChanged();
		_parent->Section.ClearChanged();
		_parent->ImageTitle.ClearChanged();
		_parent->ImageDesc.ClearChanged();

		_parent->images.ClearChanged();
		_parent->imageWidth.changed = false;
		_parent->imageHeight.changed = false;
		_parent->thumbWidth.changed = false;
		_parent->thumbHeight.changed = false;
		_parent->imageSizesLinked.changed = false;

		_parent->iconToTopOn.changed = false;
		_parent->iconInfoOn.changed = false;
		_parent->imageBorder.changed = false;
		_parent->imageBorder.color.changed = false;

		_parent->waterMark.changed = false;
	}
	return flag;
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
	if (str != s) 
	{ 
		str = s; 
//		changed = true;
	} 
	return str; 
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
	 if (str == qs )
		 return str;

	 str = qs;
	return str;
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
	tmp.str = sub + '/';						// add back the '/'
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
	if (set != s) 
	{ 
		set = s; 
		changed = true;		// ? why was it commented out
	} 
	return set; 
}

/*===========================================================================
 * TASK: assig a int to a CInt
 * EXPECTS: s - int
 * GLOBALS: config
 * RETURNS: the int
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
int &_CInt::operator=(const int s)
{
	if (val != s)
	{
		val = s;
		changed = true;		// ? why was it commented out
	} return val;
}

/*===========================================================================
 * TASK: assign a QString to a CColor but do not modify opacity
 * EXPECTS: s - QString: formats xxx or xxxxxx or #xxx or #xxxxxx, where all
 *				 x's are any hexadecimaL DIGIT
 * GLOBALS: config
 * RETURNS: color QString: always starts with a '#'
 * REMARKS:	- may modify the 'changed' field of global variable 'config'
 *			- no symbolic color names
 *			- any combination of upper and lower case hexadecimal numbers
 *			- if the QString is #xyz, then it is the same as #xxyyzz
*--------------------------------------------------------------------------*/
QString _CColor::operator=(QString s)
{
	if (s.length() > 0 && s[0] != '#')
		s = QChar('#') + s;
	if (s.length() != 4 && s.length() != 7)
		return "";
	int n;
	for (int i = 1; i < s.length(); ++i)
	{
		n = s[i].toLower().unicode();
		if (n < '0' || (n > '9' && (n < 'a' || n > 'f')))
			return "";
	}
	if (name.toLower() != s.toLower())
	{
		name = s;
		changed = true;		// ? why was it commented out
	}
	return name;
}
/*===========================================================================
 * TASK: drop the leading '# from the name
 * EXPECTS: nothing
 * GLOBALS: -
 * RETURNS: name without the leading '#'
 * REMARKS: 
 *--------------------------------------------------------------------------*/
_CColor::operator QString() const { return name.mid(1); }	// drop the '#'

/*===========================================================================
 * TASK: assign a QString to a CColor and modify opacity
 * EXPECTS: str - QString: formats xxx or xxxxxx or #xxx or #xxxxxx, where all
 *				 x's are any hexadecimaL DIGIT
 *		   opac - opacity in percent 0..100
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS:	- may modify the 'changed' field of global variable 'config'
 *			- no symbolic color names
 *			- any combination of upper and lower case hexadecimal numbers
 *			- if the QString is #xyz, then it is the same as #xxyyzz
 *--------------------------------------------------------------------------*/
void _CColor::Set(QString str, int opac)
{ 
	*this = str; 
	if (opacity != opac)
	{
		opacity = opac;
		changed = true;
	}
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
void _CFont::Set(QString fam, QString siz, int feat)
{
	if (fam == family && sizestr == siz && features == feat)
		return;
	family = fam; sizestr = siz; 
	features = feat;
	changed = true;
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
	if (!features) 
		return; 
	features = 0; 
	changed = true;
}

/*===========================================================================
 * TASK: set top,right,bttom or left shadow
 * EXPECTS: which - which side
 *		   value - size of shadow in pixels
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
*--------------------------------------------------------------------------*/
void _CShadow::Set(int which, int val)
{
	if (values[which] == val)
		return;
	values[which] = val;
	changed = true;
}
/*===========================================================================
 * TASK: set all of the shadows
 * EXPECTS: top,ight,bottom,left - size of shadow in pixels
 * GLOBALS: config
 * RETURNS: nothing
 * REMARKS: may modify the 'changed' field of global variable 'config'
 *          member 'useEver' is not set here. set it manually 
*--------------------------------------------------------------------------*/
void _CShadow::Set(int horiz, int vert, int blur, QString clr, bool used)
{
	if (values[0] == horiz &&			// horizontal
		values[1] == vert &&		// vertical
		values[2] == blur &&		// blur radius
		use == used &&				// used
		(clr.isEmpty() || (!clr.isEmpty() && clr == color)) )
		return;

	values[0] = horiz;			   // horizontal
	values[0] = vert;			   // vertical
	values[0] = blur;			   // blur radius
	use = used;
	if (!clr.isEmpty())	
		color = clr;
	changed = true;
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
	if (percent == pc && color == clr) 
		return false; 
	percent = pc; color = clr;
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
 *          member 'useEver' is not set here. set it manually
 *--------------------------------------------------------------------------*/
void _CGradient::Set(int which, int pc, QString clr) 
{ 
	changed |= gs[which].Set(pc, clr); 
	useEver = true;
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
 *          member 'useEver' is not set here. set it manually
 *--------------------------------------------------------------------------*/
void _CGradient::Set(bool usd, int topStop, QString topColor, int middleStop, QString middleColor, int bottomStop, QString bottomColor, bool chg)
{
	gs[0].Set(topStop, topColor);
	gs[1].Set(middleStop, middleColor);
	gs[2].Set(bottomStop, bottomColor);
	used = usd;
	config.changed = changed |= chg;
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
	changed = ((WaterMark&)cwm.wm != (WaterMark&)wm);	//'const'
	wm = cwm.wm;
	used = cwm.used;
	return *this;
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
	bSourceRelativePerSign.changed = cfg.bSourceRelativePerSign;

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
	images = cfg.images;
	imageWidth = cfg.imageWidth;
	imageHeight = cfg.imageHeight;
	thumbWidth = cfg.thumbWidth;
	thumbHeight = cfg.thumbHeight;
	imageSizesLinked = cfg.imageSizesLinked;
	// image decoration
	iconToTopOn = cfg.iconToTopOn;
	iconInfoOn = cfg.iconInfoOn;

	imageBorder = cfg.imageBorder;
}
//------------ ******************* ---------------------------
static inline void __ConfigReadInt( QSettings &s, _CInt &name,int defval)	
{ 
	name = s.value(name.nameStr,defval).toInt();	  
}
static inline void __ConfigReadStr( QSettings &s, _CString &name, QString defval)	
{ 
	name = s.value(name.nameStr,defval).toString(); 
}
static inline void __ConfigReadBool(QSettings &s, _CBool &name, bool defval)
{ 
	name =s.value(name.nameStr, defval).toBool(); 
}

/*============================================================================
* TASK: read configuration settings for elem
* EXPECTS: s - open settings
*		   elem - elem
*			elem default parameters
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __ConfigReadBorder(QSettings &s, _CBorder &name, bool BORDERON, int BORDERWIDTH, QString BORDERCOLOR)
{
	s.beginGroup(name.nameStr);
		name.used = s.value("used", BORDERON).toBool();
		name.width = s.value("width", BORDERWIDTH).toInt();
		name.color = s.value("color", BORDERCOLOR).toString();
	s.endGroup();
}

/*============================================================================
* TASK: read configuration settings for elem
* EXPECTS: s - open settings 
*		   elem - elem
*			elem default parameters
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __ReadElem(QSettings &s, _CElem &elem, QString COLOR, 
								QString BCKCOLOR, int OPACITY, int BOPACITY, 
								QString FONTFAMILY, QString FONTSIZE, int FEATURES, 
								bool BORDERON, int BORDERWIDTH, QString BORDERCOLOR)
{
	s.beginGroup(elem.nameStr ); 
		s.beginGroup("color");	
			elem.color.name = s.value("name", COLOR).toString(); 
			elem.color.opacity = s.value("opacity", OPACITY).toInt();	
		s.endGroup();	
		s.beginGroup("background");	
			elem.background.name = s.value("name", BCKCOLOR).toString(); 
			elem.background.opacity = s.value("opacity", BOPACITY).toInt();	
		s.endGroup();	
		s.beginGroup("font");	
			elem.font.family = s.value("family", FONTFAMILY).toString(); 
			elem.font.sizestr = s.value("size", FONTSIZE).toString(); 
			elem.font.features = s.value("features", FEATURES).toInt(); 
		s.endGroup(); 
		s.beginGroup("border");	
			elem.border.used = s.value("used", BORDERON).toBool(); 
			elem.border.width = s.value("width", BORDERWIDTH).toInt(); 
			elem.border.color = s.value("color", BORDERCOLOR).toString(); 
		s.endGroup(); 
	s.endGroup();
	elem.ClearChanged();
}

/*============================================================================
* TASK: read configuration settings for elem
* EXPECTS: s - open settings
*		   elem - elem
*			elem default parameters
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __ReadElemWShadow(QSettings &s, _CElem & elem, QString color,
	QString bckcolor, int opacity, int bopacity,
	QString fontfamily, QString fontsize, int features,
	bool borderon, int borderwidth, QString bordercolor,
	int SHHORIZ, int SHVERT, int SHBLUR, QString SHCOLOR, bool SHUSED)
{
	__ReadElem(s, elem, color, bckcolor, opacity, bopacity, fontfamily, fontsize, 
						features, borderon, borderwidth, bordercolor);
	s.beginGroup(elem.nameStr);
		s.beginGroup("shadow");
			elem.shadow.Set(s.value("horiz", SHHORIZ).toInt(), s.value("vert", SHVERT).toInt(), 
							s.value("blur", SHBLUR).toInt(),  
							s.value("color", SHCOLOR).toString(), s.value("use",SHUSED).toBool());
		s.endGroup();
		elem.shadow.changed = false;
	if (elem.shadow.values[0] | elem.shadow.values[1] | elem.shadow.values[2] | elem.shadow.values[3])
		elem.shadow.useEver = elem.shadow.use = true;
	s.endGroup();
}

/*============================================================================
* TASK: read configuration settings for elem
* EXPECTS: s - open settings
*		   elem - elem
*			elem default parameters
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __ReadElemWGradient(QSettings &s, _CElem & elem, QString color,
	QString bckcolor, int opacity, int bopacity,
	QString fontfamily, QString fontsize, int features,
	bool borderon, int borderwidth, QString bordercolor,
	int shhoriz, int shvert, int shblur, QString shcolor, bool shused,
	bool GRUSED, int GRTOPSTP, QString GRTOPCOLOR, int GRMIDDLESTP, 
	QString GRMIDDLECOLOR, int GRBOTTOMSTP, QString GRBOTTOMCOLOR)
{
	__ReadElemWShadow(s, elem, color, bckcolor, opacity, bopacity, fontfamily, 
						fontsize, features, borderon, borderwidth, bordercolor, 
										shhoriz, shvert, shblur, shcolor, shused);
	s.beginGroup(elem.nameStr);
		s.beginGroup("gradient");
			elem.gradient.Set(s.value("used", GRUSED).toBool(),
							s.value("topstop", GRTOPSTP).toInt(),
							s.value("topcolor", GRTOPCOLOR).toString(), 
							s.value("middlestop", GRMIDDLESTP).toInt(), 
							s.value("middlecolor", GRMIDDLECOLOR).toString(),
							s.value("bottomstop", GRBOTTOMSTP).toInt(), 
							s.value("bottomcolor", GRBOTTOMCOLOR).toString(),
							false);  // !changed
			s.endGroup();
	s.endGroup();
	elem.gradient.useEver = true;
}
/*============================================================================
* TASK: read configuration settings for watermark
* EXPECTS: s - open settings
*		   wm =watermark
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __ReadWaterMark(QSettings &s, _CWaterMark &elem, bool WUSED,
	QString WMTEXT, QString WMORIGIN, int WMMARGINX,int WMMARGINY, int WMCOLOR,
	int WMBACKGROUND, int WMSHADOWCOLOR, int WMOPACITY,
	QString WMFAMILY, int WMFSIZE, int WMFWEIGHT, bool WMITALIC)
{
	s.beginGroup(elem.nameStr);
		elem.used = s.value("used", WUSED).toBool();
		elem.changed = false;
		elem.wm.text = s.value("text", WMTEXT).toString();

		elem.wm.origin = s.value("origin", WMORIGIN).toInt(); // low 4 bit: vertical next 4 bits: horizontal

		elem.wm.marginX = s.value("mx", WMMARGINX).toInt();
		elem.wm.marginY = s.value("my", WMMARGINY).toInt();
		elem.wm.color = s.value("color", WMCOLOR).toInt();
		elem.wm.background = s.value("background", WMBACKGROUND).toInt();
		elem.wm.shadowColor = s.value("shadow", WMSHADOWCOLOR).toInt();
		elem.wm.opacity = s.value("opacity", WMOPACITY).toInt();
		elem.wm.font.setFamily(s.value("family",WMFAMILY).toString());
		elem.wm.font.setPointSize(s.value("size", WMFSIZE).toInt());
		elem.wm.font.setWeight(s.value("weight", WMFWEIGHT).toInt());
		elem.wm.font.setItalic(s.value("italic", WMITALIC).toBool());
	s.endGroup();
}

/*============================================================================
* TASK: constructor
* EXPECTS: nothing
* RETURNS: nothing
* REMARKS: sets up names for _CElem's
*---------------------------------------------------------------------------*/
CONFIG::CONFIG()
{
	changed.SetParent(this);
	CONFIGS_USED::parent = this;

	dsSrc.nameStr = "dsSrc";				
	dsGallery.nameStr = "dsGallery";			
	dsGRoot.nameStr = "dsGRoot";			
	dsAlbumDir.nameStr = "dsAlbumDir";		
	dsCssDir.nameStr = "dsCssDir";
	dsFontDir.nameStr = "dsFontDir";
	dsImageDir.nameStr = "dsImageDir";		
	sUplink.nameStr = "sUplink";			
	iconUplink.nameStr = "iconUplink";			
	sMainPage.nameStr = "sMainPage";
	sDescription.nameStr = "sDescription";
	sKeywords.nameStr = "sKeywords";

	bGenerateAll.nameStr = "bGenerateAll";
	bButImages.nameStr = "bButImages";
	bAddTitlesToAll.nameStr = "bAddTitlesToAll";
	bAddDescriptionsToAll.nameStr = "bAddDescriptionsToAll";
	bLowerCaseImageExtensions.nameStr = "bLowerCaseImageExtensions";
	bReadJAlbum.nameStr  =  "bReadJAlbum";
	bReadFromGallery.nameStr = "bReadFromGallery";
	bSourceRelativePerSign.nameStr = "bSourceRelativePerSign";
	bMenuToAbout.nameStr = "bMenuToAbout";
	bMenuToContact.nameStr = "bMenuToContact";
	bMenuToDescriptions.nameStr = "bMenuToDescriptions";
	bMenuToToggleCaptions.nameStr = "bMenuToToggleCaptions";

	bCanDownload.nameStr = "bCanDownload";
	bForceSSL.nameStr = "bForceSSL";			
	bRightClickProtected.nameStr = "bRightClickProtected";
	bOvrImages.nameStr = "bOvrImages";

	sServerAddress.nameStr = "sServerAddress";	

	sDefFonts.nameStr = "sDefFonts";			
	sBaseName.nameStr = "sBaseName";			
	sMailTo.nameStr = "sMailTo";			
	sAbout.nameStr = "sAbout";			

	generateLatestUploads.nameStr = "generateLatestUploads";
	newUploadInterval.nameStr = "newUploadInterval";	
	nLatestCount.nameStr = "nLatestCount";			
								
	googleAnalyticsOn.nameStr = "googleAnalyticsOn";
	googleAnalTrackingCode.nameStr = "googleAnalTrackingCode";

	sGalleryDesc.nameStr = "sGalleryDesc";
	sGalleryTitle.nameStr = "sGalleryTitle";
	sGalleryLanguages.nameStr = "sGalleryLanguages";
	bFacebookLink.nameStr = "facebook";
	// Design page

	Web.nameStr = "Web";
	Lang.nameStr = "Lang";				
	Menu.nameStr = "Menu";				
	GalleryTitle.nameStr = "GalleryTitle";		
	SmallGalleryTitle.nameStr = "SmallGalleryTitle";	
	GalleryDesc.nameStr = "GalleryDesc";		
	AlbumTitle.nameStr = "AlbumTitle";			
	AlbumDesc.nameStr = "AlbumDesc";			
	Section.nameStr = "Section";			
	ImageTitle.nameStr = "ImageTitle";			
	ImageDesc.nameStr = "ImageDesc";			
	images.nameStr = "images";				
	waterMark.nameStr = "waterMark";
	imageWidth.nameStr = "imageWidth";
	imageHeight.nameStr = "imageHeight";
	thumbWidth.nameStr = "thumbWidth";
	thumbHeight.nameStr = "thumbHeight";
	imageSizesLinked.nameStr = "imageSizesLinked";
	doNotEnlarge.nameStr = "doNotEnlarge";
	bCropThumbnails.nameStr = "CropThumbnails";
	bDistrortThumbnails.nameStr = "DistrortThumbnails";
	
	iconToTopOn.nameStr = "iconToTopOn";
	iconInfoOn.nameStr = "iconInfoOn";
	imageBorder.nameStr = "imageBorder";
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
  * REMARKS:  read CONFIGS_USED first!
 *---------------------------------------------------------------------------*/
void CONFIG::Read()		// synchronize with Write!
{
	// set Default values
	// no default for dsGRoot !     dsGRoot.defStr     = "falconG/";
	dsAlbumDir.defStr  = "albums/";
	dsImageDir.defStr  = "imgs/";
	dsThumbDir.defStr  = "thumbs/";
	sUplink.defStr     =  "";		// inside dsGRoot base1hu.html, etc
	iconUplink.defStr  =  "up-icon.png";	// in 'res' directory
	dsCssDir.defStr    =  "css/";
	dsFontDir.defStr   =  "fonts/";
	sDefFonts.defStr   =  "Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif";
	sBaseName.defStr   =  "album";
	sMainPage.defStr   = "index.html";
	sDescription.defStr= "";
	sKeywords.defStr   = "";
	sAbout.defStr      =  "about.html";


	QString sIniName = CONFIGS_USED::NameForConfig(".ini");

	QSettings s(sIniName, QSettings::IniFormat);
	s.setIniCodec("UTF-8");

	// reading order: Web must come first, as its background color used as default color 
	//	for elements without theit own background color set

	// elem, COLOR, BCKCOLOR, OPACITY, BOPACITY, FONTFAMILY, FONTSIZE, features
	__ReadElem(s, Web, 
		"#ffffff", "#a03020", -100, -100, 
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif", 
		"13px", 0, /* normal */
		false, 0, 0
		);
				/* normal */
				// elem, 
				// color, bckcolor, opacity, bOpacity, 
				// fontfamily, 
				// fontsize, features, 
				//  borderon, borderwidth, bordercolor
				// shhoriz, shvert, shblur, shcolor, shused
				// used, top stop & color, middle: stop & color, bottom stop & color
	__ReadElemWGradient(s, Menu, 
		"#ffffff", config.Web.background.name, -100, -100, 
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"13px", (int)_CFont::fBold,
		false, 1, "890222",
		0, 0, 0, "",false,
		true, 0, "a90329", 40, "890222", 100, "6D0019");
		
// elem, color, bckcolor, opacity, bOpacity, fontfamily, fontsize, features, SHHORIZ, SHVERT, SHBLUR, SHCOLOR, SHUSED
	__ReadElemWShadow(s, Lang, 
		"#dddd00", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"8pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, GalleryTitle, 
		"#dddddd", config.Web.background.name, -100, -100,
		"\"Rage Italic\",Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"14pt", (int)_CFont::fBold,
		false, 0, 0,
		10, 10, 10, "#000000", false);
	__ReadElemWShadow(s, SmallGalleryTitle, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"8pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, GalleryDesc, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"10pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, AlbumTitle, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"16pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, AlbumDesc, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"8pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, Section, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Papyrus", 
		"24pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, ImageTitle, 
		"#ddd", config.Web.background.name, -100, -100,
		"\"Rage Italic\", Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"12pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);
	__ReadElemWShadow(s, ImageDesc, 
		"#ffffff", config.Web.background.name, -100, -100,
		"Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif",
		"8pt", (int)_CFont::fBold,
		false, 0, 0,
		0, 0, 0, "", false);

	__ReadWaterMark(s, waterMark, false, "Watermark", "11", 0, 0, 0xffffff, -1, -1, 100, "", 16, QFont::Bold, false);

	// "" as default means -> use global defaults if there are such but do not display them
	// directories
	__ConfigReadStr(s, dsSrc, "");			// no default
	__ConfigReadStr(s, dsGallery, "");		// no default
	__ConfigReadStr(s, dsGRoot, "");		// 'falconG'
	__ConfigReadStr(s, dsAlbumDir, "");		// 'albums'
	__ConfigReadBool(s, bOvrImages, true);  // overwrite images or ask?

	__ConfigReadStr(s, sGalleryTitle, "Andreas Falco Photography");
	__ConfigReadStr(s, sGalleryLanguages, "");
	__ConfigReadStr(s, sUplink, "");		// no defaults
	__ConfigReadStr(s, iconUplink, "up-icon.png");		// no defaults
	__ConfigReadStr(s, sMainPage, "");		// no defaults
	__ConfigReadStr(s, sDescription, "");	// no defaults
	__ConfigReadStr(s, sKeywords, "");		// no defaults
	__ConfigReadStr(s, sServerAddress, ""); // no default
	__ConfigReadStr(s, sMailTo, "");		// send user emails here
	__ConfigReadStr(s, sAbout, "");			// about page
	__ConfigReadStr(s, sBaseName, "");		// 'album'

		// sepcial data
//	__ConfigReadBool(s, bGenerateAll, false);				// unconditional save web pages - not stored
//	__ConfigReadBool(s, bButImages, false);					// unconditional save web pages
	__ConfigReadBool(s, bAddTitlesToAll, false);			// only process changed/deleted/new images
	__ConfigReadBool(s, bAddDescriptionsToAll, false);		// only process changed/deleted/new images
	__ConfigReadBool(s, bLowerCaseImageExtensions, true);	// convert extension
//	__ConfigReadBool(s, bReadJAlbum, true);
	__ConfigReadBool(s, bMenuToContact, true);
	__ConfigReadBool(s, bMenuToAbout, true);
	__ConfigReadBool(s, bMenuToDescriptions, true);
	__ConfigReadBool(s, bMenuToToggleCaptions, true);

	__ConfigReadBool(s, bSourceRelativePerSign, true);
	__ConfigReadBool(s, bRightClickProtected, false);
	__ConfigReadBool(s, bCanDownload, false);		// images download allowed
	__ConfigReadBool(s, bForceSSL, true);			// permanent transfer to SSL
	__ConfigReadBool(s, bFacebookLink, true);

	__ConfigReadStr(s, dsImageDir, "");		// img
	__ConfigReadStr(s, dsCssDir , "");		// css
	__ConfigReadStr(s, dsFontDir, "");		// font
	__ConfigReadStr(s, sDefFonts, "");		// defFonts
		// images
	__ConfigReadInt(s, imageWidth, 1920);
	__ConfigReadInt(s, imageHeight, 1080);
	__ConfigReadInt(s, thumbWidth, 600);
	__ConfigReadInt(s, thumbHeight, 400);
	__ConfigReadInt(s, imageSizesLinked, true);
	__ConfigReadBool(s, doNotEnlarge, true);
	__ConfigReadBool(s, bCropThumbnails, false);
	__ConfigReadBool(s, bDistrortThumbnails, false);
		// image decoration
	__ConfigReadBorder(s, imageBorder, true, 2, QString("eaa41e"));
		// icons
	__ConfigReadBool(s, iconToTopOn, true);
	__ConfigReadBool(s, iconInfoOn, true);
				// 	Watermarks
		// Google Analytycs
	__ConfigReadBool(s, googleAnalyticsOn, false);
	__ConfigReadStr(s, googleAnalTrackingCode, "");
	// latest uploads
	__ConfigReadBool(s, generateLatestUploads, true);
	__ConfigReadInt(s, newUploadInterval, 14);		// days
	__ConfigReadInt(s, nLatestCount, 10);	// days
	changed = false;
	configSave = *this;
}


static void __ElemColorChanged(QSettings &s, _CElem &elem)
{
	if (elem.color.changed) 
	{	
		s.beginGroup("color");	
		s.setValue("name", elem.color.name); 
		s.setValue("opacity", elem.color.opacity);	
		s.endGroup();	
	} 
	if (elem.background.changed)
	{
		s.beginGroup("background");
			s.setValue("name", elem.background.name);
			s.setValue("opacity", elem.background.opacity);
		s.endGroup();
	}
	if(elem.font.changed)
	{
		s.beginGroup("font");	
			s.setValue("family", elem.font.family); 
			s.setValue("size", elem.font.sizestr); 
			s.setValue("features", elem.font.features);  
		s.endGroup();				   
	}
}

static void __ElemShadowChanged(QSettings &s, _CElem &elem)
{
	s.beginGroup("shadow");	
		s.setValue("horiz", elem.shadow.values[0]); 
		s.setValue("vert", elem.shadow.values[1]); 
		s.setValue("blur", elem.shadow.values[2]); 
		s.setValue("color", elem.shadow.color); 
		s.setValue("use", elem.shadow.use);
	s.endGroup();	
}

static void __ElemGradientChanged(QSettings &s, _CElem &elem)
{
	if (!elem.gradient.changed)
		return;

	s.beginGroup("gradient");
	if (elem.gradient.used)
	{
		s.setValue("topstop", elem.gradient.gs[0].percent);
		s.setValue("topcolor", elem.gradient.gs[0].color);
		s.setValue("middlestop", elem.gradient.gs[1].percent);
		s.setValue("middlecolor", elem.gradient.gs[1].color);
		s.setValue("bottomstop", elem.gradient.gs[2].percent);
		s.setValue("bottomcolor", elem.gradient.gs[2].color);
		s.setValue("used", true);
	}
	else
	{
		s.remove("topstop");
		s.remove("topcolor");
		s.remove("middlestop");
		s.remove("middlecolor");
		s.remove("bottomstop");
		s.remove("bottomcolor");
		s.setValue("used", false);
	}
	s.endGroup();
}

static void __ElemBorderChanged(QSettings &s, _CElem &elem)
{
	if (elem.border.changed)
	{
		s.beginGroup("border");
			s.setValue("used", elem.border.used); 
			s.setValue("width", elem.border.width.val); 
			s.setValue("color", elem.border.color.name); 
		s.endGroup();
	}
}

static void __ConfigWriteBorder(QSettings &s, _CBorder &border)
{
	if (border.changed || border.color.changed)
	{
		s.beginGroup(border.nameStr);
		s.setValue("used",  border.used);
		s.setValue("width", border.width.val);
		s.setValue("color", border.color.name);
		s.endGroup();
	}
}

static inline void __ConfigWriteInt(QSettings &s, _CInt name) 
{
	if (name.changed)
	{
		s.setValue(name.nameStr, name.val);
		if (__bClearChangedFlag)
			name.changed = false;
	}
}
static inline void __ConfigWriteStr(QSettings &s, _CString name)
{
	if (name.changed)
	{
		s.setValue(name.nameStr, name.str);
		if (__bClearChangedFlag)
			name.changed = false;
	}
}
static inline void __ConfigWriteBool(QSettings &s, _CBool name)
{
	if (name.changed)
	{
		s.setValue(name.nameStr, name.set);
		if (__bClearChangedFlag)
			name.changed = false;
	}
}

static void __ConfigWriteElem(QSettings &s, _CElem &elem)
{
	s.beginGroup(elem.nameStr);
		__ElemColorChanged(s, elem);
		__ElemBorderChanged(s, elem);
	s.endGroup();
}

static void __ConfigWriteElemShadow(QSettings &s, _CElem &elem)
{
	__ConfigWriteElem(s,elem);
	if (elem.shadow.useEver && elem.shadow.changed)
	{
		s.beginGroup(elem.nameStr);
			__ElemShadowChanged(s, elem);
		s.endGroup();
	}
}

static void __ConfigWriteElemGradient(QSettings &s, _CElem &elem)
{
	__ConfigWriteElemShadow(s,elem);
	if ((elem.gradient.useEver && elem.gradient.changed) || elem.border.changed)
	{
		s.beginGroup(elem.nameStr);
			__ElemGradientChanged(s, elem);
			if (elem.border.changed || elem.border.color.changed)
				__ElemBorderChanged(s, elem);
		s.endGroup();
	}
}

/*============================================================================
* TASK: read configuration settings for watermark
* EXPECTS: s - open settings
*		   wm =watermark
*	RETURNS: nothing
* REMARKS:
*---------------------------------------------------------------------------*/
static inline void __WriteWaterMark(QSettings &s, _CWaterMark &elem)
{
	QString sOrigin;
	s.beginGroup(elem.nameStr);
		s.setValue("used",elem.used);
		s.setValue("text",elem.wm.text);

		s.setValue("origin", elem.wm.origin); // low 4 bits: vertical, high 4 bits : horizontal

		s.setValue("mx",elem.wm.marginX);
		s.setValue("my",elem.wm.marginY);
		s.setValue("color",elem.wm.color);
		s.setValue("background",elem.wm.background);
		s.setValue("shadow",elem.wm.shadowColor);
		s.setValue("opacity",elem.wm.opacity);

		s.setValue("family", elem.wm.font.family());
		s.setValue("size", elem.wm.font.pointSize());
		s.setValue("weight", elem.wm.font.weight());
		s.setValue("italic", elem.wm.font.italic());
	s.endGroup();
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
	SeparateFileNamePath(dsSrc.str, p, n);
//	QString s = NameForConfig()

	__bClearChangedFlag = true;				// mark all changes as saved
	_WriteIni(dsSrc.str + n + ".ini");
	__bClearChangedFlag = false;

	changed = false;   // sets changed of all variables false too
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
	__ConfigWriteStr(s, dsSrc);
	__ConfigWriteStr(s, dsGallery);
	__ConfigWriteStr(s, dsGRoot);
	__ConfigWriteStr(s, dsAlbumDir);

	__ConfigWriteStr(s, sServerAddress);
	__ConfigWriteStr(s, sBaseName);		// 'album'
	__ConfigWriteStr(s, sMailTo);			// send user emails here
	__ConfigWriteStr(s, sAbout);			// about page
	__ConfigWriteStr(s, sGalleryTitle);
	__ConfigWriteStr(s, sGalleryLanguages);	// hu,en
	__ConfigWriteStr(s, sUplink);
	__ConfigWriteStr(s, iconUplink);
	__ConfigWriteStr(s, sMainPage);
	__ConfigWriteStr(s, sDescription);
	__ConfigWriteStr(s, sKeywords);

											// sepcial data
//	__ConfigWriteBool(s, bGenerateAll);		// images download allowed not stored
//	__ConfigWriteBool(s, bButImages);		// images download allowed
	__ConfigWriteBool(s, bAddTitlesToAll);		// images download allowed
	__ConfigWriteBool(s, bAddDescriptionsToAll);		// images download allowed
	__ConfigWriteBool(s, bLowerCaseImageExtensions);	// convert extension
//	__ConfigWriteBool(s, bReadJAlbum);
	__ConfigWriteBool(s, bMenuToContact);
	__ConfigWriteBool(s, bMenuToAbout);
	__ConfigWriteBool(s, bMenuToDescriptions);
	__ConfigWriteBool(s, bMenuToToggleCaptions);

	__ConfigWriteBool(s, bRightClickProtected);
	__ConfigWriteBool(s, bCanDownload);		// images download allowed
	__ConfigWriteBool(s, bForceSSL);			// permanent transfer to SSL
	__ConfigWriteBool(s, bOvrImages);  // overwrite images or ask?
	__ConfigWriteBool(s, bSourceRelativePerSign);
	__ConfigWriteBool(s, bFacebookLink);

	__ConfigWriteStr(s, dsImageDir);	// img
	__ConfigWriteStr(s, dsCssDir);		// css
	__ConfigWriteStr(s, dsFontDir);		// font
	__ConfigWriteStr(s, sDefFonts);		// defFonts
	
	__ConfigWriteElem(s, Web);				// page
	__ConfigWriteElemGradient(s, Menu);				// buttons
	__ConfigWriteElemShadow(s, SmallGalleryTitle);		// "andreas falco photography"
	__ConfigWriteElemShadow(s, GalleryTitle);		// "andreas falco photography"
	__ConfigWriteElemShadow(s, GalleryDesc);		// "andreas falco photography"
	__ConfigWriteElemShadow(s, AlbumTitle);		// "Hungary"
	__ConfigWriteElemShadow(s, AlbumDesc);			// "Hungary"
	__ConfigWriteElemShadow(s, Section);			// "Images" or "Albums"
	__ConfigWriteElemShadow(s, ImageTitle);		// not the image itself, just the texts!
	__ConfigWriteElemShadow(s, ImageDesc);			// 
				// images
	__ConfigWriteInt(s, imageWidth);
	__ConfigWriteInt(s, imageHeight);
	__ConfigWriteInt(s, imageSizesLinked);
	__ConfigWriteInt(s, thumbWidth);
	__ConfigWriteInt(s, thumbHeight);
	__ConfigWriteBool(s, doNotEnlarge);
	__ConfigWriteBool(s, bCropThumbnails);
	__ConfigWriteBool(s, bDistrortThumbnails);
	// image decoration
	__ConfigWriteBorder(s, imageBorder);
	
	__ConfigWriteBool(s, iconToTopOn);
	__ConfigWriteBool(s, iconInfoOn);
				// 	Watermarks
	__WriteWaterMark(s, waterMark);
			// Google Analytycs
	__ConfigWriteBool(s, googleAnalyticsOn);
	__ConfigWriteStr(s, googleAnalTrackingCode);
			// latest uploads
	__ConfigWriteBool(s, generateLatestUploads);
	__ConfigWriteInt(s, newUploadInterval);		// days
	__ConfigWriteInt(s, nLatestCount);			// days
}

