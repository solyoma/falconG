#pragma once
#pragma once

#include <QtCore>
#include "support.h"
#include "stylehandler.h"

const QString falconG_ini = "falconG.ini";

// common templates for selecting the correct type of return value from settings
template <class T>
std::enable_if_t< std::is_same_v<bool, T>, bool> Value(QSettings& s, QString name, T def) { return s.value(name, def).toBool(); }
template <class T>
std::enable_if_t< std::is_same_v<int, T>, int> Value(QSettings& s, QString name, T def) { return s.value(name, def).toInt(); }
template <class T>
std::enable_if_t< std::is_same_v<double, T>, double> Value(QSettings& s, QString name, T def) { return s.value(name, def).toDouble(); }
template <class T>
std::enable_if_t< std::is_same_v<QString, T>, QString> Value(QSettings& s, QString name, T def) { return s.value(name, def).toString(); }

// base class template for all config itemss. Based on functions Value() above.

template <class T> struct _CFG_ITEM
{
	QString itemName;	// in settings
	T	v0,			// original value
		v,			// actual value	read from settings file
		vd;			// default value
	_CFG_ITEM(QString namestr, T vd) : itemName(namestr), v0(vd), v(vd), vd(vd) {}

	virtual bool Changed() const 
	{ 
		return v != v0; 
	}

	virtual void ClearChanged()
	{
		v0 = v;
	}

	virtual void Write(QSettings& s, QString group = QString() )	// into settings, but only if changed
	{
		if (v != v0)		// changed
		{
			if(!group.isEmpty())
				s.beginGroup(group);

			if (v == vd)
				s.remove(itemName);		// from current group!
			else
				s.setValue(itemName, v);

			if (!group.isEmpty())
				s.endGroup();
		}
	}
	virtual void Read(QSettings& s, QString group = QString())
	{
		if(!group.isEmpty())
			s.beginGroup(group);

		v = Value(s, itemName, vd);
		_Setup();	// may modify 'v'
		v0 = v;

		if (!group.isEmpty())
			s.endGroup();
	}
	_CFG_ITEM& operator=( const _CFG_ITEM o) { v = o.v; v0 = o.v0, vd = o.vd; return *this; }
protected:
	virtual void _Setup() {}
	virtual void _Prepare() {}
};

//--------------------------------------------------------------------------------------------

struct _CString : public _CFG_ITEM<QString>
{

	_CString(QString namestr = "cstring", QString vd=QString()) : _CFG_ITEM(namestr, vd) {}

	QString &operator=(const QString s);
	QString& operator=(const _CString& s) { _CFG_ITEM::operator=(s);  return v; };
//	operator const QString() const { return ToString(); }
	operator QString() const { return ToString(); }
	bool IsEmpty() const { return v.isEmpty();  }
	QString ToString() const { return v.isEmpty() ? vd : v; }
	int Length() const { return ToString().length(); }
};

//--------------------------------------------------------------------------------------------


struct _CDirStr : public _CString	 // always ends with '/'
{
	_CDirStr(QString namestr="cdirstr", QString vd=QString()) : _CString(namestr, vd) {  }

	operator QString() = delete;

	void Read(QSettings& s, QString group = QString()) override { _CFG_ITEM::Read(s, group); _Setup(); }

	QString &operator=(const QString s);
	QString &operator=(const _CDirStr &s) { _CString::operator=(s);  return v; };
	_CDirStr operator+(const _CDirStr &subdir);
	_CDirStr operator+(const QString subdir);
	
	static QString AddSep(QString qs) // add separator only when needed
	{									// modifies argument!
		if (qs.length() && qs[qs.length() - 1] != QChar('/'))
			qs += "/"; 
		return qs;
	}
private:
	void _Setup() override { AddSep(v); AddSep(v0); AddSep(vd); }
};

//--------------------------------------------------------------------------------------------


struct _CBool : public _CFG_ITEM<bool>
{
	_CBool(QString namestr="cbool", bool def=false) : _CFG_ITEM(namestr, def) {}

	bool &operator=(const bool s);
	operator bool() const { return v; }
	QString ToString() const { return v ? "true" : "false"; }
};

