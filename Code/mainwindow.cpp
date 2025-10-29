#include "mainwindow.h"
#include "colorconverter.h"
#include <QStatusBar>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_updating(false)
{
    setupUI();
    setupConnections();

    hSlider->setValue(0);
    sSlider->setValue(100);
    vSlider->setValue(100);

    updateAllViews(0);
}

MainWindow::~MainWindow()
{
}
void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    colorPreview = new QFrame();
    colorPreview->setMinimumHeight(60);
    colorPreview->setFrameStyle(QFrame::Box);
    colorPreview->setStyleSheet("background-color: red; border: 2px solid black;");
    mainLayout->addWidget(colorPreview);

    colorPickerButton = new QPushButton("🎨 Выбрать цвет из палитры");
    colorPickerButton->setStyleSheet("QPushButton { font-size: 12px; padding: 6px; font-weight: bold; }");
    mainLayout->addWidget(colorPickerButton);

    mainLayout->addSpacing(20);

    QGroupBox *hsvGroup = new QGroupBox("HSV Model");
    QGridLayout *hsvLayout = new QGridLayout(hsvGroup);

    hsvLayout->addWidget(new QLabel("H (0-360):"), 0, 0);
    hSlider = new QSlider(Qt::Horizontal);
    hSlider->setRange(0, 360);
    hsvLayout->addWidget(hSlider, 0, 1);
    hSpinBox = new QDoubleSpinBox();
    hSpinBox->setRange(0, 360);
    hSpinBox->setDecimals(3);
    hsvLayout->addWidget(hSpinBox, 0, 2);

    hsvLayout->addWidget(new QLabel("S (0-100):"), 1, 0);
    sSlider = new QSlider(Qt::Horizontal);
    sSlider->setRange(0, 100);
    hsvLayout->addWidget(sSlider, 1, 1);
    sSpinBox = new QDoubleSpinBox();
    sSpinBox->setRange(0, 100);
    sSpinBox->setDecimals(3);
    hsvLayout->addWidget(sSpinBox, 1, 2);

    hsvLayout->addWidget(new QLabel("V (0-100):"), 2, 0);
    vSlider = new QSlider(Qt::Horizontal);
    vSlider->setRange(0, 100);
    hsvLayout->addWidget(vSlider, 2, 1);
    vSpinBox = new QDoubleSpinBox();
    vSpinBox->setRange(0, 100);
    vSpinBox->setDecimals(3);
    hsvLayout->addWidget(vSpinBox, 2, 2);

    mainLayout->addWidget(hsvGroup);

    QGroupBox *xyzGroup = new QGroupBox("XYZ Model");
    QGridLayout *xyzLayout = new QGridLayout(xyzGroup);

    xyzLayout->addWidget(new QLabel("X (0-100):"), 0, 0);
    xSlider = new QSlider(Qt::Horizontal);
    xSlider->setRange(0, 10000);
    xyzLayout->addWidget(xSlider, 0, 1);
    xSpinBox = new QDoubleSpinBox();
    xSpinBox->setRange(0, 100);
    xSpinBox->setDecimals(3);
    xyzLayout->addWidget(xSpinBox, 0, 2);

    xyzLayout->addWidget(new QLabel("Y (0-100):"), 1, 0);
    ySlider = new QSlider(Qt::Horizontal);
    ySlider->setRange(0, 10000);
    xyzLayout->addWidget(ySlider, 1, 1);
    ySpinBox = new QDoubleSpinBox();
    ySpinBox->setRange(0, 100);
    ySpinBox->setDecimals(3);
    xyzLayout->addWidget(ySpinBox, 1, 2);

    xyzLayout->addWidget(new QLabel("Z (0-100):"), 2, 0);
    zSlider = new QSlider(Qt::Horizontal);
    zSlider->setRange(0, 10000);
    xyzLayout->addWidget(zSlider, 2, 1);
    zSpinBox = new QDoubleSpinBox();
    zSpinBox->setRange(0, 100);
    zSpinBox->setDecimals(3);
    xyzLayout->addWidget(zSpinBox, 2, 2);

    mainLayout->addWidget(xyzGroup);

    QGroupBox *labGroup = new QGroupBox("LAB Model");
    QGridLayout *labLayout = new QGridLayout(labGroup);

    labLayout->addWidget(new QLabel("L (0-100):"), 0, 0);
    lSlider = new QSlider(Qt::Horizontal);
    lSlider->setRange(0, 10000);
    labLayout->addWidget(lSlider, 0, 1);
    lSpinBox = new QDoubleSpinBox();
    lSpinBox->setRange(0, 100);
    lSpinBox->setDecimals(2);
    labLayout->addWidget(lSpinBox, 0, 2);

    labLayout->addWidget(new QLabel("A (-128-127):"), 1, 0);
    aSlider = new QSlider(Qt::Horizontal);
    aSlider->setRange(-12800, 12700);
    labLayout->addWidget(aSlider, 1, 1);
    aSpinBox = new QDoubleSpinBox();
    aSpinBox->setRange(-128, 127);
    aSpinBox->setDecimals(2);
    labLayout->addWidget(aSpinBox, 1, 2);

    labLayout->addWidget(new QLabel("B (-128-127):"), 2, 0);
    bSlider = new QSlider(Qt::Horizontal);
    bSlider->setRange(-12800, 12700);
    labLayout->addWidget(bSlider, 2, 1);
    bSpinBox = new QDoubleSpinBox();
    bSpinBox->setRange(-128, 127);
    bSpinBox->setDecimals(2);
    labLayout->addWidget(bSpinBox, 2, 2);

    mainLayout->addWidget(labGroup);

    mainLayout->addStretch();

    setWindowTitle("Color Converter - HSV ↔ XYZ ↔ LAB");

    warningLabel = new QLabel();
    warningLabel->setStyleSheet("QLabel { color: #d32f2f; background-color: #ffebee; padding: 5px; border: 1px solid #d32f2f; border-radius: 3px; }");
    warningLabel->setVisible(false);
    warningLabel->setWordWrap(true);
    mainLayout->addWidget(warningLabel);

    resize(650, 500);
}
void MainWindow::setupConnections()
{
    connect(hSlider, &QSlider::valueChanged, this, [this](int value) {
        hSpinBox->setValue(value);
    });
    connect(hSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        hSlider->setValue(static_cast<int>(value));
    });
    connect(hSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);

    connect(sSlider, &QSlider::valueChanged, this, [this](int value) {
        sSpinBox->setValue(value);
    });
    connect(sSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        sSlider->setValue(static_cast<int>(value));
    });
    connect(sSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);

    connect(vSlider, &QSlider::valueChanged, this, [this](int value) {
        vSpinBox->setValue(value);
    });
    connect(vSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        vSlider->setValue(static_cast<int>(value));
    });
    connect(vSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);

    connect(xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);
    connect(ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);
    connect(zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);

    connect(lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    connect(aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    connect(bSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);

    connect(xSlider, &QSlider::valueChanged, this, [this](int value) {
        xSpinBox->setValue(value / 100.0);
    });
    connect(xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        xSlider->setValue(static_cast<int>(value * 100));
    });
    connect(xSlider, &QSlider::valueChanged, this, &MainWindow::onXyzChanged);

    connect(ySlider, &QSlider::valueChanged, this, [this](int value) {
        ySpinBox->setValue(value / 100.0);
    });
    connect(ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        ySlider->setValue(static_cast<int>(value * 100));
    });
    connect(ySlider, &QSlider::valueChanged, this, &MainWindow::onXyzChanged);

    connect(zSlider, &QSlider::valueChanged, this, [this](int value) {
        zSpinBox->setValue(value / 100.0);
    });
    connect(zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        zSlider->setValue(static_cast<int>(value * 100));
    });
    connect(zSlider, &QSlider::valueChanged, this, &MainWindow::onXyzChanged);


    connect(lSlider, &QSlider::valueChanged, this, [this](int value) {
        lSpinBox->setValue(value / 100.0);
    });
    connect(lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        lSlider->setValue(static_cast<int>(value * 100));
    });
    connect(lSlider, &QSlider::valueChanged, this, &MainWindow::onLabChanged);

    connect(aSlider, &QSlider::valueChanged, this, [this](int value) {
        aSpinBox->setValue(value / 100.0);
    });
    connect(aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        aSlider->setValue(static_cast<int>(value * 100));
    });
    connect(aSlider, &QSlider::valueChanged, this, &MainWindow::onLabChanged);

    connect(bSlider, &QSlider::valueChanged, this, [this](int value) {
        bSpinBox->setValue(value / 100.0);
    });
    connect(bSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        bSlider->setValue(static_cast<int>(value * 100));
    });
    connect(bSlider, &QSlider::valueChanged, this, &MainWindow::onLabChanged);

    connect(colorPickerButton, &QPushButton::clicked, this, &MainWindow::onColorPickerClicked);


    connect(hSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0 || value > 360) {
            showWarning("H значение должно быть в диапазоне 0-360");
        }
    });

    connect(sSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0 || value > 100) {
            showWarning("S значение должно быть в диапазоне 0-100");
        }
    });

    connect(vSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0 || value > 100) {
            showWarning("V значение должно быть в диапазоне 0-100");
        }
    });

    connect(xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0) {
            showWarning("X значение не может быть отрицательным");
        } else if (value > 150) {
            showWarning("X значение значительно превышает стандартный диапазон");
        }
    });

    connect(ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0) {
            showWarning("Y значение не может быть отрицательным");
        } else if (value > 150) {
            showWarning("Y значение значительно превышает стандартный диапазон");
        }
    });

    connect(zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0) {
            showWarning("Z значение не может быть отрицательным");
        } else if (value > 150) {
            showWarning("Z значение значительно превышает стандартный диапазон");
        }
    });

    connect(lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < 0 || value > 100) {
            showWarning("L значение должно быть в диапазоне 0-100");
        }
    });

    connect(aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < -128 || value > 127) {
            showWarning("A значение должно быть в диапазоне -128..127");
        }
    });

    connect(bSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (value < -128 || value > 127) {
            showWarning("B значение должно быть в диапазоне -128..127");
        }
    });
}

