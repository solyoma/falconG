/*
 *  Copyright (C) 2013-2014 Ofer Kashayov <oferkv@live.com>
 *  This file is part of Phototonic Image Viewer.
 *
 *  Phototonic is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Phototonic is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Phototonic.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "thumbnailWidget.h"

 /*=============================================================
  * TASK:		constractor
  * EXPECTS:	parent: widget to contain his
  *				thumbsize - pixel width and height of thumbnails
  * GLOBALS:
  * RETURNS:
  * REMARKS:   connects slots and sets up scrollbar
  *------------------------------------------------------------*/
ThumbnailWidget::ThumbnailWidget(QWidget *parent, int thumbsize) : QListView(parent), thumbSize(thumbsize)
{
    currentRow = 0;

    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setResizeMode(QListView::Adjust);
    setWordWrap(true);
    setDragEnabled(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformItemSizes(false);

    ThumbnailWidgetModel = new QStandardItemModel(this);
    ThumbnailWidgetModel->setSortRole(SortRole);
    setModel(ThumbnailWidgetModel);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(loadVisibleThumbs(int)));
    connect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));
/*------
	connect(this, SIGNAL(doubleClicked(
                                 const QModelIndex &)), parent, SLOT(loadSelectedThumbImage(
                                                                             const QModelIndex &)));
*-----*/

    fileFilters = new QStringList;
    emptyImg.load(":/images/no_image.png");

    QTime time = QTime::currentTime();
    qsrand((uint) time.msec());

	setAcceptDrops(true);
}

//void ThumbnailWidget::setThumbColors() {
//    QString backgroundColor = "background: rgb(%1, %2, %3); ";
//    backgroundColor = backgroundColor.arg(Settings::thumbsBackgroundColor.red())
//            .arg(Settings::thumbsBackgroundColor.green())
//            .arg(Settings::thumbsBackgroundColor.blue());
//
//    QString styleSheet = "QListView { " + backgroundColor + "background-image: url("
//                         + Settings::thumbsBackgroundImage
//                         + "); background-attachment: fixed; }";
//    setStyleSheet(styleSheet);
//
//    QPalette scrollBarOriginalPalette = verticalScrollBar()->palette();
//    QPalette thumbViewerOriginalPalette = palette();
//    thumbViewerOriginalPalette.setColor(QPalette::LanguageTexts, Settings::thumbsTextColor);
//    setPalette(thumbViewerOriginalPalette);
//    verticalScrollBar()->setPalette(scrollBarOriginalPalette);
//}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void ThumbnailWidget::selectCurrentIndex()
{
    if (currentIndex.isValid() && ThumbnailWidgetModel->rowCount() > 0) 
	{
        scrollTo(currentIndex);
        setCurrentIndex(currentIndex);
    }
}

/*=============================================================
 * TASK:	get the parameter set for 'FileNameRole'
 * EXPECTS:	nothing
 * GLOBALS:	file name set or empty string when ther are either
 *			 no or multiple selections
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QString ThumbnailWidget::getSingleSelectionFilename()
{
    if (selectionModel()->selectedIndexes().size() == 1)
        return ThumbnailWidgetModel->item(selectionModel()->selectedIndexes().first().row())->data(
                FileNameRole).toString();

    return ("");
}

/*============================================================================
  * TASK:	set list of images (needed for UNDELETE) into this
  * EXPECTS:	pidl: pointer to idList or nullptr
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
void ThumbnailWidget::SetImageList(IdList * pidl)
{
	_pImages = pidl;
}

/*=============================================================
 * TASK:	move 'currentRow' to next thumbnail
 * EXPECTS:
 * GLOBALS:
 * RETURNS:	new row or -1 when no more rows
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getNextRow()
{
    if (currentRow == ThumbnailWidgetModel->rowCount() - 1) 
        return -1;

    return currentRow + 1;
}

/*=============================================================
* TASK:	move 'currentRow' to previous thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS:	new row or -1 when no previous row
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailWidget::getPrevRow()
{
    if (currentRow == 0) 
        return -1;

    return currentRow - 1;
}

/*=============================================================
 * TASK:	gets row of last thumbnail
 * EXPECTS:
 * GLOBALS:
 * RETURNS: image count -1
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getLastRow()
{
    return ThumbnailWidgetModel->rowCount() - 1;
}

/*=============================================================
* TASK:	gets index of last thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS: ordinal of random thumbnail
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailWidget::getRandomRow()
{
    return qrand() % (ThumbnailWidgetModel->rowCount());
}

int ThumbnailWidget::getCurrentRow() 
{
    return currentRow;
}

void ThumbnailWidget::setCurrentRow(int row) 
{
    if (row >= 0) 
        currentRow = row;
    else
        currentRow = 0;
}

/*=============================================================
 * TASK:	Set title string to current/ total  ordinal in 
 *			square brackets
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: emits signal 'TitleChanged'
 *------------------------------------------------------------*/