//--------------------------------------------------------------------------------------------


struct _CInt : _CFG_ITEM<int>
{
	_CInt(QString namestr="cint", int vd=0) : _CFG_ITEM(namestr, vd) {}

	int &operator=(const int s);
	operator int() const { return v; }
	QString ToString() const { return ToString(0);  }
	QString ToString(int digits) const 
	{
		QString s = QString("%1").arg(v); 
		if (digits > 0) 
			s = (QString(digits, '0') + s).right(digits); 
		return s;	
	}
	QString ToHexString(int digits=0) const 
	{ 
		return IntToHexString(v);
	}
	static QString IntToHexString(int v, int digits = 0)
	{
		QString s = QString("%1").arg(v, digits ? digits : 8, 16).trimmed();
		if (digits > 0)
			s = (QString(digits, '0') + s).right(digits);
		return s;
	}
};

//--------------------------------------------------------------------------------------------


struct _CColor : _CFG_ITEM<QString>
{								
	// v is always in the form '#xxx' or '#AAxxxxxx' or '#xxxxxx|opacity' or
	//	'#AAxxxxxx|opacity' where x and A are any hexadecimal digit (A: alpha)
	//	opacity may be missing and when it does 100% is assumed
	_CColor(QString namestr = "ccolor", QString name = "#000000") : _CFG_ITEM(namestr, name) 
	{ 
		_Setup();  
	}

	void Set(QString str, int opac);
	void SetColor(QString clr) { _colorName = clr; _Prepare(); }
	void SetOpacity(int opac) { _opacity = opac; _Prepare(); }
	int Opacity() const { return _opacity; }			// opacity in percent. < 0: not used
	QString Name(bool nohash = false) const
	{
		return (nohash ? _colorName.mid(1) : _colorName); 			// nohash:   no '#' at start
	}
	QColor Color() const { return QColor(Name(false)); }

	QString operator=(QString s);
	_CColor& operator=(_CColor c);
//	operator QString() const;	// drop the '#'

private:
	int _opacity;
	QString _colorName;		// never has a leading '#' character
	void _Setup() 
	{
		if (v.at(0) != '#')
			v = "#" + v;

		int is = v.indexOf('|');
		_opacity = (is >= 0) ? v.mid(is + 1).toInt() : 100;
		_colorName = (is >= 0) ? v.mid(0, is) : v;
	}
	void _Prepare() override
	{
		v = _colorName;
		if (_opacity > 0 && _opacity < 100)
			v += "|" + QString().setNum(_opacity);
	}
	bool _ColorStringValid(QString &s); // accepted formats (x: hexdecimal digit): xxx, #xxx, xxxxxx,#xxxxxx
};

//--------------------------------------------------------------------------------------------

struct _CFont : _CFG_ITEM<QString>
{
	enum feature: unsigned short { fBold=1, fItalic=2, fUnderline=4, fStrikethrough=8};

	_CFont(QString namestr = "cfont", QString vd = "\"Tms Rmn\",\"Times New Roman\", Arial|10pt|0|0|10pt") : _CFG_ITEM(namestr, vd) { _Setup();  }

	QString Family() const { return _details[0]; }
	int Features() const { return _details[2].toInt(); }
	int Size() const { return _details[1].toInt(); };
	QString SizeStr() const { return _details[1]; }
	int Italic() const { return _details[2].toInt() & fItalic; }
	int Bold() const { return _details[2].toInt() & fBold; }
	int Underline() const { return _details[2].toInt() & fUnderline; }
	int StrikeThrough() const { return _details[2].toInt() & fStrikethrough; }
	bool IsFirstLineDifferent() const { return _details[3] == "1"; }
	QString FirstLineSize() const { return _details[4]; }

	void Set(QString fam, QString siz, int feat, QString sFsSize = QString());
	void SetFamily(QString fam);
	void SetFeature(int feat, bool single=false);	// one or all features?
	void SetSize(int pt);
	void SetSize(QString s);
	void SetDifferentFirstLine(bool set, QString size = QString());
	