void MainWindow::disconnectAll()
{
    disconnect(hSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);
    disconnect(sSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);
    disconnect(vSlider, &QSlider::valueChanged, this, &MainWindow::onHsvChanged);

    disconnect(xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);
    disconnect(ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);
    disconnect(zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXyzChanged);

    disconnect(lSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    disconnect(aSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    disconnect(bSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
}

void MainWindow::connectAll()
{
    setupConnections();
}

void MainWindow::onHsvChanged()
{
    if (m_updating) return;
    updateAllViews(0);
}

void MainWindow::onXyzChanged()
{
    if (m_updating) return;
    updateAllViews(1);
}

void MainWindow::onLabChanged()
{
    if (m_updating) return;
    updateAllViews(2);
}

void MainWindow::updateAllViews(int sourceModel)
{
    m_updating = true;

    try {
        QColor finalRgb;

        switch(sourceModel) {
        case 0: {
            double h = hSpinBox->value();
            double s = sSpinBox->value();
            double v = vSpinBox->value();

            finalRgb = ColorConverter::hsvToRgb(h, s, v);

            QVector3D xyz = ColorConverter::hsvToXyz(h, s, v);
            xSpinBox->setValue(xyz.x());
            ySpinBox->setValue(xyz.y());
            zSpinBox->setValue(xyz.z());

            QVector3D lab = ColorConverter::hsvToLab(h, s, v);
            lSpinBox->setValue(lab.x());
            aSpinBox->setValue(lab.y());
            bSpinBox->setValue(lab.z());
            break;
        }
        case 1: {
            double x = xSpinBox->value();
            double y = ySpinBox->value();
            double z = zSpinBox->value();
            QVector3D xyz(x, y, z);

            finalRgb = ColorConverter::xyzToRgb(xyz);

            QVector3D hsv = ColorConverter::xyzToHsv(xyz);
            hSpinBox->setValue(hsv.x());
            sSpinBox->setValue(hsv.y());
            vSpinBox->setValue(hsv.z());

            QVector3D lab = ColorConverter::xyzToLab(xyz);
            lSpinBox->setValue(lab.x());
            aSpinBox->setValue(lab.y());
            bSpinBox->setValue(lab.z());
            break;
        }
        case 2: {
            double l = lSpinBox->value();
            double a = aSpinBox->value();
            double b_val = bSpinBox->value();
            QVector3D lab(l, a, b_val);

            finalRgb = ColorConverter::xyzToRgb(ColorConverter::labToXyz(lab));

            QVector3D hsv = ColorConverter::labToHsv(lab);
            hSpinBox->setValue(hsv.x());
            sSpinBox->setValue(hsv.y());
            vSpinBox->setValue(hsv.z());

            QVector3D xyz = ColorConverter::labToXyz(lab);
            xSpinBox->setValue(xyz.x());
            ySpinBox->setValue(xyz.y());
            zSpinBox->setValue(xyz.z());
            break;
        }
        }

        QString style = QString("background-color: %1; border: 2px solid black;").arg(finalRgb.name());
        colorPreview->setStyleSheet(style);

        checkColorBounds(finalRgb);

    } catch (const std::exception& e) {
        showWarning("Ошибка преобразования цвета");
    }

    m_updating = false;
}
void MainWindow::onColorPickerClicked()
{
    m_updating = true;

    QColor color = QColorDialog::getColor(Qt::red, this, "Выберите цвет");

    if (color.isValid()) {
        QVector3D hsv = ColorConverter::rgbToHsv(color);

        hSlider->setValue(static_cast<int>(hsv.x()));
        sSlider->setValue(static_cast<int>(hsv.y()));
        vSlider->setValue(static_cast<int>(hsv.z()));

        updateAllViews(0);
    }

    m_updating = false;
}

void MainWindow::showWarning(const QString& message)
{
    warningLabel->setText("⚠️ " + message);
    warningLabel->setVisible(true);
}

void MainWindow::clearWarning()
{
    warningLabel->setVisible(false);
}

void MainWindow::checkColorBounds(const QColor& color)
{
    bool hasClipping = (color.red() == 255 || color.red() == 0 ||
                        color.green() == 255 || color.green() == 0 ||
                        color.blue() == 255 || color.blue() == 0);

    if (hasClipping) {
        showWarning("Цвет вышел за границы. Произведена корректировка значений.");
    } else {
        clearWarning();
    }
}
