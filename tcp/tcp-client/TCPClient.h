#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QTimer>
#include <QByteArray>
#include <QDataStream>

/**
 * @brief TCP 客户端类，基于 Qt 事件循环的单线程异步模型
 *
 * 功能特性：
 * - 连接到 TCP 服务器
 * - 自动处理 TCP 黏包和半包问题
 * - 消息格式：[4字节长度(大端)][UTF-8消息内容]
 * - 使用网络字节序（大端）保证跨平台兼容性
 * - 支持自动重连机制
 *
 * 线程安全：
 * - 此类使用单线程事件驱动模型，不是线程安全的
 * - 所有操作在创建对象的线程中执行
 * - 禁止从多个线程同时调用此类的方法
 * - 如需跨线程通信，请使用 Qt 的信号槽机制（队列连接）
 */
class TCPClient : public QObject {
  Q_OBJECT

public:
  explicit TCPClient(QObject *parent = nullptr);

  ~TCPClient() override;

  // 连接到服务器
  void connectToServer(const QString &host, quint16 port);

  // 断开连接
  void disconnectFromServer();

  // 发送消息（自动处理黏包和大小端）
  void sendMessage(const QString &message);

  // 获取连接状态
  bool isConnected() const;

  // 启用/禁用自动重连
  void setAutoReconnect(bool enable);

  // 设置重连间隔（毫秒）
  void setReconnectInterval(int msec);

private:
  // 打包消息：[4字节长度(大端)][消息内容]
  static QByteArray packMessage(const QString &message);

  // 解析接收到的数据，处理黏包和半包
  void parseReceivedData();

signals:
  // 连接成功
  void connected();

  // 断开连接
  void disconnected();

  // 接收到消息
  void messageReceived(const QString &message);

  // 错误信息
  void errorOccurred(const QString &error);

  // 正在重连
  void reconnecting();

private
slots:
  // 处理连接成功
  void onConnected();

  // 处理断开连接
  void onDisconnected();

  // 处理接收数据
  void onReadyRead();

  // 处理错误
  void onError(QAbstractSocket::SocketError socketError);

  // 尝试重连
  void attemptReconnect();

private:
  QTcpSocket *m_socket;
  QTimer *m_reconnectTimer;
  QByteArray m_receiveBuffer; // 接收缓冲区，处理半包

  QString m_host;
  quint16 m_port;

  bool m_autoReconnect;
  int m_reconnectInterval;
  bool m_isManualDisconnect; // 标记是否为手动断开
};

#endif // TCPCLIENT_H