	void ClearFeatures();
protected:
	void _Setup() override;		// format: <family(ies)>|<size>|<featurnumber>|<diffFirstLine>|<firstSize>
								// index:     0			   1		  2				 3			   4
	void _Prepare() override;	// from _details to 'v'
private:
	QStringList _details; 
};

//--------------------------------------------------------------------------------------------

class _CShadow : public _CFG_ITEM<QString>
{
	// format: |<use>|<horiz>|<vert>[|<blur radius-number>][|color name-starts with letter or '#'>]
	//   index 	  0      1	    2	     3	or	4              3 or 4
	QStringList _details;
	int _ixColor, _ixBlur;	// indices for blur and color (3 or ,4 order of them is not fixed, and either of them may be missing
	void _Setup() override;	// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'
public:
	_CShadow(QString nameStr = "cshadow", QString vd = "0|0|0|0|#000000") : _CFG_ITEM(nameStr, vd) {  _Setup();  }

	void Set(int which, int value);
	void Set(int horiz, int vert, int spread, QString clr, bool used);

	QString Color() const { return _ixColor > 0 ? _details[_ixColor] : QString(); }	// optional color
	int Blur() const { return _details.size() > 3 && _details[3].at(0).isDigit() ? _details[3].toInt() : 0; }
	QString ForStyleSheets() const 
	{ 
		QString res =  _details[1] + " " + _details[2]; 
		if (_ixBlur)
			res += " " + _details[_ixBlur];
		if (_ixColor)
			res += " " + _details[_ixColor];
		return res;
	}

	bool IsSet() const { return _details.size(); }
	bool Use()	const { return _details[0].at(0) == '1'; }
	int Horiz() const { return _details.size() ? _details[1].toInt() : 0; }
	int Vert() const { return _details.size()  ? _details[2].toInt() : 0; }
	void SetBlur(QString sc) { _details[_ixColor] = sc; _Prepare(); }
	void SetColor(QString sc) { _details[_ixColor] = sc; _Prepare(); }
};

//--------------------------------------------------------------------------------------------

struct _CGradientStop
{
	int percent=0;				// position
	QString color;				// name of color starting with '#'
	bool Set(int pc, QString clr);

};

//--------------------------------------------------------------------------------------------


struct _CGradient : public _CFG_ITEM<QString>
{
	_CGradientStop gs[3];   // top, middle, bottom;
	bool used = false;		// same as the checkbox state

	_CGradient(QString namestr="cgradient", QString vd="#000000") : _CFG_ITEM(namestr, vd) { _Setup(); }

	// border color is its parent's border.color
	void Set(int which, int pc, QString clr);
	void Set(bool used, int topStop, QString topColor, int middleStop, QString middleColor, int bottomStop, QString bottomcolor, bool chg = true);
private:
	bool _internal = false;	// true: do not _Prepare after set
	// format: <used>|<positionTop(%)>|<color Top>|<positionMiddle(%)>|<color Middle>|<positionBottom(%)>|<color bottom>
	// index	  0				1				2			3					4              5				6
	void _Setup() override;		// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'

};

//--------------------------------------------------------------------------------------------


struct _CBorder : public _CFG_ITEM<QString>
{
	QString color;
	int width;
	bool used;

	_CBorder(QString namestr="cborder", QString vd="0|#000000|0") : _CFG_ITEM(namestr, vd) { _Setup(); }

	_CBorder& operator=(bool on) { used = on; _Prepare(); return *this; }
	_CBorder &operator=(int w)		{ width = w; _Prepare(); return *this; }
	_CBorder &operator=(_CColor &c) { color = c.Name(); _Prepare(); return *this; }
	_CBorder &operator=(QString &c) { color = c; _Prepare(); return *this; }
private:
	// format: <used 0 or 1>|<color>|<width>
	// index		0			1		2
	void _Setup() override;		// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'
};

//--------------------------------------------------------------------------------------------


struct _CElem : public _CFG_ITEM<bool>		// v, vd, etc not used at all
{											// name is the elem name
	_CColor color = { "color","#000000" }, 
		background = { "background", "#ffffff" };
	_CFont font = {"font","\"Monotype Corsiva\",Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif|12pt|0"};
	_CShadow shadow = {"shadow","0"};
	_CGradient gradient = {"gradient", "0"};
	_CBorder border = {"border","0|#890222|1"};

