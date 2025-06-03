#pragma once
#include <QLineEdit>
#include <QColor>
#include <QComboBox>
#include <QAbstractItemModel>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomLineEdit(QComboBox* combo, QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QComboBox* m_combo;
    QColor m_placeholderColor;
    void updateCompletion();
    QString currentCompletion() const;
    //int completionLength() const;
};