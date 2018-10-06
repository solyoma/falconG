#pragma once

#include <QtCore>
#include <QtWidgets/QWidget>
#include <QDialog>
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
	SourceHistory(QStringList &list, QWidget *parent = Q_NULLPTR);
	static int Selected() { return _selected; } // -1: cancelled

private:
	Ui::SourceHistoryClass ui;
	static int _selected;
private slots:
	void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void on_btnCancel_clicked();
	void on_btnSelect_clicked();
	void on_lvHistory_doubleClicked();
	void on_btnDelete_clicked();
	void on_btnUndelete_clicked();
};
