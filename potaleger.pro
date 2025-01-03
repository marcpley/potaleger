QT       += core gui sql svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 sql

#LIBS += -lsqlite3
# Inclure le dossier contenant sqlite3.h 3.47.2
INCLUDEPATH += $$PWD/libs/sqlite
# Lier la bibliothèque statique 3.47.2
LIBS += $$PWD/libs/sqlite/libsqlite3.a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Dialogs.cpp \
    PotaUtils.cpp \
    SQL/FonctionsSQLite.cpp \
    SQL/TestSQLite.cpp \
    Structure.cpp \
    data/Data.cpp \
    main.cpp \
    mainwindow.cpp \
    potawidget.cpp \
    sqlean/eval.c \
    sqlean/manage.c

HEADERS += \
    PotaUtils.h \
    data/Data.h \
    mainwindow.h \
    potawidget.h \
    sqlean/define.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    SQL/CreateBaseData.sql \
    SQL/CreateTables.sql \
    SQL/CreateTriggers.sql \
    SQL/CreateViews.sql \
    SQL/UpdateStru2024-12-30_2025-01-xx.sql

RESOURCES += \
    images/images.qrc
