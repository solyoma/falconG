// slideWidget.h
#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QString>
#include <functional>

/*
* Image slide                                      QT style extension handled in 'SlotSetupFromStyleString' (present in CSS, but not in Qt 5.15.2):
*                                                     each option might have 1 to 4 parameters separated by spaces. 4 params: 4 separate values 
*     ┌──────────────────────────────────────┐           1 param: all 4 (top,right,bottom,left)   border: all borders
*     │                margin                │           2 params: top&bottom right&left                  <topleft & bottomright> <toprigh & bottomleft>
*     │   ┌───────────────────────────────┐  │           3 params: top right&left bottom                  <topleft> <topright & bottomleft> <bottom-right>
*     │   │            border             │  │
*     │   │   ┌───────────────────────┐   │  │     blocks have outer radius, so area with image has one, matte (padding) has one and border has one   
*     │   │   │     padding(matte)    │   │  │       
*     │   │   │  ┌─────────────────┐  │   │  │       image: may have a clipping mask with rounded corners
*     │   │   │  │                 │  │   │  │       matte: "padding: w1 [w2[ w3[ w4]]]" - width of visible matte 1 to 4 values + matte color + matte radius      
*     │   │   │  │   image         │  │   │  │              Drawn as a (rounded) and filled rectangle before the image is drawn but after the border is drawn.
*     │   │   │  │     w,h         │  │   │  │       "border: [width] <style> [color]"   - no separate sides
*     │   │   │  │                 │  │   │  │          "border-style: <style>", where    <style> = dotted, dashed,solid,double,groove,ridge,inset,outset,none,hidden
*     │   │   │  └─────────────────┘  │   │  │            separate to border-top-style, border-right-style, border-bottom-style,border-left-style
*     │   │   │                       │   │  │          "border-width: w1 [w2 [w3 [w4]]]"
*     │   │   └───────────────────────┘   │  │          "border-color: c1 [c2 [c3 [c4]]]" -  max 4 values each of them can be  
*     │   │    see settings below         │  │              name (e.g.red), hex ($abcdef), rgb(r,g,b) or hsl(h,s%,l%) or transparent
*     │   └───────────────────────────────┘  │          "border-radius: r1 [r2 [r3 [r4]]]" - rX can be in pixel "15px" or percent "15%"
*     │                                      │  
*     └──────────────────────────────────────┘    uses the CSS box model (default is "box-sizing:border-box", as in normal CSS
*                                                       "box-sizing:border-box"  (full size: widget size, image size adjusted to: widget size - border - padding/matte)
*                                                       "box-sizing:content-box:" (widget size adjusted to: image + padding/matte + border)
*
*                    Draw order: (rounded) total area with matte color, border with border pen and color, then image with clipping mask
* 
*/


class SlideWidget : public QWidget
{
    Q_OBJECT
		Q_PROPERTY(QString imagePath READ ImagePath WRITE SetImagePath)
		Q_PROPERTY(QImage image READ Image WRITE SetImage)
		Q_PROPERTY(qreal imageRadius READ ImageRadius WRITE SetImageRadius)
        Q_PROPERTY(int borderWidth READ BorderWidth WRITE SetBorderWidth)
        Q_PROPERTY(QColor borderColor READ BorderColor WRITE SetBorderColor)
        Q_PROPERTY(qreal borderRadius READ BorderRadius WRITE SetBorderRadius)
        Q_PROPERTY(RadiusUnit borderRadiusUnit READ BorderRadiusUnit WRITE SetBorderRadiusUnit)
        Q_PROPERTY(int matteWidth READ MatteWidth WRITE SetMatteWidth)
        Q_PROPERTY(QColor matteColor READ MatteColor WRITE SetMatteColor)
        Q_PROPERTY(qreal matteRadius READ MatteRadius WRITE SetMatteRadius)
        Q_PROPERTY(RadiusUnit matteRadiusUnit READ MatteRadiusUnit WRITE SetMatteRadiusUnit)

