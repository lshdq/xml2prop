#include "XmlToPropertiesConverter.h"
#include "PropertiesEscaper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

class XmlToPropertiesConverterTest : public QObject
{
    Q_OBJECT

private slots:
    void nestedElements();
    void ignoreRoot();
    void ignoreRootContentAndRepeatedChildren();
    void attributes();
    void repeatedSiblings();
    void specialCharacters();
    void propertiesEscaping();
    void whitespaceOnlyNodes();
    void invalidXml();
    void emptyLeaf();
    void utf8FileRoundTrip();
    void invalidUtf8File();
    void fileErrors();
    void failedWritePreservesExistingDirectoryContents();
    void fileUrls();
    void fileUrlClassification();
};

void XmlToPropertiesConverterTest::nestedElements()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<server><database><host>db.example</host></database></server>")),
             QStringLiteral("server.database.host=db.example"));
    QVERIFY(converter.lastError().isEmpty());
}

void XmlToPropertiesConverterTest::ignoreRoot()
{
    XmlToPropertiesConverter converter;
    const QString xml = QStringLiteral("<root><value>ok</value></root>");

    QCOMPARE(converter.convert(xml), QStringLiteral("root.value=ok"));
    QCOMPARE(converter.convert(xml, true), QStringLiteral("value=ok"));
    QCOMPARE(converter.convert(xml, false), QStringLiteral("root.value=ok"));
}

void XmlToPropertiesConverterTest::ignoreRootContentAndRepeatedChildren()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral(
                 "<root flag=\"x\">text<item a=\"1\"><name>one</name></item>"
                 "<item a=\"2\"><name>two</name></item></root>"), true),
             QStringLiteral("item[0].@a=1\nitem[0].name=one\nitem[1].@a=2\nitem[1].name=two"));
    QCOMPARE(converter.convert(QStringLiteral("<root flag=\"x\">text</root>"), true), QString());
}

void XmlToPropertiesConverterTest::attributes()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<server port=\"8080\"><host>localhost</host></server>")),
             QStringLiteral("server.@port=8080\nserver.host=localhost"));
}

void XmlToPropertiesConverterTest::repeatedSiblings()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<items><item>one</item><item>two</item></items>")),
             QStringLiteral("items.item[0]=one\nitems.item[1]=two"));
}

void XmlToPropertiesConverterTest::specialCharacters()
{
    XmlToPropertiesConverter converter;
    const QString xml = QStringLiteral("<root><value> \\path\nline</value></root>");
    QCOMPARE(converter.convert(xml), QStringLiteral("root.value=\\ \\\\path\\nline"));
}

void XmlToPropertiesConverterTest::propertiesEscaping()
{
    XmlToPropertiesConverter converter;
    const QString xml = QStringLiteral(
        "<root:node xmlns:root=\"urn:root\" xmlns:attr=\"urn:attr\" attr:name=\"=:value\">"
        "<child>=:\t\n\r&#x7F;</child></root:node>");

    QCOMPARE(converter.convert(xml),
             QStringLiteral("root\\:node.@attr\\:name=\\=\\:value\n"
                            "root\\:node.child=\\=\\:\\t\\n\\n\\u007f"));
    QVERIFY(converter.lastError().isEmpty());

    const QString controls = QStringLiteral(" =:\t\r\f") + QChar(0x01) + QChar(0x7f);
    QCOMPARE(XmlToPropertiesInternal::escapeKey(controls),
             QStringLiteral("\\ \\=\\:\\t\\r\\f\\u0001\\u007f"));
    QCOMPARE(XmlToPropertiesInternal::escapeValue(controls),
             QStringLiteral("\\ \\=\\:\\t\\r\\f\\u0001\\u007f"));
}

void XmlToPropertiesConverterTest::whitespaceOnlyNodes()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<root>\n  <empty> \t\r\n </empty>\n</root>")),
             QStringLiteral("root.empty="));
}

void XmlToPropertiesConverterTest::invalidXml()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<root><broken></root>")), QString());
    QVERIFY(!converter.lastError().isEmpty());
}

void XmlToPropertiesConverterTest::emptyLeaf()
{
    XmlToPropertiesConverter converter;
    QCOMPARE(converter.convert(QStringLiteral("<root><empty/></root>")),
             QStringLiteral("root.empty="));
}

void XmlToPropertiesConverterTest::utf8FileRoundTrip()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString inputPath = temporaryDirectory.filePath(QStringLiteral("input.xml"));
    const QString outputPath = temporaryDirectory.filePath(QStringLiteral("output.properties"));
    const QString xml = QStringLiteral("<配置><名称>中文</名称></配置>");

    XmlToPropertiesConverter converter;
    QVERIFY(converter.writeTextFile(inputPath, xml));
    QVERIFY(converter.lastError().isEmpty());
    QCOMPARE(converter.readTextFile(inputPath), xml);
    QVERIFY(converter.lastError().isEmpty());

    const QString properties = converter.convert(converter.readTextFile(inputPath));
    QCOMPARE(properties, QStringLiteral("配置.名称=中文"));
    QVERIFY(converter.writeTextFile(outputPath, properties));

    QFile outputFile(outputPath);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(outputFile.readAll()), properties);
}

