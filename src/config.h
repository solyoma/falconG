#pragma once

#include <QtCore>
#include "enums.h"
using namespace Enums;
#include "support.h"
#include "stylehandler.h"

const QString falconG_ini = "falconG.ini";

// common templates for selecting the correct type of return value from settings
template <class T>
std::enable_if_t< std::is_same_v<bool, T>, bool> Value(QSettings& s, QString name, T def) { 
	return s.value(name, def).toBool(); }
template <class T>
std::enable_if_t< std::is_same_v<int, T>, int> Value(QSettings& s, QString name, T def) { 
	return s.value(name, def).toInt(); }
template <class T>
std::enable_if_t< std::is_same_v<double, T>, double> Value(QSettings& s, QString name, T def) { 
	return s.value(name, def).toDouble(); }
template <class T>
std::enable_if_t< std::is_same_v<QString, T>, QString> Value(QSettings& s, QString name, T def) 
{ 
//	return s.value(name, def).toString();  - this returns empty string for input lines like "Playfair, Georgia,serif|10pt|0|0"
	return s.value(name, def).toStringList().join(',');
}

// base class template for all config itemss. Based on functions Value() above.

template <class T> struct _CFG_ITEM
{
	QString itemName;	// in settings
	T	v,			// actual value
		vd,			// default value
		v0;			// original value	read from settings file

		// vd must be first otherwise problems with actual declarations
	explicit _CFG_ITEM(T vd, QString namestr) : itemName(namestr), vd(vd), v0(vd), v(vd) {}

	virtual bool Changed() const 
	{ 
		return v != v0; 
	}

	virtual void ClearChanged()
	{
		v0 = v;
	}

	virtual void Write(QSettings& s, QString group = QString() )	// into settings, always
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
	virtual void Read(QSettings& s, QString group = QString())
	{
		if(!group.isEmpty())
			s.beginGroup(group);

		v0 = v = Value(s, itemName, vd);	// not changed
		_Setup();	// may modify 'v'

		if (!group.isEmpty())
			s.endGroup();
	}
	_CFG_ITEM& operator=(const T o) 
	{ 
		v = o; 
		_Setup(); 
		return *this; 
	}
	_CFG_ITEM& operator=(const _CFG_ITEM o) 
	{ 
		v = o.v; v0 = o.v0, vd = o.vd; 
		_Setup(); 
		return *this; 
	}

	virtual bool operator==(const _CFG_ITEM& other) { return v == other.v;  }
	virtual bool operator!=(const _CFG_ITEM& other) { return !((const T)v == other.v);  }
protected:
	virtual void _Setup() {}		// preprocess after read from settings  v and v0 when created
	virtual void _Prepare() {}		// preprocess before write to settings
};

//--------------------------------------------------------------------------------------------

struct _CString : public _CFG_ITEM<QString>
{

	_CString(QString vd, QString namestr) : _CFG_ITEM(vd, namestr) {}
	_CString() : _CFG_ITEM(QString(), "cstring") {}

	QString& operator=(const QString s);
	QString& operator=(const _CString& s) { _CFG_ITEM::operator=(s);  return v; };
	bool operator==(const QString s) { return v == s; }
	bool operator==(const _CString &s) { return v == s.v; }
//	operator const QString() const { return ToString(); }
	operator QString() const { return ToString(); }
	void Clear() { v.clear(); }
	bool IsEmpty() const { return v.isEmpty();  }
	QString ToString() const { return v.isEmpty() ? vd : v; }
	int Length() const { return ToString().length(); }
};

//--------------------------------------------------------------------------------------------


struct _CDirStr : public _CString	 // always ends with '/'
{
	_CDirStr(QString vd, QString namestr="cdirstr") : _CString(vd, namestr) {  }
	_CDirStr() : _CString("/", "cdirstr") {}

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
#if 0
struct _CBool
{
	QString itemName;	// in settings
	bool vd,			// default value
		 v0,			// original value
		 v;			// actual value	read from settings file

		// vd must be first otherwise problems with actual declarations
	_CBool(bool vd, QString namestr) : itemName(namestr), vd(vd), v0(vd), v(vd) {}

	_CBool() : _CBool(false, "cbool") {}

	operator bool() const { return v; }
	QString ToString() const { return v ? "true" : "false"; }

	virtual bool Changed() const
	{
		return v != v0;
	}

	virtual void ClearChanged()
	{
		v0 = v;
	}

	virtual void Write(QSettings& s, QString group = QString())	// into settings, always
	{
		if (!group.isEmpty())
			s.beginGroup(group);

		if (v == vd)
			s.remove(itemName);		// from current group!
		else
			s.setValue(itemName, v);

		if (!group.isEmpty())
			s.endGroup();
	}
	virtual void Read(QSettings& s, QString group = QString())
	{
		if (!group.isEmpty())
			s.beginGroup(group);

		v = Value(s, itemName, vd);
		_Setup();	// may modify 'v'
		v0 = v;

		if (!group.isEmpty())
			s.endGroup();
	}
	_CBool& operator=(const bool o)
	{
		v = o;
		_Setup();
		return *this;
	}
	_CBool& operator=(const _CBool o)
	{
		v = o.v; v0 = o.v0, vd = o.vd;
		_Setup();
		return *this;
	}

	virtual bool operator==(const _CBool& other) { return v == other.v; }
	virtual bool operator!=(const _CBool& other) { return !((const bool )v == other.v); }
protected:
	virtual void _Setup() {}		// preprocess after read from settings  v and v0 when created
	virtual void _Prepare() {}		// preprocess before write to settings
};
#else
struct _CBool : public _CFG_ITEM<bool>
{
	_CBool(bool def, QString namestr) : _CFG_ITEM(def, namestr) {}
	_CBool() : _CFG_ITEM(false, "cbool") {}

