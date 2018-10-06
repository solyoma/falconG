#pragma once

#include <QtCore>
#include "support.h"

const QString falconG_ini = "falconG.ini";

class _Changed
{
	bool _changed = false;
public:
	bool operator=(bool val);
	bool operator|=(bool val);
	bool operator&=(bool val);
	bool operator!=(bool val);
	bool operator==(bool val);
	operator bool() { return _changed; }
};

struct _CString
{
	QString nameStr;
	QString str,
			defStr;
	_Changed changed;
	QString &operator=(const QString s);
	QString &operator=(const _CString &s) { str = s.str; changed = s.changed; return str; };
//	operator const QString() const { return ToString(); }
	operator QString() const { return ToString(); }
	bool IsEmpty() const { return str.isEmpty();  }
	QString ToString() const { return str.isEmpty() ? defStr : str; }
	int Length() const { return ToString().length(); }
};

struct _CDirStr : public _CString
{
	operator QString() = delete;
	QString &operator=(const QString s);
	QString &operator=(const _CDirStr &s) { str = s.str; changed = s.changed; return str; };
	_CDirStr operator+(const _CDirStr &subdir);
	_CDirStr operator+(const QString subdir);
};

struct _CBool
{
	QString nameStr;
	bool set = false;
	_Changed changed;
	bool &operator=(const bool s);
	operator bool() const { return set; }
	QString ToString() const { return set ? "true" : "false"; }
};

struct _CInt
{
	QString nameStr;
	int val = 0;
	_Changed changed;
	int &operator=(const int s);
	operator int() const { return val; }
	QString ToString(int digits = 0) const 
	{
		QString s = QString("%1").arg(val); 
		if (digits > 0) 
			s = (QString(digits, '0') + s).right(digits); 
		return s;	
	}
	QString ToHexString(int digits=0) const 
	{ 
		QString s = QString("%1").arg(val, digits ? digits : 8, 16).trimmed(); 
		if (digits > 0)
			s = (QString(digits, '0') + s).right(digits);
		return s;
	}
};

struct _CColor
{
	QString name;				// always in the form '#xxx' or '#xxxxxx' where x is a hexadecimal digit
	int opacity=-100;			// opacity in percent. < 0: not used
	int changed = 0;			// 0: not, 1: color, 2: opacity 3: both changed
	QString operator=(QString s);
	operator QString() const;	// drop the '#'
	void Set(QString str, int opac);
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

	_CBorder &operator=(bool on) { changed = (used == on); used = on; return *this; }
	_CBorder &operator=(int w) { changed = (width == w); width = w; return *this; }
	_CBorder &operator=(_CColor &c) { color = c; changed = color.changed; return *this; }
	_CBorder &operator=(QString &c) { color = c; changed = color.changed; return *this; }
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

public:
	CONFIG();

	int majorStructVersion = 1, minorStructVersion = 1;		// 1.1

	void Read(const QString *dir = nullptr);
	void Write();

	CONFIG &operator=(const CONFIG &cfg);
	void FromDesign(const CONFIG &cfg);
	void FromOther(const CONFIG &cfg);
	void RestoreDesign();
	void RestoreOther();
	_CDirStr GalleryRoot() const	{ return _CDirStr(dsGallery) + _CDirStr(dsGRoot); }
	_CDirStr ImageDirectory() const { return GalleryRoot() + dsImageDir; }
	_CDirStr AlbumDirectory() const { return GalleryRoot() + dsAlbumDir; }

	_ChangedFlag changed;		// any of config is changed		THIS MUST BE THE FIRST MEMBER OF CONFIG! (why?)
	bool designChanged = false;	// page design is changed

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

	_CBool bAddTitlesToAll;		// into gallery.struct
	_CBool bAddDescriptionsToAll; // into gallery.struct
	_CBool bDisregardStruct;	// do not process gallery.struct

	_CString sUplink;			// path name of index.html file on server this gallery is embedded into.
								// this page if set should have links into dsGRoot for all languages
								// If given the main page;s uplink points here, otherwise no uplink on main page(s)
	_CString  sMainPage;		// name of the root file in the system The gallery root 
								// (default index.html or index1en.html, index1hu.html,...)
								// this is put into dsGRoot and not in the albums subdirectory, unless a path is given
								// The Home menu will link here (Example: index.html - home link: /<dsGRoot>/index.html)

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

	_CBool bfacebookLink;

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
	_CElem ImageDesc;			// 
				// images
	_CElem images;				// color for images and folders
	_CInt imageWidth;
	_CInt imageHeight;
	_CInt imageSizesLinked;
	_CBool doNotEnlarge;		// default: true
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
