#include "sourcehistory.h"
#include <QAbstractItemModel>

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void Model::Undo()
{
	beginResetModel();
	_list += _deletedList; 
	_deletedList.clear(); 
	endResetModel();
}

/*============================================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS
 * REMARKS:
*--------------------------------------------------------------------------*/
QModelIndex Model::index(int row, int column, const QModelIndex & parent) const
{
	if (column)
		return QModelIndex();
	return createIndex(row, column);
}

/*============================================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS
 * REMARKS:
*--------------------------------------------------------------------------*/
QVariant Model::data(const QModelIndex & index, int role) const
{
	if (!index.isValid())			// when called first the index is invalid
		return QVariant();

	if (role == Qt::DisplayRole)
		return _list[index.row()];

	return QVariant();
}

/*============================================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS
 * REMARKS:
*--------------------------------------------------------------------------*/
QModelIndex Model::parent(const QModelIndex & index) const
{
	return QModelIndex();
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
bool Model::removeRows(int afrom, int count, const QModelIndex & parent)
{
	beginResetModel();
	while (count--)
		_deletedList << _list[afrom];	_list.removeAt(afrom++);
	endResetModel();

	return true;
}

// *************************** SourceHistory
int SourceHistory::_selected;
/*============================================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS
 * REMARKS:
*--------------------------------------------------------------------------*/
SourceHistory::SourceHistory(QStringList &list, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	_selected = -1;

	ui.lvHistory->setModel(new Model(list, parent));
	// get selection changed info from the view (no easier way...)
	connect(ui.lvHistory->selectionModel(),	&QItemSelectionModel::selectionChanged,
		this, &SourceHistory::SelectionChanged);
}

/*============================================================================
* TASK:
* EXPECTS:
* GLOBALS:
* RETURNS
* REMARKS:
*--------------------------------------------------------------------------*/
void SourceHistory::SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	_selected = selected.size() ? selected[0].top() : -1;
	ui.btnDelete->setEnabled(_selected >= 0);
}

void SourceHistory::on_btnCancel_clicked()
{
	close();
}

void SourceHistory::on_btnSelect_clicked()
{
	Model *pmodel;
	pmodel = dynamic_cast<Model*>(ui.lvHistory->model());
	pmodel->AcceptChanges();
	close();
}

void SourceHistory::on_lvHistory_doubleClicked()
{
	on_btnSelect_clicked();
}

/*=============================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void SourceHistory::on_btnDelete_clicked()
{
	Model *pmodel;
	pmodel = dynamic_cast<Model*>(ui.lvHistory->model());
	pmodel->removeRow(_selected);
	_selected = -1;
	ui.btnUndelete->setEnabled(pmodel->UndoCount());
	ui.btnDelete->setEnabled(false);
}

/*=============================================================
* TASK:
* EXPECTS:
* GLOBALS:
* RETURNS:
* REMARKS:
*------------------------------------------------------------*/
void SourceHistory::on_btnUndelete_clicked()
{
	Model *pmodel;
	pmodel = dynamic_cast<Model*>(ui.lvHistory->model());
	pmodel->Undo();
	ui.btnUndelete->setEnabled(false);
	ui.btnDelete->setEnabled(false);
	_selected = -1;
}
