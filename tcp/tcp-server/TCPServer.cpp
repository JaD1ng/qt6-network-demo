#include "TCPServer.h"
#include <QDebug>
#include <QHostAddress>

TCPServer::TCPServer(QObject *parent)
  : QObject(parent)
    , m_server(new QTcpServer(this)) {
  connect(m_server, &QTcpServer::newConnection, this, &TCPServer::onNewConnection);
}

TCPServer::~TCPServer() {
  stopServer();
}

bool TCPServer::startServer(quint16 port) {
  if (m_server->isListening()) {
    emit errorOccurred("服务器已经在运行");
    return false;
  }

  if (!m_server->listen(QHostAddress::Any, port)) {
    emit errorOccurred(QString("启动服务器失败: %1").arg(m_server->errorString()));
    return false;
  }

  emit serverStarted(port);
  qDebug() << "TCP服务器启动成功，监听端口:" << port;
  return true;
}

void TCPServer::stopServer() {
  if (!m_server->isListening()) {
    return;
  }

  // 断开所有客户端
  for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
    it.value()->disconnectFromHost();
    it.value()->deleteLater();
  }
  m_clients.clear();
  m_receiveBuffers.clear();

  m_server->close();
  emit serverStopped();
  qDebug() << "TCP服务器已停止";
}

void TCPServer::sendMessage(qintptr clientId, const QString &message) {
  auto it = m_clients.find(clientId);
  if (it != m_clients.end()) {
    QTcpSocket *socket = it.value();
    QByteArray packet = packMessage(message);

    qint64 written = socket->write(packet);
    socket->flush();

    if (written != packet.size()) {
      emit errorOccurred(QString("发送消息不完整，客户端 %1").arg(clientId));
    } else {
      qDebug() << "发送消息给客户端" << clientId << ":" << message
          << "(字节数:" << packet.size() << ")";
    }
  } else {
    emit errorOccurred(QString("客户端 %1 不存在").arg(clientId));
  }
}

void TCPServer::broadcastMessage(const QString &message) {
  QByteArray packet = packMessage(message);

  for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
    it.value()->write(packet);
    it.value()->flush();
  }

  qDebug() << "广播消息给" << m_clients.size() << "个客户端:" << message
      << "(字节数:" << packet.size() << ")";
}

bool TCPServer::isListening() const {
  return m_server->isListening();
}

int TCPServer::clientCount() const {
  return m_clients.size();
}

QByteArray TCPServer::packMessage(const QString &message) {
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

void TCPServer::parseReceivedData(QTcpSocket *socket) {
  qintptr socketDescriptor = socket->socketDescriptor();

  // 获取该客户端的接收缓冲区
  QByteArray &buffer = m_receiveBuffers[socketDescriptor];

  // 读取所有可用数据到缓冲区
  buffer.append(std::move(socket->readAll()));

  // 循环解析完整的消息
  while (buffer.size() >= static_cast<int>(sizeof(quint32))) {
    // 读取消息长度（前4字节，大端）
    QDataStream stream(buffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 messageLength;
    stream >> messageLength;

    // 防止消息过长
    constexpr quint32 MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10MB上限
    if (messageLength > MAX_MESSAGE_SIZE) {
      qWarning() << "客户端" << socketDescriptor << "发送的消息过大:" << messageLength;
      emit errorOccurred(QString("客户端 %1 消息过大，断开连接").arg(socketDescriptor));
      socket->disconnectFromHost();
      return;
    }

    // 检查是否接收到完整消息（处理半包）
    int totalSize = static_cast<int>(sizeof(quint32) + messageLength);
    if (buffer.size() < totalSize) {
      // 数据不完整，等待更多数据（半包）
      qDebug() << "客户端" << socketDescriptor << "数据不完整，等待..."
          << "已接收:" << buffer.size() << "需要:" << totalSize;
      break;
    }

    // 提取消息内容（跳过前4字节的长度字段）
    // 直接从指针构造
    QString message = QString::fromUtf8(buffer.constData() + sizeof(quint32), messageLength);

    // 从缓冲区移除已处理的消息（处理黏包）
    buffer.remove(0, totalSize);

    // 发出消息信号
    if (!message.isEmpty()) {
      emit messageReceived(socketDescriptor, message);
      qDebug() << "收到客户端" << socketDescriptor << "完整消息:" << message;
    }
  }
}

void TCPServer::onNewConnection() {
  while (m_server->hasPendingConnections()) {
    QTcpSocket *clientSocket = m_server->nextPendingConnection();
    qintptr socketDescriptor = clientSocket->socketDescriptor();

    // 保存客户端连接
    m_clients.insert(socketDescriptor, clientSocket);
    QByteArray buffer;
    buffer.reserve(4096);  // 预分配容量，但 size = 0
    m_receiveBuffers.insert(socketDescriptor, buffer);

    // 连接信号
    connect(clientSocket, &QTcpSocket::readyRead, this, &TCPServer::onClientReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &TCPServer::onClientDisconnected);

    // 处理 IPv4/IPv6 地址显示
    QHostAddress peerAddr = clientSocket->peerAddress();
    QString clientAddressStr;
    if (peerAddr.protocol() == QAbstractSocket::IPv6Protocol) {
      // 检查是否为 IPv6 映射的 IPv4 地址（::ffff:x.x.x.x）
      QHostAddress ipv4(peerAddr.toIPv4Address());
      if (!ipv4.isNull()) {
        // 转换为纯 IPv4 显示
        clientAddressStr = ipv4.toString();
      } else {
        // 真正的 IPv6 地址，保持原样
        clientAddressStr = peerAddr.toString();
      }
    } else {
      // 纯 IPv4 地址
      clientAddressStr = peerAddr.toString();
    }

    // 构建客户端地址字符串，预分配内存
    QString clientAddress;
    clientAddress.reserve(clientAddressStr.size() + 6); // IP + ':' + 端口(最多5位)
    clientAddress.append(clientAddressStr)
                 .append(':')
                 .append(QString::number(clientSocket->peerPort()));

    emit clientConnected(socketDescriptor, clientAddress);
    qDebug() << "新客户端连接:" << clientAddress << "ID:" << socketDescriptor;
  }
}

void TCPServer::onClientReadyRead() {
  QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
  if (!clientSocket) {
    return;
  }

  // 解析接收到的数据（处理黏包和半包）
  parseReceivedData(clientSocket);
}

void TCPServer::onClientDisconnected() {
  QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
  if (!clientSocket) {
    return;
  }

  qintptr socketDescriptor = clientSocket->socketDescriptor();

  // 从映射表中移除
  m_clients.remove(socketDescriptor);
  m_receiveBuffers.remove(socketDescriptor);

  emit clientDisconnected(socketDescriptor);
  qDebug() << "客户端断开连接，ID:" << socketDescriptor;

  // 延迟删除socket
  clientSocket->deleteLater();
}
