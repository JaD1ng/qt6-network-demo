#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTcpSocket>

/**
 * @brief 客户端连接处理类，运行在独立的 I/O 线程中
 *
 * 功能特性：
 * - 在独立线程中处理单个客户端的所有 I/O 操作
 * - 自动处理 TCP 黏包和半包问题
 * - 消息格式：[4字节长度(大端)][UTF-8消息内容]
 * - 线程安全的信号槽通信
 *
 * 生命周期：
 * - 在 I/O 线程中创建和销毁
 * - 通过队列连接的信号与主线程通信
 */
class ClientHandler : public QObject {
  Q_OBJECT

public:
  explicit ClientHandler(qintptr socketDescriptor, QObject *parent = nullptr);

  ~ClientHandler() override;

  // 获取客户端 ID
  qintptr clientId() const { return m_socketDescriptor; }

  // 获取客户端地址
  QString clientAddress() const { return m_clientAddress; }

public slots:
  // 发送消息（线程安全，通过队列连接调用）
  void sendMessage(const QString &message);

  // 初始化连接（在目标线程中调用）
  void initialize();

  // 断开连接
  void disconnect();

signals:
  // 连接就绪（连接成功后发出）
  void ready(qintptr clientId, const QString &address);

  // 接收到消息
  void messageReceived(qintptr clientId, const QString &message);

  // 连接断开
  void disconnected(qintptr clientId);

  // 错误发生
  void errorOccurred(qintptr clientId, const QString &error);

private slots:
  // 处理接收数据
  void onReadyRead();

  // 处理断开连接
  void onDisconnected();

  // 处理错误
  void onError(QAbstractSocket::SocketError socketError);

private:
  // 打包消息：[4字节长度(大端)][消息内容]
  static QByteArray packMessage(const QString &message);

  // 解析接收到的数据，处理黏包和半包
  void parseReceivedData();

  qintptr m_socketDescriptor; // Socket 描述符
  QTcpSocket *m_socket;       // TCP Socket（在目标线程中创建）
  QByteArray m_receiveBuffer; // 接收缓冲区
  QString m_clientAddress;    // 客户端地址缓存
};

#endif // CLIENTHANDLER_H
