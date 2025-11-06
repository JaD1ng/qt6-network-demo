# Controllers 模块的 CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 收集所有 Controller 源文件和头文件
set(CONTROLLER_SOURCES
        TCPServerController.cpp
        TCPServerController.h
        TCPClientController.cpp
        TCPClientController.h
        UDPController.cpp
        UDPController.h
)

# 创建 Controller 模块库
add_library(controllers_module STATIC ${CONTROLLER_SOURCES})

# 设置包含目录（PUBLIC 使得依赖此库的目标也能访问这些目录）
target_include_directories(controllers_module PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 TCP 和 UDP 模块
target_link_libraries(controllers_module PUBLIC
        Qt::Core
        Qt::Qml
        tcp_module
        udp_module
)
