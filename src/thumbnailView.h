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
#include "common.h"
#include "schemes.h"
#include "albums.h"

const int BAD_IMAGE_SIZE = 64;
const int WINDOW_ICON_SIZE = 48;
const int THUMBNAIL_SIZE = 600;	// maximum size
const int THUMBNAIL_BORDER_FACTOR = 30;	// border width = thumbnail size /this factor

class ImageViewer;

class FileIcons
{

public:
	enum Flag { fiFolder=1, fiThumb=2, fiDontResize=4};
	using Flags = QFlags<Flag>;

	typedef QVector<int> dataType;

	int posFolderIcon = -1;

	void Clear();
	int Size() const;	// only the displaed size, not necessarily the count of icons
	void SetMaximumSizes(int thumbsize = THUMBNAIL_SIZE, int borderwidth = 10);
	bool HasIconFor(int pos);
	void SetFolderThumbnailPosition(int pos, bool bIsFolderThumbnail = false);

	QIcon IconForPosition(int pos, Flags flags, QString imageName = QString());
	const dataType &IconOrder() const;
	void SetIconOrder(const QVector<int>& order);
	MarkedIcon* Insert(int pos, bool isFolder, QString imageName = QString());
	void Remove(int pos);    // remove items _iconOrder[pos];
private:
	QVector<MarkedIcon> _iconList;   // all icons for actual album,  
									 // (Not QList because in Qt5 QList  does not store items adjacently.
									 //  In Qt6 QList is the same as QVector, so it would be the same)
									 // order never changes when items added or moved around
									 // access elements through _iconOrder
	dataType _iconOrder;         // indirection through this
};

extern FileIcons fileIcons;

/*=============================================================
 * info stored for items in view
 *------------------------------------------------------------*/

class ThumbnailItem : public QStandardItem
{
public:
	enum Type { none = QStandardItem::UserType + 1, image, video, folder };
	int itemPos=-1;		// original position of this item in one of the image, 
						// video or album lists of the album with '_albumId'
						// set in constructor (parameter 'pos')
	static int thumbHeight;

public:
	int type() const { return _itemType; }
	void SetType(Type typ) 
	{ 
		_itemType = typ; 
	}

	ThumbnailItem(int pos = 0, ID_t albumID = ID_t::Invalid(), Type typ = none, QIcon icon = QIcon());
	ThumbnailItem(const ThumbnailItem &other)			:ThumbnailItem(other.itemPos, other._albumId, other._itemType, other._icon) {}
	ThumbnailItem(const ThumbnailItem &&other) noexcept	:ThumbnailItem(other.itemPos, other._albumId, other._itemType, other._icon) {}
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
	void SetOwnerId(ID_t id) 
	{ 
		if (id.Val()==NO_ID) 
			id = TOPMOST_ALBUM_ID;
		_albumId = id; 
	}


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
	ID_t _albumId = { ALBUM_ID_FLAG, 0xFFFFFFFFFFFFFFFFu };	// for parent album, set before anything else
	Type _itemType;
	QIcon _icon;

	Album	*_ActAlbum() const { return albumgen.AlbumForID(_albumId); }
	Album   *_ParentAlbum() const 
	{ 
		Album* pa = _ActAlbum();
		return pa->parentId.Val() ? albumgen.AlbumForID(pa->parentId) : nullptr;
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
 * My thumbnail model
 *------------------------------------------------------------*/
class ThumbnailViewModel : public QStandardItemModel
{
	ThumbnailItem _dummyItem;	// empty item to show at insert position
	int _dummyPosition = -1;	// used during drag to signal possible drop position
								// -1: not used else position of _dummyItem
public:
	ThumbnailViewModel(QWidget *pw) : QStandardItemModel(pw) 
	{
		_dummyItem.setData(QString(), Qt::UserRole + 1);		// FileNameRole
	}
	virtual ~ThumbnailViewModel() {}
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
		ThumbnailItem *result = dynamic_cast<ThumbnailItem*>(QStandardItemModel::item(row, 0));
		return result;
	}

	QStandardItem *itemFromIndex(const QModelIndex &index) const
	{
		return item(index.row(), 0);
	}
	inline void BeginResetModel() { beginResetModel(); }
	inline void EndResetModel() { endResetModel(); }
	inline int DummyPosition() const { return _dummyPosition;  }
	void Clear();
	
};
#else
using ThumbnailViewModel = QStandardItemModel;
#endif
 /*=============================================================
 * my list view that shows icons
 * REMARKS:	In a QListView 'index' gives the 0 based index of the icon
 *------------------------------------------------------------*/

class ThumbnailView : public QListView 
{
	Q_OBJECT

public:
    ThumbnailView(QWidget *parent/*, int thumbsize = THUMBNAIL_SIZE*/);

	void Setup(ID_t aid);
	void Clear();
	void RemoveViewer(ImageViewer* pv);

    void Load();
	void Reload();
    void LoadFileList();
    bool SetCurrentIndexByName(QString &fileName);
    bool setCurrentIndexByItem(int itemIndex);	   
    void SetCurrentItem(int itemIndex);
	void SetTitle();
    void setNeedToScroll(bool needToScroll);
    void selectCurrentIndex();
    void AddThumb(int itemIndex, ThumbnailItem::Type type);		// names are in string lists
	void SetInsertPos(int here);		// into the model
    void abort();		 // abort loading of thumbs
	bool IsFinished()const 
	{ 
		return !_isProcessing; 
	}
    void selectThumbByItem(int itemIndex);
    int GetNextItem();
    int GetPrevItem();
    int GetLastItem();
    int GetRandomItem();
    int GetCurrentItem();
	ID_t AlbumID() const { return _albumId; }

