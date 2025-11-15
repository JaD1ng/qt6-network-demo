#include "IOThreadPool.h"
#include "IOThreadWorker.h"
#include <QDebug>
#include <algorithm>

IOThreadPool::IOThreadPool(int threadCount, QObject *parent)
  : QObject(parent)
    , m_nextWorkerIndex(0)
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
  if (!m_workers.isEmpty()) {
    qWarning() << "[IOThreadPool] 线程池已经启动";
    return;
  }

  // 创建并启动所有 I/O 线程和 Worker
  m_workers.reserve(m_threadCount);
  for (int i = 0; i < m_threadCount; ++i) {
    auto *thread = new QThread(this);
    thread->setObjectName(QString("IOThread-%1").arg(i));

    // 创建 Worker 对象（负责业务逻辑）
    auto *worker = new IOThreadWorker(i);

    // 将 Worker 移动到线程中
    worker->moveToThread(thread);

    // 连接信号（使用队列连接，跨线程通信）
    connect(worker, &IOThreadWorker::clientReady, this, &IOThreadPool::clientReady, Qt::QueuedConnection);
    connect(worker, &IOThreadWorker::messageReceived, this, &IOThreadPool::messageReceived, Qt::QueuedConnection);
    connect(worker, &IOThreadWorker::clientDisconnected, this, &IOThreadPool::handleClientDisconnected,
            Qt::QueuedConnection);
    connect(worker, &IOThreadWorker::errorOccurred, this, &IOThreadPool::errorOccurred, Qt::QueuedConnection);

    // 线程结束时清理 Worker
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);

    // 启动线程（QThread 自动运行事件循环）
    thread->start();

    // 保存到列表
    m_workers.emplaceBack(thread,worker);

    qDebug() << "[IOThreadPool] 线程" << i << "已启动";
  }

  qDebug() << "[IOThreadPool] 启动完成，" << m_threadCount << "个线程";
}

void IOThreadPool::stop() {
  if (m_workers.isEmpty()) {
    return;
  }

  qDebug() << "[IOThreadPool] 停止中...";

  // 先清理所有 Worker 的客户端
  for (const auto &[_, worker]: m_workers) {
    QMetaObject::invokeMethod(worker, &IOThreadWorker::cleanup, Qt::QueuedConnection);
  }

  // 停止所有线程
  for (const auto &[thread, _]: m_workers) {
    thread->quit();
  }

  // 等待所有线程结束
  for (const ThreadContext &ctx: m_workers) {
    ctx.thread->wait();
    delete ctx.thread; // Worker 会通过 finished 信号自动 deleteLater
  }

  m_workers.clear();
  m_clientWorkerMap.clear();
  m_nextWorkerIndex.store(0, std::memory_order_relaxed);

  qDebug() << "[IOThreadPool] 已停止";
}

void IOThreadPool::addClient(qintptr socketDescriptor) {
  // 使用轮询策略选择 Worker
  IOThreadWorker *selectedWorker = selectNextWorker();
  if (!selectedWorker) {
    qWarning() << "[IOThreadPool] 没有可用的 I/O Worker";
    return;
  }

  // 记录客户端到 Worker 的映射
  m_clientWorkerMap.insert(socketDescriptor, selectedWorker);

  // 添加客户端到选中的 Worker（通过队列连接调用）
  QMetaObject::invokeMethod(selectedWorker, &IOThreadWorker::addClient,
                            Qt::QueuedConnection, socketDescriptor);

  qDebug() << "[IOThreadPool] 分配客户端" << socketDescriptor
      << "到 Worker" << selectedWorker->threadId();
}

void IOThreadPool::sendMessage(qintptr clientId, const QString &message) {
  // 查找客户端所在的 Worker
  auto it = m_clientWorkerMap.find(clientId);
  if (it != m_clientWorkerMap.end()) {
    QMetaObject::invokeMethod(it.value(), &IOThreadWorker::sendMessageToClient,
                              Qt::QueuedConnection, clientId, message);
  } else {
    qWarning() << "[IOThreadPool] 客户端" << clientId << "不存在";
  }
}

void IOThreadPool::broadcastMessage(const QString &message) {
  // 直接调用每个 Worker 的 broadcastMessage
  for (const ThreadContext &ctx: m_workers) {
    QMetaObject::invokeMethod(ctx.worker, &IOThreadWorker::broadcastMessage,
                              Qt::QueuedConnection, message);
  }
  qDebug() << "[IOThreadPool] 广播消息给所有 Worker";
}

void IOThreadPool::disconnectClient(qintptr clientId) {
  // 查找客户端所在的 Worker
  auto it = m_clientWorkerMap.find(clientId);
  if (it != m_clientWorkerMap.end()) {
    QMetaObject::invokeMethod(it.value(), &IOThreadWorker::disconnectClient,
                              Qt::QueuedConnection, clientId);
  }
}

int IOThreadPool::totalClientCount() const {
  int total = 0;
  for (const ThreadContext &ctx: m_workers) {
    total += ctx.worker->clientCount();
  }
  return total;
}

IOThreadWorker *IOThreadPool::selectNextWorker() {
  if (m_workers.isEmpty()) {
    return nullptr;
  }

  // 轮询策略：依次选择下一个 Worker
  int index = m_nextWorkerIndex.fetch_add(1, std::memory_order_relaxed) % m_workers.size();
  return m_workers[index].worker;
}

void IOThreadPool::handleClientDisconnected(qintptr clientId) {
  // 从映射表中移除
  m_clientWorkerMap.remove(clientId);

  // 转发信号
  emit clientDisconnected(clientId);
}