	_CElem(QString namestr="celem", bool vd=false, bool bShadow=false, bool bGradient=false) : 
		_CFG_ITEM(namestr, vd), _bMayShadow(bShadow), _bMayGradient(bGradient) { }
	bool Changed() const override;
	void ClearChanged() override;

	void ColorsAndFontsFrom(const _CElem& other);	// all except 'name'

	virtual void Write(QSettings& s, QString group = QString()) override;
	virtual void Read(QSettings& s, QString group = QString()) override;
private:
	bool _bMayShadow, _bMayGradient;
};

//--------------------------------------------------------------------------------------------


struct _CWaterMark : public _CFG_ITEM<bool>		  // v used for changed
{
	bool used = false;
	WaterMark wm;

	bool Changed() const override { return v; }

	_CWaterMark(QString namestr = "watermark", bool vd = false) : _CFG_ITEM(namestr, vd) { }
	_CWaterMark &operator=(const _CWaterMark &cwm);

	virtual void Write(QSettings& s, QString group = QString()) override;	// into settings, but only if changed
	virtual void Read(QSettings& s, QString group = QString()) override;
};

//--------------------------------------------------------------------------------------------


/*==================================
 *  Structure of falconG.ini in program directory
 *  holds the last 'maxSavedConfigs' directories used.
 *  Each directory must have its own ini file
 *---------------------------------*/
class CONFIG;

struct CONFIGS_USED
{
	static CONFIG *parent;
	static int maxSavedConfigs;		// max this many last configuration directories are stored
	static int indexOfLastUsed;		// this was the last one used
	static QStringList lastConfigs; // these are the (source) directories
	static void Read();	// the last directories used
	static void Write();

	static QString NameForConfig(QString sExt);	// returns either last part of lastConfigs[indexOfLastUsed] + sExt or other
};

//-------------------------
class CONFIG
{
	bool _changed = false;
	double  _thumbAspect = 0;
	bool _aspectsDiffer = false;	// _imageAspect and _thumAspect
	void _WriteIni(QString name);
public:
	CONFIG();

	int majorStructVersion = 1, minorStructVersion = 2;		// 1.2
	StyleHandler styleHandler;	// for the main window with all of its elements

	void Read();
	void Write();

	bool Changed() const		{ return _changed; }
	bool SetChanged(bool chg) 
	{ 
		_changed |= chg; return _changed; 
	}
	void ClearChanged();

	CONFIG &operator=(const CONFIG &cfg);
	void FromDesign(const CONFIG &cfg);
	void FromOther(const CONFIG &cfg);
	void RestoreDesign();
	void RestoreOther();
	QSize ImageSize() { return QSize(imageWidth, imageHeight);	}
	QSize ThumbSize() { return QSize(thumbWidth, thumbHeight); }

	double ThumbAspect(bool reset=false)
	{
		if (!reset && _thumbAspect)
			return _thumbAspect;
		double imageAspect = imageHeight ? (double)imageWidth / (double)imageHeight : 1.0;
		
		_thumbAspect = thumbHeight ? (double)thumbWidth / (double)thumbHeight : 1.0;
		_aspectsDiffer = round(100.0*imageAspect) - round(100.0*_thumbAspect) > 1.0;
		return _thumbAspect;
	}

	bool ImageAndThumbAspectDiffer(bool reset = false)
	{
		ThumbAspect(reset);
		return _aspectsDiffer;
	}

	_CDirStr GalleryRoot() const	{ return _CDirStr(dsGallery) + _CDirStr(dsGRoot); }
	_CDirStr ImageDirectory() const { return GalleryRoot() + dsImageDir; }
	_CDirStr ThumbnailDirectory() const { return GalleryRoot() + dsThumbDir; }
	_CDirStr AlbumDirectory() const { return GalleryRoot() + dsAlbumDir; }
		  // **** Fields of CONFIG ***
	bool designChanged = false;	// page design is changed

