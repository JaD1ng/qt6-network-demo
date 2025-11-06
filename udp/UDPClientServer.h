#ifndef UDPCLIENTSERVER_H
#define UDPCLIENTSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <QHostAddress>

/**
 * @brief UDP 客户端/服务器类，基于 Qt 事件循环的单线程异步模型
 * 功能特性：
 * - 绑定本地端口接收数据报
 * - 发送单播消息到指定地址
 * - 发送广播消息到局域网
 * - 自动处理 IPv4/IPv6 地址显示
 *
 * 线程安全：
 * - 此类使用单线程事件驱动模型，不是线程安全的
 * - 所有操作在创建对象的线程中执行
 * - 禁止从多个线程同时调用此类的方法
 * - 如需跨线程通信，请使用 Qt 的信号槽机制（队列连接）
 */
class UDPClientServer : public QObject {
  Q_OBJECT

public:
  explicit UDPClientServer(QObject *parent = nullptr);

  ~UDPClientServer() override;

  // 绑定本地端口（接收消息）
  bool bind(quint16 port);

  // 解绑
  void unbind();

  // 发送单播消息
  void sendMessage(const QString &message, const QString &targetHost, quint16 targetPort);

  // 发送广播消息
  void sendBroadcast(const QString &message, quint16 targetPort);

  // 获取绑定状态
  bool isBound() const;

  // 获取绑定的端口
  quint16 localPort() const;

signals:
  // 绑定成功
  void bound(quint16 port);

  // 解绑
  void unbound();

  // 接收到消息
  void messageReceived(const QString &message, const QString &senderAddress, quint16 senderPort);

  // 错误信息
  void errorOccurred(const QString &error);

private
slots:
  // 处理接收到的数据报
  void onReadyRead();

private:
  QUdpSocket *m_socket;
};

#endif // UDPCLIENTSERVER_H
