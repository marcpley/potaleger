#include "potawidget.h"
#include "mainwindow.h"
#include "qlabel.h"
#include "qsqlerror.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>

PotaModel::PotaModel(QWidget *parent)
{

}

QString PotaModel::FieldName(int index)
{
    QSqlQuery query("PRAGMA table_xinfo("+tableName()+");");
    query.seek(index);
    return query.value("name").toString();
}

bool PotaModel::SelectShowErr()
{
    select();
    if (lastError().type() != QSqlError::NoError)
    {
        dynamic_cast<MainWindow*>(parent()->parent()->parent())->ui->lDBErr->setText(lastError().text());
        return false;
    }
    return true;
}

PotaQueryModel::PotaQueryModel(QWidget *parent)
{

}

QString PotaQueryModel::FieldName(int index)
{
    return "";
}

bool PotaQueryModel::setQueryShowErr(QString query)
{
    setQuery(query);
    if (lastError().type() != QSqlError::NoError)
    {
        dynamic_cast<MainWindow*>(parent()->parent()->parent())->ui->lDBErr->setText(lastError().text());
        return false;
    }
    return true;
}

PotaWidget::PotaWidget(QWidget *parent)
        : QWidget(parent)
{

}

