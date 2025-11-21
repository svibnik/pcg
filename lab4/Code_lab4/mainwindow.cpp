#include "mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGroupBox>
#include <QtMath>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , scene(new QGraphicsScene(this))
    , graphicsView(new QGraphicsView)
    , CELL_SIZE(20)
    , GRID_EXTENT(15)
{
    setupUI();
    setupScene();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setMinimumSize(700, 700);
    mainLayout->addWidget(graphicsView);

    QWidget *controlWidget = new QWidget;
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    controlWidget->setMaximumWidth(400);

    QGroupBox *paramsGroup = new QGroupBox("Параметры отрисовки");
    QVBoxLayout *paramsLayout = new QVBoxLayout(paramsGroup);

    QLabel *algorithmLabel = new QLabel("Алгоритм:");
    algorithmComboBox = new QComboBox;
    algorithmComboBox->addItem("Пошаговый алгоритм");
    algorithmComboBox->addItem("Алгоритм ЦДА");
    algorithmComboBox->addItem("Алгоритм Брезенхема (отрезок)");
    algorithmComboBox->addItem("Алгоритм Брезенхема (окружность)");
    algorithmComboBox->addItem("Алгоритм Кастла-Питвея");
    algorithmComboBox->addItem("Алгоритм Ву (сглаживание)");

    QHBoxLayout *point1Layout = new QHBoxLayout;
    point1Layout->addWidget(new QLabel("X1:"));
    x1SpinBox = new QSpinBox;
    x1SpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    x1SpinBox->setValue(0);
    point1Layout->addWidget(x1SpinBox);

    point1Layout->addWidget(new QLabel("Y1:"));
    y1SpinBox = new QSpinBox;
    y1SpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    y1SpinBox->setValue(0);
    point1Layout->addWidget(y1SpinBox);

    QHBoxLayout *point2Layout = new QHBoxLayout;
    x2Label = new QLabel("X2:");
    point2Layout->addWidget(x2Label);
    x2SpinBox = new QSpinBox;
    x2SpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    x2SpinBox->setValue(8);
    point2Layout->addWidget(x2SpinBox);

    y2Label = new QLabel("Y2:");
    point2Layout->addWidget(y2Label);
    y2SpinBox = new QSpinBox;
    y2SpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    y2SpinBox->setValue(5);
    point2Layout->addWidget(y2SpinBox);

    QHBoxLayout *radiusLayout = new QHBoxLayout;
    radiusLabel = new QLabel("Радиус:");
    radiusLabel->setVisible(false);
    radiusLayout->addWidget(radiusLabel);
    radiusSpinBox = new QSpinBox;
    radiusSpinBox->setRange(1, GRID_EXTENT);
    radiusSpinBox->setValue(5);
    radiusSpinBox->setVisible(false);
    radiusLayout->addWidget(radiusSpinBox);

    QLabel *gridLabel = new QLabel("Размер клетки:");
    gridSizeSlider = new QSlider(Qt::Horizontal);
    gridSizeSlider->setRange(10, 40);
    gridSizeSlider->setValue(CELL_SIZE);
    QLabel *gridSizeValue = new QLabel(QString::number(CELL_SIZE) + " px");

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QPushButton *drawButton = new QPushButton("Нарисовать");
    QPushButton *clearButton = new QPushButton("Очистить");
    buttonsLayout->addWidget(drawButton);
    buttonsLayout->addWidget(clearButton);

    paramsLayout->addWidget(algorithmLabel);
    paramsLayout->addWidget(algorithmComboBox);
    paramsLayout->addLayout(point1Layout);
    paramsLayout->addLayout(point2Layout);
    paramsLayout->addLayout(radiusLayout);
    paramsLayout->addWidget(gridLabel);
    paramsLayout->addWidget(gridSizeSlider);
    paramsLayout->addWidget(gridSizeValue);
    paramsLayout->addLayout(buttonsLayout);

    QGroupBox *infoGroup = new QGroupBox("Информация");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);

    timeInfoLabel = new QLabel("Время выполнения:");
    timeInfoLabel->setFrameStyle(QFrame::Box);
    timeInfoLabel->setMinimumHeight(80);

    QLabel *calcLabel = new QLabel("Пример вычислений:");
    calculationsTextEdit = new QTextEdit;
    calculationsTextEdit->setMaximumHeight(200);
    calculationsTextEdit->setReadOnly(true);
    calculationsTextEdit->setStyleSheet("QTextEdit { background-color: #f8f8f8; }");

    infoLayout->addWidget(timeInfoLabel);
    infoLayout->addWidget(calcLabel);
    infoLayout->addWidget(calculationsTextEdit);

    controlLayout->addWidget(paramsGroup);
    controlLayout->addWidget(infoGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlWidget);

    connect(drawButton, &QPushButton::clicked, this, &MainWindow::onDrawButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    connect(gridSizeSlider, &QSlider::valueChanged, this, [=](int value) {
        CELL_SIZE = value;
        gridSizeValue->setText(QString::number(value) + " px");
        setupScene();
    });
}

