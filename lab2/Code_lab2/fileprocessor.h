#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QSize>
#include <QImageReader>
#include <QImage>
#include <QThread>

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(const QString &folderPath, QObject *parent = nullptr);

public slots:
    void processFiles();

signals:
    void progressUpdated(int percent, const QString &filename);
    void fileProcessed(const QString &filename, const QSize &size,
                       int dpiX, int dpiY, int colorDepth,
                       const QString &format, const QString &compressionType,
                       double compressionRatio);
    void finished();

private:
    QString folderPath;

    QStringList getImageFiles();
    int calculateColorDepth(const QImage &image);
    QString getCompressionType(const QString &filePath);
    double calculateCompressionRatio(const QString &filePath);
    void getDpiInfo(const QString &filePath, int &dpiX, int &dpiY);
    QString getFormatFromExtension(const QString &filePath);
};

#endif
