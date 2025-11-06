#ifndef TCPCLIENTCONTROLLER_H
#define TCPCLIENTCONTROLLER_H

#include <QObject>
#include <QString>
#include "TCPClient.h"

class TCPClientController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)

  Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged)

  Q_PROPERTY(QString log READ log NOTIFY logChanged)

public:
  explicit TCPClientController(QObject *parent = nullptr);

  ~TCPClientController() override;

  bool isConnected() const;

  bool autoReconnect() const;

  void setAutoReconnect(bool enable);

  QString log() const;

  // QML可调用的方法
  Q_INVOKABLE void connectToServer(const QString &host, int port);

  Q_INVOKABLE void disconnectFromServer();

  Q_INVOKABLE void sendMessage(const QString &message);

  Q_INVOKABLE void clearLog();

signals:
  void connectedChanged();

  void autoReconnectChanged();

  void logChanged();

private
slots:
  void onConnected();

  void onDisconnected();

  void onMessageReceived(const QString &message);

  void onErrorOccurred(const QString &error);

  void onReconnecting();

private:
  void appendLog(const QString &text);

private:
  TCPClient *m_client;
  bool m_autoReconnect;
  QString m_log;
};

#endif // TCPCLIENTCONTROLLER_H
