#pragma once

#include <QtCore>
#include <QtWidgets/QWidget>
#include <QTextStream>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

#include "ui_textEditor.h"

class TextEditor : public QWidget
{
	Q_OBJECT
	
public:
	TextEditor(QWidget *parent = Q_NULLPTR);
	~TextEditor();
private:
	Ui::TextEditorClass	ui;
	
private slots:
	void on_tbBold_toggled(bool b);
	void on_tbCenterAlign_toggled(bool b);
	void on_tbIndent_toggled(bool b);
	void on_tbItalic_toggled(bool b);
	void on_tbJustify_toggled(bool b);
	void on_tbLeftAlign_toggled(bool b);
	void on_tbOpen_toggled(bool b);
	void on_tbRightAlign_toggled(bool b);
	void on_tbSave_toggled(bool b);
	void on_tbUndent_toggled(bool b);
	void on_tbUnderline_toggled(bool b);
};