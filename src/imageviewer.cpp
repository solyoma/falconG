#include "imageviewer.h"

ImageViewer::ImageViewer(QString fileName)
{
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event)
{
}

void ImageViewer::keypressEvent(QKeyEvent* event)
{
}

bool ImageViewer::_Load(QString& fileName)
{
	QImageReader reader(fileName);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) 
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Cannot load %1: %2")
			.arg(QDir::toNativeSeparators(fileName), reader.errorString()));
		return false;
	}
	scaleFactor = 1.0;

	scrollArea->setVisible(true);
	fitToWindowAct->setEnabled(true);
	updateActions();

	if (!fitToWindowAct->isChecked())
		imageLabel->adjustSize();
}

void ImageViewer::_CreateActions()
{
	QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

	QAction* openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
	openAct->setShortcut(QKeySequence::Open);

	saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
	saveAsAct->setEnabled(false);

	printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
	printAct->setShortcut(QKeySequence::Print);
	printAct->setEnabled(false);

	fileMenu->addSeparator();

	QAction* exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
	exitAct->setShortcut(tr("Ctrl+Q"));

	QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));

	copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
	copyAct->setShortcut(QKeySequence::Copy);
	copyAct->setEnabled(false);

	QAction* pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
	pasteAct->setShortcut(QKeySequence::Paste);

	QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

	zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
	zoomInAct->setShortcut(QKeySequence::ZoomIn);
	zoomInAct->setEnabled(false);

	zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
	zoomOutAct->setShortcut(QKeySequence::ZoomOut);
	zoomOutAct->setEnabled(false);

	normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
	normalSizeAct->setShortcut(tr("Ctrl+S"));
	normalSizeAct->setEnabled(false);

	viewMenu->addSeparator();

	fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
	fitToWindowAct->setEnabled(false);
	fitToWindowAct->setCheckable(true);
	fitToWindowAct->setShortcut(tr("Ctrl+F"));

	QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
	helpMenu->addAction(tr("About &Qt"), this, &QApplication::aboutQt);
}
