#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QBitmap>
#include <QImage>
#include <QPixmap>
#include <QTextStream>
#include <QPalette>
#include <stdio.h>
#include <QThread>
#include <QTextCursor>
QTextStream cout(stdout,  QIODevice::WriteOnly);
void MainWindow::check_port()
{
    if(ports_name.size()<QSerialPortInfo::availablePorts().size())
    {
        foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())
        {
            if(ports_name.contains(Info.portName()))
            {
                continue;
            }
            else{
                ports_name.append(Info.portName());
                ui->comboBox_portName->addItem( Info.portName() );
            }
        }
    }
    if(ports_name.size()>QSerialPortInfo::availablePorts().size())
    {
        for(int i=0;i<ports_name.size();i++)
        {
            port_exist[i]=false;
        }
        foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())
        {
            if(ports_name.contains(Info.portName()))
            {
                port_exist[ports_name.indexOf(Info.portName())]=true;
                continue;
            }
        }
        for(int i=0;i<ports_name.size();i++)
        {
            if(port_exist[i]==false)
            {
                if(currentport==ports_name[i])
                {
                    QMessageBox::warning(NULL, QString("报错啦"), QString("<串口异常关闭>"));
                    my_serialPort->close();
                    timer->stop();
                    ch_timer->start(200);
                    ui->prompt_label->setStyleSheet("color:black;");
                    currentport.clear();
                }
                ports_name.removeAt(i);
                ui->comboBox_portName->removeItem(i);

            }
        }
    }
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ishex=false;
    isregular=false;
    re_num=0;
    se_num=0;
    sendhex=false;
    re_camera=false;
    currentport="";
    sta=0;
    len=0;
    addline=true;
    ui->mode_select->setCurrentRow(0);
    ui->model_window->setCurrentIndex(1);
    timer = new QTimer(this);
    connect( timer, SIGNAL( timeout() ), this, SLOT( readMyCom() ) );
    re_timer = new QTimer(this);
    connect( re_timer, SIGNAL( timeout() ), this, SLOT(on_pushButton_send_clicked()) );
    ch_timer = new QTimer(this);
    connect( ch_timer, SIGNAL( timeout() ), this, SLOT(check_port()));
    xy=0;
    ch_timer->start(1000);//每秒读一次
    ui->comboBox_portName->clear();
    foreach( const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())//读取串口信息
    {
        qDebug() << "portName    :"  << Info.portName();//调试时可以看的串口信息
        qDebug() << "Description   :" << Info.description();
        qDebug() << "Manufacturer:" << Info.manufacturer();

        QSerialPort serial;
        serial.setPort(Info);
        if( serial.open( QIODevice::ReadWrite) )//如果串口是可以读写方式打开的
        {
            ui->comboBox_portName->addItem( Info.portName() );//在comboBox那添加串口号
            ports_name.append(Info.portName());
            serial.close();//然后自动关闭等待人为开启（通过那个打开串口的PushButton）
        }
    }
}

