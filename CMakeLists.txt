cmake_minimum_required(VERSION 3.16)
project(QGLWater VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Concurrent Gui Network PrintSupport Sql Widgets)

qt_standard_project_setup()

qt_add_executable(QGLWater WIN32 MACOSX_BUNDLE
    ClassDefined.h
    ClockDialog.cpp ClockDialog.h ClockDialog.ui
    CollidingRectItem.cpp CollidingRectItem.h
    CommonFile.h
    CutSolidVerticalLine.cpp CutSolidVerticalLine.h
    DMXLightDialog.cpp DMXLightDialog.h DMXLightDialog.ui
    DominoSetting.cpp DominoSetting.h DominoSetting.ui
    ElecDeviceRectItem.cpp ElecDeviceRectItem.h
    ElecDeviceShowWidget.cpp ElecDeviceShowWidget.h ElecDeviceShowWidget.ui
    LockCode.h
    Model/Track/track.cpp Model/Track/track.h
    MovableVerticalLine.cpp MovableVerticalLine.h
    MusicGraphicsView.cpp MusicGraphicsView.h
    MusicWidget.cpp MusicWidget.h
    OrbitDialog.cpp OrbitDialog.h OrbitDialog.ui
    ProgramControl.cpp ProgramControl.h ProgramControl.ui
    QDeviceInfoDialog.cpp QDeviceInfoDialog.h QDeviceInfoDialog.ui
    QFileHelper.cpp QFileHelper.h
    QOutPutDialog.cpp QOutPutDialog.h QOutPutDialog.ui
    QShowTeamsDialog.cpp QShowTeamsDialog.h QShowTeamsDialog.ui
    QSubTreeView.cpp QSubTreeView.h
    QTreeItem.cpp QTreeItem.h
    QTreeMode.cpp QTreeMode.h
    Refactor/Audio/AudioPlayer.cpp Refactor/Audio/AudioPlayer.h
    Refactor/Audio/FmodAudioPlayer.cpp Refactor/Audio/FmodAudioPlayer.h
    SelectImageDialog.cpp SelectImageDialog.h SelectImageDialog.ui
    StaticValue.cpp StaticValue.h
    addquedialog.cpp addquedialog.h addquedialog.ui
    aosetting.cpp aosetting.h aosetting.ui
    dialoglogindraw.cpp dialoglogindraw.h dialoglogindraw.ui
    drawprodialog.cpp drawprodialog.h drawprodialog.ui
    drawtabledialog.cpp drawtabledialog.h drawtabledialog.ui
    editiddialog.cpp editiddialog.h editiddialog.ui
    globalparams.h
    heartbeat.cpp heartbeat.h heartbeat.ui
    ipadimagesetdialog.cpp ipadimagesetdialog.h ipadimagesetdialog.ui
    lineitem.cpp lineitem.h
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    myitem.cpp myitem.h
    ontimesetdialog.cpp ontimesetdialog.h ontimesetdialog.ui
    ouputViewer.cpp ouputViewer.h ouputViewer.ui
    pingthread.cpp pingthread.h
    playtiming.cpp playtiming.h playtiming.ui
    remotedlg.cpp remotedlg.h remotedlg.ui
    senddatathread.cpp senddatathread.h
    spectrumwidget.cpp spectrumwidget.h spectrumwidget.ui
    vendors/qcustomplot/qcustomplot.cpp vendors/qcustomplot/qcustomplot.h
    winperformance.cpp winperformance.h
)
target_include_directories(QGLWater PRIVATE
    vendors/qcustomplot
)

target_link_libraries(QGLWater PRIVATE
    Qt::Concurrent
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Sql
    Qt::Widgets
    winmm
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(QGLWater PRIVATE
        Qt::Widgets
    )
endif()

if(WIN32)
    target_include_directories(QGLWater PRIVATE
        ext/FMOD/inc
    )

    target_link_directories(QGLWater PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/FMOD/lib/x86
    )

    target_link_libraries(QGLWater PRIVATE
        fmod_vc
    )
endif()

install(TARGETS QGLWater
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET QGLWater
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
