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

#include <algorithm>
#include <functional>

#include <QIcon>
#include <QWidget>

#include "thumbnailView.h"
#include "config.h"
#include "imageviewer.h"

// ************************ FileIcons *************************

FileIcons fileIcons;

void FileIcons::Clear()
{
	posFolderIcon = -1;
	_iconList.clear();
    _iconOrder.clear();
    MarkedIcon::Init(); // only reads images if not yet initted
}
int FileIcons::Size() const
{ 
    return _iconOrder.size();   // because _iconList may contain more items than shown
}
void FileIcons::SetMaximumSizes(int thumbsize, int borderwidth)
{
    MarkedIcon::SetMaximumSizes(thumbsize, borderwidth);
}

bool FileIcons::HasIconFor(int pos)
{
    return pos < _iconList.size();
}

void FileIcons::SetFolderThumbnailPosition(int pos, bool bIsFolderThumbnail)
{
    // pos-th item MUST exist
    MarkedIcon& micon = _iconList[ _iconOrder[pos] ];
	if (posFolderIcon >= 0)         // erase original folder icon
		micon.isFolderThumb = false;

	micon.isFolderThumb = bIsFolderThumbnail;    // and set this
	if (bIsFolderThumbnail)
		posFolderIcon = pos;
}

QIcon FileIcons::IconForPosition(int pos, Flags flags, QString imageName)
{
	if (pos < 0)
		return QIcon();

	if (pos >= _iconList.size())    // _iconList may contain more items than _iconOrder
        Insert(-1, flags.testFlag(fiFolder), imageName);    // -1: push back

    // pos-th item MUST exist
    Q_ASSERT(pos < _iconOrder.size());

    MarkedIcon& micon = _iconList[_iconOrder[pos]];
    micon.isFolderThumb = flags.testFlag(fiThumb);
    micon.dontResize = flags.testFlag(fiDontResize);
	return micon.ToIcon();
}

const QVector<int> &FileIcons::IconOrder() const
{
    return _iconOrder;
}
void FileIcons::SetIconOrder(const QVector<int>& order)
{
    _iconOrder = order;
}
MarkedIcon * FileIcons::Insert(int pos, bool isFolder, QString imageName)
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
void FileIcons::Remove(int pos)    // remove item _iconOrder[pos]
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

// ****************** ThumbnailItem ******************
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
    
    bool isFolder = itemId.IsAlbum();
    bool bIsFolderIcon = false;

    ID_t imgId = itemId;     // add marker for image that is the folder thumbnail (shown in parent)
//    Album* parent = _ParentAlbum();
    if (isFolder)
        imgId = albumgen.Albums()[itemId].thumbnailId;

    bIsFolderIcon = (pAlbum->thumbnailId == imgId);

    if (fileIcons.HasIconFor(itemPos))
    {
        FileIcons::Flags flags;
        if (isFolder)
            flags.setFlag(FileIcons::fiFolder);
        if (bIsFolderIcon)
            flags.setFlag(FileIcons::fiThumb);
        if(albumgen.ImageAt(imgId)->dontResize)
            flags.setFlag(FileIcons::fiDontResize);

        return fileIcons.IconForPosition(itemPos, flags);
    }
            // else read and create icon


//    imgId &= ID_MASK;
    Image &img = albumgen.Images()[imgId];
    if (!img.name.isEmpty())
    {
        if ((exists = QFile::exists(img.FullLinkName())))
            imageName = img.FullLinkName();
        else if ((exists = QFile::exists(img.FullSourceName())))
            imageName = img.FullSourceName();
        else
            imageName = QString(":/Preview/Resources/NoImage.jpg");
    }
    else
        imageName = QString(":/Preview/Resources/NoImage.jpg");
    
    // DEBUG
    // qDebug((QString("icon for file:'%1' of ID:%2, name:'%3'").arg(imageName).arg(itemId).arg(img.FullSourceName())).toStdString().c_str());
    // /DEBUG
    FileIcons::Flags flags;
    if (isFolder)
        flags.setFlag(FileIcons::fiFolder);
    if (bIsFolderIcon)
        flags.setFlag(FileIcons::fiThumb);
    if (albumgen.ImageAt(imgId)->dontResize)
        flags.setFlag(FileIcons::fiDontResize);

    return fileIcons.IconForPosition(itemPos, flags, imageName);
}


