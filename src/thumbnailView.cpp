/*
 * part of the code is taken from Ofer Kashayov's <oferkv@live.com>
 * free Phototonic image viewer
 * My code is marked with SA in the remarks of the function descriptions
 * Ofer Kashayov's original copyright notice:
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

#include "config.h"
#include "thumbnailView.h"
#include "imageviewer.h"
#include "treeView.h"
#include "falconG.h"

#include "videoplayer.h"

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

inline bool FileIcons::HasIconFor(int pos)
{
    return pos < _iconList.size();
}

void FileIcons::SetFolderThumbnailPosition(int pos, bool bIsFolderThumbnail)
{
    // pos-th item MUST exist

    MarkedIcon& micon = _iconList[ _iconOrder[pos] ];

    bool b = micon.flags.testFlag(fiThumb);
    if(posFolderIcon == pos && bIsFolderThumbnail == b)
		return;    // already set

	if (posFolderIcon >= 0 && b)   // erase original folder icon
		micon.flags ^= fiThumb;    // remove fiThumb flag

    if (bIsFolderThumbnail)
    {
		micon.flags |= fiThumb;    // set fiThumb flag
        posFolderIcon = pos;
    }
}

QIcon FileIcons::IconForPosition(int pos, IconFlags flags, QString imageName)
{
	if (pos < 0)
		return QIcon();

	if (pos >= _iconList.size())        // _iconList may contain more items than _iconOrder
        Insert(-1, imageName, flags);   // -1: push back

    // pos-th item MUST exist
    Q_ASSERT(pos < _iconOrder.size());

    MarkedIcon& micon = _iconList[_iconOrder[pos]];
    // DEBUG
  //  if(micon.flags.testFlag(fiDontResize))
		//qDebug((QString("IconForPosition: pos:%1, name:'%2', flags: %3").arg(pos).arg(imageName).arg((int)flags)).toStdString().c_str());
    // /DEBUG

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
MarkedIcon * FileIcons::Insert(int pos, QString imageName, IconFlags flags)
{
	MarkedIcon icon;
    FileType type = FileTypeFromName(imageName);
    if(flags == IconFlag::fiNone)
        flags.setFlag(type == ftImage ? fiImage : type==ftVideo ? fiVideo : type == ftFolder ? fiFolder : fiNone);
	icon.Read(imageName, flags); // for videos: get thumb frame

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

ThumbnailItem::ThumbnailItem(int pos, IDVal_t ownerID, Type typ, QIcon icon) : QStandardItem(pos, 1), _itemType(typ), _ownerIdVal(ownerID), itemPos(pos)
{
    QSize hintSize = QSize(thumbHeight, thumbHeight + ((int)(QFontMetrics(font()).height() * 1.5)));

    setTextAlignment(Qt::AlignTop | Qt::AlignHCenter);
}

QIcon ThumbnailItem::IconForFile() const
{
    if (fileIcons.HasIconFor(itemPos))
        return fileIcons.IconForPosition(itemPos);

    IconFlags flags;

    Album* pAlbum = _ActOwner();

    ID_t itemId = pAlbum->IdOfItem(itemPos);
    bool exists = false;
    

    if(itemId.IsVideo())       // inside of pAlbum
        flags.setFlag(IconFlag::fiVideo);
    else if(itemId.IsImage())  // inside of pAlbum
        flags.setFlag(IconFlag::fiImage);
    else if(itemId.IsAlbum())  // inside of pAlbum
		flags.setFlag(IconFlag::fiFolder);

	auto ISTHUMB = [flags]()->bool { return flags.testFlag(IconFlag::fiThumb); }; // thumbnail for pAlbum
               // for items:
	auto ISALBUM = [flags]()->bool { return flags.testFlag(IconFlag::fiFolder); }; 
    auto ISIMAGE = [flags]()->bool { return flags.testFlag(IconFlag::fiImage); };
    auto ISVIDEO = [flags]()->bool { return flags.testFlag(IconFlag::fiVideo); };

    bool isAlias = false;                  // for another item

    Image img;
    ID_t imgId = itemId;     // at this point it may be the ID of a folder, an image or a video

    if(pAlbum->thumbnailId == imgId)
		flags.setFlag(fiThumb);    // this is the thumbnail of the album
    QString imageName;
    if (ISALBUM() )
    {
        Album& aitem = albumgen.Albums()[itemId];
        imgId = aitem.thumbnailId;

        if (aitem.baseAlbumId)
            isAlias = true;
    }
    if (albumgen.ImageAt(imgId)->dontResize)
        flags.setFlag(fiDontResize);

            // else read and create icon
            // CHECFicon.rK code!
    if (ISVIDEO())
    {
       imageName = albumgen.VideoAt(imgId)->FullLinkName();
        if (!QFile::exists(imageName))
            imageName = QString(":/Preview/Resources/NoImage.jpg");
    }
    else        // for image or album: imgId is set, videos: TODO
    {
        img = albumgen.Images()[imgId];
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
    }
    // DEBUG
    // qDebug((QString("icon for file:'%1' of ID:%2, name:'%3'").arg(imageName).arg(itemId).arg(img.FullSourceName())).toStdString().c_str());
    // /DEBUG

    return fileIcons.IconForPosition(itemPos, flags, imageName);
}


QVariant ThumbnailItem::data(int role) const      // based on _itemType
{
    switch (role)
    {
        case Qt::DisplayRole:   return DisplayName();       // destination (generated) file or directory name w.o. path
        case Qt::DecorationRole: return IconForFile();      // the icon of files or albums (used when QListView is set for it)
        case Qt::ToolTipRole:   return ToolTip();           // string of full source name and destination name
        case TypeRole:          return _itemType;           // folder, image or video
        case FilePathRole:      return FilePath();          // destination path
        case FileNameRole:      return FileName();          // destination (generated) file or directory name w.o. path or source path if no dest. file
        case SourcePathNameRole: return FullSourcePath();	// full path name of image source
        case FullNameRole:      return FullLinkName();      // full destination path name
    }
    return QVariant();
}

QString ThumbnailItem::_ImageToolTip() const    // only called for images
{
    Image* pImg = albumgen.ImageAt(_ActOwner()->IdOfItem(itemPos) );
    return QString(pImg->ShortSourcePathName() + " \n(" + pImg->LinkName(config.bLowerCaseImageExtensions) + ")");

}
QString ThumbnailItem::_VideoToolTip() const    // only called for videos
{
    Video* pVid = albumgen.VideoAt(_ActOwner()->IdOfItem(itemPos));
    return QString(pVid->ShortSourcePathName() + " \n(" + pVid->LinkName(config.bLowerCaseImageExtensions) + ")");
}
QString ThumbnailItem::_FolderToolTip() const   // only called for folders !
{
    Album* pAlbum = albumgen.AlbumForID(_ActOwner()->IdOfItem(itemPos));

    QString s = pAlbum->ShortSourcePathName() + " \n(" + pAlbum->LinkName(-1, true) + ")";
    if (pAlbum->baseAlbumId != NO_ID)
        s += QObject::tr("\n--- Alias for '%1'").arg(pAlbum->BaseAlbum()->ShortSourcePathName());
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
    Image* pImg = albumgen.ImageAt(_ActOwner()->IdOfItem(itemPos));
    if (!pImg)
        return QString();
    if (QFileInfo::exists(pImg->FullLinkName()))
        return QString(pImg->LinkName(config.bLowerCaseImageExtensions));
    else
        return pImg->name;      // source name
}

QString ThumbnailItem::_VideoFileName() const
{
    Video* pVid = albumgen.VideoAt(_ActOwner()->IdOfItem(itemPos));
    if (!pVid)
        return QString();

    if (QFileInfo::exists(pVid->FullLinkName()))
        return QString(pVid->LinkName(config.bLowerCaseImageExtensions));
    else
        return pVid->name;      // source name
}

QString ThumbnailItem::_FolderFileName() const
{
   Album  owner = albumgen.Albums()[ID_t(ALBUM_ID_FLAG, _ownerIdVal)];
   return owner.LinkName(-1, NO_ID); // no language and extension no base ID ?
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
    Image* pImg = albumgen.ImageAt(_ActOwner()->IdOfItemOfType(IMAGE_ID_FLAG, itemPos));
    return pImg->FullSourceName();
}
QString ThumbnailItem::_VideoFullSourceName() const
{
    Video* pVid = albumgen.VideoAt(_ActOwner()->IdOfItemOfType(VIDEO_ID_FLAG, itemPos));
    return pVid->FullSourceName();
}
QString ThumbnailItem::_FolderFullSourceName() const
{
    //Image* pImg = albumgen.ImageAt(_ActOwner()-->thumbnailId);
    //return pImg->FullSourceName();
    return albumgen.AlbumForID(_ActOwner()->IdOfItem(itemPos))->FullSourceName();
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
        case folder: return albumgen.AlbumForID(_ActOwner()->IdOfItem(itemPos))->name;
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
void ThumbnailView::SelectCurrentIndex()
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
int ThumbnailView::GetNextItem() const
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
int ThumbnailView::GetPrevItem() const
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
int ThumbnailView::GetLastItem() const
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
int ThumbnailView::GetRandomItem() const
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
int ThumbnailView::GetCurrentItem() const
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
bool ThumbnailView::SetCurrentIndexByItem(int row)
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
		emit SignalSingleSelection(_ActAlbum()->ID, _ActAlbumId());
    else if (selectedCount == 1)
	{
        SetCurrentItem(indexesList.first().row());
		emit SignalSingleSelection(_ActAlbum(true)->items[_currentItem], _ActAlbumId());
	}
    else
    {
        IdList idList, 
               &items = _ActAlbum(true)->items;
        for (auto &e : indexesList)
        {
            ID_t id = items[e.row()];
            if (idList.indexOf(id) < 0)
                idList << items[e.row()];

            emit SignalMultipleSelection(idList, _ActAlbumId());
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
    if (_isBusy)    // during load
        return;

    QModelIndexList indexesList = selectionModel()->selectedIndexes();
    if (indexesList.isEmpty()) 
        return;

    QDrag *drag = new QDrag(this);
    ThumbMimeData *mimeData = new ThumbMimeData;
    QList<QUrl> urls;	// create pixmap from this list of names
	for (auto &f : indexesList)
	{
	    ThumbnailItem thumbRec(f.row(), _albumId.Val(), (ThumbnailItem::Type)_thumbnailViewModel->item(f.row())->type());
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
static QString __DebugPrintDragAndDrop(QDragMoveEvent *event)    // QDragEnterEvent inherits this
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
    if (_isBusy)
        return;

    if (_IsAllowedTypeToDrop(event) )
	{
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
    if (_movetimer)
    {
        disconnect(_movetimer, nullptr, this, nullptr);
        delete _movetimer;
        _movetimer = nullptr;
    }
}



/*=============================================================
 * TASK:   handles dragged object move
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:QDragMoveEvent is a descendant of QDropEvent
 *------------------------------------------------------------*/
