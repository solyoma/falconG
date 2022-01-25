#include <QColorSpace>
#include <QGuiApplication>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QContextMenuEvent>

#include "imageviewer.h"
#include "schemes.h"

ImageViewer::ImageViewer(QString fileName, QWidget* parent) : _fileName(fileName), QWidget(parent)
{
    ui.setupUi(this);
    _screenSize = QGuiApplication::primaryScreen()->availableSize();
    _CreateActions();
    _requiredWidgetSize = _screenSize * 3 / 5;
    sizePolicy().setHeightForWidth(true);
    ui.statusFrame->setStyleSheet(QString("background-color:%1").arg(Qt::transparent));
}

bool ImageViewer::LoadFile()
{
    QImageReader reader(_fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot load %1: %2")
            .arg(QDir::toNativeSeparators(_fileName), reader.errorString()));
        return false;
    }
    _SetImage(newImage);
    _CalcScaleFactor(-1.0, true, true);
    ui.lblIName->setText(_fileName);
    ui.lblSize->setText(QString("%1 x %2").arg(_image.width()).arg(_image.height()));
    return true;
}

int ImageViewer::heightForWidth(int w)
{
    if(_windowScalesWithImage)
        return w / _aspect;

    return -1;  // no dependence
}


void ImageViewer::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else 
    {
        if (event->key() == Qt::Key_0)
            _CalcScaleFactor(-1.0, true, true);
        else if (event->key() == Qt::Key_1)
            _CalcScaleFactor(1.0, true, true);
        else if (event->key() == Qt::Key_Plus)
            _CalcScaleFactor(1.25, true, true);
        else if (event->key() == Qt::Key_Minus)
            _CalcScaleFactor(0.8, true, true);
        else if (event->key() == Qt::Key_F)
            _fitToWindow = !_fitToWindow;
        else if (event->key() == Qt::Key_F11 || (event->modifiers().testFlag(Qt::AltModifier) && event->key() == Qt::EnterKeyReturn))
            _SetFullScreen(!_isFullScreen);
        else if (event->key() == Qt::Key_Question)
            _Help();
        else if (event->key() == Qt::Key_I)
            _ToggleStatus();
        else if (_IsImageMoveable())
        {
            if (event->key() == Qt::Key_Left && _topLeftOfVisibleImage.x())
                _topLeftOfVisibleImage.setX(_topLeftOfVisibleImage.x() - 10);
            else if (event->key() == Qt::Key_Right && 
                    ( (_image.width() - _topLeftOfVisibleImage.x())*_scaleFactor > width()) )
                _topLeftOfVisibleImage.setX(_topLeftOfVisibleImage.x()+10);
            else if (event->key() == Qt::Key_Down && _topLeftOfVisibleImage.y())
                _topLeftOfVisibleImage.setY(_topLeftOfVisibleImage.y() - 10);
            else if (event->key() == Qt::Key_Up && 
                    ((_image.height() - _topLeftOfVisibleImage.y()) * _scaleFactor > height()) )
                _topLeftOfVisibleImage.setY(_topLeftOfVisibleImage.y() + 10);

            if(_topLeftOfVisibleImage.x() < 0)
                _topLeftOfVisibleImage.setX(0);
            if(_topLeftOfVisibleImage.y() < 0)
                _topLeftOfVisibleImage.setY(0);
            _CalcScaleFactor(0);     // leave scale factor
        }
    }
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (_fitToWindow && event->button() == Qt::LeftButton)
    {
        _mouseLeftButtonPressed = true;
        _mousePos = event->pos();
    }
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
        QPoint dp = event->pos() - _mousePos;
    }
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    double factor = -1.0;
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
    _CalcScaleFactor(factor, true, true);

    event->accept();

}

