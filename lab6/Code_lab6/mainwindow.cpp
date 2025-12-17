#include "mainwindow.h"
#include "glwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QPushButton>
#include <QFrame>
#include <QGridLayout>
#include <QTextEdit>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , updatingRotationFromWidget(false)
    , lastRotationX(0.0f)
    , lastRotationY(0.0f)
    , lastRotationZ(0.0f)
{
    setupUI();
    createControls();

    connect(glWidget, &GLWidget::transformationChanged,
            this, &MainWindow::updateMatrixDisplay);

    connect(glWidget, &GLWidget::rotationAnglesChanged,
            this, &MainWindow::onRotationAnglesChanged);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);

    glWidget = new GLWidget(this);

    glWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    glWidget->setMinimumWidth(400);
    mainLayout->addWidget(glWidget, 1);

    rightPanelLayout = new QVBoxLayout();

    QWidget *rightPanelWidget = new QWidget();
    rightPanelWidget->setLayout(rightPanelLayout);
    rightPanelWidget->setMinimumWidth(400);
    rightPanelWidget->setMaximumWidth(400);
    rightPanelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mainLayout->addWidget(rightPanelWidget);

    mainLayout->setStretchFactor(glWidget, 3);
    mainLayout->setStretchFactor(rightPanelWidget, 1);
}

void MainWindow::createControls()
{
    QGroupBox *matrixGroup = new QGroupBox("Матрица преобразования");
    QVBoxLayout *matrixLayout = new QVBoxLayout();

    matrixTextEdit = new QTextEdit();
    matrixTextEdit->setReadOnly(true);
    matrixTextEdit->setMaximumHeight(120);
    matrixTextEdit->setMinimumHeight(120);
    matrixTextEdit->setMaximumWidth(370);
    matrixTextEdit->setMinimumWidth(370);
    matrixTextEdit->setFont(QFont("Courier New", 9));
    matrixTextEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    matrixTextEdit->setLineWrapMode(QTextEdit::NoWrap);

    matrixLayout->addWidget(matrixTextEdit);
    matrixGroup->setLayout(matrixLayout);
    rightPanelLayout->addWidget(matrixGroup);

    QGroupBox *transformGroup = new QGroupBox("Преобразования");
    QGridLayout *transformLayout = new QGridLayout();

    transformLayout->addWidget(new QLabel("Масштаб:"), 0, 0);
    scaleSpinBox = new QDoubleSpinBox();
    scaleSpinBox->setRange(0.1, 5.0);
    scaleSpinBox->setSingleStep(0.1);
    scaleSpinBox->setValue(1.0);
    scaleSpinBox->setDecimals(2);
    transformLayout->addWidget(scaleSpinBox, 0, 1);
    connect(scaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onScaleChanged(double)));

    transformLayout->addWidget(new QLabel("Перенос X:"), 1, 0);
    translateXSpinBox = new QDoubleSpinBox();
    translateXSpinBox->setRange(-5.0, 5.0);
    translateXSpinBox->setSingleStep(0.1);
    translateXSpinBox->setValue(0.0);
    translateXSpinBox->setDecimals(2);
    transformLayout->addWidget(translateXSpinBox, 1, 1);
    connect(translateXSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onTranslateXChanged(double)));

    transformLayout->addWidget(new QLabel("Перенос Y:"), 2, 0);
    translateYSpinBox = new QDoubleSpinBox();
    translateYSpinBox->setRange(-5.0, 5.0);
    translateYSpinBox->setSingleStep(0.1);
    translateYSpinBox->setValue(0.0);
    translateYSpinBox->setDecimals(2);
    transformLayout->addWidget(translateYSpinBox, 2, 1);
    connect(translateYSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onTranslateYChanged(double)));

    transformLayout->addWidget(new QLabel("Перенос Z:"), 3, 0);
    translateZSpinBox = new QDoubleSpinBox();
    translateZSpinBox->setRange(-5.0, 5.0);
    translateZSpinBox->setSingleStep(0.1);
    translateZSpinBox->setValue(0.0);
    translateZSpinBox->setDecimals(2);
    transformLayout->addWidget(translateZSpinBox, 3, 1);
    connect(translateZSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onTranslateZChanged(double)));

    transformLayout->addWidget(new QLabel("Вращение X:"), 4, 0);
    rotateXSpinBox = new QDoubleSpinBox();
    rotateXSpinBox->setRange(-180.0, 180.0);
    rotateXSpinBox->setSingleStep(5.0);
    rotateXSpinBox->setValue(0.0);
    rotateXSpinBox->setDecimals(1);
    rotateXSpinBox->setWrapping(true);
    transformLayout->addWidget(rotateXSpinBox, 4, 1);
    connect(rotateXSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRotateXChanged(double)));

    transformLayout->addWidget(new QLabel("Вращение Y:"), 5, 0);
    rotateYSpinBox = new QDoubleSpinBox();
    rotateYSpinBox->setRange(-180.0, 180.0);
    rotateYSpinBox->setSingleStep(5.0);
    rotateYSpinBox->setValue(0.0);
    rotateYSpinBox->setDecimals(1);
    rotateYSpinBox->setWrapping(true);
    transformLayout->addWidget(rotateYSpinBox, 5, 1);
    connect(rotateYSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRotateYChanged(double)));

    transformLayout->addWidget(new QLabel("Вращение Z:"), 6, 0);
    rotateZSpinBox = new QDoubleSpinBox();
    rotateZSpinBox->setRange(-180.0, 180.0);
    rotateZSpinBox->setSingleStep(5.0);
    rotateZSpinBox->setValue(0.0);
    rotateZSpinBox->setDecimals(1);
    rotateZSpinBox->setWrapping(true);
    transformLayout->addWidget(rotateZSpinBox, 6, 1);
    connect(rotateZSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRotateZChanged(double)));

    resetButton = new QPushButton("Сбросить преобразования");
    transformLayout->addWidget(resetButton, 7, 0, 1, 2);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetTransform);

    transformGroup->setLayout(transformLayout);
    rightPanelLayout->addWidget(transformGroup);

    QGroupBox *projectionGroup = new QGroupBox("Проекции");
    QVBoxLayout *projectionLayout = new QVBoxLayout();

    perspectiveRadio = new QRadioButton("Перспективная проекция");
    perspectiveRadio->setChecked(true);
    projectionLayout->addWidget(perspectiveRadio);
    connect(perspectiveRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);

    orthoXYRadio = new QRadioButton("Ортогональная XY (вид сверху)");
    projectionLayout->addWidget(orthoXYRadio);
    connect(orthoXYRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);

    orthoXZRadio = new QRadioButton("Ортогональная XZ (вид спереди)");
    projectionLayout->addWidget(orthoXZRadio);
    connect(orthoXZRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);

    orthoYZRadio = new QRadioButton("Ортогональная YZ (вид сбоку)");
    projectionLayout->addWidget(orthoYZRadio);
    connect(orthoYZRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);

    projectionGroup->setLayout(projectionLayout);
    rightPanelLayout->addWidget(projectionGroup);

    rightPanelLayout->addStretch();

    updateMatrixDisplay();
}

