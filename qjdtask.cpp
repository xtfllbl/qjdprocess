#include "qjdtask.h"
#include "ui_qjdtask.h"
#include <QDebug>
#include <QProgressBar>
#include <QProcess>
#include <QMessageBox>
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

///  读取公共文件
void qjdTask::setHistoryTableData()
{
    qDebug()<<"setHistoryTableData();";
   /// 会发现很多同名的作业，如何区分？
   // 由于是append，所以为读取到最后一个为准
   // TODO:文件变大之后非常缓慢，函数需要分离,遍历的事情只要做一边
    /// 这个设置公共文件的路径想办法弄一下
   fp.setFileName("/home/xtf/pathFile.xml");
   fp.open(QFile::ReadOnly);

   isPname=false;
   isPath=false;
   isStime=false;

   QXmlStreamReader stream(&fp);
   while (!stream.atEnd())
   {
       int a= stream.readNext();

       if(a==0)
       {
           qDebug()<<"The reader has not yet read anything.\n";
       }
       if(a==1)
       {
           qDebug()<<"An error has occurred, reported in error() and errorString().\n";
       }
       if(a==2)
       {
//           qDebug()<<stream.isStandaloneDocument();
       }
       if(a==3)
       {
//           qDebug()<<stream.isStandaloneDocument();
       }
       if(a==4)
       {
           /// 无法读取后面的链接
           QString name=stream.name().toString();
           if(name=="Process_Name")
           {
               isPname=true;
           }
           if(name=="Private_File_Path")
           {
               isPath=true;
           }
           if(name=="Start_Time")
           {
               isStime=true;
           }
       }
       if(a==5)
       {
       }
       if(a==6)
       {
           QString text=stream.text().toString();
           if(isPname==true)
           {
               pname<<text;
               isPname=false;
           }
           if(isPath==true)
           {
               path<<text;
               isPath=false;
           }
           if(isStime==true)
           {
               stime<<text;
               isStime=false;
           }
       }
       if(a==7)
       {
           qDebug()<<"The reader reports a comment in text().\n";
       }
       if(a==8)
       {
           qDebug()<<"The reader reports a DTD in text().\n";
       }
       if(a==9)
       {
           qDebug()<<"The reader reports an entity reference that could not be resolved. \n";
       }
       if(a==10)
       {
           qDebug()<<"The reader reports a processing instruction in processingInstructionTarget() and processingInstructionData().\n";
       }
   }
   if (stream.hasError())
   {
         qDebug()<<"do error handling";
   }
   fp.close();
   qDebug()<<pname<<path<<stime;

   /// ------------------------------------------------------------------------------------------------------------------- ///
   // 设置行数，不设置会显示不出
   historyTableRowNumber=path.size();
   ui->historyTable->setRowCount(historyTableRowNumber);

   // qDebug()<<path.size();  //大小正确
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

       /// 需要点击才显示相关参数
       /// 显示相关信息
        statement="";
        progress="";
        curProgress="";
        allProgress="";
        ltime="";
        endtime="";

        isCurrentTime=false;
        isStatement=false;
        isCurrentProgress=false;
        isWholeProgress=false;
//        isLeftTime=false;

        QXmlStreamReader stream(&fp2);
        while (!stream.atEnd())
        {
            int a= stream.readNext();

            if(a==0)
            {
                qDebug()<<"The reader has not yet read anything.\n";
            }
            if(a==1)
            {
                qDebug()<<"An error has occurred, reported in error() and errorString().\n";
            }
            if(a==2)
            {
//                qDebug()<<stream.isStandaloneDocument();
            }
            if(a==3)
            {
//                qDebug()<<stream.isStandaloneDocument();
            }
            if(a==4)
            {
                /// 无法读取后面的链接
                QString name=stream.name().toString();
                if(name=="Current_Time")
                {
                    isCurrentTime=true;
                }
                if(name=="Statement")
                {
                    isStatement=true;
                }
                if(name=="Current_Progress")
                {
                    isCurrentProgress=true;
                }
                if(name=="Whole_Progress")
                {
                    isWholeProgress=true;
                }
//                if(name=="Left_Time")
//                {
//                    isLeftTime=true;
//                }
            }
            if(a==5)
            {
            }
            if(a==6)
            {
                QString text=stream.text().toString();
                if(isCurrentTime==true)
                {
                    endtime=text;
                    isCurrentTime=false;
                }
                if(isStatement==true)
                {
                    statement=text;
                    isStatement=false;
                }
                if(isCurrentProgress==true)
                {
                    curProgress=text;
                    isCurrentProgress=false;
                }
                if(isWholeProgress==true)
                {
                    allProgress=text;
                    isWholeProgress=false;
                }
//                if(isLeftTime==true)
//                {
//                    endtime=text;
//                    isLeftTime=false;
//                }
            }
        }
        if (stream.hasError())
        {
              qDebug()<<"do error handling";
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
//    fActArgu.setFileName(path[hashActive.value(hashActive.keys().at(index.row()))]);
//    if(!fActArgu.open(QFile::ReadOnly))
//        qDebug()<<"open failure";
//    else
//        qDebug()<<"open success";
//    /// 需要点击才显示相关参数
//    /// 这个参数文件是不正确的，还需要另外加上一个
//    QByteArray argu=fActArgu.read(1620);
//    ui->activeArguBrowser->setText(argu);

//    fActArgu.close();
}

void qjdTask::setActiveTableData()
{
    qDebug()<<"setActiveTableData";
    // 显示名称，开始时间，参数，剩余时间，进度
    QVector<QString> cmdKeys;
//    QVector<QString> activeArguments;
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

        /// 显示相关信息
         statement="";
         progress="";
         curProgress="";
         allProgress="";
         ltime="";

         isCurrentTime=false;
         isStatement=false;
         isCurrentProgress=false;
         isWholeProgress=false;
         isLeftTime=false;

         QXmlStreamReader stream(&fp2);
         while (!stream.atEnd())
         {
             int a= stream.readNext();

             if(a==0)
             {
                 qDebug()<<"The reader has not yet read anything.\n";
             }
             if(a==1)
             {
                 qDebug()<<"An error has occurred, reported in error() and errorString().\n";
             }
             if(a==2)
             {
                 qDebug()<<stream.isStandaloneDocument();
             }
             if(a==3)
             {
                 qDebug()<<stream.isStandaloneDocument();
             }
             if(a==4)
             {
                 /// 无法读取后面的链接
                 qDebug()<<"a=4 stream.name"<<stream.name();      // 难办，开头项也是在这里被读出，不太好处理
                 QString name=stream.name().toString();
                 if(name=="Current_Time")
                 {
                     isCurrentTime=true;
                 }
                 if(name=="Statement")
                 {
                     isStatement=true;
                 }
                 if(name=="Current_Progress")
                 {
                     isCurrentProgress=true;
                 }
                 if(name=="Whole_Progress")
                 {
                     isWholeProgress=true;
                 }
                 if(name=="Left_Time")
                 {
                     isLeftTime=true;
                 }
             }
             if(a==5)
             {
                 qDebug()<<"a=5 stream.name"<<stream.name();
             }
             if(a==6)
             {
                 qDebug()<<"a=6 stream.text"<<stream.text();
                 QString text=stream.text().toString();
                 if(isCurrentTime==true)
                 {
                     endtime=text;
                     isCurrentTime=false;
                 }
                 if(isStatement==true)
                 {
                     statement=text;
                     isStatement=false;
                 }
                 if(isCurrentProgress==true)
                 {
                     curProgress=text;
                     isCurrentProgress=false;
                 }
                 if(isWholeProgress==true)
                 {
                     allProgress=text;
                     isWholeProgress=false;
                 }
                 if(isLeftTime==true)
                 {
                     ltime=text;
                     isLeftTime=false;
                 }
             }
         }
         if (stream.hasError())
         {
               qDebug()<<"do error handling";
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


void qjdTask::on_btnClose_clicked()
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

void qjdTask::on_btnStart_clicked()
{
    qDebug()<<"test start";
    /// 代参数启动作业？不太好,能接的上只是日志而已
    // 此处使用QProcess，能有效捕捉crash；自行发作业无法捕捉crash
    restartProgress();
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

/// TODO: 弹出界面，选择启动程序，参数，历史中断文件，参数文件等
/// 出错管理尝试使用原始try catch来实现
void qjdTask::restartProgress()
{
    QString program = "/home/xtf/Code/notification/notification";
    QStringList arguments;
    arguments << "-style" << "motif";

    QProcess *myProcess = new QProcess();
    connect(myProcess,SIGNAL(error(QProcess::ProcessError)),this,SLOT(handleError(QProcess::ProcessError)));
    myProcess->start(program,arguments);
    //    qDebug()<<myProcess->environment();
    // 返回5==unkown error
    //    qDebug()<<"error code:"<<myProcess->error()<<myProcess->errorString();      //kill==5 (5下的kill?并没返回kill)
    //    int errCode=myProcess->error();
    //    if(errCode==5)
    //        QMessageBox::warning(this,"Error Occured","Unkown Error");

}

void qjdTask::handleError(QProcess::ProcessError error)
{
    qDebug()<<"handle error";
}
