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
#include <QIcon>
#include "config.h"


// ****************** ThumbnailItem ******************
#ifndef _USE_QSTANDARDITEM 

int ThumbnailItem::_thumbHeight;

ThumbnailItem::ThumbnailItem(int pos,  ID_t albumID, Type typ) : QStandardItem(pos, 1), _itemType(typ), _albumId(albumID), itemPos(pos)
{
    QSize hintSize = QSize(_thumbHeight, _thumbHeight + ((int)(QFontMetrics(font()).height() * 1.5)));
    setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
}

QIcon ThumbnailItem::IconForFile() const
{
    Album* pAlbum = _ActAlbum();
    ID_t itemId = pAlbum->items[itemPos];
    bool exists = false;
    bool bIsFolderIcon = false;
    //bool isImageOrVideo = itemId & (IMAGE_ID_FLAG | VIDEO_ID_FLAG);
    bool isFolder = itemId & ALBUM_ID_FLAG;

    ID_t imgId = itemId;
    if (isFolder)
    {
        Album* parent = _ParentAlbum();
        imgId = albumgen.Albums()[itemId].thumbnail;
        bIsFolderIcon = (parent && parent->thumbnail == imgId);
    }
    else //    if (isImageOrVideo)
        bIsFolderIcon = (itemId == pAlbum->thumbnail);

//    imgId &= ID_MASK;
    Image &img = albumgen.Images()[imgId];
    QString imageName;
    if ((exists=QFile::exists(FilePath() + img.LinkName())))
        imageName = FilePath() + img.LinkName();
    else if ((exists=QFile::exists(img.FullSourceName())))
        imageName = img.FullSourceName();
    else
        imageName = QString(":/Preview/Resources/NoImage.jpg");
    
    // DEBUG
    // qDebug((QString("icon for file:'%1' of ID:%2, name:'%3'").arg(imageName).arg(itemId).arg(img.FullSourceName())).toStdString().c_str());
    // /DEBUG
    ImageMarker imageMarker(imageName, 300, bIsFolderIcon, exists, isFolder);
    return imageMarker.Read() ? imageMarker.ToIcon() : QIcon();
}


QVariant ThumbnailItem::data(int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:   return FileName();              // destination (generated) file or directory name w.o. path
        case Qt::ToolTipRole:   return ToolTip();               // string of full source name and destination name
        case Qt::DecorationRole: return IconForFile();          // the icon of files or albums (used when QListView is set for it)

        case TypeRole:          return _itemType;               // folder, image or video
        case FilePathRole:      return FilePath();              // destination path
        case FileNameRole:      return FileName();              // destination (generated) file or directory name
        case SourcePathNameRole: return FullSourcePath();		// full path name of image source
        case FullNameRole:      return FilePath() + FileName(); // full destination path name
    }
    return QVariant();
}

QString ThumbnailItem::_ImageToolTip() const    // only called for images
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->items[itemPos] );
    return QString(pImg->FullSourceName() + " \n(" + pImg->LinkName(config.bLowerCaseImageExtensions) + ")");

}
QString ThumbnailItem::_VideoToolTip() const    // only called for videos
{
    Video* pVid = albumgen.VideoAt(_ActAlbum()->items[itemPos]);
    return QString(pVid->FullSourceName() + " \n(" + pVid->LinkName(config.bLowerCaseImageExtensions) + ")");
}
QString ThumbnailItem::_FolderToolTip() const   // only called for folders
{
    Album* pAlbum = albumgen.AlbumForID(_ActAlbum()->items[itemPos]);

    QString s = pAlbum->FullSourceName() + " \n(" + pAlbum->LinkName(albumgen.ActLanguage(), true) + ")";
    return s;
}

QString ThumbnailItem::ToolTip() const
{
    switch (_itemType)
    {
        case image: return _ImageToolTip();
        case video: return _VideoToolTip();
        case folder: return _FolderToolTip();
        default: break;
    }
    return QString();
}

