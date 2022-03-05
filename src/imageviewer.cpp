#include <QColorSpace>
#include <QGuiApplication>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QContextMenuEvent>

#include "imageviewer.h"
#include "schemes.h"

constexpr double SCALE_UP = 1.1,
                 SCALE_DOWN = 0.909091,
                 SCALE_FULL = 1.0,
                 SCALE_FIT  = -1.0,
                 NO_SCALE = 0.0;

ImageViewer::ImageViewer(QString fileName, QWidget* parent) : _fileName(fileName), QWidget(parent)
{
    ui.setupUi(this);
    sizePolicy().setHeightForWidth(true);
    _screenSize = QGuiApplication::primaryScreen()->availableSize();
    _requiredWidgetSize = _screenSize * 3 / 4;
    _titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    _CreateActions();
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &ImageViewer::_PrimaryScreenChanged);
}

bool ImageViewer::LoadFile()
{
    QImageReader reader(_fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot load %1: %2"
                "<br>It is possible, that the image added was<br>"
                "in the data base already but moved to a different folder outside<br/>"
                "this program. Remove it and try to add with the <b>Keep duplicates</b><br>"
                "box checked on the <i>Gallery</i> page.")
            .arg(QDir::toNativeSeparators(_fileName), reader.errorString()));
        return false;
    }
    _aspect = (double)newImage.width() / (double)newImage.height();
    _SetMaxWidgetSizeForImageAspectRatio();

    double scaleTo = SCALE_FIT;
    // shrink widget for image if it was too large
    if (_requiredWidgetSize.width() > newImage.width() && _screenSize.width() > newImage.width() &&
        _requiredWidgetSize.height() > newImage.height() && _screenSize.height() > newImage.height())
    {
        _requiredWidgetSize = newImage.size();
        scaleTo = SCALE_FULL;
    }
    _SetImage(newImage);
    _CalcScaleFactor(scaleTo, imReshowImage | imResizeWithImage);  

    _ToggleStatus();    // hide info overlay as 
        // this text may change with widget size
    QString qsel = _ElideFileName(ui.lblIName->font(), width() - 4 * (IMAGE_MARGIN + IMAGE_BORDER));
    ui.lblIName->setText(qsel);

    _SetElidedInfoText();
        // original image size this text will not change
    ui.lblSize->setText(QString("%1 x %2").arg(_image.width()).arg(_image.height()));

    _ShowStatusInfo();
    return true;
}

int ImageViewer::heightForWidth(int w)
{
    if(_windowScalesWithImage)
        return w / _aspect;

    return -1;  // no dependence
}

static  ImageViewer::ImageFlags  __GetFlags(ImageViewer::ImageFlags flgBase, bool doFit)
{
    if (doFit)
        flgBase.setFlag(ImageViewer::imResizeWithImage);
    return flgBase;
};

void ImageViewer::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else 
    {

        if (event->key() == Qt::Key_0)                          //scale image to fit in window, may resize window
        {
            _windowScalesWithImage = true;                    // automatic scale with window
            _topLeftOfVisibleImage = { 0,0 };
            _needsFullScreen = false;
            _CalcScaleFactor(SCALE_FIT, __GetFlags(imReshowImage, true));
        }
        else if (event->key() == Qt::Key_1)                     // 100%
        {
            _windowScalesWithImage = false;                    // automatic fit to window
            _CalcScaleFactor(SCALE_FULL, __GetFlags(imReshowImage, false));          // do not resize window
        }
        else if (event->key() == Qt::Key_Plus)                  // zoom in
            _CalcScaleFactor(SCALE_UP, __GetFlags(imReshowImage, !_windowScalesWithImage));
        else if (event->key() == Qt::Key_Minus)                 // zoom out
            _CalcScaleFactor(SCALE_DOWN, __GetFlags(imReshowImage, !_windowScalesWithImage)); // 1 /1.10
        else if (event->key() == Qt::Key_F)                     // fit to window
            _FitImageToWindow();                                     // v- full screen
        else if (event->key() == Qt::Key_T)
            _ToggleTitleBar();
        else if (event->key() == Qt::Key_F11 || (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::EnterKeyReturn))
            _SetFullScreen(!_isFullScreen);
        else if (event->key() == Qt::Key_Question)              // help
            _Help();
        else if (event->key() == Qt::Key_I)                     // toggle info panel
            _ToggleStatus();
        else if (_IsImageMoveable())                            // move image
        {
            QPoint dp;
            if (event->key() == Qt::Key_Left)
                dp.setX(-10);
            else if (event->key() == Qt::Key_Right)
                dp.setX(10);
            else if (event->key() == Qt::Key_Up)
                dp.setY( - 10);
            else if (event->key() == Qt::Key_Down)
                dp.setY(10);
            _MoveImage(dp);
        }
    }
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _IsImageMoveable())
    {
        _mouseLeftButtonPressed = true;
        _mousePos = event->pos();
    }
        // /DEBUG
    //qDebug("mousePressEvent: _isImageFitToWindow:%d, _mouseLeftButtonPressed:%d", _isImageFitToWindow, _mouseLeftButtonPressed);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        _mouseLeftButtonPressed = false;
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (_mouseLeftButtonPressed)
    {
        QPoint dp = -event->pos() + _mousePos;
        _MoveImage(dp);
        _mousePos = event->pos();
    }
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    double factor = SCALE_FIT;
    if (!numPixels.isNull()) 
    {
        double dy = numPixels.y()/100.0;
        if (dy > 0)
            factor = 1.0 + dy;
        else
            factor = 1.0 / qAbs(dy);

    }
    else if (!numDegrees.isNull()) 
    {
        QPoint numSteps = numDegrees / 15;
        double dy = numSteps.y() / 24.0;
        factor = 1.0 + dy;
    }
    _CalcScaleFactor(factor, __GetFlags(imReshowImage, _windowScalesWithImage));

    event->accept();

}

