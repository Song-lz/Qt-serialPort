#include "widget.h"
#include "ui_widget.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QScrollBar>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    autoRefrePortTimer = new QTimer;
    autoSendTimer = new QTimer;
    hexFlag = false;
    showTime  = false;
    autoSend  = false;
    showSend = false;
    connect(this->autoRefrePortTimer, SIGNAL(timeout()), this, SLOT(autoRefrePortTimerTimeOut()));
    connect(this->autoSendTimer, SIGNAL(timeout()), this, SLOT(autoSendTimerTimeOut()));

    serialPort = new QSerialPort;
    connect(this->serialPort, SIGNAL(readyRead()), this, SLOT(serialPortReadyRead()));

    connect(ui->openBt, SIGNAL(click()), this, SLOT(on_openBt_clicked()));
    connect(ui->clearBt, SIGNAL(click()), this, SLOT(on_clearBt_clicked()));
    connect(ui->closeBt, SIGNAL(click()), this, SLOT(on_closeBt_clicked()));
    connect(ui->sendBt, SIGNAL(click()), this, SLOT(on_sendBt_clicked()));

    /* 获取可用端口 */
    QStringList serialPortName;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        //qDebug() << "serialPortName" << info.portName();
        serialPortName << info.portName();
    }
    ui->portCB->addItems(serialPortName);
    /* 50ms刷新一次可用端口*/
    autoRefrePortTimer->start(50);
    // 设置换行模式 不换行
    ui->recvEdit->setLineWrapMode(QTextEdit::NoWrap);
    // 设置在单词中不换行
    ui->recvEdit->setWordWrapMode(QTextOption::NoWrap);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::serialPortReadyRead()
{
    QString data;
    QTime time;
    if(hexFlag == true){
        QByteArray ba = this->serialPort->readAll();
        data = QString(ba.toHex());

    }else{
        data = this->serialPort->readAll();
    }
    //qDebug()<<"recv " << data;
    QTextCursor tc = ui->recvEdit->textCursor();
    tc.movePosition(QTextCursor::End);
    /* 本次收到的数据集没有'\r',直接显示 */
    int  ret = data.indexOf("\r");
    if(ret == -1){
        tc.insertText(data);
        return;
    }
    /* 通过'\r'分割数据 */
    QStringList list = data.split("\r", QString::SkipEmptyParts);//QString字符串分割函数

    for(int i = 0; i< list.size();i++)
    {
        QString tmp = list.at(i);
        /* '\n'直接显示 */
        if(tmp == "\n") {
            tc.movePosition(QTextCursor::End);
            tc.insertText(tmp);
            continue;
        }
        /* 显示时间 */
        if(this->showTime == true){
            tc.movePosition(QTextCursor::StartOfLine);
            tc.insertText("[");
            tc.insertText(time.currentTime().toString("hh:mm:ss.zzz"));
            tc.insertText("]");
        }
        /* 移动光标 */
        tc.movePosition(QTextCursor::End);
        tc.insertText(tmp);

        tc.movePosition(QTextCursor::End);
        //qDebug()<< "["<< time.currentTime().toString("hh:mm:ss.zzz")<<"]"<< tmp;
    }
    /** 判断滚动条位置，在最底部则滚动显示文本 */
    /* 获取滚筒条位置 */
    QScrollBar *scrollbar = ui->recvEdit->verticalScrollBar();
    /* 判断当前滚动条位置和最底位置相差的距离*/
    bool scrollbarAtBottom  = (scrollbar->value() <= (scrollbar->maximum() - 20));
    int scrollbarPrevValue = scrollbar->value();

    /* 滚动条已经滑倒最底，则滚动刷新新数据 */
    if (!scrollbarAtBottom){
        ui->recvEdit->moveCursor(QTextCursor::End);
    }



    //tc.movePosition(QTextCursor::End);
    //tc.insertText(data);
    //ui->recvEdit->setFocus();
}

void Widget::autoRefrePortTimerTimeOut()
{

    QStringList serialPortName;
    /* 添加新插入的端口到列表中 */
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        serialPortName << info.portName();
        //qDebug()<<"get port name"<<info.portName();
        if(ui->portCB->findText(info.portName()) == -1){
            ui->portCB->addItem(info.portName());
        }
    }

    /* 移除已拔出的端口 */
    QStringList ret;
    for(int i = 0; i < ui->portCB->count(); i++){
        ret = serialPortName.filter(ui->portCB->itemText(i));
        if(ret.isEmpty() == true){
            ui->portCB->removeItem(i);
        }
    }
}

