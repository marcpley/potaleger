#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

bool MainWindow::OkCancelDialog(QString sMessage)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(windowTitle()+" "+ui->lVer->text());
    msgBox.setText(sMessage);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Question);
    return (msgBox.exec() == QMessageBox::Ok);
}

void MainWindow::MessageDialog(QString sMessage, QMessageBox::Icon Icon)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(windowTitle()+" "+ui->lVer->text());
    msgBox.setText(sMessage);
    msgBox.setIcon(Icon);
    msgBox.exec();
}

void MainWindow::MessageDialog(QString sMessage)
{
    MessageDialog(sMessage,QMessageBox::NoIcon);
}

