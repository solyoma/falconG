#include "stylehandler.h"
#include "config.h"
#include "albums.h"
#include "falcong.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FalconG w;
	w.show();
	return a.exec();
}
