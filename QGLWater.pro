#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T21:15:13
#
#-------------------------------------------------

QT       += core gui widgets sql network concurrent
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QGLWater
TEMPLATE = app
RC_ICONS = icon1~2.ico

ARCH_NAME = x86
contains(QMAKE_TARGET.arch, x86_64)|contains(QMAKE_TARGET.arch, amd64)|equals(QT_ARCH, "x86_64")|equals(QT_ARCH, "x64") {
    ARCH_NAME = x64
} else:contains(QMAKE_TARGET.arch, x86)|contains(QMAKE_TARGET.arch, i386)|equals(QT_ARCH, "x86")|equals(QT_ARCH, "i386") {
    ARCH_NAME = x86
} else:contains(QMAKE_HOST.arch, x86_64)|contains(QMAKE_HOST.arch, amd64) {
    ARCH_NAME = x64
}

BUILD_ROOT  = $$PWD/build/$$ARCH_NAME

DESTDIR     = $$BUILD_ROOT
MOC_DIR     = $$BUILD_ROOT/objs/moc
RCC_DIR     = $$BUILD_ROOT/objs/rcc
UI_DIR      = $$BUILD_ROOT/objs/ui
OBJECTS_DIR = $$BUILD_ROOT/objs/obj


SOURCES += main.cpp\
        mainwindow.cpp \
    QTreeMode.cpp \
    QTreeItem.cpp \
    QFileHelper.cpp \
    QSubTreeView.cpp \
    QOutPutDialog.cpp \
    QDeviceInfoDialog.cpp \
    QShowTeamsDialog.cpp \
    StaticValue.cpp \
    MusicGraphicsView.cpp \
    myitem.cpp \
    CollidingRectItem.cpp \
    ElecDeviceRectItem.cpp \
    DominoSetting.cpp \
    SelectImageDialog.cpp \
    OrbitDialog.cpp \
    DMXLightDialog.cpp \
    MusicWidget.cpp \
    ProgramControl.cpp \
    ElecDeviceShowWidget.cpp \
    drawtabledialog.cpp \
    ClockDialog.cpp \
    dialoglogindraw.cpp \
    drawprodialog.cpp \
    ipadimagesetdialog.cpp \
    ontimesetdialog.cpp \
    addquedialog.cpp \
    lineitem.cpp \
    aosetting.cpp \
    ouputViewer.cpp \
    remotedlg.cpp \
    playtiming.cpp \
    heartbeat.cpp \
    pingthread.cpp \
    spectrumwidget.cpp \
    vendors/qcustomplot/qcustomplot.cpp \
    Model/Track/track.cpp \
    MovableVerticalLine.cpp \
    CutSolidVerticalLine.cpp \
    senddatathread.cpp \
    winperformance.cpp \
    editiddialog.cpp \
    Refactor/Audio/AudioPlayer.cpp \
    Refactor/Audio/FmodAudioPlayer.cpp

HEADERS  += mainwindow.h \
    QTreeMode.h \
    QTreeItem.h \
    QFileHelper.h \
    QSubTreeView.h \
    QOutPutDialog.h \
    QDeviceInfoDialog.h \
    QShowTeamsDialog.h \
    CommonFile.h \
    StaticValue.h \
    MusicGraphicsView.h \
    myitem.h \
    CollidingRectItem.h \
    ElecDeviceRectItem.h \
    DominoSetting.h \
    SelectImageDialog.h \
    OrbitDialog.h \
    ClassDefined.h \
    DMXLightDialog.h \
    MusicWidget.h \
    ProgramControl.h \
    ElecDeviceShowWidget.h \
    LockCode.h \
    drawtabledialog.h \
    ClockDialog.h \
    dialoglogindraw.h \
    drawprodialog.h \
    ipadimagesetdialog.h \
    ontimesetdialog.h \
    addquedialog.h \
    lineitem.h \
    aosetting.h \
    ouputViewer.h \
    remotedlg.h \
    playtiming.h \
    heartbeat.h \
    pingthread.h \
    spectrumwidget.h \
    vendors/qcustomplot/qcustomplot.h \
    globalparams.h \
    Model/Track/track.h \
    MovableVerticalLine.h \
    CutSolidVerticalLine.h \
    senddatathread.h \
    winperformance.h \
    editiddialog.h \
    Refactor/Audio/AudioPlayer.h \
    Refactor/Audio/FmodAudioPlayer.h

FORMS    += mainwindow.ui \
    QOutPutDialog.ui \
    QDeviceInfoDialog.ui \
    QShowTeamsDialog.ui \
    DominoSetting.ui \
    SelectImageDialog.ui \
    OrbitDialog.ui \
    DMXLightDialog.ui \
    ProgramControl.ui \
    ElecDeviceShowWidget.ui \
    drawtabledialog.ui \
    dialoglogindraw.ui \
    drawprodialog.ui \
    ipadimagesetdialog.ui \
    ontimesetdialog.ui \
    addquedialog.ui \
    aosetting.ui \
    ouputViewer.ui \
    remotedlg.ui \
    playtiming.ui \
    heartbeat.ui \
    spectrumwidget.ui \
    editiddialog.ui \
    ClockDialog.ui

DISTFILES += \
    dark.qss \
    C:/Users/Emma/Desktop/icon1~2.ico

VENDORS_ROOT = $$PWD/vendors
VENDORS_ARCH_NAME = $$ARCH_NAME
FMOD_INC_DIR = $$PWD/ext/FMOD/inc
FMOD_LIB_DIR = $$PWD/ext/FMOD/lib/$$ARCH_NAME