	operator bool() const { return v; }
	QString ToString() const { return v ? "true" : "false"; }
	_CBool& operator=(const bool s) { _CFG_ITEM<bool>::operator=(s); return *this; }	// w. this template function is not used
};
#endif

//--------------------------------------------------------------------------------------------


struct _CInt : _CFG_ITEM<int>
{
	_CInt(int vd, QString namestr="cint") : _CFG_ITEM(vd, namestr) {}
	_CInt() : _CFG_ITEM(0, "cint") {}
	_CInt& operator=(int vn) 
	{ 
		v = vn; return *this; 
	}

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

struct _CIntArray : _CFG_ITEM<int>	// v: how many elements in array
{
	_CIntArray(int vd, QString namestr = "cintA") : _CFG_ITEM(vd, namestr) { _Setup();  }
	_CIntArray() : _CFG_ITEM(0, "cintA") {}

	int &operator[](int i)
	{
		if (i < 0) 
			return _arr[9999];	// may throw!
		while (i >= _arr.size())
			_arr.push_back(0), ++v;
		return _arr[i];
	}

	QString ToString()
	{
		QString qs;
		if (_arr.size())
		{
			qs = QString().setNum(_arr[0]);
			for (int i = 1; i < _arr.size(); ++i)
				qs += QString("|%1").arg(_arr[i]);
		}
		return qs;
	}
	void Set(QString qs)
	{
		_arr.clear();
		QStringList sl = qs.split('|');
		for (int i = 0; i < sl.size(); ++i)
			_arr.push_back(sl[i].toInt());
		v = _arr.size();
	}
	void Read(QSettings& s, QString group = QString()) override 
	{ 
		if (!group.isEmpty())
			s.beginGroup(group);

		QString qs = s.value("cintA", QString()).toString();
		Set(qs);	// may modify 'v'
		v0 = v;

		if (!group.isEmpty())
			s.endGroup();
	}
	void Write(QSettings& s, QString group = QString()) override
	{
		if (!group.isEmpty())
			s.beginGroup(group);
		QString qs = ToString();
		s.setValue("cintA", qs);
		if (!group.isEmpty())
			s.endGroup();
	}
private:
	QVector<int> _arr;
	void _Setup() 
	{ 
		if (vd && _arr.size() < vd)
			_arr.resize(vd);
	};
};

//--------------------------------------------------------------------------------------------
// IMPORTANT:
//	ForStyleSheet() functions below must return a string which does not end in "\n" !
//--------------------------------------------------------------------------------------------

struct _CColor : _CFG_ITEM<QString>	
{								
	// v is always in the form '#RGB' or '#RRGGBB  for full opacity
	//		'#AARRGGBB' for any opacity OR '#AARRGGBB-' when not used
	//  where each letter can be any hexadecimal digit (A: alpha)
	//	opacity may be missing and when it does full opacity is assumed
	_CColor(QString defaultColorString, QString namestr = "ccolor") : _CFG_ITEM(defaultColorString, namestr)
	{					
		_Setup();  // set internal string representation and opacity
	}
	_CColor() :_CFG_ITEM("", "ccolor")		// invalid color
	{					
	}

	bool Valid() { return _ColorStringValid(v); }	// opacity not checked

	void Set(QString str, int opac);	  // set _colorName to a 9 or 7 character string in format #AARRGGBB or #RRGGBB
	void SetColor(QString clr) { _colorName = clr; _Prepare(); }
	void SetOpacity(int opac, bool used, bool percent)		// opac 0..255 OR  0..100 (percent)
	{ 
		int limit = percent ? 100 : 255;
		if (opac > limit) 
			opac = limit;

		_opacityUsed = used;
		_opacity = opac;
		if (percent)
			_opacity = (int)(((int64_t)opac * (int64_t)255 + 100 / 2) / 100);
		
		_Prepare(); 
	}

	int Opacity(bool percent) const 			// opacity 0..100 
	{ 
		if(percent)
			return int( ((int64_t)100 * (int64_t)_opacity + 254)/255 );		// always stored as 0..255
		else
			return _opacity; 
	}

	bool IsOpacityUsed() const { return _opacityUsed; }

	QString Name(bool nohash = false) const				// no opacity!
	{
		return (nohash ? _colorName.mid(1) : _colorName); 			// nohash:   no '#' at start
	}

	QString ARGB() const { return v; }	// may or may not have an opacity set
	QString ToRgba() const		// return either rgb(RR GG BB)  or rgba(RR,GG,BB,AA/266.0) 
	{
		int n = _colorName.length() == 4 ? 3 : 1;	// either 4 ("#RGB") or 7 ("#RRGGBB")characters
		QString qs = QString("rgba(%1,%2,%3").arg(_colorName.mid(n, 2).toInt(nullptr, 16))
												 .arg(_colorName.mid(n+2, 2).toInt(nullptr, 16))
												 .arg(_colorName.mid(n+4, 2).toInt(nullptr, 16));
		if (_opacity >= 0)
			qs += QString(",%1").arg((double)_opacity / 255.0);
		qs += ")";
		return qs;
	}

	QString ForStyleSheet(bool addSemiColon, bool isBackground) const;

