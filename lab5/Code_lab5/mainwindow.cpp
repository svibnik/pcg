#include "mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGroupBox>
#include <QtMath>
#include <QFile>
#include <QTextStream>

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

    QGroupBox *paramsGroup = new QGroupBox("Параметры отсечения");
    QVBoxLayout *paramsLayout = new QVBoxLayout(paramsGroup);

    QLabel *algorithmLabel = new QLabel("Алгоритм:");
    algorithmComboBox = new QComboBox;
    algorithmComboBox->addItem("Алгоритм средней точки (отрезки)");
    algorithmComboBox->addItem("Алгоритм отсечения выпуклого многоугольника");

    QGroupBox *windowGroup = new QGroupBox("Отсекающее окно");
    QGridLayout *windowLayout = new QGridLayout(windowGroup);

    windowLayout->addWidget(new QLabel("Xmin:"), 0, 0);
    xMinSpinBox = new QSpinBox;
    xMinSpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    xMinSpinBox->setValue(-5);
    windowLayout->addWidget(xMinSpinBox, 0, 1);

    windowLayout->addWidget(new QLabel("Ymin:"), 1, 0);
    yMinSpinBox = new QSpinBox;
    yMinSpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    yMinSpinBox->setValue(-5);
    windowLayout->addWidget(yMinSpinBox, 1, 1);

    windowLayout->addWidget(new QLabel("Xmax:"), 0, 2);
    xMaxSpinBox = new QSpinBox;
    xMaxSpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    xMaxSpinBox->setValue(5);
    windowLayout->addWidget(xMaxSpinBox, 0, 3);

    windowLayout->addWidget(new QLabel("Ymax:"), 1, 2);
    yMaxSpinBox = new QSpinBox;
    yMaxSpinBox->setRange(-GRID_EXTENT, GRID_EXTENT);
    yMaxSpinBox->setValue(5);
    windowLayout->addWidget(yMaxSpinBox, 1, 3);

    QLabel *gridLabel = new QLabel("Размер клетки:");
    gridSizeSlider = new QSlider(Qt::Horizontal);
    gridSizeSlider->setRange(10, 40);
    gridSizeSlider->setValue(CELL_SIZE);
    QLabel *gridSizeValue = new QLabel(QString::number(CELL_SIZE) + " px");

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    loadButton = new QPushButton("Загрузить данные");
    clipButton = new QPushButton("Выполнить отсечение");
    clearButton = new QPushButton("Очистить");
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addWidget(clipButton);
    buttonsLayout->addWidget(clearButton);

    paramsLayout->addWidget(algorithmLabel);
    paramsLayout->addWidget(algorithmComboBox);
    paramsLayout->addWidget(windowGroup);
    paramsLayout->addWidget(gridLabel);
    paramsLayout->addWidget(gridSizeSlider);
    paramsLayout->addWidget(gridSizeValue);
    paramsLayout->addLayout(buttonsLayout);

    QGroupBox *infoGroup = new QGroupBox("Информация");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);

    timeInfoLabel = new QLabel("Время выполнения:");
    timeInfoLabel->setFrameStyle(QFrame::Box);
    timeInfoLabel->setMinimumHeight(80);

    QLabel *calcLabel = new QLabel("Описание алгоритма:");
    algorithmDescriptionTextEdit = new QTextEdit;
    algorithmDescriptionTextEdit->setMaximumHeight(200);
    algorithmDescriptionTextEdit->setReadOnly(true);
    algorithmDescriptionTextEdit->setStyleSheet("QTextEdit { background-color: #f8f8f8; }");

    infoLayout->addWidget(timeInfoLabel);
    infoLayout->addWidget(calcLabel);
    infoLayout->addWidget(algorithmDescriptionTextEdit);

    controlLayout->addWidget(paramsGroup);
    controlLayout->addWidget(infoGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlWidget);

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadButtonClicked);
    connect(clipButton, &QPushButton::clicked, this, &MainWindow::onClipButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);

    connect(gridSizeSlider, &QSlider::valueChanged, this, &MainWindow::onGridSizeChanged);
    connect(gridSizeSlider, &QSlider::valueChanged, this, [=](int value) {
        gridSizeValue->setText(QString::number(value) + " px");
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

void MainWindow::drawLine(double x1, double y1, double x2, double y2, const QColor& color)
{
    int screenX1 = x1 * CELL_SIZE;
    int screenY1 = -y1 * CELL_SIZE;
    int screenX2 = x2 * CELL_SIZE;
    int screenY2 = -y2 * CELL_SIZE;

    QPen pen(color);
    pen.setWidth(2);
    scene->addLine(screenX1, screenY1, screenX2, screenY2, pen);
}

void MainWindow::drawRectangle(double xmin, double ymin, double xmax, double ymax, const QColor& color)
{
    int screenXmin = xmin * CELL_SIZE;
    int screenYmin = -ymin * CELL_SIZE;
    int screenXmax = xmax * CELL_SIZE;
    int screenYmax = -ymax * CELL_SIZE;

    QPen pen(color);
    pen.setWidth(2);

    scene->addLine(screenXmin, screenYmin, screenXmax, screenYmin, pen);
    scene->addLine(screenXmax, screenYmin, screenXmax, screenYmax, pen);
    scene->addLine(screenXmax, screenYmax, screenXmin, screenYmax, pen);
    scene->addLine(screenXmin, screenYmax, screenXmin, screenYmin, pen);
}

void MainWindow::drawPolygon(const QVector<QPointF>& points, const QColor& color)
{
    if (points.size() < 2) return;

    QPen pen(color);
    pen.setWidth(2);

    for (int i = 0; i < points.size(); i++) {
        int next = (i + 1) % points.size();
        double x1 = points[i].x() * CELL_SIZE;
        double y1 = -points[i].y() * CELL_SIZE;
        double x2 = points[next].x() * CELL_SIZE;
        double y2 = -points[next].y() * CELL_SIZE;
        scene->addLine(x1, y1, x2, y2, pen);
    }
}

int MainWindow::computeCode(double x, double y)
{
    int code = 0;

    if (x < clipWindow.left())
        code |= 1;
    if (x > clipWindow.right())
        code |= 2;
    if (y < clipWindow.top())
        code |= 4;
    if (y > clipWindow.bottom())
        code |= 8;

    return code;
}

bool MainWindow::isSegmentInside(double x1, double y1, double x2, double y2)
{
    return (x1 >= clipWindow.left() && x1 <= clipWindow.right() &&
            y1 >= clipWindow.top() && y1 <= clipWindow.bottom() &&
            x2 >= clipWindow.left() && x2 <= clipWindow.right() &&
            y2 >= clipWindow.top() && y2 <= clipWindow.bottom());
}

bool MainWindow::isSegmentOutside(double x1, double y1, double x2, double y2)
{
    if ((x1 < clipWindow.left() && x2 < clipWindow.left()) ||
        (x1 > clipWindow.right() && x2 > clipWindow.right()) ||
        (y1 < clipWindow.top() && y2 < clipWindow.top()) ||
        (y1 > clipWindow.bottom() && y2 > clipWindow.bottom())) {
        return true;
    }
    return false;
}

void MainWindow::clipMidpointRecursive(double x1, double y1, double x2, double y2, const QColor& color)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double length = sqrt(dx * dx + dy * dy);
    if (length < (1.0 / CELL_SIZE)) {
        return;
    }

    if (isSegmentInside(x1, y1, x2, y2)) {
        drawLine(x1, y1, x2, y2, color);
        return;
    }

    if (isSegmentOutside(x1, y1, x2, y2)) {
        return;
    }

    double midX = (x1 + x2) / 2.0;
    double midY = (y1 + y2) / 2.0;

    clipMidpointRecursive(x1, y1, midX, midY, color);
    clipMidpointRecursive(midX, midY, x2, y2, color);
}

void MainWindow::clipMidpoint(double x1, double y1, double x2, double y2, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    clipMidpointRecursive(x1, y1, x2, y2, color);

    timeMeasurements["Midpoint"] = timer.nsecsElapsed();
}

bool MainWindow::isInside(const QPointF& p, const QPointF& A, const QPointF& B)
{
    return (B.x() - A.x()) * (p.y() - A.y()) - (B.y() - A.y()) * (p.x() - A.x()) >= 0;
}

QPointF MainWindow::computeIntersection(const QPointF& p1, const QPointF& p2, const QPointF& A, const QPointF& B)
{
    double x1 = p1.x(), y1 = p1.y();
    double x2 = p2.x(), y2 = p2.y();
    double x3 = A.x(), y3 = A.y();
    double x4 = B.x(), y4 = B.y();

    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if (qFuzzyIsNull(denom)) {
        return p1;
    }

    double x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
    double y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

    return QPointF(x, y);
}

void MainWindow::clipPolygon(const QVector<QPointF>& inputPolygon, const QColor& color)
{
    QElapsedTimer timer;
    timer.start();

    if (inputPolygon.size() < 3) {
        timeMeasurements["PolygonClip"] = timer.nsecsElapsed();
        return;
    }

    QVector<QPointF> outputList = inputPolygon;

    QVector<QPointF> clipBoundaries = {
        QPointF(clipWindow.left(), clipWindow.top()),
        QPointF(clipWindow.right(), clipWindow.top()),
        QPointF(clipWindow.right(), clipWindow.bottom()),
        QPointF(clipWindow.left(), clipWindow.bottom())
    };

    for (int i = 0; i < clipBoundaries.size(); i++) {
        QVector<QPointF> inputList = outputList;
        outputList.clear();

        if (inputList.isEmpty()) break;

        QPointF A = clipBoundaries[i];
        QPointF B = clipBoundaries[(i + 1) % clipBoundaries.size()];

        for (int j = 0; j < inputList.size(); j++) {
            QPointF current = inputList[j];
            QPointF previous = inputList[(j - 1 + inputList.size()) % inputList.size()];

            bool currentInside = isInside(current, A, B);
            bool previousInside = isInside(previous, A, B);

            if (previousInside != currentInside) {
                QPointF intersection = computeIntersection(previous, current, A, B);
                outputList.append(intersection);
            }

            if (currentInside) {
                outputList.append(current);
            }
        }
    }

    if (!outputList.isEmpty()) {
        drawPolygon(outputList, color);
    }

    timeMeasurements["PolygonClip"] = timer.nsecsElapsed();
}

void MainWindow::onLoadButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить данные", "", "Text files (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    segments.clear();
    polygons.clear();
    timeMeasurements.clear();

    QTextStream in(&file);
    QString fileType;
    in >> fileType;

    if (fileType == "SEGMENTS") {
        int n;
        in >> n;
        for (int i = 0; i < n; i++) {
            double x1, y1, x2, y2;
            in >> x1 >> y1 >> x2 >> y2;
            segments.append(QLineF(x1, y1, x2, y2));
        }
    } else if (fileType == "POLYGON") {
        int n;
        in >> n;
        QVector<QPointF> polygon;
        for (int i = 0; i < n; i++) {
            double x, y;
            in >> x >> y;
            polygon.append(QPointF(x, y));
        }
        polygons.append(polygon);
    } else {
        int n = fileType.toInt();
        for (int i = 0; i < n; i++) {
            double x1, y1, x2, y2;
            in >> x1 >> y1 >> x2 >> y2;
            segments.append(QLineF(x1, y1, x2, y2));
        }
    }

    double xmin, ymin, xmax, ymax;
    in >> xmin >> ymin >> xmax >> ymax;
    clipWindow = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);

    file.close();

    xMinSpinBox->setValue(xmin);
    yMinSpinBox->setValue(ymin);
    xMaxSpinBox->setValue(xmax);
    yMaxSpinBox->setValue(ymax);

    setupScene();

    drawRectangle(clipWindow.left(), clipWindow.top(), clipWindow.right(), clipWindow.bottom(), Qt::red);

    if (!segments.isEmpty()) {
        for (const QLineF& segment : segments) {
            drawLine(segment.x1(), segment.y1(), segment.x2(), segment.y2(), Qt::blue);
        }
        QMessageBox::information(this, "Успех", QString("Загружено %1 отрезков").arg(segments.size()));
    } else if (!polygons.isEmpty()) {
        for (const QVector<QPointF>& polygon : polygons) {
            drawPolygon(polygon, Qt::blue);
        }
        QMessageBox::information(this, "Успех", QString("Загружен многоугольник с %1 вершинами").arg(polygons[0].size()));
    }

    drawAlgorithmDescription();
}

