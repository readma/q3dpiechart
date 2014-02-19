#include "q3dpiechart.h"


#include <math.h>
//#include <algorithm>


const double PI = 3.1415926535;
const double EPSELON = 0.00001;

q3dpiechart::q3dpiechart(QWidget* parent /* = NULL */)
    :QWidget(parent)
{
    m_startAngle = 0;
    m_totalValue = 0.0;

    m_isLegendVisible = true;
    m_isLabelVisible = true;
    m_isTransparentBg = false; 
    m_isTurning = false;
    m_isClockWiseTurning = true;

    m_gridRowCount = m_gridColCount = 1;

    m_textPen = QPen(QColor(0, 0, 0));
    m_borderPen = QPen(QColor::fromRgb(0, 0, 0, 64));

    m_pieColorAlpha = 152;
    m_pieColorAlphaDark = 200;

    m_chartBgDarkColor = QColor::fromRgb(213, 224, 241, 255);
    m_chartBgLightColor = QColor::fromRgb(255, 255, 255, 255);
    m_chartBorderColor = QColor::fromRgb(119, 141, 173);
    m_roundRadius = 10;

    m_chartMargin = 5;
    m_3DoffsetY = 20;

    m_pieMargin = 40;
    m_explodedRadius = 15;
    m_pieWidthHeightRatio = 2;

    initDefaultColors();

    startTimer(60);
}

q3dpiechart::~q3dpiechart()
{

}

void q3dpiechart::addPie(double v, const QString &desc)
{
    addPie(v, desc, getDefaultColor());
}

void q3dpiechart::addPie(double v, const QString &desc, const QColor &fillColor)
{
    Pie pie;
    pie.value = fabs(v);
    pie.description = desc;
    pie.brush = QBrush(fillColor);
    pie.isExploded = false;
    m_pies.append(pie);
    refreshChart();
}

void q3dpiechart::removePie(int pieIndex)
{
    if (pieIndex < m_pies.count())
    {
        m_pies.remove(pieIndex);
        refreshChart();
    }
}

void q3dpiechart::clear()
{
    if (m_pies.count() > 0)
    {
        m_pies.clear();
        refreshChart();
    }
}

void q3dpiechart::refreshChart()
{
    m_chartRect = rect().adjusted(m_chartMargin, m_chartMargin, -m_chartMargin, -m_chartMargin);

    m_totalValue = 0.0;

    for (QVector<Pie>::iterator itr = m_pies.begin();
        itr != m_pies.end(); itr++)
    {
        m_totalValue += itr->value;
    }
    if (m_isLegendVisible)
    {
        calculatePieLegendGrid();
    }
    calculatePieRect();
    calculatePieData();
    update();
}

void q3dpiechart::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int pieCount = m_pies.count();

    if (!m_isTransparentBg)
    {
        drawBackground(painter);
    }

    if (m_pieRect.width() <=0 || m_pieRect.height() <=0)
    {
        return;
    }

    if(pieCount <= 0 || m_totalValue < EPSELON)
    {
        drawEmptyPieChart(painter);
        return;
    }

    //画底面
    for(int i=0; i<pieCount; i++)
    {
        const Pie &pie = m_pies[i];
        if (pie.percentValue >= EPSELON)
        {
            QColor clr = pie.brush.color();
            clr.setAlpha(m_pieColorAlpha);
            QBrush brush(clr);
            draw3DPieSuface(painter, pie, pie.boundShadowRect, brush, false);
        }

    }

    //画弧形侧面，背面的
    for(int i=0; i<pieCount; i++)
    {
        const Pie &pie = m_pies[i];
        if (pie.percentValue >= EPSELON)
        {
            draw3DPieArcSide(painter, pie, false);
        }
    }

    //画矩形侧面
    for(int i=0; pieCount>1 && i<pieCount; i++)
    {
        const PieSide &ps = m_pieSides[i];
        if (ps.pie->percentValue < EPSELON)
        {
            continue;
        }

        QColor clr = ps.pie->brush.color();
        clr.setAlpha(m_pieColorAlpha);
        clr.setAlpha(224);
        //clr = Qt::black;
        QBrush brush(clr);
        draw3DPieRectSide(painter, ps, brush);
    }

    //画弧形侧面，前面的
    for(int i=0; i<pieCount; i++)
    {
        const Pie &pie = m_pies[i];
        if (pie.percentValue >= EPSELON)
        {
            draw3DPieArcSide(painter, pie);
        }
    }

    //画表面
    for(int i=0; i<pieCount; i++)
    {
        const Pie &pie = m_pies[i];
        if (pie.percentValue >= EPSELON)
        {
            QColor clr = pie.brush.color();
            clr.setAlpha(m_pieColorAlphaDark);
            QBrush brush(clr);
            draw3DPieSuface(painter, pie, pie.boundRect, brush);

            if (m_isLabelVisible)
            {
                draw3DPieLabel(painter, pie);
            }
        }
    }

    if (m_isLegendVisible)
    {
        drawLegend(painter);
    }
}