void Widget::autoSendTimerTimeOut()
{

    QByteArray data = ui->sendEdit->toPlainText().toUtf8();
    QTime time;
    /* 发送数据 */
    this->serialPort->write(data);
    QTextCursor tc = ui->recvEdit->textCursor();
    /* 显示发送则将数据添加到接受数据的文本中 */
    if(this->showSend == true){
        if(this->showTime == true){
            tc.movePosition(QTextCursor::StartOfLine);
            tc.insertText("[");
            tc.insertText(time.currentTime().toString("hh:mm:ss.zzz"));
            tc.insertText("]");
        }
        tc.movePosition(QTextCursor::End);
        if(this->hexFlag == true)tc.insertText(QString(data.toHex()));
        else tc.insertText(QString(data));
        tc.insertText("\r");
        tc.movePosition(QTextCursor::End);
    }
}

void Widget::on_openBt_clicked()
{
    qDebug()<<"on_openBt_clicked";
    if(ui->baudRateCb->currentText() == "9600"){
        baudrate = QSerialPort::Baud9600;
    }else if(ui->baudRateCb->currentText() == "38400"){
        baudrate = QSerialPort::Baud38400;
    }else if(ui->baudRateCb->currentText() == "74880"){
        baudrate = QSerialPort::Baud74880;
    }else if(ui->baudRateCb->currentText() == "115200"){
        baudrate = QSerialPort::Baud115200;
    }

    if(ui->dataBitsCb->currentText() == "8"){
        databits = QSerialPort::Data8;
    }else if(ui->dataBitsCb->currentText() == "7"){
        databits = QSerialPort::Data7;
    }else if(ui->dataBitsCb->currentText() == "6"){
        databits = QSerialPort::Data6;
    }else if(ui->dataBitsCb->currentText() == "5"){
        databits = QSerialPort::Data5;
    }

    if(ui->stopBitsCb->currentText() == "1"){
        stopbits = QSerialPort::OneStop;
    }else if(ui->stopBitsCb->currentText() == "1.5"){
        stopbits = QSerialPort::OneAndHalfStop;
    }else if(ui->stopBitsCb->currentText() == "2"){
        stopbits = QSerialPort::TwoStop;
    }

    if(ui->parityBitsCb->currentText() == "None"){
        parity = QSerialPort::NoParity;
    }else if(ui->parityBitsCb->currentText() == "Odd"){
        parity = QSerialPort::OddParity;
    }else if(ui->parityBitsCb->currentText() == "Even"){
        parity = QSerialPort::EvenParity;
    }else if(ui->parityBitsCb->currentText() == "Space"){
        parity = QSerialPort::SpaceParity;
    }else if(ui->parityBitsCb->currentText() == "Mark"){
        parity = QSerialPort::MarkParity;
    }

    this->serialPort->setPortName(ui->portCB->currentText());
    this->serialPort->setBaudRate(baudrate);
    this->serialPort->setDataBits(databits);
    this->serialPort->setStopBits(stopbits);
    this->serialPort->setParity(parity);
    if(this->serialPort->isOpen()){
        this->serialPort->close();
        ui->openBt->setStyleSheet(0);
    }else{
        if(this->serialPort->open(QIODevice::ReadWrite) == true){
            ui->openBt->setStyleSheet("background:green");
        }else{
            QMessageBox::information(this, "错误", "打开失败");
            ui->openBt->setStyleSheet(0);
        }
    }


}

void Widget::on_clearBt_clicked()
{
    ui->recvEdit->clear();
}

void Widget::on_sendBt_clicked()
{
    qDebug()<< "on_sendBt_clicked";
    QByteArray data = ui->sendEdit->toPlainText().toUtf8();
    QTime time;
    this->serialPort->write(data);
    QTextCursor tc = ui->recvEdit->textCursor();
    /* 显示发送则将数据添加到接受数据的文本中 */
    if(this->showSend == true){
        if(this->showTime == true){
            tc.movePosition(QTextCursor::StartOfLine);
            tc.insertText("[");
            tc.insertText(time.currentTime().toString("hh:mm:ss.zzz"));
            tc.insertText("]");
        }
        tc.movePosition(QTextCursor::End);
        if(this->hexFlag == true)tc.insertText(QString(data.toHex()));
        else tc.insertText(QString(data));
        tc.insertText("\r");
        tc.movePosition(QTextCursor::End);
    }

}
void Widget::on_closeBt_clicked()
{
    ui->openBt->setStyleSheet(0);
    serialPort->close();
}

