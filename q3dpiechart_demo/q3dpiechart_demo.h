#ifndef Q3DPIECHART_DEMO_H
#define Q3DPIECHART_DEMO_H

#include <QtGui/QDialog>
#include "ui_q3dpiechart_demo.h"

class q3dpiechart_demo : public QDialog
{
	Q_OBJECT

public:
	q3dpiechart_demo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~q3dpiechart_demo();

private:
	Ui::q3dpiechart_demoClass ui;
};

#endif // Q3DPIECHART_DEMO_H
