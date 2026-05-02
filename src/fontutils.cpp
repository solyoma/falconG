#include "fontutils.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QPointer>
#include <QNetworkReply>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRawFont>
#include <QDebug>

#include "logger.h"

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

    // Clear previous aliases for a fresh scan
    _slFontErrors.clear();
    QString qsPrevPath = "///";
    int plen = 5;

    while (it.hasNext()) {
        QString path = it.next();
        if (path.left(plen) != qsPrevPath)
        {
            plen = path.lastIndexOf('/') + 1;
            qsPrevPath = path.left(plen);
            logger.Log(tr("Scanning folder %1 for fonts").arg(qsPrevPath));
        }

        QString msg;
        QFileInfo fi(path);

        if (!fi.exists()) 
        {
            msg = tr("Font scan: file disappeared: %1").arg(path.mid(plen));
            _slFontErrors << msg;
			logger.Log(msg);
            continue;
        }
        if (fi.size() == 0) {
            msg = tr("Font scan: zero-size file skipped: %1").arg(path.mid(plen));
            _slFontErrors << msg;
			logger.Log(msg);
            continue;
        }

        // Quick signature check
        QFile fh(path);
        if (!fh.open(QIODevice::ReadOnly)) 
        {
            msg = tr("Font scan: cannot open file: %1").arg(path.mid(plen));
            _slFontErrors << msg;
			logger.Log(msg);
            continue;
        }
        QByteArray head = fh.read(8);
        fh.close();

        if (!(head.startsWith("\0\1\0\0") || head.startsWith("OTTO") || head.startsWith("ttcf"))) 
        {
            msg = tr("Font scan: file does not look like TTF/OTF/TTC (sig=%1):%2")
                        .arg( QString::fromLatin1(head.left(4).toHex())).arg(path.mid(plen));
            _slFontErrors << msg;
			logger.Log(msg );
            continue;
        }

        logger.Log(tr("Adding font %1").arg(path.mid(plen)));
        int id = QFontDatabase::addApplicationFont(path);
        if (id < 0)
        {
            msg = tr("Font scan: QFontDatabase::addApplicationFont failed for %1").arg(path.mid(plen));
            _slFontErrors << msg;
            logger.Log(msg);
            continue;
        }
        else if(_ids.indexOf(id) < 0)
            _ids << id;

        logger.Log(tr("Checking families"));
        QStringList fams = QFontDatabase::applicationFontFamilies(id);
        if (fams.isEmpty()) 
        {
            msg = tr("Font scan: loaded id=%1 but no families reported for %2").arg(id).arg(qPrintable(path.mid(plen)));
            _slFontErrors << msg;
			logger.Log(msg );
            continue;
        }

        // Log and register each family
        logger.Log(tr("Log and register %1 %2").arg(fams.size() > 1 ? tr("each") : "").arg(fams.size() > 1 ? tr("family"):tr("family")));
        for (const QString& f : fams) 
        {
            QString name = f;
            // Remove control chars and surrounding/trailing quotes
            name.remove(QRegularExpression("[\\x00-\\x1F]"));
            name = name.trimmed();

             msg = tr("Font scan: loaded family '%1' from '%3' (id=%4)")
                   .arg(qPrintable(name)).arg(qPrintable(path.mid(plen))).arg(id);

			logger.Log(msg );

            loadedFamilies << name;
        }
    }

    return loadedFamilies;
}

QStringList FontUtils::QueryFontVariants(const QString& familyName) const
{
    QFontDatabase db;

    QStringList families = db.families();
    // Try exact match first
    if (families.contains(familyName))
        return db.styles(familyName);

    // final fallback: try case-insensitive search over families
    for (const QString& fam : db.families()) 
    {
        if (QString::compare(fam, familyName, Qt::CaseInsensitive) == 0)
            return db.styles(fam);
    }

    return {};  // font not present
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
    if (loaded.contains(fontName)) 
    {
        callback(true, QueryFontVariants(fontName));
        return;
    }
}
