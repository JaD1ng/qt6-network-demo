#ifndef TCPSERVERCONTROLLER_H
#define TCPSERVERCONTROLLER_H

#include "TCPServer.h"
#include <QObject>
#include <QString>

class TCPServerController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isListening READ isListening NOTIFY listeningChanged)

  Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)

  Q_PROPERTY(QString log READ log NOTIFY logChanged)

public:
  explicit TCPServerController(QObject *parent = nullptr);

  ~TCPServerController() override;

  bool isListening() const;

  int clientCount() const;

  QString log() const;

  // QML可调用的方法
  Q_INVOKABLE void startServer(int port);

  Q_INVOKABLE void stopServer();

  Q_INVOKABLE void sendMessage(qintptr clientId, const QString &message);

  Q_INVOKABLE void broadcastMessage(const QString &message);

  Q_INVOKABLE void clearLog();

signals:
  void listeningChanged();

  void clientCountChanged();

  void logChanged();

private slots:
  void onServerStarted(quint16 port);

  void onServerStopped();

  void onClientConnected(qintptr clientId, const QString &address);

  void onClientDisconnected(qintptr clientId);

  void onMessageReceived(qintptr clientId, const QString &message);

  void onErrorOccurred(const QString &error);

private:
  void appendLog(const QString &text);

private:
  TCPServer *m_server;
  QString m_log;
};

#endif // TCPSERVERCONTROLLER_H
