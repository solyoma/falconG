#include <QPainter>
#include <QFontMetrics>
#include <QPen>
#include <QGraphicsDropShadowEffect>

#include "watermark.h"
#include "config.h"

// ************* struct Watermark ************

QString WaterMark::ColorToCss() const
{
	return QString("rgba(%1,%2,%3,%4)").arg((_colorWOpacity >> 16) & 0xFF).arg((_colorWOpacity >> 8) & 0xFF).arg(_colorWOpacity & 0xFF).arg(Opacity(false) / 255.0);
}
QColor WaterMark::BorderColor() const
{
	unsigned c = _colorWOpacity & 0xffffff;
	return (QColor(QString("#%1").arg(c, 6, 16, QChar(0))).value() < 170 ? "white" : "black");
}

/*=============================================================
 * TASK:	set positioning string for ".thumb::after"
 *			for the sample
 * EXPECTS:	width,height : for encllosing thumbnail
 *			ratio: thumbnail width / image width
 *			pos:	where to put it
 * GLOBALS: _marginX, _marginY
 * RETURNS:	2 line string for top and left set
 * REMARKS: ::after element would come after the main
 *			element, so both top and left will always be negative
 *------------------------------------------------------------*/
QString WaterMark::PositionToStyle(int width, int height, double ratio, POS pos) const// to set on .thumb::after
{
	if (pos < 0)
		pos = _origin;
	int left = -999999, top = -999999;	//impossible values
	switch (pos & 0xF0)
	{
	case LEFT:		left = -width + _marginX; break;
	case HCENTER:	left = -(width + _markWidth) / 2 + _marginX; break;
	case RIGHT:		left = -_markWidth - _marginX; break;
	}
	if (left < -width)
		left = -width;
	switch (pos & 0xF)
	{
	case TOP:		top = -height + _marginY; break;
	case VCENTER:	top = -(height + _markHeight) / 2 + _marginY; break;
	case BOTTOM:	top = -_markHeight - _marginY; break;
	}
	if (top < -height)
		top = -height;

	QString qs = QString("top:%1px;\n\tleft:%2px;\n").arg(int(top * ratio)).arg(int(left * ratio));
	return qs;
}

unsigned WaterMark::Color() const
{
	return _colorWOpacity;
}
double WaterMark::Opacity(bool percent) const		// 0..255 (!percent) or 0..100 (percent)
{
	return ((_colorWOpacity >> 24) & 0xFF) * (percent ? 100.0 / 255.0 : 1.0);
}

// setters
// setters	: each regenerates watermark image and saves it into 'res/'
#define SET_WM_VALUE(a,b)\
			if(a != (b))	\
			{  \
				_changed = true;	\
				a = (b); \
			}
void WaterMark::SetFont(QFont& qfont) // use either SetText() or GetMarkDimension after font is modified
{
	SET_WM_VALUE(_font, qfont);
	_font.setStyleHint(QFont::AnyStyle, QFont::PreferAntialias);
	SetupMark();
}
void WaterMark::SetText(QString  qs)	// calls GetMarkDimensions
{
	SET_WM_VALUE(_text, qs);
	GetMarkDimensions();
	SetupMark();
}
void WaterMark::SetColorWithOpacity(unsigned c_colorWOpacity)
{
	SET_WM_VALUE(_colorWOpacity, c_colorWOpacity);
	SetupMark();
}
void WaterMark::SetColorWithOpacity(QString s_colorWOpacity)
{
	SET_WM_VALUE(_colorWOpacity, s_colorWOpacity.toUInt(nullptr, 16));
	SetupMark();
}
void WaterMark::SetOpacity(int val, bool percent) // val is in percent (0..100) or not(0..255)?
{
	if (percent)
		val *= 2.55;
	unsigned cwo = (((int)(val)) << 24) + (qRed(_colorWOpacity) << 16) + (qGreen(_colorWOpacity) << 8) + qBlue(_colorWOpacity);
	SET_WM_VALUE(_colorWOpacity, cwo);
	SetupMark();
}