	QString operator=(QString s);
	_CColor& operator=(_CColor c);
	bool operator==(const _CColor& o);
	bool operator!=(const _CColor& o);

private:
	int _opacity = 255;			// 0..255 (=== 0..100 %)
	bool _opacityUsed = false;
	QString _colorName;		// starts with a '#' character	and always has 7 characters (no opacity)
							// otherwise invalid
	void _NormalizeName()
	{
		if (_colorName.isEmpty())
			return;
		if (_colorName.at(0) != '#') 
			_colorName = '#' + _colorName;
		if (_colorName.length() == 4)		// #RGB ? => #RRGGBB
			_colorName = _colorName.left(2) + _colorName.at(1) + _colorName.at(2) + _colorName.at(2) + _colorName.at(3) + _colorName.at(3);
	}
	void _Setup()			// from 'v' read from settings
	{
		if (!_ColorStringValid(v))
			return;
		int len = v.length(),			// can be 4, 7, 9, or 10 (#RGB, #RRGGBB #AARRGGBB #AARRGGBB- )  
			len1 = len == 10 ? 9 : len; // without the '-' at the end, any other cases opacity is 100% (255) and not used

		_opacityUsed = len == 9;

		if (len1 == 9)		// v == #AARRGGBB
		{
			_opacity = (v.mid(1, 2)).toInt(nullptr, 16);
			_colorName = "#" + v.mid(3,6);		// cut opacity part
		}
		else
		{	
			_colorName = v.left(len1);
			_opacity = 255;		// suppose it is not used
		}
		if (_opacity == 255)
			_opacityUsed = false;
		_NormalizeName();		// set to #AARRGGBB
	}
	void _Prepare() override		// v from internal (changed) variables
	{
		v = _colorName;			  // name w.o. opacity: #RRGGBB
		v = "#" + QString("%1").arg(_opacity, 2, 16, QChar('0')) + v.mid(1);	// name with opacity
		if (!_opacityUsed)				  // name with opacity: #AARRGGBB but AA ==0xFF => opacity = 100%
			v = v + '-';
	}
	bool _ColorStringValid(QString &s); // accepted formats (x: hexdecimal digit): xxx, #xxx, xxxxxx,#xxxxxx
};

//--------------------------------------------------------------------------------------------

// text-decoration: format single number ORed from <number for kind> and <number for style>
//						where kind: 0,1,2,4
//							  style; 8,16,32,64,128
//					see Decoration below
struct _CTextDecoration : _CFG_ITEM<int>
//--------------------------------------------------------------------------------------------
{
	
	_CTextDecoration(int vd=0, QString nameStr = "decoration") : _CFG_ITEM(vd, nameStr) {}

	void SetDecoration(Decoration d, bool on) 
	{ 
		v = (v & ~d) | (on ? (int)d : 0); 
	}

	bool IsTextDecorationLine() { return v & (tdUnderline | tdOverline | tdLinethrough); }
	bool IsUnderline() const { return v & tdUnderline; }
	bool IsOverline() const { return v & tdOverline; }
	bool IsLineThrough() const { return v & tdLinethrough; }

	QString TextDecorationLineStr() const
	{
		if( (v & (int)(tdUnderline | tdOverline | tdLinethrough)) == 0)
			return "none";

		return DecorationStr(tdUnderline) + 
			   DecorationStr(tdLinethrough) + 
			   DecorationStr(tdOverline)
			;
	}
	QString DecorationStr(Decoration deco) const
	{
	   if(v & (int)deco)
		   switch (deco)
		   {
			   case tdNone:					return "none ";
			   case tdUnderline:			return "underline ";
			   case tdLinethrough:			return "line-through ";
			   case tdOverline:				return "overline ";
			   case tdSolid:				return "solid ";
			   case tdDotted:				return "dotted ";
			   case tdDashed:				return "dashed ";
			   case tdDouble:				return "double ";
			   case tdWavy:					return "wavy ";
			   default: break;
		   }
	   return QString();
	}
	QString TextDecorationStyleStr() const
	{
		if (!v)
			return QString();

		return DecorationStr(tdSolid) + DecorationStr(tdDotted) + DecorationStr(tdDouble) + DecorationStr(tdWavy) + DecorationStr(tdDashed);	// only one of these must be set!
	}
	QString UnderlineStr() const { return v & tdUnderline ? "underline" : ""; }
	QString OverlineStr() const { return v & tdOverline ? "overline" : ""; }
	QString LineThroughStr() const { return v & tdLinethrough ? "line-through":""; }

	QString ForStyleSheet(bool addSemiColon) const;
};

struct _CTextAlign : _CFG_ITEM<int>
{
	_CTextAlign(int vd, QString nameStr) : _CFG_ITEM(vd, nameStr) {}

	QString AlignStr(Align a) const
	{
		switch (a)
		{
			default:
			case alNone:	return QString();
			case alLeft:	return "left";
			case alCenter:	return "center";
			case alRight:	return "right";
		}
	}
	QString ActAlignStr()  const
	{
		return AlignStr((Align)v);
	}
	QString ForStyleSheet(bool addSemiColon) const;
};

struct _CFont : _CFG_ITEM<QString>
{
	_CFont(QString vd, QString namestr = "cfont") : _CFG_ITEM(vd, namestr) { _Setup();  }
	_CFont() : _CFG_ITEM("\"Tms Rmn\",Times, Helvetica|10|0", "cfont") { _Setup(); }

	QString Family() const { return _details[fFam]; }
	int Features() const { return _details[fFeat].toInt(); }
	int Size() const { return _details[fSiz].toInt(); };

	QString ForStyleSheet(bool addSemiColon) const;

	QString FirstLineClassStr(const QString what)
	{
		if(IsFirstLineDifferent() )
			return what + "::first-line {\n	font-size:" + FirstLineFontSizeStr() + ";\n}\n\n";
		return QString();
	}

