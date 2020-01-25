#pragma once
#include <QtCore>

#include "support.h"
#include "stylehandler.h"

const QString falconG_ini = "falconG.ini";

class COMMON_FIELD;

enum ACONFIG_KIND { ackNone, ackBool, ackInt, ackReal, ackText, ackComp };

QString AconfigKindToString(ACONFIG_KIND);

struct FIELD_BASE
{
	QString name;
	ACONFIG_KIND kind;
	COMPOUND_FIELD  *parent;
	bool changed = false;
	FIELD_BASE(QString name, ACONFIG_KIND kind = ackNone, COMPOUND_FIELD  *parent = nullptr) : name(name), kind(kind), parent(parent) { 	}

	virtual void Store(QSettings &s) = 0;	 // - " -
	virtual QString ToString() const = 0;
	virtual	QString DefToString() const = 0;
	virtual bool NotDefault() const = 0;
	virtual ~FIELD_BASE() {}
};

struct BOOL_FIELD : public FIELD_BASE
{
	bool value;
	bool defVal;
	BOOL_FIELD(QString aname, bool aDefVal = false, bool val = false, COMPOUND_FIELD *aparent = nullptr) :
		FIELD_BASE(aname, ackBool, aparent), value(val), defVal(aDefVal) {}
	virtual void Store(QSettings &s);
	virtual QString ToString() const { return value ? "TRUE" : "FALSE"; }
	virtual QString DefToString() const { return defVal ? "TRUE" : "FALSE"; }
	virtual bool NotDefault() const { return value != defVal; }
	BOOL_FIELD& operator=(bool newVal);
};

struct INT_FIELD : public FIELD_BASE
{
	int value = 0;
	int defVal = 0;		// if Load() is unsuccesfull use this
	INT_FIELD(QString aname, int defVal = 0, int val = 0, COMPOUND_FIELD *aparent = nullptr) :
		FIELD_BASE(aname, ackInt, aparent), value(val), defVal(defVal) {}
	virtual void Store(QSettings &s);
	virtual QString ToString(int digits = 0) const
	{
		QString s = QString().setNum(value);
		return digits ? (QString(digits, '0') + s).right(digits) : s;
	}
	virtual QString ToHexString(int digits = 0, bool prefix=false) const
	{
		QString s = QString("%1").arg(value, digits ? digits : 8, 16).trimmed();
		if (digits > 0)
			s = (QString(digits, '0') + s).right(digits);
		
		return prefix ? "0x"+s : s;
	}
	virtual QString DefToString() const { return QString().setNum(defVal); }
	virtual bool NotDefault() const { return value != defVal; }
	INT_FIELD& operator=(int newVal);
};

struct REAL_FIELD : public FIELD_BASE
{
	double value = 0;
	double defVal = 0;		// if Load() is unsuccesfull use this
	REAL_FIELD(QString aname, double defVal = 0.0, double val = 0.0, COMPOUND_FIELD *aparent = nullptr) :
		FIELD_BASE(aname, ackReal, aparent), value(val), defVal(defVal) {}
	virtual void Store(QSettings &s);
	virtual QString ToString() const { return QString().setNum(value); }
	virtual QString DefToString() const { return QString().setNum(defVal); }
	virtual bool NotDefault() const { return value != defVal; }
	REAL_FIELD& operator=(double newVal);
};

struct TEXT_FIELD : public FIELD_BASE
{
	QString value;
	QString defVal;		// if Load() is unsuccesfull use this
	TEXT_FIELD(QString aname, QString aDefVal = QString(), QString val = QString(), COMPOUND_FIELD *aparent = nullptr) :
		FIELD_BASE(aname, ackText, aparent), defVal(aDefVal), value(val) {}
	virtual void Store(QSettings &s);
	virtual QString ToString() const { return value.isEmpty() ? defVal : value; }
	virtual QString DefToString() const { return defVal; }
	virtual bool NotDefault() const { return value != defVal; }
	TEXT_FIELD& operator=(QString newVal);
};

class COMPOUND_FIELD : public TEXT_FIELD
{
protected:
	QMap<QString, FIELD_BASE*> _fields;

