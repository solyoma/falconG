#pragma once
#ifndef _WATERMARK_H
#define _WATERMARK_H

#include <QString>
#include <QImage>
#include <QFont>
#include <QColor>
class WaterMark
{
public:
	enum POSITION : int {
		LEFT = 0
		, HCENTER = 16
		, RIGHT = 32
		, TOP = 0
		, VCENTER = 1
		, BOTTOM = 2
	};
	using POS = int;

protected:
	bool _enabled = true;	// otherwise do not create image
	bool _changed = false;	// any changes
	bool _regenImagesWhenChanged = false;
private:
	//using POS = int;

	QString _text;
	int _origin = LEFT + TOP;	// 0,1,2		- index of vertical	position (top,center,bottom)
	// 0,0x10,0x20	- index of horizontal position  (left, center, right)

	QImage* _pmark = nullptr;				// watermark text as an image
	int _markWidth = 0, _markHeight = 0;	// image width and height

	int _marginX = 0, // >= 0, measured from nearest horizontal image edge in pixels
		_marginY = 0; // >= 0, measured from nearest vert. image edge

	unsigned _colorWOpacity = 0x00ffffff,		// AARRGGBB
		_background = 0;
	bool	_useBackground = false;

	bool _shadowOn = false;
	int _shadowHoriz, _shadowVert;
	unsigned _shadowBlur;
	unsigned _shadowColor = unsigned(-1); // color (rgb) -1: not used

	QFont _font;
public:

	QImage* SetupMark();
	WaterMark& operator=(const WaterMark& other);
	WaterMark& operator=(const WaterMark&& other);
	bool operator!=(const WaterMark& wm) const;

	QImage* PMarkImage() const { return _pmark; }
	QString Text() const { return _text; }
	int Origin() const { return _origin; }

	bool WChanged() const { return _changed && _regenImagesWhenChanged; }

	int Width() const { return _markWidth; }
	int Height() const { return _markHeight; }
	int MarginX() const { return _marginX; }
	int MarginY() const { return _marginX; }
	QFont Font() const { return _font; }
	bool HasDropShadow(int* xoffset = nullptr, int* yoffset = nullptr, QColor* color = nullptr) const
	{
		if (xoffset) *xoffset = _shadowHoriz;
		if (xoffset) *yoffset = _shadowVert;
		if (color) *color = _shadowColor == unsigned(-1) ? QColor(63, 63, 63, 180) : QColor(_shadowColor);

		return _shadowOn;
	}
	void GetMarkDimensions();	// into _markWidth and _markHeight using current font
	QString PositionToStyle(int width, int height, double ratio, POS newPosition = -1) const;

	QString ColorToCss() const;
	QColor BorderColor() const;
	unsigned Color() const;	// with opacity
	unsigned ShadowColor() const { return _shadowColor; };	// with opacity
	unsigned Background() const { return _background; }
	double Opacity(bool percent) const;		// 0..255 (!percent) or 0..100 (percent)

	void ClearChanges() { _changed = false; }
	void SetRegenerationMode(bool regen) { _regenImagesWhenChanged = regen; }
	// setters	: each regenerates watermark image and saves it into 'res/'
#define SET_WM_VALUE(a,b)\
			if(a != (b))	\
			{  \
				_changed = true;	\
				a = (b); \
			}
	void SetPositioning(int pos) { SET_WM_VALUE(_origin, pos) }
	void SetBackground(unsigned bck) { SET_WM_VALUE(_background, bck) }
	void SetColorWithOpacity(unsigned colorWOpacity);
	void SetColorWithOpacity(QString sColorWOpacity);
	void SetFont(QFont& qfont);
	void SetMarginX(int mx) { SET_WM_VALUE(_marginX, mx) }
	void SetMarginY(int my) { SET_WM_VALUE(_marginY, my) }
	void SetText(QString  qs);
	void SetOpacity(int val, bool percent); // val is in percent (0..100) or not(0..255)?
	void SetShadowColor(unsigned color) { SET_WM_VALUE(_shadowColor, color) }
	void SetShadowOn(bool on) { SET_WM_VALUE(_shadowOn, on) }
	void SetShadowBlur(unsigned blur) { SET_WM_VALUE(_shadowBlur, blur) }
#undef SET_WM_VALUE
};

#endif //_WATERMARK_H