QString ThumbnailItem::_ImageFilePath() const
{
    return QString((config.dsGallery + config.dsGRoot + config.dsThumbDir).ToString());

}
QString ThumbnailItem::_VideoFilePath() const
{
    return QString((config.dsGallery + config.dsGRoot + config.dsVideoDir).ToString());

}
QString ThumbnailItem::_FolderFilePath() const
{
    return QString((config.dsGallery + config.dsGRoot + config.dsThumbDir).ToString());

}
/*=============================================================
 * TASK:    these functions return the generated file name w.o. path
 * PARAMS:
 * GLOBALS: 'itemPos' absolute index of the item
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QString ThumbnailItem::_ImageFileName() const
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->items[itemPos]);
	return QString(pImg->LinkName(config.bLowerCaseImageExtensions));

}
QString ThumbnailItem::_VideoFileName() const
{
    Video* pVid = albumgen.VideoAt(_ActAlbum()->items[itemPos]);
    return QString(pVid->LinkName(config.bLowerCaseImageExtensions));
}
QString ThumbnailItem::_FolderFileName() const
{
    Album album = albumgen.Albums()[_albumId];
    Album* pAlbum = albumgen.AlbumForID(album.IdOfItemOfType(ALBUM_ID_FLAG, itemPos));
    Image* pImg = albumgen.ImageAt(pAlbum->thumbnail);
    return QString(pImg->LinkName(config.bLowerCaseImageExtensions));
}

QString ThumbnailItem::FilePath() const
{
    switch (_itemType)
    {
        case image: return _ImageFilePath();
        case video: return _VideoFilePath();
        case folder: return _FolderFilePath();
        default: break;
    }
    return QString();
}

QString ThumbnailItem::FileName() const
{
    switch (_itemType)
    {
        case image: return _ImageFileName();
        case video: return _VideoFileName();
        case folder: return _FolderFileName();
        default: break;
    }
    return QString();
}

QString ThumbnailItem::_ImageFullSourceName() const
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->IdOfItemOfType(IMAGE_ID_FLAG, itemPos));
    return pImg->FullSourceName();
}
QString ThumbnailItem::_VideoFullSourceName() const
{
    Video* pVid = albumgen.VideoAt(_ActAlbum()->IdOfItemOfType(VIDEO_ID_FLAG, itemPos));
    return pVid->FullSourceName();
}
QString ThumbnailItem::_FolderFullSourceName() const
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->thumbnail);
    return pImg->FullSourceName();
}

QString ThumbnailItem::text()  const
{
    return FullSourcePath();
}

QString ThumbnailItem::FullSourcePath() const
{
    switch (_itemType)
    {
        case image: return   _ImageFullSourceName();
        case video: return   _VideoFullSourceName();
        case folder: return  _FolderFullSourceName();
        default: break;
    }
    return QString();
}

#endif // #ifndef _USE_QSTANDARDITEM 

// ****************** ThumbnailWidget ******************

 /*=============================================================
  * TASK:		constructor
  * EXPECTS:	parent: widget to contain his
  *				thumbsize - pixel width and height of thumbnails
  * GLOBALS:
  * RETURNS:
  * REMARKS:	SA
  *				- connects slots and sets up scrollbar
  *------------------------------------------------------------*/