void q3dpiechart::drawBackground(QPainter &painter)
{
    drawBackground3DShadow(painter, m_chartRect);

    QRectF &rect = m_chartRect;
    QPainterPath path;
    path.addRoundRect(rect, m_roundRadius);

    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setStart(rect.center().x(), rect.top());
    gradient.setFinalStop(rect.center().x(), rect.bottom());
    gradient.setColorAt(0, m_chartBgDarkColor);
    gradient.setColorAt(1, m_chartBgLightColor);
    QBrush brush(gradient);
    painter.fillPath(path, brush);

    QPen pen(m_chartBorderColor);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawPath(path);
}

void q3dpiechart::drawBackground3DShadow(QPainter &painter, const QRectF &rect)
{
    float radius = rect.height() > rect.width() ? rect.width() / 20 : rect.height()/20;
    float offset = radius / 3;
    QColor clr(255, 255, 255, 0);
    QRectF rc = rect.translated(offset, offset);
    QRectF topRightArcRect = rc.adjusted(rc.width() - radius * 2, 0, 0, - rc.height() + radius * 2);

    QPainterPath topRightArcPath;
    topRightArcPath.moveTo(topRightArcRect.center());
    topRightArcPath.arcTo(topRightArcRect, 0, 90);
    QRadialGradient topRightRG(topRightArcRect.center(), radius);
    topRightRG.setColorAt(0, Qt::black);
    topRightRG.setColorAt(1, clr);
    QBrush topRightBrush(topRightRG);
    painter.fillPath(topRightArcPath, topRightBrush);

    QRectF rightRc = topRightArcRect.adjusted(radius, radius, 0, rect.height() - 3 * radius);
    QLinearGradient rightLG(rightRc.left(), 0, rightRc.right(), 0);
    rightLG.setColorAt(0, Qt::black);
    rightLG.setColorAt(1, clr);
    QBrush rightBrush(rightLG);
    QPainterPath rightPath;
    rightPath.addRect(rightRc);
    painter.fillPath(rightPath, rightBrush);

    QRectF leftBottomArcRect = rc.adjusted(0, rc.height() - radius * 2, -rc.width() + radius * 2, 0);
    QPainterPath leftBottomPath;
    leftBottomPath.moveTo(leftBottomArcRect.center());
    leftBottomPath.arcTo(leftBottomArcRect, -90, -180);
    QRadialGradient leftBottomRG(leftBottomArcRect.center(), radius);
    leftBottomRG.setColorAt(0, Qt::black);
    leftBottomRG.setColorAt(1, clr);
    QBrush leftBottomBrush(leftBottomRG);
    painter.fillPath(leftBottomPath, leftBottomBrush);

    QRectF bottomRc = leftBottomArcRect.adjusted(radius, radius, rect.width() - 3 * radius, 0);
    QLinearGradient bottomLG(0, bottomRc.top(), 0, bottomRc.bottom());
    bottomLG.setColorAt(0, Qt::black);
    bottomLG.setColorAt(1, clr);
    QBrush bottomBrush(bottomLG);
    QPainterPath bottomPath;
    bottomPath.addRect(bottomRc);
    painter.fillPath(bottomPath, bottomBrush);

    QRectF bottomRightArcRect = leftBottomArcRect.translated(rect.width() - radius * 2, 0);
    QPainterPath bottomRightPath;
    bottomRightPath.moveTo(bottomRightArcRect.center());
    bottomRightPath.arcTo(bottomRightArcRect, 0, -90);
    QRadialGradient bottomRightRG(bottomRightArcRect.center(), radius);
    bottomRightRG.setColorAt(0, Qt::black);
    bottomRightRG.setColorAt(1, clr);
    QBrush bottomRightBrush(bottomRightRG);
    painter.fillPath(bottomRightPath, bottomRightBrush);
}

