// SlideWidget.cpp
#include "slideWidget.h"
#include "support.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTextLayout>
#include <QTextOption>
#include <QImageReader>

#include "stylehandler.h"

static QFont* __pf = nullptr;

// for double, ridge, groove, inset and outset: penStyle is set to Qt::Solid
// penColor is the lighter color for light mode and the darker color for dark mode

struct __Border
{
    QRectF rectf;        // for special lines (double, groove, etc) the outer rectangle
    QColor penLightColor[4];    // top, right, bottom, left. For styles from "Groove" the other color
                        // (darker  for light mode and lighter for dark mode) is calculated from these
                        // values, but these valus must contain the lighter color always
    qreal radii[4];     // TL, TR, BR, BL
    SlideWidget::SWBorderStyle penStyle[4]; // only Qt styles can be used for the actual drawing
    int bwidth[4];      // of the border, top, right, bottom, left (possibly 2 lines and a separator line)
    int penWidth[4];    // width of the pen for either the whole or partial border, top, right, bottom, left 
    int gap[4] = { 0 }; // only used when more than one line is drawn, it is set to penWidth for Double and
                        // 0 for Groove and Ridge
    void Setup(const QRectF& rect, 
               SlideWidget::SWBorderStyle style[4], 
               int width[4], QColor color[4])
    {
        Clear();
        rectf = rect;
        for (int i = 0; i < 4; ++i)
            SetupSide(SlideWidget::Side(i), style[i], width[i], color[i]);
	}
    // set the rectangle 'rectf' before the first paint message!
    
    void SetupSide(SlideWidget::Side side,
        SlideWidget::SWBorderStyle pStyle,
        int width, QColor color)
    {   
        int parts = ((int)pStyle < (int)SlideWidget::SWBorderStyle::Double) ?
                1 : (((int)pStyle == (int)SlideWidget::SWBorderStyle::Double) ?
                3 : 2);
        penStyle[(int)side] = pStyle;
        bwidth[(int)side] = width;
        penLightColor[(int)side] = color;
        if (parts > 1)    // then outer line is given with whole width
        {
            penWidth[(int)side]  = width / parts;
            gap[(int)side] = parts == 3 ? penWidth[(int)side] : 0;   // for 3 parts styles the gap is equal to the width of the line
		}
        else
        {
            penWidth[(int)side] = width;
            gap[(int)side] = 0;
        }
    }

    void Clear()
    {
        rectf = QRectF();
        for (int i = 0; i < 4; ++i)
        {
            penLightColor[i] = QColor();
            radii[i] = 0.0;
            penStyle[i] = SlideWidget::SWBorderStyle::None;
            penWidth[i] =  0;
        }
	}
    void Draw(QPainter& p, SlideWidget::Side side = SlideWidget::Side::sdAllSides, bool darkMode = false)
    {
        if (side == SlideWidget::Side::sdAllSides || (int)side > 4)
        {
			Draw(p, SlideWidget::Side::sdTop,      darkMode);
			Draw(p, SlideWidget::Side::sdRight,    darkMode);
			Draw(p, SlideWidget::Side::sdBottom,   darkMode);
			Draw(p, SlideWidget::Side::sdLeft,     darkMode);
        }
        else if (penStyle[(int)side] != SlideWidget::SWBorderStyle::None && penWidth[(int)side] > 0)
        {
            qreal w = rectf.width();
            qreal h = rectf.height();

            // Clamp radii
            qreal rs = qMax<qreal>(0, qMin(radii[(int)side], qMin(w, h) * 0.5)),
                  re = qMax<qreal>(0, qMin(radii[((int)side+1)%4], qMin(w, h) * 0.5));
            
            QPen pen(_PenStyleForSWStyle(penStyle[(int)side]));
            pen.setWidth(penWidth[(int)side]);   // already correct

            pen.setCapStyle(Qt::FlatCap);   // avoids corner overlaps

            // lines for side are drawn: 
            // top line from left to right + arc at the top right: 
            //              (left + r(top, left), top)  to  (right - radius(top, right), y)  then  right arc(90 -> -90)
            // right line down from top to bottom + arc at the bottom right: 
            //              (right, y + r(top, right))  to  (rigth, bottom - r(bottom, right)  then right bottom arc(0 -> -90)
            // bottom line from right to left + arc at bottom left:    
            //              (right - radius(bottom, right), bottom) to (x + r(bottom, left), y) then left bottom arc(270, -90)
            // left line up from bottom to top + arc at left top
            //              (left, bottom-r(left.bottom)) to (left, top - r(left, top) ) then arc left top (180 -> -90)
            int x1=0, y1=0, x2=0, y2=0, startAngle=0, sweepAngle=0; // all set below in getSideParams

            QRectF arcRect;
            QPainterPath sidePathOuter,sidePathInner;

            auto getSideParams = [&](SlideWidget::Side side)
                {
                    switch (side)
                    {
                        case SlideWidget::Side::sdTop:       // top line from left to right + arc at the top right:
                            x1 = rectf.left() + rs;
                            x2 = rectf.right() - re;
                            y1 = y2 = rectf.top();
                            startAngle = 90;
                            sweepAngle = -90;
                            arcRect = QRectF(x2 - 2*re, y1, 2*re, 2*re);
                            break;
                        case SlideWidget::Side::sdRight:       // right line down from top to bottom + arc at the bottom right
                            x1 = x2 = rectf.right();
                            y1 = rectf.top() + rs;
                            y2 = rectf.bottom() - re;
                            startAngle = 0;
                            sweepAngle = 90;
                            arcRect = QRectF(x2 - 2*re, y2, 2*re, 2*re);
                            break;
                        case SlideWidget::Side::sdBottom:       // bottom line from right to left + arc at the bottom right
                            x1 = rectf.right() - re;
                            x2 = rectf.left()  + rs;
                            y1 = y2 = rectf.bottom();
                            startAngle = 270;
                            sweepAngle = -90;
                            arcRect = QRectF(x1, y2 - 2*re, 2*re, 2*re);
                            break;
                        case SlideWidget::Side::sdLeft:      // left line up from bottom to top + arc at left top 
                            x1 = rectf.left() + rs;
                            x2 = rectf.right() - re;
                            y1 = y2 = rectf.top();
                            startAngle = 180;
                            sweepAngle = -90;
                            arcRect = QRectF(x1 , y2, 2*re, 2*re);
                            break;
                        default:    // should never selected
                            qDebug("Invalid side index:%d\n", (int)side);
                            break;
                    }

                };

            getSideParams(side);      // for outer rectangle

            p.setPen(pen); 
            p.setBrush(Qt::NoBrush);

            // prepare outer side
            _GetColor(pen, side, true, darkMode);  // modify color for actual side of outer rectangle 
            sidePathOuter.moveTo(x1, y1);
            if(re > 0)
                sidePathOuter.arcTo(arcRect, startAngle, sweepAngle);
            else
                sidePathOuter.lineTo(x2, y2);
            p.drawPath(sidePathOuter);

            if (bwidth[(int)side] != penWidth[(int)side])  // then we have another path, but inside
            {
                _GetColor(pen, side, false, darkMode);
                int dw = penWidth[(int)side] + gap[(int)side];
                switch (side)
                {
                    case SlideWidget::Side::sdTop:
                        y1 += dw; y2 += dw; x1 += dw, x2 -= dw;
                        arcRect.translate(-dw, dw);             // top right corner
                        break;
                    case SlideWidget::Side::sdRight:
                        x1 -= dw; x2 -= dw; y1 += dw; y2 -= dw;
                        arcRect.translate(-dw, -dw);            // bottom right corner
                        break;
                    case SlideWidget::Side::sdBottom:
                        y1 -= dw; y2 -= dw; x1 += dw, x2 -= dw;
                        arcRect.translate(+dw, -dw);            // bottom left corner
                        break;
                    case SlideWidget::Side::sdLeft:
                        x1 += dw; x2 += dw; y1 += dw; y2 -= dw;
                        arcRect.translate(dw, dw);            // top left corner
                        break;
                    default:
                        break;
                }
                sidePathInner.moveTo(x1, y1);
                if(re > 0)
                    sidePathInner.arcTo(arcRect, startAngle, sweepAngle);
                else
                    sidePathInner.lineTo(x2, y2);
                p.drawPath(sidePathInner);
            }
        }
    }
private:
    Qt::PenStyle _PenStyleForSWStyle(SlideWidget::SWBorderStyle sty)
    {
        if((int)sty <= (int)SlideWidget::SWBorderStyle::CustomDashLine)
		    return (Qt::PenStyle)sty;
        return Qt::SolidLine;
    }