void ImageViewer::contextMenuEvent(QContextMenuEvent* pevent)
{
    _popupMenu->exec(pevent->globalPos());
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    if (!_isFullScreen && (_windowScalesWithImage || size() != _requiredWidgetSize) )
    {
        _requiredWidgetSize = size();
        _CalcScaleFactor(_windowScalesWithImage ? SCALE_FIT : NO_SCALE, imReshowImage);
    }
    int lw = 2 * (IMAGE_MARGIN + IMAGE_BORDER);
    int w = width() - 2 * lw;
    ui.statusFrame->setGeometry(lw, lw, w, height()-2*lw);
    w -= 500;
    if (w < 0)
        w = 150;
    QString qs = _ElideFileName(ui.lblIName->font(), w);
    ui.lblIName->setText(qs);

    _SetElidedInfoText();
    if (!_infoOverlayOn)
        _ShowStatusInfo();
}

void ImageViewer::paintEvent(QPaintEvent* event)
{
    int which = PROGRAM_CONFIG::schemeIndex;
            // draw border
    QPen pen;
    pen.setColor(QColor(schemes[which].sBorderColor));
    pen.setWidth(IMAGE_BORDER);
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    QPainter painter(this);
    painter.setPen(pen);

    int x0 = IMAGE_MARGIN, x1 = width() - IMAGE_MARGIN,
        y0 = IMAGE_MARGIN, y1 = height() - IMAGE_MARGIN;
    //int iw = ->width()          -2*lw,
    //    ih =  ->height()         -2*iw;
    painter.drawLine(x0, y0, x1, y0);
    painter.drawLine(x1, y0, x1, y1);
    painter.drawLine(x0, y1, x1, y1);
    painter.drawLine(x0, y1, x0, y0);
            // draw image
    x0 += IMAGE_BORDER;
    y0 += IMAGE_BORDER;
    x1 -= IMAGE_BORDER;
    y1 -= IMAGE_BORDER;

    QPoint spt = _topLeftOfVisibleImage * _scaleFactor; // coord in _scaledImage
    int ww = width()  - 2 * (IMAGE_BORDER + IMAGE_MARGIN), 
        wh = height() - 2 * (IMAGE_BORDER + IMAGE_MARGIN);

    painter.drawImage(x0, y0, _scaledImage, spt.x(), spt.y(), ww, wh);
}


bool ImageViewer::_IsImageMoveable()
{
    QSize is = _TransformedSizeToWidgetSize(_requiredWidgetSize);
    // DEBUG
    //bool moveable = !_windowScalesWithImage ||
    //    (
    //        !_isImageFitToWindow &&
    //        (width() < is.width() || height() < is.height())
    //        );
    //qDebug("_IsImageMoveable: _isImageFitToWindow:%d, _windowScalesWithImage: %d, transfw: %d x%d, moveable:%d", 
    //            _isImageFitToWindow, _windowScalesWithImage, is.width(), is.height(), moveable);
    // /DEBUG
    return !_windowScalesWithImage ||
        (
            !_isImageFitToWindow &&
            (width() < is.width() || height() < is.height())
        );
}

