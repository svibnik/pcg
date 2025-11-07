#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QThread>
#include <QScrollArea>

class FileProcessor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectFolder();
    void analyzeFiles();
    void updateProgress(int value, const QString &filename);
    void analysisFinished();
    void onFileProcessed(const QString &filename, const QSize &size,
                         int dpiX, int dpiY, int colorDepth,
                         const QString &format, const QString &compressionType,
                         double compressionRatio);

private:
    void setupUI();
    void setupTable();
    void addFileToTable(const QString &filename, const QSize &size,
                        int dpiX, int dpiY, int colorDepth,
                        const QString &format, const QString &compressionType,
                        double compressionRatio);

    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    QGroupBox *controlGroup;
    QHBoxLayout *controlLayout;
    QLineEdit *folderPath;
    QPushButton *selectButton;
    QPushButton *analyzeButton;

    QTableWidget *table;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    QString currentFolder;
    FileProcessor *processor;
    QThread *processorThread;
};

#endif