    QColor _GetOtherColor(SlideWidget::Side side, bool darkMode) const // darker or lighter
    {
        // for dark mode, the darker color is used as the light color and the lighter color is calculated from it
        QColor otherColor = darkMode ? penLightColor[(int)side].lighter(150) : penLightColor[(int)side].darker(150);
        return otherColor;
	}
    void _GetColor(QPen &pen, SlideWidget::Side side, bool outer, bool darkMode) const 
    {
        QColor c = penLightColor[(int)side];
        switch (penStyle[(int)side])
        {
            case SlideWidget::SWBorderStyle::Groove:
                if( (outer &&  (side == SlideWidget::Side::sdTop || side ==SlideWidget::Side::sdLeft)) ||
                    (!outer && (side == SlideWidget::Side::sdBottom || side ==SlideWidget::Side::sdRight)) )     // or bottom and right inner color
                    c = _GetOtherColor(side,darkMode);          // is darker or lighter
                break;
            case SlideWidget::SWBorderStyle::Ridge:
                if( (!outer &&  (side == SlideWidget::Side::sdTop || side ==SlideWidget::Side::sdLeft)) ||
                    (outer && (side == SlideWidget::Side::sdBottom || side ==SlideWidget::Side::sdRight)) )     // or bottom and right inner color
                    c = _GetOtherColor(side,darkMode);          // is darker or lighter
                break;
            case SlideWidget::SWBorderStyle::Inset:
                if (side == SlideWidget::Side::sdTop || side == SlideWidget::Side::sdLeft)    // top and left color
                    c = _GetOtherColor(side,darkMode);          // is darker or lighter
                break;
            case SlideWidget::SWBorderStyle::Outset:
                if (side == SlideWidget::Side::sdRight || side == SlideWidget::Side::sdBottom)// top and left color
                    c = _GetOtherColor(side,darkMode);          // is darker or lighter
                break;
            default:
                break;
        }
        pen.setColor(c);
    }
};

//************* HELPERS ********************
// to draw a rectangle with different sides having different colors, 
// widths and styles and with rounded corners with different radii on each corner
    // ---------------------------------------------------------
    // 1) FILLED RECTANGLE
    // ---------------------------------------------------------
static void DrawCustomFilledCssRect(QPainter& p,
                                const QRectF& _rect,
                                const qreal _radii[4],     // TL, TR, BR, BL
                                const QColor _color,       // used as fill color for filled rectangle
                                bool filled = true,        // or border color for an outline
                                int borderWidth = 1)        // only used when not filled
{
    qreal w = _rect.width();
    qreal h = _rect.height();

    // Clamp radii
    qreal r[4];
    for (int i = 0; i < 4; ++i)
        r[i] = qMax<qreal>(0, qMin(_radii[i], qMin(w, h) * 0.5));

    const qreal x = _rect.x();
    const qreal y = _rect.y();
    const qreal x2 = x + w;
    const qreal y2 = y + h;

	QPainterPath fillPath;

	fillPath.moveTo(x + r[0], y);

	// Top → TR
	if (r[1] > 0) 
    {
		fillPath.lineTo(x2 - r[1], y);
		fillPath.arcTo(QRectF(x2 - 2 * r[1], y, 2 * r[1], 2 * r[1]), 90, -90);
	}
	else 
		fillPath.lineTo(x2, y);

	// Right → BR
	if (r[2] > 0) 
    {
		fillPath.lineTo(x2, y2 - r[2]);
		fillPath.arcTo(QRectF(x2 - 2 * r[2], y2 - 2 * r[2], 2 * r[2], 2 * r[2]), 0, -90);
	}
	else
		fillPath.lineTo(x2, y2);

	// Bottom → BL
	if (r[3] > 0) 
    {
		fillPath.lineTo(x + r[3], y2);
		fillPath.arcTo(QRectF(x, y2 - 2 * r[3], 2 * r[3], 2 * r[3]), 270, -90);
	}
	else
		fillPath.lineTo(x, y2);

	// Left → TL
	if (r[0] > 0) {
		fillPath.lineTo(x, y + r[0]);
		fillPath.arcTo(QRectF(x, y, 2 * r[0], 2 * r[0]), 180, -90);
	}
	else
		fillPath.lineTo(x, y);

	fillPath.closeSubpath();

    if (filled)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(_color);
    }
    else
    {
        QPen pen(_color);
        pen.setWidth(borderWidth);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
    }
	p.drawPath(fillPath);
	return;
}

    // ---------------------------------------------------------
    // 2) NON-FILLED: DRAW EACH SIDE SEPARATELY
	//      with given color, width and style and with arcs at 
    //      the corners if needed
    //    for this setup a __Border and use that  
    // ---------------------------------------------------------