	bool Bold() const { return _details[fFeat].toInt() & fBold; }
	bool Italic() const { return _details[fFeat].toInt() & fItalic; }

	QString LineHeightStr() const { return _details[fLineH]; }
	QString SizeStr() const { return _details[fSiz]; }
	QString ItalicStr() const { return Italic() ? "italic" : ""; }
	QString WeightStr() const { return Bold() ? "900" : "normal"; }
	QString FirstLineFontSizeStr() const { return _details[fFirstS]; }
	bool IsFirstLineDifferent() const { return _details[fDiffF] == "1"; }

	void Set(QString fam, QString siz, QString slh, int feat, QString sFsSize = QString());
	void SetFamily(QString fam);
	void SetFeature(Style feat, FeatureOp op);
	void SetFeature(Style feat,  bool on);
	void SetSize(int pt);
	void SetSize(QString s);
	void SetLineHeight(QString qslh);
	void SetDifferentFirstLine(bool set, QString size = QString());
	
	void ClearFeatures();

protected:
	void _Setup() override;		// format: <family(ies)>|<size>|<line height>|<featurnumber>[|<diffFirstLine>|<firstSize>]
								// index:     0			   1		  2				 3			   4             5
	void _Prepare() override;	// from _details to 'v'
private:
	enum _what {fFam, fSiz,fLineH,fFeat, fDiffF, fFirstS};
	QStringList _details; 		// see format above
};

//--------------------------------------------------------------------------------------------

class _CShadow : public _CFG_ITEM<QString>
{
	// format: <use>|<horiz>|<vert>[|<blur radius-number>][|spread in pixels][|color name-starts with letter or '#'>]
	//   index 	  0      1	    2	      3							4				   5
	// 'spread is only used for box-shadow!
	QStringList _details;
	void _Setup() override;	// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'
public:
	_CShadow(QString vd, QString nameStr = "cshadow") : _CFG_ITEM(vd, nameStr) {  _Setup();  }
	_CShadow() : _CFG_ITEM("0|0|0|0|#000000","cshadoe") {  _Setup();  }	 // default  = "0|0|0|0|#000000"

	void Set(ShadowPart which, int value);
	void Set(int horiz, int vert, int blur, int spread, QString clr, bool used);
	void SetBlur(QString sc) { _details[3] = sc; _Prepare(); }
	void SetSpread(QString sc) { _details[4] = sc; _Prepare(); }
	void SetColor(QString sc) { _details[5] = sc; _Prepare(); }

	bool Used()	const { return _details[0].at(0) == '1' && (Horiz() + Vert()); }
	bool IsSet() const { return _details.size(); }

	int Horiz() const { return _details.size() ? _details[1].toInt() : 0; }
	int Vert() const { return _details.size()  ? _details[2].toInt() : 0; }
	int Blur() const { return _details[3].toInt(); }
	int Spread() const { return _details[4].toInt(); }
	QString Color() const { return _details[5] == "#000" ?  QString() : _details[5]; }	// optional color

	QString ForStyleSheet(bool addSemiColon, bool first_line, int which) const;		// line: 0 or 1 prepend "text-shadow:" and "box-shadow:"
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
	//	<used>|<p Top>|<c Top>|<p Middle>|<c Middle>|<p Bottom>|<c bottom>
	// u: used 0,1, p = position (%), c = color
	_CGradientStop gs[3];   // top, middle, bottom;
	bool used = false;		// same as the checkbox state

	_CGradient(QString vd, QString namestr="cgradient") : _CFG_ITEM(vd, namestr) { _Setup(); }
	_CGradient() : _CFG_ITEM(QString(), "cgradient") { _Setup(); }

	QString Color(GradStop stop) const { return gs[(int)stop].color; }

	// border color is its parent's border.color
	void Set(GradStop which, int pc, QString clr);
	void Set(int which, int pc, QString clr) { Set(GradStop(which), pc, clr); }
	void Set(bool used, int topStop, QString topColor, int middleStop, QString middleColor, int bottomStop, QString bottomcolor, bool chg = true);
	QString ForStyleSheet(bool addSemiColon) const;
	QString ForQtStyleSheet(bool invert = false) const;
private:
	bool _internal = false;	// true: do not _Prepare after set
	// format: <used>|<positionTop(%)>|<color Top>|<positionMiddle(%)>|<color Middle>|<positionBottom(%)>|<color bottom>
	// index	  0				1				2			3					4              5				6
	void _Setup() override;		// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'

};

//--------------------------------------------------------------------------------------------



/*========================================================
 * BORDERs
 *	settings format in ini file: accepts and saves
 * siz == 4    0      1		2	  3
 *			<used>|width|style|color
 * siz == 5    0      1		2	  3		4
 *			<used>|width|style|color|radius
 * siz == 7	   0			1					 2					 3					  4			 		 5					 6
 *			<used>|width top & bottom|width right and Left| style top & bottom|style right & left|color top & bottom|color right & left
 * siz == 8	   0			1					 2					 3					  4			 		 5					 6		   7
 *			<used>|width top & bottom|width right and Left| style top & bottom|style right & left|color top & bottom|color right & left|radius
 * siz == 13   0		1		   2			3			4		   5		 6		     7			  8			 9		  10		  11		   12
 *			<used>|width top|width right|width bottom|width Left|style top|style right|style bottom|style Left|color top|color right|color bottom|color Left|
 * siz == 14   0		1		   2			3			4		   5		 6		     7			  8			 9		  10		  11		   12       13
 *			<used>|width top|width right|width bottom|width Left|style top|style right|style bottom|style Left|color top|color right|color bottom|color Left|radius
 * no other formats acceptable
 *  internally always uses the longest of these
 *-------------------------------------------------------*/
struct _CBorder : public _CFG_ITEM<QString>
{
	_CBorder(QString vd, QString namestr="cborder") : _CFG_ITEM(vd, namestr) { _Setup(); }
	_CBorder() : _CFG_ITEM("0|0|0|0|#000000", "cborder") { _Setup(); }

