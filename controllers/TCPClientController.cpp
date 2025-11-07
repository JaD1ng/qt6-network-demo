#include "TCPClientController.h"
#include <QDateTime>

TCPClientController::TCPClientController(QObject *parent)
  : QObject(parent)
    , m_client(new TCPClientWorker(this)) // 使用 TCPClientWorker（独立线程）
    , m_autoReconnect(false) {
  // 连接信号
  connect(m_client, &TCPClientWorker::connected, this, &TCPClientController::onConnected);
  connect(m_client, &TCPClientWorker::disconnected, this, &TCPClientController::onDisconnected);
  connect(m_client, &TCPClientWorker::messageReceived, this, &TCPClientController::onMessageReceived);
  connect(m_client, &TCPClientWorker::errorOccurred, this, &TCPClientController::onErrorOccurred);
  connect(m_client, &TCPClientWorker::reconnecting, this, &TCPClientController::onReconnecting);
}

TCPClientController::~TCPClientController() = default;

bool TCPClientController::isConnected() const {
  return m_client->isConnected();
}

bool TCPClientController::autoReconnect() const {
  return m_autoReconnect;
}

void TCPClientController::setAutoReconnect(bool enable) {
  if (m_autoReconnect != enable) {
    m_autoReconnect = enable;
    m_client->setAutoReconnect(enable);
    emit autoReconnectChanged();
  }
}

QString TCPClientController::log() const {
  return m_log;
}

void TCPClientController::connectToServer(const QString &host, int port) {
  m_client->connectToServer(host, static_cast<quint16>(port));
  appendLog(QString("[客户端] 正在连接 %1:%2").arg(host).arg(port));
}

void TCPClientController::disconnectFromServer() {
  m_client->disconnectFromServer();
}

void TCPClientController::sendMessage(const QString &message) {
  m_client->sendMessage(message);
  appendLog(QString("[客户端] 发送: %1").arg(message));
}

void TCPClientController::clearLog() {
  m_log.clear();
  emit logChanged();
}

void TCPClientController::onConnected() {
  appendLog("[客户端] 连接成功");
  emit connectedChanged();
}

void TCPClientController::onDisconnected() {
  appendLog("[客户端] 已断开连接");
  emit connectedChanged();
}

void TCPClientController::onMessageReceived(const QString &message) {
  appendLog(QString("[客户端] 收到: %1").arg(message));
}

void TCPClientController::onErrorOccurred(const QString &error) {
  appendLog(QString("[错误] %1").arg(error));
}

void TCPClientController::onReconnecting() {
  appendLog("[客户端] 正在尝试重连...");
}

void TCPClientController::appendLog(const QString &text) {
  // 预留空间
  if (m_log.capacity() - m_log.size() < 100) {
    m_log.reserve(m_log.size() + 4096);
  }

  // 直接构建字符串，减少临时对象
  m_log.append(QString("[%1] %2\n")
    .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), text));
  emit logChanged();
}