void MainWindow::setupScene()
{
    scene->clear();

    int sceneWidth = 2 * GRID_EXTENT * CELL_SIZE + 100;
    int sceneHeight = 2 * GRID_EXTENT * CELL_SIZE + 100;
    scene->setSceneRect(-sceneWidth/2, -sceneHeight/2, sceneWidth, sceneHeight);

    drawCoordinateSystem();
    drawGrid();
}

void MainWindow::drawCoordinateSystem()
{
    int sceneSize = GRID_EXTENT * CELL_SIZE;

    QPen axisPen(Qt::black);
    axisPen.setWidth(2);

    scene->addLine(-sceneSize, 0, sceneSize, 0, axisPen);
    scene->addLine(0, -sceneSize, 0, sceneSize, axisPen);

    int fontSize = qMax(6, CELL_SIZE / 3);
    QFont font("Arial", fontSize);

    int labelOffset = CELL_SIZE / 2;
    int tickSize = CELL_SIZE / 6;

    for (int i = -GRID_EXTENT; i <= GRID_EXTENT; i++) {
        if (i != 0) {
            QGraphicsTextItem *xLabel = scene->addText(QString::number(i));
            int xOffset = (qAbs(i) >= 10) ? CELL_SIZE / 2 : CELL_SIZE / 3;
            xLabel->setPos(i * CELL_SIZE - xOffset, labelOffset);
            xLabel->setFont(font);
            xLabel->setZValue(1);

            QGraphicsTextItem *yLabel = scene->addText(QString::number(i));
            int yOffset = -CELL_SIZE - (qAbs(i) >= 10 ? CELL_SIZE / 3 : CELL_SIZE / 4);
            yLabel->setPos(yOffset, -i * CELL_SIZE - fontSize / 2 - 5);
            yLabel->setFont(font);
            yLabel->setZValue(1);

            scene->addLine(i * CELL_SIZE, -tickSize, i * CELL_SIZE, tickSize, axisPen);
            scene->addLine(-tickSize, i * CELL_SIZE, tickSize, i * CELL_SIZE, axisPen);
        }
    }

    QGraphicsTextItem *originLabel = scene->addText("0");
    originLabel->setPos(-CELL_SIZE * 2 / 3, labelOffset - 11);
    originLabel->setFont(font);
    originLabel->setZValue(1);

    int axisFontSize = qMax(10, CELL_SIZE / 3 + 2);
    QGraphicsTextItem *xAxisLabel = scene->addText("X");
    xAxisLabel->setPos(sceneSize + labelOffset, labelOffset - 20);
    xAxisLabel->setFont(QFont("Arial", axisFontSize, QFont::Bold));
    xAxisLabel->setZValue(1);

    QGraphicsTextItem *yAxisLabel = scene->addText("Y");
    yAxisLabel->setPos(labelOffset - 18, -sceneSize - labelOffset - 20);
    yAxisLabel->setFont(QFont("Arial", axisFontSize, QFont::Bold));
    yAxisLabel->setZValue(1);
}

