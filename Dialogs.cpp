#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLEInteger>
#include "PotaUtils.h"

void MainWindow::MessageDialog(const QString &titre, const QString &message, const QString &message2, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QString sMess=message;
    QString sMess2=message2;
    if (isDarkTheme()){
        sMess=StrReplace(sMess,"<a href","<a style=\"color: #7785ff\" href");
        sMess2=StrReplace(sMess2,"<a href","<a style=\"color: #7785ff\" href");
    }
    QLabel *messageLabel = new QLabel(sMess);
    //messageLabel->setWordWrap(true);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    headerLayout->addWidget(messageLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    if (message2!=""){
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        //QHBoxLayout *textLayout = new QHBoxLayout();
        QLabel *messageLabel2 = new QLabel(sMess2);
        messageLabel2->setWordWrap(true);
        messageLabel2->setOpenExternalLinks(true);
        messageLabel2->setTextInteractionFlags(Qt::TextSelectableByMouse);
        messageLabel2->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        SetFontWeight(messageLabel2,QFont::Light);
        SetFontWeight(messageLabel,QFont::DemiBold);
        scrollArea->setWidget(messageLabel2);
        layout->addWidget(scrollArea);
        //textLayout->addWidget(messageLabel2);
        //layout->addLayout(textLayout);
        QSize screenSize = QGuiApplication::primaryScreen()->size();
        int maxHeight = screenSize.height()-200; // Par exemple, la moitié de la hauteur de l'écran
        scrollArea->setMaximumHeight(maxHeight);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    layout->addLayout(buttonLayout);

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        dialog.accept();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.
    //dialog.setMinimumWidth(w);


    dialog.exec();
}

bool MainWindow::OkCancelDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result = false;
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        result = true;
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result = false;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

int MainWindow::RadiobuttonDialog(const QString &titre, const QString &message, const QStringList &options, const int iDef, QStyle::StandardPixmap iconType, const int MinWidth) {
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QButtonGroup *buttonGroup = new QButtonGroup(&dialog);
    buttonGroup->setExclusive(true);

    QList<QRadioButton *> radioButtons;
    for (int i = 0; i < options.size(); ++i) {
        QRadioButton *radioButton = new QRadioButton(options[i]);
        buttonGroup->addButton(radioButton, i);
        layout->addWidget(radioButton);
        radioButtons.append(radioButton);
        if (i==iDef)
            radioButton->setChecked(true);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Annuler"));
    okButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogOkButton));
    cancelButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogCancelButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    int result = -1; // Valeur par défaut si annulé
    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        result = buttonGroup->checkedId(); // Récupère l'ID du bouton sélectionné
        dialog.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        result = -1;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

bool MainWindow::YesNoDialog(const QString &titre, const QString &message, QStyle::StandardPixmap iconType, const int MinWidth)
{
    QDialog dialog(this);
    dialog.setWindowTitle(titre);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QHBoxLayout *headerLayout = new QHBoxLayout();

    if (iconType!=QStyle::SP_CustomBase) {
        QLabel *iconLabel = new QLabel();
        QIcon icon;
        if (iconType==QStyle::NStandardPixmap)
            icon = QIcon(":/images/potaleger.svg");
        else
            icon = QApplication::style()->standardIcon(iconType);
        iconLabel->setPixmap(icon.pixmap(64, 64));
        iconLabel->setFixedSize(64,64);
        headerLayout->addWidget(iconLabel);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayout->addWidget(messageLabel);
    layout->addLayout(headerLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *yesButton = new QPushButton(tr("Oui"));
    QPushButton *noButton = new QPushButton(tr("Non"));
    yesButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogYesButton));
    noButton->setIcon(dialog.style()->standardIcon(QStyle::SP_DialogNoButton));

    buttonLayout->addStretch();
    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);
    layout->addLayout(buttonLayout);

    int result = false;
    QObject::connect(yesButton, &QPushButton::clicked, [&]() {
        result = true;
        dialog.accept();
    });
    QObject::connect(noButton, &QPushButton::clicked, [&]() {
        result = false;
        dialog.reject();
    });

    int w,h;
    w=fmax(dialog.sizeHint().width(),MinWidth);
    h=fmax(dialog.sizeHint().height(),150);
    dialog.setFixedSize(w,h);//User can't resize the window.

    dialog.exec();

    return result;
}

