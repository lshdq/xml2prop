#include "XmlToPropertiesConverter.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/xml2prop.svg")));

    XmlToPropertiesConverter converter;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("converter"), &converter);
    engine.load(QUrl(QStringLiteral("qrc:/Xml2Prop/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
