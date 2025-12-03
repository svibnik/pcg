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
#include <QFileDialog>
#include <QMessageBox>
#include <QLine>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadButtonClicked();
    void onClipButtonClicked();
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
    void drawAlgorithmDescription();

    void clipMidpoint(double x1, double y1, double x2, double y2, const QColor& color = Qt::blue);
    void clipPolygon(const QVector<QPointF>& polygon, const QColor& color = Qt::green);

    int computeCode(double x, double y);

    bool isInside(const QPointF& p, const QPointF& A, const QPointF& B);
    QPointF computeIntersection(const QPointF& p1, const QPointF& p2, const QPointF& A, const QPointF& B);

    void drawLine(double x1, double y1, double x2, double y2, const QColor& color);
    void drawRectangle(double xmin, double ymin, double xmax, double ymax, const QColor& color);
    void drawPolygon(const QVector<QPointF>& points, const QColor& color);

    bool isSegmentInside(double x1, double y1, double x2, double y2);
    bool isSegmentOutside(double x1, double y1, double x2, double y2);
    void clipMidpointRecursive(double x1, double y1, double x2, double y2, const QColor& color);

    QGraphicsScene *scene;
    QGraphicsView *graphicsView;

    QComboBox *algorithmComboBox;
    QSpinBox *xMinSpinBox, *yMinSpinBox, *xMaxSpinBox, *yMaxSpinBox;
    QSlider *gridSizeSlider;
    QLabel *timeInfoLabel;
    QTextEdit *algorithmDescriptionTextEdit;

    QPushButton *loadButton;
    QPushButton *clipButton;
    QPushButton *clearButton;

    QHash<QString, qint64> timeMeasurements;

    QVector<QLineF> segments;
    QVector<QVector<QPointF>> polygons;
    QRectF clipWindow;

    int CELL_SIZE;
    int GRID_EXTENT;
};

#endif
