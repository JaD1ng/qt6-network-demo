#include "ClientHandler.h"
#include <QDataStream>
#include <QDebug>
#include <QHostAddress>
#include <QThread>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), m_socketDescriptor(socketDescriptor), m_socket(nullptr) {
  // 预分配接收缓冲区
  m_receiveBuffer.reserve(4096);
}

ClientHandler::~ClientHandler() {
  if (m_socket) {
    m_socket->disconnectFromHost();
    m_socket->deleteLater();
  }
  qDebug() << "[ClientHandler]" << m_socketDescriptor << "析构";
}

void ClientHandler::initialize() {
  // 在目标线程中创建 QTcpSocket
  m_socket = new QTcpSocket(this);

  // 使用 socket 描述符设置连接
  if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
    qWarning() << "[ClientHandler] 设置 socket 描述符失败:"
               << m_socketDescriptor;
    emit errorOccurred(m_socketDescriptor, "设置 socket 描述符失败");
    deleteLater();
    return;
  }

  // 连接信号
  connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
  connect(m_socket, &QTcpSocket::disconnected, this,
          &ClientHandler::onDisconnected);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &ClientHandler::onError);

  // 获取并缓存客户端地址
  QHostAddress peerAddr = m_socket->peerAddress();
  QString clientAddressStr;

  if (peerAddr.protocol() == QAbstractSocket::IPv6Protocol) {
    // 检查是否为 IPv6 映射的 IPv4 地址
    QHostAddress ipv4(peerAddr.toIPv4Address());
    if (!ipv4.isNull()) {
      clientAddressStr = ipv4.toString();
    } else {
      clientAddressStr = peerAddr.toString();
    }
  } else {
    clientAddressStr = peerAddr.toString();
  }

  // 构建完整地址字符串
  m_clientAddress =
      QString("%1:%2").arg(clientAddressStr).arg(m_socket->peerPort());

  qDebug() << "[ClientHandler]" << m_socketDescriptor
           << "初始化完成，地址:" << m_clientAddress
           << "线程:" << QThread::currentThread();

  // 发出就绪信号
  emit ready(m_socketDescriptor, m_clientAddress);
}

void ClientHandler::sendMessage(const QString &message) {
  if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
    qWarning() << "[ClientHandler]" << m_socketDescriptor
               << "socket 未连接，无法发送消息";
    return;
  }

  QByteArray packet = packMessage(message);
  qint64 written = m_socket->write(packet);
  m_socket->flush();

  if (written != packet.size()) {
    qWarning() << "[ClientHandler]" << m_socketDescriptor << "发送消息不完整";
    emit errorOccurred(m_socketDescriptor, "发送消息不完整");
  } else {
    qDebug() << "[ClientHandler]" << m_socketDescriptor
             << "发送消息:" << message << "(字节数:" << packet.size() << ")";
  }
}

void ClientHandler::disconnect() {
  if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->disconnectFromHost();
  }
}

QByteArray ClientHandler::packMessage(const QString &message) {
  // 消息格式：[4字节长度(网络字节序/大端)][消息内容UTF-8]
  QByteArray utf8Data = message.toUtf8();
  quint32 messageLength = static_cast<quint32>(utf8Data.size());

  // 预分配内存
  QByteArray packet;
  packet.reserve(sizeof(quint32) + messageLength);

  QDataStream stream(&packet, QIODevice::WriteOnly);
  stream.setByteOrder(QDataStream::BigEndian);

  stream << messageLength;
  packet.append(utf8Data);

  return packet;
}

void ClientHandler::parseReceivedData() {
  // 读取所有可用数据到缓冲区
  m_receiveBuffer.append(m_socket->readAll());

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
      qWarning() << "[ClientHandler]" << m_socketDescriptor
                 << "收到的消息过大:" << messageLength;
      emit errorOccurred(m_socketDescriptor, "消息过大，断开连接");
      m_socket->disconnectFromHost();
      return;
    }

    // 检查是否接收到完整消息（处理半包）
    int totalSize = static_cast<int>(sizeof(quint32) + messageLength);
    if (m_receiveBuffer.size() < totalSize) {
      // 数据不完整，等待更多数据
      qDebug() << "[ClientHandler]" << m_socketDescriptor
               << "数据不完整，等待..."
               << "已接收:" << m_receiveBuffer.size() << "需要:" << totalSize;

      // 预留足够空间，避免后续频繁分配
      if (m_receiveBuffer.capacity() < totalSize) {
        m_receiveBuffer.reserve(totalSize + 1024);
      }
      break;
    }

    // 提取消息内容（跳过前4字节的长度字段）
    QString message = QString::fromUtf8(
        m_receiveBuffer.constData() + sizeof(quint32), messageLength);

    // 从缓冲区移除已处理的消息（处理黏包）
    m_receiveBuffer.remove(0, totalSize);

    // 发出消息信号
    if (!message.isEmpty()) {
      qDebug() << "[ClientHandler]" << m_socketDescriptor
               << "收到完整消息:" << message;
      emit messageReceived(m_socketDescriptor, message);
    }
  }

  // 缓冲区缩容策略：如果缓冲区空闲空间过大（>8KB）且已用空间较小，则缩容
  constexpr int SHRINK_THRESHOLD = 8192;
  if (m_receiveBuffer.capacity() > SHRINK_THRESHOLD &&
      m_receiveBuffer.size() < 1024) {
    QByteArray temp(m_receiveBuffer);
    m_receiveBuffer = std::move(temp);
    m_receiveBuffer.reserve(4096); // 恢复初始容量
  }
}

void ClientHandler::onReadyRead() { parseReceivedData(); }

void ClientHandler::onDisconnected() {
  qDebug() << "[ClientHandler]" << m_socketDescriptor << "断开连接";
  m_receiveBuffer.clear();
  emit disconnected(m_socketDescriptor);

  // 延迟删除自己
  deleteLater();
}

void ClientHandler::onError(QAbstractSocket::SocketError socketError) {
  // 过滤远程主机关闭连接，这不是错误
  if (socketError == QAbstractSocket::RemoteHostClosedError) {
    qDebug() << "[ClientHandler]" << m_socketDescriptor << "远程主机关闭连接";
    return;
  }

  QString errorString = m_socket->errorString();
  qWarning() << "[ClientHandler]" << m_socketDescriptor
             << "错误:" << errorString;
  emit errorOccurred(m_socketDescriptor, errorString);
}
