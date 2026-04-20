#include "SkinManager.h"
#include "falconGStyleEngine.h"   // your proxy style class
#include <QWidget>

SkinManager::SkinManager(QObject* parent)
    : QObject(parent)
{
}

void SkinManager::applySkin(const QString& styleSheet)
{
    m_customActive = true;

    // Remove old style (Qt does NOT delete it automatically)
    QStyle* old = qApp->style();

    // Install new proxy style based on Fusion
    qApp->setStyle(new MyProxyStyle(QStyleFactory::create("Fusion")));

    // Apply your color-driven stylesheet
    qApp->setStyleSheet(styleSheet);

    // Force full polish/unpolish cycle
    repolishAllWidgets();
}

void SkinManager::applyDefaultFusion()
{
    m_customActive = false;

    // Remove stylesheet FIRST (important!)
    qApp->setStyleSheet("");

    // Remove old style
    QStyle* old = qApp->style();

    // Install clean Fusion style
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // Force full polish/unpolish cycle
    repolishAllWidgets();
}

void SkinManager::repolishAllWidgets()
{
    for (QWidget* w : qApp->allWidgets())
    {
        if (!w)
            continue;

        w->style()->unpolish(w);
        w->style()->polish(w);
        w->update();
    }
}