    QStringList GetSelectedThumbsList();

    QString GetSingleSelectionFilename();

	// DEBUG
//	QLabel* pDragDropLabel = nullptr;

	QString statusStr;		// get status messages from here
	QString title;			// use for window title

private:
// SA
	ID_t   _albumId = TOPMOST_ALBUM_ID;		// show thumbs from this album
	IdList *_pIds = nullptr;				// images in this album

    QImage _insertPosImage;	// shows insert position

	bool _dontRemoveImageViewerFromList = false; // used when all viewers are removed by user
	QList<ImageViewer*> _lstActiveViewers;		// visible image viewers
// /SA

    QStringList *_fileFilters;
    ThumbnailViewModel *_thumbnailViewModel;
	QModelIndex _dragStartIndex;				// started drag from here
	int _insertPos = -1;						// put a spacer at this position
    QDir::SortFlags _thumbsSortFlags;
    QString _filterString;
    bool _isBusy=false;
	bool _dragFromHereInProgress = false;		// drag started from here

//    QFileInfo thumbFileInfo;
    int _currentItem=-1;
    QModelIndex _currentIndex;
    bool _doAbortThumbsLoading = false;
	bool _isProcessing = false;
    bool _isNeedToScroll = false;
    bool _scrolledForward = false;
    int _thumbsRangeFirst = -1;
    int _thumbsRangeLast = -1;
	int _rowSelectedWithRightButton = -1;
	int _rowSelectedWithLeftButton = -1;

	QStringList _slSearchPaths;		// paths to search missing images against

private:
	Album	*_ActAlbum() const 
	{ 
		return _albumId.Val() ? &albumgen.Albums()[_albumId] : nullptr;
	}
    void _InitThumbs();
    int _GetFirstVisibleThumb();
    int _GetLastVisibleThumb();
    void _UpdateThumbsCount();
	bool _IsAllowedTypeToDrop(const QDropEvent *event);
	void _AddImagesFromList(QStringList qslFileNames, int row);
	bool _AddFolder(QString folderName);	// returns if folder was added
	bool _NewVirtualFolder(QString folderName);	// returns if folder was created, false if did  already existed
	bool _AddFoldersFromList(QStringList qslFolders, int row);
	inline ThumbnailItem::Type _TypeFor(ID_t id) const
	{
		return (id.IsImage() ? ThumbnailItem::image : (id.IsVideo() ? ThumbnailItem::video : ThumbnailItem::folder));
	}

	void _RemoveAllViewers();

signals:
	void SignalTitleChanged(QString &s);		// a new 'title' (total image count) was added
	void SignalStatusChanged(QString &s);		// 'statusStr' changed
	void SignalInProcessing(bool on);			// current page or thumbnail adding started /finished
	void SignalSingleSelection(ID_t id, ID_t insideThis);		// may be album or image or video
	void SignalMultipleSelection(IdList, ID_t insideThis);		// all selected items
	void SignalFolderChanged(int row);			// move to next level in tree list inside actual folder
	void SignalAlbumStructWillChange();			// emitted befor new folder(s) added
	void SignalAlbumStructChanged(bool success);			// after the new folder is added
	void SignalMayLoadNewItems();					// when there was a new album selected in tree view
	void SignalAlbumChanged();					// add the new album to tree view as well
	void SignalImageViewerAdded(bool enableclosebutton);
	void SignalBackToParentAlbum();
protected:
    void startDrag(Qt::DropActions);			// called by QListView() 
// exper: comments
	void dragEnterEvent(QDragEnterEvent *event);	// when a drag enters this widget
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

	void _DropFromExternalSource(const ThumbMimeData *mimeData, int row);
	void dropEvent(QDropEvent *event);			// when item is dropped

	void keyReleaseEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	void contextMenuEvent(QContextMenuEvent *pevent);

public slots:
    void loadVisibleThumbs(int scrollBarValue = 0);
    void onSelectionChanged(const QItemSelection &selection);
    void invertSelection();
	void SlotDeleteSelectedList(ID_t albumId, IntList &list, bool iconsForThisAlbum);
	void DeleteSelected();
	void SynchronizeTexts();
	void UndoDelete();
	void AddImages();
	void AddFolder();
	void NewVirtualFolder();
	void RenameVirtualFolder();
	void CopyNamesToClipboard();
	void CopyOriginalNamesToClipboard();
	void OpenAlbumThumbnail();			// any image for an album thumbnal
	void SetAsAlbumThumbnail();			// from existing image/album image
	void ToggleDontResizeFlag();
	void SelectAsAlbumThumbnail();
	void ItemDoubleClicked(const QModelIndex &);
	void SlotThumbnailSizeChanged(int thumbSize);
	void FindMissingImageOrVideo();		// maybe it was moved from its position
	void SlotToCloseAllViewers();
	void SlotGetSelectionCount(ID_t &id, int &count );
//	void SlotToClearIconList();

private slots:
    void loadThumbsRange();
};


