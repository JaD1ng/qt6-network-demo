cmake_minimum_required(VERSION 3.16)
project(tcp_udp_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "D:/tools/Qt/6.9.3/msvc2022_64")

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

# 为 Windows 可执行文件设置图标
if (WIN32)
    set(APP_ICON_RESOURCE icon.ico)
endif()

# 主可执行文件
qt_add_executable(tcp_udp_demo
        main.cpp
        resources.qrc
        ${APP_ICON_RESOURCE}
)

# 设置可执行文件输出目录
set_target_properties(tcp_udp_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 添加 QML 模块
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

# Windows 平台部署：复制运行时依赖
if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # 查找 Qt 安装路径
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()

    # 复制 LLVM-MinGW 运行时库（编译器依赖，windeployqt 不会复制）
    foreach (RUNTIME_LIB libc++ libunwind)
        if (EXISTS "${QT_INSTALL_PATH}/bin/${RUNTIME_LIB}.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/bin/${RUNTIME_LIB}.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endif ()
    endforeach (RUNTIME_LIB)

#    # 使用 windeployqt6 自动部署
#    find_program(WINDEPLOYQT_EXECUTABLE
#            NAMES windeployqt6 windeployqt
#            HINTS "${QT_INSTALL_PATH}/bin")
#    if (WINDEPLOYQT_EXECUTABLE)
#        # windeployqt 会自动检测 Debug/Release 模式
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
#                --qmldir "${CMAKE_SOURCE_DIR}/qml"
#                --no-translations
#                "$<TARGET_FILE:${PROJECT_NAME}>"
#                COMMENT "Running windeployqt to deploy Qt dependencies...")
#    else ()
#        message(WARNING "windeployqt not found. Application may not run standalone.")
#    endif ()
endif ()