	BorderSide actSide = sdAll;

	int Width(BorderSide sd) const
	{
		if (sd == sdAll)
			return _widths[0];
		return _widths[(int)sd];
	}

	QString ColorStr(BorderSide sd) const
	{
		if (sd == sdAll)
			sd = sdTop;
		return _colorNames[(int)sd];
	}

	int StyleIndex(BorderSide sd) const
	{
		if (sd == sdAll)
			return _styleIndex[0];
		return _styleIndex[(int)sd];
	}

	QString StyleStr(BorderSide sd)	 const
	{
		if (sd == sdAll)
			sd = sdTop;
		switch (_styleIndex[(int)sd])
		{
			default:
			case 0:	return "none";
			case 1: return "solid";
			case 2:	return "dotted";
			case 3:	return "dashed";
			case 4:	return "double";
			case 5:	return "groove";
			case 6:	return "ridge";
			case 7:	return "inset";
			case 8:	return "outset";
			
		}
	}

	int Radius() const 
	{ 
		return  _radius;
	}

	int BorderCnt() const { return _sizeWidths; }
	int UsedSides() const { return _used; }
	void SetUsed(BorderSide side, bool on) 
	{ 

		if (side == sdAll)
			_used = on ? 15 : 0;
		else
		{
			int mask, bit;
			switch (side)
			{
				case sdTop:		mask = 14; bit = 1; break;
				case sdRight:	mask = 13; bit = 2; break;
				case sdBottom:	mask = 11; bit = 4; break;
				default:
				case sdLeft:	mask =  7; bit = 8; break;
			}
			_used &= mask;
			if(on)
				_used |= bit;
		}
		_Prepare(); 
	}
	void SetWidth(BorderSide sd, int width) 
	{ 
		switch(sd)
		{
			case sdAll:		_widths[0] = _widths[1] = _widths[2] = _widths[3] = width; break;
			case sdTop:		_widths[0] = width; break;
			case sdRight:	_widths[1] = width; break;
			case sdBottom:	_widths[2] = width; break;
			case sdLeft:	_widths[3] = width; break;
			default:;
		}
		_CountWidths();
	}
	void SetColor(BorderSide sd, QString color)		// starts with '#'
	{
		if (color.at(0) != '#')
			color = "#" + color;
		switch (sd)
		{
			case sdAll:		_colorNames[0] = _colorNames[1] = _colorNames[2] = _colorNames[3] = color; break;
			case sdTop:		_colorNames[0] = color; break;
			case sdRight:	_colorNames[1] = color; break;
			case sdBottom:	_colorNames[2] = color; break;
			case sdLeft:	_colorNames[3] = color; break;
			default:;
		}
		_CountWidths();
		_Prepare();
	}
	void SetStyleIndex(BorderSide sd, int ix)	//ix==0 -> no border
	{
		if(sd == sdAll)
		   _styleIndex[0] = _styleIndex[1] = _styleIndex[2] = _styleIndex[3] = ix; 
		else
			_styleIndex[(int)sd] = ix;
		_CountWidths();
		_Prepare();
	}
	void SetRadius(int radius)
	{
		_radius = radius;
		_Prepare();
	}

	void SetUsedSide(BorderSide sd, bool on)
	{
		if (sd == sdAll)
			_used = on ? 15 : 0;	// 15 = 1 + 2 + 4 +8 
		else if (on)
			_used |= (1 << (int)sd);
		else
			_used &= ~(1 << (int)sd);
	}

	QString ForStyleSheet(bool semicolonAtLineEnds) const;		// w. radius
	QString ForStyleSheetShort(bool semicolonAtLineEnds) const;	// if  kind is sdAll simplified, else the same as the normal one
private:
	int _used = 0;		// bits 1 to 4 correspond to: top, right,bottom,left
	int _sizeWidths;	// how many different sides are there 1,2,4 - 4: all sizes are equal, 
	int _widths[4];		// width of a given side
	int _styleIndex[4];	// style of a given side
	QString _colorNames[4]; // color -"-
	int _radius;		// in px

	int _IndexOfColor(QStringList& _details)	const	// use on unprocessed stringlist
	{
		switch (_details.size())
		{
			case 4:	return 3;
			case 5: 
			case 6: return _details[4].at(0) == '#' ? 4 : 3;
			case 7:
			case 8: return 6;
			default: return 0;
		}
	}
	void _Setup() override;		// from 'v' to _details
	void _Prepare() override;	// from _details to 'v'
	void _CountWidths();		// set _sizeWidths
};

//--------------------------------------------------------------------------------------------

struct _CElem : public _CFG_ITEM<bool>		// v, vd, etc not used at all
{											// name is the elem name
	AlbumElement kind;
	_CColor color = { "#000000" ,"color"}, 
			background = {  "" ,"background"};	// invalid color
	_CFont font = {"\"Tms Rmn\",Times,serif|12pt|12pt|0","font"};
	_CInt spaceAfter = { 0, "after"};
	_CTextDecoration decoration = { 0, "decoration" };
	_CTextAlign alignment = { 0, "alignment" };
	_CShadow shadow1[2] = { {"0|0|0|0|#000000","text-shadow1"},	// index 0: text shadow, 1: box-shadow
							{"0|0|0|0|#000000","box-shadow1"} },
		     shadow2[2] = { {"0|0|0|0|#000000","text-shadow2"},
							{"0|0|0|0|#000000","box-shadow2"} };
	_CGradient gradient = { "0|0|#A90329|40|#890222|100|#6d0019"};
	_CBorder border = {"0|2|0|#890222","border"};

