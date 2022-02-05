/*
 * from Phototonic Image Viewer
 *
 *  Copyright (C) 2013 Ofer Kashayov <oferkv@live.com>
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

#pragma once

#include <QtWidgets>
#include "enums.h"
#include "schemes.h"
#include "albums.h"

const int BAD_IMAGE_SIZE = 64;
const int WINDOW_ICON_SIZE = 48;
const int THUMBNAIL_SIZE = 600;	// maximum size
const int THUMBNAIL_BORDER_FACTOR = 30;	// border width = thumbnail size /this factor

using IntList = QVector<int>;

/*=============================================================
 * info stored for items in view
 *------------------------------------------------------------*/

class ThumbnailItem : public QStandardItem
{
public:
	enum Type { none = QStandardItem::UserType + 1, image, video, folder };
	int itemPos=-1;		// original position of this item in one of the image, 
						// video or album lists of the album with '_albumId'
	static int thumbHeight;

public:
	int type() const { return _itemType; }
	ID_t id() const 
	{ 
		return _albumId == 0xFFFFFFFFFFFFFFFFu || itemPos < 0 ? 0 : _ActAlbum()->items[itemPos];
	}
	void SetType(Type typ) 
	{ 
		_itemType = typ; 
	}

	ThumbnailItem(int pos = 0, ID_t albumID = 0, Type typ = none, QIcon icon = QIcon());
	ThumbnailItem(const ThumbnailItem &other) :ThumbnailItem(other.itemPos, other._albumId, other._itemType, other._icon) {}
	ThumbnailItem(const ThumbnailItem &&other):ThumbnailItem(other.itemPos, other._albumId, other._itemType, other._icon) {}
	ThumbnailItem &operator=(const ThumbnailItem& other) 
	{ 
		QStandardItem::operator=(other); 
		itemPos = other.itemPos; 
		_albumId = other._albumId; 
		_itemType = other._itemType; 
		return *this;
	}
	// set this only once for each run (static)
	static void SetThumbHeight(int th) {thumbHeight = th;  }
		// use this after constructing 
	void SetOwnerId(ID_t id) { _albumId = id;}


	QString text() const;		// text displayed on item, or full image file path to show thumbnail for
	QVariant data(int role = Qt::UserRole + 1) const;
	QString ToolTip() const;
	QString FilePath() const;	// for destination file
	QString FileName() const;	// - " - 
	QString FullSourcePath() const;
	QString FullLinkName() const;
	QString DisplayName() const;
	QIcon   IconForFile() const;		// uses _albumId and itemPos

private:
	ID_t _albumId=0xFFFFFFFFFFFFFFFFu;	// for parent album, set before anything else
	Type _itemType;
	QIcon _icon;

	Album	*_ActAlbum() const { return albumgen.AlbumForID(_albumId); }
	Album   *_ParentAlbum() const 
	{ 
		Album* pa = _ActAlbum();
		return pa->parent ? albumgen.AlbumForID(pa->parent) : nullptr;
	}
	QString _ImageToolTip() const;
	QString _VideoToolTip() const;
	QString _FolderToolTip() const;

	QString _ImageFilePath() const;
	QString _VideoFilePath() const;
	QString _FolderFilePath() const;

	QString _ImageFileName() const;	// returns: link name. for original name use FullSourceName()
	QString _VideoFileName() const;	// returns: link name
	QString _FolderFileName() const;// returns: link name

	QString _ImageFullSourceName() const;
	QString _VideoFullSourceName() const;
	QString _FolderFullSourceName() const;
};
 
/*=============================================================
 * my Mime Type for Drag & Drop
 *------------------------------------------------------------*/
class ThumbMimeData : public QMimeData
{
	Q_OBJECT
public:
		// IDs must be are added to this list with the << operator
	IntList thumbList;  // indices in actual album's 'items' for all selected items
	ThumbMimeData() {
		QMimeData::setData("application/x-thumb", "thumb"); // my type
	}
};

#if 1
/*=============================================================
 * my thumb model
 *------------------------------------------------------------*/
class ThumbnailWidgetModel : public QStandardItemModel
{
	int _dummyPosition = -1;	// used during drag to signal possible drop position
								// -1: not used
	ThumbnailItem _dummyItem;
public:
	ThumbnailWidgetModel(QWidget *pw) : QStandardItemModel(pw) 
	{
		_dummyItem.setData(QString(), Qt::UserRole + 1);		// FileNameRole
	}
	virtual ~ThumbnailWidgetModel() {}
	int SetDummyPos(int newPos) 
	{ 
		int old = _dummyPosition; 
		beginResetModel();
		_dummyPosition = newPos; 
		endResetModel();
		return old; 
	}
	int rowCount() 
	{ 
		return QStandardItemModel::rowCount() + (_dummyPosition >= 0 ? 1 : 0); 
	}

