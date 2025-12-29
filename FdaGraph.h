#pragma once

#include "qbarcategoryaxis.h"
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QAbstractItemModel>
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QChartView>
#include <QWheelEvent>
#include <QDateTime>
#include <QtMath>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>

class PotaGraph : public QChart
{
    Q_OBJECT
public:
    explicit PotaGraph(QGraphicsItem* parent=nullptr);

    int m_xAxisFieldNum;
    QString m_xAxisDataType;
    int m_xAxisGroup;
    bool m_xAxisYearSeries;

    QList<int> m_yAxisFieldNum;
    //QList<QString> m_yAxisDataType;
    QList<int> m_yAxisCalc;
    QList<int> m_yAxisType;
    QList<bool> m_yRightAxis;
    QList<QColor> m_yColor;

    void createSeries(QAbstractItemModel *model);
    void fillSeries(QAbstractItemModel* model);

private:
    QList<QAbstractSeries*> m_series;
    QList<QString> seriesName;
    QList<int> years;
    QList<QVariant> abscissaVar;
    QList<QList<QVariant>> ordinateVar;
    bool RightAxisCreated=false;
    QColor RightAxisColor=QColor();

    void addVerticalGuides(QCategoryAxis *axisX,
                                 QValueAxis *axisY,
                                 const QDateTime minDate,
                                 const QDateTime maxDate,
                                 const int nbJours)
    {
        if (!minDate.isValid() or !maxDate.isValid()) return;

        QDateTime cur=minDate;
        while ( cur <= maxDate) {
            auto *g=new QLineSeries(this);
            //g->setPointsVisible(false);
            g->append(cur.toMSecsSinceEpoch(), minYLAxis);
            g->append(cur.toMSecsSinceEpoch(), maxYLAxis);
            addSeries(g);
            g->attachAxis(axisX);
            g->attachAxis(axisY);
            g->setColor(axisX->gridLinePen().color());
            QObject::connect(this, &QChart::plotAreaChanged, [=]() {
                g->setColor(axisX->gridLinePen().color());
            });

            for (auto *m : legend()->markers(g)) m->setVisible(false);

            QString yearFormat;
            if (m_xAxisYearSeries) yearFormat="";
            else yearFormat="/yyyy";

            if (nbJours>365*3) {//Years axis.
                axisX->append(cur.addYears(-1).toString("yyyy"), cur.toMSecsSinceEpoch());
                cur=cur.addYears(1);
            } else if (nbJours>30*3) {//Months axis.
                axisX->append(cur.addMonths(-1).toString("MM"+yearFormat), cur.toMSecsSinceEpoch());
                cur=cur.addMonths(1);
            } else if (nbJours>7*3) {//Weeks axis.
                axisX->append(cur.addDays(-7).toString("dd/MM"+yearFormat), cur.toMSecsSinceEpoch());
                cur=cur.addDays(7);
            } else if (nbJours>3) {//Days axis.
                axisX->append(cur.addDays(-1).toString("dd/MM"+yearFormat), cur.toMSecsSinceEpoch());
                cur=cur.addDays(1);
            } else if (nbJours>0){//Hours axis.
                axisX->append(cur.addMSecs(-60*60*1000).toString("hh:mm"), cur.toMSecsSinceEpoch());
                cur=cur.addMSecs(60*60*1000);
            }
        }
    }

public:
    int seriesCount=4;
    QVariant minXAxis, maxXAxis;
    qreal minYLAxis, maxYLAxis, minYRAxis, maxYRAxis;
    void setXAxisLabels(QCategoryAxis* axis)
    {
        axis->categoriesLabels().clear();
        //for (int i=axis->min(); i < abscissaVar.size() and i <= axis->max(); ++i) {
        for (int i=0; i < abscissaVar.size(); ++i) {
            axis->append( abscissaVar[i].toString(), i+0.5 );
        }
    };
};