		Q_PROPERTY(QString watermarkText READ WatermarkText WRITE SetWatermarkText)
		Q_PROPERTY(QImage watermarkImage READ WatermarkImage WRITE SetWatermarkImage)
		Q_PROPERTY(QString watermarkImagePath READ WatermarkImagePath WRITE SetWatermarkImagePath)
		Q_PROPERTY(QFont watermarkFont READ WatermarkFont WRITE SetWatermarkFont)
		Q_PROPERTY(QColor watermarkColor READ WatermarkColor WRITE SetWatermarkColor)
		Q_PROPERTY(qreal watermarkOpacity READ WatermarkOpacity WRITE SetWatermarkOpacity)
		Q_PROPERTY(HAlign watermarkHAlignment READ WatermarkHAlignment WRITE SetWatermarkHAlignment)
		Q_PROPERTY(VAlign watermarkVAlignment READ WatermarkVAlignment WRITE SetWatermarkVAlignment)
//        Q_PROPERTY(bool keepAspectRatio READ KeepAspectRatio WRITE SetKeepAspectRatio)

public:
    enum class SlideArea {
        Border,
        Matte,
        Image,
        Watermark
    };

    enum class RadiusUnit {
        Unset,
        Pixels,
        Percent
    };

    enum class AspectRatio {
		Scale = 0,               // image is scaled to fit _imageSize w.o. cropping
		KeepAspectRatio = 1,    // keep image aspect ratio, but fit the image  into '_imageSize'
                                // may lead to different matte widths in x and y direction
		CropImage = 2           // the image is enlarged/shrinked so that the smaller side of it will fit 
                                // into _imageSize and it is cropped to fit the widget size
		                        // mutually exclusive with KeepAspectRatio
    };

    enum class SWBorderStyle {
		None            = Qt::NoPen,       // default (0)
		Solid           = Qt::SolidLine,
		Dashed          = Qt::DashLine,
		Dotted          = Qt::DotLine,
		DashDotLine     = Qt::DashDotLine,         // not in CSS, but in Qt
		DashDotDotLine  = Qt::DashDotDotLine,      // -"-
		CustomDashLine  = Qt::CustomDashLine,      // -"- and will never be drawn by SlideWidget
		Double          = 7,                       // from here: in CSS, but not in Qt : 2 lines 1/3rd of with separated by a 1/3rd of width space
		Groove,                                    // must be drawn in paintEvent      : 2 lines with different colors, left/top darker outside right/bottom lighter outside
		Ridge,                                     // lighter color is given           : 2 lines with different colors, left/top lighter outside right/bottom lighter inside
		Inset,                                     //								   : left/top line darker than right/bottom line
		Outset,                                    //								   : left/top line lighter than right/bottom line
		Hidden                                     // same as None, but also no space is reserved for the border= set width of border to 0 
	};

    using AspectFlags = QFlags<AspectRatio>;

	enum class BoxSizing {  
		BorderBox,      // default, the widget size is the total size of border + matte + image
		ContentBox      // the widget size is the size of the image, border and matte are added outside of it
	};

          // for watermarks
    enum class HAlign { None, Left, Center, Right };
    enum class VAlign { None, Top, Center, Bottom };
            // Side is not class so no need to cast it to int
    enum Side { sdAllSides=-1, sdTop=0, sdRight=1, sdBottom=2, sdLeft=3, sdTopLeft=sdTop, sdTopRight=sdRight, sdBottomLeft=sdBottom, sdBottomRight=sdLeft };

    Q_ENUM(SlideArea)
    Q_ENUM(RadiusUnit)
    Q_ENUM(AspectRatio)
    Q_ENUM(HAlign)
    Q_ENUM(VAlign)
    Q_ENUM(Side)

    using FourInts = int[4];
    using FourReals = qreal[4];

    // constructor
    explicit SlideWidget(QWidget* parent = nullptr);

    void Update();  // only when new parameters are set (_mustUpdate = true)

	// setters and getters
    void SetSizing(BoxSizing boxSizing);

	// ********************* Image **********************
	void SetAspectFlag(AspectRatio flag, bool on); // if on is true, clear other flags 

	QImage  Image() const { return _image; }

    void    SetImage(const QImage& img);
    void    SetImage(const QPixmap& pix);
    QString ImagePath() const { return _imagePath; }
    void    SetImagePath(const QString& path);

