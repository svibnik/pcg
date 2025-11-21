#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QElapsedTimer>
#include <QHash>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDrawButtonClicked();
    void onClearButtonClicked();
    void onAlgorithmChanged(int index);
    void onGridSizeChanged(int value);

private:
    void setupUI();
    void setupScene();
    void drawCoordinateSystem();
    void drawGrid();
    void drawPixel(int x, int y, const QColor& color = Qt::black);
    void updateTimeInfo();
    void drawExampleCalculations();

    void drawLineStep(int x1, int y1, int x2, int y2, const QColor& color = Qt::blue);
    void drawLineCDA(int x1, int y1, int x2, int y2, const QColor& color = Qt::red);
    void drawLineBresenham(int x1, int y1, int x2, int y2, const QColor& color = Qt::green);
    void drawCircleBresenham(int x0, int y0, int radius, const QColor& color = Qt::magenta);
    void drawLineCastlePitway(int x1, int y1, int x2, int y2, const QColor& color = Qt::darkYellow);
    void drawLineWu(int x1, int y1, int x2, int y2, const QColor& color = Qt::darkCyan);

    QGraphicsScene *scene;
    QGraphicsView *graphicsView;

    QComboBox *algorithmComboBox;
    QSpinBox *x1SpinBox, *y1SpinBox, *x2SpinBox, *y2SpinBox, *radiusSpinBox;
    QSlider *gridSizeSlider;
    QLabel *timeInfoLabel;
    QTextEdit *calculationsTextEdit;
    QLabel *radiusLabel, *x2Label, *y2Label;

    QHash<QString, qint64> timeMeasurements;

    int CELL_SIZE;
    int GRID_EXTENT;
};

#endif
