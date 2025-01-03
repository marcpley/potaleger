#include "qsqldriver.h"
#include "mainwindow.h"
#include "qsqlerror.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <sqlite3.h>
#include "sqlean/sqlite3.h"
#include "sqlean/define.h"

bool MainWindow::initCustomFunctions() {
    qDebug() << "SQLite version:" << sqlite3_libversion();

    QSqlDatabase db = QSqlDatabase::database();
    auto handle = db.driver()->handle();
    qDebug() << "driver handle : " << handle;
    sqlite3 *db_handle = *static_cast<sqlite3 **>(handle.data());
    qDebug() << "db_handle : " << db_handle;
    if (!db_handle or db_handle==0) {
        qCritical() << "Failed to retreive SQLite handle.";
        return false;
    }
    sqlite3_initialize();
    define_manage_init(db_handle);
    define_eval_init(db_handle);
    return true;
}

bool MainWindow::registerCustomFunctions() {
    QSqlQuery q1;
    if (!q1.exec("SELECT define('Somme', '? + ?');")){
        qCritical() << "Failed to register function 'Somme': "<< q1.lastError();
        return false;
    }

    q1.clear();
    if (!q1.exec("SELECT eval('insert into tmp(value) values (1), (2), (3)');"))
        qDebug() << "error" << q1.lastError();

    qDebug() << "Functions registered successfully.";

    return true;
}

bool MainWindow::testCustomFunctions() {
    QSqlQuery q1;
    if (!q1.exec("SELECT Somme(3,5);")){
        qCritical() << "Function 'Somme': "<< q1.lastError();
        return false;
    } else {
        q1.next();
        qDebug() << "Somme(3,5) = " << q1.value(0).toString();
    }
    qDebug() << "Functions externally tested.";
    return true;
}
