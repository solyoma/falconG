#include "fontutils.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QPointer>
#include <QNetworkReply>
#include <QDirIterator>
#include <QFileInfo>

// QuaZip
#include <quazip.h>
#include <quazipfile.h>

QStringList FontUtils::_ExtractAllTtf(const QString& zipPath, const QString& targetDir)
{
    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip))
        return {};

    QuaZipFile file(&zip);
    QStringList extracted;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        QString name = zip.getCurrentFileName();

        // Prefer Regular.ttf
        if (!name.endsWith(".ttf") && !name.endsWith(".otf"))
            continue;

        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        file.close();

        QDir dir(targetDir);
        dir.mkpath(".");
        QString extractedPath = dir.filePath(QFileInfo(name).fileName());

        QFile out(extractedPath);
        out.open(QIODevice::WriteOnly);
        out.write(data);
        out.close();

        extracted << extractedPath;
    }

    zip.close();
    return extracted;
}

void FontUtils::DownloadAndLoadFont(const QString& fontName,
        const QString& targetFolder,
        std::function<void(bool, QString)> callback)
{
    QDir dir(targetFolder+"/downloadedfonts");
    dir.mkpath(".");

    if (!QueryFontVariants(fontName).isEmpty())  // already loaded?
    {
        callback(true, fontName);
        return;
    }

    QString zipPath = dir.filePath(fontName + ".zip");

    // Already downloaded?
    if (QFile::exists(zipPath))
    {
        QStringList ttfs = _ExtractAllTtf(zipPath, targetFolder + "/fonts");
        QString family;
        for (const QString& ttf : ttfs) {
            int id = QFontDatabase::addApplicationFont(ttf);
            if (family.isEmpty())
                family = QFontDatabase::applicationFontFamilies(id).value(0);
        }
        callback(true, family);
        return;
    }

    // Download ZIP
    QString urlStr = QString("https://fonts.google.com/download?family=%1")
        .arg(QString(QUrl::toPercentEncoding(fontName)));
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(urlStr)));

    // quick trace to ensure request was started
    qDebug("Font download requested: %s  reply=%p", qPrintable(urlStr), reply);

    // Use QPointer to avoid dereferencing deleted reply inside lambda
    QPointer<QNetworkReply> replyPtr(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QNetworkReply* r = replyPtr.data();
        if (!r) {
            qDebug("Download finished handler: reply pointer is null (already deleted)");
            callback(false, {});
            return;
        }

        // Check for network errors here (works on all Qt5 variations)
        if (r->error() != QNetworkReply::NoError) {
            qDebug("Error: %s", r->errorString().toStdString().c_str());
            r->deleteLater();
            callback(false, {});
            return;
        }

        QByteArray data = r->readAll();
        r->deleteLater();

        if (data.isEmpty()) {
            qDebug("Downloaded data empty");
            callback(false, {});
            return;
        }

        QFile f(zipPath);
        if (!f.open(QIODevice::WriteOnly)) {
            qDebug("Failed to open %s for write", qPrintable(zipPath));
            callback(false, {});
            return;
        }
        f.write(data);
        f.close();

        QStringList ttfs = _ExtractAllTtf(zipPath, targetFolder + "/unpacked");
        QString family;
        for (const QString &ttf: ttfs) {
            int id = QFontDatabase::addApplicationFont(ttf);
            if (family.isEmpty())
                family = QFontDatabase::applicationFontFamilies(id).value(0);
        }
        if (ttfs.isEmpty()) {
            callback(false, {});
            return;
        }

        callback(true, family);
    });
}