void MainWindow::drawGrid()
{
    int sceneSize = GRID_EXTENT * CELL_SIZE;

    QPen gridPen(QColor(220, 220, 220));

    for (int i = -GRID_EXTENT; i <= GRID_EXTENT; i++) {
        scene->addLine(i * CELL_SIZE, -sceneSize, i * CELL_SIZE, sceneSize, gridPen);
    }

    for (int i = -GRID_EXTENT; i <= GRID_EXTENT; i++) {
        scene->addLine(-sceneSize, i * CELL_SIZE, sceneSize, i * CELL_SIZE, gridPen);
    }
}

void MainWindow::drawPixel(int x, int y, const QColor& color)
{
    int screenX = x * CELL_SIZE - CELL_SIZE / 2;
    int screenY = -y * CELL_SIZE - CELL_SIZE / 2;

    QGraphicsRectItem *pixel = scene->addRect(screenX, screenY, CELL_SIZE, CELL_SIZE);

    if (color.alpha() < 255) {
        pixel->setBrush(QBrush(color));
        pixel->setPen(Qt::NoPen);
    } else {
        pixel->setBrush(QBrush(color));
        QPen pen(QColor(100, 100, 100, 50));
        pen.setWidth(1);
        pixel->setPen(pen);
    }

    pixel->setZValue(0);
}

void MainWindow::drawLineStep(int x1, int y1, int x2, int y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    if (x1 == x2) {
        int startY = qMin(y1, y2);
        int endY = qMax(y1, y2);
        for (int y = startY; y <= endY; y++) {
            drawPixel(x1, y, color);
        }
        timeMeasurements["Step"] = timer.nsecsElapsed();
        return;
    }

    int startX = qMin(x1, x2);
    int endX = qMax(x1, x2);

    float k = float(y2 - y1) / (x2 - x1);
    float b = y1 - k * x1;

    for (int x = startX; x <= endX; x++) {
        float y = k * x + b;
        drawPixel(x, (int)y, color);
    }

    timeMeasurements["Step"] = timer.nsecsElapsed();
}

void MainWindow::drawLineCDA(int x1, int y1, int x2, int y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    int dx = x2 - x1;
    int dy = y2 - y1;
    float L = qMax(qAbs(dx), qAbs(dy));

    if (L == 0) {
        drawPixel(x1, y1, color);
        return;
    }

    float xIncrement = dx / L;
    float yIncrement = dy / L;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= L; i++) {
        drawPixel(qRound(x), qRound(y), color);
        x += xIncrement;
        y += yIncrement;
    }

    timeMeasurements["CDA"] = timer.nsecsElapsed();
}

void MainWindow::drawLineBresenham(int x1, int y1, int x2, int y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    int dx = qAbs(x2 - x1);
    int dy = qAbs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1;
    int y = y1;

    while (true) {
        drawPixel(x, y, color);

        if (x == x2 && y == y2) break;

        int err2 = 2 * err;

        if (err2 > -dy) {
            err -= dy;
            x += sx;
        }

        if (err2 < dx) {
            err += dx;
            y += sy;
        }
    }

    timeMeasurements["BresenhamLine"] = timer.nsecsElapsed();
}

void MainWindow::drawCircleBresenham(int x0, int y0, int radius, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;

    while (y >= x) {
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 - y, y0 - x, color);

        error = 2 * (delta + y) - 1;

        if (delta < 0 && error <= 0) {
            x++;
            delta += 2 * x + 1;
            continue;
        }

        if (delta > 0 && error > 0) {
            y--;
            delta -= 2 * y + 1;
            continue;
        }

        x++;
        delta += 2 * (x - y);
        y--;
    }

    timeMeasurements["BresenhamCircle"] = timer.nsecsElapsed();
}

