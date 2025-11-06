#include "UDPController.h"
#include <QDateTime>

UDPController::UDPController(QObject *parent)
  : QObject(parent)
    , m_udp(new UDPClientServer(this)) {
  // 连接信号
  connect(m_udp, &UDPClientServer::bound, this, &UDPController::onBound);
  connect(m_udp, &UDPClientServer::unbound, this, &UDPController::onUnbound);
  connect(m_udp, &UDPClientServer::messageReceived, this, &UDPController::onMessageReceived);
  connect(m_udp, &UDPClientServer::errorOccurred, this, &UDPController::onErrorOccurred);
}

UDPController::~UDPController() = default;

bool UDPController::isBound() const {
  return m_udp->isBound();
}

int UDPController::localPort() const {
  return m_udp->localPort();
}

QString UDPController::log() const {
  return m_log;
}

void UDPController::bind(int port) {
  m_udp->bind(static_cast<quint16>(port));
}

void UDPController::unbind() {
  m_udp->unbind();
}

void UDPController::sendMessage(const QString &message, const QString &targetHost, int targetPort) {
  m_udp->sendMessage(message, targetHost, static_cast<quint16>(targetPort));
  appendLog(QString("[UDP] 发送至 %1:%2 - %3").arg(targetHost).arg(targetPort).arg(message));
}

void UDPController::sendBroadcast(const QString &message, int targetPort) {
  m_udp->sendBroadcast(message, static_cast<quint16>(targetPort));
  appendLog(QString("[UDP] 广播至端口 %1 - %2").arg(targetPort).arg(message));
}

void UDPController::clearLog() {
  m_log.clear();
  emit logChanged();
}

void UDPController::onBound(quint16 port) {
  appendLog(QString("[UDP] 绑定成功，端口: %1").arg(port));
  emit boundChanged();
  emit localPortChanged();
}

void UDPController::onUnbound() {
  appendLog("[UDP] 已解绑");
  emit boundChanged();
  emit localPortChanged();
}

void UDPController::onMessageReceived(const QString &message, const QString &senderAddress, quint16 senderPort) {
  appendLog(QString("[UDP] 收到 %1:%2 - %3").arg(senderAddress).arg(senderPort).arg(message));
}

void UDPController::onErrorOccurred(const QString &error) {
  appendLog(QString("[错误] %1").arg(error));
}

void UDPController::appendLog(const QString &text) {
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
  m_log.append(QString("[%1] %2\n").arg(timestamp, text));
  emit logChanged();
}