    static qreal GetRadius(QString qsRadiusWithUnit, RadiusUnit& radius);
    // how to display an image:
    void SetImageSize(QSize displaySize);
	QSize ImageSize(bool forDisplay) const { return forDisplay ? _imageSize : _image.size(); }
	qreal   ImageRadius(int side = -1) const { return _imageRadius[_RealSide(side)]; }
	void	SetImageRadius(qreal value, int side = -1, RadiusUnit unit = RadiusUnit::Unset);
	void	SetImageRadius(FourReals value, RadiusUnit unit = RadiusUnit::Unset);
    RadiusUnit ImageRadiusUnit(int side = -1) const { return _imageRadiusUnit[_RealSide(side)]; }
    void SetImageRadiusUnit(RadiusUnit unit, int side = -1);

	// ********************* Matte = padding + radius + color **********************
    void SetMatteWidth(int w); // same for for all matte (padding)
    void SetMatteWidth(FourInts w); // set padding on all 4 sides separately
    void SetMatteTop(int w)     { _matteWidth[sdTop] = w;   _mustUpdate = true; }
    void SetMatteRight(int w)   { _matteWidth[sdRight] = w; _mustUpdate = true; }
    void SetMatteBottom(int w)  { _matteWidth[sdBottom] = w;_mustUpdate = true; }
    void SetMatteLeft(int w)    { _matteWidth[sdLeft] = w;  _mustUpdate = true; }
    int MatteWidth(int side = -1) const { return _matteWidth[_RealSide(side)]; }
    void SetMatteRadius(qreal value, RadiusUnit unit = RadiusUnit::Pixels);
    qreal MatteRadius(int side = -1) const { return _matteRadiusValue[_RealSide(side)]; }
    RadiusUnit MatteRadiusUnit(int side = -1) const { return _matteRadiusUnit[side < 0 ? 0 : side]; }
    void SetMatteRadiusUnit(RadiusUnit unit);

    void SetMatteColor(const QColor& c);
    QColor MatteColor(int side=-1) const {   return _matteColor[_RealSide(side)]; }

	// ********************* Border **********************
    void SetBorderWidth(int w);
    int BorderWidth(int side=-1) const {if (side < 0 || side > 3) side = 0;   return _borderWidth[side]; }

    static SWBorderStyle BorderStyleFromString(const QString qsBorderStyle);

    void SetBorderStyle(SWBorderStyle ps);
    void SetBorderStyle(QString qs);
    SWBorderStyle BorderStyle(int side=-1) const { return side>=0 ? _borderStyle[_RealSide(side)] : _borderStyle[0]; }
    void SetBorderColor(const QColor& c);
    QColor BorderColor(int side=-1) const { return _borderColor[_RealSide(side)]; }
    void SetBorderRadius(qreal value, RadiusUnit unit = RadiusUnit::Pixels);
    void SetBorderRadius(QString qsRadiusString);
    qreal BorderRadius(int side=-1) const { return _borderRadiusValue[_RealSide(side)]; }
    RadiusUnit BorderRadiusUnit(int side=-1) const { return _borderRadiusUnit[_RealSide(side)]; }
    void SetBorderRadiusUnit(RadiusUnit unit);
    void SetMatteRadius(QString qsRadiusString);

    void SetAllBordersFromString(const QString qsBorder);     // format: "7px dotted #aabbcc" no separate sides!

	// ********************* Watermark **********************
    QString WatermarkText() const { return _watermarkText; }
    void SetWatermarkText(const QString& text);
    QImage WatermarkImage() const { return _watermarkImage; }
    void SetWatermarkImage(const QImage& img);
    void ClearWatermark();
    QString WatermarkImagePath() const { return _watermarkImagePath; }
    void SetWatermarkImagePath(const QString& path);