#if 0
static void DrawCssRect(QPainter &p,
                           const QRectF &rect,
                           const qreal radii[4],       // TL, TR, BR, BL
                           const QColor color[4],      // top, right, bottom, left 
                           const SlideWidget::SWBorderStyle penStyle[4],
                           const int penWidth[4],      // top, right, bottom, left (width of border, may accomodate 2 lines and a separator line)
                           bool darkMode = false)      // for styles with 2 paths, when darkMode == false 
                                                       // color[] gives the lighter color otherwise the darker one
                                                       // which may change which one of them is used first
{
    qreal w = rect.width();
    qreal h = rect.height();

    // Clamp radii
    qreal r[4] = { 0.0 };
    for (int i = 0; i < 4; ++i)
        r[i] = qMax<qreal>(0, qMin(radii[i], qMin(w, h) * 0.5));

    const qreal x = rect.x();
    const qreal y = rect.y();
    const qreal x2 = x + w;
    const qreal y2 = y + h;

    int pathCount = 1;    // valid count in penPath, penColor, penWidth and pen
                          // 1: for Qt styles, 2 for double line, 4 for other CSS styles 
                          // w.o. Qt equivalent (0: left
	QPainterPath penPath[4];     // 1 for each side (order: top, right, bottom, left)
    QColor penColor[4];          // 2 for each side (indices 0,2: left and top, 1,3: right and bottom) 
	int  pW[4] = { 0.0 };        // pen widths, when pathCount is not 1 it is 2 for each side 
                                 // (indices 0,2: left and top, 1,3: right and bottom)
                                 // may be half (for double line) or 1/3rd (for bevelled) of penWidth[side]
    QPen sidePen[4];

        // QPen pen(color[sideIndex]);
        //      pen.setWidthF(penWidth[sideIndex] / 3.0);

	// draws one side of the rectangle with its styling. If hasArc is true, draws an arc at the end of the side
    // for non-Qt styles, call this twice with different pen widths and colors 
    auto drawSide = [&](int sideIndex,            // 0: top, 1: right, 2: bottom, 3: left
                        const QPointF &start,
                        const QPointF &end,
                        const QRectF &arcRect,
                        int startAngle,
                        int sweepAngle,
                        bool hasArc)
    {
        QPainterPath& sidePath = penPath[sideIndex];
		QPen& pen = sidePen[sideIndex];
		int& width = pW[sideIndex];

        sidePath.moveTo(start);

        if (hasArc)
            sidePath.arcTo(arcRect, startAngle, sweepAngle);
        else
            sidePath.lineTo(end);

        pen.setCapStyle(Qt::FlatCap);   // avoids corner overlaps
        // draw border using our styling because CSS and Qt border styles are different
        switch (penStyle[sideIndex]) // styles from NoPen to CustomDashLine (0..6)
        {                            // has an equivalent Qt::PenStyle style
            case SlideWidget::SWBorderStyle::DashDotLine:
                pen.setStyle(Qt::DashDotLine);
                break;
            case SlideWidget::SWBorderStyle::DashDotDotLine:
                pen.setStyle(Qt::DashDotDotLine);
                break;
            case SlideWidget::SWBorderStyle::CustomDashLine:   // no such possibility
                break;
            // for non-QT styles this function is called twice with different 
            // pen widths and colors to create the desired effect
            case SlideWidget::SWBorderStyle::Double:     // for all sides w. 2 paths, 1/3rd width wide and 1/3rd width apart
            case SlideWidget::SWBorderStyle::Groove:     // 4 calls, 2 for left+top and 2 for right+bottom, with half width
            case SlideWidget::SWBorderStyle::Ridge:      // with  different colors
			case SlideWidget::SWBorderStyle::Inset:      // 2 calls w. different colors, left top darker and right bottom lighter
			case SlideWidget::SWBorderStyle::Outset:     // 2 calls w. different colors, left top lighter and right bottom darker
                pen.setStyle(Qt::SolidLine);                   
                return;
            default: // style as CustomDashLine or invalid style
                pen.setStyle((Qt::PenStyle)penStyle[sideIndex]);
                pen.setWidthF(penWidth[sideIndex]);
                break;
        }
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
    };

    // ---- TOP (index 0) ----
    {
        QPointF s(x + r[0], y);           // start(left + left radius, y)
        QPointF e(x2 - r[1], y);          // end(right-right radius, y)

        bool arc = (r[1] > 0);            // right arc?
        QRectF arcRect(x2 - 2*r[1], y, 2*r[1], 2*r[1]);

        drawSide(0, s, e, arcRect, 90, -90, arc);
    }

    // ---- RIGHT (index 1) ----
    {
        QPointF s(x2, y + r[1]);
        QPointF e(x2, y2 - r[2]);

        bool arc = (r[2] > 0);
        QRectF arcRect(x2 - 2*r[2], y2 - 2*r[2], 2*r[2], 2*r[2]);

        drawSide(1, s, e, arcRect, 0, -90, arc);
    }

    // ---- BOTTOM (index 2) ----
    {
        QPointF s(x2 - r[2], y2);
        QPointF e(x + r[3], y2);

        bool arc = (r[3] > 0);
        QRectF arcRect(x, y2 - 2*r[3], 2*r[3], 2*r[3]);

        drawSide(2, s, e, arcRect, 270, -90, arc);
    }

    // ---- LEFT (index 3) ----
    {
        QPointF s(x, y2 - r[3]);
        QPointF e(x, y + r[0]);

        bool arc = (r[0] > 0);
        QRectF arcRect(x, y, 2*r[0], 2*r[0]);

        drawSide(3, s, e, arcRect, 180, -90, arc);
    }
}
#endif
// ******* constructor *******
SlideWidget::SlideWidget(QWidget* parent)  : QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
	//SetImage(QImage(":/Preview/Resources/placeholder.jpg"));
}



void SlideWidget::SetSizing(BoxSizing boxSizing)
{
    if (_boxSizing == boxSizing)
        return;
    _boxSizing = boxSizing;
    _mustUpdate = true;
}

void SlideWidget::SetAspectFlag(AspectRatio flag, bool on)
{
    if (on)
    {
        if (_imageSizeFlags.testFlag(flag) == on)
            return;
        _imageSizeFlags = AspectFlags();  // clear
        _imageSizeFlags.setFlag(flag, on);
    }
    else  // set flag off. Leave other flags intact
    {
        if(_imageSizeFlags.testFlag(flag))
            _imageSizeFlags.setFlag(flag, on);
    }
}

void SlideWidget::SlotSetupFromStyleString(QString& styleStr)
{
    StyleHandler sh(styleStr);
    QStringList selectors = sh.GetListOfSelectors();
    StyleRules styl;
    QString qs;

    using func = void (SlideWidget::*)(QString);
    // or 
    // typedef void (SlideWidget::*func)(QString str);

    static QMap<QString, func > rules = {
        { "box-sizing",                  &SlideWidget::_SetCssBoxSizing },
        { "font",                        &SlideWidget::_SetCssFont },
        { "font-size",                   &SlideWidget::_SetCssFontSize },
        { "font-family",                 &SlideWidget::_SetCssFontFamily },
        { "font-style",                  &SlideWidget::_SetCssFontStyle },
        { "padding",                     &SlideWidget::_SetCssPadding },
        { "padding-top",                 &SlideWidget::_SetCssPadding },
        { "paddig-right",                &SlideWidget::_SetCssPadding },
        { "paddig-bottom",               &SlideWidget::_SetCssPadding },
        { "padding-Left",                &SlideWidget::_SetCssPadding },
        { "border",                      &SlideWidget::_SetCssBorder },
        { "border-top",                  &SlideWidget::_SetCssBorder },
        { "border-right",                &SlideWidget::_SetCssBorder },
        { "border-bottom",               &SlideWidget::_SetCssBorder },
        { "border-left",                 &SlideWidget::_SetCssBorder },
        { "border-color",                &SlideWidget::_SetCssBorderColor },
        { "border-top-color",            &SlideWidget::_SetCssBorderTopColor },
        { "border-right-color",          &SlideWidget::_SetCssBorderRightColor },
        { "border-bottom-color",         &SlideWidget::_SetCssBorderBottomColor },
        { "border-left-color",           &SlideWidget::_SetCssBorderLeftColor },
        { "border-style",                &SlideWidget::_SetCssBorderStyle },
        { "border-top-style",            &SlideWidget::_SetCssBorderLeftStyle },
        { "border-right-style",          &SlideWidget::_SetCssBorderRightStyle },
        { "border-bottom-style",         &SlideWidget::_SetCssBorderBottomStyle },
        { "border-left-style",           &SlideWidget::_SetCssBorderLeftStyle },
        { "border-str",                  &SlideWidget::_SetCssBorderWidth },
        { "border-top-str",              &SlideWidget::_SetCssBorderTopWidth },
        { "border-right-str",            &SlideWidget::_SetCssBorderRightWidth },
        { "border-bottom-str",           &SlideWidget::_SetCssBorderBottomWidth },
        { "border-left-str",             &SlideWidget::_SetCssBorderLeftWidth },
        { "border-radius",               &SlideWidget::_SetCssBorderRadius },
        { "border-top-left-radius",      &SlideWidget::_SetCssBorderTopLeftRadius },
        { "border-top-right-radius",     &SlideWidget::_SetCssBorderTopRightRadius },
        { "border-bottom-right-radius",  &SlideWidget::_SetCssBorderBottomLeftRadius },
        { "border-bottom-left-radius",   &SlideWidget::_SetCssBorderBottomRightRadius },
    };
    for (QString& s : selectors)
    {
        if (sh.ObjectSelector(s) == objectName())   // then this is us
        {
            styl == sh.RulesForSelector(s);
            break;
        }
    }
    if (styl.isEmpty())
        styl = sh.Rules();
    for (auto it = styl.begin(); it != styl.end(); ++it)
    {
        if (rules.contains(it.key()))
            (this->*rules[it.key()])(it.value());
    }
}

