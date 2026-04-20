// falconGStyleEngine.cpp
#include "falconGStyleEngine.h"
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionGroupBox> 

// style string is created/changed in falcong.cpp
/*=============================================================
 *              painters
 * Each painter:
 *   is isolated
 *   is safe
 *   is compatible with your color‑driven stylesheet
 *   avoids Qt stylesheet engine bugs
 *   gives you full control
 *   is ready to plug into your drawControl / drawPrimitive overrides
 *------------------------------------------------------------*/

 // ---- GROUPBOX TITLE ----
 // Fixes disappearing titles, clipping, wrong background, wrong alignment.
void paintGroupBoxTitle(const QStyleOptionGroupBox* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    // Title rect
    QRect r = opt->rect;
    int h = opt->fontMetrics.height();
    QRect titleRect(r.left() + 8, r.top() - h / 2, r.width() - 16, h);

    // Title text
    p->setFont(widget? widget->font() : QFont());
    p->setPen(opt->palette.windowText().color());
    p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, opt->text);

    p->restore();
}

// ---- GROUPBOX FRAME ----
// Ensures consistent border thickness and avoids clipping the title.
void paintGroupBoxFrame(const QStyleOptionGroupBox* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect.adjusted(0, 8, 0, 0); // leave space for title
    QPen pen(opt->palette.mid().color(), 2);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawRect(r);

    p->restore();
}

// ---- TABBAR TAB ----
// Stable tab shape, hover, selected, pressed.
void paintTab(const QStyleOptionTab* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect;

    QColor bg;
    if (opt->state & QStyle::State_Selected)
        bg = opt->palette.highlight().color();
    else if (opt->state & QStyle::State_MouseOver)
        bg = opt->palette.button().color().lighter(110);
    else
        bg = opt->palette.button().color();

    p->setPen(opt->palette.mid().color());
    p->setBrush(bg);
    p->drawRoundedRect(r, 4, 4);

    p->setPen(opt->palette.buttonText().color());
    p->drawText(r, Qt::AlignCenter, opt->text);

    p->restore();
}

// ---- TOOLBOX TAB ----
// Same logic as QTabBar, but vertical layout.
void paintToolBoxTab(const QStyleOptionToolBox* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect;

    QColor bg = (opt->state & QStyle::State_Selected)
        ? opt->palette.highlight().color()
        : opt->palette.button().color();

    p->setPen(opt->palette.mid().color());
    p->setBrush(bg);
    p->drawRect(r);

    p->setPen(opt->palette.buttonText().color());
    p->drawText(r.adjusted(8, 0, -8, 0), Qt::AlignVCenter, opt->text);

    p->restore();
}

// ---- COMBOBOX ARROW ----
// Matches your skin colors and avoids Qt’s inconsistent arrow rendering.
void paintComboArrow(const QStyleOption* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect.adjusted(4, 4, -4, -4);

    QPolygon arrow;
    arrow << QPoint(r.left(), r.top())
        << QPoint(r.right(), r.top())
        << QPoint(r.center().x(), r.bottom());

    QColor c = (opt->state & QStyle::State_Enabled)
        ? opt->palette.buttonText().color()
        : opt->palette.mid().color();

    p->setBrush(c);
    p->setPen(Qt::NoPen);
    p->drawPolygon(arrow);

    p->restore();
}

// ---- SPINBOX ARROWS ----
// Up/down arrows with consistent geometry.
void paintSpinArrow(const QStyleOption* opt,
    QPainter* p,
    bool up)
{
    p->save();

    QRect r = opt->rect.adjusted(4, 4, -4, -4);

    QPolygon arrow;
    if (up)
        arrow << QPoint(r.center().x(), r.top())
        << QPoint(r.right(), r.bottom())
        << QPoint(r.left(), r.bottom());
    else
        arrow << QPoint(r.left(), r.top())
        << QPoint(r.right(), r.top())
        << QPoint(r.center().x(), r.bottom());

    QColor c = (opt->state & QStyle::State_Enabled)
        ? opt->palette.buttonText().color()
        : opt->palette.mid().color();

    p->setBrush(c);
    p->setPen(Qt::NoPen);
    p->drawPolygon(arrow);

    p->restore();
}

