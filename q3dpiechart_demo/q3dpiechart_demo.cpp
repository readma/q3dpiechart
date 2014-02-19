#include "q3dpiechart_demo.h"

#include "../q3dpiechart/q3dpiechart.h"

#include <QtGui/QBoxLayout>

q3dpiechart_demo::q3dpiechart_demo(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);

    q3dpiechart *piechart = new q3dpiechart(this);
    piechart->addPie(10, "aaa");
    piechart->addPie(13, "bbb");
    piechart->addPie(11, "ccc");


    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->addWidget(piechart);
    setLayout(layout);
    
}

q3dpiechart_demo::~q3dpiechart_demo()
{

}
