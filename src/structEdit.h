#pragma once

#include <QAbstractItemModel>

/*============================================================================
  * TASK:		model for tree view of albums
  * REMARKS:	- uses the data stored in the map AlbumMap (albums.h)
  *				- columnCount is the maximum depth of the hierarchy
  *				- rowCount
 *--------------------------------------------------------------------------*/
class AlbumTreeModel : public QAbstractItemModel
{
	Q_OBJECT

	bool _busy = false;
public:
	AlbumTreeModel(QObject *parent = Q_NULLPTR) : QAbstractItemModel(parent) {}

// these are for reading
	int columnCount(const QModelIndex &parent = QModelIndex()) const;	// depth of tree
	int rowCount(const QModelIndex &parent = QModelIndex()) const;		//
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QModelIndex parent(const QModelIndex &index) const;

	void ModelChanged();
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