void ThumbnailWidget::setTitle()
{
    title = ThumbnailWidgetModel->item(currentRow)->data(Qt::DisplayRole).toString()
                    + " - ["
                    + QString::number(currentRow + 1)
                    + "/"
                    + QString::number(ThumbnailWidgetModel->rowCount())
                    + "]";
	emit SignalTitleChanged(title);
}

/*=============================================================
 * TASK:	move 'currentIndex' to thumbnail whose file name 
 *			(FileNameRole) matches 'fileName'
 * EXPECTS:	'fileName' - match this
 * GLOBALS:
 * RETURNS:	whether it is found
 * REMARKS:	sets currentIndex and current thumbnail only when found
 *------------------------------------------------------------*/
bool ThumbnailWidget::setCurrentIndexByName(QString &fileName)
{
    QModelIndexList indexList = ThumbnailWidgetModel->match(ThumbnailWidgetModel->index(0, 0), FileNameRole, fileName);
    if (indexList.size()) 
	{
        currentIndex = indexList[0];
        setCurrentRow(currentIndex.row());
        return true;
    }

    return false;
}

/*=============================================================
 * TASK:	determine the model index for a given row
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool ThumbnailWidget::setCurrentIndexByRow(int row)
{
	QModelIndex idx = ThumbnailWidgetModel->indexFromItem(ThumbnailWidgetModel->item(row));
	if (idx.isValid()) 
	{
		currentIndex = idx;
		setCurrentRow(idx.row());
		return true;
	}

    return false;
}

/*=============================================================
 * TASK:	slot for signal when model selection is changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: emits signal 'SingleSelection ' with selected file name
 *------------------------------------------------------------*/
void ThumbnailWidget::onSelectionChanged(const QItemSelection &)
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    int selectedThumbs = indexesList.size();
	if (selectedThumbs == 1)
	{
		int current_row = indexesList.first().row();
		QString thumbFullPath = ThumbnailWidgetModel->item(current_row)->data(FileNameRole).toString();
		setCurrentRow(current_row);
		//  do something
		emit SingleSelection(thumbFullPath);
	}
	else
		emit SingleSelection("");

    if (selectedThumbs >= 1) 
	{
        statusStr = tr("Selected %1 of %2").arg(QString::number(selectedThumbs))
                .arg(tr(" %n image(s)", "", ThumbnailWidgetModel->rowCount()));
    } 
	else if (!selectedThumbs) 
	{
		statusStr.clear();
        updateThumbsCount();
    }
	emit SignalStatusChanged(statusStr);
}

/*=============================================================
 * TASK:	collect names of select files into a string list
 * EXPECTS:
 * GLOBALS:
 * RETURNS: list of path names of selected files
 * REMARKS:
 *------------------------------------------------------------*/
QStringList ThumbnailWidget::getSelectedThumbsList()
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    QStringList SelectedThumbsPaths;

    for (int tn = indexesList.size() - 1; tn >= 0; --tn) 
	{
        SelectedThumbsPaths << ThumbnailWidgetModel->item(indexesList[tn].row())->data(FileNameRole).toString();
    }

    return SelectedThumbsPaths;
}

