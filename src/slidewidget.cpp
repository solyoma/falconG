// SlideWidget.cpp
#include "SlideWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTextLayout>
#include <QTextOption>
#include <QImageReader>

// ******* constructor *******
SlideWidget::SlideWidget(QWidget* parent)  : QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
	//SetImage(QImage(":/Preview/Resources/placeholder.jpg"));
}

void SlideWidget::SetImage(const QImage& img)
{
    _image = img;
    update();
}

void SlideWidget::SetImage(const QPixmap& pix)
{
    _image = pix.toImage();
    update();
}

void SlideWidget::SetImagePath(const QString& path)
{
    if (path == _imagePath)
        return;
    QImage img;
    if (QImageReader(path).read(&img)) {
        _image = img;
        _imagePath = path;
        update();
    } else {
        // If loading fails, clear image but still store path
        _image = QImage();
        _imagePath = path;
        update();
    }
}

void SlideWidget::SetBorderWidth(int w)
{
    _borderWidth = w;
    update();
}

void SlideWidget::SetBorderStyle(Qt::PenStyle ps)
{
    if (_borderStyle == ps)
        return;
    _borderStyle = ps;
    update();
}

void SlideWidget::SetBorderColor(const QColor& c)
{
    _borderColor = c;
    update();
}

void SlideWidget::SetMatteWidth(int w)
{
    _matteWidth = w;
    update();
}

void SlideWidget::SetMatteColor(const QColor& c)
{
    _matteColor = c;
    update();
}

void SlideWidget::SetBorderRadius(qreal value, RadiusUnit unit)
{
    _borderRadiusValue = value;
    _borderRadiusUnit = unit;
    update();
}

void SlideWidget::SetMatteRadius(qreal value, RadiusUnit unit)
{
    _matteRadiusValue = value;
    _matteRadiusUnit = unit;
    update();
}

void SlideWidget::SetBorderRadiusUnit(RadiusUnit unit)
{
    _borderRadiusUnit = unit;
    update();
}

void SlideWidget::SetMatteRadiusUnit(RadiusUnit unit)
{
    _matteRadiusUnit = unit;
    update();
}

void SlideWidget::SetKeepAspectRatio(bool keep)
{
    _keepAspectRatio = keep;
    update();
}

void SlideWidget::SetWatermarkText(const QString& text)
{
    _watermarkIsText = true;
    _watermarkText = text;
    update();
}

void SlideWidget::SetWatermarkImage(const QImage& img)
{
    _watermarkIsText = false;
    _watermarkImage = img;
    update();
}

void SlideWidget::ClearWatermark()
{
    _watermarkText.clear();
    _watermarkImage = QImage();
    update();
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
        update();
    } else {
        _watermarkImage = QImage();
        _watermarkImagePath = path;
        update();
    }
}

void SlideWidget::SetWatermarkFont(const QFont& font)
{
    _watermarkFont = font;
    update();
}

void SlideWidget::SetWatermarkColor(const QColor& color)
{
    _watermarkColor = color;
    update();
}

void SlideWidget::SetWatermarkOpacity(qreal opacity)
{
    _watermarkOpacity = opacity;
    update();
}

void SlideWidget::SetWatermarkAlignment(HAlign h, VAlign v)
{
    _hAlign = h;
    _vAlign = v;
    update();
}

void SlideWidget::SetMousePressCallback(MouseCallback cb) { _mousePressCb = cb; }
void SlideWidget::SetMouseReleaseCallback(MouseCallback cb) { _mouseReleaseCb = cb; }
void SlideWidget::SetMouseClickCallback(MouseCallback cb) { _mouseClickCb = cb; }
void SlideWidget::SetMouseDoubleClickCallback(MouseCallback cb) { _mouseDoubleClickCb = cb; }
void SlideWidget::SetHoverCallback(HoverCallback cb) { _hoverCb = cb; }

qreal SlideWidget::EffectiveRadius(bool matte) const
{
    if(matte)
    {
        if (_matteRadiusUnit == RadiusUnit::Pixels)
            return _matteRadiusValue;
        // Percent of smallest side
        qreal minSide = qMin(width(), height());
        return minSide * _matteRadiusValue / 100.0;
    }
    else
    {
        if (_borderRadiusUnit == RadiusUnit::Pixels)
            return _borderRadiusValue;
        // Percent of smallest side
        qreal minSide = qMin(width(), height());
        return minSide * _borderRadiusValue / 100.0;
    }

}

QRect SlideWidget::BorderRect() const
{
    return rect();
}

QRect SlideWidget::MatteRect() const     // part inside the border, but outside the image
{
    if (_keepAspectRatio)
    {
        return ImageRect().adjusted(
            -_matteWidth,
            -_matteWidth,
            _matteWidth,
            _matteWidth
        );
    }
    else
    {
        return rect().adjusted(
            _borderWidth,
            _borderWidth,
            -_borderWidth,
            -_borderWidth
        );
    }
}