void SlideWidget::SetImage(const QImage& img)
{
    _image = img;
    _mustUpdate = true;
}

void SlideWidget::SetImage(const QPixmap& pix)
{
    _image = pix.toImage();
    _mustUpdate = true;
}

void SlideWidget::SetImagePath(const QString& path)
{
    if (path == _imagePath)
        return;
    QImage img;
    if (QImageReader(path).read(&img))
        _image = img;
    else
        _image = QImage();     // If loading fails, clear image but still store path
    _imagePath = path;
    _mustUpdate = true;
}

qreal SlideWidget::GetRadius(QString qsRadiusWithUnit, RadiusUnit& radius)
{
    QString qsunit;
    for (auto rit = qsRadiusWithUnit.rbegin(); rit != qsRadiusWithUnit.rend() && !rit->isDigit(); ++rit)
            qsunit.prepend(*rit);
    radius = RadiusUnit::Unset;
    if (!qsunit.isEmpty())
    {
        int n = qsRadiusWithUnit.length() - qsRadiusWithUnit.length();
        QStringRef qs = QStringRef(&qsRadiusWithUnit, n, qsRadiusWithUnit.length() - n);
        qsRadiusWithUnit.truncate(n);
        if (qs == "px")
            radius = RadiusUnit::Pixels;
        else if(qs == "%")
            radius = RadiusUnit::Percent;
    }
    return qsRadiusWithUnit.toDouble();
}

void SlideWidget::SetImageSize(QSize displaySize)
{
    if (displaySize.isValid() && _imageSize != displaySize)
    {
        _imageSize = displaySize;
        _mustUpdate = true;
    }
}

void SlideWidget::SetImageRadius(qreal value, int side, RadiusUnit unit)
{
    if (unit != RadiusUnit::Unset)
    {
        _imageRadiusUnit[_RealSide(side)] = unit;
        _mustUpdate = true;
    }
}

void SlideWidget::SetImageRadius(FourReals value, RadiusUnit unit)
{
    for (int i = 0; i < 4; ++i)
        _imageRadius[i] = value[i];
    if (unit != RadiusUnit::Unset)
    {
        _imageRadiusUnit[0] = _imageRadiusUnit[1] = _imageRadiusUnit[2] = _imageRadiusUnit[3] = unit;
    }
	_mustUpdate = true;
}

void SlideWidget::SetImageRadiusUnit(RadiusUnit unit, int side)
{
    if(side > 3)
		return;
    if(unit != RadiusUnit::Unset)
    {
        if(side < 0)
            _imageRadiusUnit[0] = _imageRadiusUnit[1] = _imageRadiusUnit[2] = _imageRadiusUnit[3] = unit;
        else
			_imageRadiusUnit[side] = unit;
        _mustUpdate = true;
	}
}

void SlideWidget::SetBorderWidth(int w)
{
    _borderWidth[0] = _borderWidth[1] = _borderWidth[2] = _borderWidth[3] = w;
    _mustUpdate = true;
}

SlideWidget::SWBorderStyle SlideWidget::BorderStyleFromString(const QString qs)
{
    if (qs == "none")
        return SWBorderStyle::None;
    else if (qs == "solid")
        return SWBorderStyle::Solid;
    else if (qs == "dashed")
        return SWBorderStyle::Dashed;
    else if (qs == "dotted")
        return SWBorderStyle::Dotted;
    else if (qs == "dashDotLine")
        return SWBorderStyle::DashDotLine;
    else if (qs == "dashDotDotLine")
        return SWBorderStyle::DashDotDotLine;
    else if (qs == "customDashLine")
        return SWBorderStyle::CustomDashLine;
    else if (qs == "double")
        return SWBorderStyle::Double;
    else if (qs == "groove,")
        return SWBorderStyle::Groove;
    else if (qs == "ridge,")
        return SWBorderStyle::Ridge;
    else if (qs == "inset,")
        return SWBorderStyle::Inset;
    else if (qs == "outset,")
        return SWBorderStyle::Outset;
    else if (qs == "hidden")
        return SWBorderStyle::Hidden;
    return SWBorderStyle();
}

void SlideWidget::SetBorderStyle(SWBorderStyle ps)
{
    _borderStyle[0] = _borderStyle[1] = _borderStyle[2] = _borderStyle[3] = ps;
    _mustUpdate = true;
}

void SlideWidget::SetBorderStyle(QString qs) // e.g. "solid"
{
    SetBorderStyle(SlideWidget::BorderStyleFromString(qs));
}

void SlideWidget::SetBorderColor(const QColor& c)
{
    _borderColor[0] = _borderColor[1] = _borderColor[2] = _borderColor[3] = c;
    _mustUpdate = true;
}

void SlideWidget::SetMatteWidth(int w)
{
    _matteWidth[0] = _matteWidth[1] = _matteWidth[2] = _matteWidth[3] = w;
    _mustUpdate = true;
}

void SlideWidget::SetMatteWidth(FourInts w)
{
     _matteWidth[0] = w[0]; _matteWidth[1] = w[1]; 
     _matteWidth[2] = w[2]; _matteWidth[3] = w[3]; 
     _mustUpdate = true; 
}

void SlideWidget::SetMatteColor(const QColor& c)
{
    _matteColor[0] = _matteColor[1] = _matteColor[2] = _matteColor[3] = c;
    _mustUpdate = true;
}

void SlideWidget::SetBorderRadius(qreal value, RadiusUnit unit)
{
    _borderRadiusValue[0] =_borderRadiusValue[1] =_borderRadiusValue[2] =_borderRadiusValue[3] = value;
    _borderRadiusUnit[0] = _borderRadiusUnit[1] = _borderRadiusUnit[2] = _borderRadiusUnit[3] = unit;
    _mustUpdate = true;
}

void SlideWidget::SetBorderRadius(QString qsRadiusString)
{
    RadiusUnit unit;
    qreal radius;
    radius = GetRadius(qsRadiusString, unit);
    SetBorderRadius(radius, unit);
}

