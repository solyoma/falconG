#pragma once
#include <QtCore>
#include <QtWidgets/QWidget>

#include "support.h"
#include "config.h"

#include <QGuiApplication>
#include <QScreen>

#include "ui_imageviewer.h"

constexpr int IMAGE_MARGIN = 9;		// pixel
constexpr int IMAGE_BORDER = 4;

class ThumbnailView;
class ImageViewer : public QWidget
{
	Q_OBJECT

	Ui::ImageViewerClass ui;

public:
	enum ImageFlag {imNone, imReshowImage, imResizeWithImage};
	Q_DECLARE_FLAGS(ImageFlags, ImageFlag);
public:
	ImageViewer(QString fileName, ThumbnailView *creator,QWidget* parent=nullptr);
	~ImageViewer();
	bool LoadFile();
signals:
	void SignalImageViewerClosed(ImageViewer* self);
protected:
	int heightForWidth(int w);

	void keyPressEvent(QKeyEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event)	override;
	void mouseReleaseEvent(QMouseEvent* event)	override;
	void mouseMoveEvent(QMouseEvent* event)		override;
	void wheelEvent(QWheelEvent* event)			override;
	void contextMenuEvent(QContextMenuEvent* pevent);
//	void showEvent(QShowEvent* event);
	void resizeEvent(QResizeEvent* event);
	void paintEvent(QPaintEvent* event);
private:

	ThumbnailView* _owner;
	QString _fileName;
	QString _elidedInfoText;						// info on the title bar
	QImage _image;
	QImage _scaledImage;
	double _aspect = 1.0;							// image width / image height, >1.0 landscape else portrait
	double	_prevScaleFactor,						// used when _scaledImage was created
			_scaleFactor = 1.0;						// size relative to image size, 
	QPoint _topLeftOfVisibleImage = QPoint(0,0);	// draw image from this point to label
													// for original image
	bool _windowScalesWithImage=true;				// resize window to fit image until full screen sizes 
													//	but not below 100 x 100 pixels
	bool _isImageFitToWindow = false;				// set when image just fits the window
													// either because the user set it so or when not full screen
	bool _scrollbarOn = false;						// set when window does not scale with image
	bool _mouseLeftButtonPressed = false;
	QPoint _mousePos;
		// resize
	bool _notResized = true;						// widget was not resized?
	bool _needsFullScreen=false;					// set when window scales with image and cannot be resized bigger
	bool _isFullScreen=false;						// set when window is already fullscreen
	bool _showTitleBar = false;						// on window
	int  _titleBarHeight = 48;						// get from QT on start of this widget
	QSize _screenSize,							
		  _maxFullScreenSize,						// when the widget is vertically or horizontally span the whole screen
		  _requiredWidgetSize;						// this large widget would contain the whole image

	bool _infoOverlayOn = true;						// image info overlay visible?
	QRect _lastNotFullScreenRect;

	QMenu*	 _popupMenu;
	QAction* _closeAct;
	QAction* _statusAct;
	QAction* _zoomInAct;
	QAction* _zoomOutAct;
	QAction* _normalSizeAct;
	QAction* _isImageFitToWindowAct;
	QAction* _fullScreenAct;

	bool	_IsImageMoveable();
	QSize	_CalcRequiredWidgetSize();	// if _windowScalesWithImage resizes widget for image to fit. returns new size, sets _needsFullScreen
	QString _ElideFileName(const QFont& font, int width);
	void	_SetElidedInfoText();
	void	_ToggleTitleBar();
	void	_SetMaxWidgetSizeForImageAspectRatio();

	QSize _TransformedSizeToWidgetSize(QSize reqSize);
	QSize _WidgetSizeToImageSize(QSize wsize);

	void _CreateActions();
//	void _Resize();
			// factor: -1 to fit in actual window, 0: to leave it, just move image
	void _CalcScaleFactor(double factor, QFlags<ImageFlag> flags);

	double _GetFitFactor(QSize wsize);	// for widget size
	void _ShowStatusInfo();
	void _MoveImage(QPoint dp);
//	void _Update(bool dont);

	void _SetImage(const QImage& newImage);
private slots:
	void _ZoomIn();
	void _ZoomOut();
	void _NormalSize();
	void _FitImageToWindow();
	bool _SetFullScreen(bool setIt);	// set window to full screen or back if rescale is true reset
	void _ToggleStatus();
	void _Help();
	void _PrimaryScreenChanged(QScreen* pScreen);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ImageViewer::ImageFlags);