/*=============================================================
 * TASK:    get widget's client area size for scaled image
 *          (without window borders or title bar)
 * PARAMS:  reqSize - required image size
 * GLOBALS:
 * RETURNS:
 * REMARKS: the image will have a border and amrgin around it
 *------------------------------------------------------------*/
QSize ImageViewer::_TransformedSizeToWidgetSize(QSize reqSize)
{
    int ilh = reqSize.height() + 2 * IMAGE_MARGIN + IMAGE_BORDER,
        ilw = reqSize.width() + +2 * IMAGE_MARGIN + IMAGE_BORDER;
    return QSize(ilw,ilh);
}

/*=============================================================
 * TASK:    get image size for widget clent area 
 *          (without window borders or title bar)
 * PARAMS:  wsize: client area size
 * GLOBALS:
 * RETURNS:
 * REMARKS: image aspect ratio remains the same
 *------------------------------------------------------------*/
QSize ImageViewer::_WidgetSizeToImageSize(QSize wsize)
{
    int ilh = wsize.height() - 2 * IMAGE_MARGIN + IMAGE_BORDER,
        ilw = wsize.width()  - 2 * IMAGE_MARGIN + IMAGE_BORDER;
    double waspect = (double)ilw / (double)ilh;
    if (waspect > _aspect)   // set width to height
        ilw = ilh * _aspect;
    else
        ilh = ilw / _aspect;

    return QSize(ilw,ilh);
}

QSize ImageViewer::_CalcRequiredWidgetSize()
{
    double  sw = _image.width() * _scaleFactor,     // image size after scaling
            sh = _image.height() * _scaleFactor;
//    QSize widgetSize = QSize(sw, sh);
    QSize widgetSize = _TransformedSizeToWidgetSize(QSize(sw, sh));

    _needsFullScreen = //_windowScalesWithImage &&
                       (widgetSize.width() >= _screenSize.width() || widgetSize.height() >= _screenSize.height() );
    if (_needsFullScreen)
        widgetSize=_maxFullScreenSize;
    return widgetSize;
}

/*=============================================================
 * TASK:    shows \ hides title bar
 * PARAMS:
 * GLOBALS: _showTitleBar
 * RETURNS: none
 * REMARKS:if title bar is shown the no need for info bar
 *------------------------------------------------------------*/
void ImageViewer::_ToggleTitleBar()
{
    Qt::WindowFlags flg = windowFlags();

    _showTitleBar = !_showTitleBar;

    if (_showTitleBar)
    {
        _titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
        setWindowTitle(_infoOverlayOn ? "falconG - Image Viewer" : _elidedInfoText);
        flg |= Qt::FramelessWindowHint;
    }
    else
        flg &= ~Qt::FramelessWindowHint;


    setWindowFlags(flg);
    show();
}

QString ImageViewer::_ElideFileName(const QFont& font, int w)
{
    QFontMetrics fm(font);
    return fm.elidedText(_fileName, Qt::ElideLeft, w);
}

void ImageViewer::_CreateActions()
{
    _popupMenu = new QMenu(this);

    _zoomInAct = _popupMenu->addAction(tr("Zoom &In"), this, &ImageViewer::_ZoomIn);
    _zoomInAct->setShortcut(QKeySequence::ZoomIn);

    _zoomOutAct = _popupMenu->addAction(tr("Zoom &Out"), this, &ImageViewer::_ZoomOut);
    _zoomOutAct->setShortcut(QKeySequence::ZoomOut);

    _normalSizeAct = _popupMenu->addAction(tr("&100%"), this, &ImageViewer::_NormalSize);
    _normalSizeAct->setShortcut(tr("Ctrl+1"));

    _isImageFitToWindowAct = _popupMenu->addAction(tr("Fit to &Window"), this, &ImageViewer::_FitImageToWindow);
    _isImageFitToWindowAct->setShortcut(tr("Ctrl+0"));
    _isImageFitToWindowAct->setCheckable(true);
    _isImageFitToWindowAct->setChecked(true);

    _fullScreenAct = _popupMenu->addAction(tr("&Full screen"), this, &ImageViewer::_SetFullScreen);
    _fullScreenAct->setShortcut(tr("Ctrl+F"));
    _fullScreenAct->setCheckable(true);


    _popupMenu->addSeparator();
    _statusAct = _popupMenu->addAction(tr("&Image info"), this, &ImageViewer::_ToggleStatus);
    _statusAct->setShortcut(tr("I"));
    _statusAct->setCheckable(true);
    _statusAct->setChecked(true);

    _popupMenu->addAction(tr("Image Viewer &Help"), this, &ImageViewer::_Help);
     
    _popupMenu->addSeparator();

    _closeAct = _popupMenu->addAction(tr("C&lose"), this, &QWidget::close);
    _closeAct->setShortcut(tr("Ctrl+Q"));

}