void q3dpiechart::draw3DPieSuface(QPainter &painter, const Pie &pie, const QRectF &rcBound, const QBrush &brush, bool isTop/* = true*/)
{
    QPainterPath path;
    QPoint ptOffset;
    if (!isTop)
    {
        ptOffset.setY(m_3DoffsetY);
    }
    path.moveTo(rcBound.center());
    path.lineTo(pie.startPoint + ptOffset);
    path.arcTo(rcBound, pie.startAngle, pie.spanAngle);
    path.closeSubpath();

    painter.fillPath(path, brush);
    painter.setPen(m_borderPen);
    painter.drawArc(rcBound, pie.startAngle * 16, pie.spanAngle * 16);
}

void q3dpiechart::draw3DPieLabel(QPainter &painter, const Pie &pie)
{
    painter.setFont(this->font());
    painter.setPen(m_textPen);
    if (1 == m_pies.count())
    {
        painter.drawText(m_pieRect, Qt::AlignCenter, pie.percentStr);
        return;
    }

    QString label = getPieLabel(pie);
    QRectF rc;
    rc = QRectF(pie.centerPoint, pie.centerPoint);
    QFontMetricsF metrics(this->font());
    double quarterW = metrics.width(label) / 2;//m_pieRect.width() / 4;
    double halfH = metrics.height() / 2;//m_pieRect.height() / 2;
    rc.adjust(-quarterW, -halfH, quarterW, halfH);


    QPainterPath path;
    path.moveTo(pie.boundRect.center());
    path.lineTo(pie.startPoint);
    path.arcTo(pie.boundRect, pie.startAngle, pie.spanAngle);
    path.closeSubpath();

    if (path.contains(rc))
    {
        painter.drawText(rc, Qt::AlignCenter, label);
    }
    else
    {
        draw3DPieLabelOut(painter, pie, rc);
    }
}

void q3dpiechart::draw3DPieLabelOut(QPainter &painter, const Pie &pie, QRectF labelSizeRect)
{
    double centerAngle = pie.startAngle + pie.spanAngle / 2;
    if (centerAngle > 360)
    {
        centerAngle -= 360;
    }

    QPainterPath path;
    path.arcMoveTo(pie.boundRect, centerAngle);

    if (centerAngle >= 0 && centerAngle <90)
    {
        labelSizeRect.moveBottomLeft(path.currentPosition());
        painter.drawLine(pie.centerPoint, path.currentPosition());
    }
    else if (centerAngle >= 90 && centerAngle <180)
    {
        labelSizeRect.moveBottomRight(path.currentPosition());
        painter.drawLine(pie.centerPoint, path.currentPosition());
    }
    else if (centerAngle >= 180 && centerAngle <270)
    {
        labelSizeRect.moveTopRight(path.currentPosition());
        painter.drawLine(pie.centerPoint, labelSizeRect.bottomRight());
    }
    else if (centerAngle >= 270 && centerAngle <=360)
    {
        labelSizeRect.moveTopLeft(path.currentPosition());
        painter.drawLine(pie.centerPoint, labelSizeRect.bottomLeft());
    }
    painter.drawLine(labelSizeRect.bottomLeft(), labelSizeRect.bottomRight());
    painter.drawText(labelSizeRect, Qt::AlignLeft, getPieLabel(pie));
}

void q3dpiechart::drawLegend(QPainter &painter)
{

}

void q3dpiechart::drawLegendCell(QPainter &painter, const Pie &pie, const QRectF &rcBound)
{

}

void q3dpiechart::draw3DPieRectSide(QPainter &painter, const Pie &pie, QBrush &brush)
{
    QPainterPath path;
    QPoint offsetPt(0, m_3DoffsetY);
    path.moveTo(pie.boundRect.center());
    path.lineTo(pie.boundShadowRect.center());
    path.lineTo(pie.startPoint + offsetPt);
    path.lineTo(pie.startPoint);
    path.lineTo(pie.boundRect.center());
    painter.fillPath(path, brush);

    path = QPainterPath();
    path.moveTo(pie.boundRect.center());
    path.lineTo(pie.boundShadowRect.center());
    path.lineTo(pie.endPoint + offsetPt);
    path.lineTo(pie.endPoint);
    path.lineTo(pie.boundRect.center());

    painter.fillPath(path, brush);

    if (pie.spanAngle != 180)
    {
        painter.setPen(m_borderPen);
        painter.drawLine(pie.boundRect.center(), pie.boundShadowRect.center());
    }
}