void XmlToPropertiesConverterTest::invalidUtf8File()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString inputPath = temporaryDirectory.filePath(QStringLiteral("invalid-utf8.xml"));
    QFile inputFile(inputPath);
    QVERIFY(inputFile.open(QIODevice::WriteOnly));
    const QByteArray invalidUtf8 = QByteArrayLiteral("<root>\xC3\x28</root>");
    QCOMPARE(inputFile.write(invalidUtf8), invalidUtf8.size());
    inputFile.close();
    QVERIFY(!inputFile.isOpen());

    XmlToPropertiesConverter converter;
    QCOMPARE(converter.readTextFile(inputPath), QString());
    QVERIFY(converter.lastError().contains(QStringLiteral("UTF-8")));
}

void XmlToPropertiesConverterTest::fileErrors()
{
    XmlToPropertiesConverter converter;
    const QString missingPath = QDir::tempPath() + QStringLiteral("/xml2prop-no-such-file-")
        + QString::number(QCoreApplication::applicationPid()) + QStringLiteral(".xml");

    QCOMPARE(converter.readTextFile(missingPath), QString());
    QVERIFY(!converter.lastError().isEmpty());

    const QString directoryPath = QDir::tempPath();
    QVERIFY(!converter.writeTextFile(directoryPath, QStringLiteral("data")));
    QVERIFY(!converter.lastError().isEmpty());

    QVERIFY(!converter.writeTextFile(QStringLiteral("https://example.invalid/output.properties"),
                                     QStringLiteral("data")));
    QVERIFY(!converter.lastError().isEmpty());
}

void XmlToPropertiesConverterTest::failedWritePreservesExistingDirectoryContents()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString targetDirectoryPath = temporaryDirectory.filePath(QStringLiteral("existing.properties"));
    QVERIFY(QDir().mkpath(targetDirectoryPath));
    const QString sentinelPath = QDir(targetDirectoryPath).filePath(QStringLiteral("sentinel.txt"));
    QFile sentinelFile(sentinelPath);
    QVERIFY(sentinelFile.open(QIODevice::WriteOnly));
    const QByteArray sentinelContents = QByteArrayLiteral("do not replace");
    QCOMPARE(sentinelFile.write(sentinelContents), sentinelContents.size());
    sentinelFile.close();

    XmlToPropertiesConverter converter;
    QVERIFY(!converter.writeTextFile(targetDirectoryPath, QStringLiteral("new contents")));
    QVERIFY(!converter.lastError().isEmpty());

    QFile preservedFile(sentinelPath);
    QVERIFY(preservedFile.open(QIODevice::ReadOnly));
    QCOMPARE(preservedFile.readAll(), sentinelContents);
}

void XmlToPropertiesConverterTest::fileUrls()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString filePath = temporaryDirectory.filePath(QStringLiteral("url file.xml"));
    const QString contents = QStringLiteral("<root><value>内容</value></root>");
    const QString fileUrl = QUrl::fromLocalFile(filePath).toString();

    XmlToPropertiesConverter converter;
    QVERIFY(converter.writeTextFile(fileUrl, contents));
    QCOMPARE(converter.readTextFile(fileUrl), contents);
    QVERIFY(converter.lastError().isEmpty());
}

void XmlToPropertiesConverterTest::fileUrlClassification()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    QString path;
    QString error;
    const QString ordinaryFileUrl = QUrl::fromLocalFile(temporaryDirectory.filePath(QStringLiteral("input.xml"))).toString();
    QVERIFY(XmlToPropertiesInternal::resolveLocalFilePath(ordinaryFileUrl, &path, &error));
    QCOMPARE(path, temporaryDirectory.filePath(QStringLiteral("input.xml")));
    QVERIFY(error.isEmpty());

    path.clear();
    error.clear();
    QVERIFY(XmlToPropertiesInternal::resolveLocalFilePath(QStringLiteral("file://server/share/input.xml"), &path, &error));
    QVERIFY(!path.isEmpty());
    QVERIFY(error.isEmpty());

    path.clear();
    error.clear();
    QVERIFY(!XmlToPropertiesInternal::resolveLocalFilePath(QStringLiteral("https://example.invalid/input.xml"), &path, &error));
    QVERIFY(!error.isEmpty());
}

QTEST_MAIN(XmlToPropertiesConverterTest)
#include "tst_XmlToPropertiesConverter.moc"
