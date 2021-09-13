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

#include "support.h"
#include "config.h"
#include "albums.h"
#include "structEdit.h"


// class 	 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// ************************************ AlbumTreeModel ***************************//
//			 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/*============================================================================
* TASK:  return raw count for parent
* EXPECTS:
* GLOBALS:
* REMARKS: model indices are relativ to parent
*--------------------------------------------------------------------------*/
int AlbumTreeModel::rowCount(const QModelIndex & parent) const
{
	if (_busy) return 0;
	AlbumMap &map = albumgen.Albums();
	if (!parent.isValid())		   // root element: it has 1 sub-item
		return map.size() ? 1 : 0;
	ID_t id = (ID_t)(parent.internalPointer());
	return map[id].SubAlbumCount();
}

/*============================================================================
* TASK:	   Create a model index for parent's  child
* EXPECTS:	row - parent relative index (< count of parent's children)
*			column: 0
*			parent: index of parent whose row-th child's index required
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
QModelIndex AlbumTreeModel::index(int row, int column, const QModelIndex & parent) const
{
	AlbumMap &map = albumgen.Albums();
	if (column || row < 0 || row >= map.size())
		return QModelIndex();

	if (!parent.isValid())				// root id for tree element
		return createIndex(row, column, quintptr( (map.size() ? ROOT_ALBUM_ID : 0)) );
	ID_t id = (ID_t)parent.internalPointer();
	id = map[id].IdOfItemOfType(ALBUM_ID_FLAG,row);
	return createIndex(row, column, quintptr(id));
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
	if (!index.isValid())				// root id for tree element
		return QVariant();

	if (!_busy && role == Qt::DisplayRole)
	{
		Album &ab = albumgen.Albums()[(ID_t)index.internalPointer()];
		QString s = ab.name;
		if (s.isEmpty())
			s = "/";
		return s + " ( " + ab.BareName() + " )";  // no language and extension
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
	if (!ind.isValid() || (ID_t)ind.internalPointer() == ROOT_ALBUM_ID)
		return QModelIndex();

	AlbumMap &map = albumgen.Albums();
	Album &ab = map[(ID_t)ind.internalPointer()];
	ID_t aParent = ab.parent;				   // get ID of parent
	if (aParent == 0)						   // no parent: topmost element
		return index(0, ind.column());		   // it is the first and only such

	Album &abp = map[aParent];				  // parent album
	ID_t bParent = abp.parent;				  // parent's parent
	if(bParent == 0)						  // parent is the topmost element
		return createIndex(0, ind.column(), quintptr(ROOT_ALBUM_ID));

	IdList albums = map[bParent].items;			// parent's parent
	int size = albums.size();					// go through its list
	for (int i = 0; i < size; ++i)
	{
		if (albums[i] == aParent)				// aParent contains ALBUM_ID_FLAG
			return createIndex(i, ind.column(), quintptr(aParent));
	}
	return QModelIndex();
}

/*============================================================================
* TASK:	   redraw tree
* EXPECTS:
* GLOBALS:
* REMARKS:
*--------------------------------------------------------------------------*/
void AlbumTreeModel::ModelChanged()
{
	beginResetModel(); 
	endResetModel();
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
	return false;
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
	return false;
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
