#pragma once
#include <QObject>
#include <QtWidgets>
#include <QTreeView>
#include <QAbstractItemModel>

/*============================================================================
  * TASK:		model for tree view of albums
  * REMARKS:	- uses the data stored in the map AlbumMap (albums.h)
  *				- columnCount() = 1 is the maximum depth of the hierarchy
  *					for parent
  *				- rowCount() is either the total size of _albumMap(albumgen)
  *						or the count of sub-albums in 'parent'
  *				- internal pointer is the ID of the album
 *--------------------------------------------------------------------------*/
class AlbumTreeModel : public QAbstractItemModel
{
	Q_OBJECT

	bool _busy = false;
public:
	AlbumTreeModel(QObject *parent = Q_NULLPTR) : QAbstractItemModel(parent) {}

// these are for reading
	int columnCount(const QModelIndex &parent = QModelIndex()) const;	// depth of tree rel. to parent
	int rowCount(const QModelIndex &parent = QModelIndex()) const;		//
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QModelIndex parent(const QModelIndex &index) const;

	void BeginResetModel()
	{
		beginResetModel();
	}
	void EndResetModel() 
	{
		endResetModel();
	}

// these are moving around
	QModelIndex CreateIndex(int row, int column, int64_t id)	// ID_t
	{
		return createIndex(row, column, quintptr(id));
	}
	
// These are for editing
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value,
		int role = Qt::EditRole) override;
	bool setHeaderData(int section, Qt::Orientation orientation,
		const QVariant &value, int role = Qt::EditRole) override;

	bool insertColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) override;
	bool removeColumns(int position, int columns,
		const QModelIndex &parent = QModelIndex()) override;
	bool insertRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) override;
	bool removeRows(int position, int rows,
		const QModelIndex &parent = QModelIndex()) override;

};

class ThumbnailView;					  // in thumbnailView.h for connecting there
#ifndef ID_T_DEFINED
	using ID_t = int64_t;		// almost all ID's are CRC32 values extended with leading bits when collison
	using IntList = QVector<int>;
#endif

class AlbumTreeView : public QTreeView
{
	Q_OBJECT

		ThumbnailView* ptnv = nullptr;
public:
	AlbumTreeView(QWidget* parent = nullptr);
	void SetTnv(ThumbnailView* p) { ptnv = p; }

signals:
	void SignalDeleteSelectedList(ID_t albumId, IntList& list, bool iconsForThisAlbum);

public slots:
	void DeleteSelectedAlbum();

protected:
	void contextMenuEvent(QContextMenuEvent* pevent);

};