void MainWindow::onScaleChanged(double value)
{
    glWidget->setScale(value);
    updateMatrixDisplay();
    glWidget->update();
}

void MainWindow::onTranslateXChanged(double value)
{
    glWidget->setTranslation(value, translateYSpinBox->value(), translateZSpinBox->value());
    updateMatrixDisplay();
    glWidget->update();
}

void MainWindow::onTranslateYChanged(double value)
{
    glWidget->setTranslation(translateXSpinBox->value(), value, translateZSpinBox->value());
    updateMatrixDisplay();
    glWidget->update();
}

void MainWindow::onTranslateZChanged(double value)
{
    glWidget->setTranslation(translateXSpinBox->value(), translateYSpinBox->value(), value);
    updateMatrixDisplay();
    glWidget->update();
}

void MainWindow::onProjectionChanged()
{
    int projectionType = 0;

    if (orthoXYRadio->isChecked()) {
        projectionType = 1;
    }
    else if (orthoXZRadio->isChecked()) {
        projectionType = 2;
    }
    else if (orthoYZRadio->isChecked()) {
        projectionType = 3;
    }

    glWidget->setProjectionType(projectionType);
    glWidget->update();
}

void MainWindow::updateMatrixDisplay()
{
    QMatrix4x4 finalMatrix = glWidget->getTransformationMatrix();

    QString matrixText;
    matrixText += "<pre style='font-size:9pt;'>";
    matrixText += "<b>Итоговая матрица преобразования:</b>\n";

    for (int i = 0; i < 4; ++i) {
        matrixText += "|";
        for (int j = 0; j < 4; ++j) {
            matrixText += QString::number(finalMatrix(i, j), 'f', 3).rightJustified(8);
        }
        matrixText += " |\n";
    }

    matrixTextEdit->setText(matrixText);
}

void MainWindow::onRotateXChanged(double value)
{
    if (updatingRotationFromWidget) return;

    float currentValue = static_cast<float>(value);
    float delta = currentValue - lastRotationX;
    lastRotationX = currentValue;

    if (qAbs(delta) > 0.01f) {
        glWidget->rotateX(delta);
        updateMatrixDisplay();
        glWidget->update();
    }
}

void MainWindow::onRotateYChanged(double value)
{
    if (updatingRotationFromWidget) return;

    float currentValue = static_cast<float>(value);
    float delta = currentValue - lastRotationY;
    lastRotationY = currentValue;

    if (qAbs(delta) > 0.01f) {
        glWidget->rotateY(delta);
        updateMatrixDisplay();
        glWidget->update();
    }
}

void MainWindow::onRotateZChanged(double value)
{
    if (updatingRotationFromWidget) return;

    float currentValue = static_cast<float>(value);
    float delta = currentValue - lastRotationZ;
    lastRotationZ = currentValue;

    if (qAbs(delta) > 0.01f) {
        glWidget->rotateZ(delta);
        updateMatrixDisplay();
        glWidget->update();
    }
}

void MainWindow::onRotationAnglesChanged(float x, float y, float z)
{
    updatingRotationFromWidget = true;

    rotateXSpinBox->setValue(x);
    rotateYSpinBox->setValue(y);
    rotateZSpinBox->setValue(z);

    lastRotationX = x;
    lastRotationY = y;
    lastRotationZ = z;

    updatingRotationFromWidget = false;

    updateMatrixDisplay();
}

void MainWindow::onResetTransform()
{
    updatingRotationFromWidget = true;

    scaleSpinBox->setValue(1.0);
    translateXSpinBox->setValue(0.0);
    translateYSpinBox->setValue(0.0);
    translateZSpinBox->setValue(0.0);
    rotateXSpinBox->setValue(0.0);
    rotateYSpinBox->setValue(0.0);
    rotateZSpinBox->setValue(0.0);
    perspectiveRadio->setChecked(true);

    lastRotationX = 0.0f;
    lastRotationY = 0.0f;
    lastRotationZ = 0.0f;

    updatingRotationFromWidget = false;

    glWidget->resetTransform();
    updateMatrixDisplay();
    glWidget->update();
}
