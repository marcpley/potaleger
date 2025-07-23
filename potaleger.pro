QT       += core gui sql svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 sql
unix: {
    LIBS += -ldl
}
# Do not use Qt SQLite (3.45)
#LIBS += -lsqlite3

# Use a home compiled shared lib (3.47.2)
#INCLUDEPATH += $$PWD/libs/sqlite
#LIBS += $$PWD/libs/sqlite/libsqlite3.a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += MUP_USE_WCHAR=0 #Pour que muParser ne provoque pas une erreur de compil sous windows.

SOURCES += \
    Dialogs.cpp \
    PotaUtils.cpp \
    SQL/FunctionsSQLite.cpp \
    Structure.cpp \
    classes/zoomgraphicsview.cpp \
    data/Data.cpp \
    main.cpp \
    mainwindow.cpp \
    muParser/muParser.cpp \
    muParser/muParserBase.cpp \
    muParser/muParserBytecode.cpp \
    muParser/muParserCallback.cpp \
    muParser/muParserDLL.cpp \
    muParser/muParserError.cpp \
    muParser/muParserInt.cpp \
    muParser/muParserTest.cpp \
    muParser/muParserTokenReader.cpp \
    potawidget.cpp \
    sqlean/eval.c \
    sqlean/manage.c \
    sqlean/module.c \
    sqlite/sqlite3.c

HEADERS += \
    Dialogs.h \
    PotaUtils.h \
    SQL/FunctionsSQLite.h \
    classes/zoomgraphicsview.h \
    data/Data.h \
    mainwindow.h \
    muParser/muParser.h \
    muParser/muParserBase.h \
    muParser/muParserBytecode.h \
    muParser/muParserCallback.h \
    muParser/muParserDLL.h \
    muParser/muParserDef.h \
    muParser/muParserError.h \
    muParser/muParserFixes.h \
    muParser/muParserInt.h \
    muParser/muParserTemplateMagic.h \
    muParser/muParserToken.h \
    muParser/muParserTokenReader.h \
    potawidget.h \
    sqlean/define.h \
    sqlite/sqlite3.h

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
    SQL/FunctionsSQLite.sql \
    SQL/UpdateBaseData.sql \
    SQL/UpdateStru.sql \
    SQL/UpdateTableParams.sql \
    install/readme.md \
    zNoRepo/todo.txt

RESOURCES += \
    images/images.qrc

RC_ICONS = images/potaleger.ico

# librairie sqlean (à la place des fichiers de Pawel ?)
# win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sqlean.so/release/ -ldefine
# else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sqlean.so/debug/ -ldefine
# else:unix: LIBS += -L$$PWD/sqlean.so/ -ldefine

# INCLUDEPATH += $$PWD/sqlean.so
# DEPENDPATH += $$PWD/sqlean.so
