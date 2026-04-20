// falconGStyleEngine.h
#pragma once
#include <QProxyStyle>
#include <QStyleFactory>
#include <QPainter>

class MyProxyStyle : public QProxyStyle
{
public:
    explicit MyProxyStyle(QStyle* base = QStyleFactory::create("Fusion"))
        : QProxyStyle(base) {
    }

    // ---- Metrics (spacing, padding, sizes) ----
    int pixelMetric(PixelMetric metric,
        const QStyleOption* opt,
        const QWidget* widget) const override;

    // ---- Subcontrol geometry (groupbox title, tabs, arrows) ----
    QRect subControlRect(ComplexControl control,
        const QStyleOptionComplex* opt,
        SubControl subControl,
        const QWidget* widget) const override;

    // ---- Painting primitives (frames, arrows, handles) ----
    void drawPrimitive(PrimitiveElement pe,
        const QStyleOption* opt,
        QPainter* p,
        const QWidget* widget) const override;

    // ---- Painting controls (groupbox title, tabs, buttons) ----
    void drawControl(ControlElement element,
        const QStyleOption* opt,
        QPainter* p,
        const QWidget* widget) const override;

    void MyProxyStyle::drawComplexControl(ComplexControl control,
        const QStyleOptionComplex* opt,
        QPainter* p,
        const QWidget* widget) const override;

    // ---- Standard palette (base colors) ----
    QPalette standardPalette() const override;
};