void MainWindow::onClipButtonClicked()
{
    int algorithmIndex = algorithmComboBox->currentIndex();

    if (algorithmIndex == 0 && segments.isEmpty()) {
        QMessageBox::warning(this, "Ошибка",
                             "Для алгоритма отсечения отрезков нужен файл с отрезками (SEGMENTS)\n"
                             "Текущие данные: " +
                                 (segments.isEmpty() ? "нет отрезков" : QString("%1 отрезков").arg(segments.size())) +
                                 (polygons.isEmpty() ? "" : QString(", %1 многоугольников").arg(polygons.size())));
        return;
    }

    if (algorithmIndex == 1 && polygons.isEmpty()) {
        QMessageBox::warning(this, "Ошибка",
                             "Для алгоритма отсечения многоугольников нужен файл с многоугольником (POLYGON)\n"
                             "Текущие данные: " +
                                 (segments.isEmpty() ? "" : QString("%1 отрезков").arg(segments.size())) +
                                 (polygons.isEmpty() ? "нет многоугольников" : QString(", %1 многоугольников").arg(polygons.size())));
        return;
    }

    if (segments.isEmpty() && polygons.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для отсечения. Загрузите файл с данными.");
        return;
    }

    setupScene();
    timeMeasurements.clear();

    clipWindow = QRectF(xMinSpinBox->value(), yMinSpinBox->value(),
                        xMaxSpinBox->value() - xMinSpinBox->value(),
                        yMaxSpinBox->value() - yMinSpinBox->value());

    drawRectangle(clipWindow.left(), clipWindow.top(), clipWindow.right(), clipWindow.bottom(), Qt::red);

    switch (algorithmIndex) {
    case 0:
        for (const QLineF& segment : segments) {
            drawLine(segment.x1(), segment.y1(), segment.x2(), segment.y2(), Qt::gray);
        }
        for (const QLineF& segment : segments) {
            clipMidpoint(segment.x1(), segment.y1(), segment.x2(), segment.y2(), Qt::blue);
        }
        break;

    case 1:
        for (const QVector<QPointF>& polygon : polygons) {
            drawPolygon(polygon, Qt::gray);
        }
        for (const QVector<QPointF>& polygon : polygons) {
            clipPolygon(polygon, Qt::green);
        }
        break;
    }

    updateTimeInfo();
    drawAlgorithmDescription();
}

