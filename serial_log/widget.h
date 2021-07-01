#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void serial_data_send_string(QString senddata);
    void serial_data_send_hex(QByteArray senddata);
    void serial_data_handle(const QByteArray &data);

    void readData();

    void on_serial_on_button_clicked();

    void on_serial_off_button_clicked();

    void on_search_button_clicked();

    void on_clearButton_clicked();

    void on_sendButton_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *serialPort;
};



#endif // WIDGET_H