ThumbnailWidget::ThumbnailWidget(QWidget *parent, int thumbheight) : QListView(parent), _thumbHeight(thumbheight)
{
    ThumbnailItem::SetThumbHeight(thumbheight);

    // prepare spacer for Drag & Drop into tnvImages
    float spacerWidth = 50; // pixel
    _insertPosImage = QImage(spacerWidth, _thumbHeight, QImage::Format_ARGB32);
    QPainter *painter = new QPainter(&_insertPosImage);
    _insertPosImage.fill(Qt::transparent);
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setColor(schemes[PROGRAM_CONFIG::schemeIndex].sSpacerColor);
    pen.setWidth(10);
    painter->setPen(pen);
    painter->drawLine(spacerWidth / 2.0, 0, spacerWidth / 2.0, _thumbHeight);
    painter->drawLine(0, 0, spacerWidth, 0);
    painter->drawLine(0, _thumbHeight, spacerWidth, _thumbHeight);
    delete painter;

    _currentItem = 0;

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

/*=============================================================
 * TASK:	move '_currentItem' to next thumbnail
 * EXPECTS:
 * GLOBALS:
 * RETURNS:	new row or -1 when no more rows
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getNextItem()
{
    if (_currentItem == _thumbnailWidgetModel->rowCount() - 1) 
        return -1;

    return _currentItem + 1;
}

/*=============================================================
* TASK:	move '_currentItem' to previous thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS:	new row or -1 when no previous row
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailWidget::getPrevItem()
{
    if (_currentItem == 0) 
        return -1;

    return _currentItem - 1;
}

/*=============================================================
 * TASK:	gets row of last thumbnail
 * EXPECTS:
 * GLOBALS:
 * RETURNS: image count -1
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailWidget::getLastItem()
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
int ThumbnailWidget::getRandomItem()
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
int ThumbnailWidget::getCurrentItem()
{
    return _currentItem;
}

void ThumbnailWidget::setCurrentItem(int index) 
{
    if (index >= 0) 
        _currentItem = index;
    else
        _currentItem = 0;
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
    title = _thumbnailWidgetModel->item(_currentItem)->data(Qt::DisplayRole).toString()
                    + " - ["
                    + QString::number(_currentItem + 1)
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
        setCurrentItem(_currentIndex.row());
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
bool ThumbnailWidget::setCurrentIndexByItem(int row)
{
	QModelIndex idx = _thumbnailWidgetModel->indexFromItem(_thumbnailWidgetModel->item(row));
	if (idx.isValid()) 
	{
		_currentIndex = idx;
		setCurrentItem(idx.row());
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
		setCurrentItem(current_row);
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
        _UpdateThumbsCount();
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
	for (auto f : indexesList)
	{
	    ThumbnailItem thumbRec(f.row(), _albumId, (ThumbnailItem::Type)_thumbnailWidgetModel->item(f.row())->type() );
		urls << QUrl(thumbRec.FileName());
		thumbRec.itemPos = f.row();
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

QString __DropActionToStr(QDragMoveEvent *event)    // QDragEnterEvent inherits this
{
	const Qt::DropAction action = event->proposedAction();
    QString qs = "Accepted actions are: ";
    if (action == Qt::IgnoreAction)	 qs += " IgnoreAction";
    if ((action & Qt::CopyAction) == Qt::CopyAction)	qs += " CopyAction";
    if ((action & Qt::MoveAction) == Qt::MoveAction)	qs += " MoveAction";
    if ((action & Qt::LinkAction) == Qt::LinkAction)	qs += " LinkAction";
    if ((action & Qt::TargetMoveAction) == Qt::TargetMoveAction) qs += " TargetMoveAction";

    auto MimeDataType = [&]() {
        const QMimeData *pd= event->mimeData();
        QString s;
        if (pd->hasText()) s += " text";
        if (pd->hasHtml()) s += " HTML";
        if (pd->hasUrls()) s += " URLs";
        if (pd->hasImage()) s += " image";
        if (pd->hasColor()) s += " color";
        if (pd->hasFormat("application/x-thumb")) s += " x-thumb";

        return s;
    };
    qs += QString("\nAt position(%1,%2) with").arg(event->pos().x()).arg(event->pos().y()) + MimeDataType();

    return qs;
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
	if (_IsAllowedToDrop(event) )
	{
// DEBUG
#if 1
// DEBUG
        if (pDragDropLabel)
		{
			delete pDragDropLabel;
            pDragDropLabel = nullptr;
		}
// /DEBUG
// DEBUG
        QString qs = __DropActionToStr(event);
        pDragDropLabel = new QLabel(qs, this, Qt::ToolTip);
		pDragDropLabel->setVisible(true);
// /DEBUG
#endif
		event->acceptProposedAction();
	}
}

/*=============================================================
 * TASK:	signals when drag leaves the area
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
// DEBUG
    if (pDragDropLabel)
    {
        delete pDragDropLabel;
        pDragDropLabel = nullptr;
    }
// /DEBUG
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
	if (!_IsAllowedToDrop(event))
		return;

	QModelIndex index = indexAt(event->pos());
// DEBUG
#if 1
//	dynamic_cast<ThumbnailWidgetModel *>(model())->SetDummyPos(row);
// DEBUG
    if (pDragDropLabel)
	{
		delete pDragDropLabel;
        pDragDropLabel = nullptr;
	}
// /DEBUG
// 
	int row = index.row();
//	QString qs = QString("mouse: (%1,%2), Item:%3").arg(event->pos().x()).arg(event->pos().y()).arg(row);
//	QString qs = QString("mouse: %1,%2, row:%3, col:%4, dummy:%5").arg(event->pos().x()).arg(event->pos().y()).arg(row).arg(index.column()).arg(dynamic_cast<ThumbnailWidgetModel *>(model())->DummyPosition());
// DEBUG
    QString qs = __DropActionToStr(event) + QString(" r:%1").arg(row);
    pDragDropLabel = new QLabel(qs, this, Qt::ToolTip);
    pDragDropLabel->setVisible(true);
    // /DEBUG
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
	if (!_IsAllowedToDrop(event))
		return;

	const ThumbMimeData *mimeData = qobject_cast<const ThumbMimeData *>(event->mimeData());
	if (mimeData->thumbList.isEmpty())
		return;
// DEBUG
    if (pDragDropLabel)
        delete pDragDropLabel;
    pDragDropLabel = nullptr;
// /DEBUG
	// get drop position
	QModelIndex index = indexAt(event->pos());
	if (_dragFromHereInProgress)	// then remove from old spot
	{

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
#if 0
    static int lastScrollBarValue = 0;

    _scrolledForward = (scrollBarValue >= lastScrollBarValue);

    lastScrollBarValue = scrollBarValue;

	emit SignalInProcessing(true);

    for (;;) {
        int firstVisible = _GetFirstVisibleThumb();
        int lastVisible  = _GetLastVisibleThumb();
        if (_isAbortThumbsLoading || firstVisible < 0 || lastVisible < 0) {
            goto FINISHED;
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
            goto FINISHED;

        _thumbsRangeFirst = firstVisible;
        _thumbsRangeLast = lastVisible;

        loadThumbsRange();
        if (_isAbortThumbsLoading) 
            break;
        
    }
  FINISHED:
#endif
	emit SignalInProcessing(false);
}

int ThumbnailWidget::_GetFirstVisibleThumb() 
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

int ThumbnailWidget::_GetLastVisibleThumb() 
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
    for (int i = 0; i < model()->rowCount(); i++) 
        addThumb(i, (ThumbnailItem::Type)model()->data(model()->index(i,0), TypeRole).toInt());
    
    _UpdateThumbsCount();

	_isBusy = false;
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
	emit SignalInProcessing(false);
}

void ThumbnailWidget::reLoad() 
{
    _isBusy = true;
    loadPrepare();

/*      never get inside as loadPrepare clears model
    if (model()->rowCount())
	{
        loadFileList();
        return;
    }
*/

     _InitThumbs();
    _UpdateThumbsCount();
    loadVisibleThumbs(0);

    _isBusy = false;
}

