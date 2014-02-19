#ifndef Q3DPIE_H
#define Q3DPIE_H

#include <QtGui/QPen>
#include <QtGui/QBrush>


class  Pie
{
public:

    double value;           
    QBrush brush;           
    QString description;    

    double percentValue;
    QString percentStr;
    double startAngle;
    double spanAngle;
    QPointF startPoint;
    QPointF endPoint;
    QPointF centerPoint;

    bool isExploded;

    QRectF boundRect;
    QRectF boundShadowRect;

private:

};

class PieSide
{
public:
    Pie *pie;
    double angle;
};

#endif // Q3DPIE_H
