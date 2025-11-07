#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <QThread>
#include <QHash>
#include <atomic>

class ClientHandler;

/**
 * @brief I/O 工作线程类，作为从 Reactor
 *
 * 功能特性：
 * - 每个线程有独立的事件循环
 * - 管理分配给该线程的所有客户端连接
 * - 处理客户端的 I/O 操作和业务逻辑
 * - 线程安全的客户端添加和移除
 *
 * 生命周期：
 * - 由 IOThreadPool 创建和管理
 * - 线程启动后持续运行直到被停止
 */
class IOThread : public QThread {
  Q_OBJECT

public:
  explicit IOThread(int threadId, QObject *parent = nullptr);

  ~IOThread() override;

  // 获取线程 ID
  int threadId() const { return m_threadId; }

  // 获取当前管理的客户端数量（线程安全）
  int clientCount() const { return m_clientCount.load(std::memory_order_acquire); }

  // 添加客户端到此线程（线程安全）
  void addClient(qintptr socketDescriptor);

  // 发送消息给指定客户端（线程安全）
  void sendMessageToClient(qintptr clientId, const QString &message);

  // 断开指定客户端（线程安全）
  void disconnectClient(qintptr clientId);

signals:
  // 客户端就绪
  void clientReady(qintptr clientId, const QString &address);

  // 接收到消息
  void messageReceived(qintptr clientId, const QString &message);

  // 客户端断开
  void clientDisconnected(qintptr clientId);

  // 错误发生
  void errorOccurred(qintptr clientId, const QString &error);

  // 内部信号：添加客户端（队列连接）
  void doAddClient(qintptr socketDescriptor);

  // 内部信号：发送消息（队列连接）
  void doSendMessage(qintptr clientId, QString message);

  // 内部信号：断开连接（队列连接）
  void doDisconnect(qintptr clientId);

protected:
  void run() override;

private
slots:
  // 处理客户端添加（在本线程中执行）
  void handleAddClient(qintptr socketDescriptor);

  // 处理客户端就绪
  void handleClientReady(qintptr clientId, const QString &address);

  // 处理客户端断开（在本线程中执行）
  void handleClientDisconnected(qintptr clientId);

  // 处理发送消息（在本线程中执行）
  void handleSendMessage(qintptr clientId, const QString &message);

  // 处理断开连接（在本线程中执行）
  void handleDisconnect(qintptr clientId);

private:
  int m_threadId; // 线程 ID
  QHash<qintptr, ClientHandler *> m_clientHandlers; // 客户端处理器映射
  std::atomic<int> m_clientCount; // 客户端数量（原子变量）
};

#endif // IOTHREAD_H