//QStringList FontUtils::ScanInstalledFontsInFolder(const QString& targetFolder)
//{
//    QStringList loadedFamilies;
//
//    QDirIterator it(targetFolder,
//        QStringList() << "*.ttf" << "*.otf",
//        QDir::Files,
//        QDirIterator::Subdirectories);
//
//    while (it.hasNext()) {
//        QString path = it.next();
//        int id = QFontDatabase::addApplicationFont(path);
//
//        if (id >= 0) {
//            QStringList fams = QFontDatabase::applicationFontFamilies(id);
//            for (const QString& f : fams)
//                loadedFamilies << f;
//        }
//    }
//
//    return loadedFamilies;
//}

QStringList FontUtils::ScanInstalledFontsInFolder(const QString & targetFolder)
{
    QStringList loadedFamilies;

    QDirIterator it(targetFolder,
        QStringList() << "*.ttf" << "*.otf" << "*.ttc",
        QDir::Files,
        QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);

        if (!fi.exists()) {
            qDebug("Font scan: file disappeared: %s", qPrintable(path));
            continue;
        }
        if (fi.size() == 0) {
            qDebug("Font scan: zero-size file skipped: %s", qPrintable(path));
            continue;
        }

        // Read file header to sanity-check format
        QFile fh(path);
        if (!fh.open(QIODevice::ReadOnly)) {
            qDebug("Font scan: cannot open file: %s", qPrintable(path));
            continue;
        }
        QByteArray head = fh.read(8);
        fh.close();

        bool looksLikeFont = false;
        // TrueType: 0x00 0x01 0x00 0x00, OpenType(CFF): "OTTO", TTC: "ttcf"
        if (head.startsWith("\0\1\0\0") || head.startsWith("OTTO") || head.startsWith("ttcf"))
            looksLikeFont = true;

        if (!looksLikeFont) {
            qDebug("Font scan: file does not look like TTF/OTF/TTC (sig=%s): %s",
                qPrintable(QString::fromLatin1(head.left(4).toHex())), qPrintable(path));
            // still try to load — sometimes signatures vary — but note it.
        }

        int id = QFontDatabase::addApplicationFont(path);
        if (id < 0) {
            qDebug("Font scan: QFontDatabase::addApplicationFont failed for %s", qPrintable(path));
            // Optionally copy the bad file aside for manual inspection:
            // QFile::copy(path, targetFolder + "/badfonts/" + QFileInfo(path).fileName());
            continue;
        }

        QStringList fams = QFontDatabase::applicationFontFamilies(id);
        if (fams.isEmpty()) {
            qDebug("Font scan: loaded id=%d but no families reported for %s", id, qPrintable(path));
            continue;
        }

        for (const QString& f : fams) {
            // sanitize family name for logging (remove control chars)
            QString name = f;
            name.remove(QRegExp("[\\x00-\\x1F]")); // strip control chars
            name = name.trimmed();
            qDebug("Font scan: loaded family '%s' from %s (id=%d)", qPrintable(name), qPrintable(path), id);
            loadedFamilies << name;
        }
    }

    return loadedFamilies;
}
QStringList FontUtils::QueryFontVariants(const QString& familyName) const
{
    QFontDatabase db;

    if (!db.families().contains(familyName))
        return {};  // font not present

    return db.styles(familyName);
}

void FontUtils::EnsureFontAvailable(const QString& fontName,
    const QString& targetFolder,
    std::function<void(bool, QStringList)> callback)
{
    QFontDatabase db;

    // Step 1: If already loaded in the app, return variants immediately
    if (db.families().contains(fontName)) {
        callback(true, db.styles(fontName));
        return;
    }

    // Step 2: Scan folder for previously downloaded fonts
    QStringList loaded = ScanInstalledFontsInFolder(targetFolder);
    if (loaded.contains(fontName)) {
        callback(true, QueryFontVariants(fontName));
        return;
    }

    // Step 3: Download ZIP, extract, load, return variants
    DownloadAndLoadFont(fontName, targetFolder,
        [=](bool ok, QString family) {
            if (!ok) {
                callback(false, {});
                return;
            }

            QStringList variants = QueryFontVariants(family);
            callback(true, variants);
        });
}
