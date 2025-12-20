#include "jpegparser.h"

QVector<QuantizationTable> JpegParser::extractQuantizationTables(const QString &filePath)
{
    QVector<QuantizationTable> tables;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << filePath;
        return tables;
    }

    QByteArray header = file.read(2);
    if (header[0] != (char)0xFF || header[1] != (char)0xD8) {
        qWarning() << "Not a valid JPEG file:" << filePath;
        return tables;
    }

    while (!file.atEnd()) {
        quint16 marker = 0;
        quint16 length = 0;

        if (!readJpegMarker(file, marker, length)) {
            break;
        }

        if (marker == 0xFFDB) {
            quint64 startPos = file.pos();

            while (file.pos() < startPos + length - 2) {
                if (file.atEnd()) break;

                quint8 tableInfo;
                if (file.read((char*)&tableInfo, 1) != 1) break;

                int precision = (tableInfo >> 4) & 0x0F;
                int destination = tableInfo & 0x0F;

                QuantizationTable qt;
                qt.precision = precision;
                qt.destination = destination;
                qt.table = parseQuantizationTable(file, precision);

                if (!qt.table.isEmpty()) {
                    tables.append(qt);
                }
            }
        }
        else if (marker == 0xFFDA || marker == 0xFFC4) {
            break;
        }
        else if (marker >= 0xFFD0 && marker <= 0xFFD9) {
            continue;
        }
        else {
            if (length >= 2) {
                file.seek(file.pos() + length - 2);
            }
        }
    }

    file.close();
    return tables;
}

bool JpegParser::readJpegMarker(QFile &file, quint16 &marker, quint16 &length)
{
    char byte;
    while (file.read(&byte, 1) == 1) {
        if ((quint8)byte == 0xFF) {
            if (file.read(&byte, 1) != 1) return false;

            while ((quint8)byte == 0xFF) {
                if (file.read(&byte, 1) != 1) return false;
            }

            marker = 0xFF00 | (quint8)byte;

            if (marker == 0xFF01 ||
                (marker >= 0xFFD0 && marker <= 0xFFD7) ||
                marker == 0xFFD8 ||
                marker == 0xFFD9) {
                length = 0;
                return true;
            }

            if (file.read((char*)&length, 2) != 2) return false;
            length = ((length >> 8) & 0x00FF) | ((length << 8) & 0xFF00);

            return true;
        }
    }

    return false;
}

QVector<QVector<int>> JpegParser::parseQuantizationTable(QFile &file, int precision)
{
    QVector<QVector<int>> table(8, QVector<int>(8, 0));

    for (int i = 0; i < 64; i++) {
        if (precision == 0) {
            quint8 value;
            if (file.read((char*)&value, 1) != 1) {
                return QVector<QVector<int>>();
            }
            table[i / 8][i % 8] = value;
        } else {
            quint16 value;
            if (file.read((char*)&value, 2) != 2) {
                return QVector<QVector<int>>();
            }
            value = ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
            table[i / 8][i % 8] = value;
        }
    }

    return table;
}

QString JpegParser::quantizationTablesToString(const QVector<QuantizationTable> &tables)
{
    if (tables.isEmpty()) {
        return "";
    }

    const QuantizationTable &qt = tables.first();
    QString result;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            result += QString("%1").arg(qt.table[row][col], 5);
        }
        if (row < 7) result += "\n";
    }

    return result;
}
