/*
 * part of the code is taken from Ofer Kashayov's <oferkv@live.com>
 * free Phototonic image viewer
 * My code is marked with SA in the remarks of the function descriptions
 * Ofer Kashayov's originla copyright notice:
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
  * REMARKS:	SA
  *				- connects slots and sets up scrollbar
  *------------------------------------------------------------*/
ThumbnailWidget::ThumbnailWidget(QWidget *parent, int thumbsize) : QListView(parent), _thumbSize(thumbsize)
{
    _currentRow = 0;

    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setResizeMode(QListView::Adjust);
    setWordWrap(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformItemSizes(false);
// experiment:
//	setMovement(QListView::Free);
//	setDragDropMode(QAbstractItemView::InternalMove);

    _thumbnailWidgetModel = new ThumbnailWidgetModel(this);
    _thumbnailWidgetModel->setSortRole(SortRole);
    setModel(_thumbnailWidgetModel);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(loadVisibleThumbs(int)));
    connect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection)));
/*------
	connect(this, SIGNAL(doubleClicked(
                                 const QModelIndex &)), parent, SLOT(loadSelectedThumbImage(
                                                                             const QModelIndex &)));
*-----*/

    _fileFilters = new QStringList;
    _emptyImg.load(":/icon/empty_image.png");

    //QTime time = QTime::currentTime();
    // qsrand((uint) time.msec());
    // using QRandomGenerator::global() which is always securely seeded

	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
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
    if (_currentIndex.isValid() && _thumbnailWidgetModel->rowCount() > 0) 
	{
        scrollTo(_currentIndex);
        setCurrentIndex(_currentIndex);
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
        return _thumbnailWidgetModel->item(selectionModel()->selectedIndexes().first().row())->data(
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
 * TASK:	move '_currentRow' to next thumbnail
 * EXPECTS:
 * GLOBALS:
 * RETURNS:	new row or -1 when no more rows
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getNextRow()
{
    if (_currentRow == _thumbnailWidgetModel->rowCount() - 1) 
        return -1;

    return _currentRow + 1;
}

/*=============================================================
* TASK:	move '_currentRow' to previous thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS:	new row or -1 when no previous row
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailWidget::getPrevRow()
{
    if (_currentRow == 0) 
        return -1;

    return _currentRow - 1;
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
    return _thumbnailWidgetModel->rowCount() - 1;
}

/*=============================================================
* TASK:	gets index of a random thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS: ordinal of random thumbnail
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailWidget::getRandomRow()
{
    return QRandomGenerator::global()->generate() % (_thumbnailWidgetModel->rowCount());
}

/*=============================================================
 * TASK:	return current row
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getCurrentRow()
{
    return _currentRow;
}

void ThumbnailWidget::setCurrentRow(int row) 
{
    if (row >= 0) 
        _currentRow = row;
    else
        _currentRow = 0;
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
    title = _thumbnailWidgetModel->item(_currentRow)->data(Qt::DisplayRole).toString()
                    + " - ["
                    + QString::number(_currentRow + 1)
                    + "/"
                    + QString::number(_thumbnailWidgetModel->rowCount())
                    + "]";
	emit SignalTitleChanged(title);
}

/*=============================================================
 * TASK:	move '_currentIndex' to thumbnail whose file name 
 *			(FileNameRole) matches 'fileName'
 * EXPECTS:	'fileName' - match this
 * GLOBALS:
 * RETURNS:	whether it is found
 * REMARKS:	sets _currentIndex and current thumbnail only when found
 *------------------------------------------------------------*/
bool ThumbnailWidget::setCurrentIndexByName(QString &fileName)
{
    QModelIndexList indexList = _thumbnailWidgetModel->match(_thumbnailWidgetModel->index(0, 0), FileNameRole, fileName);
    if (indexList.size()) 
	{
        _currentIndex = indexList[0];
        setCurrentRow(_currentIndex.row());
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
	QModelIndex idx = _thumbnailWidgetModel->indexFromItem(_thumbnailWidgetModel->item(row));
	if (idx.isValid()) 
	{
		_currentIndex = idx;
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
    int selectedCount = indexesList.size();
	if (selectedCount == 1)
	{
		int current_row = indexesList.first().row();
		QString thumbFullPath = _thumbnailWidgetModel->item(current_row)->data(FileNameRole).toString();
		setCurrentRow(current_row);
		//  do something
		emit SingleSelection(thumbFullPath);
	}
	else
		emit SingleSelection("");

    if (selectedCount >= 1) 
	{
        statusStr = tr("Selected %1 of %2").arg(QString::number(selectedCount))
                .arg(tr(" %n image(s)", "", _thumbnailWidgetModel->rowCount()));
    } 
	else if (!selectedCount) 
	{
		statusStr.clear();
        _updateThumbsCount();
    }
	emit SignalStatusChanged(statusStr);
}

/*=============================================================
 * TASK:	collect names of selected files into a string list
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
        SelectedThumbsPaths << _thumbnailWidgetModel->item(indexesList[tn].row())->data(FileNameRole).toString();
    }

    return SelectedThumbsPaths;
}

/*=============================================================
 * TASK:	starts dragging when there are selected thumbnails
 *			create drag object and thumbnail representation then 
 *			call drag->exec()
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:	- called by ThumbnailWidget (QListView)
 *			- overrides 'startDrag()' of QListView
 *			- sets all file names as URLs into the mimeData of
 *				the drag
 *			- creates a small pixmap from the first max. 5 images
 *				to show what you are dragging
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
    ThumbMimeData *mimeData = new ThumbMimeData;
    QList<QUrl> urls;	// create pixmap from this list of names
	ThumbnailRecord thumbRec;
	for (auto f : indexesList)
	{
		const QString &qs = _thumbnailWidgetModel->item(f.row())->data(FileNameRole).toString();
		urls << QUrl(qs);
		thumbRec.imageName = qs;
		thumbRec.row = f.row();
		mimeData->thumbList << thumbRec;
	}

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
            QPixmap pix1 = _thumbnailWidgetModel->item(indexesList.at(i).row())->icon().pixmap(72);
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
        pix = _thumbnailWidgetModel->item(indexesList.at(0).row())->icon().pixmap(128);
        drag->setPixmap(pix);
    }
    drag->setHotSpot(QPoint(pix.width() / 2, pix.height() / 2));

	_dragFromHereInProgress = true;

			// only action that makes sense when started for this listview is the move action
			// but from other sources the Copy and Link actions are also valid
	drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::MoveAction);
}

/*=============================================================
 * TASK:	 sent when dragging is in progress
 * EXPECTS: either a list of file names or a mime data of type 
 *			thumbs
 * GLOBALS:
 * RETURNS:	none
 * REMARKS: follewd immediately by a dragMoveEvent()
 *------------------------------------------------------------*/
void ThumbnailWidget::dragEnterEvent(QDragEnterEvent * event)
{
	if (_IsAllowedToDrop(event->mimeData()) )
	{
// DEBUG
#if 0
		Qt::DropAction action = event->proposedAction();
		static QLabel *pLabel = nullptr;
		if (pLabel)
		{
			delete pLabel;
			pLabel = nullptr;
		}
		QString qs = "Accepted actions are: ";
		if(action == Qt::IgnoreAction)	  qs += " IgnoreAction";
		if((action & Qt::CopyAction) == Qt::CopyAction)	qs += " CopyAction";
		if((action & Qt::MoveAction) == Qt::MoveAction)	qs += " MoveAction";
		if((action & Qt::LinkAction) == Qt::LinkAction)	qs += " LinkAction";
		if((action & Qt::TargetMoveAction) == Qt::TargetMoveAction) qs += " TargetMoveAction";
		
		pLabel = new QLabel(qs, this, Qt::ToolTip);
		pLabel->setVisible(true);
#endif
		event->acceptProposedAction();
	}
}

/*=============================================================
 * TASK:	signals when dareg leaves the area
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
//	dynamic_cast<ThumbnailWidgetModel *>(model())->SetDummyPos(-1);
//	QListView::dragLeaveEvent(event);
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:QDragMoveEvent is a descendant of QDropEvent
 *------------------------------------------------------------*/
void ThumbnailWidget::dragMoveEvent(QDragMoveEvent * event)
{
	if (!_IsAllowedToDrop(event->mimeData()))
		return;

	QModelIndex index = indexAt(event->pos());
// DEBUG
#if 0
	int row = index.row();
//	dynamic_cast<ThumbnailWidgetModel *>(model())->SetDummyPos(row);
	static QLabel *pLabel = nullptr;
	if (pLabel)
	{
		delete pLabel;
		pLabel = nullptr;
	}
	QString qs = QString("mouse: %1,%2, row:%3, col:%4").arg(event->pos().x()).arg(event->pos().y()).arg(row).arg(index.column());
//	QString qs = QString("mouse: %1,%2, row:%3, col:%4, dummy:%5").arg(event->pos().x()).arg(event->pos().y()).arg(row).arg(index.column()).arg(dynamic_cast<ThumbnailWidgetModel *>(model())->DummyPosition());
	pLabel = new QLabel(qs, this, Qt::ToolTip);
	pLabel->setVisible(true);
#endif
// /DEBUG
	//if (row >= 0)
	//{
	//	if (event->pos().y() < 10)
	//	{
	//		index =
	//			scrollTo(index);
	//	}
	//}
	event->accept();
	// get drop position
//	QModelIndex index = indexAt(event->pos());
	// add temporary list element to correct position
	//...
	// call original ?
//	QListView::dragMoveEvent(event);
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::dropEvent(QDropEvent * event)
{
	if (!_IsAllowedToDrop(event->mimeData()))
		return;

	const ThumbMimeData *mimeData = qobject_cast<const ThumbMimeData *>(event->mimeData());
	if (mimeData->thumbList.isEmpty())
		return;

	// get drop position
	QModelIndex index = indexAt(event->pos());
	if (_dragFromHereInProgress)	// then remove from old spot
	{
	// 
	}
	// put files at new spot

}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::abort()
{
    _isAbortThumbsLoading = true;
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::loadVisibleThumbs(int scrollBarValue)
{
    static int lastScrollBarValue = 0;

    _scrolledForward = (scrollBarValue >= lastScrollBarValue);

    lastScrollBarValue = scrollBarValue;

	emit SignalInProcessing(true);

    for (;;) {
        int firstVisible = _getFirstVisibleThumb();
        int lastVisible  = _getLastVisibleThumb();
        if (_isAbortThumbsLoading || firstVisible < 0 || lastVisible < 0) {
            return;
        }

        if (_scrolledForward) 
		{
			lastVisible += ((lastVisible - firstVisible) * 3); // (Settings::thumbsPagesReadCount + 1));
            if (lastVisible >= _thumbnailWidgetModel->rowCount()) 
                lastVisible = _thumbnailWidgetModel->rowCount() - 1;
            
        } 
		else 
		{
            firstVisible -= (lastVisible - firstVisible) * 3;  // (Settings::thumbsPagesReadCount + 1);
            if (firstVisible < 0) 
                firstVisible = 0;

            lastVisible += 10;
            if (lastVisible >= _thumbnailWidgetModel->rowCount()) 
                lastVisible = _thumbnailWidgetModel->rowCount() - 1;
        }

        if (_thumbsRangeFirst == firstVisible && _thumbsRangeLast == lastVisible) 
            return;

        _thumbsRangeFirst = firstVisible;
        _thumbsRangeLast = lastVisible;

        loadThumbsRange();
        if (_isAbortThumbsLoading) 
            break;
        
    }
	emit SignalInProcessing(false);
}

int ThumbnailWidget::_getFirstVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = 0; currThumb < _thumbnailWidgetModel->rowCount(); ++currThumb) 
	{
        idx = _thumbnailWidgetModel->indexFromItem(_thumbnailWidgetModel->item(currThumb));
        if (viewport()->rect().contains(QPoint(0, visualRect(idx).y() + visualRect(idx).height() + 1))) 
            return idx.row();
        
    }

    return -1;
}

int ThumbnailWidget::_getLastVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = _thumbnailWidgetModel->rowCount() - 1; currThumb >= 0; --currThumb) 
	{
        idx = _thumbnailWidgetModel->indexFromItem(_thumbnailWidgetModel->item(currThumb));
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
    
    _updateThumbsCount();

/*
	if (thumbNames.size() && selectionModel()->selectedIndexes().size() == 0) 
        selectThumbByRow(0);
*/
	_isBusy = false;
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
	emit SignalInProcessing(false);
}

void ThumbnailWidget::reLoad() 
{
    _isBusy = true;
    loadPrepare();

    if (thumbNames.size()) 
	{
        loadFileList();
        return;
    }

    _initThumbs();
    _updateThumbsCount();
    loadVisibleThumbs();

    _isBusy = false;
}

void ThumbnailWidget::loadPrepare() 
{

    _thumbnailWidgetModel->clear();
    setIconSize(QSize(_thumbSize, _thumbSize));
    setSpacing(QFontMetrics(font()).height());

    if (_isNeedToScroll) {
        scrollToTop();
    }

    _isAbortThumbsLoading = false;

    _thumbsRangeFirst = -1;
    _thumbsRangeLast = -1;
}

void ThumbnailWidget::_initThumbs()
{
	static QStandardItem *thumbItem;
	static int fileIndex;
	static QPixmap emptyPixMap;
	static QSize hintSize;
	int thumbsAddedCounter = 1;

	emptyPixMap = QPixmap::fromImage(_emptyImg).scaled(_thumbSize, _thumbSize);
	hintSize = QSize(_thumbSize, _thumbSize + ((int)(QFontMetrics(font()).height() * 1.5)));

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

		_thumbnailWidgetModel->appendRow(thumbItem);

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

/*=============================================================
 * TASK:	emit a signal tha image count changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::_updateThumbsCount()
{
    if (_thumbnailWidgetModel->rowCount() > 0) 
        statusStr = tr("%n image(s)", "", _thumbnailWidgetModel->rowCount());
    else 
		statusStr = tr("No images");
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
}

/*=============================================================
 * TASK:	signal if suitable mime data to drop is available
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool ThumbnailWidget::_IsAllowedToDrop(const QMimeData *mimeData)
{
 return mimeData->hasUrls() ||
		mimeData->hasFormat("application/x-thumb");
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
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
    static int _currentRowCount;
    static QString imageFileName;
    QImage thumb;
    int currThumb;
    bool imageReadOk;

    if (isInProgress) 
	{
        _isAbortThumbsLoading = true;
        QTimer::singleShot(0, this, SLOT(loadThumbsRange()));
        return;
    }

    isInProgress = true;
    _currentRowCount = _thumbnailWidgetModel->rowCount();

    for (_scrolledForward ? currThumb = _thumbsRangeFirst : currThumb = _thumbsRangeLast;
         (_scrolledForward ? currThumb <= _thumbsRangeLast : currThumb >= _thumbsRangeFirst);
         _scrolledForward ? ++currThumb : --currThumb) 
	{

        if (_isAbortThumbsLoading || _thumbnailWidgetModel->rowCount() != _currentRowCount || currThumb < 0)
            break;

        if (_thumbnailWidgetModel->item(currThumb)->data(LoadedRole).toBool())
            continue;

        imageFileName = _thumbnailWidgetModel->item(currThumb)->data(FileNameRole).toString();
        thumbReader.setFileName(imageFileName);
        currentThumbSize = thumbReader.size();
        imageReadOk = false;

        if (currentThumbSize.isValid()) 
		{
            if (currentThumbSize.width() > _thumbSize || currentThumbSize.height() > _thumbSize) 
                currentThumbSize.scale(QSize(_thumbSize, _thumbSize), Qt::KeepAspectRatio);
        
            thumbReader.setScaledSize(currentThumbSize);
            imageReadOk = thumbReader.read(&thumb);
        }

        if (imageReadOk) 
		
            _thumbnailWidgetModel->item(currThumb)->setIcon(QPixmap::fromImage(thumb));
         else 
		 {
            _thumbnailWidgetModel->item(currThumb)->setIcon(QIcon::fromTheme("image-missing",
                                                              QIcon(":/images/error_image.png")).pixmap(
															           BAD_IMAGE_SIZE, BAD_IMAGE_SIZE));
            currentThumbSize.setHeight(BAD_IMAGE_SIZE);
            currentThumbSize.setWidth(BAD_IMAGE_SIZE);
        }

        _thumbnailWidgetModel->item(currThumb)->setData(true, LoadedRole);
        QApplication::processEvents();
    }

    isInProgress = false;
    _isAbortThumbsLoading = false;
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

    hintSize = QSize(_thumbSize, _thumbSize + ((int) (QFontMetrics(font()).height() * 1.5)));

	QString imageFullPath = thumbsDir + thumbNames[which];
//    thumbFileInfo = QFileInfo(imageFullPath);
    thumbItem->setData(true, LoadedRole);
    thumbItem->setData(0, SortRole);
    thumbItem->setData(thumbsDir, FilePathRole);
	thumbItem->setData(thumbNames[which], FileNameRole);
	thumbItem->setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
    thumbItem->setData(thumbNames[which], Qt::DisplayRole);
    thumbItem->setData(originalPaths[which], Qt::ToolTipRole);
    thumbItem->setSizeHint(hintSize);

    thumbReader.setFileName(imageFullPath);
    currThumbSize = thumbReader.size();
    if (currThumbSize.isValid()) 
	{
        if (currThumbSize.width() > _thumbSize || currThumbSize.height() > _thumbSize) 
            currThumbSize.scale(QSize(_thumbSize, _thumbSize), Qt::KeepAspectRatio);

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

    _thumbnailWidgetModel->appendRow(thumbItem);
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
    if (event->angleDelta().y() < 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + _thumbSize);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - _thumbSize);
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
 * TASK:	event to scroll view when a drag and drop is in 
 *			progress and the cursor is at the top of the view area
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::mouseMoveEvent(QMouseEvent * event)
{

	QListView::mouseMoveEvent(event);	// original handler
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


	pact = new QAction(tr("Add..."), this);
	pact->setEnabled(true);
	connect(pact, &QAction::triggered, this, &ThumbnailWidget::AddImages);
	menu.addAction(pact);
    menu.addSeparator();

	pact = new QAction(tr("Set As Album Thumbnail"), this);
	bool b = selectionModel()->selectedIndexes().size() == 1;
	pact->setEnabled(b);
	if(b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::SetAsAlbumThhumbnail);
	menu.addAction(pact);
	
	b = selectionModel()->selectedIndexes().size();
	pact = new QAction(tr("&Delete"), this);
	pact->setEnabled(b);
	if (b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::DeleteSelected);
	menu.addAction(pact);

	pact = new QAction(tr("&Undo Delete"), this);
	bool undo = _pImages && !_pImages->NothingToUndo();
	pact->setEnabled(undo);
	if (undo)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::UndoDelete);
	menu.addAction(pact);

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
    QModelIndex firstIndex = _thumbnailWidgetModel->index(0, 0);
    QModelIndex lastIndex = _thumbnailWidgetModel->index(_thumbnailWidgetModel->rowCount() - 1, 0);
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
    this->_isNeedToScroll = needToScroll;
}