QRect SlideWidget::ImageRect() const     // relative to slide's rectangle
{
    if (_keepAspectRatio)  // check how to fit the image
    {
        int dw = width()  - 2 * (_borderWidth + _matteWidth),  // max. width of image + image matte
            dh = height() - 2 * (_borderWidth + _matteWidth);  // max. height of the same
        qreal magnx = ((qreal)dw) / _image.width(), // magnification in x direction to leave a margin of _borderWidth
              magny = ((qreal)dh) / _image.height(),// same for y
              magn = (magnx >= magny) ? magny : magnx; // the smaller magnification will be used
        int w = _image.width() * magn, h = _image.height() * magn;
        int ypos = _imageAlignment == VAlign::Top ? (_borderWidth+_matteWidth) : 
                                                    (_imageAlignment == VAlign::Center ? (rect().height()-h)/2 :
                                                                                         (rect().height() - h - _borderWidth - _matteWidth));
        return QRect((rect().width() - w)/2, ypos, w, h);
    }
    else
    {
        QRect mRect = MatteRect();
        return mRect.adjusted(
            _matteWidth,
            _matteWidth,
            -_matteWidth,
            -_matteWidth
        );
    }
}

QRect SlideWidget::WatermarkRect() const
{
    QRect imgRect = ImageRect();

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

    if (_hAlign == HAlign::Center)
        x = imgRect.center().x() - size.width() / 2;
    else if (_hAlign == HAlign::Right)
        x = imgRect.right() - size.width();

    if (_vAlign == VAlign::Center)
        y = imgRect.center().y() - size.height() / 2;
    else if (_vAlign == VAlign::Bottom)
        y = imgRect.bottom() - size.height();

    return QRect(QPoint(x, y), size);
}

void SlideWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    qreal radius = EffectiveRadius(false);

    // matte background                      // matte is the whole background and includes the image border
    radius = EffectiveRadius(true);
    QPainterPath mattePath; 
    mattePath.addRoundedRect(rect(), radius, radius);
    p.fillPath(mattePath, _matteColor);
    // picture frame
	QPen pen(_borderColor, _borderWidth, _borderStyle);
	p.drawRoundedRect(BorderRect(), radius, radius, Qt::RelativeSize);

    QPainterPath framePath;
    framePath.addRoundedRect(MatteRect(), radius, radius);
    // Clip Image to Matte rounded rect
    p.setClipPath(mattePath);

    if (!_image.isNull())
        p.drawImage(ImageRect(), _image);

    // icons
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
    // Watermark
    if ((!_watermarkIsText && !_watermarkImage.isNull())
        || (_watermarkIsText && !_watermarkText.isEmpty()))
    {
        p.setOpacity(_watermarkOpacity);
        QRect wRect = WatermarkRect();

        if (_watermarkIsText) {
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
}

SlideWidget::SlideArea
SlideWidget::HitTest(const QPoint& pos) const
{
    if (WatermarkRect().contains(pos))
        return SlideArea::Watermark;

    if (ImageRect().contains(pos))
        return SlideArea::Image;

    if (MatteRect().contains(pos))
        return SlideArea::Matte;

    if (BorderRect().contains(pos))
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
        QHoverEvent he(QEvent::HoverMove, e->pos(), e->pos());
        _hoverCb(HitTest(e->pos()), &he);
    }
}

void SlideWidget::enterEvent(QEvent*) {}
void SlideWidget::leaveEvent(QEvent*) {}
void SlideWidget::resizeEvent(QResizeEvent*) {}

void SlideWidget::SlotSetImage(QImage* pImage, VAlign alignment, bool keepAspectRatio)
{
    _image=*pImage;
    _imageAlignment = alignment;
    _keepAspectRatio = keepAspectRatio;
}
void SlideWidget::SlotSetImage(QPixmap* pPixmap, VAlign alignment, bool keepAspectRatio)
{
    _image= pPixmap->toImage();
    _imageAlignment = alignment;
    _keepAspectRatio = keepAspectRatio;
}
void SlideWidget::SlotSetBorder(int width, QColor color, qreal radius, RadiusUnit unit, Qt::PenStyle borderStyle)
{
    _borderWidth = width;
    _borderColor = color;
    _borderRadiusValue = radius;
    _borderRadiusUnit = unit;
    _borderStyle = borderStyle;
}
void SlideWidget::SlotSetMatte(int width, QColor color, qreal radius, RadiusUnit unit)
{
    _matteWidth = width;
    _matteColor = color;
    _matteRadiusValue = radius;
    _matteRadiusUnit = unit;
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
