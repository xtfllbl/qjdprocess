#include "qjdprocessmainwindow.h"
#include "ui_qjdprocessmainwindow.h"
#include <QStringListModel>
#include <QListView>
#include <QModelIndex>
#include <QDebug>
#include <QTest>
#include <QMessageBox>
#define refreshInterval 3000
#define swapInt(x,y,t)((t)=(x),(x)=(y),(y)=(t))
#define swapString(x,y,t)((t)=(x),(x)=(y),(y)=(t))

qjdProcessMainWindow::qjdProcessMainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::qjdProcessMainWindow)
{
    ui->setupUi(this);
    ui->tblMain->installEventFilter(this);
    ui->tblMain->resize(640,480);
    ui->tblMain->setFocusPolicy(Qt::StrongFocus);
    /// 自定义排序
    ui->tblMain->setSortingEnabled(false);       //自动排序关闭
    ui->tblMain->horizontalHeader()->setSortIndicatorShown(true);
    ui->tblMain->horizontalHeader()->setClickable(true);
    connect(ui->tblMain->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerHandle(int)));

    procview =new Procview();       //此地开始refresh
    proc=new Proc();

    options=new qjdoptions();
    options->handleCheck(); //设置一下显示区域

//    qjdtable=new qjdTable(this);
//    setTable(); //设置table

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(autoRefresh()));

    /// 自动刷新
    timer->start(refreshInterval);       //启动自动刷新

    selectRow=0;
    selectCol=0;
    processID=0;
    flagUse=false;
    flagFirstRun=false;
    flagSort=false;

    /// 设置排序
    flagSortPid=0;
    flagSortProcessName=0;
    flagSortState=0;
    flagSortNice=0;
    flagSortStartTime=0;
    flagSortWchan=0;
    flagSortCpu=0;
    flagSortMemory=0;
    flagSortPMemory=0;
    flagSortSleep=0;
    flagSortStack=0;
    flagSortIoRead=0;
    flagSortIoWrite=0;
    flagSortPCPU=0;
    flagSortWCPU=0;
    flagSortCMDLine=0;
    flagSortUid=0;
    flagSortUsrName=0;

    flagSortPidSave=0;
    flagSortProcessNameSave=0;
    flagSortStateSave=0;
    flagSortNiceSave=0;
    flagSortStartTimeSave=0;
    flagSortWchanSave=0;
    flagSortCpuSave=0;
    flagSortMemorySave=0;
    flagSortPMemorySave=0;
    flagSortSleepSave=0;
    flagSortStackSave=0;
    flagSortIoReadSave=0;
    flagSortIoWriteSave=0;
    flagSortPCPUSave=0;
    flagSortWCPUSave=0;
    flagSortCMDLineSave=0;
    flagSortUidSave=0;
    flagSortUsrNameSave=0;
    //设置右键弹出菜单
    menu=NULL;
    ui->tblMain->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tblMain, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));//this是datatable所在窗口

    //设置右键显示的内容,以及执行槽
    menu = new QMenu(ui->tblMain);
    actStop = menu->addAction("Stop");
    actCon = menu->addAction("Continue");
    actTer = menu->addAction("Terminate");
    actKill = menu->addAction("Kill");
    actHan = menu->addAction("Hang Up");
    connect(actTer, SIGNAL(triggered()), this, SLOT(terProcess()));
    connect(actKill, SIGNAL(triggered()), this, SLOT(killProcess()));
    connect(actHan, SIGNAL(triggered()), this, SLOT(hanProcess()));
    connect(actStop, SIGNAL(triggered()), this, SLOT(stopProcess()));
    connect(actCon, SIGNAL(triggered()), this, SLOT(conProcess()));

    connect(ui->comboProcess,SIGNAL(currentIndexChanged(int)),this,SLOT(autoRefresh()));
    setData();
}

qjdProcessMainWindow::~qjdProcessMainWindow()
{
    delete ui;
}

void qjdProcessMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void qjdProcessMainWindow::setData()
{    
    //确定表格的列数和行数
    int colNum=options->countCol;
    /// 警示可能的内存泄漏

    /// 已经delete, 仍然有0.3Mb的泄漏
    model = new QStandardItemModel(0, colNum, this);

    int countField=0;
    //确定列名, 排除泄漏
    if(options->pid==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("pid"));
        countField++;
    }
    if(options->cmd==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("process name"));
        countField++;
    }
    if(options->stat==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("state"));
        countField++;
    }
    if(options->nice==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("nice"));
        countField++;
    }
    if(options->startTime==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("starttime"));
        countField++;
    }
    if(options->wchan==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("wchan"));
        countField++;
    }
    if(options->whichCpu==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("cpu"));
        countField++;
    }
    if(options->mem==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("Memory"));
        countField++;
    }
    if(options->pmem==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("memory(%)"));
        countField++;
    }
    if(options->sleepAvg==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("sleep(%)"));
        countField++;
    }
    if(options->stackSize==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("stack(K)"));
        countField++;
    }
    if(options->ioRead==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("IO_READ"));
        countField++;
    }
    if(options->ioWrite==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("IO_WRITE"));
        countField++;
    }
    if(options->pcpu==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("% CPU"));
        countField++;
    }
    if(options->wcpu==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("% WCPU"));
        countField++;
    }
    if(options->cmdLine==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("COMMAND LINE"));
        countField++;
    }
    if(options->uid==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("UID"));
        countField++;
    }
    if(options->usrName==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("USERNAME"));
        countField++;
    }

    // 向表格中输入数据
    /// 在此筛选?
    int countRow=0;
    for(int i=0;i<procview->pidVector.size();i++)
    {
        aPid=QString::number(procview->pidVector.at(i));
        aCmd=procview->cmdVector.at(i);
        if(procview->statVector.at(i)=="S")
        {
            aStat="Sleep";
        }
        if(procview->statVector.at(i)=="R")
        {
            aStat="Running";
        }
        if(procview->statVector.at(i)=="Z")
        {
            aStat="Zombie";
        }
        if(procview->statVector.at(i)=="T")
        {
            aStat="Stoped";
        }
        if(procview->statVector.at(i)=="D")
        {
            aStat="Uninterruptible";
        }
        aNice=QString::number(procview->niceVector.at(i));
        aStartTime=procview->starttimeVector.at(i);
        aWchan=procview->wchanVector.at(i);
        aWhichCpu=QString::number(procview->whichcpuVector.at(i));
        aMem=procview->memVector.at(i);
        aPmem.sprintf("%f",procview->pmemVector.at(i));
        aSleepAvg=QString::number(procview->slpavgVector.at(i));
        aStack=QString::number(procview->stackVector.at(i));
        aIoread=procview->ioreadVector.at(i);
        aIowrite=procview->iowriteVector.at(i);
        aPcpu=QString::number(procview->pcpuVector.at(i));
        aWcpu.sprintf("%f",procview->wcpuVector.at(i));
        aCmdLine=procview->cmdlineVector.at(i);  //cmdlineVector
        aUid=QString::number(procview->uidVector.at(i));
        aUsrName=procview->usernameVector.at(i);

        /// 此处的不用delete, 只要delete model 这些自动回收  valgrind 判断此处问题多多
        itemPid=new QStandardItem(aPid);
        itemCmd=new QStandardItem(aCmd);
        itemStat=new QStandardItem(aStat);
        itemNice=new QStandardItem(aNice);
        itemStartTime=new QStandardItem(aStartTime);
        itemWchan=new QStandardItem(aWchan);
        itemWhichCpu=new QStandardItem(aWhichCpu);
        itemMem=new QStandardItem(aMem);
        itemPmem=new QStandardItem(aPmem);
        itemSleepAvg=new QStandardItem(aSleepAvg);
        itemStack=new QStandardItem(aStack);
        itemIoread=new QStandardItem(aIoread);
        itemIowrite=new QStandardItem(aIowrite);
        itemPcpu=new QStandardItem(aPcpu);
        itemWcpu=new QStandardItem(aWcpu);
        itemCmdLine=new QStandardItem(aCmdLine);
        itemUid=new QStandardItem(aUid);
        itemUsrName=new QStandardItem(aUsrName);

        /// 插入中间判断，符合要求，则插入数据
        if(ui->comboProcess->currentIndex()==0)
        {
            flagUse=true;       //所有情况全部符合
        }
        if(ui->comboProcess->currentIndex()==1)
        {
            if(procview->uidVector.at(i)>=500)
            {
                flagUse=true;
            }
            else
            {
                flagUse=false;
            }
        }
        if(ui->comboProcess->currentIndex()==2)
        {
            if(procview->statVector.at(i)=="R")
            {
                flagUse=true;
            }
            else
            {
                flagUse=false;
            }
        }

        if(flagUse==true)   //只有通过才添加这个进程
        {
            int countItem=0;
            if(processID==aPid.toInt())
            {
                selectRow=countRow;        //按照pid来定位选择的行
            }
            if(options->pid==true)
            {
                model->setItem(countRow,countItem,itemPid);
                countItem++;
            }
            if(options->cmd==true)
            {
                model->setItem(countRow,countItem,itemCmd);
                countItem++;
            }
            if(options->stat==true)
            {
                model->setItem(countRow,countItem,itemStat);
                countItem++;
            }
            if(options->nice==true)
            {
                model->setItem(countRow,countItem,itemNice);
                countItem++;
            }
            if(options->startTime==true)
            {
                model->setItem(countRow,countItem,itemStartTime);
                countItem++;
            }
            if(options->wchan==true)
            {
                model->setItem(countRow,countItem,itemWchan);
                countItem++;
            }
            if(options->whichCpu==true)
            {
                model->setItem(countRow,countItem,itemWhichCpu);
                countItem++;
            }
            if(options->mem==true)
            {
                model->setItem(countRow,countItem,itemMem);
                countItem++;
            }
            if(options->pmem==true)
            {
                model->setItem(countRow,countItem,itemPmem);
                countItem++;
            }
            if(options->sleepAvg==true)
            {
                model->setItem(countRow,countItem,itemSleepAvg);
                countItem++;
            }
            if(options->stackSize==true)
            {
                model->setItem(countRow,countItem,itemStack);
                countItem++;
            }
            if(options->ioRead==true)
            {
                model->setItem(countRow,countItem,itemIoread);
                countItem++;
            }
            if(options->ioWrite==true)
            {
                model->setItem(countRow,countItem,itemIowrite);
                countItem++;
            }
            if(options->pcpu==true)            {
                model->setItem(countRow,countItem,itemPcpu);
                countItem++;
            }
            if(options->wcpu==true)
            {
                model->setItem(countRow,countItem,itemWcpu);
                countItem++;
            }
            if(options->cmdLine==true)
            {
                model->setItem(countRow,countItem,itemCmdLine);
                countItem++;
            }
            if(options->uid==true)
            {
                model->setItem(countRow,countItem,itemUid);
                countItem++;
            }
            if(options->usrName==true)
            {
                model->setItem(countRow,countItem,itemUsrName);
                countItem++;
            }
            countRow++;
        }
    }
