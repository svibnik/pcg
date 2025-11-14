#include "mainwindow.h"
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setWindowTitle("Обработка изображений - Пороговая обработка и Сегментация");
    setMinimumSize(1000, 700);
    resize(1000, 700);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    originalImageLabel = new QLabel("Исходное изображение");
    processedImageLabel = new QLabel("Обработанное изображение");
    originalHistogramLabel = new QLabel("Гистограмма исходного");
    processedHistogramLabel = new QLabel("Гистограмма обработанного");

    originalImageLabel->setAlignment(Qt::AlignCenter);
    processedImageLabel->setAlignment(Qt::AlignCenter);
    originalHistogramLabel->setAlignment(Qt::AlignCenter);
    processedHistogramLabel->setAlignment(Qt::AlignCenter);

    originalImageLabel->setFrameStyle(QFrame::Box);
    processedImageLabel->setFrameStyle(QFrame::Box);
    originalHistogramLabel->setFrameStyle(QFrame::Box);
    processedHistogramLabel->setFrameStyle(QFrame::Box);

    originalImageLabel->setMinimumSize(300, 300);
    processedImageLabel->setMinimumSize(300, 300);
    originalHistogramLabel->setMinimumSize(300, 120);
    processedHistogramLabel->setMinimumSize(300, 120);

    loadButton = new QPushButton("Загрузить изображение");
    saveButton = new QPushButton("Сохранить результат");
    processButton = new QPushButton("Применить обработку");
    resetButton = new QPushButton("Сбросить");

    loadButton->setFixedHeight(35);
    saveButton->setFixedHeight(35);
    processButton->setFixedHeight(40);
    resetButton->setFixedHeight(35);

    processingMethod = new QComboBox();
    processingMethod->addItems({
        "Глобальный порог (по гистограмме)",
        "Глобальный порог (метод Оцу)",
        "Адаптивный порог",
        "Обнаружение границ",
        "Обнаружение линий",
        "Обнаружение точек"
    });
    processingMethod->setFixedHeight(30);

    thresholdSpin = new QSpinBox();
    thresholdSpin->setRange(0, 255);
    thresholdSpin->setValue(128);
    thresholdSpin->setFixedHeight(30);

    kernelSizeSpin = new QSpinBox();
    kernelSizeSpin->setRange(3, 51);
    kernelSizeSpin->setValue(15);
    kernelSizeSpin->setSingleStep(2);
    kernelSizeSpin->setFixedHeight(30);

    alphaSlider = new QSlider(Qt::Horizontal);
    alphaSlider->setRange(30, 80);
    alphaSlider->setValue(67);
    alphaSlider->setTickPosition(QSlider::TicksBelow);
    alphaSlider->setTickInterval(10);
    alphaSlider->setFixedHeight(30);

    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedHeight(20);
    progressBar->setVisible(false);
    progressBar->setTextVisible(true);

    globalParamsGroup = new QGroupBox("Параметры глобального порога");
    globalParamsGroup->setFixedHeight(80);
    QFormLayout *globalLayout = new QFormLayout();
    globalLayout->setContentsMargins(5, 15, 5, 5);
    globalLayout->setSpacing(8);
    globalLayout->addRow("Порог (0-255):", thresholdSpin);
    globalParamsGroup->setLayout(globalLayout);

    adaptiveParamsGroup = new QGroupBox("Параметры адаптивного порога");
    adaptiveParamsGroup->setFixedHeight(120);
    QFormLayout *adaptiveLayout = new QFormLayout();
    adaptiveLayout->setContentsMargins(5, 15, 5, 5);
    adaptiveLayout->setSpacing(8);
    adaptiveLayout->addRow("Размер окна:", kernelSizeSpin);
    adaptiveLayout->addRow("Коэффициент α:", alphaSlider);
    adaptiveParamsGroup->setLayout(adaptiveLayout);

    globalParamsGroup->setVisible(false);
    adaptiveParamsGroup->setVisible(false);

    QGroupBox *methodGroup = new QGroupBox("Метод обработки");
    methodGroup->setFixedHeight(80);
    QVBoxLayout *methodLayout = new QVBoxLayout();
    methodLayout->setContentsMargins(5, 15, 5, 5);
    methodLayout->addWidget(processingMethod);
    methodGroup->setLayout(methodLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(10);
    controlLayout->addWidget(loadButton);
    controlLayout->addWidget(saveButton);
    controlLayout->addWidget(resetButton);
    controlLayout->addWidget(methodGroup);
    controlLayout->addWidget(globalParamsGroup);
    controlLayout->addWidget(adaptiveParamsGroup);
    controlLayout->addWidget(progressBar);
    controlLayout->addWidget(processButton);
    controlLayout->addStretch();

    QVBoxLayout *imageLayout = new QVBoxLayout();
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(5);

    QHBoxLayout *imagesLayout = new QHBoxLayout();
    imagesLayout->setContentsMargins(0, 0, 0, 0);
    imagesLayout->setSpacing(10);

    QVBoxLayout *originalLayout = new QVBoxLayout();
    originalLayout->setContentsMargins(0, 0, 0, 0);
    originalLayout->setSpacing(2);
    originalLayout->addWidget(originalImageLabel);
    originalLayout->addWidget(originalHistogramLabel);

    QVBoxLayout *processedLayout = new QVBoxLayout();
    processedLayout->setContentsMargins(0, 0, 0, 0);
    processedLayout->setSpacing(2);
    processedLayout->addWidget(processedImageLabel);
    processedLayout->addWidget(processedHistogramLabel);

    imagesLayout->addLayout(originalLayout);
    imagesLayout->addLayout(processedLayout);

    imageLayout->addLayout(imagesLayout);

    mainLayout->addLayout(controlLayout, 1);
    mainLayout->addLayout(imageLayout, 2);

    centralWidget->setLayout(mainLayout);

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::applyProcessing);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetImage);
    connect(processingMethod, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMethodChanged);

    onMethodChanged(0);
}

