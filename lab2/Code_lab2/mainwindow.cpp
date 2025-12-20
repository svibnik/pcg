#include "mainwindow.h"
#include "fileprocessor.h"
#include <QApplication>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , processor(nullptr)
    , processorThread(nullptr)
{
    setupUI();
    setWindowTitle("Image Analyzer - Graphics File Information");
    setFixedSize(1200, 700);
}

MainWindow::~MainWindow()
{
    if (processorThread) {
        processorThread->quit();
        processorThread->wait();
    }
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);

    controlGroup = new QGroupBox("Управление");
    controlLayout = new QHBoxLayout(controlGroup);

    folderPath = new QLineEdit();
    folderPath->setPlaceholderText("Выберите папку с изображениями...");
    folderPath->setReadOnly(true);

    selectButton = new QPushButton("Обзор...");
    analyzeButton = new QPushButton("Анализировать");
    analyzeButton->setEnabled(false);

    controlLayout->addWidget(folderPath);
    controlLayout->addWidget(selectButton);
    controlLayout->addWidget(analyzeButton);

    mainLayout->addWidget(controlGroup);

    setupTable();

    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    statusLabel = new QLabel("Выберите папку для анализа");
    mainLayout->addWidget(statusLabel);

    connect(selectButton, &QPushButton::clicked, this, &MainWindow::selectFolder);
    connect(analyzeButton, &QPushButton::clicked, this, &MainWindow::analyzeFiles);
}

void MainWindow::setupTable()
{
    table = new QTableWidget();
    table->setColumnCount(7);

    QStringList headers;
    headers << "Имя файла" << "Размер (пиксели)" << "Разрешение (DPI)"
            << "Глубина цвета" << "Формат" << "Сжатие (%)"
            << "Матрица квантования";

    table->setHorizontalHeaderLabels(headers);

    table->setColumnWidth(0, 250);
    table->setColumnWidth(1, 150);
    table->setColumnWidth(2, 120);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 120);
    table->setColumnWidth(5, 100);
    table->setColumnWidth(6, 150);

    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(true);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);

    connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::showQuantizationMatrix);

    mainLayout->addWidget(table);
}

void MainWindow::selectFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку с изображениями",
                                                       QDir::homePath(),
                                                       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folder.isEmpty()) {
        currentFolder = folder;
        folderPath->setText(folder);
        analyzeButton->setEnabled(true);
        statusLabel->setText(QString("Выбрана папка: %1").arg(folder));

        table->setRowCount(0);
    }
}