//    qDebug()<<countRow<<"setItemTime"<<model->rowCount();
    ui->tblMain->setModel(model);
    ui->tblMain->horizontalHeader()->setVisible(true);
    ui->tblMain->verticalHeader()->setVisible(false);     //不需要竖轴头

//    qjdtable->setModel(model);

    if(flagFirstRun==false)
    {
        ui->tblMain->resizeColumnsToContents();
//        qjdtable->resizeColumnsToContents();
    }
    flagFirstRun=true;
//    qDebug()<<"setData"<<selectRow;
    ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
//    qjdtable->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
}

void qjdProcessMainWindow::vectorClear()
{
    delete model;       //减少一半的内存泄露
    model=NULL;

    procview->pidVector.clear();            //pid
    procview->cmdVector.clear();        //进程名
    procview->statVector.clear();        // 状态
    procview->niceVector.clear();
    procview->starttimeVector.clear();
    procview->wchanVector.clear();       //与下文wchan_str相匹配
    procview->whichcpuVector.clear();
    procview->memVector.clear();       //内存使用量
    procview->pmemVector.clear();                    //内存使用百分比
    procview->slpavgVector.clear();       //睡眠百分比
    procview->stackVector.clear();       //栈空间
    procview->ioreadVector.clear();
    procview->iowriteVector.clear();
    procview->pcpuVector.clear();                    //cpu使用百分比
    procview->wcpuVector.clear();                    //cpu使用30s内百分比
    procview->cmdlineVector.clear();
    procview->uidVector.clear();
    procview->usernameVector.clear();

    procview->originIoreadVector.clear();
    procview->originIowriteVector.clear();
    procview->originMemVector.clear();
    procview->originStarttimeVector.clear();
}

void qjdProcessMainWindow::autoRefresh()
{
    machineRefresh=true;
    vectorClear();
    procview->refresh();        //refresh 就开始泄漏了
    if(flagSort==true)
    {
        headerSort();
        setSortData();
    }
    setData();
}

