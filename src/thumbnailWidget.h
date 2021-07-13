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
#include "deletableitems.h"

const int BAD_IMAGE_SIZE = 64;
const int WINDOW_ICON_SIZE = 48;
const int THUMBNAIL_SIZE = 150;

using ID_t = uint64_t;			// Cf. albums.h
using IdList = UndeletableItemList<ID_t>;
using IntList = QVector<int>;

/*=============================================================
 * info stored for items in view
 *------------------------------------------------------------*/
struct ThumbnailRecord
{
	QString imageName;	// set using user type FileNameRole
	int row;			// original position of this item
};

/*=============================================================
 * my Mime Type for Drag & Drop
 *------------------------------------------------------------*/
class ThumbMimeData : public QMimeData
{
	Q_OBJECT
public:
	QList<ThumbnailRecord> thumbList;  // all selected items
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
	QStandardItem _dummyItem;
public:
	ThumbnailWidgetModel(QWidget *pw) : QStandardItemModel(pw) 
	{
		//QPixmap pix;
		//QPixmap(128, 112);
		//pix.fill(Qt::transparent);
		_dummyItem.setData(NULL, Qt::UserRole+1);		// FileNameRole
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
	int rowCount() { return QStandardItemModel::rowCount() + _dummyPosition >= 0 ? 1 : 0; }

	QStandardItem *	item(int row, int column = 0) const 
	{
		if (_dummyPosition >= 0)
		{
			if (row > _dummyPosition)
				return QStandardItemModel::item(row - 1,0);
			if (row == _dummyPosition)
				return const_cast<QStandardItem*>(&_dummyItem);
		}
		return QStandardItemModel::item(row,0);
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
    ThumbnailWidget(QWidget *parent, int thumbsize = THUMBNAIL_SIZE);

    void loadPrepare();
    void reLoad();
    void loadFileList();
    bool setCurrentIndexByName(QString &fileName);
    bool setCurrentIndexByItem(int itemIndex);	   
    void setCurrentItem(int itemIndex);
	void setTitle();
    void setNeedToScroll(bool needToScroll);
    void selectCurrentIndex();
    void addThumb(int itemIndex);		// names are in string lists
	void InsertSpaceAt(int here);		// into the model
	void RemoveSpace();					// from where it was set
    void abort();
    void selectThumbByItem(int itemIndex);
    int getNextItem();
    int getPrevItem();
    int getLastItem();
    int getRandomItem();
    int getCurrentItem();

    QStringList getSelectedThumbsList();

    QString getSingleSelectionFilename();

	void SetImageList(IdList *pidl);

	// DEBUG
	QLabel* pDragDropLabel = nullptr;

	QString thumbsDir;	// prepend to image file paths, must end with '/'
	QStringList thumbNames;			// list of name of image files in 'dir'
	QStringList originalPaths;		// for every element of thumbNames
	QString statusStr;		// get status messages from here
	QString title;			// use for window title

private:
    QStringList *_fileFilters;
    ThumbnailWidgetModel *_thumbnailWidgetModel;
	QModelIndex _dragStartIndex;				// started drag from here
	int _spacerPos = -1;						// put a spacer at this position
    QDir::SortFlags _thumbsSortFlags;
    int _thumbSize;
    QString _filterString;
    bool _isBusy;
	bool _dragFromHereInProgress = false;		// drag started from here

//    QFileInfo thumbFileInfo;
    QImage _emptyImg;		// put all thumbnails into this
    int _currentItem;
    QModelIndex _currentIndex;
    bool _isAbortThumbsLoading;
    bool _isNeedToScroll;
    bool _scrolledForward;
    int _thumbsRangeFirst;
    int _thumbsRangeLast;
// SA
	IdList *_pImages = nullptr;

private:
    void _initThumbs();
    int _getFirstVisibleThumb();
    int _getLastVisibleThumb();
    void _updateThumbsCount();
	bool _IsAllowedToDrop(const QDropEvent *event);

signals:
	void SignalTitleChanged(QString &s);		// a new 'title' (total image count) was added
	void SignalStatusChanged(QString &s);		// 'statusStr' changed
	void SignalInProcessing(bool on);			// current page or thumbnail adding started /finished
	void SingleSelection(QString path);			// name of selected thumbnail
	void SignalNewThumbnail();					// last selected 
protected:
    void startDrag(Qt::DropActions);			// called by QListView() 
// exper: comments
	void dragEnterEvent(QDragEnterEvent *event);	// when a drag enters this widget
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);	
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
	void UndoDelete();
	void AddImages();
	void AddFolder();
	void CopyNamesToClipboard();
	void CopyOriginalNamesToClipboard();
	void SetAsAlbumThhumbnail();

private slots:
    void loadThumbsRange();
};