void q3dpiechart::draw3DPieRectSide(QPainter &painter, const PieSide &pieSide, QBrush &brush)
{
    QPainterPath path;
    QPoint offsetPt(0, m_3DoffsetY);
    Pie &pie = *pieSide.pie;
    if (pie.startAngle == pieSide.angle)
    {
        path.moveTo(pie.boundRect.center());
        path.lineTo(pie.boundShadowRect.center());
        path.lineTo(pie.startPoint + offsetPt);
        path.lineTo(pie.startPoint);
        path.lineTo(pie.boundRect.center());
    }
    else
    {
        path.moveTo(pie.boundRect.center());
        path.lineTo(pie.boundShadowRect.center());
        path.lineTo(pie.endPoint + offsetPt);
        path.lineTo(pie.endPoint);
        path.lineTo(pie.boundRect.center());
    }
    painter.fillPath(path, brush);

    if (pie.spanAngle != 180)
    {
        painter.setPen(m_borderPen);
        painter.drawLine(pie.boundRect.center(), pie.boundShadowRect.center());
    }
}

void q3dpiechart::draw3DPieArcSide(QPainter &painter, const Pie &pie, bool front/*=true*/)
{
    QColor clrDark = pie.brush.color();
    double darkRate = 0.7;
    clrDark = QColor::fromRgb(clrDark.red() * darkRate, clrDark.green() * darkRate, clrDark.blue() * darkRate);
    clrDark.setAlpha(224);
    QColor clrLight = pie.brush.color();
    clrLight.setAlpha(0);
    clrLight = QColor::fromRgb(255, 255, 255, 128);

    QLinearGradient lg(pie.boundRect.left(), 0, pie.boundRect.right(), 0);
    lg.setColorAt(0, clrDark);
    lg.setColorAt(0.5, clrLight);
    lg.setColorAt(1, clrDark);
    QBrush frontBrush(lg);//正面的刷子为发光状
    QBrush backBrush(pie.brush.color());//背面的刷子不发光

    QPointF offsetPt(0, m_3DoffsetY);
    QPainterPath path;

    //存放拆分后的的路径和画刷对
    std::vector< std::pair<QPainterPath, QBrush> > arcSides;

    double endAngle = pie.startAngle + pie.spanAngle;

    //出现与水平面相交时，进行拆分
    if ((pie.startAngle<180 && endAngle > 180)
        ||(pie.startAngle>=180 && endAngle > 360))
    {
        double startAngle = pie.startAngle;//当前的起始角度
        if (pie.startAngle <180)//一二象限
        {
            path = make3DPieArcSidePath(startAngle, 180, pie);
            arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, backBrush));
            startAngle = 180;

            QBrush tempBrush = frontBrush;
            if (endAngle > 360)
            {
                path = make3DPieArcSidePath(startAngle, 360, pie);
                arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, frontBrush));
                startAngle = 360;
                tempBrush = backBrush;
            }

            path = make3DPieArcSidePath(startAngle, endAngle, pie);
            arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, tempBrush));
        }
        else//三四象限
        {
            path = make3DPieArcSidePath(startAngle, 360, pie);
            arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, frontBrush));
            startAngle = 360;

            QBrush tempBrush = backBrush;
            if (endAngle > 540)
            {
                path = make3DPieArcSidePath(startAngle, 540, pie);
                arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, backBrush));
                startAngle = 540;
                tempBrush = frontBrush;
            }

            path = make3DPieArcSidePath(startAngle, endAngle, pie);
            arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, tempBrush));
        }
    }
    else//不相交时
    {
        path = make3DPieArcSidePath(pie.startAngle, pie.startAngle + pie.spanAngle, pie);
        arcSides.push_back(std::make_pair<QPainterPath, QBrush>(path, pie.startAngle >= 180 ? frontBrush : backBrush));
    }
    for (int i=0; i<arcSides.size(); i++)
    {
        std::pair<QPainterPath, QBrush> &arcSide = arcSides.at(i);
        if ((front && arcSide.second == frontBrush)
            || (!front && arcSide.second == backBrush))
        {
            painter.fillPath(arcSide.first, arcSide.second);
        }
    }
}

