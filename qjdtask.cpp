#include "qjdtask.h"
#include "ui_qjdtask.h"
#include <QDebug>
#include <QProgressBar>
#define lineWidth 256

qjdTask::qjdTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdTask)
{
    ui->setupUi(this);
    historyTableColNumber=0;
    historyTableRowNumber=0;
    setHistoryTableData();
    timerA=new QTimer(this);
    connect(timerA, SIGNAL(timeout()), this, SLOT(setActiveTableData()));

//    ui->historyTable->setSortingEnabled(true);  //暂时用自带的，现无法获取cell widget中的value

    ui->historyTable->setSortingEnabled(false);       //自动排序关闭
    ui->historyTable->horizontalHeader()->setSortIndicatorShown(true);
    ui->historyTable->horizontalHeader()->setClickable(true);
    connect(ui->historyTable->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerHandleH(int)));

    ui->activeTable->setSortingEnabled(true);  //暂时用自带的，现无法获取cell widget中的value
//    ui->activeTable->setSortingEnabled(false);       //自动排序关闭
//    ui->activeTable->horizontalHeader()->setSortIndicatorShown(true);
//    ui->activeTable->horizontalHeader()->setClickable(true);
//    connect(ui->activeTable->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerHandleA(int)));
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

void qjdTask::setHistoryTableData()
{
    qDebug()<<"setHistoryTableData();";
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
           QString d=fp.readLine(lineWidth);
           if(d.contains("Start Time"))
           {
               d=fp.readLine(lineWidth);
               stime<<d.left(d.indexOf("\n"));
           }
       }
   }
   fp.close();

   //设置行数，不设置会显示不出
   ui->historyTable->setRowCount(historyTableRowNumber);

//   qDebug()<<path.size();//大小正确
   for(int i=0;i<path.size();i++)
   {
       QTableWidgetItem *itemPname = new QTableWidgetItem(pname[i]);
       QTableWidgetItem *itemStime = new QTableWidgetItem(stime[i]);
       ui->historyTable->setItem(i,0,itemPname);
       ui->historyTable->setItem(i,1,itemStime);

       fp2.setFileName(path[i]);
       qDebug()<<fp2.fileName();
       if(!fp2.open(QFile::ReadOnly))
           qDebug()<<"open failure";
       else
           qDebug()<<"open success";

       fp2.seek(1620);      //跳过前面参数部分
       /// 需要点击才显示相关参数
//       QByteArray argu=fp2.read(1620);
//       ui->historyStatBrowser->setText(argu);
       /// 显示相关信息
        statement="";
        progress="";
        curProgress="";
        allProgress="";
        ltime="";
        endtime="";
       while(!fp2.atEnd())
       {
           QString b=fp2.readLine(lineWidth);
           if(b.contains("Current Time"))
           {
               endtime=fp2.readLine(lineWidth);
               endtime=endtime.left(endtime.indexOf("\n"));
           }

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
       }

       QTableWidgetItem *itemEndTime = new QTableWidgetItem(endtime);
       QProgressBar *itemPgbar=new QProgressBar();
       QTableWidgetItem *itemStatement=new QTableWidgetItem(statement);
       itemPgbar->setMaximum(allProgress.toInt());
       itemPgbar->setValue(curProgress.toInt());
       ui->historyTable->setItem(i,2,itemEndTime);
       ui->historyTable->setCellWidget(i,3,itemPgbar);
       ui->historyTable->setItem(i,4,itemStatement);

       fp2.close();
   }
   ui->historyTable->resizeColumnsToContents();
}

void qjdTask::setHistoryTablePrivateData()
{
    fHisArgu.setFileName(path[selectRowNum]);
    if(!fHisArgu.open(QFile::ReadOnly))
        qDebug()<<"open failure";
    else
        qDebug()<<"open success";
    /// 需要点击才显示相关参数
    QByteArray argu=fHisArgu.read(1620);
    ui->historyArguBrowser->setText(argu);

    fHisArgu.close();
}

void qjdTask::on_historyTable_clicked(QModelIndex index)
{
    selectRowNum=index.row();
    setHistoryTablePrivateData();
}

void qjdTask::on_activeTable_clicked(QModelIndex index)
{
    fActArgu.setFileName(path[hashActive.value(hashActive.keys().at(index.row()))]);
    if(!fActArgu.open(QFile::ReadOnly))
        qDebug()<<"open failure";
    else
        qDebug()<<"open success";
    /// 需要点击才显示相关参数
    QByteArray argu=fActArgu.read(1620);
    ui->activeArguBrowser->setText(argu);

    fActArgu.close();
}

