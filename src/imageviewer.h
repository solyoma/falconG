#pragma once
#include <QtCore>
#include <QtWidgets/QWidget>

#include "support.h"
#include "config.h"

#include <QGuiApplication>
#include <QScreen>

#include "imageviewer.h"

#include "ui_imageviewer.h"

constexpr int IMAGE_MARGIN = 9;
constexpr int IMAGE_BORDER = 4;

class ImageViewer : public QWidget
{
	Q_OBJECT

	Ui::ImageViewerClass ui;

public:
	ImageViewer(QString fileName, QWidget* parent=nullptr);
	~ImageViewer() {}
	bool LoadFile();
protected:
	int heightForWidth(int w);

	void keyPressEvent(QKeyEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	void contextMenuEvent(QContextMenuEvent* pevent);
//	void showEvent(QShowEvent* event);
	void resizeEvent(QResizeEvent* event);
	void paintEvent(QPaintEvent* event);
private:
	QString _fileName;
	QImage _image;
	QImage _scaledImage;
	double _aspect;
	double	_prevScaleFactor,						// used when _scaledImage was created
			_scaleFactor = 1.0;						// size relative to image size, 
	QPoint _topLeftOfVisibleImage = QPoint(0,0);	// draw image from this point to label
													// for original image
	bool _windowScalesWithImage=true;				// until full screen sizes and not below 100 x 100 pixels
	bool _fitToWindow = false;						// set when window does not scale with image
													// either because the user set it so or when not full screen
	bool _scrollbarOn = false;						// set when window does not scale with image
	bool _mouseLeftButtonPressed = false;
	QPoint _mousePos;
	bool _needsFullScreen=false;					// set when window cannot be resized bigger
	bool _isFullScreen=false;						// set when window is on fullscreen
	bool _notResized = true;						// widget was not resized?
	QSize _screenSize, 
		  _requiredWidgetSize;
	bool _InfoOverlayOn = false;					// image info overlay visible?
	QRect _lastNotFullScreenRect;


	QMenu*	 _popupMenu;
	QAction* _closeAct;
	QAction* _statusAct;
	QAction* _zoomInAct;
	QAction* _zoomOutAct;
	QAction* _normalSizeAct;
	QAction* _fitToWindowAct;
	QAction* _fullScreenAct;

	bool _IsImageMoveable();
	QSize _CalcRequiredSize();	// if _windowScalesWithImage resizes widget for image to fit. returns new size, sets _needsFullScreen

	QSize _TransformedSizeToWidgetSize(QSize reqSize);
	QSize _WidgetSizeToImageSize(QSize wsize);

	void _CreateActions();
//	void _Resize();
			// factor: -1 to fit in actual window, 0: to leave it, just move image
	void _CalcScaleFactor(double factor, bool showImage=false, bool doResize=false);

	void _ZoomIn();
	void _ZoomOut();
	void _NormalSize();
	void _FitToWindow();
	bool _SetFullScreen(bool setIt);	// set window to full screen or back if rescale is true reset
	double _GetFitFactor(QSize wsize);	// for widget size
	void _Help();
	void _ToggleStatus();

	void _SetImage(const QImage& newImage);
};