void MainWindow::drawLineCastlePitway(int x1, int y1, int x2, int y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    int dx = x2 - x1;
    int dy = y2 - y1;

    bool swapXY = false;
    bool invertX = false;
    bool invertY = false;

    if (qAbs(dy) > qAbs(dx)) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        swapXY = true;
        dx = x2 - x1;
        dy = y2 - y1;
    }

    if (dx < 0) {
        x1 = -x1;
        invertX = true;
        dx = -dx;
    }

    if (dy < 0) {
        y1 = -y1;
        invertY = true;
        dy = -dy;
    }

    int x = qAbs(dx - dy);
    int y = qAbs(dy);

    QString m1 = "R";
    QString m2 = "D";

    while (x != y) {
        if (x > y) {
            x = x - y;
            std::reverse(m2.begin(), m2.end());
            m2 = m1 + m2;
        } else {
            y = y - x;
            std::reverse(m1.begin(), m1.end());
            m1 = m2 + m1;
        }
    }

    QString path;
    for (int i = 0; i < x; i++) {
        path += m1 + m2;
    }

    int currentX = x1;
    int currentY = y1;

    auto transformBack = [&](int px, int py) -> QPoint {
        int resultX = px;
        int resultY = py;

        if (invertY) {
            resultY = -resultY;
        }
        if (invertX) {
            resultX = -resultX;
        }
        if (swapXY) {
            std::swap(resultX, resultY);
        }

        return QPoint(resultX, resultY);
    };

    auto makeStep = [&](QChar direction) {
        if (direction == 'R') {
            currentX++;
        } else if (direction == 'D') {
            currentX++;
            currentY++;
        }
    };

    QPoint startPoint = transformBack(x1, y1);
    drawPixel(startPoint.x(), startPoint.y(), color);

    for (int i = 0; i < path.length(); i++) {
        QChar direction = path[i];
        makeStep(direction);
        QPoint transformedPoint = transformBack(currentX, currentY);
        drawPixel(transformedPoint.x(), transformedPoint.y(), color);
    }

    timeMeasurements["CastlePitway"] = timer.nsecsElapsed();
}

void MainWindow::drawLineWu(int x1, int y1, int x2, int y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    bool steep = qAbs(y2 - y1) > qAbs(x2 - x1);

    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    float dx = x2 - x1;
    float dy = y2 - y1;
    float gradient = (dx == 0) ? 1.0 : dy / dx;

    float y = y1;

    for (int x = x1; x <= x2; x++) {
        if (steep) {
            int baseY = qFloor(y);
            float fraction = y - baseY;

            drawPixel(baseY, x, QColor(color.red(), color.green(), color.blue(), 255 * (1 - fraction)));
            drawPixel(baseY + 1, x, QColor(color.red(), color.green(), color.blue(), 255 * fraction));
        } else {
            int baseY = qFloor(y);
            float fraction = y - baseY;

            drawPixel(x, baseY, QColor(color.red(), color.green(), color.blue(), 255 * (1 - fraction)));
            drawPixel(x, baseY + 1, QColor(color.red(), color.green(), color.blue(), 255 * fraction));
        }
        y += gradient;
    }

    timeMeasurements["Wu"] = timer.nsecsElapsed();
}

void MainWindow::onDrawButtonClicked()
{
    int x1 = x1SpinBox->value();
    int y1 = y1SpinBox->value();
    int x2 = x2SpinBox->value();
    int y2 = y2SpinBox->value();
    int radius = radiusSpinBox->value();

    setupScene();
    timeMeasurements.clear();

    int algorithmIndex = algorithmComboBox->currentIndex();

    switch (algorithmIndex) {
    case 0:
        drawLineStep(x1, y1, x2, y2);
        break;
    case 1:
        drawLineCDA(x1, y1, x2, y2);
        break;
    case 2:
        drawLineBresenham(x1, y1, x2, y2);
        break;
    case 3:
        drawCircleBresenham(x1, y1, radius);
        break;
    case 4:
        drawLineCastlePitway(x1, y1, x2, y2);
        break;
    case 5:
        drawLineWu(x1, y1, x2, y2);
        break;
    }

    updateTimeInfo();
    drawExampleCalculations();
}

