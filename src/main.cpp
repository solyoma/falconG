#include <QtWidgets>
#include <QtWidgets/QApplication>

#include <QMutex>
#include <QThread>
#include <QSplashScreen>

// debug:
#include <QSslSocket>
#include <QDebug>

#include "stylehandler.h"
#include "config.h"
#include "albums.h"
#include "falcong.h"
int main(int argc, char *argv[])
{
	// This makes all widgets scale with dpi scaling, but the whole
	// window may not be visible. If not used, however then some items 
	// may be missing during design and texts may overflow the controls
	// and get clipped
	// 
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication a(argc, argv);
	a.setStyle("fusion");
// DEBUG
qDebug("QSsl supported: %d", QSslSocket::supportsSsl());
qDebug("SSL build: %s", qPrintable(QSslSocket::sslLibraryBuildVersionString()));
qDebug("SSL runtime: %s", qPrintable(QSslSocket::sslLibraryVersionString()));
// /DEBUG

	a.setWindowIcon(QIcon(":/icons/Resources/falconG-icon.png"));
	PROGRAM_CONFIG::GetHomePath();

	ShowSplashScreen();		// no text
	// after a new translation is added add language radio boxes
	// to falconG.ini and their handlers to code

	PROGRAM_CONFIG::GetTranslations();	// sorted list of language string like "hu_HU"
	PROGRAM_CONFIG::Read();

	QTranslator translator;
	QSettings* s = new QSettings(PROGRAM_CONFIG::homePath +	"/falconG.ini",	QSettings::IniFormat);
	int ixLang = PROGRAM_CONFIG::lang; //  s->value("lang", -1).toInt();
	delete s;
	QString qs, qsn;
	qs = QLocale::system().name();	// e.g. hu_HU
	if (ixLang < 0)
		ixLang = PROGRAM_CONFIG::qslLangNames.indexOf(qs + ".qm");

	if (ixLang >= 0)
	{
		qsn = QStringLiteral(":/translations/translations/") + PROGRAM_CONFIG::qslLangNames[ixLang];
		bool loaded = translator.load(QLocale(qsn), qsn);
		if (loaded)
			qs = translator.language();

		if (loaded && qs != "en_US")	 // only set when not American English
			if (!a.installTranslator(&translator))
				QMessageBox::warning(nullptr, "falconG - Warning", "Can't load language " + qs + "using default");
	}
	ShowSplashScreen(true);	 // add text

	FalconG w;
	w.show();

	CloseSplashScreen();
	w.raise();
	w.activateWindow();
	return a.exec();
}

