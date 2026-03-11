// SlideWidget.h
#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QHoverEvent>
#include <functional>

class SlideWidget : public QWidget
{
    Q_OBJECT
		Q_PROPERTY(QImage image READ Image WRITE SetImage)
        Q_PROPERTY(int borderWidth READ BorderWidth WRITE SetBorderWidth)
        Q_PROPERTY(QColor borderColor READ BorderColor WRITE SetBorderColor)
        Q_PROPERTY(qreal borderRadius READ BorderRadius WRITE SetBorderRadius)
        Q_PROPERTY(int matteWidth READ MatteWidth WRITE SetMatteWidth)
        Q_PROPERTY(QColor matteColor READ MatteColor WRITE SetMatteColor)
        Q_PROPERTY(qreal matteRadius READ MatteRadius WRITE SetMatteRadius)

		Q_PROPERTY(QString watermarkText READ WatermarkText WRITE SetWatermarkText)
		Q_PROPERTY(QImage watermarkImage READ WatermarkImage WRITE SetWatermarkImage)
		Q_PROPERTY(QFont watermarkFont READ WatermarkFont WRITE SetWatermarkFont)
		Q_PROPERTY(QColor watermarkColor READ WatermarkColor WRITE SetWatermarkColor)
		Q_PROPERTY(qreal watermarkOpacity READ WatermarkOpacity WRITE SetWatermarkOpacity)
		Q_PROPERTY(HAlign watermarkHAlignment READ WatermarkHAlignment WRITE SetWatermarkHAlignment)
		Q_PROPERTY(VAlign watermarkVAlignment READ WatermarkVAlignment WRITE SetWatermarkVAlignment)

public:
    enum class SlideArea {
        Border,
        Matte,
        Image,
        Watermark
    };

    enum class RadiusUnit {
        Pixels,
        Percent
    };

    enum class HAlign { None, Left, Center, Right };
    enum class VAlign { None, Top, Center, Bottom };

    explicit SlideWidget(QWidget* parent = nullptr);

    // Image
	QImage Image() const { return _image; }
    void SetImage(const QImage& img);
    void SetImage(const QPixmap& pix);

    void SetImageAlignment(VAlign imageAlignment) { _imageAlignment = imageAlignment; }

    // Border
    void SetBorderWidth(int w);
    int BorderWidth() const { return _borderWidth; }

    void SetBorderColor(const QColor& c);
    QColor BorderColor() const { return _borderColor; }
    void SetBorderRadius(qreal value, RadiusUnit unit = RadiusUnit::Pixels);
    int BorderRadius() const { return _borderRadiusValue; }

    // Matte
    void SetMatteWidth(int w);
    int MatteWidth() const { return _matteWidth; }
    void SetMatteRadius(qreal value, RadiusUnit unit = RadiusUnit::Pixels);
    int MatteRadius() const { return _matteRadiusValue; }

    void SetMatteColor(const QColor& c);
    QColor MatteColor() const { return _matteColor; }

    // Watermark
    QString WatermarkText() const { return _watermarkText; }
    void SetWatermarkText(const QString& text);
    QImage WatermarkImage() const { return _watermarkImage; }
    void SetWatermarkImage(const QImage& img);
    void ClearWatermark();

    QFont WatermarkFont() const { return _watermarkFont; }
    void SetWatermarkFont(const QFont& font);
	QColor WatermarkColor() const { return _watermarkColor; }
    void SetWatermarkColor(const QColor& color);
	qreal WatermarkOpacity() const { return _watermarkOpacity; }
    void SetWatermarkOpacity(qreal opacity);
    void SetWatermarkAlignment(HAlign h, VAlign v);
	HAlign WatermarkHAlignment()const { return _hAlign; }
    void SetWatermarkHAlignment(HAlign h) { _hAlign = h; }
	VAlign WatermarkVAlignment() const { return _vAlign; }
    void SetWatermarkVAlignment(VAlign v) { _vAlign = v; }

    // Event callbacks
    using MouseCallback =
        std::function<void(SlideArea, QMouseEvent*)>;

    using HoverCallback =
        std::function<void(SlideArea, QHoverEvent*)>;

    void SetMousePressCallback(MouseCallback cb);
    void SetMouseReleaseCallback(MouseCallback cb);
    void SetMouseClickCallback(MouseCallback cb);
    void SetMouseDoubleClickCallback(MouseCallback cb);
    void SetHoverCallback(HoverCallback cb);

public slots:
    void SlotSetImage(QImage* pImage, VAlign alignment, bool keepAspectRatio = true);
    void SlotSetImage(QPixmap* pPixmap, VAlign alignment, bool keepAspectRatio = true);
    void SlotSetBorder(int width, QColor color, qreal radius, RadiusUnit unit = RadiusUnit::Pixels);
    void SlotSetMatte(int width, QColor color, qreal radius, RadiusUnit unit = RadiusUnit::Pixels);
    void SlotAddIcons(QVector<QImage> icons, HAlign hAlign, VAlign vAlign);
    void SlotSetWatermarkText(const QString &text);
    void SlotSetWatermarkImage(const QImage &image);
    void SlotSetWatermarkFont(const QFont& font);
    void SlotSetWatermarkOpacity(qreal opacity);
    void SlotSetWatermarkAlignment(HAlign ha, VAlign va);
    void SlotSetMousePressCallback(const MouseCallback cb);
    void SlotSetMouseReleaseCallback(const MouseCallback cb);
    void SlotSetMouseClickCallback(const MouseCallback cb);
    void SlotSetMouseDoubleClickCallback(const MouseCallback cb);
    void SlotSetHoverCallback(const HoverCallback cb);


protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    QRect BorderRect() const;
    QRect MatteRect() const;
    QRect ImageRect() const;
    QRect WatermarkRect() const;

    SlideArea HitTest(const QPoint& pos) const;

	qreal EffectiveRadius(bool matte) const;    // else border radius

private:
    struct Marks    // icons over the widget
    {
        QVector<QImage> imgs;    // icon images displayed in this order from left to right
                                 // position of first depends on hAlign and vAlign
		HAlign hAlign; // horizontal alignment
		VAlign vAlign; // vertical alignment
    };

    QImage _image;
    VAlign _imageAlignment = VAlign::Center;
	QVector<Marks> _marks; // for thumbnails. Example: a play icon for video thumbnails.

    bool _keepAspectRatio = true;

    int _borderWidth = 1;
    QColor _borderColor = "#FFAA00";
    RadiusUnit _borderRadiusUnit = RadiusUnit::Pixels;
    qreal       _borderRadiusValue = 0;

    int _matteWidth = 1;
    QColor _matteColor = Qt::black;
    RadiusUnit _matteRadiusUnit = RadiusUnit::Pixels;
    qreal       _matteRadiusValue = 0;


    // Watermark
    bool _watermarkIsText = true;
    QString _watermarkText;
    QImage _watermarkImage;
    QFont _watermarkFont;
    QColor _watermarkColor = Qt::white;
    qreal _watermarkOpacity = 0.5;
    HAlign _hAlign = HAlign::Right;
    VAlign _vAlign = VAlign::Bottom;

    // Mouse state
    QPoint _lastPressPos;

    // Callbacks
    MouseCallback _mousePressCb = nullptr;
    MouseCallback _mouseReleaseCb = nullptr;
    MouseCallback _mouseClickCb = nullptr;
    MouseCallback _mouseDoubleClickCb = nullptr;
    HoverCallback _hoverCb = nullptr;
};