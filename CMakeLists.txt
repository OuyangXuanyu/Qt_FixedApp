cmake_minimum_required(VERSION 3.22)
project(Qt_Project_FixedApp VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 安装根目录。这里仅提供本机默认值，仍可通过
# cmake -DQT_ROOT=/path/to/Qt/6.x.x/<kit> 覆盖。
if (APPLE)
    set(_qt_default_root "/Volumes/Mac_ExDisk/Qt/Qt_Source/6.10.2/macos")
elseif (WIN32)
    set(_qt_default_root "C:/Qt/6.10.2/msvc2022_64")
else ()
    set(_qt_default_root "")
endif ()
set(QT_ROOT "${_qt_default_root}" CACHE PATH "Qt kit installation prefix")
unset(_qt_default_root)

if (QT_ROOT)
    if (NOT EXISTS "${QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
        message(FATAL_ERROR
                "QT_ROOT does not point to a Qt 6 kit: ${QT_ROOT}\n"
                "Set -DQT_ROOT=/path/to/Qt/6.x.x/<kit>.")
    endif ()
    list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
endif ()

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

if (APPLE)
    set(APP_EXECUTABLE_TYPE MACOSX_BUNDLE)
else ()
    set(APP_EXECUTABLE_TYPE)
endif ()

qt_add_executable(Qt_Project_FixedApp ${APP_EXECUTABLE_TYPE} src/main.cpp

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
#        src/tools/diy_bl/diy_bl_cursor.cpp
#        src/tools/diy_bl/diy_bl_cursor.h

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
unset(APP_EXECUTABLE_TYPE)

if (APPLE)
    set_target_properties(Qt_Project_FixedApp PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "Qt Project FixedApp"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.fixedapp.qt-project"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/Info.plist.in"
    )
endif ()

target_link_libraries(Qt_Project_FixedApp
        PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::SerialPort
        Qt6::Bluetooth
        Qt6::Network
        OpenGL::GL
        $<$<BOOL:${WIN32}>:windowsapp>
)

if (APPLE)
    # 安装时生成可独立分发的 .app，并将 Qt frameworks/plugins 放入 bundle。
    install(TARGETS Qt_Project_FixedApp
            BUNDLE DESTINATION .
            RUNTIME DESTINATION bin)
    qt_generate_deploy_app_script(
            TARGET Qt_Project_FixedApp
            OUTPUT_SCRIPT qt_deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT "${qt_deploy_script}")
endif ()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${QT_ROOT}")
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
    foreach (QT_LIB Core Gui Widgets OpenGLWidgets OpenGL Bluetooth SerialPort Network)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif()