	QList<BOOL_FIELD>	_boolList;
	QList<INT_FIELD>	_intList;
	QList<REAL_FIELD>	_realList;
	QList<TEXT_FIELD>	_textList;
	QList<COMPOUND_FIELD> _compList;
public:
	COMPOUND_FIELD(QString aname, QString aDefVal = QString(), QString val = QString(), COMPOUND_FIELD* aparent = nullptr) :
					TEXT_FIELD(aname, aDefVal, val, aparent) {}

	void SetChanged(bool changed) { changed = changed; }
	void Clear() { _boolList.clear(); _intList.clear(); _realList.clear(); _textList.clear(); _fields.clear(); changed = false; }

	void AddBoolField(QString name, bool defVal = false, bool val = false);
	void AddIntField(QString name, int defVal = 0, int val = 0);
	void AddRealField(QString name, double defVal = 0.0, double val = 0.0);
	void AddTextField(QString name, QString defVal = QString(), QString val = QString());
	void AddCompundField(QString name, QString defVal = QString(), QString val = QString());

	virtual void Store(QSettings &s);
	virtual QString ToString() const 
	{ 
		QString s;
		for (auto f : _fields)
			s += f->ToString() + "\n";
		return s;
	}
	virtual QString DefToString() const
	{
		QString s;
		for (auto f : _fields)
			s += f->DefToString() + "\n";
		return s;
	}
	virtual bool NotDefault() const 
	{	
		for (auto pf : _fields)
			if (pf->NotDefault())
				return true;
		return false;	
	}
	size_t Size(ACONFIG_KIND kind = ackNone) const;

	COMPOUND_FIELD &CopyFrom(const ACONFIG &other);
	// debug
	void DumpFields(ACONFIG_KIND kind = ackNone, QString file = QString());	// print all
};

class ACONFIG : public COMPOUND_FIELD
{
	QTextStream _ifs;
	QTextStream _ofs;
	bool _changed;

	void _Write(FIELD_BASE *pf);
public:
	ACONFIG() : COMPOUND_FIELD("root") {}		 // root field has no group

	void Load(QString fname);	// from file
	void Store(QString fname);
	// DEBUG
		// operators
	FIELD_BASE *operator[](QString fieldn);
};

//***************************** use the above definition ***************

struct _CString : public TEXT_FIELD
{
	QString &operator=(const QString s);
	QString &operator=(const _CString &s) { value = s.value; changed = s.changed; return value; };
	QString ToString() const { return value.isEmpty() ? defVal : value; }
	int Length() const { return ToString().length(); }
};

struct _CDirStr : public _CString	 // always ends with '/'
{
	operator QString() = delete;
	
	static QString AddSep(QString qs) // add separator only when needed
	{									// modifies argument!
		if (qs.length() && qs[qs.length() - 1] != QChar('/'))
			qs += "/"; 
		return qs;
	}

	QString &operator=(const QString s);
	QString &operator=(const _CDirStr &s) { value = s.value; changed = s.changed; return value; };
	_CDirStr operator+(const _CDirStr &subdir);
	_CDirStr operator+(const QString subdir);
};

using _CBool = BOOL_FIELD;
using _CInt = INT_FIELD;

struct _CColor : public COMPOUND_FIELD
{
	//  value and defVal are always in the form '#xxx' or '#xxxxxx' where x is a hexadecimal digit
	_CColor(QString name, COMPOUND_FIELD *parent = nullptr) : COMPOUND_FIELD(name, parent) 
	{
		AddTextField(name,  ????  )
	}

	int opacity=-100;			// opacity in percent. < 0: not used
	QString operator=(QString s);
	operator QString() const;	// drop the '#'
	void Set(QString str, int opac);
	virtual void Store(QSettings &s) { COMPOUND_FIELD::Store(s); }
};

struct _CFont
{
	enum feature: unsigned short { fBold=1, fItalic=2, fUnderline=4, fStrikethrough=8};
	QString family;
	QString sizestr;	// e.g. 10pt
	union {
		unsigned short features = 0;
		struct {
			unsigned char bold : 1;
			unsigned char italic : 1;
			unsigned char underline : 1;
			unsigned char strikethrough : 1;
		};
	};
	_Changed changed;
	void Set(QString fam, QString siz, int feat);
	void ClearFeatures();
};

