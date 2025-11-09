#ifndef IOTHREADWORKER_H
#define IOTHREADWORKER_H

#include <QObject>
#include <QHash>
#include <atomic>

class ClientHandler;

/**
 * @brief I/O 工作对象，运行在独立线程中
 *
 * 设计模式：
 * QThread 只负责提供事件循环
 * Worker 负责业务逻辑
 *
 * 功能特性：
 * - 管理分配给该线程的所有客户端连接
 * - 处理客户端的 I/O 操作和业务逻辑
 * - 线程安全的客户端添加和移除
 *
 * 生命周期：
 * - 在主线程创建，moveToThread 到工作线程
 * - 由 IOThreadPool 管理
 */
class IOThreadWorker : public QObject {
  Q_OBJECT

public:
  explicit IOThreadWorker(int threadId, QObject *parent = nullptr);

  ~IOThreadWorker() override;

  // 获取线程 ID
  int threadId() const { return m_threadId; }

  // 获取当前管理的客户端数量（线程安全）
  int clientCount() const { return m_clientCount.load(std::memory_order_acquire); }

public slots:
  // 添加客户端（在工作线程中执行）
  void addClient(qintptr socketDescriptor);

  // 发送消息给指定客户端
  void sendMessageToClient(qintptr clientId, const QString &message);

  // 广播消息给此 Worker 管理的所有客户端
  void broadcastMessage(const QString &message);

  // 断开指定客户端
  void disconnectClient(qintptr clientId);

  // 清理所有客户端（线程停止前调用）
  void cleanup();

signals:
  // 客户端就绪
  void clientReady(qintptr clientId, const QString &address);

  // 接收到消息
  void messageReceived(qintptr clientId, const QString &message);

  // 客户端断开
  void clientDisconnected(qintptr clientId);

  // 错误发生
  void errorOccurred(qintptr clientId, const QString &error);

private slots:
  // 处理客户端断开（在工作线程中执行）
  void handleClientDisconnected(qintptr clientId);

private:
  int m_threadId; // 线程 ID
  QHash<qintptr, ClientHandler *> m_clientHandlers; // 客户端处理器映射
  std::atomic<int> m_clientCount; // 客户端数量（原子变量）
};

#endif // IOTHREADWORKER_H
