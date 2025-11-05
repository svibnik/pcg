#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QStatusBar>
#include <QColorDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onHsvChanged();
    void onXyzChanged();
    void onLabChanged();
    void onColorPickerClicked();

private:
    void setupUI();
    void setupConnections();
    void updateAllViews(int sourceModel);
    void disconnectAll();
    void connectAll();

    QSlider *hSlider;
    QSlider *sSlider;
    QSlider *vSlider;
    QDoubleSpinBox *hSpinBox;
    QDoubleSpinBox *sSpinBox;
    QDoubleSpinBox *vSpinBox;

    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QDoubleSpinBox *xSpinBox;
    QDoubleSpinBox *ySpinBox;
    QDoubleSpinBox *zSpinBox;

    QSlider *lSlider;
    QSlider *aSlider;
    QSlider *bSlider;
    QDoubleSpinBox *lSpinBox;
    QDoubleSpinBox *aSpinBox;
    QDoubleSpinBox *bSpinBox;

    QFrame *colorPreview;
    QPushButton *colorPickerButton;

    bool m_updating;

    QLabel *warningLabel;

    void showWarning(const QString& message);
    void clearWarning();
    void checkColorBounds(const QColor& color);
};

#endif
