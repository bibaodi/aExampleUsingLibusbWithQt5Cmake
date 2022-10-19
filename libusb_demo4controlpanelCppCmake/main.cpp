#include "usinglibusb.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    QObject *root = engine.rootObjects().value(0);
    QObject *topLevel = root->findChild<QObject *>("buttonObjName");
    if (!topLevel) {
        qDebug() << "Error: not found loader!";
        return -1;
    }
    UsingLibusb *ptr = new UsingLibusb(topLevel);
    ptr->debugQimage();
    return app.exec();
}