QVariant ThumbnailItem::data(int role) const      // based on _itemType
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
    //Image* pImg = albumgen.ImageAt(_ActAlbum()-->thumbnailId);
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
		emit SignalSingleSelection(_ActAlbum()->ID, _albumId);
    else if (selectedCount == 1)
	{
        SetCurrentItem(indexesList.first().row());
		emit SignalSingleSelection(_ActAlbum()->items[_currentItem], _albumId);
	}
    else
    {
        IdList idList, 
               &items = _ActAlbum()->items;
        for (auto &e : indexesList)
        {
            ID_t id = items[e.row()];
            if (idList.indexOf(id) < 0)
                idList << items[e.row()];

            emit SignalMultipleSelection(idList, _albumId);
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
	for (auto &f : indexesList)
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
            y = (!y) * 40;
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

#ifdef DEBUG
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

    for (auto &u : mimeData->urls())
    {
        auto s = u.toLocalFile();
        int w = what(s);
        if (w > 0)      // file
            qsl << s;
        else if (!w)    // folder
            qslF << s;
    }
    emit SignalAlbumStructWillChange();
    _AddImagesFromList(qsl, row);
    // row += qsl.size();  // position for folders
    bool b = _AddFoldersFromList(qslF, row);
    emit SignalAlbumStructChanged(b);
    if(b)
        Reload();
    _AddImagesFromList(qsl, row);
    //row += qsl.size();  // position for folders
    //bool b = false;
    //emit SignalAlbumStructWillChange();
    //if ((b = _AddFoldersFromList(qslF, row)))
    //{
    //    Reload();
    //}
    //emit SignalAlbumStructChanged(b);
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

	const ThumbMimeData *mimeData = qobject_cast<const ThumbMimeData*>(event->mimeData());
    if (!mimeData)      // why does it occur?
        return;
	// get drop position
	QModelIndex index = indexAt(event->pos());

    int row = index.row();      // -1: drop after the last item

    if (mimeData->hasUrls())
        _DropFromExternalSource(mimeData, row);
    else  if (mimeData->hasFormat("application/x-thumb") && !((const ThumbMimeData *)mimeData)->thumbList.isEmpty()) // perform drops from thumbnail list
	{
        IntList thl = ((const ThumbMimeData*)mimeData)->thumbList;  // indices in actual album's items to move

        Album* pAlbum = const_cast<Album*>(_ActAlbum());
        IdList &items = pAlbum->items;   // original ordered items

        //IdList  droppedIds(thl.size());        // id for items at positions from 'thl'
        //for (int i = 0; i < thl.size(); ++i)
        //    droppedIds[i] = items[thl[i]];
        // reorder items of actual album
        int itemSize = items.size();     // only changes when items dropped on other albums

        QVector<int> itemOrder;             // new item order indexes
        bool moveItemsIntoFolder = row >= 0 && (items[row].IsAlbum());
        if (moveItemsIntoFolder) // then move items into album with id items[row]
        {                                             
            QMessageBox mb(this);
            mb.setWindowTitle(tr("falconG - Question"));
            mb.setText(tr("Move into this folder or move before it?"));
            mb.setInformativeText(tr("Press 'Cancel' to discard possible position changes."));
            // buttons added after the existing buttons
            QPushButton *pBeforeFolderBtn = mb.addButton(tr("Reposition"), QMessageBox::NoRole);
            QPushButton *pIntoFolderBtn = mb.addButton(tr("Move into"), QMessageBox::YesRole);

#ifdef DEBUG
            QPushButton *pCancelBtn = 
#endif
                mb.addButton(tr("Cancel"), QMessageBox::RejectRole);
            mb.setDefaultButton(pBeforeFolderBtn);

            mb.exec();
            QAbstractButton* pResBtn = mb.clickedButton();

            if (pResBtn == pIntoFolderBtn)
            {
                Album* pDestAlbum = &albumgen.Albums()[items[row]];
                IdList itemsDest = pDestAlbum->items;
#ifdef DEBUG
                int destItemSize = itemsDest.size();
#endif
                // ------------- housekeeping -----------
                // add moved items to destination album  (which doesn't have icons yet )
                for (int si = 0; si < thl.size(); ++si)
                {
                    ID_t itemID = items[thl[si]];
                    if (itemID.IsAlbum())
                    {
                        Album* pab = albumgen.AlbumForID(itemID);
                        Q_ASSERT(pab);
						pab->parentId = pDestAlbum->ID;            // reparent album
                    }
					pDestAlbum->items.push_back(itemID);
                    itemsDest.push_back(itemID);
                }

                // remove indexes of moved items from source gallery and icon indices
                const QVector<int> &origIconOrder = fileIcons.IconOrder();  // original order of icons for actual album
                QVector<int> newIconOrder;                       // new icon order indexes
				IdList newItems;                                 // w.o. moved items
                
                for (int si = 0; si < itemSize; ++si)            // origIconOrder has the same number of items as items itself
                {
                    int o = origIconOrder[si];
                    if (thl.indexOf(o) < 0)
                    {
                        newItems.push_back(items[o]);
                        newIconOrder.push_back(o);
                    }
                }
                // discard moved icons
                //for (int si = itemSize - 1; si << itemSize >= 0; --si)   // origIconOrder has the same number of items as items itself
                //    if (thl.indexOf(origIconOrder[si]) >= 0)
                //        fileIcons.Remove(origIconOrder[si]);
                items = newItems;
                fileIcons.SetIconOrder(newIconOrder);
                _InitThumbs();
                            // ----- never  move items physically to virtual destination folder --------
            }
            else if( pResBtn == pBeforeFolderBtn)
                moveItemsIntoFolder = false;
            // else cancel is pressed
        }

        if(!moveItemsIntoFolder)    // = just relocate, move before selected item (or after the last one)
        {
            itemOrder.resize(itemSize);     // and original indexes are 0,1,2...

            int si = 0,     // original index
                di = 0;     // index in idl

            // here row is: when >= 0 -> row to insert items before, when < 0 -> move to the end
            if (row < 0)
                row = itemSize;

            for (; si < itemSize && si < row; ++si)
                if (thl.indexOf(si) < 0)
                    itemOrder[di++] = si;

            for (int i = 0; i < thl.size(); ++i)
                itemOrder[di++] = thl[i];

            for (si = row; si < itemSize; ++si)
                if (thl.indexOf(si) < 0)
                    itemOrder[di++] = si;

            // new order in 'itemOrder' set
            IdList idl;                         // new ordered items
            idl.resize(itemSize);

            for (int i = 0; i < itemOrder.size(); ++i)
                idl[i] = items[itemOrder[i]];

            // modify original stored itemOrder
            const QVector<int> &origIconOrder = fileIcons.IconOrder();  // original order might have been changed
            // so we must rearrange that according to 'itemOrder'
            QVector<int> iconOrder;                          // new icon order indexes
            iconOrder.resize(itemSize);
            for (int i = 0; i < itemOrder.size(); ++i)
                iconOrder[i] = origIconOrder[itemOrder[i]];

            fileIcons.SetIconOrder(iconOrder);
            items = idl;
        }
        albumgen.SetAlbumModified(*pAlbum);

        albumgen.WriteDirStruct(true);
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
    _doAbortThumbsLoading = true;
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
        if (_doAbortThumbsLoading || firstVisible < 0 || lastVisible < 0) {
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
        if (_doAbortThumbsLoading) 
            break;
        
    }
  FINISHED:
#endif
//	emit SignalInProcessing(false);
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
    _doAbortThumbsLoading = false;
    _isProcessing = true;
    _thumbsRangeFirst = -1;
    _thumbsRangeLast = -1;

    _isBusy = true;
    _thumbnailViewModel->Clear(); // clearing occurs between 'beginResetModel()' and 'endResetModel()'
    emit SignalInProcessing(true);
//    fileIcons.Clear();

    setSpacing(QFontMetrics(font()).height());

    _InitThumbs();
    if (_isProcessing)
    {
        _UpdateThumbsCount();
        loadVisibleThumbs(0);
        _isProcessing = false;
    }

    emit SignalInProcessing(false);
    emit SignalMayLoadNewItems();
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
     _albumId = TOPMOST_ALBUM_ID;
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
	int timeOutCnt = 1;
    Album &album = albumgen.Albums()[_albumId];
    if(album.pathId)
        albumgen.lastUsedAlbumPathId = album.pathId;

    _thumbnailViewModel->BeginResetModel();
    _thumbnailViewModel->Clear();
	for (fileIndex = 0; !_doAbortThumbsLoading && fileIndex < album.items.size(); ++fileIndex)
	{
	    thumbItem = new ThumbnailItem(fileIndex, album.ID, _TypeFor(album.items[fileIndex]));
		_thumbnailViewModel->appendRow(thumbItem);
		++timeOutCnt;
		if (timeOutCnt > 10)
		{
			timeOutCnt = 1;
			QApplication::processEvents();
		}
	}
    _thumbnailViewModel->EndResetModel();
    if (_doAbortThumbsLoading)
    {
        _isProcessing = _doAbortThumbsLoading = false;  // aborted
        emit SignalMayLoadNewItems();
    }
    else
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
 *          are acceptable, just looks at the allowed types
 *------------------------------------------------------------*/
bool ThumbnailView::_IsAllowedTypeToDrop(const QDropEvent *event)
{
    // DEBUG
    //qDebug() << "Mime text: " << event->mimeData()->text() 
    //         << ", hasUrls ? " << event->mimeData()->hasUrls()
    //         << "mimeData is null?" << (event->mimeData() ? "no":"yes")
    //         << ", hasImage ? " << event->mimeData()->hasImage()
    //         << ", x-thumb ? " << event->mimeData()->hasFormat("application/x-thumb")
    //    ;
    // qDebug() << "IsAllowedTypeToDrop: row=" << indexAt(event->pos()).row();

    // DEBUG
    QString qs = event->mimeData()->text();
    /*bool b = event->mimeData()->hasUrls(),
        b1 = event->mimeData()->hasImage(),
        b3 = event->mimeData()->hasFormat("application/x-thumb");*/
    // /DEBUG
     return /* (indexAt(event->pos()).row() >= 0) && */
            (event->mimeData()->hasUrls() ||    // e.g. text() == file:///I:/alma.jpg, or text() == file:///I:/folderName, each nami in its own line
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
    for (auto &folderName: qslFolders)
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
        _doAbortThumbsLoading = true;
        QTimer::singleShot(0, this, SLOT(loadThumbsRange()));
        return;
    }

    isInProgress = true;
    _currentItemCount = _thumbnailViewModel->rowCount();

    for (_scrolledForward ? currThumb = _thumbsRangeFirst : currThumb = _thumbsRangeLast;
         (_scrolledForward ? currThumb <= _thumbsRangeLast : currThumb >= _thumbsRangeFirst);
         _scrolledForward ? ++currThumb : --currThumb) 
	{

        if (_doAbortThumbsLoading || _thumbnailViewModel->rowCount() != _currentItemCount || currThumb < 0)
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
    _doAbortThumbsLoading = false;
}

/*=============================================================
 * TASK:	adds a ThumbnailItem for a thumbnail to either 
 *          the end of the rows or before the selected item
 * EXPECTS:	_albumID already set
 *          which - ordinal of data for thumbnail in
 *					albumgen.Albums()[_albumID]'s 
 *                  image, video or album lists
 *          typ   - which list should be used
 * GLOBALS: _albumId set
 * RETURNS:	nothing
 * REMARKS:	- sets albums to changed
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

    albumgen.SetAlbumModified(_albumId);
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
	switch (event->button())
	{
	    case Qt::LeftButton:
		    _rowSelectedWithLeftButton = indexAt(event->pos()).row();
		    break;
	    case Qt::RightButton:
		    _rowSelectedWithRightButton = indexAt(event->pos()).row();
		    break;
        case Qt::BackButton:
			emit SignalBackToParentAlbum();
			break;
        case Qt::ForwardButton:
            break;
	}

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
            pact = new QAction(tr("Select As Album Thumbnail..."), this);  // any image
            connect(pact, &QAction::triggered, this, &ThumbnailView::SelectAsAlbumThumbnail);
            menu.addAction(pact);

            menu.addSeparator();
        }
    }
    else // album(s) or image(s) is(are) selected
    {
        Album& album = albumgen.Albums()[_albumId];

        if (nSelSize == 1)
        {
            int pos = currentIndex().row();     
            
            ID_t id = album.items[pos];
            IABase* pItem = albumgen.IdToItem(id);
            if (id.IsImage())
                pItem = albumgen.ImageAt(id);
            else if (id.IsVideo())
                pItem = albumgen.VideoAt(id);
            else
                pItem = albumgen.AlbumForID(id);

            bool exists = id.IsAlbum() ? true : QFile::exists(pItem->FullSourceName());

            if (exists)
            {
                if (pItem->ID.IsAlbum())
                {
                    pact = new QAction(tr("Bro&wse for Album Thumbnail..."), this);
                    connect(pact, &QAction::triggered, this, &ThumbnailView::OpenAlbumThumbnail);
                    menu.addAction(pact);           // select any image
                }
                if(!pItem->ID.IsAlbum() || (pItem->ID.IsAlbum() && albumgen.ImageAt(reinterpret_cast<Album*>(pItem)->thumbnailId)->Exists()))
				{
					pact = new QAction(tr("Set As &Thumbnail for Container"), this);
					menu.addAction(pact);           // select image of this item
					connect(pact, &QAction::triggered, this, &ThumbnailView::SetAsAlbumThumbnail);
					menu.addAction(pact);           // select any image
				}
                menu.addSeparator();
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
            
            QModelIndexList list = selectionModel()->selectedIndexes();
            ID_t id;
            for (auto &i : list) // if at least a single image then this is valid
            {
                id = album.items[i.row()];

                if (id.IsImage())
                {
                    pact = new QAction(tr("Toggle 'Keep Original Size'"));
                    connect(pact, &QAction::triggered, this, &ThumbnailView::ToggleDontResizeFlag);
                    menu.addAction(pact);           // select any image
                    break;
                }
            }
            menu.addSeparator();

            pact = new QAction(tr("Copy &Name(s)"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::CopyNamesToClipboard);
            menu.addAction(pact);

            pact = new QAction(tr("Copy &Original Name(s)"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::CopyOriginalNamesToClipboard);
            menu.addAction(pact);

            menu.addSeparator();
        }
    }
	pact = new QAction(tr("&New Folder..."), this);  // create new folder inside actual folder
	connect(pact, &QAction::triggered, this, &ThumbnailView::NewVirtualFolder);
	menu.addAction(pact);

    menu.addSeparator();
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
 * TASK: Delete images in list from image list of actual album
 *		 into undo buffer
 * EXPECTS: albumId - id of album whose child/children are to be removed
 *          list    - list of IDs to remove from album's items
 *          iconsForThisAlbum - should we remove the icon(s) too?
 * GLOBALS:
 * RETURNS:
 * REMARKS: does not delete generated images, videos and albums 
 *          from disk!
 *          Tries use the recycle bin (windows) or the trash (mac)
 *------------------------------------------------------------*/
void ThumbnailView::SlotDeleteSelectedList(ID_t albumId, IntList& list, bool iconsForThisAlbum)
{
    QString s;

    int res = DeleteOrRemoveConfirmationDialog(list, this);
    if (res == 2)
        return;

    bool fromDisk = res == 1;       //  Yes
            // needs reverse order   (?)

    albumgen.RemoveItems(albumId, list, fromDisk, iconsForThisAlbum);  // also remove cached file icons
    _albumId = albumId;
    Reload();
    //    emit selectionChanged(QItemSelection(), QItemSelection());
}

/*=============================================================
 * TASK: Delete selected images from image list of actual album
 *		 into undo buffer
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: does not delete generated images, videos and albums 
 *          from disk!
 *          Tries use the recycle bin (windows) or the trash (mac)
 *------------------------------------------------------------*/
    class FalconG;           // needed here for message box, so no whole falconG.h is included
void ThumbnailView::DeleteSelected()
{
	QModelIndexList list = selectionModel()->selectedIndexes();
    if (list.isEmpty())
        return;
    IntList ilx(list.size());
    for (int i = list.size() - 1; i >= 0; --i)
        ilx[i] = list[i].row();

    SlotDeleteSelectedList(_albumId, ilx, true);
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

    IABase* pItem = nullptr;
    Album* pAlbum = _ActAlbum();

    auto Item = [&](int row)
    {
        ID_t id = pAlbum->items[row];
        if (id.IsAlbum())
            pItem = albumgen.AlbumForID(id);
        else if (id.IsImage())
            pItem = albumgen.ImageAt(id);
        else
            pItem = albumgen.VideoAt(id);

        return pItem;
    };

    emit SignalSingleSelection(Item(_rowSelectedWithRightButton)->ID, _albumId);

    if (QuestionDialog(tr("falconG - Question"),
        tr("This will set the texts to all of selected items\n"
            "to be the same as the item under the cursor when\n"
            "you choose this menu option\n\n"
            "This action cannot be undone!\n\n"
            "Do you really want to do this?"), DialogBitsOrder::dboAskSynchronize, this,
        tr("Don't ask again (use Options to re-enable)")) == QMessageBox::Yes)
    {

        int64_t titleId=0,descrId=0;
        LanguageTexts *pTitle = nullptr, *pDescr = nullptr;

        for(int i =0; i < list.size(); ++i)
            if (list[i].row() == _rowSelectedWithRightButton)   
            {
                pItem = Item(list[i].row());
                titleId = pItem->titleID;
                descrId = pItem->descID;
                pTitle = albumgen.TextsAt(titleId);
                pDescr = albumgen.TextsAt(descrId);
            }

        for (auto &it : list)
        {
            if (it.row() != _rowSelectedWithRightButton)
            {
                pItem = Item(it.row());
                pItem->titleID = titleId;
                pItem->descID = descrId;
                if (pTitle) 
                    ++pTitle->usageCount;
                if (pDescr) 
                    ++pDescr->usageCount;
            }
        }
        SetCurrentItem(_rowSelectedWithRightButton);
    }
    else    // clear save the state when the answer is no
        config.doNotShowTheseDialogs.v = config.doNotShowTheseDialogs.v & (~(1 << DialogBitsOrder::dboAskSynchronize));

    albumgen.SetAlbumModified(*pAlbum);
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
    QString dir = pathMap.AbsPath(albumgen.lastUsedAlbumPathId)+_ActAlbum()->name;
    QStringList qslFileNames = QFileDialog::getOpenFileNames(this, tr("falconG - Add images/videos"), dir, "Images(*.bmp *.jpg *.png);;Videos(*.mp4,*.ogg);;All files(*.*)");
    if (qslFileNames.isEmpty())
        return;

    int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;

    emit SignalInProcessing(true);
    _AddImagesFromList(qslFileNames, pos);
    // now add last used path
	int siz = qslFileNames.size();
    QString s;
	SeparateFileNamePath(qslFileNames[siz - 1], dir,s);
    bool added=false;
//	albumgen.lastUsedAlbumPathId = albumgen.Albums().Add(_ActAlbum()->ID, dir, added).Val();
    Reload();
    albumgen.WriteDirStruct(true);

    emit SignalInProcessing(false);
    emit SignalAlbumChanged();
    emit selectionChanged(QItemSelection(), QItemSelection());
    config.dsLastImageDir = dir;
}


/*=============================================================
 * TASK:    internal function that adds a folder recursively
 * PARAMS:  folderName - name of folder to add
 * GLOBALS:
 * RETURNS: if folder was added
 * REMARKS: 
 *------------------------------------------------------------*/
bool ThumbnailView::_AddFolder(QString folderName)
{
    Album* pParentAlbum = _ActAlbum();

    bool added, atLeastOneFolderWasAdded = false;
    emit SignalInProcessing(true);
    emit SignalAlbumStructWillChange();
    ID_t id = albumgen.Albums().Add(pParentAlbum->ID,folderName,added);
    if (added)
    {   
        atLeastOneFolderWasAdded = true;
        pParentAlbum = _ActAlbum();     // album position may have changed when new album was added to map
        albumgen.SetAlbumModified(*pParentAlbum);
        Album &album = *albumgen.AlbumForID(id);
        album.parentId = _albumId;
        albumgen.AddDirsRecursively(id);

        int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;
        pParentAlbum->AddItem(id);

        ID_t idth = album.ThumbID();
        if (idth.Val())
            folderName.clear();
        else
            folderName = albumgen.Images()[idth].FullSourceName();
        (void)fileIcons.Insert(pos, true,folderName);

        albumgen.WriteDirStruct(true);
    }

    emit SignalInProcessing(false);

    return atLeastOneFolderWasAdded;
}

/*=============================================================
 * TASK:    internal function that creates a folder
 * PARAMS:  folderName - name of folder to add w.o. any path
 * GLOBALS:
 * RETURNS: if folder was added
 * REMARKS: folder names must be unique
 *------------------------------------------------------------*/
bool ThumbnailView::_NewVirtualFolder(QString folderName)
{
    auto errMsg = [&](QString text)
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr( "Adding new album failed!\n\n%1\n"
                                                                    "Please use a different name!\n\n"
                                                                    "Album names must be unique.").arg(text));
        };

    bool added = false,
         res = false;

    Album* pParentAlbum = _ActAlbum();
    ID_t id = albumgen.Albums().Add(pParentAlbum->ID, folderName, added);

    //QString fullPath = parentPath + "/" + folderName;
    //if (albumgen.Albums().Exists(fullPath))
    //    errMsg(tr("The album is already in the gallery.") );
    //else
    //{
    //    QDir dir(parentPath);               
    //    // DEBUG
    //    qDebug("Dir:%s, subdir:%s", dir.absolutePath().toStdString().c_str(), folderName.toStdString().c_str());
    //    // /DEBUG
    //    // usually no real folder is created, all of it is logical if (dir.mkdir(fullPath))
    //    //{

            emit SignalInProcessing(true);
            emit SignalAlbumStructWillChange();
            if (added)
            {
                albumgen.SetAlbumModified(*pParentAlbum);
                Album& album = *albumgen.AlbumForID(id);
                album.parentId = _albumId;
                pParentAlbum->AddItem(id, -1);
                albumgen.WriteDirStruct(true);
                res = true;
            }
            else
                errMsg(tr("Unknown error: Folder created but wasn't added to gallery") );
            emit SignalInProcessing(false);
            emit SignalAlbumStructChanged(false);   // changes already written to disk (no backup was made nor will be)
        //}
        //else
        //   errMsg(tr("Creating folder \n%1\non disk was unsuccessful").arg(parent+"/"+folder));
//    }
    return res;
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
    QString dir = pParentAlbum->FullSourceName();
    QString qs = QFileDialog::getExistingDirectory(this, tr("falconG - Add Directory"), dir);
    if (qs.isEmpty())
        return;
    bool b = false;
    if ((b=_AddFolder(qs)) )
        Reload();
}

/*=============================================================
 * TASK:   Display dialog box to create one virtual folder in the 
 *          actual (virtual or real) folder
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - actual album is parent of new album folder
 *          - no thumbnail flag is set
 *------------------------------------------------------------*/
void ThumbnailView::NewVirtualFolder()
{
    Album* pParentAlbum = _ActAlbum();
    QString parentPath = pParentAlbum->FullSourceName();   // relative to config.dsSrc
    QString qs;
    bool ok;
    bool b = false;

    while (1)
    {
        qs = QInputDialog::getText(this, tr("falconG - New (Virtual) Folder"), tr("Folder Name:"), QLineEdit::Normal, qs, &ok);

        if(!ok || qs.isEmpty())
            return;
		if (qs.indexOf(QRegExp("[\\/:*?\"<>|]")) >= 0)
		{
			QMessageBox::warning(this, tr("falconG - Warning"), tr("Folder name contains invalid characters\nPlease use a different name!"));
			continue;
		}
        if ((b = _NewVirtualFolder(qs)) == true)
        {
//            Reload();
            break;
        }
        //qs = QString(tr("file or folder named \n'%1'\nalready exists\nPlease use a different name!")).arg(qs);
        //QMessageBox::warning(this, tr("falconG - Warning"), qs);
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
    IABase* pItem;
    for (auto &i : list)
    {
        ID_t id = _ActAlbum()->items[i.row()];// ID_t(i.internalPointer());
        pItem = albumgen.IdToItem(id);
        
        switch (pItem->Type())
        {
            case IABase::iatImage: 
            case IABase::iatVideo: 
                s += QString().setNum(id.Val()) + pItem->Extension() + "\n";
                break;
            default:
            case IABase::iatAlbum:
                s += config.sBaseName.ToString() + "\n";
                break;
        }
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
	for (auto &i : list)
		s += selectionModel()->model()->data(i, SourcePathNameRole).toString() + "\n";
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->clear();
	clipboard->setText(s);
}

/*=============================================================
 * TASK   : opens any image to be used as thumbnail of an album
 *          no matter if the album item already has a thumbnail
 * PARAMS :
 * EXPECTS: popup menu called for an album inside the actual one,
 *          which always has items
 * GLOBALS: albumgen
 * RETURNS: nothing
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::OpenAlbumThumbnail()
{
    int pos = currentIndex().row();     // new thumbnail index for actual item
    ID_t parentId = _albumId;
    Album* pParent =  &albumgen.Albums()[_albumId]; 
    // get actual selected album inside parent
    Album *album = pParent ? &albumgen.Albums()[ pParent->items[pos]] : &albumgen.Albums()[_albumId];
    QString s = album->FullSourceName();

    QString filter = tr("Image files (*.bmp *.gif *.jpg *.png)");

    s = QFileDialog::getOpenFileName(this, tr("falconG - Open file for Thumbnail"), s, filter);
                                            
    if (!s.isEmpty())
    {
        bool added;
        ID_t id = albumgen.Images().Add(s, added, true);      // will not add it when it is already in data base
        album->SetThumbnail(id);                              // increases image's thumbnailCount
        Reload();
    }
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
        if (album.thumbnailId == album.items[i])
            cthix = i;
    if (cthix == pos)       // no change
        return;

    ID_t th = album.items[pos];

    album.SetThumbnail(th.IsAlbum() ? albumgen.Albums()[th].thumbnailId : albumgen.ImageAt(th)->ID );
    albumgen.SetAlbumModified(_albumId);
    if(album.parentId.Val())
        albumgen.SetAlbumModified(album.parentId);
    emit SignalAlbumChanged();
}

/*=============================================================
 * TASK:    toggles images don't resize flag
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::ToggleDontResizeFlag()
{
    Album& album = albumgen.Albums()[_albumId];
    bool changed = false;

    QModelIndexList list = selectionModel()->selectedIndexes();
    emit SignalAlbumStructWillChange();

    Image* pImage;
    for (auto &i : list)
    {
        ID_t id = album.items[i.row()];
        if (id.IsImage())     // toggle original size for image
        {
            pImage = albumgen.ImageAt(id);
            pImage->dontResize = !pImage->dontResize;
            changed = true;
        }
    }
    emit SignalAlbumStructChanged(changed);
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
    QString path = pathMap.AbsPath(albumgen.Images().lastUsedPathId);
    QString thname = QFileDialog::getOpenFileName(this, 
                                            tr("falconG - Select Thumbnail Image"),
                                            path, 
                                            "Image files (*.jpg *.png)");
    if (thname.isEmpty())
        return;

    QString qsd;
    QString name;
    SeparateFileNamePath(thname, qsd, name);
    albumgen.Images().lastUsedPathId = pathMap.Add(qsd);
    // add as thumbnail to this album, but do not signal elapsed time and do not add to items
    albumgen.AddItemToAlbum(_albumId, thname, true, false, true);

    Album& album = *_ActAlbum();
    albumgen.SetAlbumModified(album);

    if (album.parentId.Val())
    {
        Album* parent = albumgen.AlbumForID(album.parentId);
        albumgen.SetAlbumModified(*parent);
    }
    emit SignalAlbumChanged();
}


void ThumbnailView::ItemDoubleClicked(const QModelIndex& mix)
{
//    _currentIndex = mix;
    int row = mix.row();
    ID_t id = _ActAlbum()->items[row];
    if (id.IsAlbum())
    {
        emit SignalFolderChanged(row);
    }
    else        // display full image in window      
    {

        IABase *pb = id.IsImage() ? (IABase*)albumgen.ImageAt(id) : (IABase*)albumgen.VideoAt(id);
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

void ThumbnailView::SlotThumbnailSizeChanged(int newSize)
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
    IABase* pItem = albumgen.IdToItem(id);

    auto __getNameAndPath = [&](ID_t id, QString& n, QString& p) {
		n = pItem->name;
		p = pItem->Path();
	};

    auto __pathindex = [&]() {
        for(int i = 0; i < _slSearchPaths.size(); ++i)
            if (QFile::exists(_slSearchPaths[i] + pItem->name))
                return i;
        return -1;
    };
    auto __namesMatch = [&](QString &foundName, QString &p, QString &n) {
        SeparateFileNamePath(foundName, p, n);
        if (n != pItem->name)
            return false;
        return true;

    };

    auto __setPathId = [&](const QString &p)
        {
            pItem->pathId = pathMap.Add(p); // doesn't add it when it has already benn added
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
        QString n, p;
        if(!__namesMatch(foundName, p, n))
        {
            if (QMessageBox::question(this, tr("falconG - Warning"), tr("File names do not match.\nDo you accept the new name?")) == QMessageBox::No)
                return;
            pItem->name = n;    // new name accepted
        }
        _slSearchPaths.push_back(p);
        __setPathId(p);

        pItem->ID.SetFlag(EXISTING_FLAG, QFile::exists(pItem->FullSourceName()) );  // must always be exExists because of dialog settings
        pItem->changed = true;
        albumgen.SetAlbumModified(album);

        // check all missing files against all search paths collected so far
        for (auto &id1 : album.items)
        {
            pItem = albumgen.IdToItem(id1);
            if (!QFile::exists(pItem->FullSourceName()))
            {
                int j = __pathindex();
                if (j >= 0)
                {
                    __setPathId(_slSearchPaths[j] );
                    pItem->ID.SetFlag(EXISTING_FLAG);
                }
            }
        }
        emit SignalAlbumChanged();
        Reload();
    }
}

void ThumbnailView::SlotToCloseAllViewers()
{
    _RemoveAllViewers();
}

void ThumbnailView::SlotGetSelectionCount(ID_t &id, int &count)
{
    id = _albumId;
    count = selectionModel()->selectedIndexes().size();
}

//void ThumbnailView::SlotToClearIconList()
//{
//    fileIcons.Clear();
//}

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