	_CBool bNoMoreWarnings;		// stop displaying warnings

	// Gallery page
					// local directories
	_CDirStr dsApplication;		// home directory for the application (needed for copying resources from here)
	_CDirStr dsSrc = {"dsSrc", ""};				// source directory (jalbum gallery root contains the actual falconG,ini)
	_CDirStr dsGallery = { "dsGallery","" };			// destination directory on local machine (corresponds to 'public_html' on server)

					// remote directories
	_CDirStr dsGRoot = { "dsgRoot","" };			// name of root directory on server (in public_html or in dsGallery)
	_CDirStr dsAlbumDir = {"dsAlbumDir","albums/"};		// (path) name of album directory (usually inside dsGRoot)
	_CDirStr dsCssDir = {"dsCssDir","css/"};
	_CDirStr dsFontDir = {"dsFontDir","fonts/"};
	_CDirStr dsImageDir = {"dsImageDir","imgs/"};		// images on server AND or destination
	_CDirStr dsThumbDir = {"dsThumbDir","thumbs/"};		// thumbnail directory on server AND or destination

	
	_CBool bAddTitlesToAll = {"bAddTitlesToAll",false};		// into gallery.struct
	_CBool bAddDescriptionsToAll = {"bAddDescriptionsToAll",false}; // into gallery.struct
	_CBool bLowerCaseImageExtensions = {"bLowerCaseImageExtensions",true}; // convert all image extensions to lowercase
	_CBool bReadJAlbum = {"bReadJAlbum",false};			// do not process gallery.struct, read from JAlbum structure
	_CBool bReadFromGallery = {"bReadFromGallery",false};	// read back from HTML files in gallery (you loose original names and paths!
		// next two bools will not be read from or saved into the configuration file
	_CBool bGenerateAll = {"bGenerateAll",false};		// must regenerate all pages regardless of status?
	_CBool bButImages = {"bButImages",false};			// used with 'generate All'

	_CString sUplink = {"sUplink",""};			// path name of index.html file on server this gallery is embedded into.
								// this page if set should have links into dsGRoot for all languages
								// If given the main page;s uplink points here, otherwise no uplink on main page(s)
	_CString iconUplink = {"iconUplink","up-icon.png"};		// name of the icon file in the 'res' sub-directory
	_CString  sMainPage = {" sMainPage","index.html"};		// name of the root file in the system The gallery root 
								// (default index.html or index1en.html, index1hu.html,...)
								// this is put into dsGRoot and not in the albums subdirectory, unless a path is given
								// The Home menu will link here (Example: index.html - home link: /<dsGRoot>/index.html)
	_CString sDescription = {"sDescription",""};		// page description: appears in we bsearch results
	_CString sKeywords = {"sKeywords",""};			// comma separated list of keywords

	_CBool bCanDownload = {"bCanDownload",false};		// images from the server (.htaccess)
	_CBool bForceSSL = {"bForceSSL", false};			// using .htaccess
	_CBool bRightClickProtected = {"bRightClickProtected",true}; // right click protection on
	_CBool bMenuToContact = {"bMenuToContact",false};		// show menu button for 'Contact' page
	_CBool bMenuToAbout = {"bMenuToAbout",false};		// show menu button for 'About' page?
	_CBool bMenuToDescriptions = {"bMenuToDescriptions",true};   // toggles image/album description visibility. If bMenuToToggleCaptions is false it also turns on/off captions and the icons
	_CBool bMenuToToggleCaptions = {"bMenuToToggleCaptions",false}; // show captions at all? (see above)

	_CString sServerAddress = {"sServerAddress",""};	// server address:
	// special

	_CString sDefFonts = {"sDefFonts","Constantia,Palatino,\"Palatino Linotype\",\"Palatino LT STD\",Georgia,serif"};			// default font names
	_CString sBaseName = {"sBaseName","album"};			// default base name
	_CString sMailTo = {"sMailTo",""};			// send user emails here no defaults
	_CString sAbout = {"sAbout",""};			// address of about page, either relative to dsGRoot or any URI
	_CBool bOvrImages = {"bOvrImages",true};			// overwrite images (default: true)
	_CBool bSourceRelativePerSign = {"bSourceRelativePerSign",true}; // when true add source gallery to paths starting with '/'

