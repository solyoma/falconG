#include "stylehandler.h"
#include "config.h"
#include "albums.h"
#include "falcong.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	// test style sheets
//	w.setStyleSheet("* {background-color:#222; color:#ccc;}"); // working
	QString homePath = QDir::homePath() +
#if defined (Q_OS_Linux)   || defined (Q_OS_Darwin) || defined (__linux__)
		"/.falconG";
#elif defined(Q_OS_WIN)
		"/Appdata/Local/falconG";
#endif

	// after a new translation is added add language radio boxes
	// to falconG.ini and their handlers to code
	

	QStringList fileNames = GetTranslations();	// sorted list of language string like "hu_HU"

	QTranslator translator;
	QSettings* s = new QSettings(homePath +	"/falconG.ini",	QSettings::IniFormat);
	int ixLang = s->value("lang", -1).toInt();
	delete s;
	QString qs, qsn;
	qs = QLocale::system().name();	// e.g. hu_HU
	if (ixLang < 0)
		ixLang = fileNames.indexOf("falconG_" + qs + ".qm");

	if (ixLang >= 0)
	{
		qsn = QStringLiteral(":/translations/translations/") + fileNames[ixLang];
		bool loaded = translator.load(QLocale(qsn), qsn);
		if (loaded)
			qs = translator.language();

		if (loaded && qs != "en_US")	 // only set when not American English
			if (!a.installTranslator(&translator))
				QMessageBox::warning(nullptr, "falconG - Warning", "Can't load language " + qs + "using default");
	}
	FalconG w;

	w.show();
	return a.exec();
}