	_CElem* parent = nullptr;

	_CElem(AlbumElement kind, QString className, bool vd, QString namestr="celem", bool bShadow=false, bool bGradient=false) : 
		_CFG_ITEM(vd, namestr), kind(kind), _bMayShadow(bShadow), _bMayGradient(bGradient), _className(className) { }
	_CElem() : 
		_CFG_ITEM(false, "celem"), kind(aeUndefined), _bMayShadow(false), _bMayGradient(false) { }

	void SetParent(_CElem* p)
	{
		parent = p;
	}

	QString ClassName() const { return _className; }	// HTML class name for this element
	QString ColorsForStyleSheet(bool addSemicolon);
	QString ForStyleSheet(bool addSemicolon);		// internals of class for this element, excluding "first-line", ca't be const
	bool Changed() const override;
	void ClearChanged() override;

	void ColorsAndFontsFrom(const _CElem& other);	// all except 'name'

	virtual void Write(QSettings& s, QString group = QString()) override;
	virtual void Read(QSettings& s, QString group = QString()) override;

	_CElem& operator=(const _CElem& other);
	bool operator==(const _CElem& other) 
	{
		return  (color		  == other.color	 ) &&
				(background	  == other.background) &&
				(font		  == other.font		 ) &&
				(decoration	  == other.decoration) &&
				(alignment	  == other.alignment ) &&
				(shadow1[0]   == other.shadow1[0] ) &&
				(shadow2[0]   == other.shadow2[0] ) &&
				(shadow1[1]   == other.shadow1[1] ) &&
				(shadow2[1]   == other.shadow2[1] ) &&
				(gradient	  == other.gradient	 ) &&
				(border		  == other.border	 );
	}
private:
	bool _bMayShadow, _bMayGradient;
	QString _className;
};

//--------------------------------------------------------------------------------------------


struct _CWaterMark : public _CFG_ITEM<bool>, public WaterMark	  // v used for changed
{
	bool used = false;

	bool Changed() const override { return v; }

	_CWaterMark(bool vd, QString namestr = "watermark") : _CFG_ITEM(vd, namestr) { }
	_CWaterMark() : _CFG_ITEM(false, "watermark") { }
	_CWaterMark &operator=(const _CWaterMark &cwm);

	virtual void Write(QSettings& s, QString group = QString()) override;	// into settings, but only if changed
	virtual void Read(QSettings& s, QString group = QString()) override;
};

//--------------------------------------------------------------------------------------------
struct _CBackgroundImage : _CFG_ITEM<int>	// int: see enum 'BackgroundImageSizing'
{
	QString fileName;
	int size = 100;			// 1..100 (%)

	_CBackgroundImage(BackgroundImageSizing how, QString  nameStr="backgroundImagw") : _CFG_ITEM((int)how, nameStr) {}

	QString Url(bool addSemicolon) const;
	QString Size(bool addSemicolon) const;
	QString Position(bool addSemicolon) const;
	QString Repeat(bool addSemicolon) const;
	QString ForStyleSheet(bool addSemicolon) const;
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

struct PROGRAM_CONFIG
{
	static CONFIG *parent;		// struct Config 
	static QString homePath;	// local home directory
	static QString samplePath;	// sample directory
	// design page
	static int splitterLeft;
	static int splitterRight;
	// Options page
	static int lang;
	static QStringList qslLangNames;
	static int schemeIndex;			// 0->default, 1 -> system, 2 and more: changable.

	static int maxSavedConfigs;		// max this many last configuration directories are stored
	static int indexOfLastUsed;		// this was the last one used (may change to -1 for new source directory)
	static QStringList lastConfigs; // these are the (source) directories

	static void Read();	// the last directories used
	static void Write();

	static void GetHomePath();		// in users's home directory
	static void GetTranslations();	// from resources
	static QString LangNameListWDelim();	// e.g. en_US:hu_HU (always the default, en_US comes first)
	static void MakeValidLastConfig();	// using config.dsSrc
	static QString NameForConfig(bool forSave, QString sExt);	// returns either last part of lastConfigs[indexOfLastUsed] + sExt or other
};

//-------------------------
// IF MODIFIED modify Read(), Write(), ClearChanges(), FromOther and FromDesign() too
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

	bool Changed() const		
	{ 
		return _changed; 
	}

	bool SetChanged(bool chg) 
	{ 
		_changed |= chg; 
		return _changed; 
	}
	void ClearChanged();

	CONFIG &operator=(const CONFIG &cfg);

	QString RemoveSourceFromPath(QString path);
	QString AddSourceToPath(QString path);	// conditionally

	void FromDesign(const CONFIG &cfg);	// set designer part of cfg 
	void FromOther(const CONFIG &cfg);	// set other part from cfg

	void SaveDesign();		// into configSave in config.cpp
	void SaveOther();

	void RestoreDesign();	// from configSave
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
			// these are on the local machine
	_CDirStr GalleryRoot() const				{ return _CDirStr(dsGallery) + dsGRoot.ToString(); }
	_CDirStr LocalImageDirectory() const		{ return GalleryRoot() + dsImageDir; }
	_CDirStr LocalVideoDirectory() const		{ return GalleryRoot() + dsVideoDir; }
	_CDirStr LocalThumbnailDirectory() const	{ return GalleryRoot() + dsThumbDir; }
	_CDirStr LocalAlbumDirectory() const		{ return GalleryRoot() + dsAlbumDir; }
		// these are relative to the album directory
	_CDirStr RelativeImageDirectory() const		{ return _CDirStr("../") + dsImageDir; }
	_CDirStr RelativeVideoDirectory() const		{ return _CDirStr("../") + dsVideoDir; }
	_CDirStr RelativeThumbnailDirectory() const { return _CDirStr("../") + dsThumbDir; }