void ImageViewer::contextMenuEvent(QContextMenuEvent* pevent)
{
    _popupMenu->exec(pevent->globalPos());
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    if (_windowScalesWithImage && size() != _requiredWidgetSize)
    {
        _CalcScaleFactor(0.0);
    }
    int lw = 2 * (IMAGE_MARGIN + IMAGE_BORDER);
    int w = width() - 2 * lw;
    ui.statusFrame->setGeometry(lw, lw, w, height()-2*lw);
    w -= 500;
    if (w < 0)
        w = 50;
    QFontMetrics fm(ui.lblIName->font());
    QString qs = fm.elidedText(_fileName, Qt::ElideLeft, w);
    ui.lblIName->setText(qs);
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
    int ww = width()  - 2 * (IMAGE_BORDER + IMAGE_MARGIN) - spt.x(), 
        wh = height() - 2 * (IMAGE_BORDER + IMAGE_MARGIN) - spt.y();

    painter.drawImage(x0, y0, _scaledImage, spt.x(), spt.y(), ww, wh);
}

//void ImageViewer::showEvent(QShowEvent* event)
//{
//    //static bool shown = false;
//    //if (!shown)
//    //{
//    //    _CalcScaleFactor(-1.0);      // fit
//    //    shown = true;
//    //}
//}
//

bool ImageViewer::_IsImageMoveable()
{
    return !_windowScalesWithImage && !_fitToWindow;
}

//void ImageViewer::_Resize()    // resize window does not change image in imageFrame
//{   
//    QSize siz = size();
//    if (siz != _requiredWidgetSize)
//    {
//        resize(_requiredWidgetSize);
//        ui.statusFrame->setGeometry(0, 0, width(), height());
//    }
//
//}

QSize ImageViewer::_TransformedSizeToWidgetSize(QSize reqSize)
{
    int ilh = reqSize.height() + 2 * IMAGE_MARGIN + IMAGE_BORDER,
        ilw = reqSize.width() + +2 * IMAGE_MARGIN + IMAGE_BORDER;
    return QSize(ilw,ilh);
}

QSize ImageViewer::_WidgetSizeToImageSize(QSize wsize)
{
    int ilh = wsize.height() - 2 * IMAGE_MARGIN + IMAGE_BORDER,
        ilw = wsize.width()  - 2 * IMAGE_MARGIN + IMAGE_BORDER;
    return QSize(ilw,ilh);
}

QSize ImageViewer::_CalcRequiredSize()
{
    double  sw = _image.width() * _scaleFactor,     // image size after scaling
            sh = _image.height() * _scaleFactor;
//    QSize widgetSize = QSize(sw, sh);
    QSize widgetSize = _TransformedSizeToWidgetSize(QSize(sw, sh));

    _needsFullScreen = widgetSize.width() >= _screenSize.width() || widgetSize.height() >= _screenSize.height();
    //if (_needsFullScreen)
    //    return _screenSize;
    return widgetSize;
}

void ImageViewer::_CreateActions()
{
    _popupMenu = new QMenu(this);

    _zoomInAct = _popupMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::_ZoomIn);
    _zoomInAct->setShortcut(QKeySequence::ZoomIn);

    _zoomOutAct = _popupMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::_ZoomOut);
    _zoomOutAct->setShortcut(QKeySequence::ZoomOut);

    _normalSizeAct = _popupMenu->addAction(tr("&100%"), this, &ImageViewer::_NormalSize);
    _normalSizeAct->setShortcut(tr("Ctrl+1"));

    _fitToWindowAct = _popupMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::_FitToWindow);
    _fitToWindowAct->setShortcut(tr("Ctrl+0"));
    _fitToWindowAct->setCheckable(true);
    _fitToWindowAct->setChecked(true);

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
    _CalcScaleFactor(1.25, true, true);
}

void ImageViewer::_ZoomOut()
{
    _CalcScaleFactor(0.8, true, true);
}

void ImageViewer::_NormalSize()
{
    _CalcScaleFactor(1.0, true, true);
}

void ImageViewer::_FitToWindow()
{
    _CalcScaleFactor(-1.0, true, true);
}