void MainWindow::analyzeFiles()
{
    if (currentFolder.isEmpty()) return;

    selectButton->setEnabled(false);
    analyzeButton->setEnabled(false);

    table->setRowCount(0);

    processorThread = new QThread(this);
    processor = new FileProcessor(currentFolder);
    processor->moveToThread(processorThread);

    connect(processorThread, &QThread::started, processor, &FileProcessor::processFiles);
    connect(processor, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
    connect(processor, &FileProcessor::fileProcessed, this, &MainWindow::onFileProcessed);
    connect(processor, &FileProcessor::finished, this, &MainWindow::analysisFinished);
    connect(processor, &FileProcessor::finished, processorThread, &QThread::quit);
    connect(processor, &FileProcessor::finished, processor, &QObject::deleteLater);
    connect(processorThread, &QThread::finished, processorThread, &QObject::deleteLater);

    progressBar->setVisible(true);
    progressBar->setValue(0);
    statusLabel->setText("Анализ файлов...");

    processorThread->start();
}

void MainWindow::updateProgress(int value, const QString &filename)
{
    progressBar->setValue(value);
    statusLabel->setText(QString("Обработка: %1 (%2%)").arg(filename).arg(value));
}

void MainWindow::onFileProcessed(const QString &filename, const QSize &size,
                                 int dpiX, int dpiY, int colorDepth,
                                 const QString &format, const QString &compressionType,
                                 double compressionRatio, const QString &quantizationInfo)
{
    addFileToTable(filename, size, dpiX, dpiY, colorDepth,
                   format, compressionType, compressionRatio, quantizationInfo);
    table->scrollToBottom();
}

void MainWindow::addFileToTable(const QString &filename, const QSize &size,
                                int dpiX, int dpiY, int colorDepth,
                                const QString &format, const QString &compressionType,
                                double compressionRatio, const QString &quantizationInfo)
{
    int row = table->rowCount();
    table->insertRow(row);

    QString dpiString;
    if (dpiX == dpiY) {
        dpiString = QString("%1").arg(dpiX);
    } else {
        dpiString = QString("%1 × %2").arg(dpiX).arg(dpiY);
    }

    table->setItem(row, 0, new QTableWidgetItem(filename));
    table->setItem(row, 1, new QTableWidgetItem(QString("%1 × %2").arg(size.width()).arg(size.height())));
    table->setItem(row, 2, new QTableWidgetItem(dpiString));
    table->setItem(row, 3, new QTableWidgetItem(QString("%1 бит").arg(colorDepth)));
    table->setItem(row, 4, new QTableWidgetItem(format));
    table->setItem(row, 5, new QTableWidgetItem(QString("%1%").arg(compressionRatio, 0, 'f', 1)));

    QTableWidgetItem *qtItem = new QTableWidgetItem();

    if (format.toUpper() == "JPEG" || format.toUpper() == "JPG") {
        if (!quantizationInfo.isEmpty() &&
            quantizationInfo != "Не найдено" &&
            !quantizationInfo.contains("не найдено")) {
            qtItem->setText("Есть (двойной клик)");
            qtItem->setData(Qt::UserRole, quantizationInfo);
            qtItem->setToolTip("Двойной клик для просмотра матрицы квантования");

            qtItem->setForeground(Qt::black);
            qtItem->setFont(QFont());

        } else {
            qtItem->setText("Нет данных");
            qtItem->setForeground(Qt::darkGray);
        }
    } else {
        qtItem->setText("Не применимо");
        qtItem->setForeground(Qt::darkGray);
    }

    table->setItem(row, 6, qtItem);

    table->viewport()->update();
}

void MainWindow::analysisFinished()
{
    selectButton->setEnabled(true);
    analyzeButton->setEnabled(true);
    progressBar->setVisible(false);
    statusLabel->setText(QString("Анализ завершен. Обработано файлов: %1").arg(table->rowCount()));

    processor = nullptr;
    processorThread = nullptr;
}

void MainWindow::showQuantizationMatrix(int row, int column)
{
    if (column != 4 && column != 6) {
        return;
    }

    QTableWidgetItem *formatItem = table->item(row, 4);
    if (!formatItem) return;

    QString format = formatItem->text().toUpper();
    if (format != "JPEG" && format != "JPG") {
        QMessageBox::information(this, "Информация", "Матрица квантования доступна только для JPEG файлов");
        return;
    }

    QTableWidgetItem *qtItem = table->item(row, 6);
    if (!qtItem) return;

    QString qtMatrix = qtItem->data(Qt::UserRole).toString();

    if (!qtMatrix.isEmpty() && qtMatrix != "Нет данных" && qtMatrix != "Не применимо") {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("Матрица квантования - " + table->item(row, 0)->text());
        dialog->setFixedSize(350, 200);

        QVBoxLayout *layout = new QVBoxLayout(dialog);

        QTextEdit *textEdit = new QTextEdit(dialog);
        textEdit->setPlainText(qtMatrix);
        textEdit->setReadOnly(true);

        QFont font("Consolas", 10);
        textEdit->setFont(font);
        textEdit->setLineWrapMode(QTextEdit::NoWrap);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        QPushButton *closeButton = new QPushButton("Закрыть", dialog);
        connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

        layout->addWidget(textEdit);
        layout->addWidget(closeButton);

        dialog->exec();
        dialog->deleteLater();
    } else {
        QMessageBox::information(this, "Информация", "Матрица квантования не найдена или недоступна для этого файла");
    }
}
