#include "potagraph.h"
#include "Dialogs.h"
#include "qbarcategoryaxis.h"
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QDateTimeAxis>
#include <QLocale>
#include "PotaUtils.h"


// PotaGraph::PotaGraph(QGraphicsItem* parent,
//                      int idxAbscisse,
//                      QString dataTypeAbscisse,
//                      int idxOrdonnee1,
//                      int aggregationType1,
//                      int type1,
//                      int idxOrdonnee2,
//                      bool ordonnee2RightAxis,
//                      int aggregationType2,
//                      int type2,
//                      QAbstractItemModel *model)
//     : QChart(parent),
//       m_idxAbscisse(idxAbscisse),
//       m_dataTypeAbscisse(dataTypeAbscisse),
//       m_idxOrdonnee1(idxOrdonnee1),
//       m_aggregationType1(aggregationType1),
//       m_type1(type1),
//       m_idxOrdonnee2(idxOrdonnee2),
//       m_ordonnee2RightAxis(ordonnee2RightAxis),
//       m_aggregationType2(aggregationType2),
//       m_type2(type2)
PotaGraph::PotaGraph(QGraphicsItem* parent)
    : QChart(parent){
    for (int i =0;i<seriesCount;i++) {
        m_yAxisFieldNum.resize(i+1);
        m_yAxisFieldNum[i]=-1;
        // m_yAxisDataType.resize(i+1);
        // m_yAxisDataType[i]="";
        m_yAxisCalc.resize(i+1);
        m_yAxisCalc[i]=calcSeriesNotNull;
        m_yAxisType.resize(i+1);
        m_yAxisType[i]=typeSeriesLine;
        m_yRightAxis.resize(i+1);
        m_yRightAxis[i]=false;
        m_yColor.resize(i+1);
        m_yColor[i]=QColor();
    }
}

void PotaGraph::createSeries(QAbstractItemModel* model)
{
    removeAllSeries();

    if (m_xAxisYearSeries) {
        for (int row=0; row < model->rowCount(); ++row) {
            int year=model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().year();
            if (!years.contains(year))
                years.append(year);
        }
        m_series.resize(years.count());
        seriesName.resize(years.count());
        ordinateVar.resize(years.count());
        seriesCount=years.count();

        for (int i =0;i<years.count();i++) {
            seriesName[i]=str(years[i]);
            if (m_yAxisType[0]==typeSeriesScatter or m_yAxisType[0]==typeSeriesScatNotNull)
                m_series[i]=new QScatterSeries(this);
            else if (m_yAxisType[0]==typeSeriesBar)
                m_series[i]=new QBarSeries(this);
            else
                m_series[i]=new QLineSeries(this);
            addSeries(m_series[i]);
        }
    } else {
        m_series.resize(seriesCount);
        seriesName.resize(seriesCount);
        ordinateVar.resize(seriesCount);
        for (int i =0;i<seriesCount;i++) {
            seriesName[i]="";
            if (m_yAxisFieldNum[i]>=0  or i==0) {
                if (m_yAxisType[i]==typeSeriesScatter or m_yAxisType[i]==typeSeriesScatNotNull)
                    m_series[i]=new QScatterSeries(this);
                else if (m_yAxisType[i]==typeSeriesBar)
                    m_series[i]=new QBarSeries(this);
                else
                    m_series[i]=new QLineSeries(this);
                addSeries(m_series[i]);
            } else m_series[i]=nullptr;
        }
    }
}