void ThumbnailView::dragMoveEvent(QDragMoveEvent * event)
{
    if (_isBusy)
        return;

    if (!_IsAllowedTypeToDrop(event))
		return;

	QModelIndex index = indexAt(event->pos());

	event->accept();
	int pose = event->pos().y();

    if (pose < 30)
    {
        if (!_movetimer)
        {
            _movetimer = new QTimer(this);
            QThread::msleep(200);
            connect(_movetimer, &QTimer::timeout, [&]() { verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub); });
            _movetimer->start(200);
        }

    }
    else if (pose > height() - 30)
    {
        if (!_movetimer)
        {
            _movetimer = new QTimer(this);
            QThread::msleep(200);               // works with both 4 parameters with 'this' or with 3 paramas without it as above
            connect(_movetimer, &QTimer::timeout, this, [&]() { verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd); });
            _movetimer->start(200);
        }
    }
    else if (_movetimer)
    {
		disconnect(_movetimer, nullptr, this, nullptr);
        delete _movetimer;
        _movetimer = nullptr;
    }
}

/*=============================================================
 * TASK:    drop files and folders from disk
 * PARAMS:  ThumbMimeData poiter containing an URL list
 * GLOBALS: _albumID
 * RETURNS:
 * REMARKS: - Files and folders are not moved physically
 *          - Files and folders already in gallery are not
 *              added again. 
 *          - can't drop anything onto alias albums
 *------------------------------------------------------------*/