void MainWindow::onClearButtonClicked()
{
    segments.clear();
    polygons.clear();
    timeMeasurements.clear();

    clipWindow = QRectF(-5, -5, 10, 10);
    xMinSpinBox->setValue(-5);
    yMinSpinBox->setValue(-5);
    xMaxSpinBox->setValue(5);
    yMaxSpinBox->setValue(5);

    setupScene();
    timeInfoLabel->setText("Время выполнения:");
    algorithmDescriptionTextEdit->clear();
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

void MainWindow::drawAlgorithmDescription()
{
    QString description = "Описание алгоритма:\n\n";

    int algorithmIndex = algorithmComboBox->currentIndex();

    if (algorithmIndex == 0 && segments.isEmpty()) {
        description = "ОШИБКА: Для алгоритма отсечения отрезков нужен файл с отрезками (SEGMENTS)\n\n"
                      "Загрузите файл с тегом SEGMENTS в начале файла";
        algorithmDescriptionTextEdit->setPlainText(description);
        return;
    }

    if (algorithmIndex == 1 && polygons.isEmpty()) {
        description = "ОШИБКА: Для алгоритма отсечения многоугольников нужен файл с многоугольником (POLYGON)\n\n"
                      "Загрузите файл с тегом POLYGON в начале файла";
        algorithmDescriptionTextEdit->setPlainText(description);
        return;
    }

    switch (algorithmIndex) {
    case 0:
        description += QString("Алгоритм средней точки (рекурсивный):\n\n"
                               "Отсекающее окно: [%1, %2] - [%3, %4]\n\n"
                               "Принцип работы:\n"
                               "1. Если длина отрезка меньше размера пикселя - завершаем\n"
                               "2. Если отрезок полностью внутри окна - отображаем его\n"
                               "3. Если отрезок полностью вне окна - отбрасываем\n"
                               "4. Иначе делим отрезок пополам и рекурсивно обрабатываем обе половины\n\n"
                               "Преимущества: простая реализация, не требует вычисления точек пересечения\n"
                               "Недостатки: может быть медленнее для длинных отрезков");
        break;

    case 1:
        description += QString("Алгоритм отсечения выпуклого многоугольника (Сазерленда-Ходжмана):\n\n"
                               "Принцип работы:\n"
                               "1. Для каждой границы отсекающего окна:\n"
                               "2. Берем последовательные вершины многоугольника\n"
                               "3. Проверяем каждое ребро на пересечение с текущей границей\n"
                               "4. Если ребро пересекает границу - добавляем точку пересечения\n"
                               "5. Если вершина внутри - добавляем её\n"
                               "6. Переходим к следующей границе окна\n\n"
                               "Результат - новый отсеченный многоугольник\n\n"
                               "Особенности: работает с выпуклыми многоугольниками, сохраняет порядок вершин");
        break;

    default:
        description += "Выберите алгоритм для просмотра описания";
        break;
    }

    algorithmDescriptionTextEdit->setPlainText(description);
}

void MainWindow::onAlgorithmChanged(int index)
{
    drawAlgorithmDescription();
}