QPainterPath q3dpiechart::make3DPieArcSidePath(double startAngle, double endAngle, const Pie &pie)
{
    QPainterPath path;
    path.arcMoveTo(pie.boundRect, startAngle);
    path.arcTo(pie.boundRect, startAngle, endAngle - startAngle);

    QPointF pt = path.currentPosition();
    pt.setY(pt.y() + m_3DoffsetY);
    path.lineTo(pt);
    path.arcTo(pie.boundShadowRect, endAngle, startAngle - endAngle);
    path.closeSubpath();
    return path;
}


void q3dpiechart::drawEmptyPieChart(QPainter &painter)
{
    QRectF rcBound = m_pieRect;
    rcBound.setHeight(rcBound.height() - m_3DoffsetY);

    QPointF pt1(rcBound.left(), rcBound.center().y());

    QPen borderPen(Qt::darkGray);
    painter.setPen(borderPen);
    painter.drawEllipse(rcBound);

    rcBound.translate(0, m_3DoffsetY);
    QPointF pt2(rcBound.left(), rcBound.center().y());

    painter.drawEllipse(rcBound);

    painter.drawLine(pt1, pt2);
    pt1.setX(pt1.x() + rcBound.width());
    pt2.setX(pt2.x() + rcBound.width());
    painter.drawLine(pt1, pt2);
}

void q3dpiechart::showGridToolTip(const QPoint &pt)
{

}

void q3dpiechart::calculatePieRect()
{
    
    const legendWidth = m_gridRect.width();
    if (0 == m_pies.count() || !m_isLegendVisible)//没有添加内容或不显示描述信息，以整个区域为饼图区域
    {
        m_pieRect = m_chartRect.adjusted(m_pieMargin, m_pieMargin, -m_pieMargin, -m_pieMargin);
    }
    else
    {
        m_pieRect = m_chartRect.adjusted(m_pieMargin, m_pieMargin, -m_pieMargin, -m_pieMargin - legendWidth);
    }

    double w = m_pieRect.width();
    double h = m_pieRect.height();

    if (h <= 0 || w <=0)
    {
        m_pieRect.setRect(0, 0, 0, 0);
        return;
    }

    if (w / h > m_pieWidthHeightRatio)
    {
        double half = (w - h * m_pieWidthHeightRatio) / 2;
        m_pieRect.adjust(half, 0, -half, 0);
    }
    else
    {
        double half = (h - w / m_pieWidthHeightRatio) / 2;
        m_pieRect.adjust(0, half, 0, -half);
    }

    m_3DoffsetY = m_pieRect.height() / 5;
    double maxOffsetY = m_pieMargin * 2;
    if (m_3DoffsetY > maxOffsetY)
    {
        m_3DoffsetY = maxOffsetY;
    }

    m_explodedRadius = m_pieRect.height() / 6;
    double maxExplodedRadius = m_pieMargin * 1.5;
    if (m_explodedRadius > maxExplodedRadius)
    {
        m_explodedRadius = maxExplodedRadius;
    }

    m_pieShadowRect = m_pieRect.translated(0, m_3DoffsetY);
}