void qjdProcessMainWindow::on_actionExit_triggered()
{
    exit(0);
}



void qjdProcessMainWindow::on_actionChoose_Field_triggered()
{
    options->show();
}

void qjdProcessMainWindow::on_tblMain_pressed(QModelIndex index)
{
    index = ui->tblMain->indexAt(ui->tblMain->viewport()->mapFromGlobal(QCursor::pos()));
//    index = qjdtable->indexAt(qjdtable->viewport()->mapFromGlobal(QCursor::pos()));
    selectRow=index.row();
    selectCol=index.column();

    //获取所在行的pid
    processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
//    processID=qjdtable->model()->index(selectRow,0).data().toInt();
    qDebug()<<processID;
}

void qjdProcessMainWindow::showContextMenu(QPoint point)
{
    if(menu)
    {
        menu->exec(QCursor::pos());
    }
}

void qjdProcessMainWindow::killProcess()
{
    send_to_selected(SIGKILL);
}

void qjdProcessMainWindow::terProcess()
{
    send_to_selected(SIGTERM);
}

void qjdProcessMainWindow::hanProcess()
{
    send_to_selected(SIGHUP);
}
void qjdProcessMainWindow::stopProcess()
{
    send_to_selected(SIGSTOP);
}
void qjdProcessMainWindow::conProcess()
{
    send_to_selected(SIGCONT);
}
void qjdProcessMainWindow::send_to_selected(int sig)
{
    sendsig(processID, sig);
}

void qjdProcessMainWindow::sendsig(int pid, int sig)
{
    if(kill(pid, sig) < 0)
    {
        if(errno == EPERM)
        {
            QString s;
            s.sprintf("You do not have permission to send a signal to"
                      " process %d (", processID);
            s.append(").\n"
                     "Only the super-user and the owner of the process"
                     " may send signals to it.");
            QMessageBox::warning(this, "Permission denied", s);
        }
    }
}

