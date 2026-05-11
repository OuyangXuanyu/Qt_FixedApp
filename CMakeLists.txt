cmake_minimum_required(VERSION 3.31)
project(Qt_Project_FixedApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")

find_package(OpenGL REQUIRED)
find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        OpenGLWidgets
        SerialPort
        Bluetooth
        Network
        REQUIRED)

add_executable(Qt_Project_FixedApp src/main.cpp

        # 窗口总代理
        src/ui/manager_of_ui.cpp
        src/ui/manager_of_ui.h

        # 各个窗口
        src/ui/login_dialog/login_dialog.cpp
        src/ui/login_dialog/login_dialog.h
        src/ui/login_dialog/login_dialog.ui

        src/ui/mainwindow/mainwindow.cpp
        src/ui/mainwindow/mainwindow.h
        src/ui/mainwindow/mainwindow.ui

        src/ui/mainwindow_addpatient/mainwindow_addpatient.cpp
        src/ui/mainwindow_addpatient/mainwindow_addpatient.h
        src/ui/mainwindow_addpatient/mainwindow_addpatient.ui

        src/ui/mainwindow_searchpatient/mainwindow_searchpatient.cpp
        src/ui/mainwindow_searchpatient/mainwindow_searchpatient.h
        src/ui/mainwindow_searchpatient/mainwindow_searchpatient.ui

        src/ui/mainwindow_playbackdata/mainwindow_playbackdata.cpp
        src/ui/mainwindow_playbackdata/mainwindow_playbackdata.h
        src/ui/mainwindow_playbackdata/mainwindow_playbackdata.ui

        # 第三方库
        src/third_src/sqlite3/sqlite3.c
        src/third_src/sqlite3/sqlite3.h

        # 自定义工具库
        src/tools/diy_database/patient_database.cpp
        src/tools/diy_database/patient_database.h
        src/tools/diy_database/user_database.cpp
        src/tools/diy_database/user_database.h

        src/tools/diy_encryption/diy_encryption.cpp
        src/tools/diy_encryption/diy_encryption.h

        src/tools/diy_serial/diy_serial.cpp
        src/tools/diy_serial/diy_serial.h

        src/tools/diy_oscilloscope-plot/diy_oscilloscope-plot.cpp
        src/tools/diy_oscilloscope-plot/diy_oscilloscope-plot.h

        src/tools/diy_bl/diy_bl.cpp
        src/tools/diy_bl/diy_bl.h

        src/tools/diy_start-test/diy_start-test.cpp
        src/tools/diy_start-test/diy_start-test.h

        src/tools/diy_playback-plot/diy_playback-plot.cpp
        src/tools/diy_playback-plot/diy_playback-plot.h

        src/tools/diy_ui_toast-notification/diy_ui_toast-notification.cpp
        src/tools/diy_ui_toast-notification/diy_ui_toast-notification.h

        src/ui/mainwindow/serial_ui/mainwindow_serial.cpp
        src/ui/mainwindow/serial_ui/mainwindow_serial.h
        src/ui/mainwindow/serial_ui/mainwindow_serial.ui

        src/ui/mainwindow/bl_ui/mainwindow_bl.cpp
        src/ui/mainwindow/bl_ui/mainwindow_bl.h
        src/ui/mainwindow/bl_ui/mainwindow_bl.ui

        src/ui/mainwindow/bottom_menu/mainwindow_bottom_menu.cpp
        src/ui/mainwindow/bottom_menu/mainwindow_bottom_menu.h
        src/ui/mainwindow/bottom_menu/mainwindow_bottom_menu.ui
)
target_link_libraries(Qt_Project_FixedApp
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
        Qt::SerialPort
        Qt::Bluetooth
        Qt::Network
        OpenGL::GL
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets OpenGLWidgets OpenGL SerialPort Bluetooth Network)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif()