/*=============================================================
 * TASK:	starts dragging when there are selected thumbnails
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- overrides 'startDrag()' of QListView
 *			- sets all file names as URLs into the mimeData of
 *				the drag
 *			- creates a pixmap to show what you are dragging
 *				for the drag object
 *			- sets hot spot into the middle of this pixmap
 *			- executes drag with allowing all actions with a 
 *				default of ignoring the drop
 *------------------------------------------------------------*/
void ThumbnailWidget::startDrag(Qt::DropActions)
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    if (indexesList.isEmpty()) 
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    for (QModelIndexList::const_iterator it = indexesList.constBegin(),
                 end = indexesList.constEnd(); it != end; ++it) 
	{
        urls << QUrl(ThumbnailWidgetModel->item(it->row())->data(FileNameRole).toString());
    }
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);
    QPixmap pix;
    if (indexesList.count() > 1) 
	{
        pix = QPixmap(128, 112);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::white, 2));
        int x = 0, y = 0, xMax = 0, yMax = 0;
        for (int i = 0; i < qMin(5, indexesList.count()); ++i) 
		{
            QPixmap pix1 = ThumbnailWidgetModel->item(indexesList.at(i).row())->icon().pixmap(72);
            if (i == 4) 
			{
                x = (xMax - pix1.width()) / 2;
                y = (yMax - pix1.height()) / 2;
            }
            painter.drawPixmap(x, y, pix1);
            xMax = qMax(xMax, qMin(128, x + pix1.width()));
            yMax = qMax(yMax, qMin(112, y + pix1.height()));
            painter.drawRect(x + 1, y + 1, qMin(126, pix1.width() - 2), qMin(110, pix1.height() - 2));
            x = !(x == y) * 56;
            y = !y * 40;
        }
        painter.end();
        pix = pix.copy(0, 0, xMax, yMax);
        drag->setPixmap(pix);
    } 
	else 
	{
        pix = ThumbnailWidgetModel->item(indexesList.at(0).row())->icon().pixmap(128);
        drag->setPixmap(pix);
    }
    drag->setHotSpot(QPoint(pix.width() / 2, pix.height() / 2));
	drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::MoveAction); //Qt::IgnoreAction);
}

void ThumbnailWidget::abort() 
{
    isAbortThumbsLoading = true;
}

void ThumbnailWidget::loadVisibleThumbs(int scrollBarValue) 
{
    static int lastScrollBarValue = 0;

    scrolledForward = (scrollBarValue >= lastScrollBarValue);

    lastScrollBarValue = scrollBarValue;

	emit SignalInProcessing(true);

    for (;;) {
        int firstVisible = getFirstVisibleThumb();
        int lastVisible = getLastVisibleThumb();
        if (isAbortThumbsLoading || firstVisible < 0 || lastVisible < 0) {
            return;
        }

        if (scrolledForward) 
		{
			lastVisible += ((lastVisible - firstVisible) * 3); // (Settings::thumbsPagesReadCount + 1));
            if (lastVisible >= ThumbnailWidgetModel->rowCount()) 
                lastVisible = ThumbnailWidgetModel->rowCount() - 1;
            
        } 
		else 
		{
            firstVisible -= (lastVisible - firstVisible) * 3;  // (Settings::thumbsPagesReadCount + 1);
            if (firstVisible < 0) 
                firstVisible = 0;

            lastVisible += 10;
            if (lastVisible >= ThumbnailWidgetModel->rowCount()) 
                lastVisible = ThumbnailWidgetModel->rowCount() - 1;
        }

        if (thumbsRangeFirst == firstVisible && thumbsRangeLast == lastVisible) 
            return;

        thumbsRangeFirst = firstVisible;
        thumbsRangeLast = lastVisible;

        loadThumbsRange();
        if (isAbortThumbsLoading) 
            break;
        
    }
	emit SignalInProcessing(false);
}

int ThumbnailWidget::getFirstVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = 0; currThumb < ThumbnailWidgetModel->rowCount(); ++currThumb) 
	{
        idx = ThumbnailWidgetModel->indexFromItem(ThumbnailWidgetModel->item(currThumb));
        if (viewport()->rect().contains(QPoint(0, visualRect(idx).y() + visualRect(idx).height() + 1))) 
            return idx.row();
        
    }

    return -1;
}

