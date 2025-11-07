#ifndef IOTHREADPOOL_H
#define IOTHREADPOOL_H

#include <QObject>
#include <QList>
#include <QHash>
#include <atomic>
#include <thread>

class IOThread;

/**
 * @brief I/O 线程池管理类
 *
 * 功能特性：
 * - 管理多个 I/O 工作线程
 * - 使用轮询（Round Robin）策略分配客户端连接
 * - 线程池大小可配置，默认基于 CPU 核心数
 * - 线程安全的客户端管理
 *
 * 负载均衡：
 * - Round Robin：依次将新连接分配给各个线程
 * - 保证负载相对均衡
 */
class IOThreadPool : public QObject {
  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param threadCount 线程数量，0 表示使用 CPU 核心数
   * @param parent 父对象
   */
  explicit IOThreadPool(int threadCount = 0, QObject *parent = nullptr);

  ~IOThreadPool() override;

  // 启动线程池
  void start();

  // 停止线程池
  void stop();

  // 添加客户端连接（使用轮询策略分配）
  void addClient(qintptr socketDescriptor);

  // 发送消息给指定客户端
  void sendMessage(qintptr clientId, const QString &message);

  // 广播消息给所有客户端
  void broadcastMessage(const QString &message);

  // 断开指定客户端
  void disconnectClient(qintptr clientId);

  // 获取线程池大小
  int threadCount() const { return m_threads.size(); }

  // 获取总客户端数量
  int totalClientCount() const;

signals:
  // 客户端就绪
  void clientReady(qintptr clientId, const QString &address);

  // 接收到消息
  void messageReceived(qintptr clientId, const QString &message);

  // 客户端断开
  void clientDisconnected(qintptr clientId);

  // 错误发生
  void errorOccurred(qintptr clientId, const QString &error);

private:
  // 根据轮询策略选择下一个线程
  IOThread *selectNextThread();

  // 处理客户端断开，更新映射表
  void handleClientDisconnected(qintptr clientId);

private:
  QList<IOThread *> m_threads; // I/O 线程列表
  QHash<qintptr, IOThread *> m_clientThreadMap; // 客户端到线程的映射
  std::atomic<int> m_nextThreadIndex; // 下一个线程索引（轮询）
  int m_threadCount; // 线程数量
};

#endif // IOTHREADPOOL_H
