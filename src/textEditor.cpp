
#include "textEditor.h"

TextEditor::TextEditor(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
}

TextEditor::~TextEditor()
{
}

void TextEditor::SetupWebFonts(QString fonts)
{

    // set up WEB fonts
    QStringList qsl = fonts.split('|');
    qsl.sort(Qt::CaseInsensitive);
    ui.cbFontFamily->clear();
    ui.cbFontFamily->addItem("Default");
    for (auto s : qsl)
    {
        if (!s.isEmpty())
        {
            s.replace('+', ' ');
            ui.cbFontFamily->addItem(s);
        }
    }
    if (ui.cbFontFamily->count())
        ui.cbFontFamily->setCurrentIndex(0);

    ui.cbFontFamily->setEnabled(qsl.size());
}

void TextEditor::Setup()
{
}

void TextEditor::_SetFormat(QTextCharFormat fmt)
{
    QTextEdit* textEdit = ui.edtText;
    QTextCursor cursor = textEdit->textCursor();
//    if (!cursor.hasSelection())
//        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(fmt);
    textEdit->mergeCurrentCharFormat(fmt);
    textEdit->setFocus();
}

void TextEditor::_ModifyIndentation(int amount)
{
    QTextEdit* textEdit = ui.edtText;
    QTextCursor cursor = textEdit->textCursor();
    cursor.beginEditBlock();
    if (cursor.currentList()) {
        QTextListFormat listFmt = cursor.currentList()->format();
        // See whether the line above is the list we want to move this item into,
        // or whether we need a new list.
        QTextCursor above(cursor);
        above.movePosition(QTextCursor::Up);
        if (above.currentList() && listFmt.indent() + amount == above.currentList()->format().indent()) {
            above.currentList()->add(cursor.block());
        }
        else {
            listFmt.setIndent(listFmt.indent() + amount);
            cursor.createList(listFmt);
        }
    }
    else {
        QTextBlockFormat blockFmt = cursor.blockFormat();
        blockFmt.setIndent(blockFmt.indent() + amount);
        cursor.setBlockFormat(blockFmt);
    }
    cursor.endEditBlock();
    ui.edtText->setFocus();
}

void TextEditor::_CurrentCharFormatChanged(const QTextCharFormat& format)
{
    _FontChanged(format.font());
}

void TextEditor::_FontChanged(const QFont& f)
{
    ui.cbFontFamily->setCurrentIndex(ui.cbFontFamily->findText(QFontInfo(f).family()));
    ui.cbPointSize->setCurrentIndex (ui.cbPointSize->findText(QString::number(f.pointSize())));
    ui.tbBold->setChecked(f.bold());
    ui.tbItalic->setChecked(f.italic());
    ui.tbUnderline->setChecked(f.underline());
    ui.edtText->setFocus();
}

void TextEditor::_AlignmentChanged(Qt::Alignment a)
{
    QTextEdit *textEdit = ui.edtText;
    if (a & Qt::AlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a & Qt::AlignHCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a & Qt::AlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a & Qt::AlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}

void TextEditor::on_tbBold_toggled(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(b ? QFont::Bold : QFont::Normal);
    _SetFormat(fmt);
}
void TextEditor::on_tbCenterAlign_toggled(bool b)
{
    _AlignmentChanged(Qt::AlignCenter);
}
void TextEditor::on_tbIndent_clicked()
{
    _ModifyIndentation(1);
}
void TextEditor::on_tbItalic_toggled(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(b);
    _SetFormat(fmt);
}
void TextEditor::on_tbJustify_toggled(bool b)
{
    _AlignmentChanged(Qt::AlignJustify);
}
void TextEditor::on_tbLeftAlign_toggled(bool b)
{
    _AlignmentChanged(Qt::AlignLeft);

}
void TextEditor::on_tbOpen_toggled(bool b)
{

}
void TextEditor::on_tbRightAlign_toggled(bool b)
{
    _AlignmentChanged(Qt::AlignRight);

}
void TextEditor::on_tbSave_toggled(bool b)
{

}
void TextEditor::on_tbUndent_clicked()
{
    _ModifyIndentation(-1);
}
void TextEditor::on_tbUnderline_toggled(bool b)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(b);
    _SetFormat(fmt);
}

void TextEditor::on_cbFontFamily_currentTextChanged(QString qs)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(qs);
    _SetFormat(fmt);
}

void TextEditor::on_cbPointSize_currentIndexChanged(int val)
{
    qreal pointSize = val;
    if (val > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        _SetFormat(fmt);
    }
}
