#pragma once

#include <QtCore>
#include <QtWidgets/QWidget>
#include <QDialog>
#include <QFileDialog>
#include "ui_sourcehistory.h"
#include <QStringList>

class Model : public QAbstractItemModel
{
	Q_OBJECT

		QStringList &_lstSrc,			 // external list
					_list,				 // visible elements
					_deletedList;		 // deleted elements (for undo)
public:
	Model(QStringList &list, QObject *parent = Q_NULLPTR) : QAbstractItemModel(parent), _lstSrc(list), _list(list) {}
	int UndoCount() const { return _deletedList.size(); }
	void Undo();
	void AcceptChanges() { _lstSrc = _list; }
	bool Insert(QString s)	// returns changed state
	{
#ifdef _MSC_VER		// windows: LC UC letters are the same in path
		s = s.toLower();
#endif
		if (s[s.length() - 1] != '/')
			s += "/";
		if (_lstSrc.indexOf(s) < 0)	
		{
			beginResetModel();
			_list.insert(0, s);
			endResetModel();
			return true;
		}
		return false;
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const { return _list.size(); }
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const { return 1; }
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QModelIndex parent(const QModelIndex &index) const;
	bool removeRows(int afrom, int count, const QModelIndex &parent = QModelIndex());
};


class SourceHistory : public QDialog
{
	Q_OBJECT

public:
	SourceHistory(QStringList &list, QString defaultPath, QWidget *parent = Q_NULLPTR);
	static int Selected() { return _selected; } // -1: cancelled
	static bool Changed() { return _changed; }

private:
	Ui::SourceHistoryClass ui;
	static int _selected;
	static bool _changed;
	QString _srcPath;
private slots:
	void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void on_btnAddDirectory_clicked();
	void on_btnCancel_clicked();
	void on_btnSelect_clicked();
	void on_lvHistory_doubleClicked();
	void on_btnDelete_clicked();
	void on_btnUndelete_clicked();
};
