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
    FdaGraph.cpp \
    FdaSqlFormat.cpp \
    FdaUtils.cpp \
    FdaWidget.cpp \
    Structure.cpp \
    classes/zoomgraphicsview.cpp \
    data/FdaCalls.cpp \
    main.cpp \
    mainwindow.cpp \
    muParser/mpError.cpp \
    muParser/mpFuncCmplx.cpp \
    muParser/mpFuncCommon.cpp \
    muParser/mpFuncMatrix.cpp \
    muParser/mpFuncNonCmplx.cpp \
    muParser/mpFuncStr.cpp \
    muParser/mpICallback.cpp \
    muParser/mpIOprt.cpp \
    muParser/mpIOprtBinShortcut.cpp \
    muParser/mpIPackage.cpp \
    muParser/mpIToken.cpp \
    muParser/mpIValReader.cpp \
    muParser/mpIValue.cpp \
    muParser/mpIfThenElse.cpp \
    muParser/mpOprtBinAssign.cpp \
    muParser/mpOprtBinCommon.cpp \
    muParser/mpOprtBinShortcut.cpp \
    muParser/mpOprtCmplx.cpp \
    muParser/mpOprtIndex.cpp \
    muParser/mpOprtMatrix.cpp \
    muParser/mpOprtNonCmplx.cpp \
    muParser/mpOprtPostfixCommon.cpp \
    muParser/mpPackageCmplx.cpp \
    muParser/mpPackageCommon.cpp \
    muParser/mpPackageMatrix.cpp \
    muParser/mpPackageNonCmplx.cpp \
    muParser/mpPackageStr.cpp \
    muParser/mpPackageUnit.cpp \
    muParser/mpParser.cpp \
    muParser/mpParserBase.cpp \
    muParser/mpParserMessageProvider.cpp \
    muParser/mpRPN.cpp \
    muParser/mpScriptTokens.cpp \
    muParser/mpTest.cpp \
    muParser/mpTokenReader.cpp \
    muParser/mpValReader.cpp \
    muParser/mpValue.cpp \
    muParser/mpValueCache.cpp \
    muParser/mpVariable.cpp \
    script/ScriptEditor.cpp \
    script/fadahighlighter.cpp \
    script/fadascriptedit.cpp \
    script/fadascriptengine2.cpp

HEADERS += \
    Dialogs.h \
    FdaGraph.h \
    FdaSqlFormat.h \
    FdaUtils.h \
    FdaWidget.h \
    classes/zoomgraphicsview.h \
    data/FdaCalls.h \
    mainwindow.h \
    muParser/mpDefines.h \
    muParser/mpError.h \
    muParser/mpFuncCmplx.h \
    muParser/mpFuncCommon.h \
    muParser/mpFuncMatrix.h \
    muParser/mpFuncNonCmplx.h \
    muParser/mpFuncStr.h \
    muParser/mpFwdDecl.h \
    muParser/mpICallback.h \
    muParser/mpIOprt.h \
    muParser/mpIOprtBinShortcut.h \
    muParser/mpIPackage.h \
    muParser/mpIPrecedence.h \
    muParser/mpIToken.h \
    muParser/mpIValReader.h \
    muParser/mpIValue.h \
    muParser/mpIfThenElse.h \
    muParser/mpMatrix.h \
    muParser/mpMatrixError.h \
    muParser/mpOprtBinAssign.h \
    muParser/mpOprtBinCommon.h \
    muParser/mpOprtBinShortcut.h \
    muParser/mpOprtCmplx.h \
    muParser/mpOprtIndex.h \
    muParser/mpOprtMatrix.h \
    muParser/mpOprtNonCmplx.h \
    muParser/mpOprtPostfixCommon.h \
    muParser/mpPackageCmplx.h \
    muParser/mpPackageCommon.h \
    muParser/mpPackageMatrix.h \
    muParser/mpPackageNonCmplx.h \
    muParser/mpPackageStr.h \
    muParser/mpPackageUnit.h \
    muParser/mpParser.h \
    muParser/mpParserBase.h \
    muParser/mpParserMessageProvider.h \
    muParser/mpRPN.h \
    muParser/mpScriptTokens.h \
    muParser/mpStack.h \
    muParser/mpStringConversionHelper.h \
    muParser/mpTest.h \
    muParser/mpTokenReader.h \
    muParser/mpTypes.h \
    muParser/mpValReader.h \
    muParser/mpValue.h \
    muParser/mpValueCache.h \
    muParser/mpVariable.h \
    muParser/suSortPred.h \
    muParser/suStringTokens.h \
    muParser/utGeneric.h \
    script/ScriptEditor.h \
    script/fadahighlighter.h \
    script/fadascriptedit.h \
    script/fadascriptengine2.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path=/tmp/$${TARGET}/bin
else: unix:!android: target.path=/opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    install/readme.md \
    license.md \
    zNoRepo/todo.txt

RESOURCES += \
    SQL/sql.qrc \
    images/images.qrc \
    md/md.qrc \
    model/model.qrc

RC_ICONS=images/potaleger.ico

#Pour que muParser ne provoque pas une erreur de compil sous windows.
DEFINES -= MUPARSER_DLL
DEFINES += MUPARSER_STATIC
QMAKE_CXXFLAGS += -DMUPARSER_STATIC