win32 {
    YAML_CPP_DIR = $$VENDORS_ROOT/yaml-cpp/$$VENDORS_ARCH_NAME
    MBEDTLS_DIR = $$VENDORS_ROOT/mbedtls/$$VENDORS_ARCH_NAME

    LIBS += -L$$FMOD_LIB_DIR -lfmod_vc
    INCLUDEPATH += $$FMOD_INC_DIR
    DEPENDPATH += $$FMOD_INC_DIR

    SOURCE_ROOT_REL = $$relative_path($$PWD, $$OUT_PWD)
    isEmpty(SOURCE_ROOT_REL) {
        SOURCE_ROOT_REL = .
    }
    SOURCE_ROOT_REL_NATIVE = $$system_path($$SOURCE_ROOT_REL)

    VENDORS_ROOT_REL = $$relative_path($$VENDORS_ROOT, $$OUT_PWD)
    isEmpty(VENDORS_ROOT_REL) {
        VENDORS_ROOT_REL = .
    }
    VENDORS_ROOT_REL_NATIVE = $$system_path($$VENDORS_ROOT_REL)

    DESTDIR_REL = $$relative_path($$DESTDIR, $$OUT_PWD)
    DESTDIR_REL_NATIVE = $$system_path($$DESTDIR_REL)
    WINDEPLOYQT_EXE = $$[QT_INSTALL_BINS]/windeployqt.exe
    WINDEPLOYQT = $$quote($$system_path($$WINDEPLOYQT_EXE))
    WINDEPLOYQT_ARGS = --compiler-runtime

    QSS_SOURCE_REL = $$SOURCE_ROOT_REL_NATIVE\\dark.qss
    QSS_OUTPUT_FILE_REL = $$DESTDIR_REL_NATIVE\\dark.qss
    CFG_SOURCE_DIR_REL = $$SOURCE_ROOT_REL_NATIVE\\cfgtemp\\*
    CFG_OUTPUT_DIR_REL = $$DESTDIR_REL_NATIVE\\

    YAML_CPP_DLL_REL = $$VENDORS_ROOT_REL_NATIVE\\yaml-cpp\\$$VENDORS_ARCH_NAME\\bin\\yaml-cpp.dll
    MBEDCRYPTO_DLL_REL = $$VENDORS_ROOT_REL_NATIVE\\mbedtls\\$$VENDORS_ARCH_NAME\\lib\\mbedcrypto.dll
    MBEDTLS_DLL_REL = $$VENDORS_ROOT_REL_NATIVE\\mbedtls\\$$VENDORS_ARCH_NAME\\lib\\mbedtls.dll
    MBEDX509_DLL_REL = $$VENDORS_ROOT_REL_NATIVE\\mbedtls\\$$VENDORS_ARCH_NAME\\lib\\mbedx509.dll

    FMOD_DLL_NAME = fmod.dll
    CONFIG(debug, debug|release) {
        FMOD_DLL_NAME = fmodL.dll
    }
    FMOD_DLL_SOURCE = $$FMOD_LIB_DIR/$$FMOD_DLL_NAME
    !exists($$FMOD_DLL_SOURCE) {
        equals(ARCH_NAME, "x64") {
            FMOD_DLL_NAME = fmod64.dll
            CONFIG(debug, debug|release) {
                FMOD_DLL_NAME = fmodL64.dll
            }
        } else {
            FMOD_DLL_NAME = fmod.dll
            CONFIG(debug, debug|release) {
                FMOD_DLL_NAME = fmodL.dll
            }
        }
        FMOD_DLL_SOURCE = $$VENDORS_ROOT/fmod11020/lib/$$FMOD_DLL_NAME
    }
    FMOD_DLL_REL = $$system_path($$relative_path($$FMOD_DLL_SOURCE, $$OUT_PWD))

    QMAKE_POST_LINK += if not exist $$DESTDIR_REL_NATIVE mkdir $$DESTDIR_REL_NATIVE $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += xcopy /D /Y $$QSS_SOURCE_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += xcopy /D /E /I /Y $$CFG_SOURCE_DIR_REL $$CFG_OUTPUT_DIR_REL $$escape_expand(\\n\\t)

    exists($$YAML_CPP_DIR/bin/yaml-cpp.dll) {
        QMAKE_POST_LINK += xcopy /D /Y $$YAML_CPP_DLL_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    }
    exists($$MBEDTLS_DIR/lib/mbedcrypto.dll) {
        QMAKE_POST_LINK += xcopy /D /Y $$MBEDCRYPTO_DLL_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    }
    exists($$MBEDTLS_DIR/lib/mbedtls.dll) {
        QMAKE_POST_LINK += xcopy /D /Y $$MBEDTLS_DLL_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    }
    exists($$MBEDTLS_DIR/lib/mbedx509.dll) {
        QMAKE_POST_LINK += xcopy /D /Y $$MBEDX509_DLL_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    }
    exists($$FMOD_DLL_SOURCE) {
        QMAKE_POST_LINK += xcopy /D /Y $$FMOD_DLL_REL $$DESTDIR_REL_NATIVE\\ $$escape_expand(\\n\\t)
    }
    exists($$WINDEPLOYQT_EXE) {
        QMAKE_POST_LINK += $$WINDEPLOYQT $$WINDEPLOYQT_ARGS $$quote($$DESTDIR_REL_NATIVE\\$${TARGET}.exe) $$escape_expand(\\n\\t)
    }
}

INCLUDEPATH += $$PWD/vendors/qcustomplot
DEPENDPATH += $$PWD/vendors/qcustomplot
LIBS += -lwinmm