bool ImageViewer::_SetFullScreen(bool setIt)
{
	Qt::WindowFlags flg;
	if (!setIt && _isFullScreen)
	{
		flg = windowFlags() & ~(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
		_fullScreenAct->setChecked(false);
		_isFullScreen = false;
		setWindowFlags(flg);
        setGeometry(_lastNotFullScreenRect);
        show();                // when changed Qt hides window
		return true;
	}
	else if (setIt & !_isFullScreen)
	{
		flg |= Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint;
		_fullScreenAct->setChecked(true);
		_isFullScreen = true;
		setWindowFlags(flg);
        _lastNotFullScreenRect = geometry();
        move(0,0);
        resize(_screenSize);
        show();                // when changed Qt hides window
		return true;
	}
	return false;

}

/*=============================================================
 * TASK:    determine the scale factor which will make w
 *          the image fit into the image frame of the widget
 * PARAMS:  wsize: widget size
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
 *                   0.0: just redisplay with current parameters
 *                  -1.0: fit in window
 * GLOBALS: _scaleFactor
 *          popup actions
 *          _requiredWidgetSize
 * RETURNS:
 * REMARKS: - if the window scales with the image the image always
 *              fills the window
 *          -
 *------------------------------------------------------------*/
void ImageViewer::_CalcScaleFactor(double factor, bool showImage, bool doResize)
{
    double fitfact = _GetFitFactor( size() );  // with tis image will fit into the actual widget

    if (factor < 0)
        _scaleFactor = fitfact; // from actual widget size
    else if (factor == 1.0)
        _scaleFactor = 1.0;
    else if (factor != 0.0)
        _scaleFactor *= factor;

    QSize newSize = _CalcRequiredSize();    // widget size for _scaleFactor

    _SetFullScreen(_needsFullScreen);       // does nothing when no change

    if (_scaleFactor > 3.0)
        _scaleFactor = 3;
    else if (_scaleFactor < 0.1)
        _scaleFactor = 0.1;

    _zoomInAct->setEnabled(_scaleFactor < 3.0);
    _zoomOutAct->setEnabled(_scaleFactor > 0.1);

    _fitToWindowAct->setChecked(_fitToWindow = _scaleFactor == fitfact);

    double  w = _image.width(),     // width of original image
            h = _image.height();

    int sw = w * _scaleFactor,
        sh = h * _scaleFactor;

    if (_scaleFactor != _prevScaleFactor)
    {
        _scaledImage = _image.scaled(sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        _prevScaleFactor = _scaleFactor;
    }

    _requiredWidgetSize = _TransformedSizeToWidgetSize(QSize(sw, sh));

    ui.lblZoom->setText(QString("%1%").arg(_scaleFactor * 100.0, 10, 'f', 0));
    if (doResize && _windowScalesWithImage && size() != _requiredWidgetSize)
    {
        resize(_requiredWidgetSize);
    }

    if (showImage)
        update();

}

void ImageViewer::_SetImage(const QImage& newImage)
{
    _image = newImage;
    if (_image.colorSpace().isValid())
        _image.convertToColorSpace(QColorSpace::SRgb);
//    ->setPixmap(QPixmap::fromImage(_image));
    _aspect = double(_image.width()) / double(_image.height());
}

void ImageViewer::_Help()
{
    QMessageBox::about(this, tr("falconG - Help"),
        tr("<p><b>Keyboard and Mouse for the image viewr</b></p>"
           "<p><b>?</b> - show this help</p>"
           "<p><b>I</b> - toggle status display</p>"
           "<p><b>1</b> - zoom to 100%</p>"
           "<p><b>0</b> - zoom to fit window</p>"
           "<p><b>+</b> - zoom in 25%</p>"
           "<p><b>-</b> - zoom out 25%</p>"
           "<p><b>f</b> - toggle fit window to image</p>"
           "<p><b>arrow keys</b> - pan the image</p>"
           "<p><b>Mouse</b> - Use the wheel to zoom in or out<br>"
            "Press and hold the <b>left</b> mouse button while"
            "moving the mouse to pan the image</p>"
        ));
}

void ImageViewer::_ToggleStatus()
{
    static bool isOn = false;

    if (isOn)
        ui.statusFrame->hide(), isOn = false, _statusAct->setChecked(false);
    else
        ui.statusFrame->show(), isOn = true, _statusAct->setChecked(true);
}