void q3dpiechart::calculatePieData()
{
    double startAngle = m_startAngle;
    double quarterW = m_pieRect.width() / 6;
    double quarterH = m_pieRect.height() / 6;
    QRectF halfPieRect = m_pieRect.adjusted(quarterW, quarterH, -quarterW, -quarterH);
    QRectF explodedRect(m_pieRect.center(), m_pieRect.center());
    explodedRect.adjust(-m_explodedRadius, -m_explodedRadius, m_explodedRadius, m_explodedRadius);
    for (int i=0; i<m_pies.count(); i++)
    {
        Pie &pie = m_pies[i];
        pie.startAngle = startAngle;

        QPainterPath path;
        path.arcMoveTo(m_pieRect, startAngle);
        pie.startPoint = path.currentPosition();//起始点

        if (m_totalValue < EPSELON)
        {
            pie.percentValue = 0;//1.0 / m_pies.count();
        }
        else
        {
            pie.percentValue = pie.value / m_totalValue;//所占比例
        }
        pie.percentStr = QString("%1%").arg(pie.percentValue*100, 3, 'G', 3);

        pie.spanAngle = 360 * pie.percentValue;//跨度
        if (i == m_pies.count()-1)//最后一个饼，用360度减去前面的总度数，以防止由于误差导致的合不上
        {
            pie.spanAngle = 360 - (startAngle - m_startAngle);
        }

        path.arcTo(m_pieRect, startAngle, pie.spanAngle);
        pie.endPoint = path.currentPosition();

        path.arcMoveTo(halfPieRect, startAngle + pie.spanAngle / 2);
        pie.centerPoint = path.currentPosition();

        if (pie.isExploded)
        {
            path.arcMoveTo(explodedRect, startAngle + pie.spanAngle / 2);
            QPointF pt = path.currentPosition() - m_pieRect.center();
            pie.centerPoint += pt;
            pie.startPoint += pt;
            pie.endPoint += pt;
            pie.boundRect = m_pieRect.translated(pt);
        }
        else
        {
            pie.boundRect = m_pieRect;
        }
        pie.boundShadowRect = pie.boundRect.translated(0, m_3DoffsetY);

        startAngle += pie.spanAngle;
        while (pie.startAngle > 360)
        {
            pie.startAngle -= 360;
        }
    }

    sortPieRectSide();
}

void q3dpiechart::calculatePieLegendGrid()
{
    QFontMetricsF metrics(this->font());
    m_gridCellHeight = metrics.height() * 2;
    m_gridCellWidth = getMaxLegendWidth();//metrics.width("123456789012345");
    m_gridCellWidth += m_gridCellHeight + m_gridCellHeight / 4;//描述前留下一个高度的方块，用于显示颜色，后面留下1/4高度的宽度，以防止抵到表格右边
    if (m_gridCellWidth > m_chartRect.width() / 2)
    {
        m_gridCellWidth = m_chartRect.width() / 2;
    }

    m_gridColCount = m_pies.count();
    m_gridRowCount = 1;

    m_gridRect = m_chartRect.adjusted(m_pieMargin, m_pieMargin, -m_pieMargin, -m_pieMargin);
    m_gridRect.setLeft(m_gridRect.width() - m_gridCellWidth);
}

//排序规则：0-180度的越靠近90度越在后面（显示顺序），180-360的越靠近270度越在前面（显示顺序）
bool pieSideLesTthan(const PieSide &s1, const PieSide &s2)
{
    if (s1.angle <180)
    {
        if (s2.angle >=180)
        {
            return true;
        }
        else
        {
            return fabs(s1.angle - 90) < fabs(s2.angle - 90);
        }
    }
    else
    {
        if (s2.angle >=180)
        {
            return fabs(s1.angle - 270) > fabs(s2.angle - 270);
        }
        else
        {
            return false;
        }
    }
}

void q3dpiechart::sortPieRectSide()
{
    m_pieSides.clear();
    for (int i=0; i<m_pies.count(); i++)
    {
        Pie &pie = m_pies[i];
        double startAngle = pie.startAngle;
        double endAngle = startAngle + pie.spanAngle;
        if (endAngle>=360)
        {
            endAngle-=360;
        }

        //每个Pie有两边
        PieSide startSide = {&pie, startAngle};
        PieSide endSide = {&pie, endAngle};
        m_pieSides.push_back(startSide);
        m_pieSides.push_back(endSide);
    }

    qSort(m_pieSides.begin(), m_pieSides.end(), pieSideLesTthan);
}

