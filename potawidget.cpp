#include "potawidget.h"
#include "mainwindow.h"
#include "qlabel.h"
#include "qsqlerror.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>

//PotaTableModel::PotaTableModel(){}

QString PotaTableModel::FieldName(int index)
{
    QSqlQuery query("PRAGMA table_xinfo("+tableName()+");");
    if (query.seek(index))
        return query.value("name").toString();
    else
        return "";
}

bool PotaTableModel::SelectShowErr()
{
    select();
    if ((lastError().type() != QSqlError::NoError)and(parent()->objectName().startsWith("PW")))
    {
        dynamic_cast<PotaWidget*>(parent())->lErr->setText(lastError().text());//A faire : tester
        return false;
    }
    return true;
}

bool PotaTableModel::SubmitAllShowErr()
{
    submitAll();
    if (lastError().type() == QSqlError::NoError)
    {
        if (parent()->objectName().startsWith("PW"))
            dynamic_cast<PotaWidget*>(parent())->lErr->setText(tableName()+": "+tr("modifications écrites dans la table."));
    }
    else
    {
        if (parent()->objectName().startsWith("PW"))
            dynamic_cast<PotaWidget*>(parent())->lErr->setText(lastError().text());
        return false;
    }
    return true;
}

bool PotaTableModel::RevertAllShowErr()
{
    revertAll();
    if (lastError().type() == QSqlError::NoError)
    {
        if (parent()->objectName().startsWith("PW"))
            dynamic_cast<PotaWidget*>(parent())->lErr->setText(tableName()+": "+tr("modifications abandonnées."));
    }
    else
    {
        if (parent()->objectName().startsWith("PW"))
            dynamic_cast<PotaWidget*>(parent())->lErr->setText(lastError().text());
        return false;
    }
    return true;
}

//PotaQueryModel::PotaQueryModel(){}

QString PotaQueryModel::FieldName(int index)
{
    if (TableName=="")
        return "";
    else
    {
        if (index < columnCount())
            return headerData(index,Qt::Horizontal).toString();
        else
            return "";
    }
}

bool PotaQueryModel::setQueryShowErr(QString query)
{
    setQuery(query);
    if (lastError().type() != QSqlError::NoError)
    {
        if (lErr!=nullptr)
            lErr->setText(lastError().text());
        return false;

    }
    return true;
}

bool PotaQueryModel::setMultiQueryShowErr(QString querys)
{
    QStringList QueryList = querys.split(";");
    for (int i=0;i<QueryList.count();i++)
    {
        if (QueryList[i].trimmed().isEmpty())
            continue;
        setQuery(QueryList[i]);
        if (lastError().type() != QSqlError::NoError)
        {
            if (lErr!=nullptr)
                lErr->setText(lastError().text());
            return false;

        }
    }
    return true;
}

PotaWidget::PotaWidget(QWidget *parent) : QWidget(parent)
{
    model = new PotaTableModel(this);
    model->setParent(this);//A faire: le constructeur de PotaTableModel ne set pas le parent. Pourquoi ?
    tv = new QTableView(this);
    tv->setParent(this);
}

