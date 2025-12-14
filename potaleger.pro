QT += core gui sql svg charts

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

SOURCES += \
    Dialogs.cpp \
    PotaUtils.cpp \
    Structure.cpp \
    classes/fdasqlformat.cpp \
    classes/zoomgraphicsview.cpp \
    data/Data.cpp \
    main.cpp \
    mainwindow.cpp \
    muParser/muParser.cpp \
    muParser/muParserBase.cpp \
    muParser/muParserBytecode.cpp \
    muParser/muParserCallback.cpp \
    muParser/muParserError.cpp \
    muParser/muParserInt.cpp \
    muParser/muParserTokenReader.cpp \
    potagraph.cpp \
    potawidget.cpp

HEADERS += \
    Dialogs.h \
    PotaUtils.h \
    classes/fdasqlformat.h \
    classes/zoomgraphicsview.h \
    data/Data.h \
    mainwindow.h \
    muParser/muParser.h \
    muParser/muParserBase.h \
    muParser/muParserBytecode.h \
    muParser/muParserCallback.h \
    muParser/muParserDef.h \
    muParser/muParserError.h \
    muParser/muParserFixes.h \
    muParser/muParserInt.h \
    muParser/muParserTemplateMagic.h \
    muParser/muParserToken.h \
    muParser/muParserTokenReader.h \
    potagraph.h \
    potawidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path=/tmp/$${TARGET}/bin
else: unix:!android: target.path=/opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    install/readme.md \
    zNoRepo/todo.txt

RESOURCES += \
    SQL/sql.qrc \
    images/images.qrc

RC_ICONS=images/potaleger.ico

#Pour que muParser ne provoque pas une erreur de compil sous windows.
DEFINES -= MUPARSER_DLL
DEFINES += MUPARSER_STATIC
QMAKE_CXXFLAGS += -DMUPARSER_STATIC
