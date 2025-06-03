#include "CustomLineEdit.h"
#include <QPainter>
#include <QKeyEvent>
#include <QAbstractItemModel>
#include <QAbstractItemView>

CustomLineEdit::CustomLineEdit(QComboBox* combo, QWidget* parent)
    : QLineEdit(parent), m_combo(combo)
{
    m_placeholderColor = palette().color(QPalette::PlaceholderText);
    connect(this, &QLineEdit::textEdited, this, [this]{ updateCompletion(); });
}

void CustomLineEdit::keyPressEvent(QKeyEvent* event)
{
    int pos = cursorPosition();
    QString prefix = text().left(pos);
    QChar nextChar = event->text().isEmpty() ? QChar() : event->text().at(0);

    if (event->key() == Qt::Key_Backspace) {
        QLineEdit::keyPressEvent(event);
        updateCompletion();
        return;
    }
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
        QLineEdit::keyPressEvent(event);
        update();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        setText(currentCompletion());
        setCursorPosition(text().length());
        update();
        return;
    }

    // Only allow valid next characters
    if (!nextChar.isNull()) {
        QAbstractItemModel* model = m_combo->model();
        bool valid = false;
        int newPos = pos;
        QString newPrefix = text();
        if (hasSelectedText() && selectionStart() == pos) {
            // If user is overwriting placeholder, replace selection
            newPrefix = text().left(pos) + nextChar + text().mid(cursorPosition() + selectedText().length());
            newPos = pos + 1;
        } else {
            newPrefix = text().left(pos) + nextChar + text().mid(pos);
            newPos = pos + 1;
        }
        for (int i = 0; i < model->rowCount(); ++i) {
            QString item = model->data(model->index(i, 0)).toString();
            if (item.startsWith(newPrefix, Qt::CaseInsensitive)) {
                valid = true;
                break;
            }
        }
        if (valid) {
            insert(nextChar);
            setCursorPosition(newPos);
            updateCompletion();
        }
        // else ignore
    }
}

void CustomLineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);
    int pos = cursorPosition();
    QString prefix = text().left(pos);
    QString completion = currentCompletion();
    if (completion.length() > pos) {
        QPainter p(this);
        QFontMetrics fm(font());
        int x = fm.horizontalAdvance(displayText().left(pos));
        QString tail = completion.mid(pos);
        QRect r = rect();
        int y = (r.height() + fm.ascent() - fm.descent()) / 2;
        p.setPen(m_placeholderColor);
        p.drawText(x + 4, y, tail);
    }
}

QString CustomLineEdit::currentCompletion() const
{
    int pos = cursorPosition();
    QString prefix = text().left(pos);
    QAbstractItemModel* model = m_combo->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        QString item = model->data(model->index(i, 0)).toString();
        if (item.startsWith(prefix, Qt::CaseInsensitive))
            return item;
    }
    return text();
}

void CustomLineEdit::updateCompletion()
{
    m_combo->setEditText(text());
    if (!m_combo->view()->isVisible())
        m_combo->showPopup();
    update();
}