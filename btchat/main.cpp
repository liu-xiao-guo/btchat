#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QObject>
#include <QQmlContext>

#include <chat.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Chat d;

    QQuickView view;
    view.rootContext()->setContextProperty("chat", &d);
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    view.show();
    return app.exec();
}

