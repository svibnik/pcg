#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMatrix4x4>
#include <QVector3D>

class GLWidget;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QGroupBox;
class QDoubleSpinBox;
class QRadioButton;
class QPushButton;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onScaleChanged(double value);
    void onTranslateXChanged(double value);
    void onTranslateYChanged(double value);
    void onTranslateZChanged(double value);
    void onRotateXChanged(double value);
    void onRotateYChanged(double value);
    void onRotateZChanged(double value);
    void onResetTransform();
    void onProjectionChanged();
    void onRotationAnglesChanged(float x, float y, float z);

private:
    void setupUI();
    void createControls();
    void updateMatrixDisplay();

    GLWidget *glWidget;
    QTextEdit *matrixTextEdit;

    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *rightPanelLayout;

    QDoubleSpinBox *scaleSpinBox;
    QDoubleSpinBox *translateXSpinBox;
    QDoubleSpinBox *translateYSpinBox;
    QDoubleSpinBox *translateZSpinBox;
    QDoubleSpinBox *rotateXSpinBox;
    QDoubleSpinBox *rotateYSpinBox;
    QDoubleSpinBox *rotateZSpinBox;

    QRadioButton *perspectiveRadio;
    QRadioButton *orthoXYRadio;
    QRadioButton *orthoXZRadio;
    QRadioButton *orthoYZRadio;

    QPushButton *resetButton;

    bool updatingRotationFromWidget;

    float lastRotationX;
    float lastRotationY;
    float lastRotationZ;
};

#endif