int ThumbnailWidget::getLastVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = ThumbnailWidgetModel->rowCount() - 1; currThumb >= 0; --currThumb) 
	{
        idx = ThumbnailWidgetModel->indexFromItem(ThumbnailWidgetModel->item(currThumb));
        if (viewport()->rect().contains(QPoint(0, visualRect(idx).y() + visualRect(idx).height() + 1))) 
            return idx.row();
    }

    return -1;
}

void ThumbnailWidget::loadFileList() 
{
	emit SignalInProcessing(true);
    for (int i = 0; i < thumbNames.size(); i++) 
        addThumb(i);
    
    updateThumbsCount();

/*
	if (thumbNames.size() && selectionModel()->selectedIndexes().size() == 0) 
        selectThumbByRow(0);
*/
	isBusy = false;
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
	emit SignalInProcessing(false);
}

void ThumbnailWidget::reLoad() 
{
    isBusy = true;
    loadPrepare();

    if (thumbNames.size()) 
	{
        loadFileList();
        return;
    }

    initThumbs();
    updateThumbsCount();
    loadVisibleThumbs();

    isBusy = false;
}

void ThumbnailWidget::loadPrepare() 
{

    ThumbnailWidgetModel->clear();
    setIconSize(QSize(thumbSize, thumbSize));
    setSpacing(QFontMetrics(font()).height());

    if (isNeedToScroll) {
        scrollToTop();
    }

    isAbortThumbsLoading = false;

    thumbsRangeFirst = -1;
    thumbsRangeLast = -1;
}

void ThumbnailWidget::initThumbs()
{
	static QStandardItem *thumbItem;
	static int fileIndex;
	static QPixmap emptyPixMap;
	static QSize hintSize;
	int thumbsAddedCounter = 1;

	emptyPixMap = QPixmap::fromImage(emptyImg).scaled(thumbSize, thumbSize);
	hintSize = QSize(thumbSize, thumbSize + ((int)(QFontMetrics(font()).height() * 1.5)));

	thumbItem = new QStandardItem();
	QString imageFullPath;
	for (fileIndex = 0; fileIndex < thumbNames.size(); ++fileIndex)
	{
		imageFullPath = thumbsDir + thumbNames[fileIndex];

		thumbItem->setData(false, LoadedRole);
		thumbItem->setData(fileIndex, SortRole);
		thumbItem->setData(thumbsDir, FilePathRole);
		thumbItem->setData(thumbNames[fileIndex], FileNameRole);
		thumbItem->setData(originalPaths[fileIndex] + " \n(" + originalPaths[fileIndex] + ")", Qt::DisplayRole);
		thumbItem->setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
		thumbItem->setSizeHint(hintSize);
		thumbItem->setText(/*imageFullPath + "\n" + */originalPaths[fileIndex]);

		ThumbnailWidgetModel->appendRow(thumbItem);

		++thumbsAddedCounter;
		if (thumbsAddedCounter > 100)
		{
			thumbsAddedCounter = 1;
			QApplication::processEvents();
		}
	}

	if (thumbNames.size() && selectionModel()->selectedIndexes().size() == 0)
		selectThumbByRow(0);

	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
}

void ThumbnailWidget::updateThumbsCount() 
{
    if (ThumbnailWidgetModel->rowCount() > 0) 
        statusStr = tr("%n image(s)", "", ThumbnailWidgetModel->rowCount());
    else 
		statusStr = tr("No images");
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
}

void ThumbnailWidget::selectThumbByRow(int row) 
{
    setCurrentIndexByRow(row);
    selectCurrentIndex();
}

