#include "widget.h"
#include "ui_widget.h"
#include "crc.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    this->setWindowTitle("serial");
    /* 创建一个串口对象 */
    serialPort = new QSerialPort(this);
    ui->serial_off_button->setEnabled(false);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_serial_on_button_clicked()
{

    /* 串口设置 */
    serialPort->setPortName(ui->serial_select->currentText());
    serialPort->setBaudRate(ui->baudrate_select->currentText().toInt());
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setParity(QSerialPort::NoParity);

    /* 打开串口提示框 */
    if (true == serialPort->open(QIODevice::ReadWrite))
    {
        ui->serial_on_button->setEnabled(false);
        ui->serial_off_button->setEnabled(true);
        connect(serialPort, &QSerialPort::readyRead, this, &Widget::readData);
    }
    else
    {
        QMessageBox::critical(this, "提示", "串口打开失败");
    }
}

void Widget::readData()
{
    QByteArray buf;
    buf = serialPort->readAll();
    serial_data_handle(buf);
    if(ui->hex_display->isChecked() == true)
    {
        buf = buf.toHex();
        QString redata = QString(buf);
        ui->result_text->insertPlainText(redata);
    }
    else
    {
        ui->result_text->insertPlainText(buf);
    }
}

void Widget::on_serial_off_button_clicked()
{
    serialPort->close();
    ui->serial_off_button->setEnabled(false);
    ui->serial_on_button->setEnabled(true);
}

void Widget::on_search_button_clicked()
{
    QStringList serialNamePort;
    ui->serial_select->clear();
    /* 搜索所有可用串口 */
    foreach (const QSerialPortInfo &inf0, QSerialPortInfo::availablePorts()) {
        serialNamePort<<inf0.portName();
    }
    ui->serial_select->addItems(serialNamePort);
}

void Widget::on_clearButton_clicked()
{
    ui->result_text->clear();
}

void Widget::on_sendButton_clicked()
{
    if(serialPort->isOpen()==false)  //判断串口是否正常打开
    {
        QMessageBox::warning(NULL , "提示", "请打开串口！");
        return;
    }
    QByteArray senddata = ui->send_text->toPlainText().toUtf8();
    //判断是否有非16进制字符
    if(ui->hex_send->isChecked()==true) //勾选了16进制发送
     {
        int cnt = senddata.size();          //要发送数据的长度

        char *data = senddata.data();
        for(int i=0;i<cnt;i++)//判断是否有非16进制字符
        {
            if(data[i]>='0' && (data[i]<='9'))
                continue;
            else if(data[i]>='a' && (data[i]<='f'))
                continue;
            else if(data[i]>='A' && (data[i]<='F'))
                continue;
            else if(data[i] == ' ')     //输入为空格
                continue;
            else
            {
                QMessageBox::warning(NULL , "提示", "输入非16进制字符！");
                return;
            }
        }

        //字符串转化为16进制数   "1234" --> 0X1234
        //转换时会自动除去非16进制字符
        senddata = senddata.fromHex(senddata);
    }
    serialPort->write(senddata);
}

void Widget::serial_data_send_hex(QByteArray senddata)
{
    serialPort->write(senddata);
}

void Widget::serial_data_send_string(QString senddata)
{
    QByteArray send = senddata.toUtf8();
    send = send.fromHex(send);
    serialPort->write(send);
}

QString handShakeRequest    = "be500003000001ed";
QString handShakeSuccess    = "be500003020001eb";
QString ramrunAddrSuccess   = "be53000100ed";
QString loadRamrunSuccess   = "be54a201202a";

QString sendHandshake       = "be50000101ef";
//QString sendSetRamrunAddr   = "be53000cdc050120c4080100f008fbef31";
QString sendSetRamrunHeard  = "be53000c";

#define RAMRUN_BUFF_OFFSET  1052
#define RAMRUN_TAIL_LEN     4

void Widget::serial_data_handle(const QByteArray &data)
{
    QString redata = QString(data.toHex());
    if(redata.compare(handShakeRequest) == 0)
    {
        qDebug()<<"handshake cmd";
        serial_data_send_string(sendHandshake);
    }
    if(redata.compare(handShakeSuccess) == 0)
    {
        qDebug()<<"handShakeSuccess cmd";
        QFile file("./programmer1400.bin");
        if(!file.open(QIODevice::ReadWrite))
        {
           qDebug("open fail ");
           QMessageBox::critical(this, "提示", "文件打开失败");
        }
        QByteArray buf = file.readAll();//读取文件所有数据
        file.close();//关闭文件
        QByteArray sendbuf;
        QByteArray headbuf = QByteArray::fromHex(sendSetRamrunHeard.toLocal8Bit());
        QByteArray codeEntrybuf = buf.mid(buf.length()-RAMRUN_TAIL_LEN, RAMRUN_TAIL_LEN);
        QByteArray subbuf = buf.remove(0, RAMRUN_BUFF_OFFSET);
        subbuf = subbuf.remove(subbuf.length()-RAMRUN_TAIL_LEN, RAMRUN_TAIL_LEN);
        uint32_t crc = CRC32(subbuf, subbuf.length());
        QByteArray crcbuf = intToByteArray(crc);
        QByteArray lenbuf = intToByteArray(subbuf.length());
        sendbuf = headbuf.append(codeEntrybuf).append(crcbuf).append(lenbuf);
        QByteArray checksumbuf = u8ToByteArray(besChecksum(sendbuf));
        sendbuf = sendbuf.append(checksumbuf);
        serial_data_send_hex(sendbuf);
    }
    if(redata.compare(ramrunAddrSuccess) == 0)
    {
        qDebug()<<"set ramrun addr success";
    }
}

