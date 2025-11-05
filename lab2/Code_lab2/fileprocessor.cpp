#include "fileprocessor.h"
#include <QFileInfo>

FileProcessor::FileProcessor(const QString &folderPath, QObject *parent)
    : QObject(parent)
    , folderPath(folderPath)
{
}

void FileProcessor::processFiles()
{
    QStringList imageFiles = getImageFiles();
    int totalFiles = imageFiles.size();

    for (int i = 0; i < totalFiles; ++i) {
        const QString &filePath = imageFiles[i];
        QFileInfo fileInfo(filePath);

        QImageReader reader(filePath);

        QSize size(0, 0);
        int dpiX = 72, dpiY = 72;
        int colorDepth = 0;
        QString format = "Unknown";
        QString compression = "Unknown";

        if (reader.canRead()) {
            size = reader.size();
            format = reader.format().toUpper();
        }

        if (format.isEmpty() || format == "UNKNOWN") {
            format = getFormatFromExtension(filePath);
        }

        QImage image;
        if (image.load(filePath)) {
            dpiX = image.dotsPerMeterX() * 0.0254;
            dpiY = image.dotsPerMeterY() * 0.0254;
            colorDepth = calculateColorDepth(image);

            if (dpiX <= 0) dpiX = 72;
            if (dpiY <= 0) dpiY = 72;
        }

        compression = getCompressionInfo(filePath);

        if (size.isEmpty() && !image.isNull()) {
            size = image.size();
        }

        emit fileProcessed(fileInfo.fileName(), size, dpiX, dpiY,
                           colorDepth, compression, format);

        int percent = (i + 1) * 100 / totalFiles;
        emit progressUpdated(percent, fileInfo.fileName());
    }

    emit finished();
}

void FileProcessor::getDpiInfo(const QString &filePath, int &dpiX, int &dpiY)
{
    QImage image;
    if (image.load(filePath)) {
        dpiX = image.dotsPerMeterX() * 0.0254;
        dpiY = image.dotsPerMeterY() * 0.0254;

        if (dpiX <= 0) dpiX = 72;
        if (dpiY <= 0) dpiY = 72;
    } else {
        dpiX = 72;
        dpiY = 72;
    }
}

QStringList FileProcessor::getImageFiles()
{
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif"
            << "*.bmp" << "*.tif" << "*.tiff" << "*.pcx";

    QDir directory(folderPath);
    QStringList files = directory.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);

    QStringList fullPaths;
    for (const QString &file : files) {
        fullPaths.append(directory.absoluteFilePath(file));
    }

    return fullPaths;
}

int FileProcessor::calculateColorDepth(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        return 1;
    case QImage::Format_Indexed8:
        return 8;
    case QImage::Format_RGB16:
        return 16;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        return 32;
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
        return 64;
    default:
        return image.depth();
    }
}

QString FileProcessor::getCompressionInfo(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    if (extension == "jpeg" || extension == "jpg") {
        return "JPEG";
    }
    else if (extension == "png") {
        return "PNG (Deflate)";
    }
    else if (extension == "gif") {
        return "GIF (LZW)";
    }
    else if (extension == "bmp") {
        return "BMP (Uncompressed)";
    }
    else if (extension == "tiff" || extension == "tif") {
        return "TIFF";
    }
    else if (extension == "pcx") {
        return "PCX (RLE)";
    }

    return "Unknown";
}

QString FileProcessor::getFormatFromExtension(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toUpper();

    if (extension == "JPEG" || extension == "JPG") {
        return "JPEG";
    }
    else if (extension == "PNG") {
        return "PNG";
    }
    else if (extension == "GIF") {
        return "GIF";
    }
    else if (extension == "BMP") {
        return "BMP";
    }
    else if (extension == "TIFF" || extension == "TIF") {
        return "TIFF";
    }
    else if (extension == "PCX") {
        return "PCX";
    }

    return "Unknown";
}
