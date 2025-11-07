#ifndef UDPCONTROLLER_H
#define UDPCONTROLLER_H

#include <QObject>
#include <QString>
#include "UDPClientServer.h"

class UDPController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isBound READ isBound NOTIFY boundChanged)

  Q_PROPERTY(int localPort READ localPort NOTIFY localPortChanged)

  Q_PROPERTY(QString log READ log NOTIFY logChanged)

public:
  explicit UDPController(QObject *parent = nullptr);

  ~UDPController() override;

  bool isBound() const;

  int localPort() const;

  QString log() const;

  // QML可调用的方法
  Q_INVOKABLE void bind(int port);

  Q_INVOKABLE void unbind();

  Q_INVOKABLE void sendMessage(const QString &message, const QString &targetHost, int targetPort);

  Q_INVOKABLE void sendBroadcast(const QString &message, int targetPort);

  Q_INVOKABLE void clearLog();

signals:
  void boundChanged();

  void localPortChanged();

  void logChanged();

private
slots:
  void onBound(quint16 port);

  void onUnbound();

  void onMessageReceived(const QString &message, const QString &senderAddress, quint16 senderPort);

  void onErrorOccurred(const QString &error);

private:
  void appendLog(const QString &text);

private:
  UDPClientServer *m_udp;
  QString m_log;
};

#endif // UDPCONTROLLER_H
