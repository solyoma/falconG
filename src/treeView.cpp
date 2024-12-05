#include <QString>
#include <QStringList>
#include <QDir>
#include <QList>
#include <QPixmap>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QListView>
#include <QTextStream>
#include <QMenu>
#include <QTreeView>

#include "support.h"
#include "config.h"
#include "albums.h"
#include "treeView.h"
#include "thumbnailView.h"


// class 	 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// ************************************ AlbumTreeModel ***************************//
//			 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/*============================================================================
* TASK:		return row count for parent (count of albums in it)
* EXPECTS:	parent - model index of parent album or invalid model index for root
* GLOBALS:	albumgen.Albums()
* REMARKS: model indices are relativ to parent
*--------------------------------------------------------------------------*/
int AlbumTreeModel::rowCount(const QModelIndex & parent) const
{
	if (_busy) 
		return 0;
	AlbumMap &map = albumgen.Albums();
	if (!parent.isValid())		   // root element: it has 2 items, only one matters
		return	1; 
	
	ID_t id = { ALBUM_ID_FLAG, (uint64_t)(parent.internalPointer()) };
	// DEBUG
	if (!map.contains(id))
		return 0;
	return map[id].SubAlbumCount();
}

/*============================================================================
* TASK:	   Create a model index for parent's row-th child
* EXPECTS:	row - parent relative index of albums (< parent's AlbumCount())
*			column: 0
*			parent: index of parent whose row-th child's index is required
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QModelIndex AlbumTreeModel::index(int row, int column, const QModelIndex & parent) const
{
	if (column || row < 0)
		return QModelIndex();

	AlbumMap &map = albumgen.Albums();

	if (!parent.isValid())				// only the root item has invalid parent
		return row ? QModelIndex() : createIndex(row, column, quintptr( (map.size() ? TOPMOST_ALBUM_ID.Val() : 0)));
	ID_t idParent = { ALBUM_ID_FLAG, (uint64_t)parent.internalPointer() };

	ID_t id = map[idParent].IdOfItemOfType(ALBUM_ID_FLAG,row);
	return createIndex(row, column, quintptr(id.Val()));
}

/*=============================================================
 * TASK:	get full name of folder in tree
 * PARAMS:	mix - model index for item with ID in the internal
 *					pointer
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
const QString AlbumTreeModel::TextForIndex(const QModelIndex& mix) const
{
	ID_t id = { ALBUM_ID_FLAG, (uint64_t)mix.internalPointer() };
	Album& ab = albumgen.Albums()[id];
	QString sChangedFlag = ab.changed ? "*" : "";
	QString s = ab.name;
	if (ab.changed)
		s += "*";
	if (id == TOPMOST_ALBUM_ID)
		return "/ ( " + s + " )";
	return s + " ( " + ab.BareName() + " )";  // no language or extension
}

/*============================================================================
* TASK:		this tree model has only one column
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
int AlbumTreeModel::columnCount(const QModelIndex & parent) const
{
	return 1;
}

/*============================================================================
* TASK:		returns album name for given index, using the internal pointer 
*			of the index as an album ID
* EXPECTS:
* GLOBALS:
* REMARKS: only returns valid data for DisplayRole
*--------------------------------------------------------------------------*/
QVariant AlbumTreeModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())				// root id for tree element
	{
		if (!_busy && role == Qt::DisplayRole)
			return TextForIndex(index);
	}
	return QVariant();
}