void ImageViewer::_ZoomIn()
{
    _CalcScaleFactor(SCALE_UP, imReshowImage | imResizeWithImage);
}

void ImageViewer::_ZoomOut()
{
    _CalcScaleFactor(SCALE_DOWN, imReshowImage | imResizeWithImage);
}

void ImageViewer::_NormalSize()
{
    _CalcScaleFactor(SCALE_FULL, imReshowImage | imResizeWithImage);
}

void ImageViewer::_FitImageToWindow()
{
    if (_isFullScreen)
        _SetFullScreen(false);
    if(!_isFullScreen)  //else fit to window has no effect
        _windowScalesWithImage = !_windowScalesWithImage;

    bool b = _windowScalesWithImage && !_isFullScreen;
    _isImageFitToWindowAct->setChecked(b);
    if (_windowScalesWithImage)
        _topLeftOfVisibleImage = { 0,0 };
    _CalcScaleFactor(b ? SCALE_FIT : NO_SCALE, imReshowImage | imResizeWithImage);
}

/*=============================================================
 * TASK:    Toggles full screen status
 * PARAMS:  setIt - set or reset it
 *          reqSize - is the required widget size for a given image
 *              may be (0,0)
 * GLOBALS: _isFullScreen, _lastNotFullScreenRect
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool ImageViewer::_SetFullScreen(bool setIt)
{
    Qt::WindowFlags flg = windowFlags();

    // DEBUG
    // bool isf = isFullScreen();
    bool result = false;
    if (!setIt && _isFullScreen)    // then reset to window
	{
        flg &= ~Qt::FramelessWindowHint;
        setWindowFlags(flg);   // when changed Qt hides window
        _showTitleBar = true;
		_fullScreenAct->setChecked(false);
		_isFullScreen = false;
        setGeometry(_lastNotFullScreenRect);
        show();                
        _titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
        setWindowTitle(_infoOverlayOn ? "falconG - Image Viewer" : _elidedInfoText);
		result = true;
	}
	else if (setIt & !_isFullScreen)    // set window to full screen (hide top bar)
	{
        _showTitleBar = false;
		_fullScreenAct->setChecked(true);
		_isFullScreen = true;
        _lastNotFullScreenRect = geometry();
        int w = _screenSize.width() - _maxFullScreenSize.width();
        if (w < 0)
            w = 0;
        flg |= Qt::FramelessWindowHint;
        setWindowFlags(flg);   // when changed Qt hides window
                               // must come before setGeometry
        show();                // when flags changed Qt hides window
        setGeometry( w /2,0,_maxFullScreenSize.width(),_maxFullScreenSize.height());
        result = true;
	}
	return result;
}

/*=============================================================
 * TASK:    determine the scale factor which will make w
 *          the image fit into the image frame of the widget
 * PARAMS:  wsize: widget size (client area)
 * GLOBALS:
 * RETURNS:
 * REMARKS: uses the layout margins 
 *------------------------------------------------------------*/
double ImageViewer::_GetFitFactor(QSize wsize)
{
    QSize siz = _WidgetSizeToImageSize(wsize);

    double  w = _image.width(),     // width of original image
            h = _image.height(),
        hf = siz.width() / w,      // horizontal factor
        vf = siz.height() / h;
        // the image scale factor which will make the image fit the widget
    return hf * h > siz.height() ? vf : hf;
}

/*=============================================================
 * TASK:    show scaled image in window
 * PARAMS:  factor - scale _image with this factor. Special cases:
 *                  NO_SCALE: just redisplay with current parameters
 *                  SCALE_FIT: fit in window
 *          flags: ORED value of image flags:
                    imReshowImage: showImage: display image even when no size changes
 *                  imResizeWithImage : should resize window when image size changes
 * GLOBALS: _scaleFactor
 *          popup actions
 *          _requiredWidgetSize
 * RETURNS:
 * REMARKS: - if the window scales with the image the image always
 *              fills the window
 *          -  _isImageFitToWindow is cleared for SCALE_FULL (user button '1')
 *          -  _isImageFitToWindow is set for SCALE_FIT image (user button '0')
 *------------------------------------------------------------*/