/*=============================================================
 * TASK:	load next range of thumbnails when scrolling the view
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::loadThumbsRange()
{
    static bool isInProgress = false;
    QImageReader thumbReader;
    static QSize currentThumbSize;
    static int currentRowCount;
    static QString imageFileName;
    QImage thumb;
    int currThumb;
    bool imageReadOk;

    if (isInProgress) 
	{
        isAbortThumbsLoading = true;
        QTimer::singleShot(0, this, SLOT(loadThumbsRange()));
        return;
    }

    isInProgress = true;
    currentRowCount = ThumbnailWidgetModel->rowCount();

    for (scrolledForward ? currThumb = thumbsRangeFirst : currThumb = thumbsRangeLast;
         (scrolledForward ? currThumb <= thumbsRangeLast : currThumb >= thumbsRangeFirst);
         scrolledForward ? ++currThumb : --currThumb) 
	{

        if (isAbortThumbsLoading || ThumbnailWidgetModel->rowCount() != currentRowCount || currThumb < 0)
            break;

        if (ThumbnailWidgetModel->item(currThumb)->data(LoadedRole).toBool())
            continue;

        imageFileName = ThumbnailWidgetModel->item(currThumb)->data(FileNameRole).toString();
        thumbReader.setFileName(imageFileName);
        currentThumbSize = thumbReader.size();
        imageReadOk = false;

        if (currentThumbSize.isValid()) 
		{
            if (currentThumbSize.width() > thumbSize || currentThumbSize.height() > thumbSize) 
                currentThumbSize.scale(QSize(thumbSize, thumbSize), Qt::KeepAspectRatio);
        
            thumbReader.setScaledSize(currentThumbSize);
            imageReadOk = thumbReader.read(&thumb);
        }

        if (imageReadOk) 
		
            ThumbnailWidgetModel->item(currThumb)->setIcon(QPixmap::fromImage(thumb));
         else 
		 {
            ThumbnailWidgetModel->item(currThumb)->setIcon(QIcon::fromTheme("image-missing",
                                                              QIcon(":/images/error_image.png")).pixmap(
															           BAD_IMAGE_SIZE, BAD_IMAGE_SIZE));
            currentThumbSize.setHeight(BAD_IMAGE_SIZE);
            currentThumbSize.setWidth(BAD_IMAGE_SIZE);
        }

        ThumbnailWidgetModel->item(currThumb)->setData(true, LoadedRole);
        QApplication::processEvents();
    }

    isInProgress = false;
    isAbortThumbsLoading = false;
}

/*=============================================================
 * TASK:	adds a QStandardItem for athumbnail to the end of the 
 *			rows
 * EXPECTS:	which - ordinal of data for thumbnail in
 *					thumbNames[] and originalPaths[]
 * GLOBALS:
 * RETURNS:	nothing
 * REMARKS:	- sets 
 *------------------------------------------------------------*/
void ThumbnailWidget::addThumb(int which)
{
    QStandardItem *thumbItem = new QStandardItem();
    QImageReader thumbReader;
    QSize hintSize;
    QSize currThumbSize;
    static QImage thumb;

    hintSize = QSize(thumbSize, thumbSize + ((int) (QFontMetrics(font()).height() * 1.5)));

	QString imageFullPath = thumbsDir + thumbNames[which];
//    thumbFileInfo = QFileInfo(imageFullPath);
    thumbItem->setData(true, LoadedRole);
    thumbItem->setData(0, SortRole);
    thumbItem->setData(thumbsDir, FilePathRole);
	thumbItem->setData(thumbNames[which], FileNameRole);
	thumbItem->setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
    thumbItem->setData(thumbNames[which] + " \n(" + originalPaths[which] + ")", Qt::DisplayRole);
    thumbItem->setSizeHint(hintSize);

    thumbReader.setFileName(imageFullPath);
    currThumbSize = thumbReader.size();
    if (currThumbSize.isValid()) 
	{
        if (currThumbSize.width() > thumbSize || currThumbSize.height() > thumbSize) 
            currThumbSize.scale(QSize(thumbSize, thumbSize), Qt::KeepAspectRatio);

        thumbReader.setScaledSize(currThumbSize);
        thumb = thumbReader.read();

        thumbItem->setIcon(QPixmap::fromImage(thumb));
    } 
	else 
	{
        thumbItem->setIcon(
                QIcon::fromTheme("image-missing", QIcon(":/images/error_image.png")).pixmap(BAD_IMAGE_SIZE,
                                                                                            BAD_IMAGE_SIZE));
        currThumbSize.setHeight(BAD_IMAGE_SIZE);
        currThumbSize.setWidth(BAD_IMAGE_SIZE);
    }

    ThumbnailWidgetModel->appendRow(thumbItem);
}