void ThumbnailWidget::Setup(ID_t aid)
{
    _albumId = aid;
//    _InitThumbs();
}

void ThumbnailWidget::loadPrepare()
{

    _thumbnailWidgetModel->clear();
    setIconSize(QSize(_thumbHeight, _thumbHeight));
    setSpacing(QFontMetrics(font()).height());

    if (_isNeedToScroll) {
        scrollToTop();
    }

    _isAbortThumbsLoading = false;

    _thumbsRangeFirst = -1;
    _thumbsRangeLast = -1;
}

void ThumbnailWidget::_InitThumbs()
{
	static QStandardItem *thumbItem;
	static int fileIndex;
	static QSize hintSize;
	int thumbsAddedCounter = 1;
    Album &album = albumgen.Albums()[_albumId];

    // Add images
	for (fileIndex = 0; fileIndex < album.items.size(); ++fileIndex)
	{
	    thumbItem = new ThumbnailItem(fileIndex, album.ID, _TypeFor(album.items[fileIndex]));
		_thumbnailWidgetModel->appendRow(thumbItem);
		++thumbsAddedCounter;
		if (thumbsAddedCounter > 100)
		{
			thumbsAddedCounter = 1;
			QApplication::processEvents();
		}
	}

	if (model()->rowCount() && selectionModel()->selectedIndexes().size() == 0)
		selectThumbByItem(0);

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
void ThumbnailWidget::_UpdateThumbsCount()
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
bool ThumbnailWidget::_IsAllowedToDrop(const QDropEvent *event)
{
     return (indexAt(event->pos()).row() >= 0) && 
            (event->mimeData()->hasUrls() ||
		    event->mimeData()->hasFormat("application/x-thumb"));
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::selectThumbByItem(int row)
{
    setCurrentIndexByItem(row);
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
    static int _currentItemCount;
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
    _currentItemCount = _thumbnailWidgetModel->rowCount();

    for (_scrolledForward ? currThumb = _thumbsRangeFirst : currThumb = _thumbsRangeLast;
         (_scrolledForward ? currThumb <= _thumbsRangeLast : currThumb >= _thumbsRangeFirst);
         _scrolledForward ? ++currThumb : --currThumb) 
	{

        if (_isAbortThumbsLoading || _thumbnailWidgetModel->rowCount() != _currentItemCount || currThumb < 0)
            break;

        if (_thumbnailWidgetModel->item(currThumb)->data(LoadedRole).toBool())
            continue;

        imageFileName = _thumbnailWidgetModel->item(currThumb)->data(FileNameRole).toString();
        thumbReader.setFileName(imageFileName);
        currentThumbSize = thumbReader.size();
        imageReadOk = false;

        if (currentThumbSize.isValid()) 
		{
            if (currentThumbSize.width() > _thumbHeight || currentThumbSize.height() > _thumbHeight) 
                currentThumbSize.scale(QSize(_thumbHeight, _thumbHeight), Qt::KeepAspectRatio);
        
            thumbReader.setScaledSize(currentThumbSize);
            imageReadOk = thumbReader.read(&thumb);
        }

        if (imageReadOk) 
		
            _thumbnailWidgetModel->item(currThumb)->setIcon(QPixmap::fromImage(thumb));
         else 
		 {
            _thumbnailWidgetModel->item(currThumb)->setIcon(QIcon::fromTheme("image-missing",
                                                              QIcon(":/Preview/Resources/NoImage.jpg")).pixmap(
                                                                  _thumbHeight, _thumbHeight));
															           //BAD_IMAGE_SIZE, BAD_IMAGE_SIZE));
            //currentThumbSize.setHeight(BAD_IMAGE_SIZE);
            //currentThumbSize.setWidth(BAD_IMAGE_SIZE);
        }

        _thumbnailWidgetModel->item(currThumb)->setData(true, LoadedRole);
        QApplication::processEvents();
    }

    isInProgress = false;
    _isAbortThumbsLoading = false;
}

/*=============================================================
 * TASK:	adds a ThumbnailItem for a thumbnail to the end of the 
 *			rows
 * EXPECTS:	_albumID already set
 *          which - ordinal of data for thumbnail in
 *					albumgen.Albums()[_albumID]'s 
 *                  image, video or album lists
 *          typ   - which list should be used
 * GLOBALS: _albumID set
 * RETURNS:	nothing
 * REMARKS:	- sets 
 *------------------------------------------------------------*/
void ThumbnailWidget::addThumb(int which, ThumbnailItem::Type type)
{
    ThumbnailItem *thumbItem = new ThumbnailItem(which, _albumId, type);
    QImageReader thumbReader;
    QSize currThumbSize;

    thumbReader.setFileName(reinterpret_cast<ThumbnailWidgetModel*>(model())->item(which,0)->text());
    currThumbSize = thumbReader.size();
    if (currThumbSize.isValid()) 
	{
        if (currThumbSize.width() > _thumbHeight || currThumbSize.height() > _thumbHeight) 
            currThumbSize.scale(QSize(_thumbHeight, _thumbHeight), Qt::KeepAspectRatio);

        thumbReader.setScaledSize(currThumbSize);
        QImage thumb = thumbReader.read();

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


/*========================================================
 * TASK:
 * PARAMS:  here - insert space before this item index (row)
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void ThumbnailWidget::SetInsertPos(int here)
{
    _insertPos = here;
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
        verticalScrollBar()->setValue(verticalScrollBar()->value() + _thumbHeight);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - _thumbHeight);
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
 * TASK: Event handler for right floating popup menu
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::contextMenuEvent(QContextMenuEvent * pevent)
{
	QMenu menu(this);
	QAction *pact;

	pact = new QAction(tr("Set As Album &Thumbnail"), this);
	bool b = selectionModel()->selectedIndexes().size() == 1;
	pact->setEnabled(b);
	if(b)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::SetAsAlbumThumbnail);
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
    
    menu.addSeparator();

	pact = new QAction(tr("Add &Images..."), this);  // any number of images from a directory
	pact->setEnabled(true);
	connect(pact, &QAction::triggered, this, &ThumbnailWidget::AddImages);
	menu.addAction(pact);

	pact = new QAction(tr("Add &Folder..."), this);  // one folder added to the folder tree inside this album
	pact->setEnabled(true);                          // the album need not be physical
	connect(pact, &QAction::triggered, this, &ThumbnailWidget::AddFolder);
	menu.addAction(pact);

	menu.addSeparator();

    b = selectionModel()->selectedIndexes().size();
    pact = new QAction(tr("&Remove"), this);
    pact->setEnabled(b);
    if (b)
        connect(pact, &QAction::triggered, this, &ThumbnailWidget::DeleteSelected);
    menu.addAction(pact);

#if 0
    // how to make it work?
    pact = new QAction(tr("&Undo Delete"), this);
#define CAN_UNDO(a) (a && !a->NothingToUndo())
	bool undo = model()->rowCount() && (CAN_UNDO() || CAN_UNDO);
	pact->setEnabled(undo);
	if (undo)
		connect(pact, &QAction::triggered, this, &ThumbnailWidget::UndoDelete);
	menu.addAction(pact);
#endif

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
    if (list.isEmpty())
        return;

    QString plurali = tr("s"), plurala = tr("s");   // plural for image and album. May differ in other languages
    QString qs = QMainWindow::tr("Do you want to delete selected image%1 / album%1 from disk, or just to remove them from gallery?").arg(list.size() > 1 ? plurali :"").arg(list.size() > 1 ? plurala : "");
    QMessageBox msg;
    msg.setWindowTitle(tr("falconG - Question"));
    msg.setText(qs);
    msg.addButton(tr("Delete"),QMessageBox::YesRole);
    msg.addButton(tr("Remove"),QMessageBox::NoRole);
    msg.addButton(tr("Cancel"),QMessageBox::RejectRole);
    msg.setDefaultButton(QMessageBox::Cancel);
    msg.setIcon(QMessageBox::Question);
    int res = msg.exec();

    if (res == QMessageBox::Cancel)
        return;

    Album &album = albumgen.Albums()[_albumId];
    int ix;
    for (auto mi : list)
    {
        ix = mi.row();
        album.items.remove(ix);
    }
    reLoad();

    emit selectionChanged(QItemSelection(), QItemSelection());
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
 * REMARKS: connected to popup menu
 *------------------------------------------------------------*/
void ThumbnailWidget::AddImages()
{
    QString dir = config.dsLastImageDir.ToString();
    QStringList qslFileNames = QFileDialog::getOpenFileNames(this, tr("flaconG - Add image"), dir, "Images(*.bmp *.jpg *.png *.tif);;Videos(*.mp4,*.ogg);;All files(*.*)");
    if (qslFileNames.isEmpty())
        return;
    // get all file names into  _imageMap or _videoMap,
    // plus into the actual album into albumgen's _albumMap
    // plus into _thumbnailWidgetModel
    // and display it
    int pos = currentIndex().row();
    bool res = true;

    int i;
    for (i=0; i < qslFileNames.size(); ++i)
    {
        res &= albumgen.AddImageOrVideoFromString(qslFileNames[i], *_ActAlbum(), pos);
        if (!res)
            break;
    }
    reLoad();

    emit selectionChanged(QItemSelection(), QItemSelection());
    config.dsLastImageDir = dir;

}

/*=============================================================
 * TASK:   Display dialog box to add one folder into this 
 *          album
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailWidget::AddFolder()
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
		s += selectionModel()->model()->data(i, SourcePathNameRole).toString() + "\n";
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->clear();
	clipboard->setText(s);
}

/*============================================================================
  * TASK:	signal set current file name as album thumbnail
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: only called when there is exactly one selected image
 *--------------------------------------------------------------------------*/
void ThumbnailWidget::SetAsAlbumThumbnail()
{
    int pos = currentIndex().row();     // new thumbnail index

    Album& album = albumgen.Albums()[_albumId];
    int cthix = -1;// old thumbnail index in 'items'. -1: not from current album
    for (int i = 0; i < album.items.size(); ++i)
        if (album.thumbnail)
            cthix = i;
    if (cthix == pos)       // no change
        return;

    ID_t th = album.items[pos];

    album.thumbnail = th & ALBUM_ID_FLAG ? albumgen.Albums()[th].thumbnail : albumgen.ImageAt(th)->ID;

    reLoad();
}

void ThumbnailWidget::setNeedToScroll(bool needToScroll) 
{
    this->_isNeedToScroll = needToScroll;
}
