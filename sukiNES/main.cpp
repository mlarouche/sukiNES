// Qt includes
#include <QtWidgets/QApplication>

// Local incldues
#include "sukinesmainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	sukiNESMainWindow w;
	w.show();

	return a.exec();
}