void ThumbnailView::_DropFromExternalSource(const ThumbMimeData* mimeData, int row)
{
    if (_isBusy)
        return;

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
    _AddImagesAndVideosFromList(qsl, row, true);        // just new images, not duplicate ones
    // row += qsl.size();  // position for folders
    bool b = _AddFoldersFromList(qslF, row);
    emit SignalAlbumStructChanged(b);
    if(b)
        Reload();
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
 *          - when album(s) moved into another album checks 
 *              for clash which may occur because
 *              - the destination album may already contains
 *                either this album or a link to this album
 *------------------------------------------------------------*/
void ThumbnailView::dropEvent(QDropEvent * event)
{
	if (_isBusy || !_IsAllowedTypeToDrop(event))
		return;

	const ThumbMimeData *mimeData = qobject_cast<const ThumbMimeData*>(event->mimeData());
    if (!mimeData)      // why does it occur?
        return;
	// get drop position
	QModelIndex index = indexAt(event->pos());

    int row = index.row();      // -1: drop after the last item

    bool doWriteStructFile = true;
    if (mimeData->hasUrls())
        _DropFromExternalSource(mimeData, row);
    else  if (mimeData->hasFormat("application/x-thumb") && !((const ThumbMimeData *)mimeData)->thumbList.isEmpty()) // perform drops from thumbnail list
	{
        IntList thl = ((const ThumbMimeData*)mimeData)->thumbList;  // indices in actual album's items to move

        Album* pAlbum = const_cast<Album*>(_ActAlbum());
        Q_ASSERT(pAlbum);
        if(pAlbum->baseAlbumId != NO_ID) // then it is an alias for another album
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr("You cannot drop items into an alias album.\n"
                "Please switch to the original album '%1' first!").arg(pAlbum->BaseAlbum()->name));
			return;
		}
        IdList &items = pAlbum->items;   // original ordered items

        //IdList  droppedIds(thl.size());        // id for items at positions from 'thl'
        //for (int i = 0; i < thl.size(); ++i)
        //    droppedIds[i] = items[thl[i]];
        // reorder items of actual album
        int itemSize = items.size();     // only changes when items dropped on other albums

        QVector<int> itemOrder;             // new item order indexes
        bool moveItemsIntoFolder = row >= 0 && (items[row].IsAlbum());   // now just see if the items moved above an album

        if (moveItemsIntoFolder) // then possibly move items into album with id items[row]
        {                        // or relocate them or cancel operation                     
            QMessageBox mb(this);
            mb.setWindowTitle(tr("falconG - Question"));
            mb.setText(tr("Reposition selection before this folder or Move into it?"));
            mb.setInformativeText(tr("Press 'Cancel' to discard possible position changes."));
            // buttons added after the existing buttons
            QPushButton *pBeforeFolderBtn = mb.addButton(tr("Re&position"), QMessageBox::YesRole); // YesRole comes first, no after and cancel after that
            QPushButton *pIntoFolderBtn = mb.addButton(tr("&Move into"), QMessageBox::NoRole);     

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
                // -- check if any of the items to move is a folder and if it is then
                //    whether it or any of its siblings are already in the destination album
                for (int i = 0; i < thl.size(); ++i)
                {
                    ID_t itemID = items[thl[i]];
                    if (itemID.IsAlbum())
                    {
                        Album* pab = albumgen.AlbumForID(itemID);
                        Q_ASSERT(pab);

                        if(albumgen.IsCircular(pab, pAlbum) ) //         if (pab->BaseAlbum()->ID.Val() == pAlbum->BaseAlbum()->ID.Val())
                        {
                            QMessageBox::warning(this, tr("falconG - Warning"), tr("Ivalid move!\n"
                                "Album \n'%1'\n is either an alias for album\n'%2'\n"
                                "or they are aliases of the same album.\nCancelling move.").arg(pab->name).arg(pAlbum->name) );
                            return;
                        }

                                // now the album to be moved is not an alias of the album to move to.
                        IDVal_t isThere = NO_ID;
                        IDValList idvl = pab->BaseAlbum()->aliasesList; // list of albums linked to this album
                        if (idvl.isEmpty())     // no linked albums for this one
                            isThere = pDestAlbum->items.indexOf(itemID) >= 0 ? itemID.Val() : NO_ID; // check if album is already in destination album
                        else
                            for (auto& v : idvl)
                            {
                                ID_t idv(ALBUM_ID_FLAG, v);
                                if (pDestAlbum->items.indexOf(idv) >= 0)
                                {
                                    isThere = v;
                                    break;  // found at least one linked album in destination album
                                }
                            }

                        if (isThere != NO_ID)    // album is already in destination album
                        {
                            QString qs = tr("The album '%1' to be moved is already in the destination album '%2'.\n").arg(pab->name).arg(pDestAlbum->name);
                            if (isThere != itemID.Val())             // another alias for the same album
                                qs += tr("under the name '%1'\n").arg(albumgen.AlbumForIDVal(isThere)->name);
                            qs += tr("Please remove it from the selection!");
                            QMessageBox::warning(this, tr("falconG - Warning"), qs);
                            return;
                        }
                    }
                }

                // ------------- housekeeping -----------
                // add moved items to destination album  (which doesn't have icons yet )
                for (int si = 0; si < thl.size(); ++si)
                {
                    ID_t itemID = items[thl[si]];
                    if (itemID.IsAlbum())
                    {
                        Album* pab = albumgen.AlbumForID(itemID);
                        Q_ASSERT(pab);
                        pab->parentId = pDestAlbum->ID.Val();      // reparent album
                        albumgen.AddToModifiedList(itemID, true);   // so that it gets written 
                    }
                    if(!itemsDest.contains(itemID))
                        itemsDest.push_back(itemID);
                }
                pDestAlbum->items = itemsDest;

                // remove indexes of moved items from source gallery and icon indices
                const QVector<int>& origIconOrder = fileIcons.IconOrder();  // original order of icons for actual album
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
            else if (pResBtn == pBeforeFolderBtn)
                moveItemsIntoFolder = false;
            else        // else cancel is pressed
                doWriteStructFile = false;

        }

        if(doWriteStructFile && !moveItemsIntoFolder)    // = just relocate, move before selected item (or after the last one)
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
        if (doWriteStructFile)
        {
            albumgen.AddToModifiedList(*pAlbum);

            albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);
        }
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
void ThumbnailView::Abort()
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