	ThumbnailItem*	item(int row, int column = 0) const
	{
		if (_dummyPosition >= 0)
		{
			if (row > _dummyPosition)
				return item(row - 1,0);
			if (row == _dummyPosition)
				return const_cast<ThumbnailItem*>(&_dummyItem);
		}
		// DEBUG
		ThumbnailItem *result = dynamic_cast<ThumbnailItem*>(QStandardItemModel::item(row, 0));
		return result;
		// return dynamic_cast<ThumbnailItem*>(QStandardItemModel::item(row,0) );
	}

	QStandardItem *itemFromIndex(const QModelIndex &index) const
	{
		return item(index.row(), 0);
	}
	int DummyPosition() const { return _dummyPosition;  }
	
};
#else
using ThumbnailWidgetModel = QStandardItemModel;
#endif
 /*=============================================================
 * my list view that shows icons
 * REMARKS:	In a QListView 'index' gives the 0 based index of the icon
 *------------------------------------------------------------*/

class ThumbnailWidget : public QListView 
{
Q_OBJECT

public:
    ThumbnailWidget(QWidget *parent/*, int thumbsize = THUMBNAIL_SIZE*/);

	void Setup(ID_t aid);
	void Clear() { _pIds = nullptr; _albumId = 0; }

    void Load();
	void Reload();
    void loadFileList();
    bool setCurrentIndexByName(QString &fileName);
    bool setCurrentIndexByItem(int itemIndex);	   
    void setCurrentItem(int itemIndex);
	void setTitle();
    void setNeedToScroll(bool needToScroll);
    void selectCurrentIndex();
    void addThumb(int itemIndex, ThumbnailItem::Type type);		// names are in string lists
	void SetInsertPos(int here);		// into the model
    void abort();
    void selectThumbByItem(int itemIndex);
    int getNextItem();
    int getPrevItem();
    int getLastItem();
    int getRandomItem();
    int getCurrentItem();

    QStringList getSelectedThumbsList();

    QString getSingleSelectionFilename();

	// DEBUG
//	QLabel* pDragDropLabel = nullptr;

	QString statusStr;		// get status messages from here
	QString title;			// use for window title

private:
// SA
	ID_t   _albumId = 0;	// show thumbs from this album (0: root)
	IdList *_pIds = nullptr;	    // images in this album

    QImage _insertPosImage;	// shows insert position
// /SA

    QStringList *_fileFilters;
    ThumbnailWidgetModel *_thumbnailWidgetModel;
	QModelIndex _dragStartIndex;				// started drag from here
	int _insertPos = -1;						// put a spacer at this position
    QDir::SortFlags _thumbsSortFlags;
    QString _filterString;
    bool _isBusy;
	bool _dragFromHereInProgress = false;		// drag started from here

//    QFileInfo thumbFileInfo;
    int _currentItem;
    QModelIndex _currentIndex;
    bool _isAbortThumbsLoading;
    bool _isNeedToScroll;
    bool _scrolledForward;
    int _thumbsRangeFirst;
    int _thumbsRangeLast;

private:
	Album	*_ActAlbum() const { return &albumgen.Albums()[_albumId]; }
    void _InitThumbs();
    int _GetFirstVisibleThumb();
    int _GetLastVisibleThumb();
    void _UpdateThumbsCount();
	bool _IsAllowedTypeToDrop(const QDropEvent *event);
	void _AddImagesFromList(QStringList qslFileNames, int row);
	void _AddFoldersFromList(QStringList qslFolders, int row);
	inline ThumbnailItem::Type _TypeFor(ID_t id) const
	{
		return (id & IMAGE_ID_FLAG ? ThumbnailItem::image : (id & VIDEO_ID_FLAG ? ThumbnailItem::video : ThumbnailItem::folder));
	}

signals:
	void SignalTitleChanged(QString &s);		// a new 'title' (total image count) was added
	void SignalStatusChanged(QString &s);		// 'statusStr' changed
	void SignalInProcessing(bool on);			// current page or thumbnail adding started /finished
	void SignalSingleSelection(ID_t id);		// may be album or image or video
	void SignalMultipleSelection(IdList);		// all selected items
	void SignalFolderChanged(int row);			// move to next level in tree list inside actual folder
	void SignalFolderAdded();					// add the new album to tree view as well
protected:
    void startDrag(Qt::DropActions);			// called by QListView() 
// exper: comments
	void dragEnterEvent(QDragEnterEvent *event);	// when a drag enters this widget
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

	void _DropFromExternalSource(const ThumbMimeData *mimeData, int row);
	void dropEvent(QDropEvent *event);			// when item is dropped

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	void contextMenuEvent(QContextMenuEvent *pevent);

public slots:
    void loadVisibleThumbs(int scrollBarValue = 0);
    void onSelectionChanged(const QItemSelection &selection);
    void invertSelection();
	void DeleteSelected();
	void SynchronizeTexts();
	void UndoDelete();
	void AddImages();
	void AddFolder();
	void CopyNamesToClipboard();
	void CopyOriginalNamesToClipboard();
	void SetAsAlbumThumbnail();			// from existing image/album image
	void SelectAsAlbumThumbnail();
	void ItemDoubleClicked(const QModelIndex &);
	void ThumbnailSizeChanged(int thumbSize);

private slots:
    void loadThumbsRange();
};


