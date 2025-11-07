#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QString>

class IOThreadPool;

/**
 * @brief TCP 服务器类，基于多 Reactor 模式
 *
 * 架构设计：
 * - 主 Reactor（Main Reactor）：TCPServer 运行在主线程
 *   - 负责监听端口和接受新连接
 *   - 使用轮询策略将新连接分配给从 Reactor
 * - 从 Reactor（Sub Reactor）：I/O 线程池
 *   - 每个线程处理部分客户端的 I/O 操作
 *   - 每个线程有独立的事件循环
 *   - 在 I/O 线程中处理业务逻辑
 *
 * 功能特性：
 * - 支持高并发多客户端连接
 * - 自动处理 TCP 黏包和半包问题
 * - 消息格式：[4字节长度(大端)][UTF-8消息内容]
 * - 使用网络字节序（大端）保证跨平台兼容性
 * - 线程池大小可配置，默认基于 CPU 核心数
 *
 * 线程安全：
 * - 此类是线程安全的
 * - 使用信号槽机制实现跨线程通信
 * - 所有信号都使用队列连接
 */
class TCPServer : public QTcpServer {
  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param threadCount I/O 线程池大小，0 表示使用 CPU 核心数
   * @param parent 父对象
   */
  explicit TCPServer(int threadCount = 0, QObject *parent = nullptr);

  ~TCPServer() override;

  // 启动服务器
  bool startServer(quint16 port);

  // 停止服务器
  void stopServer();

  // 发送消息给指定客户端（线程安全）
  void sendMessage(qintptr clientId, const QString &message);

  // 广播消息给所有客户端（线程安全）
  void broadcastMessage(const QString &message);

  // 获取服务器状态
  bool isListening() const;

  // 获取当前连接数
  int clientCount() const;

  // 获取线程池大小
  int threadPoolSize() const;

  signals:
  // 服务器启动成功
  

  void serverStarted(quint16 port);

  // 服务器停止
  void serverStopped();

  // 新客户端连接
  void clientConnected(qintptr clientId, const QString &address);

  // 客户端断开
  void clientDisconnected(qintptr clientId);

  // 接收到消息
  void messageReceived(qintptr clientId, const QString &message);

  // 错误信息
  void errorOccurred(const QString &error);

protected:
  // 重写 QTcpServer 的虚函数，直接处理底层连接
  void incomingConnection(qintptr socketDescriptor) override;

private:
  IOThreadPool *m_threadPool; // I/O 线程池（从 Reactor）
};

#endif // TCPSERVER_H