void qjdProcessMainWindow::headerSort()
{
    if(colNameSave==colName)
    {
        addFlag=1;
    }
    else
    {
        addFlag=0;
        flagClear();
    }

    if(colName=="pid")
    {
        //qDebug()<<machineRefresh<<flagSortPidSave;
        if(machineRefresh==false)
        {
            if(flagSortPidSave==2)
            {
                flagSortPid=1;
            }
            else
            {
                flagSortPid=1+addFlag;
            }
        }
        flagSortPidSave=flagSortPid;
    }
    //qDebug()<<addFlag<<flagSortPidSave;

    if(colName=="process name")
    {
        if(machineRefresh==false)
        {
            if(flagSortProcessNameSave==2)
            {
                flagSortProcessName=1;
            }
            else
            {
                flagSortProcessName=1+addFlag;
            }
        }
        flagSortProcessNameSave=flagSortProcessName;
    }
    if(colName=="state")
    {
        if(machineRefresh==false)
        {
            if(flagSortStateSave==2)
            {
                flagSortState=1;
            }
            else
            {
                flagSortState=1+addFlag;
            }
        }
        flagSortStateSave=flagSortState;
    }
    if(colName=="nice")
    {
        if(machineRefresh==false)
        {
            if(flagSortNiceSave==2)
            {
                flagSortNice=1;
            }
            else
            {
                flagSortNice=1+addFlag;
            }
        }
        flagSortNiceSave=flagSortNice;
    }
    if(colName=="starttime")
    {
        if(machineRefresh==false)
        {
            if(flagSortStartTimeSave==2)
            {
                flagSortStartTime=1;
            }
            else
            {
                flagSortStartTime=1+addFlag;
            }
        }
        flagSortStartTimeSave=flagSortStartTime;
    }
    if(colName=="wchan")
    {
        if(machineRefresh==false)
        {
            if(flagSortWchanSave==2)
            {
                flagSortWchan=1;
            }
            else
            {
                flagSortWchan=1+addFlag;
            }
        }
        flagSortWchanSave=flagSortWchan;
    }
    if(colName=="cpu")
    {
        if(machineRefresh==false)
        {
            if(flagSortCpuSave==2)
            {
                flagSortCpu=1;
            }
            else
            {
                flagSortCpu=1+addFlag;
            }
        }
        flagSortCpuSave=flagSortCpu;
    }
    if(colName=="Memory")
    {
        if(machineRefresh==false)
        {
            if(flagSortMemorySave==2)
            {
                flagSortMemory=1;
            }
            else
            {
                flagSortMemory=1+addFlag;
            }
        }
        flagSortMemorySave=flagSortMemory;
    }
    if(colName=="memory(%)")
    {
        if(machineRefresh==false)
        {
            if(flagSortPMemorySave==2)
            {
                flagSortPMemory=1;
            }
            else
            {
                flagSortPMemory=1+addFlag;
            }
        }
        flagSortPMemorySave=flagSortPMemory;
    }
    if(colName=="sleep(%)")
    {
        if(machineRefresh==false)
        {
            if(flagSortSleepSave==2)
            {
                flagSortSleep=1;
            }
            else
            {
                flagSortSleep=1+addFlag;
            }
        }
        flagSortSleepSave=flagSortSleep;
    }
    if(colName=="stack(K)")
    {
        if(machineRefresh==false)
        {
            if(flagSortStackSave==2)
            {
                flagSortStack=1;
            }
            else
            {
                flagSortStack=1+addFlag;
            }
        }
        flagSortStackSave=flagSortStack;
    }
    if(colName=="IO_READ")
    {
        if(machineRefresh==false)
        {
            if(flagSortIoReadSave==2)
            {
                flagSortIoRead=1;
            }
            else
            {
                flagSortIoRead=1+addFlag;
            }
        }
        flagSortIoReadSave=flagSortIoRead;
    }
    if(colName=="IO_WRITE")
    {
        if(machineRefresh==false)
        {
            if(flagSortIoWriteSave==2)
            {
                flagSortIoWrite=1;
            }
            else
            {
                flagSortIoWrite=1+addFlag;
            }
        }
        flagSortIoWriteSave=flagSortIoWrite;
    }
    if(colName=="% CPU")
    {
        if(machineRefresh==false)
        {
            if(flagSortPCPUSave==2)
            {
                flagSortPCPU=1;
            }
            else
            {
                flagSortPCPU=1+addFlag;
            }
        }
        flagSortPCPUSave=flagSortPCPU;
    }
    if(colName=="% WCPU")
    {
        if(machineRefresh==false)
        {
            if(flagSortWCPUSave==2)
            {
                flagSortWCPU=1;
            }
            else
            {
                flagSortWCPU=1+addFlag;
            }
        }
        flagSortWCPUSave=flagSortWCPU;
    }
    if(colName=="COMMAND LINE")
    {
        if(machineRefresh==false)
        {
            if(flagSortCMDLineSave==2)
            {
                flagSortCMDLine=1;
            }
            else
            {
                flagSortCMDLine=1+addFlag;
            }
        }
        flagSortCMDLineSave=flagSortCMDLine;
    }
    if(colName=="UID")
    {
        if(machineRefresh==false)
        {
            if(flagSortUidSave==2)
            {
                flagSortUid=1;
            }
            else
            {
                flagSortUid=1+addFlag;
            }
        }
        flagSortUidSave=flagSortUid;
    }
    if(colName=="USERNAME")
    {
        if(machineRefresh==false)
        {
            if(flagSortUsrNameSave==2)
            {
                flagSortUsrName=1;
            }
            else
            {
                flagSortUsrName=1+addFlag;
            }
        }
        flagSortUsrNameSave=flagSortUsrName;
    }

    colNameSave=colName;
    flagSort=true;
}