    QFont WatermarkFont() const { return _watermarkFont; }
    void SetWatermarkFont(const QFont& font);
 	QColor WatermarkColor() const { return _watermarkColor; }
    void SetWatermarkColor(const QColor& color);
 	qreal WatermarkOpacity() const { return _watermarkOpacity; }
    void SetWatermarkOpacity(qreal opacity);
    void SetWatermarkAlignment(HAlign h, VAlign v);
 	HAlign WatermarkHAlignment()const { return _hAlignWmark; }
    void SetWatermarkHAlignment(HAlign h) { _hAlignWmark  = h; }
 	VAlign WatermarkVAlignment() const { return _vAlignWmark; }
    void SetWatermarkVAlignment(VAlign v) { _vAlignWmark = v; }
    AspectRatio ImageAspectRatio() const;
    void SetImageAspectRatio(AspectRatio ar, bool on);

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
    void SlotSetBorder(int which, int width, QColor color, qreal radius, RadiusUnit unit = RadiusUnit::Pixels, SWBorderStyle borderStyle=SWBorderStyle::Solid);
    void SlotSetMatte(int which, int width, QColor color, qreal radius, RadiusUnit unit = RadiusUnit::Pixels);
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

    void SlotSetupFromStyleString(QString& styleStr);


protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif    
    void leaveEvent(QEvent*) override;

private:
    QRect _BorderRect();            // these are the geometries of the areas
    QRect _MatteRect();             // so before display they must be 
    QRect _ImageRect();             // adjusted with   left() and top()
    QRect _WatermarkRect();         // relative to the image area!

    SlideArea HitTest(const QPoint& pos) const;

private:


    struct Marks    // icons over the widget
    {
        QVector<QImage> imgs;    // icon images displayed in this order from left to right
                                 // position of first depends on hAlign and vAlign
		HAlign hAlign; // horizontal alignment
		VAlign vAlign; // vertical alignment
    };

private:
    bool _mustUpdate = true;   // after all settings are set, call Update()
	BoxSizing _boxSizing = BoxSizing::BorderBox;
    QFlags<AspectRatio> _imageSizeFlags;

    QRect _borderRect,          // required for display, may be larger than geometry()
                                // when box-sizing:content-box is used
          _matteRect,           // always relative to _borderRect
          _imageRect;           // equal to QRect(0,0,_imageSize()) when box-sizing:content-box is used
	QRect _watermarkRect;       // relative to image area, calculated based on _hAlignWmark and _vAlignWmark

    QString _imagePath;
    QImage _image;
    QSize _imageSize;   // uncropped size, set bymay be different from the real size of the image
    qreal _magn;        // magnification factor for the image to fill the image area
	FourReals _imageRadius = { 0.0 }; // radius of image corners, in pixels or percent depending on _imageRadiusUnit
    RadiusUnit _imageRadiusUnit[4] = { RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels};
	QVector<Marks> _marks; // for thumbnails. Example: a play icon for video thumbnails.


    int _matteWidth[4] = { 2,2,2,2 };         // width of area NOT including the border for top,right,bottom,left
    QColor _matteColor[4] = { Qt::black, Qt::black, Qt::black, Qt::black};
    RadiusUnit _matteRadiusUnit[4] = { RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels};
    FourReals _matteRadiusValue    = { 0.0 };

    int _borderWidth[4] = { 1,1,1,1 };                              // from the edge of the matte
    QColor _borderColor[4] = { "#FFAA00", "#FFAA00", "#FFAA00", "#FFAA00"};
    RadiusUnit _borderRadiusUnit[4] = {RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels, RadiusUnit::Pixels};
    FourReals _borderRadiusValue = { 0.0, 0.0, 0.0, 0.0, };
    SWBorderStyle _borderStyle[4] = { SWBorderStyle::None, SWBorderStyle::None, SWBorderStyle::None, SWBorderStyle::None};

    // Watermark
    bool _watermarkIsText = true;
	QString _watermarkImagePath;
    QString _watermarkText;
    QImage _watermarkImage;
    QFont _watermarkFont;
    QColor _watermarkColor = Qt::white;
    qreal _watermarkOpacity = 0.5;
    HAlign _hAlignWmark = HAlign::Right;
    VAlign _vAlignWmark = VAlign::Bottom;

    // Mouse state
    QPoint _lastPressPos;
private:
    QStringList _GetCssWidthStyleColor(const QString& str, 
                                                    QString& size, 
                                                    SWBorderStyle& penStyle, 
                                                    QColor& color);
private:

