#pragma once

#include <QString>

namespace XmlToPropertiesInternal {

inline QString unicodeEscape(QChar character)
{
    return QStringLiteral("\\u%1").arg(static_cast<int>(character.unicode()), 4, 16, QLatin1Char('0'));
}

inline QString escapeKey(const QString &value)
{
    QString result;
    result.reserve(value.size());
    for (const QChar character : value) {
        switch (character.unicode()) {
        case '\\': result += QStringLiteral("\\\\"); break;
        case '\t': result += QStringLiteral("\\t"); break;
        case '\n': result += QStringLiteral("\\n"); break;
        case '\r': result += QStringLiteral("\\r"); break;
        case '\f': result += QStringLiteral("\\f"); break;
        case ' ': case '=': case ':': case '#': case '!':
            result += QLatin1Char('\\');
            result += character;
            break;
        default:
            if (character.unicode() < 0x20 || character.unicode() == 0x7f)
                result += unicodeEscape(character);
            else
                result += character;
            break;
        }
    }
    return result;
}

inline QString escapeValue(const QString &value)
{
    QString result;
    result.reserve(value.size());
    bool leadingWhitespace = true;
    for (const QChar character : value) {
        switch (character.unicode()) {
        case '\\': result += QStringLiteral("\\\\"); break;
        case '\t': result += QStringLiteral("\\t"); break;
        case '\n': result += QStringLiteral("\\n"); break;
        case '\r': result += QStringLiteral("\\r"); break;
        case '\f': result += QStringLiteral("\\f"); break;
        case '=': case ':':
            result += QLatin1Char('\\');
            result += character;
            break;
        case ' ':
            if (leadingWhitespace)
                result += QLatin1Char('\\');
            result += character;
            break;
        default:
            if (character.unicode() < 0x20 || character.unicode() == 0x7f)
                result += unicodeEscape(character);
            else
                result += character;
            break;
        }
        if (!character.isSpace())
            leadingWhitespace = false;
    }
    return result;
}

} // namespace XmlToPropertiesInternal
