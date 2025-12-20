#ifndef JPEGPARSER_H
#define JPEGPARSER_H

#include <QVector>
#include <QByteArray>
#include <QFile>
#include <QDebug>

struct QuantizationTable {
    int precision;
    int destination;
    QVector<QVector<int>> table;
};

class JpegParser
{
public:
    static QVector<QuantizationTable> extractQuantizationTables(const QString &filePath);
    static QString quantizationTablesToString(const QVector<QuantizationTable> &tables);

private:
    static bool readJpegMarker(QFile &file, quint16 &marker, quint16 &length);
    static QVector<QVector<int>> parseQuantizationTable(QFile &file, int precision);
};

#endif