void MainWindow::onMethodChanged(int index)
{
    globalParamsGroup->setVisible(false);
    adaptiveParamsGroup->setVisible(false);

    QString method = processingMethod->itemText(index);
    if (method == "Глобальный порог (по гистограмме)") {
        globalParamsGroup->setVisible(true);
    }
    else if (method == "Адаптивный порог") {
        adaptiveParamsGroup->setVisible(true);
    }
}

void MainWindow::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Загрузить изображение", "", "Файлы изображений (*.png *.jpg *.bmp *.tiff)");

    if (!fileName.isEmpty()) {
        originalImage.load(fileName);
        processedImage = originalImage;

        originalImageLabel->setPixmap(QPixmap::fromImage(originalImage.scaled(300, 300, Qt::KeepAspectRatio)));
        processedImageLabel->setPixmap(QPixmap::fromImage(processedImage.scaled(300, 300, Qt::KeepAspectRatio)));

        updateHistograms();
    }
}

void MainWindow::saveImage()
{
    if (processedImage.isNull()) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить изображение", "", "Файлы изображений (*.png *.jpg *.bmp)");

    if (!fileName.isEmpty()) {
        processedImage.save(fileName);
    }
}

void MainWindow::updateProgress(int value)
{
    progressBar->setValue(value);
}

void MainWindow::applyProcessing()
{
    if (originalImage.isNull()) return;

    progressBar->setVisible(true);
    progressBar->setValue(0);

    QString method = processingMethod->currentText();
    double alpha = alphaSlider->value() / 100.0;

    updateProgress(10);

    if (method == "Глобальный порог (по гистограмме)") {
        processedImage = globalThreshold(originalImage, thresholdSpin->value());
    }
    else if (method == "Глобальный порог (метод Оцу)") {
        processedImage = otsuThreshold(originalImage);
    }
    else if (method == "Адаптивный порог") {
        processedImage = adaptiveThreshold(originalImage, kernelSizeSpin->value(), alpha);
    }
    else if (method == "Обнаружение границ") {
        processedImage = detectEdges(originalImage);
    }
    else if (method == "Обнаружение линий") {
        processedImage = detectLines(originalImage);
    }
    else if (method == "Обнаружение точек") {
        processedImage = detectPoints(originalImage);
    }

    updateProgress(100);
    processedImageLabel->setPixmap(QPixmap::fromImage(processedImage.scaled(300, 300, Qt::KeepAspectRatio)));
    updateHistograms();

    QTimer::singleShot(2000, [this]() {
        progressBar->setVisible(false);
        progressBar->setValue(0);
    });
}

