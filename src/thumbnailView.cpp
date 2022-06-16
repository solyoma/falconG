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

#include "thumbnailView.h"
#include <QIcon>
#include "config.h"
#include "imageviewer.h"


// ****************** ThumbnailItem ******************
static class FileIcons
{

    QVector<MarkedIcon> _iconList;   // all icons for actual album,  
                                     // (not QLIst as in Qt6 QList is the same as QVector)
                                     // order never changes when items added or moved around
                                     // access elements through _iconOrder
    QVector<int> _iconOrder;         // indirection through this
public:
    int posFolderIcon = -1;

    void Clear()
    {
		posFolderIcon = -1;
		_iconList.clear();
        _iconOrder.clear();
        MarkedIcon::Init(); // only reads images if not yet initted
    }
    int Size() const 
    { 
        return _iconList.size(); 
    }
    void SetMaximumSizes(int thumbsize=THUMBNAIL_SIZE, int borderwidth = 10)
    {
        MarkedIcon::SetMaximumSizes(thumbsize, borderwidth);
    }

    bool HasIconFor(int pos)
    {
        return pos < _iconList.size();
    }

    void SetFolderThumbnailPosition(int pos, bool bIsFolderThumbnail = false)
    {
        // pos-th item MUST exist
        MarkedIcon& micon = _iconList[ _iconOrder[pos] ];
		if (posFolderIcon >= 0)         // erase original folder icon
			micon.isFolderThumb = false;

		micon.isFolderThumb = bIsFolderThumbnail;    // and set this
		if (bIsFolderThumbnail)
			posFolderIcon = pos;
    }

	QIcon IconForPosition(int pos, bool isFolder, bool isFolderThumb, QString imageName = QString())
	{

		if (pos < 0)
			return QIcon();

		if (pos >= _iconList.size())
            Insert(-1, isFolder, imageName);    

        // pos-th item MUST exist
        Q_ASSERT(pos < _iconOrder.size());

        MarkedIcon& micon = _iconList[_iconOrder[pos]];
        micon.isFolderThumb = isFolderThumb;
		return micon.ToIcon();
	}
    QVector<int> IconOrder() const
    {
        return _iconOrder;
    }
    void SetIconOrder(QVector<int>& order)
    {
        _iconOrder = order;
    }
    MarkedIcon * Insert(int pos, bool isFolder, QString imageName = QString())
    {
		MarkedIcon icon;
		icon.Read(imageName, isFolder);

		if (pos < 0)
            _iconOrder.push_back(_iconList.size());
        else
            _iconOrder.insert(pos, _iconList.size());

		_iconList.push_back(icon);
        return &_iconList[_iconList.size() - 1];
    }
    void Remove(int pos)    // remove items _iconOrder[pos]
    {
        if (pos < 0 || pos > _iconList.size())
            return;
        int removed = _iconOrder[pos];
        _iconList.removeAt(removed);
        _iconOrder.removeAt(pos);
        // all larger indexes must be decreased
        for (int i = 0; i < _iconOrder.size(); ++i)
            if (_iconOrder[i] > removed)
                --_iconOrder[i];
    }
} fileIcons;

int ThumbnailItem::thumbHeight=150;

ThumbnailItem::ThumbnailItem(int pos,  ID_t albumID, Type typ, QIcon icon) : QStandardItem(pos, 1), _itemType(typ), _albumId(albumID), itemPos(pos)
{
    QSize hintSize = QSize(thumbHeight, thumbHeight + ((int)(QFontMetrics(font()).height() * 1.5)));

    setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
}

QIcon ThumbnailItem::IconForFile() const
{
    QString imageName;

    Album* pAlbum = _ActAlbum();

    ID_t itemId = pAlbum->IdOfItem(itemPos);
    bool exists = false;
    
    bool isFolder = itemId & ALBUM_ID_FLAG;
    bool bIsFolderIcon = false;

    ID_t imgId = itemId;     // add marker for image that is the folder thumbnail (shown in parent)
//    Album* parent = _ParentAlbum();
    if (isFolder)
        imgId = albumgen.Albums()[itemId].thumbnail;

    bIsFolderIcon = (pAlbum->thumbnail == imgId);

    if (fileIcons.HasIconFor(itemPos))
        return fileIcons.IconForPosition(itemPos, isFolder, bIsFolderIcon);
            // else read and create icon


//    imgId &= ID_MASK;
    Image &img = albumgen.Images()[imgId];
    if ((exists=QFile::exists(img.FullLinkName())))
        imageName = img.FullLinkName();
    else if ((exists=QFile::exists(img.FullSourceName())))
        imageName = img.FullSourceName();
    else
        imageName = QString(":/Preview/Resources/NoImage.jpg");
    
    // DEBUG
    // qDebug((QString("icon for file:'%1' of ID:%2, name:'%3'").arg(imageName).arg(itemId).arg(img.FullSourceName())).toStdString().c_str());
    // /DEBUG
    return fileIcons.IconForPosition(itemPos, isFolder, bIsFolderIcon, imageName);
}


QVariant ThumbnailItem::data(int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:   return DisplayName();              // destination (generated) file or directory name w.o. path
        case Qt::ToolTipRole:   return ToolTip();               // string of full source name and destination name
        case Qt::DecorationRole: return IconForFile();          // the icon of files or albums (used when QListView is set for it)
        case TypeRole:          return _itemType;               // folder, image or video
        case FilePathRole:      return FilePath();              // destination path
        case FileNameRole:      return FileName();              // destination (generated) file or directory name w.o. path or source path if no dest. file
        case SourcePathNameRole: return FullSourcePath();		// full path name of image source
        case FullNameRole:      return FullLinkName(); // full destination path name
    }
    return QVariant();
}

