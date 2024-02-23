#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
    QString getFile(bool isSave);
private slots:
    void onReadyRead();
    void on_btnOpenPort_clicked();

    void on_btnSaveData_clicked();

    void on_btnOpenFile_clicked();

    void on_btnSend_clicked();

    void on_btnSendFile_clicked();

    void on_btnStopSend_clicked();

    void on_checkTimer_clicked(bool checked);

    void on_hexDisplayChx_clicked(bool checked);

    void on_checkDTR_clicked(bool checked);

    void on_checkRTS_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QSerialPort mSerialPort;
    QTimer mTimer;
};
#endif // MAINWINDOW_H
