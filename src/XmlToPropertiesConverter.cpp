#include "XmlToPropertiesConverter.h"
#include "PropertiesEscaper.h"

#include <QFile>
#include <QHash>
#include <QSaveFile>
#include <QUrl>
#include <QStringDecoder>
#include <QStringList>
#include <QVector>
#include <QXmlStreamReader>

#include <functional>
#include <memory>
#include <vector>

namespace {

struct Node
{
    QString name;
    QString text;
    QVector<QPair<QString, QString>> attributes;
    std::vector<std::unique_ptr<Node>> children;
};

bool isWhitespaceOnly(const QString &text)
{
    return text.trimmed().isEmpty();
}

} // namespace

bool XmlToPropertiesInternal::resolveLocalFilePath(const QString &input, QString *path, QString *error)
{
    // QUrl treats a Windows drive letter as a URL scheme. Keep ordinary
    // drive and UNC paths as paths, while handing actual file URLs to Qt so
    // that encoded characters and UNC hosts are converted correctly.
    const bool isDrivePath = input.size() >= 3
        && input.at(1) == QLatin1Char(':')
        && ((input.at(0) >= QLatin1Char('A') && input.at(0) <= QLatin1Char('Z'))
            || (input.at(0) >= QLatin1Char('a') && input.at(0) <= QLatin1Char('z')))
        && (input.at(2) == QLatin1Char('/') || input.at(2) == QLatin1Char('\\'));
    const bool isUncPath = input.startsWith(QStringLiteral("\\\\"))
        || input.startsWith(QStringLiteral("//"));

    const QUrl url(input);
    if (!isDrivePath && !isUncPath && !url.scheme().isEmpty()) {
        if (!url.isLocalFile()) {
            *error = QStringLiteral("仅支持本地文件路径或 file:// 文件 URL");
            return false;
        }
        *path = url.toLocalFile();
    } else {
        *path = input;
    }
    return true;
}

XmlToPropertiesConverter::XmlToPropertiesConverter(QObject *parent)
    : QObject(parent)
{
}

QString XmlToPropertiesConverter::lastError() const
{
    return m_lastError;
}

void XmlToPropertiesConverter::clearError()
{
    setError(QString());
}

void XmlToPropertiesConverter::setError(const QString &error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}

QString XmlToPropertiesConverter::convert(const QString &xmlText)
{
    return convert(xmlText, false);
}

QString XmlToPropertiesConverter::convert(const QString &xmlText, bool ignoreRoot)
{
    clearError();

    QXmlStreamReader reader(xmlText);
    std::unique_ptr<Node> root;
    std::vector<Node *> stack;

    while (!reader.atEnd()) {
        const QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            auto node = std::make_unique<Node>();
            node->name = reader.qualifiedName().toString();
            const QXmlStreamAttributes attributes = reader.attributes();
            for (const QXmlStreamAttribute &attribute : attributes)
                node->attributes.append({attribute.qualifiedName().toString(), attribute.value().toString()});

            Node *nodePointer = node.get();
            if (stack.empty()) {
                if (root) {
                    setError(QStringLiteral("XML contains more than one root element"));
                    return QString();
                }
                root = std::move(node);
            } else {
                stack.back()->children.push_back(std::move(node));
            }
            stack.push_back(nodePointer);
        } else if (token == QXmlStreamReader::EndElement) {
            if (stack.empty()) {
                setError(QStringLiteral("XML has an unexpected closing element"));
                return QString();
            }
            stack.pop_back();
        } else if (token == QXmlStreamReader::Characters
                   || token == QXmlStreamReader::EntityReference
                   || token == QXmlStreamReader::DTD) {
            if (!stack.empty())
                stack.back()->text += reader.text().toString();
        }
    }

    if (reader.hasError()) {
        setError(reader.errorString());
        return QString();
    }
    if (!root || !stack.empty()) {
        setError(QStringLiteral("XML does not contain a complete root element"));
        return QString();
    }

    QStringList lines;
    std::function<void(const Node &, const QString &)> appendNode;
    std::function<void(const Node &, const QString &)> appendChildren;
    appendNode = [&lines, &appendNode, &appendChildren](const Node &node, const QString &path) {
        for (const auto &attribute : node.attributes) {
            lines.append(XmlToPropertiesInternal::escapeKey(path + QStringLiteral(".@") + attribute.first)
                         + QLatin1Char('=') + XmlToPropertiesInternal::escapeValue(attribute.second));
        }

        if (node.children.empty()) {
            const QString value = isWhitespaceOnly(node.text) ? QString() : node.text;
            lines.append(XmlToPropertiesInternal::escapeKey(path) + QLatin1Char('=')
                         + XmlToPropertiesInternal::escapeValue(value));
            return;
        }

        if (!isWhitespaceOnly(node.text))
            lines.append(XmlToPropertiesInternal::escapeKey(path) + QLatin1Char('=')
                         + XmlToPropertiesInternal::escapeValue(node.text));

        appendChildren(node, path);
    };

    appendChildren = [&appendNode](const Node &node, const QString &parentPath) {
        QHash<QString, int> siblingCounts;
        for (const auto &child : node.children)
            ++siblingCounts[child->name];

        QHash<QString, int> siblingIndexes;
        for (const auto &child : node.children) {
            QString childPath = parentPath.isEmpty()
                ? child->name
                : parentPath + QLatin1Char('.') + child->name;
            if (siblingCounts.value(child->name) > 1)
                childPath += QStringLiteral("[%1]").arg(siblingIndexes[child->name]++);
            appendNode(*child, childPath);
        }
    };

    if (ignoreRoot)
        appendChildren(*root, QString());
    else
        appendNode(*root, root->name);
    return lines.join(QLatin1Char('\n'));
}

QString XmlToPropertiesConverter::readTextFile(const QString &filePath)
{
    clearError();
    QString localPath;
    QString pathError;
    if (!XmlToPropertiesInternal::resolveLocalFilePath(filePath, &localPath, &pathError)) {
        setError(pathError);
        return QString();
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("无法读取文件 \"%1\": %2").arg(filePath, file.errorString()));
        return QString();
    }
    const QByteArray data = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        setError(QStringLiteral("读取文件 \"%1\" 失败: %2").arg(filePath, file.errorString()));
        return QString();
    }
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString decoded = decoder.decode(data);
    if (decoder.hasError()) {
        setError(QStringLiteral("文件 \"%1\" 包含无效的 UTF-8 数据").arg(filePath));
        return QString();
    }
    return decoded;
}

bool XmlToPropertiesConverter::writeTextFile(const QString &filePath, const QString &contents)
{
    clearError();
    QString localPath;
    QString pathError;
    if (!XmlToPropertiesInternal::resolveLocalFilePath(filePath, &localPath, &pathError)) {
        setError(pathError);
        return false;
    }

    QSaveFile file(localPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(QStringLiteral("无法写入文件 \"%1\": %2").arg(filePath, file.errorString()));
        return false;
    }

    const QByteArray data = contents.toUtf8();
    if (file.write(data) != data.size() || !file.commit()) {
        setError(QStringLiteral("写入文件 \"%1\" 失败: %2").arg(filePath, file.errorString()));
        return false;
    }
    return true;
}