void Widget::on_HexFlagCb_clicked()
{
    if(ui->HexFlagCb->isChecked() == true){
        hexFlag =true;
    }else{
        hexFlag = showSend;
    }
}

void Widget::on_showTimeCb_clicked()
{
    if(ui->showTimeCb->isChecked() == true){
        showTime =true;
    }else{
        showTime = false;
    }
}

void Widget::on_baudRateCb_activated(const QString &arg1)
{

}

void Widget::on_baudRateCb_currentIndexChanged(const QString &arg1)
{
    if(ui->baudRateCb->currentText() == "9600"){
        baudrate = QSerialPort::Baud9600;
    }else if(ui->baudRateCb->currentText() == "38400"){
        baudrate = QSerialPort::Baud38400;
    }else if(ui->baudRateCb->currentText() == "74880"){
        baudrate = QSerialPort::Baud74880;
    }else if(ui->baudRateCb->currentText() == "115200"){
        baudrate = QSerialPort::Baud115200;
    }
    this->serialPort->setBaudRate(baudrate);
}

void Widget::on_dataBitsCb_currentIndexChanged(const QString &arg1)
{
    if(ui->dataBitsCb->currentText() == "8"){
        databits = QSerialPort::Data8;
    }else if(ui->dataBitsCb->currentText() == "7"){
        databits = QSerialPort::Data7;
    }else if(ui->dataBitsCb->currentText() == "6"){
        databits = QSerialPort::Data6;
    }else if(ui->dataBitsCb->currentText() == "5"){
        databits = QSerialPort::Data5;
    }
}

void Widget::on_parityBitsCb_currentIndexChanged(const QString &arg1)
{
    if(ui->parityBitsCb->currentText() == "None"){
        parity = QSerialPort::NoParity;
    }else if(ui->parityBitsCb->currentText() == "Odd"){
        parity = QSerialPort::OddParity;
    }else if(ui->parityBitsCb->currentText() == "Even"){
        parity = QSerialPort::EvenParity;
    }else if(ui->parityBitsCb->currentText() == "Space"){
        parity = QSerialPort::SpaceParity;
    }else if(ui->parityBitsCb->currentText() == "Mark"){
        parity = QSerialPort::MarkParity;
    }
}

void Widget::on_stopBitsCb_currentIndexChanged(const QString &arg1)
{
    if(ui->stopBitsCb->currentText() == "1"){
        stopbits = QSerialPort::OneStop;
    }else if(ui->stopBitsCb->currentText() == "1.5"){
        stopbits = QSerialPort::OneAndHalfStop;
    }else if(ui->stopBitsCb->currentText() == "2"){
        stopbits = QSerialPort::TwoStop;
    }
}

void Widget::on_showSendCb_clicked()
{
    if(ui->showSendCb->isChecked() == true){
        showSend =true;
    }else{
        showSend = false;
    }
}


void Widget::on_autoSendCb_clicked()
{
    if(ui->autoSendCb->isChecked() == true){
        autoSend =true;
        this->autoSendTimer->stop();
        this->autoSendTimer->start(ui->autoSendTimeEdit->text().toInt());
    }else{
        this->autoSendTimer->stop();
        autoSend = false;
    }
}


void Widget::on_autoSendTimeEdit_editingFinished()
{
    if(autoSend == true){
        this->autoSendTimer->stop();
        this->autoSendTimer->start(ui->autoSendTimeEdit->text().toInt());
    }
}

void Widget::on_portCB_currentTextChanged(const QString &arg1)
{
    if(this->serialPort->isOpen()){
        this->serialPort->close();
        this->serialPort->setPortName(ui->portCB->currentText());
        if(this->serialPort->open(QIODevice::ReadWrite) == true){
            ui->openBt->setStyleSheet("background:green");
        }else{
            QMessageBox::information(this, "错误", "打开失败");
            ui->openBt->setStyleSheet(0);
        }
    }else{
        this->serialPort->setPortName(ui->portCB->currentText());
    }


}
