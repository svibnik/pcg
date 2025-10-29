#include "colorconverter.h"
#include <algorithm>

const QVector3D ColorConverter::D65(95.047, 100.000, 108.883);

QColor ColorConverter::hsvToRgb(double h, double s, double v)
{
    h = std::fmod(h, 360.0);
    if (h < 0) h += 360.0;
    s = std::clamp(s, 0.0, 100.0);
    v = std::clamp(v, 0.0, 100.0);

    double c = (v / 100.0) * (s / 100.0);
    double x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
    double m = (v / 100.0) - c;

    double r, g, b;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return QColor::fromRgbF(
        std::clamp(r + m, 0.0, 1.0),
        std::clamp(g + m, 0.0, 1.0),
        std::clamp(b + m, 0.0, 1.0)
        );
}

QVector3D ColorConverter::rgbToHsv(const QColor& rgb)
{
    double r = rgb.redF();
    double g = rgb.greenF();
    double b = rgb.blueF();

    double max = std::max({r, g, b});
    double min = std::min({r, g, b});
    double delta = max - min;

    double h = 0, s = 0, v = max * 100;

    if (delta > 1e-6) {
        s = (max > 0) ? (delta / max) * 100 : 0;

        if (max == r) h = 60 * std::fmod(((g - b) / delta), 6);
        else if (max == g) h = 60 * (((b - r) / delta) + 2);
        else h = 60 * (((r - g) / delta) + 4);

        if (h < 0) h += 360;
    }

    return QVector3D(h, s, v);
}

QVector3D ColorConverter::rgbToXyz(const QColor& rgb)
{
    double r = rgb.redF();
    double g = rgb.greenF();
    double b = rgb.blueF();

    auto linearize = [](double c) {
        return (c > 0.04045) ? std::pow((c + 0.055) / 1.055, 2.4) : c / 12.92;
    };

    r = linearize(r);
    g = linearize(g);
    b = linearize(b);

    double x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
    double y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
    double z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;

    return QVector3D(x * 100, y * 100, z * 100);
}

QColor ColorConverter::xyzToRgb(const QVector3D& xyz)
{
    double x = xyz.x() / 100.0;
    double y = xyz.y() / 100.0;
    double z = xyz.z() / 100.0;

    double r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    double g = x * -0.9692660 + y * 1.8760108 + z * 0.0415560;
    double b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    auto delinearize = [](double c) {
        return (c > 0.0031308) ? (1.055 * std::pow(c, 1/2.4) - 0.055) : 12.92 * c;
    };

    r = std::clamp(delinearize(r), 0.0, 1.0);
    g = std::clamp(delinearize(g), 0.0, 1.0);
    b = std::clamp(delinearize(b), 0.0, 1.0);

    return QColor::fromRgbF(r, g, b);
}

double ColorConverter::labF(double t)
{
    return (t > 0.008856) ? std::pow(t, 1.0/3.0) : (7.787 * t + 16.0/116.0);
}

double ColorConverter::labFInv(double t)
{
    return (t > 0.2068966) ? std::pow(t, 3.0) : ((t - 16.0/116.0) / 7.787);
}

QVector3D ColorConverter::xyzToLab(const QVector3D& xyz)
{
    double x = xyz.x() / D65.x();
    double y = xyz.y() / D65.y();
    double z = xyz.z() / D65.z();

    double fx = labF(x);
    double fy = labF(y);
    double fz = labF(z);

    double l = 116 * fy - 16;
    double a = 500 * (fx - fy);
    double b_val = 200 * (fy - fz);

    return QVector3D(l, a, b_val);
}

QVector3D ColorConverter::labToXyz(const QVector3D& lab)
{
    double l = lab.x();
    double a_val = lab.y();
    double b_val = lab.z();

    double fy = (l + 16) / 116;
    double fx = a_val / 500 + fy;
    double fz = fy - b_val / 200;

    double x = D65.x() * labFInv(fx);
    double y = D65.y() * labFInv(fy);
    double z = D65.z() * labFInv(fz);

    return QVector3D(x, y, z);
}

QVector3D ColorConverter::hsvToXyz(double h, double s, double v)
{
    QColor rgb = hsvToRgb(h, s, v);
    return rgbToXyz(rgb);
}

QVector3D ColorConverter::xyzToHsv(const QVector3D& xyz)
{
    QColor rgb = xyzToRgb(xyz);
    return rgbToHsv(rgb);
}

QVector3D ColorConverter::labToHsv(const QVector3D& lab)
{
    QVector3D xyz = labToXyz(lab);
    QColor rgb = xyzToRgb(xyz);
    return rgbToHsv(rgb);
}

QVector3D ColorConverter::hsvToLab(double h, double s, double v)
{
    QVector3D xyz = hsvToXyz(h, s, v);
    return xyzToLab(xyz);
}