/*============================================================================
* TASK:		determine the index of the parent of the album with the 'ind'-t
*			index
* EXPECTS:	ind: index of element (pointer is the ID cast to pointer)
* GLOBALS:
* REMARKS:	- topmost album does not have a parent (invalid index)
*			- there is only one topmost album
*			- the row of the topmost album is 0
*			- The row of the parent is the index of the parent in the
*				album list of *its* parent. 
*				Example:  tree						row  row of parent
*					album0							 0		-
*						album1                       0		0
*						album2						 1		0
*							album3					 0		1
*							album4					 1		1
*					album5							 1		0
*--------------------------------------------------------------------------*/
QModelIndex AlbumTreeModel::parent(const QModelIndex & ind) const
{
	if (!ind.isValid() || (uint64_t)ind.internalPointer() == TOPMOST_ALBUM_ID.Val())
		return QModelIndex();

	AlbumMap &map = albumgen.Albums();
	ID_t parentId = { ALBUM_ID_FLAG, (uint64_t)ind.internalPointer() };
	if (!map.contains(parentId))
		return QModelIndex();
	Album &ab = map[parentId];
	ID_t aParentID = ab.parentId;				   // get ID of parent
	if (aParentID.IsInvalid())					   // no parent: topmost element
		return QModelIndex(); // invalid index(0, ind.column());		   // it is the first and only such

	Album &abp = map[aParentID];				  // parent album
	ID_t bParentID = abp.parentId;				// parent's parent
	if (bParentID == ID_t( ALBUM_ID_FLAG, 0) )	  // parent is the topmost element
		return createIndex(0, ind.column(), quintptr(TOPMOST_ALBUM_ID.Val()));

	IdList albums = map[bParentID].items;			// parent's parent
	int size = albums.size();					// go through its list
	for (int i = 0; i < size; ++i)
	{
		if (albums[i] == aParentID)				// aParent contains ALBUM_ID_FLAG
			return createIndex(i, ind.column(), quintptr(aParentID.Val()));
	}
	return QModelIndex();
}

/*============================================================================
  * TASK:	return property flags for index
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
Qt::ItemFlags AlbumTreeModel::flags(const QModelIndex & index) const
{
	if (!index.isValid())
		return Qt::ItemFlags();

    Qt::ItemFlags iflags = QAbstractItemModel::flags(index)	// contains the commented out flags below
		//| Qt::ItemIsSelectable
		| Qt::ItemIsEditable
		//| Qt::ItemIsEnabled
		| Qt::ItemIsDropEnabled
		// | Qt::ItemIsUserCheckable
		;
	if (index.internalPointer() == 0)		// root item not moveable
		return iflags;
	else
		return iflags | Qt::ItemIsDragEnabled;
}

/*============================================================================
  * TASK:	  add a data to the tree 
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	return false;
}

/*============================================================================
  * TASK:	 change header data
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: no heder data shown
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role)
{
	return false;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS: not used
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::insertColumns(int position, int columns, const QModelIndex & parent)
{
	return true;
}

/*============================================================================
  * TASK:
  * EXPECTS:
  * RETURNS:
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::removeColumns(int position, int columns, const QModelIndex & parent)
{
	return true;
}

/*============================================================================
  * TASK:	insert rows into a parent
  * EXPECTS: row: insert before this row of the parent (0: before, rowCount(): after)
  *			 count: insert this many rows
  *			 parent: item these new rows will be children of
  * RETURNS: if inserted (default: false)
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::insertRows(int row, int count, const QModelIndex & parent)
{
	return false;
}

/*============================================================================
  * TASK:	remove count rows from a parent
  * EXPECTS: row: remove starting with this row of the parent
  *			 count: remove this many rows
  *			 parent: item these new rows are children of
  * RETURNS: if removeed (default: false)
  * GLOBALS:
  * REMARKS:
 *--------------------------------------------------------------------------*/
bool AlbumTreeModel::removeRows(int position, int rows, const QModelIndex & parent)
{
	return false;
}


// ************ AlbumTreeView *****************

AlbumTreeView::AlbumTreeView(QWidget* parent) : QTreeView(parent)
{
	setRootIsDecorated(false);
}
/*=============================================================
 * TASK:	tooltips for folder names
 * PARAMS:	event: any types of event
 * GLOBALS:
 * RETURNS: true - handled the event, false - did not
 * REMARKS: only used for tool typ events, all others have
 *			separate event handlers
 *------------------------------------------------------------*/
bool AlbumTreeView::event(QEvent* event)
{
	if (event->type() == QEvent::ToolTip)	// no separate event to handle this
	{
		QHelpEvent* phe = reinterpret_cast<QHelpEvent*>(event);
		QModelIndex ix = indexAt(phe->pos());
		if (ix.isValid())
		{
			AlbumTreeModel* pm = reinterpret_cast<AlbumTreeModel*>(model());
			QToolTip::showText(phe->globalPos(), pm->TextForIndex(ix));
		}
		else
			QToolTip::hideText();
		return true;
	}

	QWidget::event(event);
	return false;
}

void AlbumTreeView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton)
		return;
	return QTreeView::mousePressEvent(event);
}

