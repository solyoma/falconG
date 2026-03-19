#pragma once
#include <QObject>
#include <QFontDatabase>
#include <QHash>

class FontUtils : public QObject {
    Q_OBJECT
public:
    explicit FontUtils(QObject* parent = nullptr) : QObject(parent) {}

    QStringList ScanInstalledFontsInFolder(const QString& targetFolder);
    QStringList QueryFontVariants(const QString& familyName) const;     // in global QFontDataBase
        // next function checks font availability 
        // downloads and unzips them if necessary
        // and ads them to global QFontDataBase
    void EnsureFontAvailable(const QString& fontName,
        const QString& targetFolder,
        std::function<void(bool, QStringList)> callback);

private:
    QStringList _slFontErrors;
};