void SlideWidget::SetMatteRadius(qreal value, RadiusUnit unit)
{
    _matteRadiusValue[0] = _matteRadiusValue[1] = _matteRadiusValue[2] = _matteRadiusValue[3] = value;
    _matteRadiusUnit[0]  = _matteRadiusUnit[1]  = _matteRadiusUnit[2]  = _matteRadiusUnit[3]  = unit;
    _mustUpdate = true;
}

void SlideWidget::SetMatteRadius(QString qsRadiusString)
{
    RadiusUnit unit;
    qreal radius;
    radius = GetRadius(qsRadiusString, unit);
    SetMatteRadius(radius, unit);
}

void SlideWidget::SetAllBordersFromString(const QString qsBorder)
{
    QStringList qsl = qsBorder.split(' ');
    if (qsl[0] == 'none')
        return;

    qreal radius;
    RadiusUnit runit;
    radius = GetRadius(qsl[0], runit);
    SetBorderRadius(radius, runit);
    if (qsl.size() > 1)
    {
        SetBorderStyle(qsl[1]);
        if (qsl.size() > 2)
            SetBorderColor(qsl[2]);
    }
}

void SlideWidget::SetBorderRadiusUnit(RadiusUnit unit)
{
    _borderRadiusUnit[0] = _borderRadiusUnit[1] = _borderRadiusUnit[2] = _borderRadiusUnit[3] = unit;
    _mustUpdate = true;
}