struct _CShadow
{
	bool useEver = false;		// if false: will not be saved!
	bool use = false;
	union {
		int32_t value;
		unsigned char values[4] = { 0 };	//  sizes: horizontal, vertical, blur radius, dummy
	};
	QString color;	// optional color
	_Changed changed;
	void Set(int which, int value);
	void Set(int horiz, int vert, int spread, QString clr, bool used);

	int Horiz() const { return values[0]; }
	int Vert() const { return values[1]; }
	int Blur() const { return values[2]; }
	bool IsSet() const { return *(int32_t*)values; }
};
struct _CGradientStop
{
	int percent=0;				// position
	QString color;				// name of color w.o. '#'
	bool Set(int pc, QString clr);

};

struct _CGradient
{
	bool useEver = false;   // if false: will not be saved!
	_CGradientStop gs[3];   // top, middle, bottom;
	bool used = false;		// same as the checkbox state
	_Changed changed;
	// border color is its parent's border.color
	void Set(int which, int pc, QString clr);
	void Set(bool used, int topStop, QString topColor, int middleStop, QString middleColor, int bottomStop, QString bottomcolor, bool chg = true);
};

struct _CBorder
{
	QString nameStr;
	_CColor color;
	_CInt width;
	bool used;
	_Changed changed;

	_CBorder &operator=(bool on)	{ if(used != on) changed = true; used = on; return *this; }
	_CBorder &operator=(int w)		{ if(width != w) changed = true; width = w; return *this; }
	_CBorder &operator=(_CColor &c) { color = c; if(color.changed) changed = true; return *this; }
	_CBorder &operator=(QString &c) { color = c; if(color.changed) changed = true; return *this; }
};

struct _CElem
{
	QString nameStr;
	_CColor color, background;
	_CFont font;
	_CShadow shadow;
	_CGradient gradient;
	_CBorder border;
	bool Changed() { return color.changed || background.changed || shadow.changed || gradient.changed || font.changed || border.changed; }
	void ClearChanged() { color.changed = background.changed = shadow.changed = gradient.changed = font.changed = border.changed = false; };
};

struct _CWaterMark
{
	QString nameStr;

	WaterMark wm;

	bool used = false;
	_Changed changed;

	_CWaterMark &operator=(const _CWaterMark &cwm);
};