void MainWindow::onClearButtonClicked()
{
    setupScene();
    timeMeasurements.clear();
    timeInfoLabel->setText("Время выполнения:");
    calculationsTextEdit->clear();
}

void MainWindow::onAlgorithmChanged(int index)
{
    bool isCircle = (index == 3);
    radiusLabel->setVisible(isCircle);
    radiusSpinBox->setVisible(isCircle);
    x2Label->setVisible(!isCircle);
    x2SpinBox->setVisible(!isCircle);
    y2Label->setVisible(!isCircle);
    y2SpinBox->setVisible(!isCircle);
}

void MainWindow::onGridSizeChanged(int value)
{
    CELL_SIZE = value;
    setupScene();
}

void MainWindow::updateTimeInfo()
{
    QString timeText = "Время выполнения:";

    for (auto it = timeMeasurements.begin(); it != timeMeasurements.end(); ++it) {
        timeText += QString("\n%1: %2 нс").arg(it.key()).arg(it.value());
    }

    timeInfoLabel->setText(timeText);
}

void MainWindow::drawExampleCalculations()
{
    QString calculations = "Пример вычислений:\n\n";

    int algorithmIndex = algorithmComboBox->currentIndex();
    int x1 = x1SpinBox->value();
    int y1 = y1SpinBox->value();
    int x2 = x2SpinBox->value();
    int y2 = y2SpinBox->value();
    int radius = radiusSpinBox->value();

    switch (algorithmIndex) {
    case 0:
        calculations += QString("Пошаговый алгоритм:\n"
                                "Уравнение прямой: y = kx + b\n"
                                "k = (y2 - y1)/(x2 - x1) = (%1 - %2)/(%3 - %4) = %5\n"
                                "b = y1 - k*x1 = %6 - %7*%8 = %9\n\n"
                                "Для каждого x от %10 до %11:\n"
                                "y = k*x + b = %12*x + %13")
                            .arg(y2).arg(y1).arg(x2).arg(x1).arg(float(y2 - y1) / (x2 - x1))
                            .arg(y1).arg(float(y2 - y1) / (x2 - x1)).arg(x1).arg(y1 - (float(y2 - y1) / (x2 - x1)) * x1)
                            .arg(qMin(x1, x2)).arg(qMax(x1, x2))
                            .arg(float(y2 - y1) / (x2 - x1)).arg(y1 - (float(y2 - y1) / (x2 - x1)) * x1);
        break;

    case 1:
        calculations += QString("Алгоритм ЦДА:\n"
                                "dx = x2 - x1 = %1 - %2 = %3\n"
                                "dy = y2 - y1 = %4 - %5 = %6\n"
                                "L = max(|dx|, |dy|) = max(|%7|, |%8|) = %9\n\n"
                                "x_increment = dx/L = %10/%11 = %12\n"
                                "y_increment = dy/L = %13/%14 = %15")
                            .arg(x2).arg(x1).arg(x2 - x1)
                            .arg(y2).arg(y1).arg(y2 - y1)
                            .arg(x2 - x1).arg(y2 - y1).arg(qMax(qAbs(x2 - x1), qAbs(y2 - y1)))
                            .arg(x2 - x1).arg(qMax(qAbs(x2 - x1), qAbs(y2 - y1))).arg(float(x2 - x1) / qMax(qAbs(x2 - x1), qAbs(y2 - y1)))
                            .arg(y2 - y1).arg(qMax(qAbs(x2 - x1), qAbs(y2 - y1))).arg(float(y2 - y1) / qMax(qAbs(x2 - x1), qAbs(y2 - y1)));
        break;

    case 2:
        calculations += QString("Алгоритм Брезенхема (отрезок):\n"
                                "dx = |x2 - x1| = |%1 - %2| = %3\n"
                                "dy = |y2 - y1| = |%4 - %5| = %6\n"
                                "sx = %7, sy = %8\n"
                                "err = dx - dy = %9 - %10 = %11\n\n"
                                "Начальные значения:\n"
                                "x = %12, y = %13, err = %14")
                            .arg(x2).arg(x1).arg(qAbs(x2 - x1))
                            .arg(y2).arg(y1).arg(qAbs(y2 - y1))
                            .arg((x1 < x2) ? 1 : -1).arg((y1 < y2) ? 1 : -1)
                            .arg(qAbs(x2 - x1)).arg(qAbs(y2 - y1)).arg(qAbs(x2 - x1) - qAbs(y2 - y1))
                            .arg(x1).arg(y1).arg(qAbs(x2 - x1) - qAbs(y2 - y1));
        break;

    case 3:
        calculations += QString("Алгоритм Брезенхема (окружность):\n"
                                "Центр: (%1, %2)\n"
                                "Радиус: %3\n\n"
                                "Начальные значения:\n"
                                "x = 0, y = радиус = %4\n"
                                "delta = 1 - 2*радиус = 1 - 2*%5 = %6")
                            .arg(x1).arg(y1).arg(radius)
                            .arg(radius).arg(radius).arg(1 - 2 * radius);
        break;

    case 4:
        calculations += QString("Алгоритм Кастла-Питвея (строковая версия):\n"
                                "Исходные координаты: (%1,%2) -> (%3,%4)\n\n"
                                "Вычисление разностей:\n"
                                "dx = x2 - x1 = %5\n"
                                "dy = y2 - y1 = %6\n\n"
                                "Инициализация алгоритма:\n"
                                "x = |dx - dy| = |%7 - %8| = %9\n"
                                "y = |dy| = |%10| = %11\n\n"
                                "Строковые команды:\n"
                                "m1 = \"R\" (движение вправо)\n"
                                "m2 = \"D\" (диагональное движение)\n\n"
                                "Преобразования координат:\n"
                                "swapXY = %12, invertX = %13, invertY = %14")
                            .arg(x1).arg(y1).arg(x2).arg(y2)
                            .arg(x2 - x1).arg(y2 - y1)
                            .arg(x2 - x1).arg(y2 - y1).arg(qAbs((x2 - x1) - (y2 - y1)))
                            .arg(y2 - y1).arg(qAbs(y2 - y1))
                            .arg(qAbs(y2 - y1) > qAbs(x2 - x1) ? "да" : "нет")
                            .arg((x2 - x1) < 0 ? "да" : "нет")
                            .arg((y2 - y1) < 0 ? "да" : "нет");
        break;

    case 5:
        calculations += QString("Алгоритм Ву (сглаживание):\n"
                                "dx = x2 - x1 = %1 - %2 = %3\n"
                                "dy = y2 - y1 = %4 - %5 = %6\n"
                                "gradient = dy/dx = %7/%8 = %9\n\n"
                                "steep = |dy| > |dx| = |%10| > |%11| = %12")
                            .arg(x2).arg(x1).arg(x2 - x1)
                            .arg(y2).arg(y1).arg(y2 - y1)
                            .arg(y2 - y1).arg(x2 - x1).arg(float(y2 - y1) / (x2 - x1))
                            .arg(y2 - y1).arg(x2 - x1).arg(qAbs(y2 - y1) > qAbs(x2 - x1) ? "true" : "false");
        break;

    default:
        calculations += "Выберите алгоритм для просмотра подробных вычислений";
        break;
    }

    calculationsTextEdit->setPlainText(calculations);
}
