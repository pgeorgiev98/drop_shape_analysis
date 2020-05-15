#include "backend.h"
#include "dropgenerator.h"
#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    a.setApplicationName("drop-shape-analysis");
    a.setOrganizationName("drop-shape-analysis");
    a.setOrganizationDomain("drop-shape-analysis.com");
    qRegisterMetaType<TheoreticalModelParameters>();
    qmlRegisterType<Backend>("DropShapeAnalysis.Backend", 1, 0, "Backend");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &a, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return a.exec();
}
