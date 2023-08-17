#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "whiteboard.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Whiteboard>("Bigno", 1, 0, "Whiteboard");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/test4/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