// ---- SLIDER HANDLE ----
// Stable handle size and color.
void paintSliderHandle(const QStyleOptionSlider* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect.adjusted(2, 2, -2, -2);

    QColor bg = (opt->state & QStyle::State_MouseOver)
        ? opt->palette.highlight().color()
        : opt->palette.button().color();

    p->setPen(opt->palette.mid().color());
    p->setBrush(bg);
    p->drawEllipse(r);

    p->restore();
}

// ---- PROGRESSBAR CHUNK ----
// Matches your %15 color and avoids Qt’s inconsistent chunk spacing.
void paintProgressChunk(const QStyleOptionProgressBar* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect.adjusted(1, 1, -1, -1);

    p->setPen(Qt::NoPen);
    p->setBrush(opt->palette.highlight().color());
    p->drawRoundedRect(r, 3, 3);

    p->restore();
}

// ---- TOOLBUTTON ----
// Stable hover/pressed states without relying on stylesheet engine.
void paintToolButton(const QStyleOptionToolButton* opt,
    QPainter* p,
    const QWidget* widget)
{
    p->save();

    QRect r = opt->rect;

    QColor bg;
    if (opt->state & QStyle::State_Sunken)
        bg = opt->palette.highlight().color().darker(120);
    else if (opt->state & QStyle::State_MouseOver)
        bg = opt->palette.highlight().color();
    else
        bg = opt->palette.button().color();

    p->setPen(Qt::NoPen);
    p->setBrush(bg);
    p->drawRoundedRect(r, 12, 12);

    p->setPen(opt->palette.buttonText().color());
    p->drawText(r, Qt::AlignCenter, opt->text);

    p->restore();
}

// ---- CHECKBOX / RADIO INDICATOR ----
// Consistent indicator size and color.
void paintIndicator(const QStyleOption* opt,
    QPainter* p,
    bool radio)
{
    p->save();

    QRect r = opt->rect.adjusted(2, 2, -2, -2);

    QColor border = opt->palette.mid().color();
    QColor fill = (opt->state & QStyle::State_On)
        ? opt->palette.highlight().color()
        : opt->palette.base().color();

    p->setPen(border);
    p->setBrush(fill);

    if (radio)
        p->drawEllipse(r);
    else
        p->drawRect(r);

    p->restore();
}

// ---- TREEVIEW BRANCH ----
// Fixes the two rules you said “do not work”.
void paintTreeBranch(const QStyleOption* opt,
    QPainter* p,
    bool open)
{
    p->save();

    QRect r = opt->rect.adjusted(4, 4, -4, -4);

    QPolygon arrow;
    if (open)
        arrow << QPoint(r.left(), r.top())
        << QPoint(r.right(), r.top())
        << QPoint(r.center().x(), r.bottom());
    else
        arrow << QPoint(r.left(), r.top())
        << QPoint(r.left(), r.bottom())
        << QPoint(r.right(), r.center().y());

    p->setBrush(opt->palette.buttonText().color());
    p->setPen(Qt::NoPen);
    p->drawPolygon(arrow);

    p->restore();
}

// ------------------------------------------------------------
// 1. METRICS
// ------------------------------------------------------------
int MyProxyStyle::pixelMetric(PixelMetric metric,
    const QStyleOption* opt,
    const QWidget* widget) const
{
    switch (metric)
    {
        case PM_DefaultFrameWidth:
            return 2;

        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return 13;

        case PM_SliderThickness:
        case PM_SliderLength:
            return 16;

        default:
            return QProxyStyle::pixelMetric(metric, opt, widget);
    }
}

// ------------------------------------------------------------
// 2. SUBCONTROL GEOMETRY
// ------------------------------------------------------------
QRect MyProxyStyle::subControlRect(ComplexControl control,
    const QStyleOptionComplex* opt,
    SubControl subControl,
    const QWidget* widget) const
{
    if (control == CC_GroupBox && subControl == SC_GroupBoxLabel)
    {
        QRect r = QProxyStyle::subControlRect(control, opt, subControl, widget);
        r.moveTop(r.top() - 4);
        return r;
    }

    return QProxyStyle::subControlRect(control, opt, subControl, widget);
}