/*=============================================================
 * TASK:	modifies scrollbar position for wheel events
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + thumbSize);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - thumbSize);
    }
}

/*=============================================================
 * TASK:	executes mouse press
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - it does nothing new yet
 *------------------------------------------------------------*/
void ThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);
}

/*=============================================================
 * TASK: Event handler for right floating menu
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::contextMenuEvent(QContextMenuEvent * pevent)
{
	QMenu menu(this);
	QAction *pact;

	bool b = selectionModel()->selectedIndexes().size() == 1;

	pact = new QAction(tr("Set As Album Thumbnail"), this);
	pact->setEnabled(b);
	if(b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::SetAsAlbumThhumbnail);
	menu.addAction(pact);
	
	b = selectionModel()->selectedIndexes().size();
	pact = new QAction(tr("&Delete"), this);
	pact->setEnabled(b);
	if (b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::DeleteSelected);

	pact = new QAction(tr("&Undo Delete"), this);
	bool undo = _pImages && !_pImages->NothingToUndo();
	pact->setEnabled(undo);
	if (undo)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::UndoDelete);


	pact = new QAction(tr("Add..."), this);
	pact->setEnabled(b);
	if (b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::AddImages);

	menu.addSeparator();

	pact = new QAction(tr("Copy &Name(s)"), this);
	pact->setEnabled(b);
	if (b)
		connect(pact, &QAction::triggered, this,  &ThumbnailWidget::CopyNamesToClipboard);
	menu.addAction(pact);

	pact = new QAction(tr("Copy &Original Name(s)"), this);
	pact->setEnabled(b);
	if (b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::CopyOriginalNamesToClipboard);
	menu.addAction(pact);

	menu.exec(pevent->globalPos());
}

void ThumbnailWidget::invertSelection() 
{
    QItemSelection toggleSelection;
    QModelIndex firstIndex = ThumbnailWidgetModel->index(0, 0);
    QModelIndex lastIndex = ThumbnailWidgetModel->index(ThumbnailWidgetModel->rowCount() - 1, 0);
    toggleSelection.select(firstIndex, lastIndex);
    selectionModel()->select(toggleSelection, QItemSelectionModel::Toggle);
}

/*=============================================================
 * TASK: Delete selected images from image list of actual album
 *		 into undo buffer
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::DeleteSelected()
{
	QString s;
	QModelIndexList list = selectionModel()->selectedIndexes();
	for (auto i : list)
		s += selectionModel()->model()->data(i, FileNameRole).toString();
}

/*=============================================================
 * TASK:	add deleted images back
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::UndoDelete()
{
}

/*=============================================================
 * TASK:   Display dialog box to add images to image list
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::AddImages()
{
}

/*============================================================================
  * TASK:	copy names of all selected thumbnail names to clipboard
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: never called when there is no selection
 *--------------------------------------------------------------------------*/
void ThumbnailWidget::CopyNamesToClipboard()
{
	QString s;
	QModelIndexList list = selectionModel()->selectedIndexes();
	for (auto i : list)
		s += selectionModel()->model()->data(i, FileNameRole).toString() + "\n"; 
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->clear();
	clipboard->setText(s);
}

/*============================================================================
* TASK:	copy new and original full path names of all selected thumbnails to clipboard
* EXPECTS:
* RETURNS:
* GLOBALS:
* REMARKS: never called when there is no selection
*--------------------------------------------------------------------------*/
void ThumbnailWidget::CopyOriginalNamesToClipboard()
{
	QString s;
	QModelIndexList list = selectionModel()->selectedIndexes();
	for (auto i : list)
		s += selectionModel()->model()->data(i, Qt::DisplayRole).toString() + "\n";
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->clear();
	clipboard->setText(s);
}

/*============================================================================
  * TASK:	signal set current file name as album thumbnail
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: only called when teher is exactly one selected image
 *--------------------------------------------------------------------------*/
void ThumbnailWidget::SetAsAlbumThhumbnail()
{
	emit SignalNewThumbnail();
}

void ThumbnailWidget::setNeedToScroll(bool needToScroll) {
    this->isNeedToScroll = needToScroll;
}
