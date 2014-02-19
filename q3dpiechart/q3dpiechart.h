#ifndef Q3DPIECHART_H
#define Q3DPIECHART_H


#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QToolTip>
#include <QtGui/QMouseEvent>

#include "q3dpiechart_global.h"
#include "pie.h"


class Q3DPIECHART_EXPORT q3dpiechart : public QWidget
{
public:
	q3dpiechart(QWidget* parent = NULL);
	~q3dpiechart();

public:
    void addPie(double v, const QString &desc);
    void addPie(double v, const QString &desc, const QColor &fillColor);
    void removePie(int pieIndex);
    void clear();


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent * event);


private:
    void refreshChart();

    void drawBackground(QPainter &painter);
    void drawBackground3DShadow(QPainter &painter, const QRectF &rect);
    void draw3DPieSuface(QPainter &painter, const Pie &pie, const QRectF &rcBound, const QBrush &brush, bool isTop = true);
    void draw3DPieLabel(QPainter &painter, const Pie &pie);
    void draw3DPieLabelOut(QPainter &painter, const Pie &pie, QRectF labelSizeRect);
    void drawLegend(QPainter &painter);
    void drawLegendCell(QPainter &painter, const Pie &pie, const QRectF &rcBound);
    void draw3DPieRectSide(QPainter &painter, const Pie &pie, QBrush &brush);
    void draw3DPieRectSide(QPainter &painter, const PieSide &pieSide, QBrush &brush);
    void draw3DPieArcSide(QPainter &painter, const Pie &pie, bool front=true);
    void drawEmptyPieChart(QPainter &painter);
    void showGridToolTip(const QPoint &pt);

    void calculatePieRect();//计算饼矩形范围
    void calculatePieData();//计算饼位置、百分比等
    void calculatePieLegendGrid();//计算图例表格宽度高度等
    void sortPieRectSide();//计算侧面的先后顺序
    QPainterPath make3DPieArcSidePath(double startAngle, double endAngle, const Pie &pie);//生成弧形侧面路径
    Pie *hitTest(QPoint pt);//判断是否点中
    double getMaxLegendWidth();//取得描述文字最宽的宽度
    void initDefaultColors();
    QColor getDefaultColor();
    QString getPieToolTip(const Pie &pie);
    QString getPieLabel(const Pie &pie);

private:
    QVector<Pie> m_pies;
    QVector<PieSide> m_pieSides;

    QString m_unit;
    int m_startAngle;
    double m_totalValue;

    bool m_isLegendVisible;
    bool m_isLabelVisible;
    bool m_isTransparentBg;
    bool m_isTurning;
    bool m_isClockWiseTurning;

    QPen m_textPen;
    QPen m_borderPen;

    QRectF m_chartRect;

    double m_3DoffsetY;
    double m_explodedRadius;

    double m_chartMargin; 
    double m_pieMargin;

    int m_pieColorAlpha; 
    int m_pieColorAlphaDark; 

    QRectF m_pieRect;
    QRectF m_pieShadowRect;

    QColor m_chartBgDarkColor;
    QColor m_chartBgLightColor;
    QColor m_chartBorderColor;
    double m_roundRadius;
    double m_pieWidthHeightRatio;

    int m_gridRowCount;
    int m_gridColCount;
    double m_gridCellWidth;
    double m_gridCellHeight;
    QRectF m_gridRect;

    QPoint m_mouseDownPoint;


    QVector<QColor> m_defaultColors;
};

#endif // Q3DPIECHART_H