		  // **** Fields of CONFIG ***
	bool designChanged = false;	// page design is changed

	_CBool bNoMoreWarnings;		// stop displaying warnings

	// Gallery page
					// local directories
	_CDirStr dsApplication;		// home directory for the application (needed for copying resources from here)
	_CDirStr dsSrc = { "","dsSrc"};				// source directory (jalbum gallery root contains the actual falconG,ini)
	_CDirStr dsGallery = { "" ,"dsGallery"};			// destination directory on local machine (corresponds to 'public_html' on server)

					// remote directories
	_CDirStr dsGRoot = { "" ,"dsgRoot"};			// name of root directory on server (in public_html or in dsGallery)
	_CDirStr dsAlbumDir = {"albums/","dsAlbumDir"};		// (path) name of album directory (usually inside dsGRoot)
	_CDirStr dsCssDir = {"css/","dsCssDir"};
	_CDirStr dsFontDir = {"fonts/","dsFontDir"};
	_CDirStr dsImageDir = {"imgs/","dsImageDir"};		// images on server AND or destination
	_CDirStr dsThumbDir = {"thumbs/","dsThumbDir"};		// thumbnail directory on server AND or destination
	_CDirStr dsVideoDir = {"vids/","dsVideoDir"};		// videos on server AND or destination
	_CDirStr dsLastImageDir = { "","dsLastImageDir"};	// lats new image is loaded from here


	_CBool bSeparateFoldersForLanguages = { false, "cbSFL"};
	_CBool bKeepDuplicates = { false, "bKeepDuplicates" }; // when tru duplicates are kept
	_CBool bAddTitlesToAll = {false,"bAddTitlesToAll"};		// into gallery.struct
	_CBool bAddDescriptionsToAll = {false,"bAddDescriptionsToAll"}; // into gallery.struct
	_CBool bLowerCaseImageExtensions = {true,"bLowerCaseImageExtensions"}; // convert all image extensions to lowercase
	_CBool bReadFromDirs = {false,"bReadFromDirs"};			// do not process gallery.struct, read from JAlbum structure
	_CBool bReadFromGallery = {false,"bReadFromGallery"};	// read back from HTML files in gallery (you loose original names and paths!
		// next two bools will not be read from or saved into the configuration file
	_CBool bGenerateAll = {false,"bGenerateAll"};		// must regenerate all pages regardless of status?
	_CBool bButImages = {false,"bButImages"};			// used with 'generate All'
	_CBool bFixedLatestThumbnail = { true, "bFixedLatestThumbnail" };

	_CString sUplink = {"","sUplink"};			// path name of index.html file on server this gallery is embedded into.
								// this page if set should have links into dsGRoot for all languages
								// If given the main page;s uplink points here, otherwise no uplink on main page(s)
	_CString iconUplink = {"up-icon.png","iconUplink"};		// name of the icon file in the 'res' sub-directory
	_CString sMainPage = {"index.html"," sMainPage"};		// name of the root file in the system The gallery root 
								// (default index.html or index-en_US.html, index-hu_HU.html,...)
								// this is put into dsGRoot and not in the albums subdirectory, unless a path is given
								// The Home menu will link here (Example: index_en_US.html - home link: /<dsGRoot>/index.html)
	_CString sDescription = {"","sDescription"};		// page description: appears in we bsearch results
	_CString sKeywords = {"","sKeywords"};			// comma separated list of keywords

	_CBool bCanDownload = {false,"bCanDownload"};		// images from the server (.htaccess)
	_CBool bForceSSL = { true,"bForceSSL"};			// using .htaccess
	_CBool bRightClickProtected = {false,"bRightClickProtected"}; // right click protection on
	_CBool bMenuToContact = {false,"bMenuToContact"};		// show menu button for 'Contact' page
	_CBool bMenuToAbout = {false,"bMenuToAbout"};		// show menu button for 'About' page?
	_CBool bMenuToDescriptions = {true,"bMenuToDescriptions"};   // toggles image/album description visibility. If bMenuToToggleCaptions is false it also turns on/off captions and the icons
	_CBool bMenuToToggleCaptions = {false,"bMenuToToggleCaptions"}; // show captions at all? (see above)

	_CString sServerAddress = {"","sServerAddress"};	// server address:
	// special

	_CString sGoogleFonts = { "","sGoogleFonts"};		// default font names "https://fonts.googleapis.com/css?family="+these names separated with a pipe character
	_CString sDefFonts = { "","sDefFonts"};			// default serif font names
	_CString sBaseName = {"album","sBaseName"};			// default base name
	_CString sMailTo = {"","sMailTo"};			// send user emails here no defaults
	_CString sAbout = {"","sAbout"};			// address of about page, either relative to dsGRoot or any URI
	_CBool bOvrImages = {true,"bOvrImages"};			// overwrite images (default: true)
	_CBool bSourceRelativePerSign = {true,"bSourceRelativePerSign"}; // when true add source gallery to paths starting with '/'

