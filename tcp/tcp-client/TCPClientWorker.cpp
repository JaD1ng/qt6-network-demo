#include "TCPClientWorker.h"
#include "TCPClient.h"
#include <QDebug>

TCPClientWorker::TCPClientWorker(QObject *parent)
  : QObject(parent)
    , m_workerThread(new QThread(this))
    , m_client(nullptr)
    , m_isConnected(false) {
  // 创建 TCPClient（在主线程）
  m_client = new TCPClient();

  // 移动到工作线程
  m_client->moveToThread(m_workerThread);

  // 连接所有信号（使用队列连接，跨线程通信）
  // connected 和 disconnected 信号同时更新状态缓存，避免重复监听
  connect(m_client, &TCPClient::connected, this, [this]() {
    m_isConnected = true;
    emit connected();
  }, Qt::QueuedConnection);

  connect(m_client, &TCPClient::disconnected, this, [this]() {
    m_isConnected = false;
    emit disconnected();
  }, Qt::QueuedConnection);

  connect(m_client, &TCPClient::messageReceived, this, &TCPClientWorker::messageReceived, Qt::QueuedConnection);
  connect(m_client, &TCPClient::errorOccurred, this, &TCPClientWorker::errorOccurred, Qt::QueuedConnection);
  connect(m_client, &TCPClient::reconnecting, this, &TCPClientWorker::reconnecting, Qt::QueuedConnection);

  // 线程启动时初始化
  connect(m_workerThread, &QThread::started, this, &TCPClientWorker::initializeClient);

  // 确保 TCPClient 在线程结束时被删除
  connect(m_workerThread, &QThread::finished, m_client, &QObject::deleteLater);

  // 启动工作线程
  m_workerThread->start();

  qDebug() << "[TCPClientWorker] 创建完成，工作线程:" << m_workerThread;
}

TCPClientWorker::~TCPClientWorker() {
  qDebug() << "[TCPClientWorker] 析构中...";

  // 停止工作线程
  m_workerThread->quit();
  m_workerThread->wait();

  qDebug() << "[TCPClientWorker] 析构完成";
}

void TCPClientWorker::connectToServer(const QString &host, quint16 port) {
  // 线程安全：通过队列连接调用
  QMetaObject::invokeMethod(m_client, [this, host, port]() {
    m_client->connectToServer(host, port);
  }, Qt::QueuedConnection);
}

void TCPClientWorker::disconnectFromServer() {
  // 线程安全：通过队列连接调用
  QMetaObject::invokeMethod(m_client, [this]() {
    m_client->disconnectFromServer();
  }, Qt::QueuedConnection);
}

void TCPClientWorker::sendMessage(const QString &message) {
  // 线程安全：通过队列连接调用
  QMetaObject::invokeMethod(m_client, [this, message]() {
    m_client->sendMessage(message);
  }, Qt::QueuedConnection);
}

bool TCPClientWorker::isConnected() const {
  // 返回缓存的连接状态，避免跨线程阻塞调用
  return m_isConnected;
}

void TCPClientWorker::setAutoReconnect(bool enable) {
  // 线程安全：通过队列连接调用
  QMetaObject::invokeMethod(m_client, [this, enable]() {
    m_client->setAutoReconnect(enable);
  }, Qt::QueuedConnection);
}

void TCPClientWorker::setReconnectInterval(int msec) {
  // 线程安全：通过队列连接调用
  QMetaObject::invokeMethod(m_client, [this, msec]() {
    m_client->setReconnectInterval(msec);
  }, Qt::QueuedConnection);
}

void TCPClientWorker::initializeClient() {
  // 工作线程启动时的初始化（如果需要）
  qDebug() << "[TCPClientWorker] 工作线程已启动:" << QThread::currentThread();
}