bool WaterMark::operator!=(const WaterMark& wm) const
{
	return (_changed != wm._changed)
		|| (_text != wm._text)
		|| (_origin != wm._origin)
		|| (_marginX != wm._marginX)
		|| (_marginY != wm._marginY)
		|| (_colorWOpacity != wm._colorWOpacity)
		|| (_background != wm._background)
		|| (_shadowColor != wm._shadowColor)
		|| (_font != wm._font);
}

void WaterMark::GetMarkDimensions()
{
	QFontMetrics fm(_font);
	_markWidth = fm.horizontalAdvance(_text);
	_markHeight = fm.height();
	if (_shadowOn)	// leave space for the shadow
	{
		_markWidth += _shadowHoriz;
		_markHeight += _shadowVert;
	}
}

/*=============================================================
 * TASK: generate watermark image for watermark text and
 *
 * EXPECTS:	font set, and correct text is also set,
 *			GetMarkDimensions() called at least once
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QImage* WaterMark::SetupMark()
{
	if (!_enabled)
		return nullptr;

	delete _pmark;
	_pmark = new QImage(_markWidth, _markHeight, QImage::Format_ARGB32);
	_pmark->fill(qRgba(0, 0, 0, 0));	// transparent image
	QPainter painter(_pmark);
	painter.setFont(_font);
	QColor c(qRed(_colorWOpacity), qGreen(_colorWOpacity), qBlue(_colorWOpacity), Opacity(false));
	// Debug 
	//QColor c(0xff,0,0,128);
	//QString n = c.rgba();
	QPen pen(c);
	painter.setPen(pen);

	int startX = 0, startY = 0;
	if (_shadowOn)
		startX = _shadowHoriz / 2, startY = _shadowVert / 2;
	painter.drawText(startX, startY, _markWidth, _markHeight, Qt::AlignCenter, _text);

	// DEBUG

	/*QFile fdbg("debug-watermark.txt");
	fdbg.open(QIODevice::WriteOnly);
	QTextStream odbg(&fdbg);
	odbg << "Font: " << font.family() << ", " << font.pointSize() << "pt, rgba:" << QString("#%1").arg(c.rgba(), 8, 16, QChar('0')) << ", penw:" << pen.width() << "\n";
	*/
	if (_shadowOn)
	{
		class MyShadow : public QGraphicsDropShadowEffect
		{
		public:
			MyShadow(QObject* parent = nullptr) :QGraphicsDropShadowEffect(parent) {}
			void draw(QPainter* painter) override { drawSource(painter); }
		}	shadow(nullptr);
		shadow.setXOffset(_shadowHoriz);
		shadow.setYOffset(_shadowVert);
		shadow.setBlurRadius(_shadowBlur);
		if (_shadowColor)
			shadow.setColor(_shadowColor);
		shadow.draw(&painter);
	}

	_pmark->save(PROGRAM_CONFIG::samplePath + "res/watermark.png");	// used on image
	return _pmark;
}

WaterMark& WaterMark::operator=(const WaterMark& other)
{
	_text = other._text;
	_origin = other._origin;

	delete _pmark;
	_pmark = nullptr;
	if (other._pmark)
		_pmark = new QImage(*other._pmark);

	_changed = other._changed;
	_markWidth = other._markWidth;
	_markHeight = other._markHeight;
	_colorWOpacity = other._colorWOpacity;
	_background = other._background;
	_useBackground = other._useBackground;
	_shadowOn = other._shadowOn;
	_shadowHoriz = other._shadowHoriz;
	_shadowVert = other._shadowVert;
	_shadowBlur = other._shadowBlur;
	_shadowColor = other._shadowColor;
	_font = other._font;
	return *this;
}

WaterMark& WaterMark::operator=(const WaterMark&& other)
{
	_text = other._text;
	_origin = other._origin;

	_pmark = other._pmark;

	_changed = other._changed;
	_markWidth = other._markWidth;
	_markHeight = other._markHeight;
	_colorWOpacity = other._colorWOpacity;
	_background = other._background;
	_useBackground = other._useBackground;
	_shadowOn = other._shadowOn;
	_shadowHoriz = other._shadowHoriz;
	_shadowVert = other._shadowVert;
	_shadowBlur = other._shadowBlur;
	_shadowColor = other._shadowColor;
	_font = other._font;
	return *this;
}