QString ThumbnailItem::_ImageToolTip() const    // only called for images
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->IdOfItem(itemPos) );
    return QString(pImg->ShortSourcePathName() + " \n(" + pImg->LinkName(config.bLowerCaseImageExtensions) + ")");

}
QString ThumbnailItem::_VideoToolTip() const    // only called for videos
{
    Video* pVid = albumgen.VideoAt(_ActAlbum()->IdOfItem(itemPos));
    return QString(pVid->ShortSourcePathName() + " \n(" + pVid->LinkName(config.bLowerCaseImageExtensions) + ")");
}
QString ThumbnailItem::_FolderToolTip() const   // only called for folders
{
    Album* pAlbum = albumgen.AlbumForID(_ActAlbum()->IdOfItem(itemPos));

    QString s = pAlbum->ShortSourcePathName() + " \n(" + pAlbum->LinkName(-1, true) + ")";
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
 * TASK:    these functions return the generated file name 
 *          w.o. path if it exists or the source name if it is not
 * PARAMS:
 * GLOBALS: 'itemPos' absolute index of the item
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
QString ThumbnailItem::_ImageFileName() const
{
    Image* pImg = albumgen.ImageAt(_ActAlbum()->IdOfItem(itemPos));
    if (!pImg)
        return QString();
    if (QFileInfo::exists(pImg->FullLinkName()))
        return QString(pImg->LinkName(config.bLowerCaseImageExtensions));
    else
        return pImg->name;      // source name
}

QString ThumbnailItem::_VideoFileName() const
{
    Video* pVid = albumgen.VideoAt(_ActAlbum()->IdOfItem(itemPos));
    if (!pVid)
        return QString();

    if (QFileInfo::exists(pVid->FullLinkName()))
        return QString(pVid->LinkName(config.bLowerCaseImageExtensions));
    else
        return pVid->name;      // source name
}

QString ThumbnailItem::_FolderFileName() const
{
   Album album = albumgen.Albums()[_albumId];
   return album.LinkName(-1); // no language and extension
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
    //Image* pImg = albumgen.ImageAt(_ActAlbum()->thumbnail);
    //return pImg->FullSourceName();
    return albumgen.AlbumForID(_ActAlbum()->IdOfItem(itemPos))->FullSourceName();
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

QString ThumbnailItem::FullLinkName() const
{
    if(QFile::exists(FilePath() + FileName()))
        return FilePath() + FileName();
    return FullSourcePath();
}

QString ThumbnailItem::DisplayName() const
{
    switch (_itemType)
    {
        case image: return _ImageFileName();
        case video: return _VideoFileName();
        case folder: return albumgen.AlbumForID(_ActAlbum()->IdOfItem(itemPos))->name;
        default: break;
    }
    return QString();
}

// ****************** ThumbnailView ******************

 /*=============================================================
  * TASK:		constructor
  * EXPECTS:	parent: widget to contain his
  *				thumbsize - pixel width and height of thumbnails
  * GLOBALS:
  * RETURNS:
  * REMARKS:	SA
  *				- connects slots and sets up scrollbar
  *------------------------------------------------------------*/
ThumbnailView::ThumbnailView(QWidget *parent) : QListView(parent)
{
    // prepare spacer for Drag & Drop into tnvImages
    float spacerWidth = 50; // pixel
    _insertPosImage = QImage(spacerWidth, ThumbnailItem::thumbHeight, QImage::Format_ARGB32);
    QPainter *painter = new QPainter(&_insertPosImage);
    _insertPosImage.fill(Qt::transparent);
    painter->setBrush(Qt::NoBrush);
    QPen pen;
    pen.setColor(schemes[PROGRAM_CONFIG::schemeIndex].sSpacerColor);
    pen.setWidth(10);
    painter->setPen(pen);
    painter->drawLine(spacerWidth / 2.0, 0, spacerWidth / 2.0, ThumbnailItem::thumbHeight);
    painter->drawLine(0, 0, spacerWidth, 0);
    painter->drawLine(0, ThumbnailItem::thumbHeight, spacerWidth, ThumbnailItem::thumbHeight);
    delete painter;

    _currentItem = 0;

    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setResizeMode(QListView::Adjust);
    setWordWrap(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformItemSizes(false);

    int thumbheight = ThumbnailItem::thumbHeight;   // mod
    setIconSize(QSize(thumbheight, thumbheight));
// experiment:
//	setMovement(QListView::Free);
//	setDragDropMode(QAbstractItemView::InternalMove);

    _thumbnailViewModel = new ThumbnailViewModel(this);
    _thumbnailViewModel->setSortRole(SortRole);
    setModel(_thumbnailViewModel);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ThumbnailView::loadVisibleThumbs);
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ThumbnailView::onSelectionChanged);

	connect(this, &QListView::doubleClicked, this, &ThumbnailView::ItemDoubleClicked);


    _fileFilters = new QStringList;

    //QTime time = QTime::currentTime();
    // qsrand((uint) time.msec());
    // using QRandomGenerator::global() which is always securely seeded

	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
}

//void ThumbnailView::setThumbColors() {
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
void ThumbnailView::selectCurrentIndex()
{
    if (_currentIndex.isValid() && _thumbnailViewModel->rowCount() > 0) 
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
QString ThumbnailView::GetSingleSelectionFilename()
{
    if (selectionModel()->selectedIndexes().size() == 1)
        return _thumbnailViewModel->item(selectionModel()->selectedIndexes().first().row())->data(
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
int ThumbnailView::GetNextItem()
{
    if (_currentItem == _thumbnailViewModel->rowCount() - 1) 
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
int ThumbnailView::GetPrevItem()
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
int ThumbnailView::GetLastItem()
{
    return _thumbnailViewModel->rowCount() - 1;
}

/*=============================================================
* TASK:	gets index of a random thumbnail
* EXPECTS:
* GLOBALS:
* RETURNS: ordinal of random thumbnail
* REMARKS:
*------------------------------------------------------------*/
int ThumbnailView::GetRandomItem()
{
    return QRandomGenerator::global()->generate() % (_thumbnailViewModel->rowCount());
}

/*=============================================================
 * TASK:	return current row
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
int ThumbnailView::GetCurrentItem()
{
    return _currentItem;
}

void ThumbnailView::SetCurrentItem(int index) 
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
void ThumbnailView::SetTitle()
{
    title = _thumbnailViewModel->item(_currentItem)->data(Qt::DisplayRole).toString()
                    + " - ["
                    + QString::number(_currentItem + 1)
                    + "/"
                    + QString::number(_thumbnailViewModel->rowCount())
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
bool ThumbnailView::SetCurrentIndexByName(QString &fileName)
{
    QModelIndexList indexList = _thumbnailViewModel->match(_thumbnailViewModel->index(0, 0), FileNameRole, fileName);
    if (indexList.size()) 
	{
        _currentIndex = indexList[0];
        SetCurrentItem(_currentIndex.row());
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
bool ThumbnailView::setCurrentIndexByItem(int row)
{
	QModelIndex idx = _thumbnailViewModel->indexFromItem(_thumbnailViewModel->item(row));
	if (idx.isValid()) 
	{
		_currentIndex = idx;
		SetCurrentItem(idx.row());
		return true;
	}

    return false;
}

/*=============================================================
 * TASK:	slot for signal when model selection is changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: emits signal 'SignalSingleSelection ' with selected item id
 *------------------------------------------------------------*/
void ThumbnailView::onSelectionChanged(const QItemSelection &)
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    int selectedCount = indexesList.size();
	if(!selectedCount)
		emit SignalSingleSelection(_ActAlbum()->ID);
    else if (selectedCount == 1)
	{
        SetCurrentItem(indexesList.first().row());
		emit SignalSingleSelection(_ActAlbum()->items[_currentItem]);
	}
    else
    {
        IdList idList, 
               &items = _ActAlbum()->items;
        for (auto e : indexesList)
        {
            ID_t id = items[e.row()];
            if (idList.indexOf(id) < 0)
                idList << items[e.row()];

            emit SignalMultipleSelection(idList);
        }
    }

    if (selectedCount >= 1) 
	{
        int rcnt = _thumbnailViewModel->rowCount();
        statusStr = tr("Selected %1 of %2").arg(QString::number(selectedCount))
                .arg(tr(" %n %3", "", rcnt))
                .arg(rcnt == 1 ? tr("item") :tr("items"));
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
QStringList ThumbnailView::GetSelectedThumbsList()
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    QStringList SelectedThumbsPaths;

    for (int tn = indexesList.size() - 1; tn >= 0; --tn) 
	{
        SelectedThumbsPaths << _thumbnailViewModel->item(indexesList[tn].row())->data(FileNameRole).toString();
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
 * REMARKS:	- called by ThumbnailView (QListView)
 *			- overrides 'startDrag()' of QListView
 *			- sets all file names as URLs into the mimeData of
 *				the drag
 *			- creates a small pixmap from the first max. 5 images
 *				to show what you are dragging
 *			- sets hot spot into the middle of this pixmap
 *			- executes drag with allowing all actions with a 
 *				default of ignoring the drop
 *------------------------------------------------------------*/
void ThumbnailView::startDrag(Qt::DropActions)
{
    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    if (indexesList.isEmpty()) 
        return;

    QDrag *drag = new QDrag(this);
    ThumbMimeData *mimeData = new ThumbMimeData;
    QList<QUrl> urls;	// create pixmap from this list of names
	for (auto f : indexesList)
	{
	    ThumbnailItem thumbRec(f.row(), _albumId, (ThumbnailItem::Type)_thumbnailViewModel->item(f.row())->type() );
		urls << QUrl(thumbRec.FileName());
		thumbRec.itemPos = f.row();
		mimeData->thumbList << f.row();
	}

    drag->setMimeData(mimeData);    // to move items
    // now generate visual representations of images dragged
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
            QPixmap pix1 = _thumbnailViewModel->item(indexesList.at(i).row())->icon().pixmap(72);
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
        pix = _thumbnailViewModel->item(indexesList.at(0).row())->icon().pixmap(128);
        drag->setPixmap(pix);
    }
    drag->setHotSpot(QPoint(pix.width() / 2, pix.height() / 2));

	_dragFromHereInProgress = true;

			// only action that makes sense when started for this listview is the move action
			// but from other sources the Copy and Link actions are also valid
	drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::MoveAction);
}

#ifdef DEBUG_ME
QString __DebugPrintDragAndDrop(QDragMoveEvent *event)    // QDragEnterEvent inherits this
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

    qDebug() << qs;
    return qs;
}
#else
    #define __DebugPrintDragAndDrop(e)
#endif

/*=============================================================
 * TASK:	 sent when dragging is in progress
 * EXPECTS: either a list of file names or a mime data of type 
 *			x-thumbs
 * GLOBALS:
 * RETURNS:	none
 * REMARKS: follewd immediately by a dragMoveEvent()
 *------------------------------------------------------------*/
void ThumbnailView::dragEnterEvent(QDragEnterEvent * event)
{
	if (_IsAllowedTypeToDrop(event) )
	{
// DEBUG
#if 1
// DEBUG
  //      if (pDragDropLabel)
		//{
		//	delete pDragDropLabel;
  //          pDragDropLabel = nullptr;
		//}
// /DEBUG
// DEBUG
//        QString qs = __DebugPrintDragAndDrop(event);
  //      pDragDropLabel = new QLabel(qs, this, Qt::ToolTip);
		//pDragDropLabel->setVisible(true);
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
void ThumbnailView::dragLeaveEvent(QDragLeaveEvent * event)
{
// DEBUG
    //if (pDragDropLabel)
    //{
    //    delete pDragDropLabel;
    //    pDragDropLabel = nullptr;
    //}
// /DEBUG
    //	dynamic_cast<ThumbnailViewModel *>(model())->SetDummyPos(-1);
//	QListView::dragLeaveEvent(event);
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:QDragMoveEvent is a descendant of QDropEvent
 *------------------------------------------------------------*/
void ThumbnailView::dragMoveEvent(QDragMoveEvent * event)
{
	if (!_IsAllowedTypeToDrop(event))
		return;

	QModelIndex index = indexAt(event->pos());
// DEBUG
#if 1
//	dynamic_cast<ThumbnailViewModel *>(model())->SetDummyPos(row);
// DEBUG
 //   if (pDragDropLabel)
	//{
	//	delete pDragDropLabel;
 //       pDragDropLabel = nullptr;
	//}
// /DEBUG
// 
//	int row = index.row();
//	QString qs = QString("mouse: (%1,%2), Item:%3").arg(event->pos().x()).arg(event->pos().y()).arg(row);
//	QString qs = QString("mouse: %1,%2, row:%3, col:%4, dummy:%5").arg(event->pos().x()).arg(event->pos().y()).arg(row).arg(index.column()).arg(dynamic_cast<ThumbnailViewModel *>(model())->DummyPosition());
// DEBUG
    // QString qs = __DebugPrintDragAndDrop(event) + QString(" r:%1").arg(row);
    //pDragDropLabel = new QLabel(qs, this, Qt::ToolTip);
    //pDragDropLabel->setVisible(true);
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
	int pose = event->pos().y();

	if (pose < 30)
		verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
	else if (pose > height() - 30)
		verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

/*=============================================================
 * TASK:    drop files and folders from disk
 * PARAMS:  ThumbMimeData poiter containing an URL list
 * GLOBALS: _albumID
 * RETURNS:
 * REMARKS: - Files and folders are not moved physically
 *          - Files and folders already in gallery are not
 *              added again. 
 *          - TODO: add folders as virtual folders
 *------------------------------------------------------------*/
void ThumbnailView::_DropFromExternalSource(const ThumbMimeData* mimeData, int row)
{
    QStringList qsl, qslF;      // for files and Folders
    auto what = [&](QString s)  {
        QFileInfo fi(s);
        if (fi.isDir())
        {
            return 0;   // folder
        }
        else // file
        {
            QString ext;
            int ld = s.lastIndexOf('.');
            if (ld >= 0)
                ext = s.mid(ld + 1);
            return ext == "jpg" || ext == "png" || ext == "mp4" || ext == "ogg" ? 1 : -1;   // file
        }
        return -1; // can't use'
    };

    for (auto u : mimeData->urls())
    {
        auto s = u.toLocalFile();
        int w = what(s);
        if (w > 0)      // file
            qsl << s;
        else if (!w)    // folder
            qslF << s;
    }
    _AddImagesFromList(qsl, row);
    row += qsl.size();  // position for folders
    if (_AddFoldersFromList(qslF, row))
    {

        emit SignalAlbumStructChanged();
        Reload();
    }
}

/*=============================================================
 * TASK:    moves images/folders or add new images and folders
 *          to the data base at the given position
 * PARAMS:  event - contains mimeData with either an URL list or
 *              a list of indexes in actual albums items' list
 * GLOBALS: 
 * RETURNS:
 * REMARKS: - on move images
 *              order in thumbList depends on selection order
 *              images can be put to the same position they 
 *              were before
 *          - changes the icon order in 'fileIcons' too
 *          - 
 *------------------------------------------------------------*/
void ThumbnailView::dropEvent(QDropEvent * event)
{
	if (!_IsAllowedTypeToDrop(event))
		return;

	const ThumbMimeData *mimeData = qobject_cast<const ThumbMimeData *>(event->mimeData());
    if (!mimeData)      // why does it occur?
        return;
	// get drop position
	QModelIndex index = indexAt(event->pos());

    int row = index.row();

    if (mimeData->hasUrls())
        _DropFromExternalSource(mimeData, row);
    else
	{
		if (mimeData->thumbList.isEmpty())
			return;

        IntList thl = mimeData->thumbList;  // index in thumbList items to move
        IntList thl0 = thl;                 // original index array of thumblist to check items against
        // reorder list of actual album
        Album* pAlbum = const_cast<Album*>(_ActAlbum());
        IdList &items = pAlbum->items;      // original ordered items

        QVector<int> itemOrder;             // new item order indexes
        itemOrder.resize(items.size());     // original indexes are 0,1,2...

        // special handling for drops one item to the right
        // simply exchanges items
        if (thl.size() == 1 && thl[0] == row - 1)
            ++row;

        int si = 0,     // original index
            di = 0;     // index in idl
        // rearrange 0,1,2,... to new order
        while(si < itemOrder.size())          // di <= si
        {
            if (!thl.size())           // no more moved items
            {
                if(thl0.indexOf(si) < 0)    // don't move twice
                    itemOrder[di++] = si;
                ++si;
            }
            else
            {
                if (si == row)
                {
                    for (int j = 0; j < thl.size(); ++j)
                        itemOrder[di++] = thl[j];
                    thl.clear();
                }
                if (thl0.indexOf(si) < 0)
                    itemOrder[di++] = si;
                ++si;
            }
        }
        // new order in 'itemOrder' set
        IdList idl;                         // new ordered items
        idl.resize(items.size());

        for (int i = 0; i < itemOrder.size(); ++i)
            idl[i] = items[itemOrder[i]];

        // modify original stored itemOrder
        QVector<int> origOrder = fileIcons.IconOrder();  // original order might have been changed
                                                         // so we must rearrange that according to 'itemOrder'
        QVector<int> iconOrder;                          // new icon order indexes
        iconOrder.resize(items.size());     
        for (int i = 0; i < itemOrder.size(); ++i)
            iconOrder[i] = origOrder[itemOrder[i]];

        fileIcons.SetIconOrder(iconOrder);

		if (_dragFromHereInProgress)	// then remove from old spot
		{

		}
        items = idl;
        pAlbum->changed = true;
        Reload();
		// DEBUG
		//if (pDragDropLabel)
		//	delete pDragDropLabel;
		//pDragDropLabel = nullptr;
		// /DEBUG

		// put files at new spot

	}
//    emit SignalFolderAdded();
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::abort()
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
void ThumbnailView::loadVisibleThumbs(int scrollBarValue)
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
            if (lastVisible >= _thumbnailViewModel->rowCount()) 
                lastVisible = _thumbnailViewModel->rowCount() - 1;
            
        } 
		else 
		{
            firstVisible -= (lastVisible - firstVisible) * 3;  // (Settings::thumbsPagesReadCount + 1);
            if (firstVisible < 0) 
                firstVisible = 0;

            lastVisible += 10;
            if (lastVisible >= _thumbnailViewModel->rowCount()) 
                lastVisible = _thumbnailViewModel->rowCount() - 1;
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

int ThumbnailView::_GetFirstVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = 0; currThumb < _thumbnailViewModel->rowCount(); ++currThumb) 
	{
        idx = _thumbnailViewModel->indexFromItem(_thumbnailViewModel->item(currThumb));
        if (viewport()->rect().contains(QPoint(0, visualRect(idx).y() + visualRect(idx).height() + 1))) 
            return idx.row();
        
    }

    return -1;
}

int ThumbnailView::_GetLastVisibleThumb() 
{
    QModelIndex idx;

    for (int currThumb = _thumbnailViewModel->rowCount() - 1; currThumb >= 0; --currThumb) 
	{
        idx = _thumbnailViewModel->indexFromItem(_thumbnailViewModel->item(currThumb));
        if (viewport()->rect().contains(QPoint(0, visualRect(idx).y() + visualRect(idx).height() + 1))) 
            return idx.row();
    }

    return -1;
}

void ThumbnailView::LoadFileList() 
{
	emit SignalInProcessing(true);
    for (int i = 0; i < model()->rowCount(); i++) 
        AddThumb(i, (ThumbnailItem::Type)model()->data(model()->index(i,0), TypeRole).toInt());
    
    _UpdateThumbsCount();

	_isBusy = false;
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
	emit SignalInProcessing(false);
}

void ThumbnailView::Load() 
{
    fileIcons.Clear();      // clear all icons in memory
    _slSearchPaths.clear(); // and all search paths for missing images inside previous folder
    Reload();
}

void ThumbnailView::Reload()
{
    _isAbortThumbsLoading = false;
    _thumbsRangeFirst = -1;
    _thumbsRangeLast = -1;

    _isBusy = true;
    _thumbnailViewModel->Clear(); // clearing occurs between 'beginResetModel()' and 'endResetModel()'
//    fileIcons.Clear();

    setSpacing(QFontMetrics(font()).height());

    //if (_isNeedToScroll) 
    //{
    //    scrollToTop();
    //}

    _InitThumbs();
    _UpdateThumbsCount();
    loadVisibleThumbs(0);
    _isBusy = false;
}

void ThumbnailView::Setup(ID_t aid)
{
    _albumId = aid;
//    _InitThumbs();
}

void ThumbnailView::Clear()
{
     _pIds = nullptr; 
     _albumId = ROOT_ALBUM_ID;
    _thumbnailViewModel->Clear();
}

void ThumbnailView::RemoveViewer(ImageViewer* pv)
{
    if (!_dontRemoveImageViewerFromList)
    {
        int n = _lstActiveViewers.indexOf(pv);
        if (n >= 0)
            _lstActiveViewers.removeAt(n);
    }
    emit SignalImageViewerAdded(!_lstActiveViewers.isEmpty());
}

void ThumbnailView::_RemoveAllViewers()
{
    _dontRemoveImageViewerFromList = true;
    for (auto p : _lstActiveViewers)
    {
        p->close();
        delete p;
    }
    _lstActiveViewers.clear();
    _dontRemoveImageViewerFromList = false;
    emit SignalImageViewerAdded(false);
}

void ThumbnailView::_InitThumbs()
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
		_thumbnailViewModel->appendRow(thumbItem);
		++thumbsAddedCounter;
		if (thumbsAddedCounter > 10)
		{
			thumbsAddedCounter = 1;
			QApplication::processEvents();
		}
	}

	if (model()->rowCount() && selectionModel()->selectedIndexes().size() == 0)
		selectThumbByItem(0);
}

/*=============================================================
 * TASK:	emit a signal tha image count changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::_UpdateThumbsCount()
{
    if (_thumbnailViewModel->rowCount() > 0) 
        statusStr = tr("%1 item(s)").arg(_thumbnailViewModel->rowCount());
    else 
		statusStr = tr("No items");
	emit SignalStatusChanged(statusStr);
	emit SignalTitleChanged(title);
}

/*=============================================================
 * TASK:	signal if suitable mime data to drop is available
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: does not check if the files or folders in an url list
 *          ar acceptable, just see the allowed type
 *------------------------------------------------------------*/
bool ThumbnailView::_IsAllowedTypeToDrop(const QDropEvent *event)
{
    // DEBUG
    qDebug() << "Mime text: " << event->mimeData()->text() 
             << ", hasUrls ? " << event->mimeData()->hasUrls()
             << "mimeData is null?" << (event->mimeData() ? "no":"yes")
             << ", hasImage ? " << event->mimeData()->hasImage()
             << ", x-thumb ? " << event->mimeData()->hasFormat("application/x-thumb")
        ;
     return (indexAt(event->pos()).row() >= 0) && 
            (event->mimeData()->hasUrls() ||    // e.g. text() == file:///I:/alma.jpg, or text() == file:///I:/folderName
            event->mimeData()->hasImage() ||    // image/...
		    event->mimeData()->hasFormat("application/x-thumb")     // drag and drop inside this application
                );
}

/*=============================================================
 * TASK:    get all file names into  _imageMap or _videoMap,
 *          plus into the actual album into albumgen's _albumMap
 *          plus into _thumbnailViewModel
 * PARAMS:  qslFileNames - file names to add
 *          row - add before this row
 * GLOBALS:
 * RETURNS:
 * REMARKS: no thumbnail flag is set for any of the icons
 *------------------------------------------------------------*/
void ThumbnailView::_AddImagesFromList(QStringList qslFileNames,int row)
{
    bool res = true;

    int i;
    for (i = 0; i < qslFileNames.size(); ++i)
    {
        res &= albumgen.AddImageOrVideoFromString(qslFileNames[i], *_ActAlbum(), row);
        if (!res)     // then already used somewehere // TODO: virtual albums
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr("Adding new image / video failed!"));
            continue;
        }
        else
            (void)fileIcons.Insert(row, false, qslFileNames[i]);  
    }
}

/*=============================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool ThumbnailView::_AddFoldersFromList(QStringList qslFolders, int row)
{
    bool atLeastOneFolderWasAdded = false;
    for (auto folderName: qslFolders)
            atLeastOneFolderWasAdded |= _AddFolder(folderName);

    return atLeastOneFolderWasAdded;
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::selectThumbByItem(int row)
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
void ThumbnailView::loadThumbsRange()
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
    _currentItemCount = _thumbnailViewModel->rowCount();

    for (_scrolledForward ? currThumb = _thumbsRangeFirst : currThumb = _thumbsRangeLast;
         (_scrolledForward ? currThumb <= _thumbsRangeLast : currThumb >= _thumbsRangeFirst);
         _scrolledForward ? ++currThumb : --currThumb) 
	{

        if (_isAbortThumbsLoading || _thumbnailViewModel->rowCount() != _currentItemCount || currThumb < 0)
            break;

        if (_thumbnailViewModel->item(currThumb)->data(LoadedRole).toBool())
            continue;

        imageFileName = _thumbnailViewModel->item(currThumb)->data(FileNameRole).toString();
        thumbReader.setFileName(imageFileName);
        currentThumbSize = thumbReader.size();
        imageReadOk = false;

        if (currentThumbSize.isValid()) 
		{
            if (currentThumbSize.width() > ThumbnailItem::thumbHeight || currentThumbSize.height() > ThumbnailItem::thumbHeight) 
                currentThumbSize.scale(QSize(ThumbnailItem::thumbHeight, ThumbnailItem::thumbHeight), Qt::KeepAspectRatio);
        
            thumbReader.setScaledSize(currentThumbSize);
            imageReadOk = thumbReader.read(&thumb);
        }

        if (imageReadOk) 
		
            _thumbnailViewModel->item(currThumb)->setIcon(QPixmap::fromImage(thumb));
         else 
		 {
            _thumbnailViewModel->item(currThumb)->setIcon(QIcon::fromTheme("image-missing",
                                                              QIcon(":/Preview/Resources/NoImage.jpg")).pixmap(
                                                                  ThumbnailItem::thumbHeight, ThumbnailItem::thumbHeight));
															           //BAD_IMAGE_SIZE, BAD_IMAGE_SIZE));
            //currentThumbSize.setHeight(BAD_IMAGE_SIZE);
            //currentThumbSize.setWidth(BAD_IMAGE_SIZE);
        }

        _thumbnailViewModel->item(currThumb)->setData(true, LoadedRole);
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
void ThumbnailView::AddThumb(int which, ThumbnailItem::Type type)
{
    ThumbnailItem *thumbItem = new ThumbnailItem(which, _albumId, type);
    QImageReader thumbReader;
    QSize currThumbSize;

    thumbReader.setFileName(reinterpret_cast<ThumbnailViewModel*>(model())->item(which,0)->text());
    currThumbSize = thumbReader.size();
    if (currThumbSize.isValid()) 
	{
        if (currThumbSize.width() > ThumbnailItem::thumbHeight || currThumbSize.height() > ThumbnailItem::thumbHeight) 
            currThumbSize.scale(QSize(ThumbnailItem::thumbHeight, ThumbnailItem::thumbHeight), Qt::KeepAspectRatio);

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

    _thumbnailViewModel->appendRow(thumbItem);
}


/*========================================================
 * TASK:
 * PARAMS:  here - insert space before this item index (row)
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
void ThumbnailView::SetInsertPos(int here)
{
    _insertPos = here;
}



/*=============================================================
 * TASK:
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::keyReleaseEvent(QKeyEvent* event)
{
    if (currentIndex().isValid() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
        DeleteSelected();
    else
        QListView::keyReleaseEvent(event);
}

/*=============================================================
 * TASK:	modifies scrollbar position for wheel events
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::wheelEvent(QWheelEvent *event)
{

    int step = verticalScrollBar()->maximum() / _ActAlbum()->items.size() * width() / (ThumbnailItem::thumbHeight + spacing());

    if (event->angleDelta().y() < 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + step);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - step);
    }
}

/*=============================================================
 * TASK:	executes mouse press
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - it does nothing new yet
 *------------------------------------------------------------*/
void ThumbnailView::mousePressEvent(QMouseEvent *event)
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
void ThumbnailView::mouseMoveEvent(QMouseEvent * event)
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
void ThumbnailView::contextMenuEvent(QContextMenuEvent * pevent)
{
	QMenu menu(this);
	QAction *pact;
    int nSelSize = selectionModel()->selectedIndexes().size();

    if (nSelSize == 0)
    {       // 2 albums always added to AlbumMap: root and latest
        if (albumgen.Albums().size() > 2 && !_ActAlbum()->items.isEmpty())
        {
            pact = new QAction(tr("Select Album Thumbnail..."), this);  // any image
            connect(pact, &QAction::triggered, this, &ThumbnailView::SelectAsAlbumThumbnail);
            menu.addAction(pact);

            menu.addSeparator();
        }
    }
    else
    {
        if (nSelSize == 1)
        {
            int pos = currentIndex().row();     
            Album& album = albumgen.Albums()[_albumId];
            
            ID_t id = album.items[pos];
            IABase* pItem = nullptr;
            if (id & IMAGE_ID_FLAG)
                pItem = albumgen.ImageAt(id);
            else if (id & VIDEO_ID_FLAG)
                pItem = albumgen.VideoAt(id);
            else
                pItem = albumgen.AlbumForID(id);

            bool exists = QFile::exists(pItem->FullSourceName());

            if (exists)
            {
                pact = new QAction(tr("Set As Album &Thumbnail"), this);
                // select from existing image
                connect(pact, &QAction::triggered, this, &ThumbnailView::SetAsAlbumThumbnail);
                menu.addAction(pact);           // select any image
            }
            else
            {
                pact = new QAction(tr("Find missing item"), this);
                connect(pact, &QAction::triggered, this, &ThumbnailView::FindMissingImageOrVideo);
                menu.addAction(pact);
            }
        }


        if (nSelSize)
        {
            pact = new QAction(tr("Copy &Name(s)"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::CopyNamesToClipboard);
            menu.addAction(pact);

            pact = new QAction(tr("Copy &Original Name(s)"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::CopyOriginalNamesToClipboard);
            menu.addAction(pact);

            menu.addSeparator();
        }
    }
	pact = new QAction(tr("Add &Images..."), this);  // any number of images from a directory
	pact->setEnabled(true);
	connect(pact, &QAction::triggered, this, &ThumbnailView::AddImages);
	menu.addAction(pact);

	pact = new QAction(tr("Add &Folder..."), this);  // one folder added to the folder tree inside this album
	connect(pact, &QAction::triggered, this, &ThumbnailView::AddFolder);
	menu.addAction(pact);

    if(nSelSize > 1)
    {
	    menu.addSeparator();

        pact = new QAction(tr("&Synchronize texts"), this);
        connect(pact, &QAction::triggered, this, &ThumbnailView::SynchronizeTexts);
        menu.addAction(pact);
    }


    if (nSelSize)
    {
	    menu.addSeparator();
        pact = new QAction(tr("&Remove"), this);
        connect(pact, &QAction::triggered, this, &ThumbnailView::DeleteSelected);
        menu.addAction(pact);
    }


#if 0
    // how to make it work?
    pact = new QAction(tr("&Undo Delete"), this);
#define CAN_UNDO(a) (a && !a->NothingToUndo())
	bool undo = model()->rowCount() && (CAN_UNDO() || CAN_UNDO);
	pact->setEnabled(undo);
	if (undo)
		connect(pact, &QAction::triggered, this, &ThumbnailView::UndoDelete);
	menu.addAction(pact);
#endif

	menu.exec(pevent->globalPos());
}

void ThumbnailView::invertSelection() 
{
    QItemSelection toggleSelection;
    QModelIndex firstIndex = _thumbnailViewModel->index(0, 0);
    QModelIndex lastIndex = _thumbnailViewModel->index(_thumbnailViewModel->rowCount() - 1, 0);
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
void ThumbnailView::DeleteSelected()
{
	QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.isEmpty())
        return;
	QString s;

    QString plurali = tr("images"), plurala = tr("albums");   // plural for image and album. May differ in other languages
    QString qs = QMainWindow::tr("Do you want to delete selected %1 / %2 from disk, or just to remove them from gallery?")
                                    .arg(list.size() > 1 ? plurali :tr("image"))
                                    .arg(list.size() > 1 ? plurala : tr("album"));
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
    // list is ordered by ascending row's and we need delete descending
    for (int i = list.size()-1; i >= 0; --i)
    {
        int ix = list[i].row();
        ID_t id = album.items[ix];
        if (id & ALBUM_ID_FLAG)    // remove recursively
        {
            albumgen.Albums().RemoveRecursively(id);
        }
        else if (id & IMAGE_ID_FLAG)    // remove recursively
        {
            Image *img = albumgen.ImageAt(id);
            if (!--img->usageCount)
                albumgen.Images().remove(id);
        }
        else if (id & VIDEO_ID_FLAG)    // remove recursively
        {
            Video *vid = albumgen.VideoAt(id);
            if (!--vid->usageCount)
                albumgen.Videos().remove(id);
        }
        
        album.items.remove(ix);
        fileIcons.Remove(ix);
    }
    if (res == QMessageBox::Yes)     // then delete from disk too
    {
        // TODO
    }
    emit SignalAlbumStructChanged();
    Reload();
//    emit selectionChanged(QItemSelection(), QItemSelection());
}

/*=============================================================
 * TASK:    When multiple images selected synchronize texts 
 *          between them by replacing adding the texts set for 
 *          the first selected items to all items
 *          This is not undoable
 * PARAMS:
 * GLOBALS: selectionModel
 * RETURNS:
 * REMARKS: - order in selectionModel reflects the selection order
 *------------------------------------------------------------*/
void ThumbnailView::SynchronizeTexts()
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.isEmpty())
        return;


    if (QMessageBox::question(this, tr("falconG - Question"),
        tr("This will set the same texts to all selected items\n\n"
            "This action cannot be undone!\n\n"
            "Do you really want to do this?"),
        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        IABase* pItem = nullptr;
        Album* pAlbum = _ActAlbum();

        QModelIndexList::iterator it = list.begin();
        auto Item = [&](int r)
        {
            ID_t id = pAlbum->items[r];
            if (id & ALBUM_ID_FLAG)
                pItem = albumgen.AlbumForID(id);
            else if (id & IMAGE_ID_FLAG)
                pItem = albumgen.ImageAt(id);
            else
                pItem = albumgen.VideoAt(id);

            return pItem;
        };

        pItem = Item(it->row());
        ID_t titleId = pItem->titleID,
                descrId = pItem->descID;
        while (++it != list.end())
        {
            pItem = Item(it->row());
            pItem->titleID = titleId;
            pItem->descID = descrId;
        }
    }
}

/*=============================================================
 * TASK:	add deleted images back
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::UndoDelete()
{
}


/*=============================================================
 * TASK:   Display dialog box to add images to image list
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: connected to popup menu
 *------------------------------------------------------------*/
void ThumbnailView::AddImages()
{
    QString dir = _ActAlbum()->FullSourceName();
    QStringList qslFileNames = QFileDialog::getOpenFileNames(this, tr("falconG - Add images/videos"), dir, "Images(*.bmp *.jpg *.png *.tif);;Videos(*.mp4,*.ogg);;All files(*.*)");
    if (qslFileNames.isEmpty())
        return;
    int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;
    _AddImagesFromList(qslFileNames, pos);
    Reload();

    emit SignalAlbumChanged();
    emit selectionChanged(QItemSelection(), QItemSelection());
    config.dsLastImageDir = dir;
}


/*=============================================================
 * TASK:    internal function that adds a folder recursively
 * PARAMS:  folderName - name of folder to add
 * GLOBALS:
 * RETURNS: if folder was added
 * REMARKS: does not send signal 
 *------------------------------------------------------------*/
bool ThumbnailView::_AddFolder(QString folderName)
{
    Album* pParentAlbum = _ActAlbum();

    bool added, atLeastOneFolderWasAdded = false;
    ID_t id = albumgen.Albums().Add(folderName,added);
    if (added)
    {   
        atLeastOneFolderWasAdded = true;
        pParentAlbum = _ActAlbum();     // album position may have changed when new album was added to map
        pParentAlbum->changed = true;
        Album &album = *albumgen.AlbumForID(id);
        album.parent = _albumId;
        albumgen.AddDirsRecursively(id);

        int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;
        if(pos < 0)
            pParentAlbum->items.push_back(id);
        else
            pParentAlbum->items.insert(pos, id);

        ID_t idth = album.ThumbID();
        if (!idth)
            folderName.clear();
        else
            folderName = albumgen.Images()[idth].FullSourceName();
        (void)fileIcons.Insert(pos, true,folderName);
    }
    else
        QMessageBox::warning(this, tr("falconG - Warning"), tr("Adding new album failed!\n\nMaybe the album is already in the gallery."));

    return atLeastOneFolderWasAdded;
}

/*=============================================================
 * TASK:   Display dialog box to add one folder into the 
 *          actual album
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - actual album is parent of new album folder
 *          - no thumbnail flag is set
 *------------------------------------------------------------*/
void ThumbnailView::AddFolder()
{
    Album* pParentAlbum = _ActAlbum();
    QString dir = pParentAlbum->path;
    QString qs = QFileDialog::getExistingDirectory(this, tr("falconG - Add Directory"), dir);
    if (qs.isEmpty())
        return;
    if (_AddFolder(qs))
    {
        emit SignalAlbumStructChanged();
        Reload();
    }
}

/*============================================================================
  * TASK:	copy names of all selected thumbnail to clipboard
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: never called when there is no selection
 *--------------------------------------------------------------------------*/
void ThumbnailView::CopyNamesToClipboard()
{
	QString s;
	QModelIndexList list = selectionModel()->selectedIndexes();
    for (auto i : list)
    {
        ID_t id = _ActAlbum()->items[i.row()];// ID_t(i.internalPointer());
        s += QString("%1%2").arg(id & ALBUM_ID_FLAG ? config.sBaseName.ToString() : "").arg(id & BASE_ID_MASK) + "\n";
    }
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
void ThumbnailView::CopyOriginalNamesToClipboard()
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
  * TASK:	slot to set current file name as album thumbnail, shown on
  *         parents aimage list
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: - only called when there is exactly one selected image
  *          - if the selected item is an album with valid thumbnail,
  *             its thumbnail is used
  *          - if the selected item is an album with no valid thumbnail 
  *             (e.g. new album just added), the album id is used as thumbnail
  *             in the database. such thumbnails are not shown
 *--------------------------------------------------------------------------*/
void ThumbnailView::SetAsAlbumThumbnail()
{
    int pos = currentIndex().row();     // new thumbnail index

    Album& album = albumgen.Albums()[_albumId];
    int cthix = -1;// old thumbnail index in 'items'. -1: not from current album
    for (int i = 0; i < album.items.size(); ++i)
        if (album.thumbnail == album.items[i])
            cthix = i;
    if (cthix == pos)       // no change
        return;

    ID_t th = album.items[pos];

    album.thumbnail = th & ALBUM_ID_FLAG ? albumgen.Albums()[th].thumbnail : albumgen.ImageAt(th)->ID;
    album.changed = true;
    if(album.parent)
    {
        Album * parent = albumgen.AlbumForID(album.parent);
        parent->changed = true;
    }
    albumgen.SetGalleryModified(_albumId);
}
/*=============================================================
 * TASK:    slot to select an image from any image on disk
 *          and set it as album thumbnail shown on parent's
 *          image list
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::SelectAsAlbumThumbnail()
{
    QString thname = QFileDialog::getOpenFileName(this, 
                                            tr("falconG - Select Thumbnail Image"),
                                            albumgen.Images().lastUsedPath,
                                            "Image files (*.jpg *.png)");
    if (thname.isEmpty())
        return;
    // add as thumbnail to this album, but do not signal elapsed time and do not add to items
    albumgen.AddImageOrAlbum(_albumId, thname, true, false, true);

    Album& album = *_ActAlbum();

    if (album.parent)
    {
        Album* parent = albumgen.AlbumForID(album.parent);
        parent->changed = true;
    }
}


void ThumbnailView::ItemDoubleClicked(const QModelIndex& mix)
{
    int row = mix.row();
    ID_t id = _ActAlbum()->items[row];
    if (id & ALBUM_ID_FLAG)
    {
        emit SignalFolderChanged(row);
    }
    else        // display full image in window      
    {

        IABase *pb = id & IMAGE_ID_FLAG ? (IABase*)albumgen.ImageAt(id) : (IABase*)albumgen.VideoAt(id);
        QString name = pb->FullSourceName();
        
        ImageViewer* pViewer = new ImageViewer(name, this, nullptr);
        pViewer->setAttribute(Qt::WA_DeleteOnClose);
        if (pViewer->LoadFile())
        {
            pViewer->show();
            _lstActiveViewers.push_back(pViewer);

            emit SignalImageViewerAdded(true);
        }
        else
        {
            pViewer->close();
            delete pViewer;
        }
    }
}

void ThumbnailView::ThumbnailSizeChanged(int newSize)
{
    setIconSize(QSize(newSize, newSize));
    ThumbnailItem::SetThumbHeight(newSize);
}

/*=============================================================
 * TASK:    slot for missing image\video search
 * PARAMS:
 * GLOBALS: _searchPaths
 * RETURNS:
 * REMARKS: - only used when a single missing image/video 
 *              selected
 *          - appends the path (w.o. name of file) to search 
 *              paths then goes through all missing 
 *              images/videos and tries to find them in any
 *              folder in the search paths
 *------------------------------------------------------------*/
void ThumbnailView::FindMissingImageOrVideo()
{
    int pos = currentIndex().row();
    Album& album = albumgen.Albums()[_albumId];
    ID_t id = album.items[pos];
    QString name, path;
    IABase* pItem = nullptr;

    auto __getItem = [&](ID_t id) {
        if (id & IMAGE_ID_FLAG)
            pItem = albumgen.ImageAt(id);
        else if (id & IMAGE_ID_FLAG)
            pItem = albumgen.VideoAt(id);
        return pItem;
    };
    auto __getNameAndPath = [&](ID_t id, QString& n, QString& p) {
        __getItem(id);
		n = pItem->name;
		p = pItem->path;
	};

    auto __pathindex = [&]() {
        for(int i = 0; i < _slSearchPaths.size(); ++i)
            if (QFile::exists(_slSearchPaths[i] + pItem->name))
                return i;
        return -1;
    };

    __getNameAndPath(id, name, path);
    QString caption = tr("falconG - Find file: ") + name;
    QFileDialog fd(this, caption, path);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setNameFilter("*" + name.right(4));
    fd.selectFile(name);
    QString foundName;
    if (fd.exec())
    {
        foundName = fd.selectedFiles()[0];

    //if (!foundName.isEmpty())
    //{
        QString n, p;
        SeparateFileNamePath(foundName, p, n);
        if (n != name)
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr("File names do not match. "));
            return;
        }
        _slSearchPaths.push_back(p);
        pItem->path = p;     // replace path
        pItem->exists = QFile::exists(pItem->FullSourceName());
        pItem->changed = true;
        album.changed = true;
        emit SignalAlbumChanged();

        // check all missing files against search paths
        for (auto id1 : album.items)
        {
            __getItem(id1);
            if (!QFile::exists(pItem->FullSourceName()))
            {
                int j = __pathindex();
                if (j >= 0)
                {
                    pItem->path = _slSearchPaths[j];
                    pItem->exists = true;
                }
            }
        }
        Reload();
    }
}

void ThumbnailView::SlotToRemoveAllViewers()
{
    _RemoveAllViewers();
}

void ThumbnailView::setNeedToScroll(bool needToScroll) 
{
    this->_isNeedToScroll = needToScroll;
}

void ThumbnailViewModel::Clear()
{
    beginResetModel();
    fileIcons.Clear();
    QStandardItemModel::clear();
    endResetModel();
}