void AlbumTreeView::mouseReleaseEvent(QMouseEvent* event)
{
	return QTreeView::mouseReleaseEvent(event);
}

//void AlbumTreeView::mouseMoveEvent(QMouseEvent* event)
//{
//	QModelIndex ix = indexAt(reinterpret_cast<QHelpEvent *>(event)->pos());
//	AlbumTreeModel* pm = reinterpret_cast<AlbumTreeModel *>(model());
//	setToolTip(pm->TextForIndex(ix));
//	QHelpEvent he(QEvent::ToolTip, event->pos(), event->globalPos());
//
//}

/*=============================================================
 * TASK:  slot to delete selected directory recursively
 * PARAMS: none
 * GLOBALS: albumgen
 * RETURNS: none
 * REMARKS: Because there are only albums in the tree view we 
 *			must determine the correct index as the parent may
 *			have images and videos beside the albums
 *			- at the moment only a single album can be deleted
 *------------------------------------------------------------*/
void AlbumTreeView::SlotDeleteSelectedAlbum()
{
	QModelIndexList list = selectionModel()->selectedIndexes();
	ID_t albumId = { ALBUM_ID_FLAG, (uint64_t)list[0].internalPointer() };
	ID_t parentID = albumgen.Albums()[albumId].parentId;
	Album album = albumgen.Albums()[parentID];		
	IntList ilx;
	IdList& idList = album.items;
	for (int i = 0; i < idList.size(); ++i)
		if (idList[i] == albumId)
			ilx.push_back(i);
	emit SignalDeleteSelectedList(parentID, ilx, false);
}

/*=============================================================
 * TASK:	move selected images in thumbnailView to this folder
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void AlbumTreeView::SlotMoveImages()
{
	qDebug("TODO:Dropped images here from thumbnail viewer");
}

/*=============================================================
 * TASK:	copy selected images in thumbnailView to this folder
 * PARAMS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void AlbumTreeView::SlotCopyImages()
{
	qDebug("TODO:Copied images here from thumbnail viewer");
}

void AlbumTreeView::SlotActAlbumChanged(int row)
{
	QModelIndex current = currentIndex();
	if (current.isValid())
	{
		expand(current);
#ifdef DEBUG
		int rowCount = model()->rowCount();		 // rowCount var. only for debug
#endif
		QModelIndex newix = model()->index(row, 0, current);
		setCurrentIndex(newix);
	}
}

void AlbumTreeView::contextMenuEvent(QContextMenuEvent* pevent)
{
	QMenu menu(this);
	QAction* pact;
	int nSelSize = selectionModel()->selectedIndexes().size();
	ID_t selId = { ALBUM_ID_FLAG, nSelSize ? (uint64_t)selectionModel()->selectedIndexes()[0].internalPointer() : 0 },
		 id = selId;
	int cnt = 0;
	emit SignalGetSelectionCount(id, cnt);

	if (nSelSize == 0)
	{
		return;
	}
	else
	{
		if (cnt && id != selId)
		{
			pact = new QAction(tr("&Copy Selected Images Here"), this);  // any number of images from a directory
			pact->setEnabled(true);
			connect(pact, &QAction::triggered, this, &AlbumTreeView::SlotCopyImages);
			menu.addAction(pact);

			pact = new QAction(tr("&Move Selected Images Here"), this);  // any number of images from a directory
			pact->setEnabled(true);
			connect(pact, &QAction::triggered, this, &AlbumTreeView::SlotMoveImages);
			menu.addAction(pact);

			menu.addSeparator();
		}
		pact = new QAction(tr("Add &Images..."), this);  // any number of images from a directory
		pact->setEnabled(true);
		connect(pact, &QAction::triggered, ptnv, &ThumbnailView::AddImages);
		menu.addAction(pact);

		pact = new QAction(tr("Add &Folder..."), this);  // one folder added to the folder tree inside this album
		connect(pact, &QAction::triggered, ptnv, &ThumbnailView::AddFolder);
		menu.addAction(pact);

		menu.addSeparator();
		pact = new QAction(tr("&Remove"), this);
		connect(pact, &QAction::triggered, this, &AlbumTreeView::SlotDeleteSelectedAlbum);
		menu.addAction(pact);

	}
	menu.exec(pevent->globalPos());
}

