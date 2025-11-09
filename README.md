# Qt Network Demo

[![Qt Version](https://img.shields.io/badge/Qt-6.9.3-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/yourusername/qt-network-reactor)

> 基于 Qt6 和 QML 的高性能网络通信演示项目，实现了多 Reactor 模式的 TCP 服务器和异步 UDP 通信

### 架构设计

- **多 Reactor 模式 TCP 服务器**
    - 主 Reactor：监听连接并分配到工作线程
    - 从 Reactor：I/O 线程池，每个线程独立事件循环
    - Round Robin 负载均衡策略
    - 自动处理 TCP 黏包和半包问题

- **异步 TCP 客户端**
    - 单线程事件驱动模型
    - 自动重连机制

- **UDP 单播/广播**
    - 支持 IPv4/IPv6
    - 单播和广播消息发送
    - 异步接收处理

### 消息协议格式

| 字段 | 长度 | 类型 | 说明 |
|------|------|------|------|
| 消息长度 | 4 字节 | uint32_t | 大端字节序（网络字节序） |
| 消息内容 | N 字节 | UTF-8 字符串 | N = 消息长度字段的值 |

```
发送 "Hello"
字节序列：0x00 0x00 0x00 0x05  H  e  l  l  o
          └─────长度=5─────┘  └───消息内容───┘
```

### 线程池大小

默认使用 CPU 核心数，可在创建 `TCPServer` 时自定义：

```cpp
TCPServer *server = new TCPServer(4);  // 使用 4 个 I/O 线程
```

### 自动重连间隔

```cpp
TCPClient *client = new TCPClient();
client->setAutoReconnect(true);
client->setReconnectInterval(3000);  // 3 秒
```

### UDP 数据报最大大小

```cpp
// UDPClientServer.cpp:5
constexpr qint64 MAX_UDP_DATAGRAM_SIZE = 1472;  // 避免 IP 分片
```