void ImageViewer::_CalcScaleFactor(double factor, QFlags<ImageFlag> flags)
{
    bool showImage = flags.testFlag(imReshowImage),
         doResize = flags.testFlag(imResizeWithImage);
    double fitfact = _GetFitFactor( _requiredWidgetSize );  // with this image will fit into the actual widget

    if (factor < 0)
        _scaleFactor = fitfact; // from actual widget size
    else if (factor == SCALE_FULL)
        _scaleFactor = SCALE_FULL;
    else if (factor != NO_SCALE)
        _scaleFactor *= factor;

    if (_scaleFactor > 3.0)
        _scaleFactor = 3.0;
    else if (_scaleFactor < 0.1)
        _scaleFactor = 0.1;

    QSize reqSize = _CalcRequiredWidgetSize();    // widget size for _scaleFactor, sets _needFullScreen

    if(doResize)
        _SetFullScreen(_needsFullScreen);       // does nothing when no change

    _zoomInAct->setEnabled(_scaleFactor < 3.0);
    _zoomOutAct->setEnabled(_scaleFactor > 0.1);

    _isImageFitToWindow = _scaleFactor == fitfact;
    _isImageFitToWindowAct->setChecked(_isImageFitToWindow );

    double  w = _image.width(),     // width of original image
            h = _image.height();

    int sw = w * _scaleFactor,
        sh = h * _scaleFactor;

    if (_scaleFactor != _prevScaleFactor)
    {
        _scaledImage = _image.scaled(sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        _prevScaleFactor = _scaleFactor;
    }

    if (doResize)
    {
        _requiredWidgetSize = _TransformedSizeToWidgetSize(QSize(sw, sh));
        if (_requiredWidgetSize.width() > _screenSize.width())
            _requiredWidgetSize.setWidth(_screenSize.width());
        if (_requiredWidgetSize.height() > _screenSize.height())
            _requiredWidgetSize.setHeight(_screenSize.height());

        ui.lblZoom->setText(QString("%1% (%2 x %3)").arg(_scaleFactor * 100.0, 10, 'f', 0)
                                    .arg(_scaledImage.width()).arg(_scaledImage.height()));
        _SetElidedInfoText();
        if (!_isFullScreen && (_windowScalesWithImage || factor == SCALE_FIT) && size() != _requiredWidgetSize)
        {
            resize(_requiredWidgetSize);    // automatic paint event after resize
        }
        else
            update();
    }
    else if (showImage)
    {
        if (_scaledImage.width() - _topLeftOfVisibleImage.x() < width() - 2 * (IMAGE_MARGIN + IMAGE_BORDER))
            _topLeftOfVisibleImage.setX(_scaledImage.width() - width() + 2 * (IMAGE_MARGIN + IMAGE_BORDER));
        if (_topLeftOfVisibleImage.x() < 0)
            _topLeftOfVisibleImage.setX(0);
        if (_scaledImage.height() - _topLeftOfVisibleImage.x() < height() - 2 * (IMAGE_MARGIN + IMAGE_BORDER))
            _topLeftOfVisibleImage.setY(_scaledImage.height() - height() + 2 * (IMAGE_MARGIN + IMAGE_BORDER));
        if (_topLeftOfVisibleImage.x() < 0)
            _topLeftOfVisibleImage.setY(0);

        ui.lblZoom->setText(QString("%1%").arg(_scaleFactor * 100.0, 10, 'f', 0));
        update();
    }

}

void ImageViewer::_SetImage(const QImage& newImage)
{
    _image = newImage;
    try                         // for some images where the colorSpace() is 0
                                // Qt's convertToColorSpace generate out of range ASSERT
    {                           // even with valid original color space
        if (_image.colorSpace().isValid() && _image.colorSpace().operator QVariant().toInt() != 0 && _image.colorSpace() != QColorSpace::SRgb)
            _image.convertToColorSpace(QColorSpace::SRgb);

    }
    catch (...)
    {
        ;
    }
//    ->setPixmap(QPixmap::fromImage(_image));
// already set on load and does not change    _aspect = double(_image.width()) / double(_image.height());
}

void ImageViewer::_Help()
{
    QMessageBox::about(this, tr("falconG - Image Viewer Help"),
        tr("<p><b>Keyboard and Mouse for the image viewer</b></p>"
           "<p><b>?</b> - show this help</p>"
           "<p><b>I</b> - toggle status display</p>"
           "<p><b>1</b> - zoom to 100% (enforce no scale window to image)</p>"
           "<p><b>0</b> - zoom to fit window</p>"
           "<p><b>+</b> - zoom in</p>"
           "<p><b>-</b> - zoom out</p>"
           "<p><b>F11</b> - Full screen toggle</p>"
           "<p><b>F</b> - toggle fit window to image</p>"
           "<p><b>arrow keys</b> - pan the image</p>"
           "<p><b>Mouse</b> - Use the wheel to zoom in or out<br>"
            "Press and hold the <b>left</b> mouse button while<br>"
            "moving the mouse to pan the image</p>"
            "<p><b>Right click</b>: popup menu</p>"
        ));
}

void ImageViewer::_ShowStatusInfo()
{
    if (_infoOverlayOn)
    {
        ui.statusFrame->show();
        _statusAct->setChecked(true);
        setWindowTitle("falconG - Image Viewer");
    }
    else
    {
        ui.statusFrame->hide();
        _statusAct->setChecked(false);
        setWindowTitle(_elidedInfoText);
    }
}

void ImageViewer::_ToggleStatus()
{
    _infoOverlayOn = !_infoOverlayOn;
    _ShowStatusInfo();
}

void ImageViewer::_MoveImage(QPoint dp)
{
    _topLeftOfVisibleImage += dp;

    //int w = width() - 2 * (IMAGE_MARGIN + IMAGE_BORDER),
    //    h = height() - 2 * (IMAGE_MARGIN + IMAGE_BORDER);
    if (_topLeftOfVisibleImage.x() < 0)
        _topLeftOfVisibleImage.setX(0);
    //else if (_scaledImage.width() - _topLeftOfVisibleImage.x() < w)
    //    _topLeftOfVisibleImage.setX(w - _scaledImage.width());
    if (_topLeftOfVisibleImage.y() < 0)
        _topLeftOfVisibleImage.setY(0);
    //else if (_scaledImage.height() - _topLeftOfVisibleImage.y() < h)
    //    _topLeftOfVisibleImage.setY(h - _scaledImage.width());

    update();

}

// This size is the maximum widget size that still contains the whole image.
// when full screen mode requested. If before reaching full screen mode the
// size of the image is smaller than or equal to this size then the image is enlarged
// to fill this widget. However if the image size is already larger than this size
// then the image is not resized
// so that it still fits into the screen (without a title bar) In full screen mode if the scaled image
// zooming in such an image may increase this size to encompass the whole
// screen
// height is considerd first and width is determined based on image aspect ratio
// however this may give an image partially outsize of the screen
void ImageViewer::_SetMaxWidgetSizeForImageAspectRatio()
{
    Q_ASSERT(_aspect != 0.0);

    double asp = _aspect < 1.0 ? _aspect : 1 / _aspect,
          sasp = _screenSize.width() / _screenSize.height();

    _maxFullScreenSize = _screenSize;
    if (qAbs(sasp - asp) > 1.301e-4)   //1 pixel difference on 8K (size: 7680 x 4320) screen is 1.302e-4
        _maxFullScreenSize.setWidth(_screenSize.height() * _aspect);

    // get maximum size a widget may occupy on the screen
    if (_maxFullScreenSize.width() > _screenSize.width())
        _maxFullScreenSize.setWidth(_screenSize.width());
}

void ImageViewer::_PrimaryScreenChanged(QScreen* pScreen)
{
    _screenSize = QGuiApplication::primaryScreen()->availableSize();
    _titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    _requiredWidgetSize = _screenSize * 3 / 4;
    sizePolicy().setHeightForWidth(true);
    _SetMaxWidgetSizeForImageAspectRatio();
}

void ImageViewer::_SetElidedInfoText()
{
    QString qsel = _ElideFileName(ui.lblIName->font(), width() - 4 * (IMAGE_MARGIN + IMAGE_BORDER));

    _elidedInfoText = QString(" %1,    Size:%2 x %3,    Zoom: %4% (%5 x %6)   - '?'= Help").arg(qsel)
        .arg(_image.width()).arg(_image.height()).arg(_scaleFactor * 100.0, 10, 'f', 0)
        .arg(_scaledImage.width()).arg(_scaledImage.height());

}