		 		// latest uploads
	_CBool generateLatestUploads = {"generateLatestUploads", false};	// or not
	_CInt newUploadInterval = {"newUploadInterval", 10};		// date period of images calculated from the latest to include into latest images
	_CInt nLatestCount = {"nLatestCount", 10};				// max this many images in the latest upload gallery
				// Google Analytycs
	_CBool googleAnalyticsOn = {"googleAnalyticsOn",false};
	_CString googleAnalTrackingCode = {"googleAnalTrackingCode",""};

	_CString sGalleryTitle = {"sGalleryTitle","Andreas Falco Photography"};
	_CString sGalleryDesc = {"sGalleryDesc",""};
	_CString sGalleryLanguages = {"sGalleryLanguages",""};	// empty or a series of language abbreviations. Example: hu,en. 
								// Order important, either files '<abbrev>.txt' must exist 
								// (in either the program directory or in the source gallery directory)
								// or in the '.struct' file

	_CBool bFacebookLink = {"bFacebookLink", false};

// Design page 
#define _DFLT_ false
#define _SHDW_ true
#define _NOSH_ false
#define _GRAD_ true
#define _NGRD_ false
	_CElem Web = {"Web",_DFLT_, _NOSH_, _NGRD_};		// page
	_CElem Lang = {"Lang",_DFLT_, _NOSH_, _NGRD_};		// language link button
	_CElem Menu = {"Menu",_DFLT_, _SHDW_, _GRAD_};		// menu buttons
	// special handling : include shadows
	_CElem GalleryTitle = {"GalleryTitle",_DFLT_, _SHDW_, _NGRD_};		// "andreas falco photography"
	_CElem SmallGalleryTitle = {"SmallGalleryTitle",_DFLT_, _SHDW_, _NGRD_};	// "andreas falco photography"
	_CElem GalleryDesc = {"GalleryDesc",_DFLT_, _SHDW_, _NGRD_};			// "andreas falco photography"
	_CElem AlbumTitle = {"AlbumTitle",_DFLT_, _SHDW_, _NGRD_};			// "Hungary"
	_CElem AlbumDesc = {"AlbumDesc",_DFLT_, _SHDW_, _NGRD_};			// "Hungary"
	_CElem Section = {"Section",_DFLT_, _SHDW_, _NGRD_};				// "Images" or "Albums"
	_CElem ImageTitle = {"ImageTitle",_DFLT_, _SHDW_, _NGRD_};			// not the image itself, just the texts!
	_CElem ImageDesc = {"ImageDesc",_DFLT_, _SHDW_, _NGRD_};		
	// 
				// image page
	_CInt imageWidth = {"imageWidth",1920};
	_CInt imageHeight = {"imageHeight",1080};
	_CInt thumbWidth = {"thumbWidth",600};
	_CInt thumbHeight = {"thumbHeight",400};
	_CInt imageSizesLinked = {"imageSizesLinked",true};
	_CBool doNotEnlarge = {"doNotEnlarge", true};		// default: true
	_CBool bCropThumbnails = { "bCropThumbnails", false };	// default: false exclusive with bDistortThumbnails
	_CBool bDistrortThumbnails = {"bDistrortThumbnails",false}; // default: false eclusive with bCropThumbnails
				// image decoration
	_CBool iconToTopOn = {"iconToTopOn",false};
	_CBool iconInfoOn = {"iconInfoOn", false};
	_CBorder imageBorder = {"imageBorder","0|EAA41E|2"};
				// 	Watermarks
	_CWaterMark waterMark = {"waterMark",""};

	// Options page
	_CInt styleIndex = {"styleIndex",0};

	// Debug
	_CBool bDebugging = {"bDebugging",false};
#undef _DFLT_ 
#undef _SHDW_ 
#undef _NOSH_ 
#undef _GRAD_ 
#undef _NGRD_ 

		// calculated values	not saved not even included in the backup
	QString homeLink;		// actual home link for given langyage hierarchy
};

extern CONFIG config;		// Global variable
