#include "IOThreadWorker.h"
#include "ClientHandler.h"
#include <QDebug>
#include <QThread>

IOThreadWorker::IOThreadWorker(int threadId, QObject *parent)
  : QObject(parent)
    , m_threadId(threadId)
    , m_clientCount(0) {
  qDebug() << "[IOThreadWorker" << m_threadId << "] 创建";
}

IOThreadWorker::~IOThreadWorker() {
  qDebug() << "[IOThreadWorker" << m_threadId << "] 析构";
}

void IOThreadWorker::addClient(qintptr socketDescriptor) {
  // 通过队列连接调用，在工作线程的事件循环中执行
  qDebug() << "[IOThreadWorker" << m_threadId << "] 添加客户端" << socketDescriptor
      << "，运行在线程:" << QThread::currentThread();

  // 在工作线程中创建 ClientHandler
  ClientHandler *handler = new ClientHandler(socketDescriptor, this);

  // 连接信号（直接连接，因为在同一线程）
  connect(handler, &ClientHandler::ready, this, &IOThreadWorker::clientReady, Qt::DirectConnection);
  connect(handler, &ClientHandler::messageReceived, this, &IOThreadWorker::messageReceived, Qt::DirectConnection);
  connect(handler, &ClientHandler::disconnected, this, &IOThreadWorker::handleClientDisconnected, Qt::DirectConnection);
  connect(handler, &ClientHandler::errorOccurred, this, &IOThreadWorker::errorOccurred, Qt::DirectConnection);

  // 保存到映射表
  m_clientHandlers.insert(socketDescriptor, handler);
  m_clientCount.fetch_add(1, std::memory_order_release);

  // 初始化连接
  handler->initialize();

  qDebug() << "[IOThreadWorker" << m_threadId << "] 当前客户端数:"
      << m_clientCount.load(std::memory_order_acquire);
}

void IOThreadWorker::handleClientDisconnected(qintptr clientId) {
  // 从映射表中移除
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    // ClientHandler 会自动 deleteLater，无需手动删除
    m_clientHandlers.erase(it);
    m_clientCount.fetch_sub(1, std::memory_order_release);

    qDebug() << "[IOThreadWorker" << m_threadId << "] 移除客户端" << clientId
        << "，当前客户端数:" << m_clientCount.load(std::memory_order_acquire);
  }

  // 转发信号到外部
  emit clientDisconnected(clientId);
}

void IOThreadWorker::sendMessageToClient(qintptr clientId, const QString &message) {
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    it.value()->sendMessage(message);
  } else {
    qWarning() << "[IOThreadWorker" << m_threadId << "] 客户端" << clientId << "不存在";
  }
}

void IOThreadWorker::disconnectClient(qintptr clientId) {
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    it.value()->disconnect();
  }
}

void IOThreadWorker::broadcastMessage(const QString &message) {
  // 遍历本线程管理的所有客户端，发送消息
  for (auto it = m_clientHandlers.begin(); it != m_clientHandlers.end(); ++it) {
    it.value()->sendMessage(message);
  }
  qDebug() << "[IOThreadWorker" << m_threadId << "] 广播消息给"
      << m_clientHandlers.size() << "个客户端";
}

void IOThreadWorker::cleanup() {
  // 清理所有客户端处理器（线程停止前调用）
  qDebug() << "[IOThreadWorker" << m_threadId << "] 清理" << m_clientHandlers.size() << "个客户端";

  for (auto it = m_clientHandlers.begin(); it != m_clientHandlers.end(); ++it) {
    it.value()->deleteLater();
  }
  m_clientHandlers.clear();
  m_clientCount.store(0, std::memory_order_release);
}
