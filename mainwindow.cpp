#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    setWindowTitle("Qt串口助手");
    connect(ui->baudRateCmb,&QComboBox::currentIndexChanged,this,[=](){
      //设置波特率
      auto br=ui->baudRateCmb->currentData().value<QSerialPort::BaudRate>();
        if(!mSerialPort.setBaudRate(br)){
          QMessageBox::warning(this,"false","设置波特率失败"+mSerialPort.errorString());
        }
    });

    connect(ui->dataBitsCmb,&QComboBox::currentIndexChanged,this,[=](){
      //设置数据位
      auto value=ui->dataBitsCmb->currentData().value<QSerialPort::DataBits>();
      if(!mSerialPort.setDataBits(value)){
        QMessageBox::warning(this,"false","设置数据位失败"+mSerialPort.errorString());
      }
    });

    connect(ui->stopBitsCmb,&QComboBox::currentIndexChanged,this,[=](){
      //设置数据位
      auto value=ui->stopBitsCmb->currentData().value<QSerialPort::StopBits>();
      if(!mSerialPort.setStopBits(value)){
        QMessageBox::warning(this,"false","设置停止位失败"+mSerialPort.errorString());
      }
      qInfo()<<"sdflksjdfklsfkd";
    });

    connect(ui->parityCmb,&QComboBox::currentIndexChanged,this,[=](){
      //设置校验位
      auto value=ui->parityCmb->currentData().value<QSerialPort::Parity>();
      if(!mSerialPort.setParity(value)){
        QMessageBox::warning(this,"false","设置校验位失败"+mSerialPort.errorString());
      }
    });

    auto protsInfo=QSerialPortInfo::availablePorts();
    for(auto & info :protsInfo){
      qInfo()<<info.description()<<info.portName()<<info.systemLocation();
      ui->protsCmb->addItem(info.portName()+":"+info.description(),info.portName());
    }
    //获取标准波特率
    auto baudRates = QSerialPortInfo::standardBaudRates();
    for(auto br:baudRates){
      ui->baudRateCmb->addItem(QString::number(br),br);
    }
    ui->baudRateCmb->setCurrentText("9600");
    //设置停止位
    ui->stopBitsCmb->addItem("1",QSerialPort::OneStop);
    ui->stopBitsCmb->addItem("1.5",QSerialPort::OneAndHalfStop);
    ui->stopBitsCmb->addItem("2",QSerialPort::TwoStop);
    //设置数据位
    ui->dataBitsCmb->addItem("5",QSerialPort::Data5);
    ui->dataBitsCmb->addItem("6",QSerialPort::Data6);
    ui->dataBitsCmb->addItem("7",QSerialPort::Data7);
    ui->dataBitsCmb->addItem("8",QSerialPort::Data8);
    ui->dataBitsCmb->setCurrentText("8");
    //设置校验位
    ui->parityCmb->addItem("NoParty",QSerialPort::NoParity);
    ui->parityCmb->addItem("EvenParity",QSerialPort::EvenParity);
    ui->parityCmb->addItem("OddParity",QSerialPort::OddParity);
    ui->parityCmb->addItem("SpaceParity",QSerialPort::SpaceParity);
    ui->parityCmb->addItem("MarkParity",QSerialPort::MarkParity);
    //DTR
    mSerialPort.setDataTerminalReady(ui->checkDTR->isChecked());
    //RTS
    mSerialPort.setRequestToSend(ui->checkRTS->isChecked());

    connect(ui->btnClearData,&QPushButton::clicked,ui->recvEdit,&QPlainTextEdit::clear);
    connect(ui->btnClearSend,&QPushButton::clicked,ui->editCode,&QPlainTextEdit::clear);
    connect(&mSerialPort,&QSerialPort::readyRead,this,&MainWindow::onReadyRead);
    //定时发送 监听
    mTimer.callOnTimeout([=]{
      on_btnSend_clicked();
    });
}
//读取收到的信息
void MainWindow::onReadyRead(){

    auto data=mSerialPort.readAll();
    QString content= QString::fromLocal8Bit(data);
    if(ui->checkTimeStamp->isChecked()){
       QDateTime time = QDateTime::currentDateTime();
       QString timeStr = time.toString("yyyy-MM-dd hh:mm:ss ddd");
       content.append(" ").append(timeStr);
    }
    ui->recvEdit->appendPlainText(content);
}
void MainWindow::on_btnOpenPort_clicked()
{
    //关闭串口
    if(mSerialPort.isOpen()){
      mSerialPort.close();
      ui->btnOpenPort->setText("打开串口");
      if(mTimer.isActive()){
        mTimer.stop();
      }
      ui->btnOpenPort->setStyleSheet("QPushButton{background-color: rgb(201,201,201);}");
      return;
    }
    //auto baudTate=ui->baudRateCmb->currentData().value<QSerialPort::BaudRate>()

    auto portName=ui->protsCmb->currentData().toString();
    mSerialPort.setPortName(portName);
    if(!mSerialPort.open(QIODevice::ReadWrite)){
      QMessageBox::warning(this,"warning",portName+"open failed:"+mSerialPort.errorString());
      return;
    }
    ui->btnOpenPort->setStyleSheet("QPushButton{background-color: rgb(117,250,97);border-color:black;}");



    ui->btnOpenPort->setText("关闭串口");
}

