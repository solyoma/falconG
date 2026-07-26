#pragma once
#include <QObject>
#include <QApplication>
#include <QStyleFactory>
#include <QPointer>

class SkinManager : public QObject
{
    Q_OBJECT

public:
    explicit SkinManager(QObject* parent = nullptr);

    // Apply your custom skin (with your generated stylesheet)
    void ApplySkin(const QString& styleSheet);

    // Revert to default Fusion style (no proxy style, no stylesheet)
    void ApplyDefaultFusion();

    // Returns true if custom skin is active
    bool IsCustomSkinActive() const { return m_customActive; }

private:
    void RepolishAllWidgets();

private:
    bool m_customActive = false;
};

/* Usage:
* Create the manager in main.cpp after ctreating the application:
*  	QApplication a(argc, argv);
*	a.setStyle(QStyleFactory::create("Fusion"));
*	SkinManager skinManager;
* Apply your custom skin (with your generated stylesheet):
*   QString ss = buildStyleSheetFromScheme(index);
*   skinMgr->ApplySkin(ss);
* To revert to default Fusion style (no proxy style, no stylesheet):
*  skinMgr->ApplyDefaultFusion();
* Check if custom skin is active:
*  if (skinMgr->IsCustomSkinActive()) { ... }
  */