#include "XmlToPropertiesConverter.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QPixmap>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QtTest>

class Xml2PropQmlSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void runtimeApplicationIcon();
    void mainWindowWorkflow();
};

void Xml2PropQmlSmokeTest::runtimeApplicationIcon()
{
    const QIcon icon(QStringLiteral(":/icons/xml2prop.svg"));
    QVERIFY2(!icon.isNull(), "The application SVG icon could not be loaded from Qt resources");

    const QPixmap pixmap = icon.pixmap(QSize(32, 32));
    QVERIFY2(!pixmap.isNull(), "Qt could not render a pixmap from the application SVG icon");
    QVERIFY(pixmap.width() > 0);
    QVERIFY(pixmap.height() > 0);
}

void Xml2PropQmlSmokeTest::mainWindowWorkflow()
{
    XmlToPropertiesConverter converter;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("converter"), &converter);
    engine.load(QUrl(QStringLiteral("qrc:/Xml2Prop/qml/Main.qml")));
    QVERIFY2(!engine.rootObjects().isEmpty(), "qml/Main.qml did not load");

    QObject *window = engine.rootObjects().constFirst();
    QVERIFY(window != nullptr);
    QObject *input = window->findChild<QObject *>(QStringLiteral("inputEditor"));
    QObject *output = window->findChild<QObject *>(QStringLiteral("outputEditor"));
    QObject *convert = window->findChild<QObject *>(QStringLiteral("convertButton"));
    QObject *clear = window->findChild<QObject *>(QStringLiteral("clearButton"));
    QObject *ignoreRoot = window->findChild<QObject *>(QStringLiteral("ignoreRootCheckBox"));
    QObject *openDialog = window->findChild<QObject *>(QStringLiteral("openDialog"));
    QObject *saveDialog = window->findChild<QObject *>(QStringLiteral("saveDialog"));
    QObject *status = window->findChild<QObject *>(QStringLiteral("statusLabel"));
    QVERIFY(input != nullptr);
    QVERIFY(output != nullptr);
    QVERIFY(convert != nullptr);
    QVERIFY(clear != nullptr);
    QVERIFY(ignoreRoot != nullptr);
    QCOMPARE(ignoreRoot->property("checked").toBool(), true);
    QVERIFY(openDialog != nullptr);
    QVERIFY(saveDialog != nullptr);
    QVERIFY(status != nullptr);
    QCOMPARE(saveDialog->property("defaultSuffix").toString(), QStringLiteral("properties"));

    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString inputPath = temporaryDirectory.filePath(QStringLiteral("input.xml"));
    const QString outputPath = temporaryDirectory.filePath(QStringLiteral("output"));
    QFile inputFile(inputPath);
    QVERIFY(inputFile.open(QIODevice::WriteOnly));
    const QByteArray xml = QByteArrayLiteral("<root><value>loaded</value></root>");
    QCOMPARE(inputFile.write(xml), xml.size());
    inputFile.close();

    const QVariant inputUrl = QVariant::fromValue(QUrl::fromLocalFile(inputPath));
    QVERIFY(QMetaObject::invokeMethod(window, "loadXmlFile", Q_ARG(QVariant, inputUrl)));
    QTRY_COMPARE(input->property("text").toString(), QStringLiteral("<root><value>loaded</value></root>"));
    QCOMPARE(window->property("statusError").toBool(), false);

    output->setProperty("text", QStringLiteral("root.value=saved"));
    const QVariant outputUrl = QVariant::fromValue(QUrl::fromLocalFile(outputPath));
    QVERIFY(QMetaObject::invokeMethod(window, "savePropertiesFile", Q_ARG(QVariant, outputUrl)));
    QCOMPARE(window->property("statusError").toBool(), false);
    QFile outputFile(outputPath);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    QCOMPARE(outputFile.readAll(), QByteArrayLiteral("root.value=saved"));

    const QVariant invalidOutputUrl = QVariant::fromValue(QUrl::fromLocalFile(temporaryDirectory.path()));
    QVERIFY(QMetaObject::invokeMethod(window, "savePropertiesFile", Q_ARG(QVariant, invalidOutputUrl)));
    QCOMPARE(window->property("statusError").toBool(), true);
    QVERIFY(!status->property("text").toString().isEmpty());

    const QString missingPath = temporaryDirectory.filePath(QStringLiteral("missing.xml"));
    const QVariant missingUrl = QVariant::fromValue(QUrl::fromLocalFile(missingPath));
    QVERIFY(QMetaObject::invokeMethod(window, "loadXmlFile", Q_ARG(QVariant, missingUrl)));
    QCOMPARE(window->property("statusError").toBool(), true);
    QVERIFY(!status->property("text").toString().isEmpty());

    input->setProperty("text", QStringLiteral("<root><value>ok</value></root>"));
    QVERIFY(QMetaObject::invokeMethod(convert, "click"));
    QTRY_COMPARE(output->property("text").toString(), QStringLiteral("value=ok"));
    QCOMPARE(window->property("statusError").toBool(), false);

    ignoreRoot->setProperty("checked", false);
    QVERIFY(QMetaObject::invokeMethod(convert, "click"));
    QTRY_COMPARE(output->property("text").toString(), QStringLiteral("root.value=ok"));

    output->setProperty("text", QStringLiteral("stale output"));
    input->setProperty("text", QStringLiteral("<root><broken></root>"));
    QVERIFY(QMetaObject::invokeMethod(convert, "click"));
    QTRY_VERIFY(output->property("text").toString().isEmpty());
    QCOMPARE(window->property("statusError").toBool(), true);
    QVERIFY(!status->property("text").toString().isEmpty());

    input->setProperty("text", QStringLiteral("input"));
    output->setProperty("text", QStringLiteral("output"));
    QVERIFY(QMetaObject::invokeMethod(clear, "click"));
    QTRY_VERIFY(input->property("text").toString().isEmpty());
    QCOMPARE(output->property("text").toString(), QString());
    QCOMPARE(window->property("statusError").toBool(), false);
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Xml2PropQmlSmokeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_Xml2PropQmlSmoke.moc"