    constexpr int _RealSide(const int side) const { return (side < 0 || side > 3) ? 0 : side; }
    // CSS evaluation functions

	void _SetCssBoxSizing(QString boxSizing);          // box-sizing: border-box|content-box, no "initial" or "inherit"!

    void _SetCssFont(QString fnt);                      // font:[style] [variant] [weight] <size>["/"line-height] <family>
    void _SetCssFontSize(QString size);                 // font-size:15px      - no 'large' or '%' !
    void _SetCssFontFamily(QString family);             // font-family:Arial[,<next name>[,...]]
    void _SetCssFontStyle(QString style);               // font-style:normal|italic|oblique
    void _SetCssFontWeight(QString weight);             // font-style:normal|italic|oblique

    // matte = padding, except for radius, which doesn't exist
    void _SetCssPadding(QString padding);               // padding:p1 [p2 [p3 [p4]]] in "px"  - matte width
    void _SetCssPaddingTop(QString padding);            // padding-top:"px"  - matte width
    void _SetCssPaddingRight(QString padding);          // padding-"px"  - matte width
    void _SetCssPaddingBottom(QString padding);         // padding-"px"  - matte width
    void _SetCssPaddingLeft(QString padding);           // padding-"px"  - matte width
    
    void _SetCssMatteRadius(QString radius);    // no such css property! units:  "px" or "%" - matte radius

    void _SetCssBorder(QString border);                 // border:[width] <style> [color] - all border
    void _SetCssBorderColor(QString width);             // border-color:c1 [c2 [c3 [c4]]] 
    void _SetCssBorderRadius(QString width);            // border-radius:r1 [r2 [r3 [r4]]] 
    void _SetCssBorderStyle(QString style);             // border-style:w1 [w2 [w3 [w4]]] 
    void _SetCssBorderWidth(QString width);             // border-width:w1 [w2 [w3 [w4]]]

    void _SetCssBorderTop(QString border);              // border-top:[width] <style> [color] - in "px"
    void _SetCssBorderTopColor(QString border);         // border-top-color:<color>
    void _SetCssBorderTopLeftRadius(QString border);    // border-bottom-left-radius: length [length]; - px or %
    void _SetCssBorderTopRightRadius(QString border);   // border-bottom-left-radius: length [length]; - px or %
    void _SetCssBorderTopStyle(QString style);          // border-top-style:s1 
    void _SetCssBorderTopWidth(QString border);         // border-top-width:width>

    void _SetCssBorderRight(QString border);            // border-right:[width] <style> [color] - in "px"
    void _SetCssBorderRightColor(QString border);       // border-right-color:<color>
    void _SetCssBorderRightStyle(QString style);        // border-right-style:s1 
    void _SetCssBorderRightWidth(QString border);       // border-right-width:<width>

    void _SetCssBorderBottom(QString border);           // border-bottom:[width] <style> [color] - in "px"
    void _SetCssBorderBottomColor(QString border);      // border-bottom-color:<color>
    void _SetCssBorderBottomStyle(QString style);       // border-bottom-style:s1 
    void _SetCssBorderBottomWidth(QString border);      // border-bottom-width:<width>
    void _SetCssBorderBottomLeftRadius(QString border); // border-bottom-left-radius: length [length]; - px or %
    void _SetCssBorderBottomRightRadius(QString border);// border-bottom-left-radius: length [length]; - px or %

    void _SetCssBorderLeft(QString border);             // border-left:[width] <style> [color] - in "px"
    void _SetCssBorderLeftColor(QString border);        // border-left-color:<color>
    void _SetCssBorderLeftStyle(QString style);         // border-left-style:s1 
    void _SetCssBorderLeftWidth(QString border);        // border-left-width:<width>

    // Callbacks
    MouseCallback _mousePressCb = nullptr;
    MouseCallback _mouseReleaseCb = nullptr;
    MouseCallback _mouseClickCb = nullptr;
    MouseCallback _mouseDoubleClickCb = nullptr;
    HoverCallback _hoverCb = nullptr;
};
