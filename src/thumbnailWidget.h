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
#include "deletableitems.h"

const int BAD_IMAGE_SIZE = 64;
const int WINDOW_ICON_SIZE = 48;
const int THUMBNAIL_SIZE = 150;

using ID_t = uint64_t;			// Cf. albums.h
using IdList = DeletableItemList<ID_t>;
using IntList = QVector<int>;

class ThumbnailWidget : public QListView {
Q_OBJECT

public:
    enum UserRoles {
        FileNameRole = Qt::UserRole + 1,		 // file name w.o. path, e.g. 12345.jpg
		FilePathRole,							 // path of file ending in '/'
        SortRole,
        LoadedRole
    };

    ThumbnailWidget(QWidget *parent, int thumbsize = THUMBNAIL_SIZE);

    void loadPrepare();
    void reLoad();
    void loadFileList();
    bool setCurrentIndexByName(QString &fileName);
    bool setCurrentIndexByRow(int row);
    void setCurrentRow(int row);
	void setTitle();
    void setNeedToScroll(bool needToScroll);
    void selectCurrentIndex();
    void addThumb(int index);		// names are in string lists
    void abort();
    void selectThumbByRow(int row);
    int getNextRow();
    int getPrevRow();
    int getLastRow();
    int getRandomRow();
    int getCurrentRow();

    QStringList getSelectedThumbsList();

    QString getSingleSelectionFilename();

	void SetImageList(IdList *pidl);
	QString thumbsDir;	// prepend to image file paths, must end with '/'
	QStringList thumbNames;			// list of name of image files in 'dir'
	QStringList originalPaths;		// for every element of thumbNames
	QString statusStr;		// get status messages from here
	QString title;			// use for window title

    QStringList *fileFilters;
    QStandardItemModel *ThumbnailWidgetModel;
    QDir::SortFlags thumbsSortFlags;
    int thumbSize;
    QString filterString;
    bool isBusy;
signals:
	void SignalTitleChanged(QString &s);		// a new 'title' (total image count) was added
	void SignalStatusChanged(QString &s);		// 'statusStr' changed
	void SignalInProcessing(bool on);			// current page or thumbnail adding started /finished
	void SingleSelection(QString path);			// name of selected thumbnail
	void SignalNewThumbnail();					// last selected 
protected:
    void startDrag(Qt::DropActions);

    void wheelEvent(QWheelEvent *event);

    void mousePressEvent(QMouseEvent *event);

	void contextMenuEvent(QContextMenuEvent *pevent);

private:
    void initThumbs();

    int getFirstVisibleThumb();

    int getLastVisibleThumb();

    void updateThumbsCount();

//    QFileInfo thumbFileInfo;
    QImage emptyImg;		// put all thumbnails into this
    int currentRow;
    QModelIndex currentIndex;
    bool isAbortThumbsLoading;
    bool isNeedToScroll;
    bool scrolledForward;
    int thumbsRangeFirst;
    int thumbsRangeLast;
// SA
	IdList *_pImages = nullptr;
public slots:

    void loadVisibleThumbs(int scrollBarValue = 0);

    void onSelectionChanged(const QItemSelection &selection);

    void invertSelection();

	void DeleteSelected();
	void UndoDelete();
	void AddImages();
	void CopyNamesToClipboard();
	void CopyOriginalNamesToClipboard();
	void SetAsAlbumThhumbnail();

private slots:

    void loadThumbsRange();
};