void SlideWidget::SetMatteRadiusUnit(RadiusUnit unit)
{
    _matteRadiusUnit[0] = _matteRadiusUnit[1] = _matteRadiusUnit[2] = _matteRadiusUnit[3] = unit;
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkText(const QString& text)
{
    _watermarkIsText = true;
    _watermarkText = text;
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkImage(const QImage& img)
{
    _watermarkIsText = false;
    _watermarkImage = img;
    _mustUpdate = true;
}

void SlideWidget::ClearWatermark()
{
    _watermarkText.clear();
    _watermarkImage = QImage();
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkImagePath(const QString& path)
{
    if (path == _watermarkImagePath)
        return;
    QImage img;
    if (QImageReader(path).read(&img)) {
        _watermarkImage = img;
        _watermarkImagePath = path;
        _watermarkIsText = false;
        _mustUpdate = true;
    } else {
        _watermarkImage = QImage();
        _watermarkImagePath = path;
        _mustUpdate = true;
    }
}

void SlideWidget::SetWatermarkFont(const QFont& font)
{
    _watermarkFont = font;
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkColor(const QColor& color)
{
    _watermarkColor = color;
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkOpacity(qreal opacity)
{
    _watermarkOpacity = opacity;
    _mustUpdate = true;
}

void SlideWidget::SetWatermarkAlignment(HAlign h, VAlign v)
{
    _hAlignWmark = h;
    _vAlignWmark = v;
    _mustUpdate = true;
}

SlideWidget::AspectRatio SlideWidget::ImageAspectRatio() const
{
    return (AspectRatio)(int)_imageSizeFlags;
}

void SlideWidget::SetImageAspectRatio(AspectRatio ar, bool on)
{
    if (on == _imageSizeFlags.testFlag(ar))
        return;
	if (on)
    {
        _imageSizeFlags = AspectFlags();   // clear all flags
        _imageSizeFlags.setFlag(ar, true); // and only set this one
    }
	_mustUpdate = true;
}

void SlideWidget::SetMousePressCallback(MouseCallback cb) { _mousePressCb = cb; }
void SlideWidget::SetMouseReleaseCallback(MouseCallback cb) { _mouseReleaseCb = cb; }
void SlideWidget::SetMouseClickCallback(MouseCallback cb) { _mouseClickCb = cb; }
void SlideWidget::SetMouseDoubleClickCallback(MouseCallback cb) { _mouseDoubleClickCb = cb; }
void SlideWidget::SetHoverCallback(HoverCallback cb) { _hoverCb = cb; }

QRect SlideWidget::_BorderRect()        // call this first when _boxSizing == BorderBox
{               
    if (_mustUpdate && _boxSizing == BoxSizing::BorderBox)       // call first
            _borderRect = rect();                            // relative to widget
    return _borderRect;
}

QRect SlideWidget::_MatteRect()     // part inside the border, including the image area
{                                   // relative to border
    if (_mustUpdate)     // call after _borderRect is set
      _matteRect = _borderRect.adjusted(
                _borderWidth[sdLeft],
                _borderWidth[sdTop],
                -_borderWidth[sdRight],
                -_borderWidth[sdBottom]
                                       );
    return _matteRect;
}

/*=============================================================
 * TASK   : optionally calculate and return the image rectangle
 *          depending on _boxSizing
 from the total drawable 
 *          rectangle of the widget, i.e. the border rectangle
 * PARAMS : none
 * EXPECTS: _imageSize, _mustUpdate, _boxSizing are set
 *          if _boxSizing == BorderBox: both border and matte 
 *              widths are already set and _imageRect is
 *              the area inside them, relative to borderRect
 *              and _imageSize doesn't matter
 *          else _boxSizing == ContextBox so the widget rectangle
 *              must be changed, because _imageRect is now
 *              determined by _imageSize
 *              and this function must be called before the matte
 *              then the border rectangle is calculated
 * GLOBALS: width(), height(), _borderWidth, _broderheight, 
 *          _matteWidth, _matteHeight, _keepAspectRatio, 
 *          _image.width(), _image.height()
 * RETURNS: image rectangle
 * REMARKS: - if _mustUpdate == false return pre-calculated value
 *            else calculate it
 *          - if _boxSizing == BorderBox:
 *                  subtracts matte and border
 *            if _boxSizing == ContentBox 
 *                  returns the image geometry
 *------------------------------------------------------------*/
QRect SlideWidget::_ImageRect()
{
    auto imageRectFromMatteRect = [&]()->void
        {
            _imageRect = _matteRect.adjusted  (
                                _matteWidth[sdLeft],
                                _matteWidth[sdTop],
                                -_matteWidth[sdRight],
                                -_matteWidth[sdBottom]
                            			);
        };

	if (_mustUpdate)
	{
        QRect r;
        if (_boxSizing == BoxSizing::BorderBox)  
        {                                        
            _BorderRect();      // the whole widget area
            _MatteRect();       // matte (padding) inside _borderRect
            // _imageRect: area that remains after border and matte are set
            imageRectFromMatteRect();
            // _imageRect may differ from the one determined by _imageSize
            // 
            // for KeepAspectRatio this must be corrected so that the whole image is 
            //  confined into it

            if (_imageSizeFlags.testFlag(AspectRatio::KeepAspectRatio)) // then fit the image into _imageRect and keep aspect ratio
            {
                int dw = _imageRect.width(), dh = _imageRect.height();
                qreal magnx = ((qreal)dw) / _image.width(), // magnification in x direction to leave horizontal matte margins
                    magny = ((qreal)dh) / _image.height();  // same for y leaving vertical matte margins
                // the smaller magnification will be used
                if (magnx > magny)       // then the x size of the image rectangle is correct
                    _imageRect.setWidth(_image.width() * magny);
				else if (magnx < magny)       // then the y size of the image rectangle is correct
                    _imageRect.setHeight(_image.height() * magnx);
				// else magnifications are the same so _imageRect is ok as it is
            }
        }
        else // if(_boxSizing == BoxSizing::ContentBox) -> _imageRect must be kept
        {
			int dw = _imageSize.width()  + _borderWidth[sdLeft] + _borderWidth[sdRight]  + _matteWidth[sdLeft] + _matteWidth[sdRight],
				dh = _imageSize.height() + _borderWidth[sdTop]  + _borderWidth[sdBottom] + _matteWidth[sdTop]  + _matteWidth[sdBottom];
			_borderRect = QRect(0, 0, dw, dh);       // expand from _imageRect
            (void)_MatteRect(); // relative to borderRect
			imageRectFromMatteRect();               // don't forget to resize the widget!
        }
		// _imageRect is now the rectangle to be used for the image, relative to the border rectangle.
	}
    return _imageRect;
}

QRect SlideWidget::_WatermarkRect()     // _ImageRect() must be called before this
{
    QRect imgRect = _ImageRect();

    QSize size;
    if (_watermarkIsText) {
        QTextLayout layout(_watermarkText, _watermarkFont);
        layout.beginLayout();
        QTextLine line = layout.createLine();
        layout.endLayout();
        size = QSize(
            static_cast<int>(line.naturalTextWidth()),
            static_cast<int>(line.height())
             );
    }
    else {
        size = _watermarkImage.size();
    }

    int x = imgRect.left();
    int y = imgRect.top();

    if (_hAlignWmark == HAlign::Center)
        x = imgRect.center().x() - size.width() / 2;
    else if (_hAlignWmark == HAlign::Right)
        x = imgRect.right() - size.width();

    if (_vAlignWmark == VAlign::Center)
        y = imgRect.center().y() - size.height() / 2;
    else if (_vAlignWmark == VAlign::Bottom)
        y = imgRect.bottom() - size.height();

    return QRect(QPoint(x, y), size);
}

/*=============================================================
 * TASK   : paints the slide widget
 * PARAMS : event - the paint event
 * EXPECTS: all rectangles and radii are set (calculated) and cached in member variables
 *          if box sizing is *ContentBox* the widget sizes must have been adjusted already!
 * GLOBALS:
 * RETURNS: none
 * REMARKS: - first paints the whole widget with the matte color 
 *            and rounded corners if any is specified
 *          - then the image inside _imageRect either scaled to fit
 *              or scaled and cropped using the apect ratio flags
 *          - then the icons if any are specified in _marks into their
 *              rectangle on the image
 *          - then a (possibly rounded) rectangle around the image
 *              in the matte colors
 *          - then draw the border with the same rounded corners 
 *              as for the whole widget
 *              rectangle already set relative to the image rectangle
 *          - lastly draw the mark icons if any specified
 *              over the border if it is wide enough or on the matte if it isn't
 *------------------------------------------------------------*/
void SlideWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Step 1 matte background                      // matte is the whole background and includes the image border
	DrawCustomFilledCssRect(p, rect(), _matteRadiusValue, _matteColor[0]);
    // Step 2 - image
    if (!_image.isNull())
    {
		QRect imageRect = _ImageRect();
		if(_imageSizeFlags.testFlag(AspectRatio::CropImage)) // then crop the image to fit _imageRect
        {
            QImage scaled = _image.scaled(imageRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QRect cropRect((scaled.width() - imageRect.width()) / 2, (scaled.height() - imageRect.height()) / 2, imageRect.width(), imageRect.height());
            p.drawImage(_imageRect, scaled, cropRect);
		}
        else // 'AspectRatio::Scale' or 'AspectRatio::KeepAspectRatio:
            p.drawImage(imageRect, _image);  //  possibly stretch the image to fit _imageRect
    }

    // Step 3 - Watermark
    if ((!_watermarkIsText && !_watermarkImage.isNull())
        || (_watermarkIsText && !_watermarkText.isEmpty()))
    {
        p.setOpacity(_watermarkOpacity);
        QRect wRect = _WatermarkRect();

        if (_watermarkIsText) 
        {
            p.setPen(_watermarkColor);
            
            QTextLayout layout(_watermarkText, _watermarkFont);
            QTextOption opt;
            opt.setAlignment(Qt::AlignCenter);
            layout.setTextOption(opt);
            
            layout.beginLayout();
            QTextLine line = layout.createLine();
            line.setLineWidth(wRect.width());
            layout.endLayout();
            
            line.draw(&p, QPointF(
                    wRect.left(),
                    wRect.top()
                     ));
        }
        else {
            p.drawImage(wRect, _watermarkImage);
        }
        p.setOpacity(1.0);
    }
	// Step 4 - matte border (not a filled rectangle) around the image
	 DrawCustomFilledCssRect(p, _imageRect, _matteRadiusValue, _matteColor[0], false /*not filled*/, _borderRadiusValue[0]);
	 // Step 5 - border around the whole widget
     __Border border;
     border.Setup(_borderRect, _borderStyle, _borderWidth, _borderColor);
     border.Draw(p, SlideWidget::Side::sdAllSides, false);       // TODO false = light mode
    // icons on image
    if(_marks.isEmpty() == false)
    {
        for(const auto& marks : _marks)
        {
            int x = 0;
            for(const auto& mark : marks.imgs)
            {
                QRect markRect(x, 0, mark.width(), mark.height());
                if(marks.hAlign == HAlign::Center)
                    markRect.moveLeft((width() - mark.width()) / 2);
                else if(marks.hAlign == HAlign::Right)
                    markRect.moveLeft(width() - mark.width());
                if(marks.vAlign == VAlign::Center)
                    markRect.moveTop((height() - mark.height()) / 2);
                else if(marks.vAlign == VAlign::Bottom)
                    markRect.moveTop(height() - mark.height());
                p.drawImage(markRect, mark);
                x += mark.width();
            }
        }
	}
}

SlideWidget::SlideArea
SlideWidget::HitTest(const QPoint& pos) const
{
    if (_watermarkRect.contains(pos))
        return SlideArea::Watermark;

    if (_imageRect.contains(pos))
        return SlideArea::Image;

    if (_matteRect.contains(pos))
        return SlideArea::Matte;

    if (_borderRect.contains(pos))
        return SlideArea::Border;

    return SlideArea::Image;
}

void SlideWidget::mousePressEvent(QMouseEvent* e)
{
    _lastPressPos = e->pos();
    if (_mousePressCb)
        _mousePressCb(HitTest(e->pos()), e);
}

void SlideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (_mouseReleaseCb)
        _mouseReleaseCb(HitTest(e->pos()), e);

    if ((e->pos() - _lastPressPos).manhattanLength() < 4) {
        if (_mouseClickCb)
            _mouseClickCb(HitTest(e->pos()), e);
    }
}

void SlideWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (_mouseDoubleClickCb)
        _mouseDoubleClickCb(HitTest(e->pos()), e);
}

void SlideWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (_hoverCb) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QHoverEvent he(QEvent::HoverMove, e->position(), e->position());
        _hoverCb(HitTest(e->position().toPoint()), &he);
#else
        QHoverEvent he(QEvent::HoverMove, e->pos(), e->pos());
        _hoverCb(HitTest(e->pos()), &he);
#endif
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SlideWidget::enterEvent(QEnterEvent*) {}
#else
void SlideWidget::enterEvent(QEvent*) {}
#endif
void SlideWidget::leaveEvent(QEvent*) {}
void SlideWidget::resizeEvent(QResizeEvent*) {}

void SlideWidget::SlotSetImage(QImage* pImage, VAlign alignment, bool keepAspectRatio)
{
    _image=*pImage;
    SetAspectFlag(AspectRatio::KeepAspectRatio, keepAspectRatio);
}
void SlideWidget::SlotSetImage(QPixmap* pPixmap, VAlign alignment, bool keepAspectRatio)
{
    _image= pPixmap->toImage();
    SetAspectFlag(AspectRatio::KeepAspectRatio, keepAspectRatio);
}
void SlideWidget::SlotSetBorder(int side, int width, QColor color, qreal radius, RadiusUnit unit, SWBorderStyle borderStyle)
{
    if (side < 0 || side > 3)
    {
		_borderWidth[0] = _borderWidth[1] = _borderWidth[2] = _borderWidth[3] = width;
        _borderColor[0] = _borderColor[1] = _borderColor[2] = _borderColor[3] = color;
        _borderRadiusValue[0] = _borderRadiusValue[1] = _borderRadiusValue[2] = _borderRadiusValue[3] = radius;
        _borderRadiusUnit[0] = _borderRadiusUnit[1] = _borderRadiusUnit[2] = _borderRadiusUnit[3] = unit;
        _borderStyle[0] = _borderStyle[1] = _borderStyle[2] = _borderStyle[3] = borderStyle;
    }
    else
    {
        _borderWidth[side] = width;
        _borderColor[side] = color;
        _borderRadiusValue[side]= radius;
        _borderRadiusUnit [side]= unit;
        _borderStyle[side] = borderStyle;
    }
}
void SlideWidget::SlotSetMatte(int side, int width, QColor color, qreal radius, RadiusUnit unit)
{
    if (side < 0 || side > 3)
    {
        _matteWidth[0] = _matteWidth[1] = _matteWidth[2] = _matteWidth[3] = width;
        _matteColor[0] = _matteColor[1] = _matteColor[2] = _matteColor[3] = color;
        _matteRadiusValue[0] = _matteRadiusValue[1] = _matteRadiusValue[2] = _matteRadiusValue[3] = radius;
        _matteRadiusUnit[0] = _matteRadiusUnit[1] = _matteRadiusUnit[2] = _matteRadiusUnit[3] = unit;
    }
    else
    {
        _matteWidth[side] = width;
        _matteColor[side] = color;
        _matteRadiusValue[side] = radius;
        _matteRadiusUnit[side] = unit;
    }
}
void SlideWidget::SlotAddIcons(QVector<QImage> icons, HAlign hAlign, VAlign vAlign)
{
    Marks marks; marks.imgs = icons; marks.hAlign = hAlign; marks.vAlign = vAlign;
    _marks.push_back(marks);
}
void SlideWidget::SlotSetWatermarkText(const QString &text)
{
    SetWatermarkText(text);
}
void SlideWidget::SlotSetWatermarkImage(const QImage &image)
{
    SetWatermarkImage(image);
}
void SlideWidget::SlotSetWatermarkFont(const QFont& font)
{
    SetWatermarkFont(font);
}
void SlideWidget::SlotSetWatermarkOpacity(qreal opacity)
{
    SetWatermarkOpacity(opacity);
}
void SlideWidget::SlotSetWatermarkAlignment(HAlign ha, VAlign va)
{
    SetWatermarkAlignment(ha, va);
}
void SlideWidget::SlotSetMousePressCallback(const MouseCallback cb)
{
    SetMousePressCallback(cb);
}
void SlideWidget::SlotSetMouseReleaseCallback(const MouseCallback cb)
{
    SetMouseReleaseCallback(cb);
}
void SlideWidget::SlotSetMouseClickCallback(const MouseCallback cb)
{
    SetMouseClickCallback(cb);
}
void SlideWidget::SlotSetMouseDoubleClickCallback(const MouseCallback cb)
{
    SetMouseDoubleClickCallback(cb);
}
void SlideWidget::SlotSetHoverCallback(const HoverCallback cb)
{
    SetHoverCallback(cb);
}
void SlideWidget::Update()
{
    if (!_mustUpdate) return;

    QRect gr = geometry();  // starting geometry of the widget
                            // either the border rectangle (if box-sizing is border-box) 
                            // or the image rectangle (if box-sizing is content-box)
                            // when the 2nd: the widget is resized
    if (_boxSizing == BoxSizing::BorderBox) // gr == rect()
    {
        (void)_BorderRect();// which now == rect()
        (void)_MatteRect();     // here this uses _BorderRect()
        (void)_ImageRect();     // this uses _MatteRect()
    }
    else    // _boxSizeing == BoxSizing::ContentBox
    {
        (void)_ImageRect();     // returns _imageRect = gr. which will be modified later
		(void)_MatteRect();     // now this uses _ImageRect()
		(void)_BorderRect();    // this uses _MatteRect() and _ImageRect()
    }
    _mustUpdate = false; 
    update(); 
}
void SlideWidget::_SetCssBoxSizing(QString boxSizing)  // no "initial" or "inherit"!
{
    if (boxSizing == "border-box")
        SetSizing(BoxSizing::BorderBox);
    else // if (boxSizing == "content-box")
        SetSizing(BoxSizing::ContentBox);
}
//------------------------------------------------------------------------------------
   // functions for css
void SlideWidget::_SetCssFont(QString fontdef)         // font:[style] [variant] [weight] <size>["/"line-height] <family>
{                                                      // but variant is not possible (TODO) and line-height is discarded
    QFont f = font();
    __pf = &f;     // static global used in functions
    QStringList sl = fontdef.split(' ', Qt::SkipEmptyParts);    // 2 ... 5 strings
    switch (sl.size())
    {
        case 2: // size and family            only px sizes are handled
            _SetCssFontSize(sl[0]);
            _SetCssFontFamily(sl[1]);
            break;
        case 3: // style size family OR weight size family
            if (sl[0] == "italic")
                f.setItalic(true);
            else if (sl[0] == "bold")
                f.setBold(true);
            else if (sl[0][0].isDigit())// weight in number like 700
                f.setWeight(QFont::Weight(sl[0].toInt()));
            else
                f.setItalic(false), f.setBold(false);   // yes, comma
            _SetCssFontSize(sl[0]);
            _SetCssFontFamily(sl[1]);
            break;
        case 4: // style weight size family
            f.setItalic(sl[0] == "italic");
            if (sl[1] == "bold")
                f.setBold(true);
            else if (sl[1][0].isDigit())// weight in number like 700
                f.setWeight(QFont::Weight(sl[1].toInt()));
            else
                f.setBold(false);
            _SetCssFontSize(sl[0]);
            _SetCssFontFamily(sl[1]);
            break;
        default:
            break;
    }
    setFont(f);
    __pf = nullptr;
}
void SlideWidget::_SetCssFontSize(QString s)
{
        int pos = s.indexOf('/');
        int n = pos;
        if (pos < 0)                   // else line height is also given
            n = s.length();
        if(__pf)
            __pf->setPixelSize(s.left(n - 2).toInt());
        else
        {
            QFont f = font();
            f.setPixelSize(s.left(n - 2).toInt());
            setFont(f);
        }
}

void SlideWidget::_SetCssFontFamily(QString family)  // "Arial , Tms Rmn "
{
    QFont f = font(), *pf = __pf;
    if (!__pf)
         pf = &f;

    if (family.indexOf(','))
    {
        QStringList sl = family.split(QRegularExpression("[ \t]*,[ \t]*"), Qt::SkipEmptyParts);
        pf->setFamilies(sl);
    }
    else
        __pf->setFamily(family);
    if (!__pf)
        setFont(f);
}     
void SlideWidget::_SetCssFontStyle(QString style)
{
    QFont f = font(),
         *pf = __pf;
    if (!pf)
        pf = &f;

    pf->setItalic(style == "italic");

    if (!__pf)
        setFont(f);
}       

void SlideWidget::_SetCssFontWeight(QString s)
{
    QFont f = font(),
         *pf = __pf;
    if (!pf)
        pf = &f;

    if (s == "bold")
        pf->setBold(true);
    else if (s[0].isDigit())// weight in number like 700
        pf->setWeight(QFont::Weight(s.toInt()));
    else
        pf->setBold(false);

    if (!__pf)
        setFont(f);
}       

void SlideWidget::_SetCssMatteRadius(QString radius)
{
}
// 'nonQtPenStyle' for styles not present in Qt::PenStyle, e.g. "double" or "groove" etc. which will be handled in DrawCssRect
QStringList SlideWidget::_GetCssWidthStyleColor(const QString& str, QString &size, SWBorderStyle &penStyle, QColor &color)
{
	static const QStringList styles = { "none", "solid", "dotted", "dashed", "double", 
                                        "groove", "ridge", "inset", "outset" };
	static const SWBorderStyle penStyles[] = { SWBorderStyle::None, SWBorderStyle::Solid, 
                                                SWBorderStyle::Dotted, SWBorderStyle::Dashed, 
                                                SWBorderStyle::Double,SWBorderStyle::Groove, 
                                                SWBorderStyle::Ridge, SWBorderStyle::Inset, 
                                                SWBorderStyle::Outset
    };

    int n = -1;

    QStringList sl = str.split(' ', Qt::SkipEmptyParts);
    auto getCsspart = [&](const QString& s) -> void {
		if (s.endsWith("px") || s.endsWith("em") || s.endsWith("rem") || s.endsWith("%"))
            size = s;
        else if ((n = styles.indexOf(s)) >= 0)
            penStyle = penStyles[n];
        else
            color = s;
	};

    for(int i = 0; i < sl.size(); ++i)
        getCsspart(sl[i]);
    return sl;
}

void SlideWidget::_SetCssPadding(QString padding)
{
    QStringList sl = padding.split(' ', Qt::SkipEmptyParts);
    switch (sl.size())
    {
		case 1:  _matteWidth[sdTop] = _matteWidth[sdRight] = _matteWidth[sdBottom] = _matteWidth[sdLeft] = sl[0].toInt(); break;
		case 2:  _matteWidth[sdTop] = _matteWidth[sdBottom] = sl[0].toInt(); _matteWidth[sdRight] = _matteWidth[sdLeft] = sl[1].toInt(); break;
		case 3:  _matteWidth[sdTop] = sl[0].toInt(); _matteWidth[sdRight] = _matteWidth[sdLeft] = sl[1].toInt(); _matteWidth[sdBottom] = sl[2].toInt(); break;
		default: _matteWidth[sdTop] = sl[0].toInt(); _matteWidth[sdRight] = sl[1].toInt(); _matteWidth[sdBottom] = sl[2].toInt(); _matteWidth[sdLeft] = sl[3].toInt(); break;
    }
    _mustUpdate = true;
}       
void SlideWidget::_SetCssPaddingTop(QString padding)
{
    _matteWidth[sdTop] = padding.toInt();
	_mustUpdate = true;
}    
void SlideWidget::_SetCssPaddingRight(QString padding)
{
	_matteWidth[sdRight] = padding.toInt();
	_mustUpdate = true;
}  
void SlideWidget::_SetCssPaddingBottom(QString padding)
{
	_matteWidth[sdBottom] = padding.toInt();
	_mustUpdate = true;
} 
void SlideWidget::_SetCssPaddingLeft(QString padding)
{
	_matteWidth[sdLeft] = padding.toInt();
	_mustUpdate = true;
}   

void SlideWidget::_SetCssBorder(QString borderStyleString)   // "width style color" common for all sides
{
	QString width; // includes 'px','%','en','em' etc. but only 'px' is recognized yet
	SWBorderStyle style = SWBorderStyle::None;
	QColor color;
    _GetCssWidthStyleColor(borderStyleString, width, style, color);
    if (!width.isEmpty())
    {
        if (width.endsWith("px"))     // but only 'px' is recognized yet
            width.chop(2);
        else
            for (int n = width.length() - 1; 0; --n)    // default value if unit is not recognized
                if (!width[n].isDigit())
                    width.chop(1);
    }
    else
        width = "1";   // default value if width is not given

    _borderWidth[sdTop] = _borderWidth[sdRight] = _borderWidth[sdBottom] = _borderWidth[sdLeft] = width.toInt();
    if ((int)style != (int)Qt::CustomDashLine)
		_borderStyle[sdTop] = _borderStyle[sdRight] = _borderStyle[sdBottom] = _borderStyle[sdLeft] = style;
}

void SlideWidget::_SetCssBorderTop(QString border)
{
}    
void SlideWidget::_SetCssBorderRight(QString border)
{
}  
void SlideWidget::_SetCssBorderBottom(QString border)
{
} 
void SlideWidget::_SetCssBorderLeft(QString border)
{
}   

void SlideWidget::_SetCssBorderColor(QString str)
{
}     

void SlideWidget::_SetCssBorderTopColor(QString border)
{
}    
void SlideWidget::_SetCssBorderRightColor(QString border)
{
}  
void SlideWidget::_SetCssBorderBottomColor(QString border)
{
} 
void SlideWidget::_SetCssBorderLeftColor(QString border)
{
}   

void SlideWidget::_SetCssBorderWidth(QString str)
{
}     
void SlideWidget::_SetCssBorderRadius(QString str)
{
}    

void SlideWidget::_SetCssBorderStyle(QString style)
{
}         
void SlideWidget::_SetCssBorderTopStyle(QString style)
{
}      
void SlideWidget::_SetCssBorderRightStyle(QString style)
{
}    
void SlideWidget::_SetCssBorderBottomStyle(QString style)
{
}   
void SlideWidget::_SetCssBorderLeftStyle(QString style)
{
}     

void SlideWidget::_SetCssBorderTopWidth(QString str)
{
}
void SlideWidget::_SetCssBorderRightWidth(QString str)
{
}
void SlideWidget::_SetCssBorderBottomWidth(QString str)
{
}
void SlideWidget::_SetCssBorderLeftWidth(QString str)
{
}

void SlideWidget::_SetCssBorderTopLeftRadius(QString str)
{
}
void SlideWidget::_SetCssBorderTopRightRadius(QString str)
{
}
void SlideWidget::_SetCssBorderBottomLeftRadius(QString str)
{
}
void SlideWidget::_SetCssBorderBottomRightRadius(QString str)
{

}
