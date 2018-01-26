#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QStackedWidget>
#include <QBitmap>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_send_clicked();

    void readMyCom();

    void on_hex_display_stateChanged(int arg1);

    void on_button_clear_receieved_clicked();

    void on_button_clear_send_clicked();

    void on_clear_re_num_clicked();

    void on_clear_se_num_clicked();

    void on_send_regularly_stateChanged(int arg1);

    void check_port();

    void on_send_hex_stateChanged(int arg1);

    void on_mode_select_currentRowChanged(int currentRow);

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QSerialPort *my_serialPort;//(实例化一个指向串口的指针，可以用于访问串口)
    QByteArray requestData;//（用于存储从串口那读取的数据）
    QTimer *timer;//（用于计时）
    QTimer *re_timer;//(用于定时发送)
    QTimer *ch_timer;//(用于实时监测串口信息)
    bool ishex; //是否十六进制显示
    bool isregular;//是否定时发送
    bool sendhex; //是否十六进制发送
    int re_num; //接收数目
    int se_num; //发送数目
    QList<QString> ports_name; //可用串口列表
    bool port_exist[10]; //串口是否被拔掉
    QString currentport; //当下使用的串口
    bool re_camera; //是否摄像头显示
    int sta;//摄像头数据开始传输
    uchar imgs[10000];//储存摄像头数据
    int len;//imgs的长度计数
    int widthof; //imgs的宽
    int heightof; //imgs的长
    int imgtype; //img的图片类型
    bool bitseq; //数据顺序
    bool bitdis; //黑白配or白黑配
    int size;//像素总量
    QBitmap bitmap; //深度为1的位图
    bool addline;//发送结尾带/r/n
    bool xy;
};

#endif // MAINWINDOW_H
