#include "TCPClient.h"
#include <QDebug>

TCPClient::TCPClient(QObject *parent)
  : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_port(0)
    , m_autoReconnect(false)
    , m_reconnectInterval(3000) // 默认3秒重连
    , m_isManualDisconnect(false) {
  m_receiveBuffer.reserve(4096);

  // 连接信号
  connect(m_socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &TCPClient::onDisconnected);
  connect(m_socket, &QTcpSocket::readyRead, this, &TCPClient::onReadyRead);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &TCPClient::onError);

  // 配置重连定时器
  m_reconnectTimer->setSingleShot(true);
  connect(m_reconnectTimer, &QTimer::timeout, this, &TCPClient::attemptReconnect);
}

TCPClient::~TCPClient() {
  disconnectFromServer();
}

void TCPClient::connectToServer(const QString &host, quint16 port) {
  if (m_socket->state() == QAbstractSocket::ConnectedState) {
    emit errorOccurred("已经连接到服务器");
    return;
  }

  // 保存连接参数以便重连
  m_host = host;
  m_port = port;
  m_isManualDisconnect = false;
  m_receiveBuffer.clear(); // 清空接收缓冲区

  qDebug() << "正在连接到服务器:" << host << ":" << port;
  m_socket->connectToHost(host, port);
}

void TCPClient::disconnectFromServer() {
  m_isManualDisconnect = true;
  m_reconnectTimer->stop();

  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->disconnectFromHost();
    qDebug() << "手动断开连接";
  }

  m_receiveBuffer.clear(); // 清空接收缓冲区
}

void TCPClient::sendMessage(const QString &message) {
  if (m_socket->state() != QAbstractSocket::ConnectedState) {
    emit errorOccurred("未连接到服务器");
    return;
  }

  QByteArray packet = packMessage(message);
  qint64 written = m_socket->write(packet);
  m_socket->flush();

  if (written != packet.size()) {
    emit errorOccurred("发送消息不完整");
  } else {
    qDebug() << "发送消息:" << message << "(字节数:" << packet.size() << ")";
  }
}

bool TCPClient::isConnected() const {
  return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TCPClient::setAutoReconnect(bool enable) {
  m_autoReconnect = enable;
  if (!enable) {
    m_reconnectTimer->stop();
  }
}

void TCPClient::setReconnectInterval(int msec) {
  m_reconnectInterval = msec;
}

QByteArray TCPClient::packMessage(const QString &message) {
  // 消息格式：[4字节长度(网络字节序/大端)][消息内容UTF-8]
  QByteArray utf8Data = message.toUtf8();
  quint32 messageLength = static_cast<quint32>(utf8Data.size());

  // 预分配内存：4字节长度 + 消息内容
  QByteArray packet;
  packet.reserve(sizeof(quint32) + messageLength);

  QDataStream stream(&packet, QIODevice::WriteOnly);
  stream.setByteOrder(QDataStream::BigEndian); // 设置为网络字节序（大端）

  stream << messageLength; // 写入4字节长度
  packet.append(utf8Data); // 追加消息内容

  return packet;
}

void TCPClient::parseReceivedData() {
  // 读取所有可用数据到缓冲区
  m_receiveBuffer.append(std::move(m_socket->readAll()));

  // 循环解析完整的消息
  while (m_receiveBuffer.size() >= static_cast<int>(sizeof(quint32))) {
    // 读取消息长度（前4字节，大端）
    QDataStream stream(m_receiveBuffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 messageLength;
    stream >> messageLength;

    // 检查消息长度合法性
    constexpr quint32 MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10MB上限
    if (messageLength > MAX_MESSAGE_SIZE) {
      qWarning() << "收到的消息过大:" << messageLength;
      emit errorOccurred(QString("消息过大，断开连接"));
      m_socket->disconnectFromHost();
      return;
    }

    // 检查是否接收到完整消息（处理半包）
    int totalSize = static_cast<int>(sizeof(quint32) + messageLength);
    if (m_receiveBuffer.size() < totalSize) {
      // 数据不完整，等待更多数据（半包）
      qDebug() << "数据不完整，等待..."
          << "已接收:" << m_receiveBuffer.size() << "需要:" << totalSize;
      break;
    }

    // 提取消息内容（跳过前4字节的长度字段）
    // 直接从指针构造
    QString message = QString::fromUtf8(m_receiveBuffer.constData() + sizeof(quint32), messageLength);

    // 从缓冲区移除已处理的消息（处理黏包）
    m_receiveBuffer.remove(0, totalSize);

    // 发出消息信号
    if (!message.isEmpty()) {
      qDebug() << "收到完整消息:" << message;
      emit messageReceived(message);
    }
  }
}

void TCPClient::onConnected() {
  m_reconnectTimer->stop();

  // 智能处理 IPv4/IPv6 地址显示
  QHostAddress peerAddr = m_socket->peerAddress();
  QString serverAddressStr;
  if (peerAddr.protocol() == QAbstractSocket::IPv6Protocol) {
    // 检查是否为 IPv6 映射的 IPv4 地址（::ffff:x.x.x.x）
    QHostAddress ipv4(peerAddr.toIPv4Address());
    if (!ipv4.isNull()) {
      // 转换为纯 IPv4 显示
      serverAddressStr = ipv4.toString();
    } else {
      // 真正的 IPv6 地址，保持原样
      serverAddressStr = peerAddr.toString();
    }
  } else {
    // 纯 IPv4 地址
    serverAddressStr = peerAddr.toString();
  }

  qDebug() << "已连接到服务器:" << serverAddressStr << ":" << m_socket->peerPort();
  emit connected();
}

void TCPClient::onDisconnected() {
  qDebug() << "与服务器断开连接";
  m_receiveBuffer.clear(); // 清空接收缓冲区
  emit disconnected();

  // 自动重连逻辑
  if (m_autoReconnect && !m_isManualDisconnect && !m_host.isEmpty()) {
    qDebug() << "将在" << m_reconnectInterval << "毫秒后尝试重连...";
    emit reconnecting();
    m_reconnectTimer->start(m_reconnectInterval);
  }
}

void TCPClient::onReadyRead() {
  // 解析接收到的数据（自动处理黏包和半包）
  parseReceivedData();
}

void TCPClient::onError(QAbstractSocket::SocketError socketError) {
  Q_UNUSED(socketError)
  QString errorString = m_socket->errorString();
  qDebug() << "TCP客户端错误:" << errorString;
  emit errorOccurred(errorString);

  // 连接失败时也尝试重连
  if (m_autoReconnect && !m_isManualDisconnect &&
      m_socket->state() == QAbstractSocket::UnconnectedState) {
    qDebug() << "将在" << m_reconnectInterval << "毫秒后尝试重连...";
    emit reconnecting();
    m_reconnectTimer->start(m_reconnectInterval);
  }
}

void TCPClient::attemptReconnect() {
  if (m_socket->state() == QAbstractSocket::UnconnectedState &&
      !m_host.isEmpty() && m_port > 0) {
    qDebug() << "尝试重新连接到:" << m_host << ":" << m_port;
    m_receiveBuffer.clear(); // 清空接收缓冲区
    m_socket->connectToHost(m_host, m_port);
  }
}
