cmake_minimum_required(VERSION 3.16)
project(tcp_udp_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/tools/Qt/6.9.3/llvm-mingw_64")

# 查找 Qt6 组件：Core, Gui, Qml, Quick, Network, QuickControls2
find_package(Qt6 COMPONENTS
        Core
        Gui
        Qml
        Quick
        QuickControls2
        Network
        REQUIRED)

# 设置 Qt6 QML 策略
qt_policy(SET QTP0001 NEW)  # 使用新的 ':/qt/qml/' 资源前缀
qt_policy(SET QTP0004 NEW)  # 允许子目录中的 QML 文件不需要单独的 qmldir

# 添加子目录
add_subdirectory(tcp)
add_subdirectory(udp)
add_subdirectory(controllers)

# 主可执行文件（QML 应用程序）
qt_add_executable(tcp_udp_demo
        main.cpp
        resources.qrc
)

# 使用 qt_add_qml_module 添加 QML 模块
qt_add_qml_module(tcp_udp_demo
        URI NetworkDemo
        VERSION 1.0
        QML_FILES
        qml/main.qml
        qml/TCPServerPage.qml
        qml/TCPClientPage.qml
        qml/UDPPage.qml
        RESOURCES
        icon.png
        icon.ico
)

target_link_libraries(tcp_udp_demo PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        Qt::Network
        controllers_module
)

# 定义 Qt QML 模块路径
get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
get_filename_component(QT_INSTALL_DIR "${QT_BIN_DIR}" DIRECTORY)
set(QT_QML_DIR "${QT_INSTALL_DIR}/qml")

target_compile_definitions(tcp_udp_demo PRIVATE
        QT_QML_IMPORT_PATH="${QT_QML_DIR}"
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
    foreach (QT_LIB Core Gui Qml Quick QuickControls2 Network)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