void MainWindow::updateHistograms()
{
    if (originalImage.isNull()) return;

    auto originalHistogram = calculateHistogram(originalImage);
    auto processedHistogram = calculateHistogram(processedImage);

    originalHistogramLabel->setPixmap(drawHistogram(originalHistogram, "Исходное изображение"));
    processedHistogramLabel->setPixmap(drawHistogram(processedHistogram, "Обработанное изображение"));
}

void MainWindow::resetImage()
{
    if (!originalImage.isNull()) {
        processedImage = originalImage;
        processedImageLabel->setPixmap(QPixmap::fromImage(processedImage.scaled(300, 300, Qt::KeepAspectRatio)));
        updateHistograms();
    }
}

QImage MainWindow::convertToGrayscale(const QImage &image)
{
    QImage result(image.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            int gray = qGray(color.red(), color.green(), color.blue());
            result.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }

    return result;
}

std::vector<int> MainWindow::calculateHistogram(const QImage &image)
{
    std::vector<int> histogram(256, 0);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            int gray = qGray(color.red(), color.green(), color.blue());
            histogram[gray]++;
        }
    }

    return histogram;
}

QPixmap MainWindow::drawHistogram(const std::vector<int> &histogram, const QString &title)
{
    int width = 300, height = 100;
    QImage histImage(width, height, QImage::Format_RGB32);
    histImage.fill(Qt::white);

    QPainter painter(&histImage);

    int maxCount = *std::max_element(histogram.begin(), histogram.end());
    if (maxCount == 0) maxCount = 1;

    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(5, 10, title);

    painter.setPen(Qt::gray);
    painter.drawLine(20, 15, 20, height - 15); // Y ось
    painter.drawLine(20, height - 15, width - 5, height - 15); // X ось

    painter.setPen(Qt::blue);
    double scaleX = (width - 25.0) / 256.0;

    for (int i = 0; i < 256; ++i) {
        int barHeight = (histogram[i] * (height - 30)) / maxCount;
        int x = 20 + i * scaleX;
        painter.drawLine(x, height - 15, x, height - 15 - barHeight);
    }

    painter.setPen(Qt::black);
    painter.drawText(15, height - 5, "0");
    painter.drawText(width - 15, height - 5, "255");

    return QPixmap::fromImage(histImage);
}

QImage MainWindow::globalThreshold(const QImage &image, int threshold)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    for (int y = 0; y < grayImage.height(); ++y) {
        for (int x = 0; x < grayImage.width(); ++x) {
            QRgb pixel = grayImage.pixel(x, y);
            int gray = qGray(pixel);
            int newValue = (gray > threshold) ? 255 : 0;
            result.setPixel(x, y, qRgb(newValue, newValue, newValue));
        }
    }

    return result;
}

QImage MainWindow::otsuThreshold(const QImage &image)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    std::vector<int> histogram(256, 0);
    for (int y = 0; y < grayImage.height(); ++y) {
        for (int x = 0; x < grayImage.width(); ++x) {
            QRgb pixel = grayImage.pixel(x, y);
            int gray = qGray(pixel);
            histogram[gray]++;
        }
    }

    int total = grayImage.width() * grayImage.height();

    double sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    double sumB = 0;
    int wB = 0;
    int wF = 0;

    double varMax = 0;
    int threshold = 128;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) continue;

        wF = total - wB;
        if (wF == 0) break;

        sumB += i * histogram[i];

        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;

        double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    for (int y = 0; y < grayImage.height(); ++y) {
        for (int x = 0; x < grayImage.width(); ++x) {
            QRgb pixel = grayImage.pixel(x, y);
            int gray = qGray(pixel);
            int newValue = (gray > threshold) ? 255 : 0;
            result.setPixel(x, y, qRgb(newValue, newValue, newValue));
        }
    }

    return result;
}

