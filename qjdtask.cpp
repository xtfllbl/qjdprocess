#include "qjdtask.h"
#include "ui_qjdtask.h"
#include <QDebug>
#define lineWidth 256
qjdTask::qjdTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdTask)
{
    ui->setupUi(this);
    historyTableColNumber=0;
    historyTableRowNumber=0;
    setTableData();
}

qjdTask::~qjdTask()
{
    delete ui;
}

void qjdTask::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void qjdTask::setTableData()
{
    qDebug()<<"setTableData();";
   /// 会发现很多同名的作业，如何区分？
   // 由于是append，所以为读取到最后一个为准
   // TODO:文件变大之后非常缓慢，函数需要分离,遍历的事情只要做一边
   fp.setFileName("/home/xtf/pathFile.txt");
   fp.open(QFile::ReadOnly);
   while(!fp.atEnd())
   {
       QString a=fp.readLine(lineWidth);
       if(a.contains("----------")==true)
       {
           historyTableRowNumber++;
           a=a.right(a.size()-10);
           a=a.left(a.size()-11);       //因为结尾处有\n，所以要加1
           pname<<a;
           QString b=fp.readLine(lineWidth);
           if(b.contains("Private File Path"))
           {
               b=fp.readLine(lineWidth);
               path<<b.left(b.indexOf("\n"));
           }

           QString c=fp.readLine(lineWidth);
           if(c.contains("Arguments"))
           {
               c=fp.readLine(lineWidth);
               arguments<<c.left(c.indexOf("\n"));
           }
           QString d=fp.readLine(lineWidth);
           if(d.contains("Start Time"))
           {
               d=fp.readLine(lineWidth);
               stime<<d.left(d.indexOf("\n"));
           }
       }
   }
   fp.close();

//   qDebug()<<stime;
   ui->historyTable->setRowCount(historyTableRowNumber);
//   ui->historyTable->setColumnCount(4);

//   qDebug()<<path.size()<<arguments.size()<<stime.size();
   for(int i=0;i<path.size();i++)
   {
       QTableWidgetItem *itemPname = new QTableWidgetItem(pname[i]);
//       QTableWidgetItem *itemPath = new QTableWidgetItem(path[i]);
       QTableWidgetItem *itemArgu = new QTableWidgetItem(arguments[i]);
       QTableWidgetItem *itemStime = new QTableWidgetItem(stime[i]);

       ui->historyTable->setItem(i,0,itemPname);
//       ui->historyTable->setItem(i,1,itemPath);
       ui->historyTable->setItem(i,1,itemArgu);
       ui->historyTable->setItem(i,2,itemStime);
   }
   ui->historyTable->resizeColumnsToContents();
}

void qjdTask::setTablePrivateData()
{
    fp2.setFileName(path[selectRowNum]);
    qDebug()<<fp2.fileName();
    if(!fp2.open(QFile::ReadOnly))
        qDebug()<<"open failure";
    else
        qDebug()<<"open success";

    /// 显示相关信息
     statement="";
     progress="";
     curProgress="";
     allProgress="";
     ltime="";
    while(!fp2.atEnd())
    {
        QString b=fp2.readLine(lineWidth);
        if(b.contains("Statement"))
        {
            statement=fp2.readLine(lineWidth);
            statement=statement.left(statement.indexOf("\n"));
        }
        if(b.contains("Progress"))
        {
            progress=fp2.readLine(lineWidth);
            progress=progress.left(progress.indexOf("\n"));
            allProgress=progress.right(progress.size()-progress.indexOf("/")-1);
            curProgress=progress.left(progress.indexOf("/"));
        }
        if(b.contains("Left Time"))
        {
            ltime=fp2.readLine(lineWidth);
            ltime=ltime.left(ltime.indexOf("\n"));
        }
    }
    fp2.close();

    ui->lblStat->setText(statement);
    ui->lblLtime->setText(ltime);
    ui->pgBar->setMaximum(allProgress.toInt());
    ui->pgBar->setValue(curProgress.toInt());

    // 显示选中作业的启动日期
//    qDebug()<<stime;
    QString dateStr;
    QString year;
    QString month;
    QString day;
    QDate date;

    //处理显示日期
    dateStr=stime[selectRowNum];
    dateStr=dateStr.left(dateStr.indexOf(" "));
    year=dateStr.left(dateStr.indexOf("-"));
    month=dateStr.right(dateStr.size()-dateStr.indexOf("-")-1);
    day=month.right(month.size()-month.indexOf("-")-1);
    month=month.left(month.indexOf("-"));
//    qDebug()<<month<<day;
    date.setDate(year.toInt(),month.toInt(),day.toInt());
    ui->calWidget->setSelectedDate(date);
}

void qjdTask::on_historyTable_clicked(QModelIndex index)
{
//    qDebug()<<index.row();
    selectRowNum=index.row();
    setTablePrivateData();
}