MainWindow::~MainWindow()
{
    my_serialPort->close();
    timer->stop();
    ch_timer->stop();
    re_timer->stop();
    delete timer;
    delete ch_timer;
    delete re_timer;
    delete my_serialPort;
    delete ui;
}
void MainWindow::readMyCom()//读取缓冲的数据，每秒读一次
{
    //int x,y;

    if(my_serialPort->isReadable())//如果到数据,就处理
    {
        if(re_camera==true)
        {
            if(sta==0)
            {
                requestData=my_serialPort->read(2);
                QString be=requestData.toHex().data();
                be=be.toUpper();

                if(be.mid(0,4)=="01FE")
                {
                    cout<<"start"<<endl;
                    sta=1;
                }
            }
            else if(sta==1)
            {
                if(size==0)
                {
                    requestData=my_serialPort->read(2);
                    QString be=requestData.toHex().data();
                    be=be.toUpper();
                    if(be.mid(0,4)=="FE01")
                    {
                        xy=1;

                        size=widthof*heightof/8;
                        sta=0;
                        len=0;
                        bitmap=QBitmap::fromData(QSize(widthof,heightof),imgs,QImage::Format_Mono);
                        ui->bitimage->setPixmap(bitmap);
                    }
                }
                else
                {
                    int num=size>200?200:size;
                    size=size-num;
                    char da[num];
                    //cout<<"ing"<<a<<": "<<num<<endl;
                    if(my_serialPort->read(da,num)>0)
                    {
                        for(int i=0;i<num;i++)
                        {
                            imgs[len++]=(uchar)da[i];
                        }
                    }
                }

            }

        }
        else
        {
            requestData=my_serialPort->readAll();
            re_num += requestData.length();
            if(re_num>=999999999)
            {
                re_num=0;
            }
            ui->receieve_num->setNum(re_num);
            if(ishex==false)
            {
                QString myStrTemp = QString::fromLocal8Bit(requestData);
                ui->textEdit_read->insertPlainText(myStrTemp);
            }
            else
            {
                QString strDis;
                QString myhex = requestData.toHex().data();
                myhex=myhex.toUpper();
                for(int i=0;i<myhex.length();i+=2)
                {
                    QString st=myhex.mid(i,2);
                    strDis+=st;
                    strDis+=" ";
                }
                ui->textEdit_read->append(strDis);
            }
            QTextCursor cursor = ui->textEdit_read->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit_read->setTextCursor(cursor);
        }
    }
    requestData.clear();    //清除缓冲区
}
void MainWindow::on_pushButton_open_clicked()
{
    if(ports_name.size()==0)
    {
        QMessageBox::warning(NULL, QString("报错啦"), QString("<无串口可使用>"));
    }
    else if(!timer->isActive())
    {
        my_serialPort = new QSerialPort(this);
        my_serialPort->setPortName( ui->comboBox_portName->currentText());
        if(!my_serialPort->open( QIODevice::ReadWrite ))
        {
            QMessageBox::warning(NULL, QString("报错啦"), QString("<该串口已打开>"));
            my_serialPort->clear();
            return;
        }
        qDebug() << ui->comboBox_portName->currentText();
        my_serialPort->setBaudRate(  ui->comboBox_baudRate->currentText().toInt() );//波特率
        my_serialPort->setDataBits( QSerialPort::Data8 );//数据字节，8字节
        my_serialPort->setParity( QSerialPort::NoParity );//校验，无
        my_serialPort->setFlowControl( QSerialPort::NoFlowControl );//数据流控制,无
        my_serialPort->setStopBits( QSerialPort::OneStop );//一位停止位
        ui->prompt_label->setStyleSheet("color:red;");
        currentport=my_serialPort->portName();
        if(re_camera==true)
        {
//            if(ui->img_type->currentText()=="二值化图像")
//            {
//                imgtype=0;
//            }
            widthof=ui->img_width->text().toInt();
            heightof=ui->img_height->text().toInt();
//            if(ui->bit_sequence->currentText()=="先高后低")
//            {
//                bitseq=0;
//            }
//            if(ui->white_black->currentText()=="1黑0白")
//            {
//                bitdis=0;
//            }
            size=widthof*heightof/8;
            timer->start(50);//每15m秒读一次
        }
        else
        {
            timer->start(200);
        }
    }
    else
    {
        QMessageBox::warning(NULL, QString("报错啦"), QString("<请先关闭串口>"));
    }
}

void MainWindow::on_pushButton_close_clicked()
{
    if(timer->isActive()){
         QByteArray cl=my_serialPort->readAll();
         my_serialPort->close();
         timer->stop();
         ch_timer->start(1000);
         ui->prompt_label->setStyleSheet("color:black;");
         currentport="";
         sta=0;
         len=0;
    }
    else
    {
        QMessageBox::warning(NULL, QString("报错啦"), QString("<请先打开串口>"));
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    if(timer->isActive()){
        QString sendData = ui->lineEdit_write->toPlainText();
        se_num += sendData.length();
        if(se_num>=999999999){
            se_num=0;
        }
        ui->send_num->setNum(se_num);
        if(sendhex==false)
        {
            my_serialPort->write(sendData.toUtf8());
        }
        else
        {
            QByteArray sendda;
            sendda.append(sendData);
            my_serialPort->write(sendda.toHex());
        }
        if(addline==true)
        {
            my_serialPort->write("\r\n");
        }
    }
    else
    {
        QMessageBox::warning(NULL, QString("报错啦"), QString("<请先打开串口>"));
        ui->send_regularly->setCheckState(Qt::Unchecked);
    }
}
void MainWindow::on_hex_display_stateChanged(int arg1)
{
    if(arg1==Qt::Checked)
    {
        ishex=true;
    }
    else{
        ishex=false;
    }
}

void MainWindow::on_button_clear_receieved_clicked()
{
    ui->textEdit_read->clear();
}

void MainWindow::on_button_clear_send_clicked()
{
    ui->lineEdit_write->clear();
}

void MainWindow::on_clear_re_num_clicked()
{
    re_num=0;
    ui->receieve_num->setNum(re_num);
}

void MainWindow::on_clear_se_num_clicked()
{
    se_num=0;
    ui->send_num->setNum(se_num);
}

void MainWindow::on_send_regularly_stateChanged(int arg1)
{
    if(arg1==Qt::Checked)
    {
        QString time=ui->timing->text();
        isregular=true;
        re_timer->start(time.toInt());//每秒读一次
    }
    else
    {
        isregular=false;
        re_timer->stop();
    }
}

void MainWindow::on_send_hex_stateChanged(int arg1)
{
    if(arg1==Qt::Checked)
    {
        sendhex=true;
    }
    else
    {
        sendhex=false;
    }
}

void MainWindow::on_mode_select_currentRowChanged(int currentRow)
{
     ui->model_window->setCurrentIndex(currentRow==1?0:1);
     if(currentRow==1)
     {
        re_camera=true;
     }
     else
     {
        re_camera=false;
     }
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if(arg1==Qt::Checked)
    {
        addline=true;
    }
    else
    {
        addline=false;
    }
}