/*==================================
 *  Structure of falconG.ini in program directory
 *  holds the last N directories used
 *  each directory must have its own ini file
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
	class _ChangedFlag {
		bool _state = false;
		CONFIG *_parent = nullptr;
	public:
		void SetParent(CONFIG *p) { _parent = p; }
		bool operator=(bool val);
		operator bool() { return _state; }
	};

	double  _thumbAspect = 0;
	bool _aspectsDiffer = false;	// _imageAspect and _thumAspect
	void _WriteIni(QString name);


public:
	CONFIG();

	int majorStructVersion = 1, minorStructVersion = 2;		// 1.2
	StyleHandler styleHandler;	// for the main window with all of its elements

	void Read();
	void Write();

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

	_ChangedFlag changed;		// any of config is changed		THIS MUST BE THE FIRST MEMBER OF CONFIG! (why?)
	bool designChanged = false;	// page design is changed

	_CBool bNoMoreWarnings;		// stop displaying warnings

	// Gallery page
					// local directories
	_CDirStr dsApplication;		// home directory for the application (needed for copying resources from here)
	_CDirStr dsSrc;				// source directory (jalbum gallery root contains the actual falconG,ini)
	_CDirStr dsGallery;			// destination directory on local machine (corresponds to 'public_html' on server)

					// remote directories
	_CDirStr dsGRoot;			// name of root directory on server (in public_html or in dsGallery)
	_CDirStr dsAlbumDir;		// (path) name of album directory (usually inside dsGRoot)
	_CDirStr dsCssDir;
	_CDirStr dsFontDir;
	_CDirStr dsImageDir;		// images on server AND or destination
	_CDirStr dsThumbDir;		// thumbnail directory on server AND or destination

	
	_CBool bAddTitlesToAll;		// into gallery.struct
	_CBool bAddDescriptionsToAll; // into gallery.struct
	_CBool bLowerCaseImageExtensions; // convert all image extensions to lowercase
	_CBool bReadJAlbum;			// do not process gallery.struct, read from JAlbum structure
	_CBool bReadFromGallery;	// read back from HTML files in gallery (you loose original names and paths!
		// next two bools will not be read from or saved into the configuration file
	_CBool bGenerateAll;		// must regenerate all pages regardless of status?
	_CBool bButImages;			// used with 'generate All'

	_CString sUplink;			// path name of index.html file on server this gallery is embedded into.
								// this page if set should have links into dsGRoot for all languages
								// If given the main page;s uplink points here, otherwise no uplink on main page(s)
	_CString iconUplink;		// name of the icon file in the 'res' sub-directory
	_CString  sMainPage;		// name of the root file in the system The gallery root 
								// (default index.html or index1en.html, index1hu.html,...)
								// this is put into dsGRoot and not in the albums subdirectory, unless a path is given
								// The Home menu will link here (Example: index.html - home link: /<dsGRoot>/index.html)
	_CString sDescription;		// page description: appears in we bsearch results
	_CString sKeywords;			// comma separated list of keywords

	_CBool bCanDownload;		// images from the server (.htaccess)
	_CBool bForceSSL;			// using .htaccess
	_CBool bRightClickProtected; // right click protection on
	_CBool bMenuToContact;		// show menu button for 'Contact' page?
	_CBool bMenuToAbout;		// show menu button for 'About' page?
	_CBool bMenuToDescriptions;   // toggles image/album description visibility. If bMenuToToggleCaptions is false it also turns on/off captions and the icons
	_CBool bMenuToToggleCaptions; // show captions at all? (see above)

	_CString sServerAddress;	// server address:
	// special

	_CString sDefFonts;			// default font names
	_CString sBaseName;			// default base name
	_CString sMailTo;			// send user emails here no defaults
	_CString sAbout;			// address of about page, either relative to dsGRoot or any URI
	_CBool bOvrImages;			// overwrite images (default: true)
	_CBool bSourceRelativePerSign; // when true add source gallery to paths starting with '/'

		 		// latest uploads
	_CBool generateLatestUploads;	// or not
	_CInt newUploadInterval;		// date period of images calculated from the latest to include into latest images
	_CInt nLatestCount;				// max this many images in the latest upload gallery
				// Google Analytycs
	_CBool googleAnalyticsOn;
	_CString googleAnalTrackingCode;

	_CString sGalleryTitle;
	_CString sGalleryDesc;
	_CString sGalleryLanguages;	// empty or a series of language abbreviations. Example: hu,en. 
								// Order important, either files '<abbrev>.txt' must exist 
								// (in either the program directory or in the source gallery directory)
								// or in the '.struct' file

	_CBool bFacebookLink;

// Design page

	_CElem Web;					// page
	_CElem Lang;				// language link button
	_CElem Menu;				// menu buttons
	// special handling : include shadows
	_CElem GalleryTitle;		// "andreas falco photography"
	_CElem SmallGalleryTitle;	// "andreas falco photography"
	_CElem GalleryDesc;			// "andreas falco photography"
	_CElem AlbumTitle;			// "Hungary"
	_CElem AlbumDesc;			// "Hungary"
	_CElem Section;				// "Images" or "Albums"
	_CElem ImageTitle;			// not the image itself, just the texts!
	_CElem ImageDesc;		
	// 
				// image page
	_CElem images;				// color for images and folders
	_CInt imageWidth;
	_CInt imageHeight;
	_CInt thumbWidth;
	_CInt thumbHeight;
	_CInt imageSizesLinked;
	_CBool doNotEnlarge;		// default: true
	_CBool bCropThumbnails    ;	// default: false exclusive with bDistortThumbnails
	_CBool bDistrortThumbnails; // default: false eclusive with bCropThumbnails
				// image decoration
	_CBool iconToTopOn;
	_CBool iconInfoOn;
	_CBorder imageBorder;
				// 	Watermarks
	_CWaterMark waterMark;
		// calculated values	not saved not even included in the backup
	QString homeLink;		// actual home link for given langyage hierarchy
};

extern CONFIG config;		// Global variable
