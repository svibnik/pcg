#ifndef COLORCONVERTER_H
#define COLORCONVERTER_H

#include <QColor>
#include <QVector3D>
#include <cmath>

class ColorConverter
{
public:
    static QColor hsvToRgb(double h, double s, double v);
    static QVector3D rgbToHsv(const QColor& rgb);

    static QVector3D rgbToXyz(const QColor& rgb);
    static QColor xyzToRgb(const QVector3D& xyz);

    static QVector3D xyzToLab(const QVector3D& xyz);
    static QVector3D labToXyz(const QVector3D& lab);

    static QVector3D hsvToXyz(double h, double s, double v);
    static QVector3D xyzToHsv(const QVector3D& xyz);
    static QVector3D labToHsv(const QVector3D& lab);
    static QVector3D hsvToLab(double h, double s, double v);

private:
    static const QVector3D D65;
    static double labF(double t);
    static double labFInv(double t);
};

#endif
