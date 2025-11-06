#include "TCPClientController.h"
#include <QDateTime>

TCPClientController::TCPClientController(QObject *parent)
  : QObject(parent)
    , m_client(new TCPClient(this))
    , m_autoReconnect(false) {
  // 连接信号
  connect(m_client, &TCPClient::connected, this, &TCPClientController::onConnected);
  connect(m_client, &TCPClient::disconnected, this, &TCPClientController::onDisconnected);
  connect(m_client, &TCPClient::messageReceived, this, &TCPClientController::onMessageReceived);
  connect(m_client, &TCPClient::errorOccurred, this, &TCPClientController::onErrorOccurred);
  connect(m_client, &TCPClient::reconnecting, this, &TCPClientController::onReconnecting);
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
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
  m_log.append(QString("[%1] %2\n").arg(timestamp, text));
  emit logChanged();
}