		 		// latest uploads
	_CBool bGenerateLatestUploads = { false,"bGenerateLatestUploads"};	// or not
	_CInt newUploadInterval = { 10,"newUploadInterval"};		// date period of images calculated from the latest to include into latest images
	_CInt nLatestCount = { 10,"nLatestCount"};				// max this many images in the latest upload gallery
				// Google Analytycs
	_CBool googleAnalyticsOn = {false,"googleAnalyticsOn"};
	_CString googleAnalTrackingCode = {"","googleAnalTrackingCode"};

	_CString sGalleryTitle = {"Andreas Falco Photography","sGalleryTitle"};
	_CString sGalleryDesc = {"","sGalleryDesc"};
	_CString sGalleryLanguages = {"","sGalleryLanguages"};	// empty or a series of language abbreviations. Example: hu,en. 
								// Order important, 
								// All files for all abbreviations (e.g. 'en.lang') must exist 
								// Places: in the program directory, the source gallery directory or after the  
								// "[Language count:" line in the '.struct' file.

	_CBool bFacebookLink = { false,"bFacebookLink"};

	_CBackgroundImage backgroundImage = { hNotUsed, "backgroundImage" };
// Design page 
#define _DFLT_ false			// default?
#define _SHDW_ true				// may have shadow?
#define _NOSH_ false			// may not have shadow
#define _GRAD_ true				// may have gradient?
#define _NGRD_ false			// may not have gradient
	_CElem Web = {aeWebPage, "body", _DFLT_,"Web", _NOSH_, _NGRD_};		// page
	_CElem Header = {aeHeader, "header", _DFLT_,"Header", _NOSH_, _NGRD_ };		// header
	_CElem Menu = {aeMenuButtons,"menu-item", _DFLT_,"Menu", _SHDW_, _GRAD_};		// menu buttons
	_CElem Lang = {aeLangButton,"langs",  _DFLT_,"Lang", _NOSH_, _NGRD_};		// language link button
	// special handling : include shadows
	_CElem SmallGalleryTitle = {aeSmallTitle, "falconG", _DFLT_,"SmallGalleryTitle", _SHDW_, _NGRD_};	// "andreas falco photography"
	_CElem GalleryTitle = {aeGalleryTitle,"gallery-title", _DFLT_,"GalleryTitle", _SHDW_, _NGRD_};		
	_CElem GalleryDesc = {aeGalleryDesc,"gallery-desc", _DFLT_,"GalleryDesc", _SHDW_, _NGRD_};			
	_CElem Section = {aeSection,"fgsection", _DFLT_,"Section", _SHDW_, _NGRD_};							// "Images" or "Albums"
	_CElem Thumb = { aeThumb, "thumb", _DFLT_, "Thumb", _NOSH_, _NGRD_ };								// thumb: only a placeholder not saved into ini
	_CElem ImageTitle = {aeImageTitle, "title",_DFLT_, "AlbumOrImageTitle", _SHDW_, _NGRD_};			// not the image itself, just the texts!
	_CElem ImageDesc = {aeImageDesc, "desc", _DFLT_, "AlbumOrImageDesc", _SHDW_, _NGRD_};
	_CElem LightboxTitle = {aeLightboxTitle, "lightbox-caption",_DFLT_, "LightboxTitle", _SHDW_, _NGRD_};			// not the image itself, just the texts!
	_CElem LightboxDesc = {aeLightboxDescription, "lightbox-desc", _DFLT_, "LightboxDesc", _SHDW_, _NGRD_};
	_CElem Footer = {aeFooter, "footer", _DFLT_, "Footer", _NOSH_, _NGRD_};
#undef _DFLT_ 
#undef _SHDW_ 
#undef _NOSH_ 
#undef _GRAD_ 
#undef _NGRD_ 
	// 
				// image page
	_CInt imageWidth = {1920, "imageWidth"};
	_CInt imageHeight = {1080, "imageHeight"};
	_CInt imageQuality = { 0, "imageQuality" };	// same as original
	_CInt thumbWidth = {600, "thumbWidth"};
	_CInt thumbHeight = {400, "thumbHeight"};
	_CInt imageSizesLinked = {true, "imageSizesLinked"};
	_CBool doNotEnlarge = {true, "doNotEnlarge"};		// default: true
	_CBool bCropThumbnails = { false , "bCropThumbnails"};	// default: false exclusive with bDistortThumbnails
	_CBool bDistrortThumbnails = {false, "bDistrortThumbnails"}; // default: false eclusive with bCropThumbnails
				// image decoration
	_CBool iconToTopOn = {false, "iconToTopOn"};
	_CBool iconInfoOn = {false, "iconInfoOn"};
	_CInt  imageMargin = { 2, "imageMargin" };			// on img-container
	_CBorder imageBorder = {"0|2|0|#EAA41E", "imageBorder"};
	_CInt imageMatteWidth = { 0, "imageMatteWidth" };					  // width
	_CInt imageMatteRadius = { 0, "imgMRad" };
	_CColor imageMatteColor = { "#ccc", "imgMatteColor" };
	_CInt albumMatteWidth = { 0, "albumMatteWidth" };					  // width
	_CColor albumMatteColor = { "#ccc", "albumMatteColor" };
	_CInt albumMatteRadius = { 0, "abMRad" };
				// 	Watermarks
	_CWaterMark waterMark = {"", "Watermark"};

	_CInt  doNotShowTheseDialogs = { 0, "_doNotShowTheseDialogs" };
	_CIntArray defaultAnswers = { 6, "defaultAnswers" };	// for question dialogs 1 integer for each 
															// modify  number 6 for as many as there are items in enum 'DialogBits'
	// Debug
	_CBool bDebugging = {false, "bDebugging"};

		// calculated values	not saved not even included in the backup
	QString homeLink;		// actual home link for given language hierarchy
};

extern CONFIG config;		// Global variable