QImage MainWindow::adaptiveThreshold(const QImage &image, int blockSize, double alpha)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    int K = blockSize / 2;
    int maxIterations = 10;

    for (int y = 0; y < grayImage.height(); ++y) {
        for (int x = 0; x < grayImage.width(); ++x) {

            int currentK = K;
            bool thresholdFound = false;
            int iterations = 0;

            while (!thresholdFound && iterations < maxIterations) {
                int f_max = 0;
                int f_min = 255;
                int sum = 0;
                int count = 0;

                for (int j = -currentK; j <= currentK; ++j) {
                    for (int i = -currentK; i <= currentK; ++i) {
                        int nx = x + i;
                        int ny = y + j;

                        if (nx >= 0 && nx < grayImage.width() && ny >= 0 && ny < grayImage.height()) {
                            QRgb pixel = grayImage.pixel(nx, ny);
                            int gray = qGray(pixel);
                            f_max = std::max(f_max, gray);
                            f_min = std::min(f_min, gray);
                            sum += gray;
                            count++;
                        }
                    }
                }

                if (count == 0) continue;

                int f_avg = sum / count;
                int delta_f = f_max - f_min;

                int threshold;
                if (delta_f > alpha * 255) {
                    threshold = static_cast<int>(alpha * f_min + (1 - alpha) * f_max);
                }
                else if (delta_f < alpha * 128) {
                    threshold = static_cast<int>((1 - alpha) * f_min + alpha * f_max);
                }
                else {
                    currentK++;
                    iterations++;
                    continue;
                }

                QRgb pixel = grayImage.pixel(x, y);
                int gray = qGray(pixel);
                int newValue = (gray > threshold) ? 255 : 0;
                result.setPixel(x, y, qRgb(newValue, newValue, newValue));

                thresholdFound = true;
            }

            if (!thresholdFound) {
                QRgb pixel = grayImage.pixel(x, y);
                int gray = qGray(pixel);
                int newValue = (gray > 128) ? 255 : 0;
                result.setPixel(x, y, qRgb(newValue, newValue, newValue));
            }
        }
    }

    return result;
}

QImage MainWindow::detectEdges(const QImage &image)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    for (int y = 1; y < grayImage.height() - 1; ++y) {
        for (int x = 1; x < grayImage.width() - 1; ++x) {
            int gx = 0, gy = 0;

            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    QRgb pixel = grayImage.pixel(x + i, y + j);
                    int gray = qGray(pixel);

                    gx += gray * sobelX[j + 1][i + 1];
                    gy += gray * sobelY[j + 1][i + 1];
                }
            }

            int magnitude = std::min(255, (int)std::sqrt(gx * gx + gy * gy));
            result.setPixel(x, y, qRgb(magnitude, magnitude, magnitude));
        }
    }

    return result;
}

QImage MainWindow::detectLines(const QImage &image)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    int kernels[4][3][3] = {
        {{-1, -1, -1}, {2, 2, 2}, {-1, -1, -1}},
        {{-1, 2, -1}, {-1, 2, -1}, {-1, 2, -1}},
        {{-1, -1, 2}, {-1, 2, -1}, {2, -1, -1}},
        {{2, -1, -1}, {-1, 2, -1}, {-1, -1, 2}}
    };

    for (int y = 1; y < grayImage.height() - 1; ++y) {
        for (int x = 1; x < grayImage.width() - 1; ++x) {
            int maxResponse = 0;

            for (int k = 0; k < 4; ++k) {
                int response = 0;

                for (int j = -1; j <= 1; ++j) {
                    for (int i = -1; i <= 1; ++i) {
                        QRgb pixel = grayImage.pixel(x + i, y + j);
                        int gray = qGray(pixel);
                        response += gray * kernels[k][j + 1][i + 1];
                    }
                }

                maxResponse = std::max(maxResponse, std::abs(response));
            }

            int value = std::min(255, maxResponse);
            result.setPixel(x, y, qRgb(value, value, value));
        }
    }

    return result;
}

QImage MainWindow::detectPoints(const QImage &image)
{
    QImage grayImage = convertToGrayscale(image);
    QImage result = grayImage;

    for (int y = 1; y < grayImage.height() - 1; ++y) {
        for (int x = 1; x < grayImage.width() - 1; ++x) {
            int Ix = 0, Iy = 0;

            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    QRgb pixel = grayImage.pixel(x + i, y + j);
                    int gray = qGray(pixel);

                    if (i == 0) {
                        Iy += gray * (j * 2);
                    }
                    if (j == 0) {
                        Ix += gray * (i * 2);
                    }
                }
            }

            int response = std::min(255, std::abs(Ix * Iy) / 100);
            result.setPixel(x, y, qRgb(response, response, response));
        }
    }

    return result;
}
