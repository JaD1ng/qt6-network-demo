#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QString>
#include <QByteArray>
#include <QDataStream>

/**
 * @brief TCP 服务器类，基于 Qt 事件循环的单线程异步模型
 *
 * 功能特性：
 * - 支持多客户端并发连接
 * - 自动处理 TCP 黏包和半包问题
 * - 消息格式：[4字节长度(大端)][UTF-8消息内容]
 * - 使用网络字节序（大端）保证跨平台兼容性
 *
 * 线程安全：
 * - 此类使用单线程事件驱动模型，不是线程安全的。
 * - 所有客户端在同一线程中处理
 * - 禁止从多个线程同时调用此类的方法
 * - 如需跨线程通信，请使用 Qt 的信号槽机制（队列连接）
 */
class TCPServer : public QObject {
  Q_OBJECT

public:
  explicit TCPServer(QObject *parent = nullptr);

  ~TCPServer() override;

  // 启动服务器
  bool startServer(quint16 port);

  // 停止服务器
  void stopServer();

  // 发送消息给指定客户端（处理黏包和大小端）
  void sendMessage(qintptr clientId, const QString &message);

  // 广播消息给所有客户端（处理黏包和大小端）
  void broadcastMessage(const QString &message);

  // 获取服务器状态
  bool isListening() const;

  // 获取当前连接数
  int clientCount() const;

private:
  // 打包消息：[4字节长度(大端)][消息内容]
  static QByteArray packMessage(const QString &message);

  // 解析接收到的数据，处理黏包和半包
  void parseReceivedData(QTcpSocket *socket);

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

private
slots:
  // 处理新连接
  void onNewConnection();

  // 处理客户端数据
  void onClientReadyRead();

  // 处理客户端断开
  void onClientDisconnected();

private:
  QTcpServer *m_server;
  QHash<qintptr, QTcpSocket *> m_clients; // 客户端映射表
  QHash<qintptr, QByteArray> m_receiveBuffers; // 每个客户端的接收缓冲区，处理半包
};

#endif // TCPSERVER_H