void qjdTask::setActiveTableData()
{
    qDebug()<<"setActiveTableData";
    // 显示名称，开始时间，参数，剩余时间，进度
    QVector<QString> cmdKeys;
    QVector<QString> activeArguments;
    QVector<QString> activeStime;

    ui->activeTable->setRowCount(hashActive.keys().size());

    for(int i=0;i<hashActive.keys().size();i++)
    {
        cmdKeys.append(hashActive.keys().at(i));
        activeStime.append(stime[hashActive.value(cmdKeys[i])]);

        QTableWidgetItem *itemActivePname = new QTableWidgetItem(cmdKeys[i]);
        QTableWidgetItem *itemActiveStime = new QTableWidgetItem(activeStime[i]);
        ui->activeTable->setItem(i,0,itemActivePname);
        ui->activeTable->setItem(i,1,itemActiveStime);

        fActive.setFileName(path[hashActive.value(cmdKeys[i])]);
        qDebug()<<fActive.fileName();
        if(!fActive.open(QFile::ReadOnly))
            qDebug()<<"open failure";
        else
            qDebug()<<"open success";

        fActive.seek(1620); //跳过参数部分
        /// 显示相关信息
         statement="";
         progress="";
         curProgress="";
         allProgress="";
         ltime="";
        while(!fActive.atEnd())
        {
            QString b=fActive.readLine(lineWidth);
            if(b.contains("Statement"))
            {
                statement=fActive.readLine(lineWidth);
                statement=statement.left(statement.indexOf("\n"));
            }
            if(b.contains("Progress"))
            {
                progress=fActive.readLine(lineWidth);
                progress=progress.left(progress.indexOf("\n"));
                allProgress=progress.right(progress.size()-progress.indexOf("/")-1);
                curProgress=progress.left(progress.indexOf("/"));
            }
            if(b.contains("Left Time"))
            {
                ltime=fActive.readLine(lineWidth);
                ltime=ltime.left(ltime.indexOf("\n"));
            }
        }
        fActive.close();

        QTableWidgetItem *itemActiveStat = new QTableWidgetItem(statement);
        QTableWidgetItem *itemActiveLtime = new QTableWidgetItem(ltime);

        QProgressBar *itemActiveProgress=new QProgressBar();
        itemActiveProgress->setMaximum(allProgress.toInt());
        itemActiveProgress->setValue(curProgress.toInt());

        ui->activeTable->setItem(i,2,itemActiveStat);
        ui->activeTable->setCellWidget(i,3,itemActiveProgress);
        ui->activeTable->setItem(i,4,itemActiveLtime);
    }
    ui->activeTable->resizeColumnsToContents();
}

void qjdTask::on_tabWidget_selected(QString selected)
{
    // 不要尝试在判断中connect，和disconnect,会有逻辑错误
    if(selected=="Active Task")
    {
        timerA->start(3000);
    }
    else
    {
        timerA->stop();
    }
}

void qjdTask::closeEvent(QCloseEvent *)
{
    //备用
     if(fp.isOpen())
         fp.close();
     if(fp2.isOpen())
         fp2.close();
     if(fHisArgu.isOpen())
         fHisArgu.close();
     if(fActArgu.isOpen())
         fActArgu.close();
     if(fActive.isOpen())
         fActive.close();
}


void qjdTask::on_pushButton_clicked()
{
    this->close();
}

/// TODO: Get value of a widget in tablewidget
// mu zi ga xi
void qjdTask::headerHandleH(int colNum)
{
    qDebug()<<colNum;
    for(int i=0;i<ui->historyTable->rowCount();i++)
    {
//    qDebug()<<ui->historyTable->item(i,colNum)->text();  //不支持widget，支持item
        QWidget *a=ui->historyTable->cellWidget(i,colNum);  //始终获取不到value
        qDebug()<<a;
//        QProgressBar *b=a;        // debug 显示为 progressbar，但是无法转换成progressbar
    }
}

void qjdTask::headerHandleA(int colNum)
{
    qDebug()<<colNum;
}

void qjdTask::on_pushButton_2_clicked()
{
    qDebug()<<"mu qian shi fu yun";
    /// 代参数启动作业？不太好,能接的上只是日志而已
}

void qjdTask::on_btnRefresh_clicked()
{
    /// 刷新界面，比如新增加的作业，去除完成的不在运行作业
    // 其实就是更新All Table和hashActive

    // 更新all table
    historyTableRowNumber=0;
    pname.clear();
    path.clear();
    stime.clear();
    setHistoryTableData();
    ui->historyTable->resizeColumnsToContents();

    // 更新active table
    emit sigRefresh();      //发送信号，在mainwindow中接受并处理
    ui->activeTable->resizeColumnsToContents();
}
