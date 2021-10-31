#pragma once

#include <QtCore>
#include <QtWidgets/QWidget>
#include <QTextStream>
#include <QtextList>

#include "ui_textEditor.h"

class TextEditor : public QWidget
{
	Q_OBJECT
	
public:
	TextEditor(QWidget *parent = Q_NULLPTR);
	~TextEditor();
	void Setup();
	void SetupFiles(QStringList files);
	void SetupWebFonts(QString fonts);
private:
	Ui::TextEditorClass	ui;
	void _SetFormat(QTextCharFormat fmt);
private slots:
	void on_tbBold_toggled(bool b);
	void on_tbCenterAlign_toggled(bool b);
	void on_tbIndent_clicked();
	void on_tbItalic_toggled(bool b);
	void on_tbJustify_toggled(bool b);
	void on_tbLeftAlign_toggled(bool b);
	void on_tbOpen_toggled(bool b);
	void on_tbRightAlign_toggled(bool b);
	void on_tbSave_toggled(bool b);
	void on_tbUndent_clicked();
	void on_tbUnderline_toggled(bool b);
	void on_cbFontFamily_currentTextChanged(QString gs);
	void on_cbPointSize_currentIndexChanged(int ix);

	void _CurrentCharFormatChanged(const QTextCharFormat& format);
	void _ModifyIndentation(int amount);
	void _FontChanged(const QFont& f);
	void _AlignmentChanged(Qt::Alignment a);
};