class PotaChartView : public QChartView
{
public:
    PotaChartView(QWidget *parent=nullptr) : QChartView(parent)
    {
        setMouseTracking(true);
        //setRubberBand(QChartView::HorizontalRubberBand);

    }

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) { //Pan
            bool panLeft=(event->angleDelta().y() > 0);
            if (QBarCategoryAxis* axisX=qobject_cast<QBarCategoryAxis*>(chart()->axes(Qt::Horizontal).first())) {
                const QList<QString> categories=axisX->categories();
                if (categories.isEmpty()) return;

                QString visibleMinCategory=axisX->min();
                QString visibleMaxCategory=axisX->max();
                int visibleMinCategoryIndex=categories.indexOf(visibleMinCategory);
                int visibleMaxCategoryIndex=categories.indexOf(visibleMaxCategory);

                if ((visibleMinCategoryIndex==-1)or
                    (visibleMaxCategoryIndex==-1)or
                    (panLeft and visibleMinCategoryIndex==0)or
                    (!panLeft and visibleMaxCategoryIndex==(categories.size()-1)))
                    return;//Pan is already max.

                if (panLeft)
                    axisX->setRange(categories[visibleMinCategoryIndex-1], categories[visibleMaxCategoryIndex-1]);
                else
                    axisX->setRange(categories[visibleMinCategoryIndex+1], categories[visibleMaxCategoryIndex+1]);
            }
            else if (qobject_cast<PotaGraph *>(chart())->m_xAxisDataType=="DATE") {
                if (QCategoryAxis *axisX=qobject_cast<QCategoryAxis *>(chart()->axes(Qt::Horizontal).first())) {
                    qint64 oldRange=axisX->max() - axisX->min();
                    qint64 newMin;
                    qint64 newMax;
                    qint64 absMin=qobject_cast<PotaGraph *>(chart())->minXAxis.toDateTime().toMSecsSinceEpoch();
                    qint64 absMax=qobject_cast<PotaGraph *>(chart())->maxXAxis.toDateTime().toMSecsSinceEpoch();
                    if (panLeft) {
                        newMin=axisX->min() - qint64(oldRange/10);
                        newMin=std::max(newMin, absMin);
                        newMax=newMin + oldRange;
                    } else {
                        newMax=axisX->max() + qint64(oldRange/10);
                        newMax=std::min(newMax, absMax);
                        newMin=newMax - oldRange;
                    }
                    axisX->setRange(newMin,newMax);
                }
            }
            else if (QValueAxis *axisX=qobject_cast<QValueAxis *>(chart()->axes(Qt::Horizontal).first())) {
                qreal oldRange=axisX->max() - axisX->min();
                qreal newMin;
                qreal newMax;
                if (panLeft) {
                    newMin=axisX->min() - qint64(oldRange/10);
                    newMin=fmax(newMin,qobject_cast<PotaGraph *>(chart())->minXAxis.toDouble());
                    newMax=newMin + oldRange;
                } else {
                    newMax=axisX->max() + qint64(oldRange/10);
                    newMax=fmin(newMax,qobject_cast<PotaGraph *>(chart())->maxXAxis.toDouble());
                    newMin=newMax - oldRange;
                }

                axisX->setRange(newMin, newMax);
                // if (//!zoomIn and
                //     newMin>qobject_cast<PotaGraph *>(chart())->minXAxis.toDouble() and
                //     newMax<qobject_cast<PotaGraph *>(chart())->maxXAxis.toDouble())
                //     axisX->applyNiceNumbers();
            }
        } else { //Zoom
            bool zoomIn=(event->angleDelta().y() > 0);
            qreal zoomFactor=zoomIn ? 1.1 : 1/1.1;
            QAbstractSeries* series=chart()->series().first();
            if (!series) return;

            QPointF mousePos=event->position();

            if (QBarCategoryAxis* axisX=qobject_cast<QBarCategoryAxis*>(chart()->axes(Qt::Horizontal).first())) {
                const QList<QString> categories=axisX->categories();
                if (categories.isEmpty()) return;

                QString visibleMinCategory=axisX->min();
                QString visibleMaxCategory=axisX->max();
                int visibleMinCategoryIndex=categories.indexOf(visibleMinCategory);
                int visibleMaxCategoryIndex=categories.indexOf(visibleMaxCategory);

                if (not zoomIn and visibleMinCategoryIndex==0 and visibleMaxCategoryIndex==(categories.size()-1))
                    return;//Zoom is already max.

                if (visibleMinCategoryIndex==-1 || visibleMaxCategoryIndex==-1) {
                    qDebug() << "catégories visibles ne sont pas trouvées";
                    return;  // Si les catégories visibles ne sont pas trouvées, on arrête
                }

                int visiblesCategoriesCount=visibleMaxCategoryIndex - visibleMinCategoryIndex + 1;
                QRectF plotArea=chart()->plotArea();
                qreal categoryWidth=plotArea.width() / visiblesCategoriesCount;// Largeur d'une catégorie avant zoom
                qreal mouseX=mousePos.x() - plotArea.left();
                qreal ratioMousePos=mouseX/plotArea.width();

                int categoryIndexUnderMouse=ceil(mouseX / categoryWidth) + visibleMinCategoryIndex - 1;
                categoryIndexUnderMouse=fmax(0,fmin(categoryIndexUnderMouse, categories.size() - 1));

                qreal newVisibleRange=visiblesCategoriesCount / zoomFactor;
                while (round(newVisibleRange)==visiblesCategoriesCount)
                    newVisibleRange=newVisibleRange / zoomFactor;
                newVisibleRange=fmax(1,fmin(round(newVisibleRange),categories.size()));
                //Minimum 2 bars more or 2 bars less to freeze the bar under mouse.
                // if (newVisibleRange < categories.size() and newVisibleRange==visiblesCategoriesCount+1) newVisibleRange += 1;
                // else if (newVisibleRange > 1 and newVisibleRange==visiblesCategoriesCount-1) newVisibleRange -= 1;

                int newMinCategoryIndex;
                int newMaxCategoryIndex;
                if (categoryIndexUnderMouse<categories.size()/2) {
                    newMinCategoryIndex=fmax(0,categoryIndexUnderMouse - round(newVisibleRange*ratioMousePos));
                    newMaxCategoryIndex=fmin(newMinCategoryIndex + newVisibleRange - 1, categories.size()-1);
                } else {
                    newMaxCategoryIndex=fmin(categoryIndexUnderMouse + round(newVisibleRange*(1-ratioMousePos)),categories.size()-1);
                    newMinCategoryIndex=fmax(0,newMaxCategoryIndex - newVisibleRange + 1);
                }
                QString newMinCategory=categories[newMinCategoryIndex];
                QString newMaxCategory=categories[newMaxCategoryIndex];

                // qDebug() << " ";
                // qDebug() << "ratioMousePos  : " << ratioMousePos;
                // qDebug() << "categoryWidth: " << categoryWidth;
                // qDebug() << "visibleMinCategory: " << visibleMinCategory << " index: " << visibleMinCategoryIndex;
                // qDebug() << "visibleMaxCategory: " << visibleMaxCategory << " index: " << visibleMaxCategoryIndex;
                // qDebug() << "visiblesCategoriesCount: " << visiblesCategoriesCount;
                // qDebug() << "categoryUnderMouse: " << categories[categoryIndexUnderMouse] << " index: " << categoryIndexUnderMouse;
                // qDebug() << "newVisibleRange: " << newVisibleRange;
                // qDebug() << "newMinCategory: " << newMinCategory << " index: " << newMinCategoryIndex;
                // qDebug() << "newMaxCategory: " << newMaxCategory << " index: " << newMaxCategoryIndex;

                axisX->setRange(newMinCategory, newMaxCategory);
            }
            else if (qobject_cast<PotaGraph *>(chart())->m_xAxisDataType=="DATE") {
                if (QCategoryAxis *axisX=qobject_cast<QCategoryAxis *>(chart()->axes(Qt::Horizontal).first())) {
                    QPointF chartPos=chart()->mapToValue(mousePos, series);
                    qint64 mouseX=static_cast<qint64>(chartPos.x());  // en ms epoch
                    qint64 oldMin=axisX->min();
                    qint64 oldMax=axisX->max();
                    qint64 oldRange=oldMax - oldMin;
                    qint64 newRange=static_cast<qint64>(oldRange / zoomFactor);
                    qreal ratio=qreal(mouseX - oldMin) / qreal(oldRange);
                    qint64 newMin=mouseX - qint64(ratio * newRange);
                    qint64 newMax=newMin + newRange;
                    qint64 absMin=qobject_cast<PotaGraph *>(chart())->minXAxis.toDateTime().toMSecsSinceEpoch();
                    qint64 absMax=qobject_cast<PotaGraph *>(chart())->maxXAxis.toDateTime().toMSecsSinceEpoch();
                    newMin=std::max(newMin, absMin);
                    newMax=std::min(newMax, absMax);
                    qDebug() << " ";
                    qDebug() << "minXAxis: " << qobject_cast<PotaGraph *>(chart())->minXAxis;
                    qDebug() << "maxXAxis: " << qobject_cast<PotaGraph *>(chart())->maxXAxis;
                    qDebug() << "newMinDate: " << newMin;
                    qDebug() << "newMaxDate: " << newMax;
                    axisX->setRange(newMin,newMax);
                }
            }
            else if (QValueAxis *axisX=qobject_cast<QValueAxis *>(chart()->axes(Qt::Horizontal).first())) {
                QPointF chartPos=chart()->mapToValue( mousePos,  series);
                qreal mouseX=chartPos.x();
                qreal oldRange=axisX->max() - axisX->min();
                qreal newRange=oldRange / zoomFactor;
                qreal ratio=(mouseX - axisX->min()) / oldRange;
                qreal newMin=mouseX - ratio * newRange;
                qreal newMax=newMin + newRange;

                newMin=fmax(newMin,qobject_cast<PotaGraph *>(chart())->minXAxis.toDouble());
                newMax=fmin(newMax,qobject_cast<PotaGraph *>(chart())->maxXAxis.toDouble());
                axisX->setRange(newMin, newMax);
                if (!zoomIn and
                    newMin>qobject_cast<PotaGraph *>(chart())->minXAxis.toDouble() and
                    newMax<qobject_cast<PotaGraph *>(chart())->maxXAxis.toDouble())
                    axisX->applyNiceNumbers();
                // if (oldMin==axisX->min() and oldMax==axisX->max())
                //     axisX->setRange(newMin, newMax);
            }
            // else if (QDateTimeAxis *axisX=qobject_cast<QDateTimeAxis *>(chart()->axes(Qt::Horizontal).first())) {
            //     QPointF chartPos=chart()->mapToValue(mousePos, series);
            //     qint64 mouseX=static_cast<qint64>(chartPos.x());  // en ms epoch
            //     qint64 oldMin=axisX->min().toMSecsSinceEpoch();
            //     qint64 oldMax=axisX->max().toMSecsSinceEpoch();
            //     qint64 oldRange=oldMax - oldMin;
            //     qint64 newRange=static_cast<qint64>(oldRange / zoomFactor);
            //     qreal ratio=qreal(mouseX - oldMin) / qreal(oldRange);
            //     qint64 newMin=mouseX - qint64(ratio * newRange);
            //     qint64 newMax=newMin + newRange;
            //     qint64 absMin=qobject_cast<PotaGraph *>(chart())->minXAxis.toDateTime().toMSecsSinceEpoch();
            //     qint64 absMax=qobject_cast<PotaGraph *>(chart())->maxXAxis.toDateTime().toMSecsSinceEpoch();
            //     newMin=std::max(newMin, absMin);
            //     newMax=std::min(newMax, absMax);
            //     // qDebug() << " ";
            //     // qDebug() << "minXAxis: " << qobject_cast<PotaGraph *>(chart())->minXAxis;
            //     // qDebug() << "maxXAxis: " << qobject_cast<PotaGraph *>(chart())->maxXAxis;
            //     // qDebug() << "newMinDate: " << newMinDate;
            //     // qDebug() << "newMaxDate: " << newMaxDate;
            //     axisX->setRange(QDateTime::fromMSecsSinceEpoch(newMin),
            //                     QDateTime::fromMSecsSinceEpoch(newMax));
            // }
        }
        event->accept();
    }
};



