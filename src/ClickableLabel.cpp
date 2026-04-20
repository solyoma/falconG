#include "clickablelabel.h"

#include <QColor>
#include <QStyle>

#include "stylehandler.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {

}

ClickableLabel::~ClickableLabel() {}

static StyleHandler __handler;
static QString __itemName,
               __origBackgroundColor;

void ClickableLabel::mousePressEvent(QMouseEvent* event) 
{
    __handler.FromCss(styleSheet());
    QStringList qsl = __handler.GetListOfSelectors();
    if (qsl.size())
        __itemName = qsl[0];
    __origBackgroundColor = __handler.GetItem("", __itemName, "background-color");
    __origBackgroundColor = __handler.ColorToStr(__origBackgroundColor);
    QColor qc, qcn;
    if (__origBackgroundColor.isEmpty())
    {
        qcn = QColor("#555500");
    }
    else
	{
        qc = __origBackgroundColor;
		qcn = qc.darker();
	}

    
    __handler.SetItem(__itemName, "background-color", qcn.name() );
    setStyleSheet(__handler.StyleSheet() );
}
void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) 
{
    if (__origBackgroundColor.isEmpty())
        __handler.RemoveItem(__itemName, "background-color");
    else
        __handler.SetItem(__itemName, "background-color", __origBackgroundColor );
    setStyleSheet(__handler.StyleSheet() );  
    __origBackgroundColor.clear();
    __itemName.clear();
    // DEBUG
	//qDebug("MouseRelease: styleSheet: %s", styleSheet().toStdString().c_str());
    // /DEBUG
    emit clicked();
}