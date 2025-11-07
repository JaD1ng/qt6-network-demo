#ifndef TCPCLIENTWORKER_H
#define TCPCLIENTWORKER_H

#include <QObject>
#include <QThread>
#include <QString>

class TCPClient;

/**
 * @brief TCP 客户端工作线程封装类
 *
 * 功能特性：
 * - 封装 TCPClient，在独立线程中运行
 * - 提供线程安全的接口
 * - 不阻塞 GUI 主线程
 * - API 与 TCPClient 完全兼容
 *
 * 线程安全：
 * - 所有公共方法都是线程安全的
 * - 使用队列连接实现跨线程通信
 * - 可以从任意线程调用
 */
class TCPClientWorker : public QObject {
  Q_OBJECT

public:
  explicit TCPClientWorker(QObject *parent = nullptr);

  ~TCPClientWorker() override;

  // 连接到服务器（线程安全）
  void connectToServer(const QString &host, quint16 port);

  // 断开连接（线程安全）
  void disconnectFromServer();

  // 发送消息（线程安全）
  void sendMessage(const QString &message);

  // 获取连接状态（线程安全）
  bool isConnected() const;

  // 启用/禁用自动重连（线程安全）
  void setAutoReconnect(bool enable);

  // 设置重连间隔（毫秒）（线程安全）
  void setReconnectInterval(int msec);

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

private slots:
  // 初始化工作线程中的 TCPClient
  void initializeClient();

private:
  QThread *m_workerThread; // 工作线程
  TCPClient *m_client; // TCP 客户端（在工作线程中）
};

#endif // TCPCLIENTWORKER_H
