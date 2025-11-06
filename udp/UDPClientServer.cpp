#include "UDPClientServer.h"
#include <QDebug>

// UDP数据报推荐最大大小（避免IP分片）
constexpr qint64 MAX_UDP_DATAGRAM_SIZE = 1472;

UDPClientServer::UDPClientServer(QObject *parent)
  : QObject(parent)
    , m_socket(new QUdpSocket(this)) {
  connect(m_socket, &QUdpSocket::readyRead, this, &UDPClientServer::onReadyRead);
}

UDPClientServer::~UDPClientServer() {
  unbind();
}

bool UDPClientServer::bind(quint16 port) {
  if (m_socket->state() == QAbstractSocket::BoundState) {
    emit errorOccurred("Socket已经绑定");
    return false;
  }

  // 绑定到指定端口，接收所有网络接口的数据
  // 使用 ShareAddress 和 ReuseAddressHint 允许多个程序绑定同一端口（用于广播接收）
  if (!m_socket->bind(QHostAddress::Any, port,
                      QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
    emit errorOccurred(QString("绑定端口失败: %1").arg(m_socket->errorString()));
    return false;
  }

  emit bound(port);
  qDebug() << "UDP绑定成功，端口:" << port;
  return true;
}

void UDPClientServer::unbind() {
  if (m_socket->state() == QAbstractSocket::BoundState) {
    m_socket->close();
    emit unbound();
    qDebug() << "UDP已解绑";
  }
}

void UDPClientServer::sendMessage(const QString &message, const QString &targetHost, quint16 targetPort) {
  QByteArray data = message.toUtf8();

  // 检查消息大小
  if (data.size() > MAX_UDP_DATAGRAM_SIZE) {
    emit errorOccurred(QString("消息过大 (%1字节)，建议不超过%2字节")
      .arg(data.size()).arg(MAX_UDP_DATAGRAM_SIZE));
    // 仍然尝试发送，但可能会被分片
  }

  QHostAddress targetAddress(targetHost);
  if (targetAddress.isNull()) {
    emit errorOccurred(QString("无效的目标地址: %1").arg(targetHost));
    return;
  }

  qint64 sent = m_socket->writeDatagram(data, targetAddress, targetPort);
  if (sent == -1) {
    emit errorOccurred(QString("发送失败: %1").arg(m_socket->errorString()));
  } else if (sent != data.size()) {
    emit errorOccurred(QString("发送不完整: 发送%1字节，实际%2字节").arg(data.size()).arg(sent));
  } else {
    qDebug() << "UDP发送成功 ->" << targetHost << ":" << targetPort
        << "内容:" << message << "(字节数:" << data.size() << ")";
  }
}

void UDPClientServer::sendBroadcast(const QString &message, quint16 targetPort) {
  QByteArray data = message.toUtf8();

  // 检查消息大小
  if (data.size() > MAX_UDP_DATAGRAM_SIZE) {
    emit errorOccurred(QString("消息过大 (%1字节)，建议不超过%2字节")
      .arg(data.size()).arg(MAX_UDP_DATAGRAM_SIZE));
  }

  qint64 sent = m_socket->writeDatagram(data, QHostAddress::Broadcast, targetPort);
  if (sent == -1) {
    emit errorOccurred(QString("广播失败: %1").arg(m_socket->errorString()));
  } else if (sent != data.size()) {
    emit errorOccurred(QString("广播不完整: 发送%1字节，实际%2字节").arg(data.size()).arg(sent));
  } else {
    qDebug() << "UDP广播成功 -> 端口:" << targetPort
        << "内容:" << message << "(字节数:" << data.size() << ")";
  }
}

bool UDPClientServer::isBound() const {
  return m_socket->state() == QAbstractSocket::BoundState;
}

quint16 UDPClientServer::localPort() const {
  return m_socket->localPort();
}

void UDPClientServer::onReadyRead() {
  QByteArray datagram;
  QHostAddress senderAddress;
  quint16 senderPort;

  // 处理所有待接收的数据报
  while (m_socket->hasPendingDatagrams()) {
    datagram.resize(static_cast<int>(m_socket->pendingDatagramSize()));

    qint64 received = m_socket->readDatagram(datagram.data(), datagram.size(),
                                             &senderAddress, &senderPort);

    if (received == -1) {
      emit errorOccurred(QString("接收失败: %1").arg(m_socket->errorString()));
      continue;
    }

    // 解码为UTF-8字符串
    QString message = QString::fromUtf8(datagram.constData(), received);

    // 处理 IPv4/IPv6 地址显示
    QString senderAddressStr;
    if (senderAddress.protocol() == QAbstractSocket::IPv6Protocol) {
      // 检查是否为 IPv6 映射的 IPv4 地址（::ffff:x.x.x.x）
      QHostAddress ipv4(senderAddress.toIPv4Address());
      if (!ipv4.isNull()) {
        // 转换为纯 IPv4 显示（x.x.x.x）
        senderAddressStr = ipv4.toString();
      } else {
        // 真正的 IPv6 地址，保持原样
        senderAddressStr = senderAddress.toString();
      }
    } else {
      // IPv4 地址
      senderAddressStr = senderAddress.toString();
    }

    qDebug() << "UDP收到消息 <-" << senderAddressStr << ":" << senderPort
        << "内容:" << message << "(字节数:" << received << ")";

    emit messageReceived(message, senderAddressStr, senderPort);
  }
}
