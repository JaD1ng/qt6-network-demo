#include "IOThreadPool.h"
#include "IOThread.h"
#include <QDebug>
#include <algorithm>

IOThreadPool::IOThreadPool(int threadCount, QObject *parent)
  : QObject(parent)
    , m_nextThreadIndex(0)
    , m_threadCount(threadCount) {
  // 如果未指定线程数，使用 CPU 核心数
  if (m_threadCount <= 0) {
    m_threadCount = static_cast<int>(std::thread::hardware_concurrency());
    if (m_threadCount <= 0) {
      m_threadCount = 4; // 默认值
    }
  }

  qDebug() << "[IOThreadPool] 线程池大小:" << m_threadCount;
}

IOThreadPool::~IOThreadPool() {
  stop();
}

void IOThreadPool::start() {
  if (!m_threads.isEmpty()) {
    qWarning() << "[IOThreadPool] 线程池已经启动";
    return;
  }

  // 创建并启动所有 I/O 线程
  m_threads.reserve(m_threadCount);
  for (int i = 0; i < m_threadCount; ++i) {
    IOThread *thread = new IOThread(i, this);

    // 连接信号（使用队列连接，跨线程通信）
    connect(thread, &IOThread::clientReady, this, &IOThreadPool::clientReady, Qt::QueuedConnection);
    connect(thread, &IOThread::messageReceived, this, &IOThreadPool::messageReceived, Qt::QueuedConnection);
    connect(thread, &IOThread::clientDisconnected, this, &IOThreadPool::handleClientDisconnected, Qt::QueuedConnection);
    connect(thread, &IOThread::errorOccurred, this, &IOThreadPool::errorOccurred, Qt::QueuedConnection);

    thread->start();
    m_threads.append(thread);
  }

  qDebug() << "[IOThreadPool] 启动完成，" << m_threadCount << "个线程";
}

void IOThreadPool::stop() {
  if (m_threads.isEmpty()) {
    return;
  }

  qDebug() << "[IOThreadPool] 停止中...";

  // 停止所有线程
  for (IOThread *thread: m_threads) {
    thread->quit();
  }

  // 等待所有线程结束
  for (IOThread *thread: m_threads) {
    thread->wait();
    delete thread;
  }

  m_threads.clear();
  m_clientThreadMap.clear();
  m_nextThreadIndex.store(0, std::memory_order_release);

  qDebug() << "[IOThreadPool] 已停止";
}

void IOThreadPool::addClient(qintptr socketDescriptor) {
  // 使用轮询策略选择线程
  IOThread *selectedThread = selectNextThread();
  if (!selectedThread) {
    qWarning() << "[IOThreadPool] 没有可用的 I/O 线程";
    return;
  }

  // 记录客户端到线程的映射
  m_clientThreadMap.insert(socketDescriptor, selectedThread);

  // 添加客户端到选中的线程
  selectedThread->addClient(socketDescriptor);

  qDebug() << "[IOThreadPool] 分配客户端" << socketDescriptor
      << "到线程" << selectedThread->threadId();
}

void IOThreadPool::sendMessage(qintptr clientId, const QString &message) {
  // 查找客户端所在的线程
  auto it = m_clientThreadMap.find(clientId);
  if (it != m_clientThreadMap.end()) {
    it.value()->sendMessageToClient(clientId, message);
  } else {
    qWarning() << "[IOThreadPool] 客户端" << clientId << "不存在";
  }
}

void IOThreadPool::broadcastMessage(const QString &message) {
  // 向所有线程广播消息
  for (IOThread *thread: m_threads) {
    // 获取该线程的所有客户端并发送消息
    // 注意：这里简化实现，实际需要遍历所有客户端
    // 可以通过在 IOThread 中添加 broadcastMessage 方法优化
    for (auto it = m_clientThreadMap.begin(); it != m_clientThreadMap.end(); ++it) {
      if (it.value() == thread) {
        thread->sendMessageToClient(it.key(), message);
      }
    }
  }
}

void IOThreadPool::disconnectClient(qintptr clientId) {
  // 查找客户端所在的线程
  auto it = m_clientThreadMap.find(clientId);
  if (it != m_clientThreadMap.end()) {
    it.value()->disconnectClient(clientId);
  }
}

int IOThreadPool::totalClientCount() const {
  int total = 0;
  for (const IOThread *thread: m_threads) {
    total += thread->clientCount();
  }
  return total;
}

IOThread *IOThreadPool::selectNextThread() {
  if (m_threads.isEmpty()) {
    return nullptr;
  }

  // 轮询策略：依次选择下一个线程
  int index = m_nextThreadIndex.fetch_add(1, std::memory_order_relaxed) % m_threads.size();
  return m_threads[index];
}

void IOThreadPool::handleClientDisconnected(qintptr clientId) {
  // 从映射表中移除
  m_clientThreadMap.remove(clientId);

  // 转发信号
  emit clientDisconnected(clientId);
}