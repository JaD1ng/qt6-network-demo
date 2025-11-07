#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "TCPServerController.h"
#include "TCPClientController.h"
#include "UDPController.h"

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  // 设置应用程序图标
  app.setWindowIcon(QIcon(":/icon.png"));

  // 注册 QML 类型
  qmlRegisterType<TCPServerController>("NetworkDemo", 1, 0, "TCPServerController");
  qmlRegisterType<TCPClientController>("NetworkDemo", 1, 0, "TCPClientController");
  qmlRegisterType<UDPController>("NetworkDemo", 1, 0, "UDPController");

  QQmlApplicationEngine engine;

  // 添加 Qt QML 模块的导入路径
#ifdef QT_QML_IMPORT_PATH
  engine.addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif

  // 加载主 QML 文件
  const QUrl url(QStringLiteral("qrc:/qt/qml/NetworkDemo/qml/main.qml"));
  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreated,
    &app, [url](QObject *obj, const QUrl &objUrl) {
      if (!obj && url == objUrl)
        QCoreApplication::exit(-1);
    },
    Qt::QueuedConnection);

  engine.load(url);

  return app.exec();
}
