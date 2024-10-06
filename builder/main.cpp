#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStyleHints>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("Basic");
    QGuiApplication::setFont(QFont("Roboto Mono"));

    QQmlApplicationEngine engine;
    engine.setOutputWarningsToStandardError(true);
    const QUrl url(u"qrc:/Builder/view/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            &app, [url, &engine](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                {
                    const auto error = engine.catchError();
                    QDebug(QtFatalMsg) << error.toString();
                    QCoreApplication::exit(-1);
                }
            }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