//保存写入文件
void MainWindow::on_btnSaveData_clicked()
{
    auto fileName=getFile(false);
    if(fileName.isEmpty()){
      //用只读的方式打开文件
      QFile file(fileName);
      //是否可以以只读的方式打开
      if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        return;
      }
      QString str=ui->recvEdit->toPlainText();
      QByteArray sstrByTest= str.toUtf8();
      //写入文件
      file.write(sstrByTest,sstrByTest.length());
      file.close();
    }
}


void MainWindow::on_btnOpenFile_clicked(){
    auto fileName=getFile(false);
    if(fileName.isEmpty()){
      ui->editFilePath->setText(fileName);
    }
}
//选择文件
void MainWindow::on_btnSend_clicked()
{
    if(mSerialPort.isOpen()){
      auto  dataStr=ui->editCode->toPlainText();
      qInfo()<<"send:"<<dataStr;
      if(ui->checkSendHex->isChecked()){
        dataStr=dataStr.toLocal8Bit().toHex(' ');
      }
      if(ui->checNewLine->isChecked()){
        dataStr.append("\r\n");
      }
      mSerialPort.write(dataStr.toLocal8Bit());
    }

}
//发送文件
void MainWindow::on_btnSendFile_clicked()
{

  auto filename=ui->editFilePath->text();
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly)){
      QMessageBox::warning(this,"warning",filename+"open failed"+file.errorString());
      return;
  }
  mSerialPort.write(QString::fromUtf8(file.readAll()).toLocal8Bit());
}

//停止发送
void MainWindow::on_btnStopSend_clicked()
{
  mSerialPort.clear();
  if(mTimer.isActive()){
      mTimer.stop();
  }
}

//定时发送
void MainWindow::on_checkTimer_clicked(bool checked)
{
  if(checked){
      int timer=1000;
      if(!ui->timerPeriodEdit->text().isEmpty()){
        timer=ui->timerPeriodEdit->text().toUInt();
      }else{
        ui->timerPeriodEdit->setText(QString::number(timer));
      }
      mTimer.start(timer);
      ui->timerPeriodEdit->setEnabled(false);
  }else{
      mTimer.stop();
      ui->timerPeriodEdit->setEnabled(true);
  }
}



QString MainWindow::getFile(bool save)
{
  QString curPath=QCoreApplication::applicationDirPath();
  QString dlgTitle="选择一个文件";
  QString filter="文本文件(*.txt);;所有文件(*.*)";
  QString fileName="";
  if(save){
      fileName=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
  } else {
      fileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
  }

  if (fileName.isEmpty()){
      return "";
  }

  QFileInfo info(fileName);
  QDir::setCurrent(info.absoluteFilePath());
  return fileName;
}
//显示16进制
void MainWindow::on_hexDisplayChx_clicked(bool checked)
{
  if(checked){
      auto dataStr=ui->recvEdit->toPlainText();
      auto hexDat=dataStr.toLocal8Bit().toHex(' ').toUpper();
      ui->recvEdit->setPlainText(hexDat);
  } else{
      auto dataStr=ui->recvEdit->toPlainText();
      auto textData=QString::fromLocal8Bit(dataStr.toLocal8Bit());
      ui->recvEdit->setPlainText(textData);
  }
}
//DTR
void MainWindow::on_checkDTR_clicked(bool checked)
{
  if(mSerialPort.isOpen()){
      mSerialPort.setDataTerminalReady(checked);
  }
}

//RTS
void MainWindow::on_checkRTS_clicked(bool checked)
{
  if(mSerialPort.isOpen()){
      mSerialPort.setRequestToSend(checked);
  }
}

