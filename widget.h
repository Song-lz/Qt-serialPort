#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_openBt_clicked();
    void on_clearBt_clicked();
    void on_sendBt_clicked();
    void on_closeBt_clicked();

    void autoRefrePortTimerTimeOut();
    void autoSendTimerTimeOut();
    void serialPortReadyRead();
    void on_HexFlagCb_clicked();

    void on_showTimeCb_clicked();

    void on_baudRateCb_activated(const QString &arg1);

    void on_baudRateCb_currentIndexChanged(const QString &arg1);

    void on_dataBitsCb_currentIndexChanged(const QString &arg1);

    void on_parityBitsCb_currentIndexChanged(const QString &arg1);

    void on_stopBitsCb_currentIndexChanged(const QString &arg1);

    void on_showSendCb_clicked();


    void on_autoSendCb_clicked();


    void on_autoSendTimeEdit_editingFinished();

    void on_portCB_currentTextChanged(const QString &arg1);

private:
    Ui::Widget *ui;
    QSerialPort *serialPort;
    QTimer *autoRefrePortTimer;
    QTimer *autoSendTimer;
    bool hexFlag;
    bool showTime;
    bool showSend;
    bool autoSend;
    QSerialPort::BaudRate baudrate;
    QSerialPort::DataBits databits;
    QSerialPort::StopBits stopbits;
    QSerialPort::Parity parity;
    QSerialPort::FlowControl flowcontrol;
};

#endif // WIDGET_H
