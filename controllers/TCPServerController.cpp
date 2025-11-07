#include "TCPServerController.h"
#include <QDateTime>

TCPServerController::TCPServerController(QObject *parent)
  : QObject(parent)
    , m_server(new TCPServer(0, this)) {
  // 使用默认线程数（基于CPU核心数）
  // 连接信号
  connect(m_server, &TCPServer::serverStarted, this, &TCPServerController::onServerStarted);
  connect(m_server, &TCPServer::serverStopped, this, &TCPServerController::onServerStopped);
  connect(m_server, &TCPServer::clientConnected, this, &TCPServerController::onClientConnected);
  connect(m_server, &TCPServer::clientDisconnected, this, &TCPServerController::onClientDisconnected);
  connect(m_server, &TCPServer::messageReceived, this, &TCPServerController::onMessageReceived);
  connect(m_server, &TCPServer::errorOccurred, this, &TCPServerController::onErrorOccurred);
}

TCPServerController::~TCPServerController() = default;

bool TCPServerController::isListening() const {
  return m_server->isListening();
}

int TCPServerController::clientCount() const {
  return m_server->clientCount();
}

QString TCPServerController::log() const {
  return m_log;
}

void TCPServerController::startServer(int port) {
  m_server->startServer(static_cast<quint16>(port));
}

void TCPServerController::stopServer() {
  m_server->stopServer();
}

void TCPServerController::sendMessage(qintptr clientId, const QString &message) {
  m_server->sendMessage(clientId, message);
}

void TCPServerController::broadcastMessage(const QString &message) {
  m_server->broadcastMessage(message);
}

void TCPServerController::clearLog() {
  m_log.clear();
  emit logChanged();
}

void TCPServerController::onServerStarted(quint16 port) {
  appendLog(QString("[服务器] 启动成功，监听端口: %1").arg(port));
  emit listeningChanged();
}

void TCPServerController::onServerStopped() {
  appendLog("[服务器] 已停止");
  emit listeningChanged();
  emit clientCountChanged();
}

void TCPServerController::onClientConnected(qintptr clientId, const QString &address) {
  appendLog(QString("[客户端 %1] 已连接 %2").arg(clientId).arg(address));
  emit clientCountChanged();
}

void TCPServerController::onClientDisconnected(qintptr clientId) {
  appendLog(QString("[客户端 %1] 已断开").arg(clientId));
  emit clientCountChanged();
}

void TCPServerController::onMessageReceived(qintptr clientId, const QString &message) {
  appendLog(QString("[客户端 %1] 收到: %2").arg(clientId).arg(message));
}

void TCPServerController::onErrorOccurred(const QString &error) {
  appendLog(QString("[错误] %1").arg(error));
}

void TCPServerController::appendLog(const QString &text) {
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
  m_log.append(QString("[%1] %2\n").arg(timestamp, text));
  emit logChanged();
}
