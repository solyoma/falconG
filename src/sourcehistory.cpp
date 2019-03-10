#include "sourcehistory.h"
#include <QAbstractItemModel>


// ************************ class Model ********************************

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
	{
		_deletedList << _list[afrom];
		_list.removeAt(afrom++);		// ++ ? why
	}
	endResetModel();

	return true;
}

// *************************** class SourceHistory **************************


int SourceHistory::_selected = -1;
bool SourceHistory::_changed = false;
/*============================================================================
 * TASK:
 * EXPECTS:
 * GLOBALS:
 * RETURNS
 * REMARKS:
*--------------------------------------------------------------------------*/
SourceHistory::SourceHistory(QStringList &list, QString defaultPath, QWidget *parent)
	: _srcPath(defaultPath), QDialog(parent)
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

/*=============================================================
 * TASK:   Add a directory to the list of directories
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void SourceHistory::on_btnAddDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Add a new source directory", _srcPath.isEmpty() ? "" : _srcPath);
	if (!s.isEmpty())
	{
		Model *pmodel;
		pmodel = dynamic_cast<Model*>(ui.lvHistory->model());
		_changed = pmodel->Insert(s);
	}
}

void SourceHistory::on_btnCancel_clicked()
{
	_selected = -1;
	_changed = false;
	close();
}

/*=============================================================
 * TASK:	set the last used list into the caller's list
 *			and signal when selected or changed
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
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
	_changed = true;
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
	_changed = true;
	_selected = -1;
}