void qjdProcessMainWindow::setSortData()
{
    int t;
    QString tempS;
    int temp;

    // pid 排序
    if(flagSortPid==1 || flagSortPid==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPid==1)
                {
                    if(procview->pidVector[j] < procview->pidVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPid==2)
                {
                    if(procview->pidVector[j] > procview->pidVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // Process Name 排序
    if(flagSortProcessName==1 || flagSortProcessName==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortProcessName==1)
                {
                    if(procview->cmdVector[j] < procview->cmdVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortProcessName==2)
                {
                    if(procview->cmdVector[j] > procview->cmdVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // state 排序
    if(flagSortState==1 || flagSortState==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortState==1)
                {
                    if(procview->statVector[j] < procview->statVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortState==2)
                {
                    if(procview->statVector[j] > procview->statVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // nice 排序
    if(flagSortNice==1 || flagSortNice==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortNice==1)
                {
                    if(procview->niceVector[j] < procview->niceVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortNice==2)
                {
                    if(procview->niceVector[j] > procview->niceVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // starttime 排序
    if(flagSortStartTime==1 || flagSortStartTime==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortStartTime==1)
                {
                    if(procview->originStarttimeVector[j] < procview->originStarttimeVector[j+1])
                    {
                        swapInt(procview->originStarttimeVector[j],procview->originStarttimeVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortStartTime==2)
                {
                    if(procview->originStarttimeVector[j] > procview->originStarttimeVector[j+1])
                    {
                        swapInt(procview->originStarttimeVector[j],procview->originStarttimeVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // wchan 排序
    if(flagSortWchan==1 || flagSortWchan==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortWchan==1)
                {
                    if(procview->wchanVector[j] < procview->wchanVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortWchan==2)
                {
                    if(procview->wchanVector[j] > procview->wchanVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // which cpu 排序
    if(flagSortCpu==1 || flagSortCpu==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortCpu==1)
                {
                    if(procview->whichcpuVector[j] < procview->whichcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortCpu==2)
                {
                    if(procview->whichcpuVector[j] > procview->whichcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // memory 排序
    if(flagSortMemory==1 || flagSortMemory==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortMemory==1)
                {
                    if(procview->originMemVector[j] < procview->originMemVector[j+1])
                    {
                        swapInt(procview->originMemVector[j],procview->originMemVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortMemory==2)
                {
                    if(procview->originMemVector[j] > procview->originMemVector[j+1])
                    {
                        swapInt(procview->originMemVector[j],procview->originMemVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // memory(%) 排序
    if(flagSortPMemory==1 || flagSortPMemory==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPMemory==1)
                {
                    if(procview->pmemVector[j] < procview->pmemVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPMemory==2)
                {
                    if(procview->pmemVector[j] > procview->pmemVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // sleep(%) 排序
    if(flagSortSleep==1 || flagSortSleep==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortSleep==1)
                {
                    if(procview->slpavgVector[j] < procview->slpavgVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortSleep==2)
                {
                    if(procview->slpavgVector[j] > procview->slpavgVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // stack 排序
    if(flagSortStack==1 || flagSortStack==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortStack==1)
                {
                    if(procview->stackVector[j] < procview->stackVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortStack==2)
                {
                    if(procview->stackVector[j] > procview->stackVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // IO_READ 排序
    if(flagSortIoRead==1 || flagSortIoRead==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortIoRead==1)
                {
                    if(procview->originIoreadVector[j] < procview->originIoreadVector[j+1])
                    {
                        swapInt(procview->originIoreadVector[j],procview->originIoreadVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortIoRead==2)
                {
                    if(procview->originIoreadVector[j] > procview->originIoreadVector[j+1])
                    {
                        swapInt(procview->originIoreadVector[j],procview->originIoreadVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // IO_WRITE 排序
    if(flagSortIoWrite==1 || flagSortIoWrite==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortIoWrite==1)
                {
                    if(procview->originIowriteVector[j] < procview->originIowriteVector[j+1])
                    {
                        swapInt(procview->originIowriteVector[j],procview->originIowriteVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortIoWrite==2)
                {
                    if(procview->originIowriteVector[j] > procview->originIowriteVector[j+1])
                    {
                        swapInt(procview->originIowriteVector[j],procview->originIowriteVector[j+1],temp);

                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // %CPU 排序
    if(flagSortPCPU==1 || flagSortPCPU==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPCPU==1)
                {
                    if(procview->pcpuVector[j] < procview->pcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPCPU==2)
                {
                    if(procview->pcpuVector[j] > procview->pcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // %WCPU 排序
    if(flagSortWCPU==1 || flagSortWCPU==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortWCPU==1)
                {
                    if(procview->wcpuVector[j] < procview->wcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortWCPU==2)
                {
                    if(procview->wcpuVector[j] > procview->wcpuVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // cmdline 排序
    if(flagSortCMDLine==1 || flagSortCMDLine==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortCMDLine==1)
                {
                    if(procview->cmdlineVector[j] < procview->cmdlineVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortCMDLine==2)
                {
                    if(procview->cmdlineVector[j] > procview->cmdlineVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // uid 排序
    if(flagSortUid==1 || flagSortUid==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortUid==1)
                {
                    if(procview->uidVector[j] < procview->uidVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortUid==2)
                {
                    if(procview->uidVector[j] > procview->uidVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // username 排序
    if(flagSortUsrName==1 || flagSortUsrName==2)
    {
        for(int i=0;i<procview->pidVector.size();i++)
        {
            t=procview->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortUsrName==1)
                {
                    if(procview->usernameVector[j] < procview->usernameVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortUsrName==2)
                {
                    if(procview->usernameVector[j] > procview->usernameVector[j+1])
                    {
                        swapInt(procview->pidVector[j],procview->pidVector[j+1],temp);
                        swapInt(procview->niceVector[j],procview->niceVector[j+1],temp);
                        swapInt(procview->whichcpuVector[j],procview->whichcpuVector[j+1],temp);
                        swapInt(procview->pmemVector[j],procview->pmemVector[j+1],temp);
                        swapInt(procview->slpavgVector[j],procview->slpavgVector[j+1],temp);
                        swapInt(procview->stackVector[j],procview->stackVector[j+1],temp);
                        swapInt(procview->pcpuVector[j],procview->pcpuVector[j+1],temp);
                        swapInt(procview->wcpuVector[j],procview->wcpuVector[j+1],temp);
                        swapInt(procview->uidVector[j],procview->uidVector[j+1],temp);
                        swapString(procview->cmdVector[j],procview->cmdVector[j+1],tempS);
                        swapString(procview->statVector[j],procview->statVector[j+1],tempS);
                        swapString(procview->starttimeVector[j],procview->starttimeVector[j+1],tempS);
                        swapString(procview->wchanVector[j],procview->wchanVector[j+1],tempS);
                        swapString(procview->memVector[j],procview->memVector[j+1],tempS);
                        swapString(procview->ioreadVector[j],procview->ioreadVector[j+1],tempS);
                        swapString(procview->iowriteVector[j],procview->iowriteVector[j+1],tempS);
                        swapString(procview->cmdlineVector[j],procview->cmdlineVector[j+1],tempS);
                        swapString(procview->usernameVector[j],procview->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

}

void qjdProcessMainWindow::flagClear()
{
    flagSortPid=0;
    flagSortProcessName=0;
    flagSortState=0;
    flagSortNice=0;
    flagSortStartTime=0;
    flagSortWchan=0;
    flagSortCpu=0;
    flagSortMemory=0;
    flagSortPMemory=0;
    flagSortSleep=0;
    flagSortStack=0;
    flagSortIoRead=0;
    flagSortIoWrite=0;
    flagSortPCPU=0;
    flagSortWCPU=0;
    flagSortCMDLine=0;
    flagSortUid=0;
    flagSortUsrName=0;
}

void qjdProcessMainWindow::headerHandle(int colNum)
{
    machineRefresh=false;
    colNumberSave=colNum;
    colName=ui->tblMain->model()->headerData(colNum,Qt::Horizontal).toString(); // 获得列名，用来排序
//    colName=qjdtable->model()->headerData(colNum,Qt::Horizontal).toString(); // 获得列名，用来排序
    vectorClear();
    procview->refresh();
    headerSort();
    setSortData();
    setData();
}

void qjdProcessMainWindow::on_actionManualRefresh_triggered()
{
    machineRefresh=true;
    vectorClear();
    procview->refresh();
    if(flagSort==true)
    {
        headerSort();
        setSortData();
    }
    setData();
}

void qjdProcessMainWindow::on_actionStopRefresh_triggered(bool checked)
{
    if(checked)
    {
        timer->stop();
    }
    if(checked==false)
    {
        timer->start(refreshInterval);
    }
}

bool qjdProcessMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug("Ate key press %d", keyEvent->key());
        keyPress(keyEvent);
        return true;
    }
//    else if (event->type() == QEvent::Wheel)
//    {
//        QWheelEvent *weve = static_cast<QWheelEvent *>(event);
//        qDebug("Ate wheel %d", weve->delta());
//        return true;
//    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void qjdProcessMainWindow::keyPress(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        selectRow=selectRow-1;
        if(selectRow<=0)
            selectRow=0;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_Down:
        selectRow=selectRow+1;
        if(selectRow>=model->rowCount())
            selectRow=model->rowCount()-1;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_PageUp:
        selectRow=selectRow-5;
        if(selectRow<=0)
            selectRow=0;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_PageDown:
        selectRow=selectRow+5;
        if(selectRow>=model->rowCount())
            selectRow=model->rowCount()-1;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;
    }
}

void qjdProcessMainWindow::setTable()
{
//    qjdtable->move(10,100);
//    qjdtable->resize(640,200);
//    qjdtable->setFocusPolicy(Qt::StrongFocus);
//    qjdtable->setSelectionBehavior(QAbstractItemView::SelectRows);
//    qjdtable->setSelectionMode(QAbstractItemView::SingleSelection);
//    qjdtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    qjdtable->setAlternatingRowColors(true);
//    qjdtable->verticalHeader()->setVisible(false);
//
//    QPalette palette;
//    QLinearGradient gradient(1, 0, 0.623398, 0.983);
//    gradient.setSpread(QGradient::PadSpread);
//    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
//    gradient.setColorAt(0, QColor(80, 170, 255, 255));
//    gradient.setColorAt(1, QColor(85, 85, 255, 255));
//    QBrush brush(gradient);
//    palette.setBrush(QPalette::Active, QPalette::Highlight, brush);
//    QBrush brush1(QColor(255, 255, 255, 255));
//    brush1.setStyle(Qt::SolidPattern);
//    palette.setBrush(QPalette::Active, QPalette::HighlightedText, brush1);
//    QBrush brush2(QColor(255, 255, 220, 255));
//    brush2.setStyle(Qt::SolidPattern);
//    palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush2);
//    QLinearGradient gradient1(1, 0, 0.623398, 0.983);
//    gradient1.setSpread(QGradient::PadSpread);
//    gradient1.setCoordinateMode(QGradient::ObjectBoundingMode);
//    gradient1.setColorAt(0, QColor(80, 170, 255, 255));
//    gradient1.setColorAt(1, QColor(85, 85, 255, 255));
//    QBrush brush3(gradient1);
//    palette.setBrush(QPalette::Inactive, QPalette::Highlight, brush3);
//    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, brush1);
//    palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush2);
//    QLinearGradient gradient2(1, 0, 0.623398, 0.983);
//    gradient2.setSpread(QGradient::PadSpread);
//    gradient2.setCoordinateMode(QGradient::ObjectBoundingMode);
//    gradient2.setColorAt(0, QColor(80, 170, 255, 255));
//    gradient2.setColorAt(1, QColor(85, 85, 255, 255));
//    QBrush brush4(gradient2);
//    palette.setBrush(QPalette::Disabled, QPalette::Highlight, brush4);
//    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, brush1);
//    palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush2);
//    qjdtable->setPalette(palette);
//
//    qjdtable->setStyleSheet(QString::fromUtf8("gridline-color: rgb(200, 200, 200);\n"
//"selection-color: rgb(255, 255, 255);\n"
//"font: 10pt \"\346\226\207\346\263\211\351\251\277\346\255\243\351\273\221\";\n"
//"selection-background-color: qlineargradient(spread:pad, x1:1, y1:0, x2:0.623398, y2:0.983, stop:0 rgba(80, 170, 255, 255), stop:1 rgba(85, 85, 255, 255));"));
//
//    /// 排序功能
//    qjdtable->setSortingEnabled(false);       //自动排序关闭
//    qjdtable->horizontalHeader()->setSortIndicatorShown(true);
//    qjdtable->horizontalHeader()->setClickable(true);
//    connect(qjdtable->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerHandle(int)));
//
//    /// 设置右键弹出菜单
//    menu=NULL;
//    qjdtable->setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(qjdtable, SIGNAL(customContextMenuRequested(const QPoint&)),
//            this, SLOT(showContextMenu(const QPoint&)));//this是datatable所在窗口
//
//    menu = new QMenu(qjdtable);
//    actStop = menu->addAction("Stop");
//    actCon = menu->addAction("Continue");
//    actTer = menu->addAction("Terminate");
//    actKill = menu->addAction("Kill");
//    actHan = menu->addAction("Hang Up");
//    connect(actTer, SIGNAL(triggered()), this, SLOT(terProcess()));
//    connect(actKill, SIGNAL(triggered()), this, SLOT(killProcess()));
//    connect(actHan, SIGNAL(triggered()), this, SLOT(hanProcess()));
//    connect(actStop, SIGNAL(triggered()), this, SLOT(stopProcess()));
//    connect(actCon, SIGNAL(triggered()), this, SLOT(conProcess()));
}