Pie *q3dpiechart::hitTest(QPoint pt)
{
    //优先判断是否点中上表面和三四象限的弧侧面
    for (int i=0; i<m_pies.count(); i++)
    {
        Pie &pie = m_pies[i];

        QPainterPath path;
        path.moveTo(pie.boundRect.center());
        path.arcMoveTo(pie.boundRect, pie.startAngle);
        path.lineTo(path.currentPosition());
        path.arcTo(pie.boundRect, pie.startAngle, pie.spanAngle);
        path.lineTo(pie.boundRect.center());

        //点在了正面
        if (path.contains(pt))
        {
            return &pie;
        }

        //如果是点在180-360的弧形侧面，也算点中了，分几种情况进行判断
        double endAngle = pie.startAngle + pie.spanAngle;
        if (pie.startAngle < 180 && endAngle >180)
        {
            if (make3DPieArcSidePath(180, qMin(360.0, endAngle), pie).contains(pt))
            {
                return &pie;
            }
        }
        else if (pie.startAngle >180)
        {
            if (make3DPieArcSidePath(pie.startAngle, qMin(360.0, endAngle), pie).contains(pt))
            {
                return &pie;
            }

            if (endAngle > 540)
            {
                if (make3DPieArcSidePath(540, endAngle, pie).contains(pt))
                {
                    return &pie;
                }
            }
        }
    }

    //再判断Pie的矩形侧面
    for (int i=m_pieSides.count()- 1; i>-1; i--)
    {
        PieSide &pieSide = m_pieSides[i];
        Pie &pie = *pieSide.pie;
        QPainterPath path;
        QPoint offsetPt(0, m_3DoffsetY);

        if (pie.startAngle == pieSide.angle)
        {
            path.moveTo(pie.boundRect.center());
            path.lineTo(pie.boundShadowRect.center());
            path.lineTo(pie.startPoint + offsetPt);
            path.lineTo(pie.startPoint);
            path.lineTo(pie.boundRect.center());
            if (path.contains(pt))
            {
                return &pie;
            }
        }
        else
        {
            path.moveTo(pie.boundRect.center());
            path.lineTo(pie.boundShadowRect.center());
            path.lineTo(pie.endPoint + offsetPt);
            path.lineTo(pie.endPoint);
            path.lineTo(pie.boundRect.center());

            if (path.contains(pt))
            {
                return &pie;
            }
        }
    }

    return NULL;
}

double q3dpiechart::getMaxLegendWidth()
{
    return 0;
}

void q3dpiechart::mousePressEvent(QMouseEvent *event)
{
    m_mouseDownPoint = event->globalPos();
}

void q3dpiechart::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton 
        && m_pies.count() >1 && m_mouseDownPoint == event->globalPos())
    {
        Pie *pie = hitTest(event->pos());

        if (pie != NULL)
        {
            pie->isExploded = !pie->isExploded;
            refreshChart();
        }
    }
}

void q3dpiechart::mouseMoveEvent(QMouseEvent *event)
{
    //Pie *pie = hitTest(event->pos());

    //if (pie != NULL)
    //{
    //    setToolTip(getPieToolTip(*pie));
    //}
}

QString q3dpiechart::getPieToolTip(const Pie &pie)
{
    return QString("<font color=red>%1</font>:<font color=green>%2</font><font color=blue>(%3)</font>")
        .arg(pie.description)
        .arg(pie.value/*, 0, 'f', 2*/)
        .arg(pie.percentStr);
}

QString q3dpiechart::getPieLabel(const Pie &pie)
{
    return QString("%1:%2").arg(pie.description).arg(pie.percentStr);
}


void q3dpiechart::resizeEvent(QResizeEvent *event)
{
    refreshChart();
}

void q3dpiechart::timerEvent(QTimerEvent * event)
{
    if (m_isTurning)
    {
        m_startAngle += m_isClockWiseTurning ? 1 : -1;

        if (m_startAngle < 0)
        {
            m_startAngle += 360;
        }
        else if (m_startAngle > 360)
        {
            m_startAngle -= 360;
        }

        calculatePieData();
        update();
    }
}

void q3dpiechart::initDefaultColors()
{
    m_defaultColors.clear();
    m_defaultColors.append(QColor(192, 0, 0, 255));
    m_defaultColors.append(QColor(0, 192, 0, 255));
    m_defaultColors.append(QColor(192, 192, 0, 255));
    m_defaultColors.append(QColor(192, 0, 192, 255));
    m_defaultColors.append(QColor(0, 192, 192, 255));
    int r = 0;
    int g = 0;
    int b = 192;
    for (int i=0; i<20; i++)
    {
        m_defaultColors.append(QColor(r, g, b, 255));

        r += 48;
        g += 24;
        b += 24;
        if (r > 255)
        {
            r -=255;
        }
        if (g > 255)
        {
            g -= 255;
        }
        if (b > 255)
        {
            b -= 255;
        }
    }
}

QColor q3dpiechart::getDefaultColor()
{
    if (m_defaultColors.count() == 0)
    {
        return Qt::blue;
    }
    int index = m_pies.count();
    if (index >= m_defaultColors.count())
    {
        index = 0;
    }
    return m_defaultColors[index];
}
