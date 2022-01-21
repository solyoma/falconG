#pragma once
#include <QtWidgets>
#include <QScrollArea>
#include <QImage>
#include <QEvents>

#include "support.h"
#include "config.h"
#include "stylehandler.h"
#include "albums.h"

#include "ui_imageViewer.h"

class ImageViewer : public QWidget
{
public:
	ImageViewer(QString fileName);
protected:
	void wheelEvent(QWheelEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void keypressEvent(QKeyEvent* event);
private:
	QImage image;
	double scaleFactor = 1;
	QAction* zoomInAct;
	QAction* zoomOutAct;
	QAction* normalSizeAct;
	QAction* fitToWindowAct;

	bool _Load(QString &filename);
	void _CreateActions();

};