#include "IOThread.h"
#include "ClientHandler.h"
#include <QDebug>
#include <QEventLoop>

IOThread::IOThread(int threadId, QObject *parent)
  : QThread(parent)
    , m_threadId(threadId)
    , m_clientCount(0) {
  // 连接内部信号到槽（队列连接，确保在本线程中执行）
  connect(this, &IOThread::doAddClient, this, &IOThread::handleAddClient, Qt::QueuedConnection);
  connect(this, &IOThread::doSendMessage, this, &IOThread::handleSendMessage, Qt::QueuedConnection);
  connect(this, &IOThread::doDisconnect, this, &IOThread::handleDisconnect, Qt::QueuedConnection);
}

IOThread::~IOThread() {
  // 停止线程
  quit();
  wait();

  qDebug() << "[IOThread" << m_threadId << "] 析构";
}

void IOThread::addClient(qintptr socketDescriptor) {
  // 通过信号发送到本线程（线程安全）
  emit doAddClient(socketDescriptor);
}

void IOThread::sendMessageToClient(qintptr clientId, const QString &message) {
  // 通过信号发送到本线程（线程安全）
  emit doSendMessage(clientId, message);
}

void IOThread::disconnectClient(qintptr clientId) {
  // 通过信号发送到本线程（线程安全）
  emit doDisconnect(clientId);
}

void IOThread::run() {
  qDebug() << "[IOThread" << m_threadId << "] 启动，线程:" << QThread::currentThread();

  // 启动事件循环
  exec();

  // 事件循环结束后，清理所有客户端处理器
  for (auto it = m_clientHandlers.begin(); it != m_clientHandlers.end(); ++it) {
    delete it.value();
  }
  m_clientHandlers.clear();
  m_clientCount.store(0, std::memory_order_release);

  qDebug() << "[IOThread" << m_threadId << "] 停止";
}

void IOThread::handleAddClient(qintptr socketDescriptor) {
  // 在本线程中创建 ClientHandler
  ClientHandler *handler = new ClientHandler(socketDescriptor);

  // 连接信号（直接连接，因为在同一线程）
  connect(handler, &ClientHandler::ready, this, &IOThread::handleClientReady, Qt::DirectConnection);
  connect(handler, &ClientHandler::messageReceived, this, &IOThread::messageReceived, Qt::DirectConnection);
  connect(handler, &ClientHandler::disconnected, this, &IOThread::handleClientDisconnected, Qt::DirectConnection);
  connect(handler, &ClientHandler::errorOccurred, this, &IOThread::errorOccurred, Qt::DirectConnection);

  // 保存到映射表
  m_clientHandlers.insert(socketDescriptor, handler);
  m_clientCount.fetch_add(1, std::memory_order_release);

  // 初始化连接
  handler->initialize();

  qDebug() << "[IOThread" << m_threadId << "] 添加客户端" << socketDescriptor
      << "，当前客户端数:" << m_clientCount.load(std::memory_order_acquire);
}

void IOThread::handleClientReady(qintptr clientId, const QString &address) {
  // 转发信号到外部
  emit clientReady(clientId, address);
}

void IOThread::handleClientDisconnected(qintptr clientId) {
  // 从映射表中移除
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    // ClientHandler 会自动 deleteLater，无需手动删除
    m_clientHandlers.erase(it);
    m_clientCount.fetch_sub(1, std::memory_order_release);

    qDebug() << "[IOThread" << m_threadId << "] 移除客户端" << clientId
        << "，当前客户端数:" << m_clientCount.load(std::memory_order_acquire);
  }

  // 转发信号到外部
  emit clientDisconnected(clientId);
}

void IOThread::handleSendMessage(qintptr clientId, const QString &message) {
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    it.value()->sendMessage(message);
  } else {
    qWarning() << "[IOThread" << m_threadId << "] 客户端" << clientId << "不存在";
  }
}

void IOThread::handleDisconnect(qintptr clientId) {
  auto it = m_clientHandlers.find(clientId);
  if (it != m_clientHandlers.end()) {
    it.value()->disconnect();
  }
}