void ThumbnailView::UpdateTreeView(bool forParentOfCurrentIndex)
{
    AlbumTreeView* ptrv = frmMain->GetTreeViewPointer(); // signal it on the treeView too
    QModelIndex mx = ptrv->currentIndex(), pmx = mx.parent().isValid() ? mx.parent() : QModelIndex();
    if (forParentOfCurrentIndex)
    {
        if (pmx.isValid())
			ptrv->update(pmx);      // show changed flag on parent
    }
    else
        ptrv->update(mx);           // show changed flag on actual index
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

/*=============================================================
 * TASK   : load and prepare thumbnails
 * PARAMS : none
 * EXPECTS:
 * GLOBALS:
 * RETURNS: nothing
 * REMARKS: -_albumId is the id of the album to load, but it can
 *            be an alias for another album. If it is then use
 *            the real album for the items
 *------------------------------------------------------------*/
void ThumbnailView::_InitThumbs()
{
	static QStandardItem *thumbItem;
	static int fileIndex;
	static QSize hintSize;
	int timeOutCnt = 1;
    Album &album = albumgen.Albums()[_albumId];
    Album* pBase = album.baseAlbumId != NO_ID ? albumgen.AlbumForIDVal(album.baseAlbumId) : &album;
    
    if(pBase->pathId)
        albumgen.lastUsedAlbumPathId = pBase->pathId;

    _thumbnailViewModel->BeginResetModel();
    _thumbnailViewModel->Clear();
	for (fileIndex = 0; !_doAbortThumbsLoading && fileIndex < pBase->items.size(); ++fileIndex)
	{
	    thumbItem = new ThumbnailItem(fileIndex, pBase->ID.Val(), _TypeFor(pBase->items[fileIndex]));
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
	    	SelectThumbByItem(0);   // in thumbnailView
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
 *          onlyNew - do not add duplicates to the same album
 * GLOBALS:
 * RETURNS:
 * REMARKS: - no thumbnail flag is set for any of the icons
 *          - the same image may appear in any number of albums,
 *             but not twice in the same album
 *------------------------------------------------------------*/
void ThumbnailView::_AddImagesAndVideosFromList(QStringList qslFileNames,int row, bool onlyNew)
{
	AlbumGenerator::AddedStatus ares = AlbumGenerator::AddedStatus::asAdded;
    int notAddedCnt = 0, errcnt = 0;
	int i;
	Album& thisAlbum = albumgen.Albums()[_albumId];
	Album& album = *thisAlbum.BaseAlbum();  // same as thisAlbum for non alias albums

	for (i = 0; i < qslFileNames.size(); ++i)
	{
		ares = albumgen.AddImageOrVideoFromString(qslFileNames[i], album, onlyNew, row);
        if (ares == AlbumGenerator::AddedStatus::asFailed)     // then already used somewehere // TODO: virtual albums
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr("Adding new image / video failed!"));
            ++errcnt;
            continue;
        }
        else if (ares == AlbumGenerator::AddedStatus::asAdded)       // do not add duplicates
            (void)fileIcons.Insert(row, qslFileNames[i]);
        else
            ++notAddedCnt;
	}
    if (notAddedCnt)
    {
        QString msg = tr("Added %1 items, not added %2 duplicates.").arg(i-errcnt-notAddedCnt).arg(notAddedCnt);
        if (errcnt)
            msg += tr("\nFailed to add %1 items").arg(errcnt);
        QMessageBox::information(this, tr("falconG - Information"), msg);

    }
    else if(errcnt)
            QMessageBox::warning(this, tr("falconG - Warning"), tr("Adding %1 new image%2 / video%2 failed!").arg(errcnt).arg(errcnt > 1 ? "s" : ""));
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
void ThumbnailView::SelectThumbByItem(int row)
{
    SetCurrentIndexByItem(row);          // in thumbnailView
    SelectCurrentIndex();                // in thumbnailView
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
 * GLOBALS: _ownerIdVal set
 * RETURNS:	nothing
 * REMARKS:	- sets albums to changed
 *------------------------------------------------------------*/
void ThumbnailView::AddThumb(int which, ThumbnailItem::Type type)
{
    ThumbnailItem *thumbItem = new ThumbnailItem(which, _albumId.Val(), type);
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

    albumgen.AddToModifiedList(_albumId,true);
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
    Album& album = albumgen.Albums()[_albumId];
    if (album.baseAlbumId == NO_ID && currentIndex().isValid() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
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

    int step = verticalScrollBar()->maximum() / _ActAlbum(true)->items.size() * width() / (ThumbnailItem::thumbHeight + spacing());

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
 * REMARKS: - items can't be added to nor deleted from or moved
 *            outside of an alias album
 *          - title and description however can be modified even
 *            in an alias album
 *          - menus depend on how many and what kind of items 
 *            are selected
 *------------------------------------------------------------*/
void ThumbnailView::contextMenuEvent(QContextMenuEvent * pevent)
{
	QMenu menu(this);
	QAction *pact;
	bool isAliasAlbum = (_ActAlbum()->baseAlbumId != NO_ID);
    int nSelSize = selectionModel()->selectedIndexes().size();

    if(nSelSize) // album(s) or image(s) or videos are selected
    {
        Album& album = *albumgen.Albums()[_albumId].BaseAlbum();

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
                    connect(pact, &QAction::triggered, this, &ThumbnailView::AskAndGetThumbnail);
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
            else if(!isAliasAlbum)
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
    if (!_ActAlbum()->baseAlbumId)    // can't add any items to an alias album
    {
        pact = new QAction(tr("&New Album or new Alias..."), this);  // create new folder, or folder alias inside actual folder
        connect(pact, &QAction::triggered, this, &ThumbnailView::NewVirtualFolder);
        menu.addAction(pact);

        pact = new QAction(tr("&Rename Album..."), this);  // rename existing folder
        connect(pact, &QAction::triggered, this, &ThumbnailView::RenameVirtualFolder);
        menu.addAction(pact);

        menu.addSeparator();
        pact = new QAction(tr("Add &Images/Videos from disk ..."), this);  // any number of images from a directory
        pact->setEnabled(true);
        connect(pact, &QAction::triggered, this, &ThumbnailView::AddImages);
        menu.addAction(pact);

        pact = new QAction(tr("Add &Folder from disk..."), this);  // one folder added to the folder tree inside this album
        connect(pact, &QAction::triggered, this, &ThumbnailView::AddFolder);
        menu.addAction(pact);

        if (nSelSize > 1)
        {
            menu.addSeparator();

            pact = new QAction(tr("&Synchronize texts"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::SynchronizeTexts);
            menu.addAction(pact);
        }


        if (nSelSize)
        {
            menu.addSeparator();
            pact = new QAction(tr("&Remove/Delete"), this);
            connect(pact, &QAction::triggered, this, &ThumbnailView::DeleteSelected);
            menu.addAction(pact);
        }
    }
    if (nSelSize == 0)
    {
        menu.addSeparator();   
        // 2 albums always added to AlbumMap: root and latest
        if (albumgen.Albums().size() > 2 && !_ActAlbum()->items.isEmpty())
        {
            pact = new QAction(tr("Thumbnail for Parent..."), this);  // any image
            connect(pact, &QAction::triggered, this, &ThumbnailView::SelectAsAlbumThumbnail);         // for this album, even when it is an alias
            menu.addAction(pact);

            menu.addSeparator();
        }

        // even when no image or video recursive listing is possible
        {
            pact = new QAction(tr("E&xport list to CSV..."), this);
            pact->setToolTip(tr("Export original and generated file names from this album to a CSV file"));
            connect(pact, &QAction::triggered, this, &ThumbnailView::ExportAsCSV);
            menu.addAction(pact);
        }
    }

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
 * REMARKS: - does not delete generated images, videos and albums 
 *             from disk!
 *          - Can't delete items from an alias album
 *          - Tries use the recycle bin (windows) or the trash (mac)
 *------------------------------------------------------------*/
void ThumbnailView::SlotDeleteSelectedList(ID_t albumId, IntList& list, bool iconsForThisAlbum)
{
    QString s;

    int res = DeleteOrRemoveConfirmationDialog(list, this);
    if (res == 2)
        return;

    bool fromDisk = res == 1;       //  Yes
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
    Album* pAlbum = albumgen.Albums()[_albumId].BaseAlbum();

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

    emit SignalSingleSelection(Item(_rowSelectedWithRightButton)->ID, _ActAlbumId());

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

    albumgen.AddToModifiedList(*pAlbum);
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
 * EXPECTS: not called for alias albums
 * GLOBALS:
 * RETURNS:
 * REMARKS: connected to popup menu
 *------------------------------------------------------------*/
void ThumbnailView::AddImages()
{   
    QString dir = pathMap.AbsPath(albumgen.lastUsedAlbumPathId)+_ActAlbum()->name;
    QStringList qslFileNames = QFileDialog::getOpenFileNames(this, tr("falconG - Add images/videos"), dir, "Images(*.bmp *.jpg *.png);;Videos(*.mp4);;All files(*.*)");
    if (qslFileNames.isEmpty())
        return;

    int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;

    emit SignalInProcessing(true);
    _AddImagesAndVideosFromList(qslFileNames, pos, true);
    // now add last used path
	int siz = qslFileNames.size();
    QString s;
	SeparateFileNamePath(qslFileNames[siz - 1], dir,s);

    Reload();
    albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);

    emit SignalInProcessing(false);
    emit SignalAlbumChanged();
    emit selectionChanged(QItemSelection(), QItemSelection());
    config.dsLastImageDir = dir;
}


/*=============================================================
 * TASK:    internal function that adds a folder recursively
 * PARAMS:  folderName - full path name of folder to add
 * GLOBALS:
 * RETURNS: if folder was added
 * REMARKS: 
 *------------------------------------------------------------*/
bool ThumbnailView::_AddFolder(QString folderName)
{
    Album* pParentAlbum = albumgen.Albums()[_albumId].BaseAlbum();

    bool added, atLeastOneFolderWasAdded = false;
    emit SignalInProcessing(true);
	emit SignalSaveTreeViewExpandedState(); // save expanded state of tree view before changing album structure
    emit SignalAlbumStructWillChange();
    IDVal_t idVal = pParentAlbum->ID.Val();
    ID_t id = albumgen.Albums().Add(idVal, folderName, added);     // set new dirIndex too
    if (added)
    {   
        atLeastOneFolderWasAdded = true;
        pParentAlbum = albumgen.Albums()[_albumId].BaseAlbum();     // album position may have changed when new album was added to map
        albumgen.AddToModifiedList(*pParentAlbum);
        Album &album = *albumgen.AlbumForID(id);
        album.parentId = _albumId.Val();
        albumgen.AddDirsRecursively(id);

        int pos = selectionModel()->hasSelection() ? currentIndex().row() : -1;
        // already added pParentAlbum->AddItem(id);

        ID_t idth = album.ThumbID();
        if (idth.Val())
            folderName.clear();
        else
            folderName = albumgen.Images()[idth].FullSourceName();
        (void)fileIcons.Insert(pos,folderName);

        albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);
    }

	emit SignalRestoreTreeViewExpandedState(); // restore expanded state of tree view after changing album structure
    emit SignalInProcessing(false);
    emit SignalAlbumStructChanged(true);

    return atLeastOneFolderWasAdded;
}

/*=============================================================
 * TASK:    internal function that creates a folder
 * PARAMS:  folderName - name of folder to add w.o. any path
 *          baseAlbumId - id of real album or 0 if this album is not an alias
 * GLOBALS:
 * RETURNS: if folder was added
 * REMARKS: folder names must be unique
 *------------------------------------------------------------*/
bool ThumbnailView::_NewVirtualFolder(QString folderName, IDVal_t baseAlbumID)
{
    auto errMsg = [&](QString text)
        {
            QMessageBox::warning(this, tr("falconG - Warning"), tr( "Adding new album failed!\n\n%1\n"
                                                                    "Please use a different name!\n\n"
                                                                    "Album names must be unique.").arg(text));
        };

    bool addedToMap = false,
         res = false;

    emit SignalInProcessing(true);
    emit SignalAlbumStructWillChange();

    Album& thisAlbum = albumgen.Albums()[_albumId]; // either this is the parent of the new album
    Album& parentAlbum = *thisAlbum.BaseAlbum();    // or its base album is

	IDVal_t idPVal = parentAlbum.ID.Val();
    ID_t id = albumgen.Albums().Add(idPVal, folderName, addedToMap, baseAlbumID);

    if (addedToMap)
    {
        albumgen.AddToModifiedList(parentAlbum);                 // either this album or its base album

        albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);
        res = true;
    }
    else
        errMsg(tr("Unknown error: Folder created but wasn't added to gallery") );
    emit SignalInProcessing(false);
    emit SignalAlbumStructChanged(false);   // changes already written to disk (no backup was made nor will be)

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
    Album* thisAlbum = albumgen.Albums()[_albumId].BaseAlbum();
    QString dir = thisAlbum->FullSourceName();
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
 *         This may be an alias to an exiting folder or a new one
 *         
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - actual album is parent of new album folder
 *          - no thumbnail flag is set
 *------------------------------------------------------------*/
void ThumbnailView::NewVirtualFolder()
{

    GetNewAlbumNameDialog aDlg(albumgen.Albums(), this);
    IDVal_t baseIdVal = NO_ID; // reset baseIdVal

    if (aDlg.exec() == QDialog::Accepted)
    {
        baseIdVal = aDlg.GetBaseAlbumId(); // may be NO_ID for new album
        QString folderName = aDlg.GetAlbumName(),
            baseAlbumName = aDlg.GetBaseAlbumName(baseIdVal);

        if (_NewVirtualFolder(folderName, baseIdVal)) // refreshes treeview's model too
            frmMain->GetTreeViewPointer()->update();
    }
}

/*=============================================================
 * TASK   : rename selected virtual folder
 * PARAMS : none
 * EXPECTS:
 * GLOBALS:
 * RETURNS: none
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::RenameVirtualFolder()
{
	QModelIndexList list = selectionModel()->selectedIndexes(); // list must onle have a single item
    Album& thisAlbum = albumgen.Albums()[_albumId];
    Album& album = *thisAlbum.BaseAlbum();
    ID_t id = album.items[list[0].row()];
    Album* pItem = albumgen.AlbumForID(id);
    QString text = QInputDialog::getText(
        this,                                                   // Parent widget
        tr("falconG - Rename Virtual Album"),                   // Dialog title
        tr("Old name:%1\n\nNew name:").arg(pItem->name),        // Prompt text
        QLineEdit::Normal,                  // Input mode (QLineEdit::Normal for single-line text)
        pItem->name                         // Default text
    );
	if (text.isEmpty())
		return;
	pItem->name = text;
	albumgen.AddToModifiedList(*pItem);
    AlbumTreeView* ptrv = frmMain->GetTreeViewPointer();
    ptrv->update(ptrv->currentIndex());
    Reload();
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
    Album& thisAlbum = albumgen.Albums()[_albumId];
    Album* album = thisAlbum.BaseAlbum();
    for (auto &i : list)
    {
        ID_t id = album->items[i.row()];// ID_t(i.internalPointer());
        pItem = albumgen.IdToItem(id);
        
        switch (pItem->Type())
        {
            case IABase::iatImage: 
            case IABase::iatVideo: 
                s += QString().setNum(id.Val()) + pItem->Extension() + "\n";
                break;
            default:
            case IABase::iatAlbum:
                s += config.sBaseName.ToString() + QString().setNum(id.Val()) +"\n";
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
void ThumbnailView::AskAndGetThumbnail()
{
    int pos = currentIndex().row();     // new thumbnail index for actual item
    Album* pParent =  albumgen.Albums()[_albumId].BaseAlbum(); // real album which has the items
    // get actual selected album inside parent
    Album *album = pParent ? &albumgen.Albums()[ pParent->items[pos]] : &albumgen.Albums()[_albumId];
    QString s = album->FullSourceName();

    QString filter = tr("Image files (*.bmp *.gif *.jpg *.png)");

    s = QFileDialog::getOpenFileName(this, tr("falconG - Open file for Thumbnail"), s, filter);
                                            
    if (!s.isEmpty())
    {
        bool added;             //            as thumbnail
        ID_t id = albumgen.Images().Add(s, added, IS_THUMBNAIL);   // will not add it when it is already in data base
        album->SetThumbnail(id);                                   // increases image's thumbnailCount
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

    Album& thisAlbum = albumgen.Albums()[_albumId];
    Album &album     = *thisAlbum.BaseAlbum(); // real album which has the items
    int cthix = -1;// old thumbnail index in 'items'. -1: not from current album
    for (int i = 0; i < album.items.size(); ++i)
        if (thisAlbum.thumbnailId == album.items[i])
            cthix = i;
    if (cthix == pos)       // no change
        return;

    ID_t th = album.items[pos];                                  

    thisAlbum.SetThumbnail(th.IsAlbum() ? albumgen.Albums()[th].thumbnailId : albumgen.ImageAt(th)->ID);
    albumgen.AddToModifiedList(_albumId, true);    // this one is modified, not the base album

    UpdateTreeView(false);
    if (album.parentId)
    {
        UpdateTreeView(true);
        albumgen.AddToModifiedList(ID_t(ALBUM_ID_FLAG, album.parentId), true);  // the parent of this album always changes
    }
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
    Album& thisAlbum = albumgen.Albums()[_albumId];
    Album& album = *thisAlbum.BaseAlbum();
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
 * EXPECTS: not called for alias albums
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
    // add as thumbnail to this album
    albumgen.AddItemToAlbum(_albumId, thname, IS_THUMBNAIL, !SIGNAL_ELAPSED_TIME, !ADD_TO_ALBUM_ITEMS);

    Album& album = *_ActAlbum();
    albumgen.AddToModifiedList(album);

    if (album.parentId)
    {
        Album* parent = albumgen.AlbumForIDVal(album.parentId);

        albumgen.AddToModifiedList(*parent);     // only for the parent of this album and not all parents!
    }
    emit SignalAlbumChanged();
}

void ThumbnailView::_ExportCSVFromAlbum(const Album& album, QTextStream& out, bool recursive)
{
    for (ID_t id : album.items)
    {
        IABase* pItem = albumgen.IdToItem(id);
        if (!id.IsAlbum())
        {
            QString sext = id.IsImage() ? ".jpg" : ".mp4";
            out << '"' << pItem->name << "\", " << pItem->ID.ValToString() << sext << "\n";
        }
    }

    if (recursive)
    {
        for (ID_t id : album.items)
        {
            if(id.IsAlbum())
            {
                Album* subAlbum = albumgen.AlbumForID(id);
                if (subAlbum)
                {
                    out << subAlbum->name << "," << config.sBaseName.ToString() << subAlbum->ID.Val() << "\n";
                    _ExportCSVFromAlbum(*subAlbum, out, recursive);
                }
            }
		}
    }
};

/*=============================================================
 * TASK   : export both original and calculated file names from 
 *          current album to a CSV file.
 * PARAMS : none
 * EXPECTS: there are image/video files in the current album
 * GLOBALS: _albumId, _ActAlbum()
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void ThumbnailView::ExportAsCSV()
{
	QFileDialog fd(this, tr("falconG - Export CSV"), pathMap.AbsPath(albumgen.Images().lastUsedPathId), "CSV files (*.csv)");

	QStringList fileNames;
    if(fd.exec())
    {
        fileNames = fd.selectedFiles();
        if (fileNames.isEmpty())
            return;
    }
    bool recursive = false;
    if(_ActAlbum()->SubAlbumCount(true) > 0)
    {
        if (QMessageBox::question(this, tr("falconG - CSV export"), tr("There are sub-albums in this album\nDo you want to process sub-albums too?")) == QMessageBox::Yes)
            recursive = true;
	}
    QString csvname = fileNames[0];
    int extp = csvname.lastIndexOf(".");
    if (extp < 0)
        csvname += ".csv";
    QFile file(csvname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("falconG - Warning"), tr("Cannot open file for writing: %1").arg(file.fileName()));
        return;
    }
    QTextStream out(&file);

	out << "falconG files in album '" << _ActAlbum()->name << "'\nOriginal,Generated\n";

	_ExportCSVFromAlbum(*_ActAlbum(), out, recursive);

	file.close();
}


void ThumbnailView::ItemDoubleClicked(const QModelIndex& mix)
{
//    _currentIndex = mix;
    int row = mix.row();
    ID_t id = _ActAlbum(true)->items[row];
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
    Album& thisAlbum = albumgen.Albums()[_albumId];
    Album& album = *thisAlbum.BaseAlbum();  // real album that has the items
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
        albumgen.AddToModifiedList(album);

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

void ThumbnailView::SetNeedToScroll(bool needToScroll) 
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

//********* getNewAlbumsDialog **********
GetNewAlbumNameDialog::GetNewAlbumNameDialog(const AlbumMap & albumMap, QWidget * parent)
    : QDialog(parent), _baseAlbumId(0)
{
    setWindowTitle(tr("falconG - new Album"));
    /*
    * Layout:                |---------------------------------|
    *                        || album tree   | new album data ||
    *                        ||          ...                  ||
    *                        |---------------------------------
	*                        ||     OK and Cancel buttons      ||
	*                        |---------------------------------|
    */
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	//mainLayout->setContentsMargins(1, 1, 1, 1);
    // fill up the left side with album tree
    QHBoxLayout* hLayout = new QHBoxLayout(this);

    QWidget* w = new QWidget(this);
    hLayout->addWidget(w);
	QVBoxLayout* wLayout = new QVBoxLayout(w);
    _treeView = new AlbumTreeView(w);          // selection of an album to be used as alias
    _treeView->header()->hide();
    _treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _treeView->setModel(frmMain->GetTreeViewPointer()->model()); // use the album tree model
    _treeView->setRootIsDecorated(false);
    _treeView->setExpanded(_treeView->model()->index(0,0), true);
    //_treeView->expandAll();
    _treeView->setMinimumWidth(300);
    // DEBUG
    //int n = _treeView->model()->rowCount(),
    //    n1= frmMain->GetTreeViewPointer()->model()->rowCount();        

    // _treeView->setEnabled(false); // until the checkbox is checked
    _treeView->setVisible(false);
	w->layout()->addWidget(_treeView);

    // the right side asks for the new album data
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Album's name:"), this);

    _lineEdit = new QLineEdit(this);
    _lineEdit->setPlaceholderText(tr("Name of the new album"));
    QRegExpValidator validator(QRegExp("[^\\/:*?\"<>|]*"), this);
    _lineEdit->setValidator(&validator);

    layout->addWidget(label);
    layout->addWidget(_lineEdit);

    _checkBox = new QCheckBox(QObject::tr("Use as alias for:"), this);

    connect(_checkBox, &QCheckBox::toggled, this, [this, w](bool checked) {
		// _treeView->setEnabled(checked); 
        _treeView->setVisible(checked);
        });
    layout->addWidget(_checkBox);
    _lblBaseName = new QLabel();
	_lblBaseName->setText(tr("No base album selected")); 
    layout->addWidget(_lblBaseName);
	layout->addItem(new QSpacerItem(40,80, QSizePolicy::Minimum, QSizePolicy::Expanding)); // fill up the space

	hLayout->addLayout(layout);

	mainLayout->addLayout(hLayout);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* okBtn = new QPushButton(tr("O&K"), this);
    okBtn->setEnabled(false); // Initially disabled, enabled on getting the name
    QPushButton* cancelBtn = new QPushButton(tr("&Cancel"), this);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(_lineEdit, &QLineEdit::textChanged, this, [okBtn](const QString& text)
        {
            okBtn->setEnabled(!text.trimmed().isEmpty()); // Enable if text is not empty
        });

    connect(_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GetNewAlbumNameDialog::_SlotTreeSelectionChanged);

    connect(okBtn, &QPushButton::clicked, this, &GetNewAlbumNameDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &GetNewAlbumNameDialog::reject);
}

void GetNewAlbumNameDialog::_SlotTreeSelectionChanged(const QItemSelection& current, const QItemSelection& previous)
{
   if(current.indexes().size() == 1) // only one item selected
    {
        QModelIndex index = current.indexes().first();
        _baseAlbumId = IDVal_t(index.internalPointer()); // store the selected album id
        Album* pa = albumgen.Albums().AlbumForIDVal(_baseAlbumId);
        _lblBaseName->setText(tr("%1(%2)").arg(pa->name).arg(_baseAlbumId));
    }
    else
    {
        _baseAlbumId = 0; // no valid selection
        _lblBaseName->setText(tr(""));
   }
}
