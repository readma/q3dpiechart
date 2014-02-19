#include <QtGui/QApplication>
#include "q3dpiechart_demo.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	q3dpiechart_demo w;
	w.show();
	return a.exec();
}