void PotaGraph::fillSeries(QAbstractItemModel* model)
{
    if (!model or model->rowCount()==0) return;

    //Suppression des axes.
    while (axes().count()>0){
        QAbstractAxis* axis=axes()[0];
        removeAxis(axis);
        axis->deleteLater();
    }

    QString abscisseName,axisYLeftTitle;
    abscisseName=model->headerData(m_xAxisFieldNum,Qt::Horizontal).toString();
    int nbJours=0;

    minYLAxis=std::numeric_limits<double>::max();
    maxYLAxis=std::numeric_limits<double>::lowest();
    minYRAxis=std::numeric_limits<double>::max();
    maxYRAxis=std::numeric_limits<double>::lowest();

    for (int i =0;i<seriesCount;i++) {
        if (m_series[i]) {
            int yAxisFieldNum,yAxisCalc,yAxisType;
            bool yRightAxis;
            QColor yColor;
            //Series name
            if (m_xAxisYearSeries) {
                yAxisFieldNum=m_yAxisFieldNum[0];
                yAxisCalc=m_yAxisCalc[0];
                yAxisType=m_yAxisType[0];
                yRightAxis=m_yRightAxis[0];
                yColor=QColor();
            } else {
                yAxisFieldNum=m_yAxisFieldNum[i];
                yAxisCalc=m_yAxisCalc[i];
                yAxisType=m_yAxisType[i];
                yRightAxis=m_yRightAxis[i];
                yColor=m_yColor[i];
            }

            if (yAxisFieldNum==-1) {
                seriesName[i]=QObject::tr("Nombre de lignes");
            } else {
                seriesName[i]=model->headerData(yAxisFieldNum,Qt::Horizontal).toString();
                if (m_xAxisGroup==xAxisGroupNo) {}
                else if (yAxisCalc==calcSeriesNotNull)
                    seriesName[i]+=" ("+QObject::tr("Nb valeurs non vides")+")";
                else if (yAxisCalc==calcSeriesDistinct)
                    seriesName[i]+=" ("+QObject::tr("Nb valeurs distinctes")+")";
                else if (yAxisCalc==calcSeriesFirst)
                    seriesName[i]+=" ("+QObject::tr("1ère valeur")+")";
                else if (yAxisCalc==calcSeriesLast)
                    seriesName[i]+=" ("+QObject::tr("Dernière valeur")+")";
                else if (yAxisCalc==calcSeriesAverage)
                    seriesName[i]+=" ("+QObject::tr("Moyenne")+")";
                else if (yAxisCalc==calcSeriesMin)
                    seriesName[i]+=" ("+QObject::tr("Minimun")+")";
                else if (yAxisCalc==calcSeriesMax)
                    seriesName[i]+=" ("+QObject::tr("Maximum")+")";
                else if (yAxisCalc==calcSeriesSum)
                    seriesName[i]+=" ("+QObject::tr("Somme")+")";
            }

            if (m_xAxisYearSeries) {
                axisYLeftTitle=seriesName[i];
                seriesName[i].setNum(years[i]);
            }

            //Extract data from model.
            abscissaVar.clear();
            ordinateVar[i].clear();

            if (m_xAxisGroup==xAxisGroupNo) { // All abscissa values
                QString label;
                QMap<QString, int> labelCounts;
                for (int row=0; row < model->rowCount(); ++row) {
                    label=model->index(row, m_xAxisFieldNum).data().toString();
                    int count=labelCounts.value(label, 0) + 1;
                    labelCounts[label]=count;
                    if (count > 1)
                        abscissaVar << label+" ("+QString::number(count)+")";
                    else
                        abscissaVar << label;
                    ordinateVar[i] << model->index(row, yAxisFieldNum).data(Qt::EditRole).toDouble();
                    //qDebug() << abscissaVar << ordinateVar[i];
                }
            } else { //Group abscissa values
                QMap<QString, QList<QVariant>> abscissaVarNotGrouped;
                for (int row=0; row < model->rowCount(); ++row) { //read data from model and prepare grouping.
                    if (!m_xAxisYearSeries or model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().year()==years[i]) {
                        QString key;
                        if (model->index(row, m_xAxisFieldNum).data(Qt::EditRole).isNull()) {
                            key=" ";
                        } else if (m_xAxisGroup==xAxisGroupSame) {
                            key=model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toString();
                        } else if  (m_xAxisDataType=="TEXT") {
                            if (m_xAxisGroup==xAxisGroupFirstWord)
                                key=model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toString().split(" ").first();
                            else if (m_xAxisGroup >= xAxisGroupfirstChar) //First chars
                                key=model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toString().first(m_xAxisGroup-2);
                        } else if  (m_xAxisDataType=="DATE") {
                            if (m_xAxisGroup==xAxisGroupYear)
                                key=QDate(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().year(),1,1).toString("yyyy-MM-dd");
                            else if (m_xAxisGroup==xAxisGroupMonth)
                                key=QDate(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().year(),
                                            model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().month(),1).toString("yyyy-MM-dd");
                            else if (m_xAxisGroup==xAxisGroupWeek)
                                key=firstDayOffWeek(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate()).toString("yyyy-MM-dd");
                            else if (m_xAxisGroup==xAxisGroupDay)
                                key=model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDate().toString("yyyy-MM-dd");
                        } else if  (m_xAxisDataType=="REAL" or m_xAxisDataType.startsWith("INT")) {
                            if (m_xAxisGroup==xAxisGroup1000)
                                key=QString::number(std::floor(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDouble()/1000)*1000);
                            else if (m_xAxisGroup==xAxisGroup100)
                                key=QString::number(std::floor(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDouble()/100)*100);
                            else if (m_xAxisGroup==xAxisGroup10)
                                key=QString::number(std::floor(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDouble()/10)*10);
                            if  (m_xAxisDataType=="REAL") {
                                if (m_xAxisGroup==xAxisGroup1)
                                    key=QString::number(std::floor(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDouble()/1)*1);
                                else if (m_xAxisGroup >= xAxisGroup1Decimal)
                                    key=QString::number(std::floor(model->index(row, m_xAxisFieldNum).data(Qt::EditRole).toDouble()*std::pow(10,m_xAxisGroup-1))/std::pow(10,m_xAxisGroup-1));
                            }
                        }
                        if (key.isEmpty()) key=" ";
                        QVariant value=model->index(row, yAxisFieldNum).data(Qt::EditRole);
                        if (m_xAxisYearSeries)
                            key="2000-"+key.last(5);
                        abscissaVarNotGrouped[key].append(value);
                    }
                }

                if  (m_xAxisDataType=="DATE") { //Add zero value if point will missing on xAxis
                    if (m_xAxisGroup==xAxisGroupYear) {
                        if (!abscisseName.contains(" ("+tr("année")+")"))
                            abscisseName+=" ("+tr("année")+")";
                        int xMin=std::numeric_limits<int>::max();
                        int xMax=std::numeric_limits<int>::min();
                        for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                            //qDebug() << it.key();
                            xMin=fmin(xMin,QDate::fromString(it.key(),"yyyy-MM-dd").year());
                            xMax=fmax(xMax,QDate::fromString(it.key(),"yyyy-MM-dd").year());
                        }
                        int x=xMin+1;
                        while (x<xMax) {
                            bool present=false;
                            for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                                if (x==QDate::fromString(it.key(),"yyyy-MM-dd").year()) {
                                    present=true;
                                    break;
                                }
                            }
                            if (!present)
                                abscissaVarNotGrouped[QString::number(x)+"-01-01"].append(0);
                            x++;
                        }
                    } else if (m_xAxisGroup==xAxisGroupMonth) {
                        if (!abscisseName.contains(" ("+tr("mois")+")"))
                            abscisseName+=" ("+tr("mois")+")";
                        int xMin=std::numeric_limits<int>::max();
                        int xMax=std::numeric_limits<int>::min();
                        for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                            //qDebug() << it.key();
                            xMin=fmin(xMin,QDate::fromString(it.key(),"yyyy-MM-dd").year()*12+QDate::fromString(it.key(),"yyyy-MM-dd").month());
                            xMax=fmax(xMax,QDate::fromString(it.key(),"yyyy-MM-dd").year()*12+QDate::fromString(it.key(),"yyyy-MM-dd").month());
                        }
                        int x=xMin+1;
                        while (x<xMax) {
                            bool present=false;
                            for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                                if (x==QDate::fromString(it.key(),"yyyy-MM-dd").year()*12+QDate::fromString(it.key(),"yyyy-MM-dd").month()) {
                                    present=true;
                                    break;
                                }
                            }

                            if (!present)
                                abscissaVarNotGrouped[QString::number(floor(x/12))+"-"+StrLast("0"+QString::number(x-floor(x/12)*12),2)+"-01"].append(0);
                            x++;
                        }
                    } else if (m_xAxisGroup==xAxisGroupWeek) {
                        if (!abscisseName.contains(" ("+tr("semaine")+")"))
                            abscisseName+=" ("+tr("semaine")+")";
                        int xMin=std::numeric_limits<int>::max();
                        int xMax=std::numeric_limits<int>::min();
                        for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                            //qDebug() << it.key();
                            xMin=fmin(xMin,floor(QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay()/7));
                            xMax=fmax(xMax,floor(QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay()/7));
                        }
                        int x=xMin+1;
                        while (x<xMax) {
                            bool present=false;
                            for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                                if (x==floor(QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay()/7)) {
                                    present=true;
                                    break;
                                }
                            }
                            if (!present)
                                abscissaVarNotGrouped[QDate::fromJulianDay(x*7).toString("yyyy-MM-dd")].append(0);
                            x++;
                        }
                    } else if (m_xAxisGroup==xAxisGroupDay) {
                        int xMin=std::numeric_limits<int>::max();
                        int xMax=std::numeric_limits<int>::min();
                        for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                            xMin=fmin(xMin,QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay());
                            xMax=fmax(xMax,QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay());
                        }
                        int x=xMin+1;
                        while (x<xMax) {
                            bool present=false;
                            for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) {
                                if (x==QDate::fromString(it.key(),"yyyy-MM-dd").toJulianDay()) {
                                    present=true;
                                    break;
                                }
                            }
                            if (!present)
                                abscissaVarNotGrouped[QDate::fromJulianDay(x).toString("yyyy-MM-dd")].append(0);
                            x++;
                        }
                    }
                }

                for (auto it=abscissaVarNotGrouped.begin(); it != abscissaVarNotGrouped.end(); ++it) { //Group data.
                    const QVariant& absc=it.key();
                    const QList<QVariant>& ordos=it.value();

                    QVariant y=0;
                    if (yAxisFieldNum==-1) { //Count
                        y=ordos.size();
                    } else if (yAxisCalc==calcSeriesNotNull) {
                        int count=0;
                        for (const QVariant& v : ordos) if (!v.isNull()) ++count;
                        y=count;
                    } else if (yAxisCalc==calcSeriesDistinct) {
                        QSet<QString> uniqueVals;
                        for (const QVariant& v : ordos) uniqueVals.insert(v.toString());
                        y=uniqueVals.size();
                    } else if (yAxisCalc==calcSeriesFirst) {
                        y=ordos.first();
                    } else if (yAxisCalc==calcSeriesLast) {
                        y=ordos.last();
                    } else if (yAxisCalc==calcSeriesAverage) {
                        for (const QVariant& v : ordos) y=y.toDouble() + v.toDouble();
                        y=y.toDouble() / ordos.size();
                    } else if (yAxisCalc==calcSeriesMin) {
                        y=std::numeric_limits<double>::max();
                        for (const QVariant& v : ordos) y=std::min(y.toDouble(), v.toDouble());
                    } else if (yAxisCalc==calcSeriesMax) {
                        y=std::numeric_limits<double>::min();
                        for (const QVariant& v : ordos) y=std::max(y.toDouble(), v.toDouble());
                    } else if (yAxisCalc==calcSeriesSum) {
                        for (const QVariant& v : ordos) y=y.toDouble() + v.toDouble();
                    }
                    // } else {
                    //     for (const QVariant& v : ordos) {
                    //         abscissaVar << absc;
                    //         ordinateVar[i] << v.toDouble();
                    //     }
                    //     continue;
                    abscissaVar << absc;
                    ordinateVar[i] << y;
                }

            }

            //Calculate min and max on Y axis.
            if (yRightAxis) {
                for (int j=0; j < ordinateVar[i].size(); ++j) {
                    if (minYRAxis > ordinateVar[i][j].toDouble()) minYRAxis=ordinateVar[i][j].toDouble();
                    if (maxYRAxis < ordinateVar[i][j].toDouble()) maxYRAxis=ordinateVar[i][j].toDouble();
                }
            } else {
                for (int j=0; j < ordinateVar[i].size(); ++j) {
                    if (minYLAxis > ordinateVar[i][j].toDouble()) minYLAxis=ordinateVar[i][j].toDouble();
                    if (maxYLAxis < ordinateVar[i][j].toDouble()) maxYLAxis=ordinateVar[i][j].toDouble();
                }
            }

            // Populate series and create axis X and Y
            if (yAxisType==typeSeriesBar){
                QBarSeries* barSeries=qobject_cast<QBarSeries*>(m_series[i]);
                barSeries->clear();
                QBarSet* set=new QBarSet(seriesName[i]);
                QStringList categories;
                for (int j=0; j < abscissaVar.size(); ++j) { // && j < ordinateVar[j].size()
                    categories << abscissaVar[j].toString();
                    *set << ordinateVar[i][j].toDouble();
                }
                if (yColor.isValid()) {
                    set->setColor(yColor);
                    QObject::connect(this, &QChart::plotAreaChanged, [=]() {
                        set->setColor(yColor);
                    });
                }
                barSeries->append(set);
                minXAxis=0;
                maxXAxis=abscissaVar.size();

                if (i==0) {
                    // X axis
                    QBarCategoryAxis* axisXBar=new QBarCategoryAxis;
                    axisXBar->append(categories);
                    addAxis(axisXBar, Qt::AlignBottom);
                    //barSeries->attachAxis(axisXBar);

                    // Y left axis
                    QValueAxis* axisY=new QValueAxis;
                    addAxis(axisY, Qt::AlignLeft);
                    //barSeries->attachAxis(axisY);
                    //axisY->setTitleText(seriesName[i]);
                } else if (yRightAxis and !RightAxisCreated) {
                    // Y right axis
                    QValueAxis* axisY=new QValueAxis;
                    addAxis(axisY, Qt::AlignRight);
                    //barSeries->attachAxis(axisY);
                    //axisY->setTitleText(seriesName[i]);
                    RightAxisCreated=true;
                    RightAxisColor=yColor;
                }
            } else { //Line or scatter.
                QXYSeries* series=qobject_cast<QXYSeries*>(m_series[i]);
                series->clear();
                series->setName(seriesName[i]);
                if (yColor.isValid()) {
                    series->setColor(yColor);
                    QObject::connect(this, &QChart::plotAreaChanged, [=]() {
                        series->setColor(yColor);
                    });
                }

                for (int j=0; j < abscissaVar.size(); ++j) { // && j < ordinateVar[i].size()
                    double y=ordinateVar[i][j].toDouble();
                    if (yAxisType!=typeSeriesScatNotNull or y>0) {
                        if (m_xAxisDataType=="REAL" or m_xAxisDataType.startsWith("INT") or m_xAxisDataType.startsWith("BOOL")) {
                            series->append(abscissaVar[j].toDouble(), y);
                        } else if (m_xAxisDataType=="DATE") {
                            QDateTime dt;
                            dt=abscissaVar[j].toDateTime();
                            if (dt.isValid())
                                series->append(dt.toMSecsSinceEpoch(), y);
                        } else {
                            series->append(j, y);
                        }
                    }
                }

                if (i==0) {
                    //X axis.
                    if (m_xAxisDataType=="DATE") {
                        minXAxis=QDateTime();
                        maxXAxis=QDateTime();
                        QDateTime dt;
                        for (const QPointF &point : series->points()) {
                            dt=QDateTime::fromMSecsSinceEpoch(point.x());
                            if ((minXAxis==QDateTime()) or (dt < minXAxis.toDateTime()))
                                minXAxis=dt;
                            if ((maxXAxis==QDateTime()) or (dt > maxXAxis.toDateTime()))
                                maxXAxis=dt;
                        }
                        nbJours=minXAxis.toDate().daysTo(maxXAxis.toDate());
                        //qDebug() << "nbJours: " << nbJours;
                        if (nbJours>365*3) {//Years axis.
                            minXAxis=QDate(minXAxis.toDate().year(), 1, 1).startOfDay();
                            maxXAxis=QDate(maxXAxis.toDate().year() + 1, 1, 1).startOfDay();
                        } else if (nbJours>30*3) {//Months axis.
                            minXAxis=QDate(minXAxis.toDate().year(),minXAxis.toDate().month(), 1).startOfDay();
                            maxXAxis=QDate(maxXAxis.toDate().year(),maxXAxis.toDate().month(), 1).addMonths(1).startOfDay();
                        } else if (nbJours>7*3) {//Weeks axis.
                            minXAxis=minXAxis.toDate().startOfDay();
                            maxXAxis=maxXAxis.toDate().addDays(1).startOfDay();
                        } else if (nbJours>3) {//Days axis.
                            minXAxis=minXAxis.toDate().startOfDay();
                            maxXAxis=maxXAxis.toDate().addDays(1).startOfDay();
                        } else if (nbJours>0){//Hours axis.
                            minXAxis=minXAxis.toDate().startOfDay();
                        }
                    } else {
                        minXAxis=std::numeric_limits<double>::max();
                        maxXAxis=std::numeric_limits<double>::lowest();
                        for (const QPointF &point : series->points()) {
                            minXAxis=fmin(minXAxis.toDouble(),point.x());
                            maxXAxis=fmax(maxXAxis.toDouble(),point.x());
                        }
                        if (series->points().count()>1) //Augmenter la taille de l'axe pour que le dernier point ne soit pas au bout.
                            maxXAxis=maxXAxis.toDouble()+series->points()[1].x()-series->points()[0].x();
                    }

                    if (m_xAxisDataType=="REAL" or m_xAxisDataType.startsWith("INT") or m_xAxisDataType.startsWith("BOOL")) {
                        auto* axis=new QValueAxis;
                        axis->setRange(minXAxis.toDouble(),maxXAxis.toDouble());
                        axis->applyNiceNumbers();
                        addAxis(axis, Qt::AlignBottom);
                    } else if (m_xAxisDataType=="DATE") {
                        auto* axis=new QCategoryAxis; //Don't use QDateTimeAxis
                        //axis->setFormat("yyyy-MM-dd");//HH:mm:ss
                        if (m_xAxisYearSeries) {
                            minXAxis="2000-01-01";
                            maxXAxis="2000-12-31";
                        }
                        axis->setRange(minXAxis.toDateTime().toMSecsSinceEpoch(),maxXAxis.toDateTime().toMSecsSinceEpoch());
                        addAxis(axis, Qt::AlignBottom);
                    } else {
                        auto* axis=new QCategoryAxis;
                        axis->setRange(0, abscissaVar.size());// ne pas mettre abscissaVar.size()-1 pour que le dernier points ne soit pas au bout.
                        setXAxisLabels(axis);
                        addAxis(axis, Qt::AlignBottom);
                    }

                    // Y left axis
                    auto* axisYLeft=new QValueAxis;
                    addAxis(axisYLeft, Qt::AlignLeft);
                    //axisYLeft->setTitleText(seriesName[i]);

                } else if (yRightAxis and !RightAxisCreated) {
                    // Y right axis
                    auto* axisYRight=new QValueAxis;
                    addAxis(axisYRight, Qt::AlignRight);
                    //axisYRight->setTitleText(seriesName[i]);
                    RightAxisCreated=true;
                    RightAxisColor=yColor;
                }
            }
            m_series[i]->attachAxis(axes(Qt::Horizontal).first());
            if (yRightAxis) {
                m_series[i]->attachAxis(axes(Qt::Vertical).last());
                if (axes(Qt::Vertical).last()->titleText().isEmpty())
                    axes(Qt::Vertical).last()->setTitleText(seriesName[i]);
                else
                    axes(Qt::Vertical).last()->setTitleText(axes(Qt::Vertical).last()->titleText()+" "+seriesName[i]);
            } else {
                m_series[i]->attachAxis(axes(Qt::Vertical).first());
                if (!m_xAxisYearSeries) {
                    if (axes(Qt::Vertical).first()->titleText().isEmpty())
                        axes(Qt::Vertical).first()->setTitleText(seriesName[i]);
                    else
                        axes(Qt::Vertical).first()->setTitleText(axes(Qt::Vertical).first()->titleText()+" "+seriesName[i]);
                }
            }
        }
    }

    //Apply title, min and max on axis.

    if ((minYLAxis>0 and minYLAxis<maxYLAxis/2)or(minYLAxis<0 and minYLAxis>maxYLAxis/2)) minYLAxis=0;
    else minYLAxis=std::round(minYLAxis*0.9);
    maxYLAxis=std::round(maxYLAxis*1.1);
    if ((minYRAxis>0 and minYRAxis<maxYRAxis/2)or(minYRAxis<0 and minYRAxis>maxYRAxis/2)) minYRAxis=0;
    else minYRAxis=std::round(minYRAxis*0.9);
    maxYRAxis=std::round(maxYRAxis*1.1);
    axes(Qt::Horizontal).first()->setTitleText(abscisseName);
    if (QValueAxis* axisYLeft=qobject_cast<QValueAxis*>(axes(Qt::Vertical).first())) {
        if (m_xAxisDataType=="DATE") {
            if (QCategoryAxis* axisX=qobject_cast<QCategoryAxis*>(axes(Qt::Horizontal).first())) {
                addVerticalGuides(axisX, axisYLeft, minXAxis.toDateTime() , maxXAxis.toDateTime(),nbJours);
                axisX->setGridLineVisible(false);
            }
        }
        axisYLeft->setRange(minYLAxis, maxYLAxis);
        axisYLeft->applyNiceNumbers();
        if (m_xAxisYearSeries)
            axisYLeft->setTitleText(axisYLeftTitle);
    }

    if (m_yColor[0].isValid()) {
        axes(Qt::Vertical).first()->setGridLineColor(m_yColor[0]);
        axes(Qt::Vertical).first()->setLabelsColor(m_yColor[0]);
        QObject::connect(this, &QChart::plotAreaChanged, [=]() {
            axes(Qt::Vertical).first()->setGridLineColor(m_yColor[0]);
            axes(Qt::Vertical).first()->setLabelsColor(m_yColor[0]);
        });
    }

    if (RightAxisCreated) {
       if (QValueAxis* axisY=qobject_cast<QValueAxis*>(axes(Qt::Vertical).last())) {
            axisY->setRange(minYRAxis, maxYRAxis);
            axisY->applyNiceNumbers();
        }
        if (RightAxisColor.isValid()) {
            axes(Qt::Vertical).last()->setGridLineColor(RightAxisColor);
            axes(Qt::Vertical).last()->setLabelsColor(RightAxisColor);
            QObject::connect(this, &QChart::plotAreaChanged, [=]() {
                axes(Qt::Vertical).last()->setGridLineColor(RightAxisColor);
                axes(Qt::Vertical).last()->setLabelsColor(RightAxisColor);
            });
        }
    }

}
