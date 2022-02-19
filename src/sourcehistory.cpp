#include "sourcehistory.h"
#include <QStandardItemModel>

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

int Model::IndexOf(QString qs) // in source list
{
	for (int i = 0; i < _lstSrc.size(); ++i)
#ifdef _MSC_VER		// windows: LC UC letters are the same in path
		if (qs.toLower() == _lstSrc[i].toLower())
#else
		if (qs == _lstSrc[i])
#endif
			return i;
	return -1;
}

bool Model::Insert(QString s)	// returns changed state
{
	if (s[s.length() - 1] != '/')
		s += "/";
	if (IndexOf(s) < 0)
	{
		beginResetModel();
		_list.insert(0, s);
		endResetModel();
		return true;
	}
	return false;
}

void Model::Sort() // of source list
{
	const int n = _lstSrc.size();
	std::vector<int> ix(n);
	for (int i = 0; i < n; ++i)
		ix[i] = i;
	auto sf = [&](int left, int right)
	{
#ifdef _MSC_VER
		return _lstSrc[ix[left]].toLower() < _lstSrc[ix[right]].toLower();
#else
		return _lstSrc[ix[left]] < _lstSrc[ix[right]];
#endif
	};

	beginResetModel();

	std::sort(ix.begin(), ix.end(), sf);
	for (int i = 0; i < n; ++i)
		std::swap(_lstSrc[i], _lstSrc[ix[i]]);
	endResetModel();
}

void Model::MoveLine(int &which, bool up)
{
	int pos = which;
	QString qs = _lstSrc[pos];
	pos += up ? -1 : 1;
	if (pos < 0 || pos > _lstSrc.size())
		return;
	beginResetModel();
	_lstSrc.removeAt(which);
	_lstSrc.insert(pos, qs);
	endResetModel();
	which = pos;
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
SourceHistory::SourceHistory(int maxSize, QStringList &list, QString defaultPath, QWidget *parent)
	:_list(list), _srcPath(defaultPath), QDialog(parent)
{
	ui.setupUi(this);

	_selected = -1;

	ui.sbHistSize->setValue(maxSize);

	ui.lvHistory->setModel(new Model(list, parent));
	// get selection changed info from the view (no easier way...)
	connect(ui.lvHistory->selectionModel(),	&QItemSelectionModel::selectionChanged,
		this, &SourceHistory::SelectionChanged);
	_pModel = dynamic_cast<Model*>(ui.lvHistory->model());
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
	_pModel->AcceptChanges();
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
	_pModel->removeRow(_selected);
	_selected = -1;
	_changed = true;
	ui.btnUndelete->setEnabled(_pModel->UndoCount());
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
	_pModel->Undo();
	ui.btnUndelete->setEnabled(false);
	ui.btnDelete->setEnabled(false);
	_changed = true;
	_selected = -1;
}

void SourceHistory::on_btnSort_clicked()
{
	_pModel->Sort();
	_changed = true;
	_selected = -1;
}

void SourceHistory::on_btnMoveUp_clicked()
{
	_pModel->MoveLine(_selected, true);
}
void SourceHistory::on_btnMoveDown_clicked()
{
	_pModel->MoveLine(_selected, false);
}

void SourceHistory::on_sbHistSize_valueChanged(int n)
{
	_changed = true;
	_selected = -1;
}