// ------------------------------------------------------------
// 3. PRIMITIVES
// ------------------------------------------------------------
void MyProxyStyle::drawPrimitive(PrimitiveElement pe,
    const QStyleOption* opt,
    QPainter* p,
    const QWidget* widget) const
{
    switch (pe)
    {
        case PE_FrameGroupBox:
        {
            const QStyleOptionGroupBox* gb =
                qstyleoption_cast<const QStyleOptionGroupBox*>(opt);
            if (!gb)
                break;

            // Paint frame
            paintGroupBoxFrame(gb, p, widget);

            // Paint title
            paintGroupBoxTitle(gb, p, widget);
            return;
        }

        // other primitives...
        default:
            break;
    }

    QProxyStyle::drawPrimitive(pe, opt, p, widget);
}

// ------------------------------------------------------------
// 4. CONTROLS
// ------------------------------------------------------------
void MyProxyStyle::drawControl(ControlElement element,
    const QStyleOption* opt,
    QPainter* p,
    const QWidget* widget) const
{
    switch (element)
    {
        case CE_TabBarTab:
            paintTab(static_cast<const QStyleOptionTab*>(opt), p, widget);
            return;

        case CE_ToolBoxTab:
            paintToolBoxTab(static_cast<const QStyleOptionToolBox*>(opt), p, widget);
            return;

        case CE_ComboBoxLabel:
            QProxyStyle::drawControl(element, opt, p, widget);
            paintComboArrow(opt, p, widget);
            return;

        case CE_ProgressBarContents:
            paintProgressChunk(static_cast<const QStyleOptionProgressBar*>(opt), p, widget);
            return;

        case CE_ToolButtonLabel:
            paintToolButton(static_cast<const QStyleOptionToolButton*>(opt), p, widget);
            return;

        default:
            break;
    }

    QProxyStyle::drawControl(element, opt, p, widget);
}

void MyProxyStyle::drawComplexControl(ComplexControl control,
    const QStyleOptionComplex* opt,
    QPainter* p,
    const QWidget* widget) const
{
    switch (control)
    {
        case CC_SpinBox:
        {
            const QStyleOptionSpinBox* sb = qstyleoption_cast<const QStyleOptionSpinBox*>(opt);
            if (!sb) break;

            // Let base draw frame, etc.
            QProxyStyle::drawComplexControl(control, opt, p, widget);

            // Up arrow
            if (opt->subControls & SC_SpinBoxUp)
            {
                QStyleOption upOpt = *opt;
                upOpt.rect = subControlRect(control, opt, SC_SpinBoxUp, widget);
                paintSpinArrow(&upOpt, p, true);
            }

            // Down arrow
            if (opt->subControls & SC_SpinBoxDown)
            {
                QStyleOption downOpt = *opt;
                downOpt.rect = subControlRect(control, opt, SC_SpinBoxDown, widget);
                paintSpinArrow(&downOpt, p, false);
            }
            return;
        }

        case CC_Slider:
        {
            const QStyleOptionSlider* sl = qstyleoption_cast<const QStyleOptionSlider*>(opt);
            if (!sl) break;

            // Let base draw groove, etc.
            QProxyStyle::drawComplexControl(control, opt, p, widget);

            if (opt->subControls & SC_SliderHandle)
            {
                QStyleOptionSlider handleOpt(*sl);
                handleOpt.rect = subControlRect(control, opt, SC_SliderHandle, widget);
                paintSliderHandle(&handleOpt, p, widget);
            }
            return;
        }

        default:
            break;
    }

    QProxyStyle::drawComplexControl(control, opt, p, widget);
}


// ------------------------------------------------------------
// 5. PALETTE + FONT
// ------------------------------------------------------------
QPalette MyProxyStyle::standardPalette() const
{
    return QProxyStyle::standardPalette();
}
