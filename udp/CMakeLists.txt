# UDP 模块的 CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 收集所有 UDP 源文件和头文件
set(UDP_SOURCES
        UDPClientServer.cpp
        UDPClientServer.h
)

# 创建 UDP 模块库
add_library(udp_module STATIC ${UDP_SOURCES})

# 设置包含目录
target_include_directories(udp_module PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接 Qt6 网络模块
target_link_libraries(udp_module PUBLIC
        Qt::Core
        Qt::Network
)