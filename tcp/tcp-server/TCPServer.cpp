#include "TCPServer.h"
#include "IOThreadPool.h"
#include <QDebug>
#include <QHostAddress>

TCPServer::TCPServer(int threadCount, QObject *parent)
  : QTcpServer(parent)
    , m_threadPool(new IOThreadPool(threadCount, this)) {
  // 连接线程池信号（队列连接，跨线程通信）
  connect(m_threadPool, &IOThreadPool::clientReady, this, &TCPServer::clientConnected, Qt::QueuedConnection);
  connect(m_threadPool, &IOThreadPool::messageReceived, this, &TCPServer::messageReceived, Qt::QueuedConnection);
  connect(m_threadPool, &IOThreadPool::clientDisconnected, this, &TCPServer::clientDisconnected, Qt::QueuedConnection);
  connect(m_threadPool, &IOThreadPool::errorOccurred, this, [this](qintptr clientId, const QString &error) {
    Q_UNUSED(clientId)
    emit errorOccurred(error);
  }, Qt::QueuedConnection);
}

TCPServer::~TCPServer() {
  stopServer();
}

bool TCPServer::startServer(quint16 port) {
  if (isListening()) {
    emit errorOccurred("服务器已经在运行");
    return false;
  }

  // 启动线程池
  m_threadPool->start();

  // 监听端口
  if (!listen(QHostAddress::Any, port)) {
    emit errorOccurred(QString("启动服务器失败: %1").arg(errorString()));
    m_threadPool->stop();
    return false;
  }

  qDebug() << "[TCPServer] 启动成功，监听端口:" << port
           << "，线程池大小:" << m_threadPool->threadCount();
  emit serverStarted(port);
  return true;
}

void TCPServer::stopServer() {
  if (!isListening()) {
    return;
  }

  qDebug() << "[TCPServer] 停止中...";

  // 关闭服务器
  close();

  // 停止线程池（会断开所有客户端）
  m_threadPool->stop();

  emit serverStopped();
  qDebug() << "[TCPServer] 已停止";
}

void TCPServer::sendMessage(qintptr clientId, const QString &message) {
  m_threadPool->sendMessage(clientId, message);
}

void TCPServer::broadcastMessage(const QString &message) {
  m_threadPool->broadcastMessage(message);
  qDebug() << "[TCPServer] 广播消息给所有客户端:" << message;
}

bool TCPServer::isListening() const {
  return QTcpServer::isListening();
}

int TCPServer::clientCount() const {
  return m_threadPool->totalClientCount();
}

int TCPServer::threadPoolSize() const {
  return m_threadPool->threadCount();
}

void TCPServer::incomingConnection(qintptr socketDescriptor) {
  // 主 Reactor：直接获取 socket 描述符并分配给从 Reactor
  qDebug() << "[TCPServer] 接受新连接，socket 描述符:" << socketDescriptor;

  // 将 socket 描述符分配给线程池（轮询策略）
  m_threadPool->addClient(socketDescriptor);
}