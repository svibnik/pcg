#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QProgressBar>
#include <QTimer>
#include <vector>
#include <cmath>
#include <algorithm>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void saveImage();
    void applyProcessing();
    void resetImage();
    void onMethodChanged(int index);

private:
    QLabel *originalImageLabel;
    QLabel *processedImageLabel;
    QLabel *originalHistogramLabel;
    QLabel *processedHistogramLabel;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *processButton;
    QPushButton *resetButton;
    QComboBox *processingMethod;
    QSpinBox *thresholdSpin;
    QSpinBox *kernelSizeSpin;
    QSlider *alphaSlider;
    QProgressBar *progressBar;

    QGroupBox *globalParamsGroup;
    QGroupBox *adaptiveParamsGroup;

    QImage originalImage;
    QImage processedImage;

    QImage globalThreshold(const QImage &image, int threshold);
    QImage otsuThreshold(const QImage &image);
    QImage adaptiveThreshold(const QImage &image, int blockSize, double alpha);
    QImage detectEdges(const QImage &image);
    QImage detectLines(const QImage &image);
    QImage detectPoints(const QImage &image);

    std::vector<int> calculateHistogram(const QImage &image);
    QImage convertToGrayscale(const QImage &image);
    QPixmap drawHistogram(const std::vector<int> &histogram, const QString &title = "");
    void updateHistograms();
    void updateProgress(int value);

    void setupUI();
};

#endif
