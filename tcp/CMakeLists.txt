# UDP 模块的 CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 收集所有 TCP 源文件和头文件
set(TCP_SOURCES
        tcp-client/TCPClient.cpp
        tcp-client/TCPClient.h
        tcp-server/TCPServer.cpp
        tcp-server/TCPServer.h
)

# 创建 TCP 模块库
add_library(tcp_module STATIC ${TCP_SOURCES})

# 设置包含目录
target_include_directories(tcp_module PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp-client
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp-server
)

# 链接 Qt6 网络模块
target_link_libraries(tcp_module PUBLIC
        Qt::Core
        Qt::Network
)