#pragma once

#include <QObject>
#include <QString>

namespace XmlToPropertiesInternal
{
bool resolveLocalFilePath(const QString &input, QString *path, QString *error);
}

class XmlToPropertiesConverter final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit XmlToPropertiesConverter(QObject *parent = nullptr);

    QString lastError() const;

    Q_INVOKABLE QString convert(const QString &xmlText);
    Q_INVOKABLE QString convert(const QString &xmlText, bool ignoreRoot);
    Q_INVOKABLE QString readTextFile(const QString &filePath);
    Q_INVOKABLE bool writeTextFile(const QString &filePath, const QString &contents);

signals:
    void lastErrorChanged();

private:
    void clearError();
    void setError(const QString &error);

    QString m_lastError;
};
