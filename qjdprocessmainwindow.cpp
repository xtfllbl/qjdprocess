#include "qjdprocessmainwindow.h"
#include "ui_qjdprocessmainwindow.h"
#include <QStringListModel>
#include <QListView>
#include <QModelIndex>
#include <QDebug>
#include <QMessageBox>

#include <QProgressBar>
#include <QProcess>
#include <QLineEdit>

#define refreshInterval 3000
#define swapInt(x,y,t)((t)=(x),(x)=(y),(y)=(t))
#define swapString(x,y,t)((t)=(x),(x)=(y),(y)=(t))

qjdProcessMainWindow::qjdProcessMainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::qjdProcessMainWindow)
{
    ui->setupUi(this);
    ui->tblMain->installEventFilter(this);
    ui->tblMain->setFocusPolicy(Qt::StrongFocus);
    ui->actionSystemProcess->setChecked(true);

    filterEdit=new qjdFilterLineEdit();     // 自有LineEdit
    ui->horizontalLayoutSys->insertWidget(0,filterEdit);
    connect(filterEdit,SIGNAL(returnPressed()),this,SLOT(filterProcess()));
    /// 自定义排序
    ui->tblMain->setSortingEnabled(false);       //自动排序关闭
    ui->tblMain->horizontalHeader()->setSortIndicatorShown(true);
    ui->tblMain->horizontalHeader()->setClickable(true);
    connect(ui->tblMain->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerHandle(int)));

    proc=new Proc();

    colNum=11;  //初始默认值

    cmd2=true;
    cmdLine2=false;
    ioRead2=false;
    ioWrite2=false;
    mem2=true;
    nice2=true;
    pcpu2=true;
    pid2=true;
    pmem2=false;
    sleepAvg2=false;
    stackSize2=true;
    startTime2=true;
    stat2=true;
    uid2=false;
    usrName2=true;
    wcpu2=true;
    wchan2=false;
    whichCpu2=true;

    hasOptions=false;

    willRemoveRecord=false;
    //    qjdtable=new qjdTable(this);
    //    setTable(); //设置table

    reportTimer = new QTimer(this);
    reportIsShow=false;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(autoRefresh()));

    /// 自动刷新
    timer->start(refreshInterval);       //启动自动刷新

    selectRow=0;
    selectCol=0;
    processID=0;
    selfID=0;
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
            this, SLOT(showContextMenuTblMain(const QPoint&)));

//    menu2=NULL;
//    ui->tableJob->setContextMenuPolicy(Qt::CustomContextMenu);
//    /// 同时要连接2个信号和2个槽
//    // 目前为止按找顺序执行，不知更换平台后会发生什么
//    connect(ui->tableJob, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(prepareToShowTableJob())); // must run 1st
//    connect(ui->tableJob, SIGNAL(cellClicked(int,int)),this, SLOT(showContextMenuTableJob(int,int))); // must run 2nd

    menuShowLog=NULL;
    ui->historyTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->historyTable, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(prepareToShowLog())); // must run 1st
    connect(ui->historyTable, SIGNAL(cellClicked(int,int)),this, SLOT(showContextMenuHistoryTable(int,int))); // must run 2nd

    menuActiveLogBrowser=NULL;
    ui->activeLogBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->activeLogBrowser, SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(showActiveLogMenu())); // must run 1st

    //设置右键显示的内容,以及执行槽
    menu = new QMenu(ui->tblMain); //设置为tblMain的菜单
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

//    menu2 = new QMenu(ui->tableJob); //设置为tableJob的菜单
//    actSaveAs = menu2->addAction("Save As...");
//    actClose = menu2->addAction("Close");
//    connect(actSaveAs, SIGNAL(triggered()), this, SLOT(saveLog()));
//    connect(actClose, SIGNAL(triggered()), this, SLOT(closeLog()));

    menuShowLog = new QMenu(ui->historyTable); //设置为tableJob的菜单
//    actShowLog = menuShowLog->addAction("Show Detail Log");
    actDelLog = menuShowLog->addAction("Delete this Record");
    connect(actDelLog, SIGNAL(triggered()), this, SLOT(deleteLog()));

    menuActiveLogBrowser=new QMenu(ui->activeLogBrowser);
    actRefreshLog=menuActiveLogBrowser->addAction("Refresh this log");
    connect(actRefreshLog, SIGNAL(triggered()), this, SLOT(refreshActiveLog()));


    /// 按要求显示进程名称
    connect(ui->comboProcess,SIGNAL(currentIndexChanged(int)),this,SLOT(autoRefresh()));

    proc->refresh();
    filterText="";
    setData();

    /// --------------------------------------------------------------------------------------------------------///

    importFileName="/home/xtf/pathFile.index"; // 默认索引文件位置

    ui->historyTable->setSortingEnabled(false);       //自动排序关闭
    ui->historyTable->horizontalHeader()->setSortIndicatorShown(true);
    ui->historyTable->horizontalHeader()->setClickable(true);
    ui->activeTable->setSortingEnabled(true);  //暂时用自带的，现无法获取cell widget中的value

    historyTableColNumber=0;
    historyTableRowNumber=0;
    setHistoryTableData();
    timerA=new QTimer();
    connect(timerA, SIGNAL(timeout()), this, SLOT(refreshTable()));
    setFirstActiveTableData();  /// 用来设置hash表的，既然移植过来了，看看可否去掉或者简化
    /// --------------------------------------------------------------------------------------------------------///

    hashLog.insert(0,1);
    hashLog.insert(1,2);

    /// 可以删除一个tab，用处很大
    /// 准备随时添加tab

    options=new qjdoptions();
    optionsIndex=ui->stackedWidgetPage->addWidget(options);
    connect(options,SIGNAL(sigCloseOption()),this,SLOT(closeOption()));
    //    options->setMouseTracking(true);

    if(cmd2==true)
    {
        options->ui->chkCMD->setChecked(true);
    }
    if(stat2==true)
    {
        options->ui->chkStat->setChecked(true);
    }
    if(nice2==true)
    {
        options->ui->chkNice->setChecked(true);
    }
    if(startTime2==true)
    {
        options->ui->chkStartTime->setChecked(true);
    }
    if(wchan2==true)
    {
        options->ui->chkWchan->setChecked(true);
    }
    if(whichCpu2==true)
    {
        options->ui->chkWhichCPU->setChecked(true);
    }
    if(mem2==true)
    {
        options->ui->chkMem->setChecked(true);
    }
    if(pmem2==true)
    {
        options->ui->chkPMem->setChecked(true);
    }
    if(sleepAvg2==true)
    {
        options->ui->chkSleepAvg->setChecked(true);
    }
    if(stackSize2==true)
    {
        options->ui->chkStackSize->setChecked(true);
    }
    if(ioRead2==true)
    {
        options->ui->chkIOREAD->setChecked(true);
    }
    if(ioWrite2==true)
    {
        options->ui->chkIOWRITE->setChecked(true);
    }
    if(pcpu2==true)
    {
        options->ui->chkPCPU->setChecked(true);
    }
    if(wcpu2==true)
    {
        options->ui->chkWCPU->setChecked(true);
    }
    if(cmdLine2==true)
    {
        options->ui->chkCMDline->setChecked(true);
    }
    if(uid2==true)
    {
        options->ui->chkUID->setChecked(true);
    }
    if(usrName2==true)
    {
        options->ui->chkUsrName->setChecked(true);
    }

    ui->tableSys->setCurrentCell(0,0);
    ui->stackedWidgetTable->setCurrentIndex(0);
    ui->stackedWidgetPage->setCurrentIndex(0);
    ui->actionSystem_Monitor->setChecked(true);

    countRestartTimes=0;

    qDebug()<<"gou zao wan cheng";
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
    if(hasOptions==true)
    {
        hasOptions=false;
        colNum=options->countCol;

        cmd2=options->cmd;
        cmdLine2=options->cmdLine;
        ioRead2=options->ioRead;
        ioWrite2=options->ioWrite;
        mem2=options->mem;
        nice2=options->nice;
        pcpu2=options->pcpu;
        pid2=options->pid;
        pmem2=options->pmem;
        sleepAvg2=options->sleepAvg;
        stackSize2=options->stackSize;
        startTime2=options->startTime;
        stat2=options->stat;
        uid2=options->uid;
        usrName2=options->usrName;
        wcpu2=options->wcpu;
        wchan2=options->wchan;
        whichCpu2=options->whichCpu;
    }
    model = new QStandardItemModel(0, colNum, this);

    int countField=0;
    //确定列名, 排除泄漏
    if(pid2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("pid"));
        countField++;
    }
    if(cmd2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("process name"));
        countField++;
    }
    if(stat2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("state"));
        countField++;
    }
    if(nice2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("nice"));
        countField++;
    }
    if(startTime2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("starttime"));
        countField++;
    }
    if(wchan2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("wchan"));
        countField++;
    }
    if(whichCpu2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("cpu"));
        countField++;
    }
    if(mem2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("Memory"));
        countField++;
    }
    if(pmem2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("memory(%)"));
        countField++;
    }
    if(sleepAvg2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("sleep(%)"));
        countField++;
    }
    if(stackSize2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("stack(K)"));
        countField++;
    }
    if(ioRead2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("IO_READ"));
        countField++;
    }
    if(ioWrite2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("IO_WRITE"));
        countField++;
    }
    if(pcpu2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("% CPU"));
        countField++;
    }
    if(wcpu2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("% WCPU"));
        countField++;
    }
    if(cmdLine2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("COMMAND LINE"));
        countField++;
    }
    if(uid2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("UID"));
        countField++;
    }
    if(usrName2==true)
    {
        model->setHeaderData(countField, Qt::Horizontal, tr("USERNAME"));
        countField++;
    }

    // 向表格中输入数据
    /// 在此筛选,重要循环
    int countRow=0;
    for(int i=0;i<proc->pidVector.size();i++)
    {
        aPid=QString::number(proc->pidVector.at(i),10);
        aCmd=proc->cmdVector.at(i);     // 通过cmd对过滤器作出反应
        if(proc->statVector.at(i)=="S")
        {
            aStat="Sleep";
        }
        if(proc->statVector.at(i)=="R")
        {
            aStat="Running";
        }
        if(proc->statVector.at(i)=="Z")
        {
            aStat="Zombie";
        }
        if(proc->statVector.at(i)=="T")
        {
            aStat="Stoped";
        }
        if(proc->statVector.at(i)=="D")
        {
            aStat="Uninterruptible";
        }
        aNice=QString::number(proc->niceVector.at(i),10);
        aStartTime=proc->starttimeVector.at(i);
        aWchan=proc->wchanVector.at(i);
        aWhichCpu=QString::number(proc->whichcpuVector.at(i),10);
        aMem=proc->memVector.at(i);
        aPmem.sprintf("%f",proc->pmemVector.at(i));
        aSleepAvg=QString::number(proc->slpavgVector.at(i),10);
        aStack=QString::number(proc->stackVector.at(i),10);
        aIoread=proc->ioreadVector.at(i);
        aIowrite=proc->iowriteVector.at(i);
        aPcpu=QString::number(proc->pcpuVector.at(i),10);
        aWcpu.sprintf("%f",proc->wcpuVector.at(i));
        aCmdLine=proc->cmdlineVector.at(i);
        aUid=QString::number(proc->uidVector.at(i),10);
        aUsrName=proc->usernameVector.at(i);

        // valgrind 判断此处问题多多，真的会回收么,有一小部分泄漏[已解决]
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
            /// 加入过滤器
            if(filterText!="")
            {
                if(itemCmd->text().contains(filterText,Qt::CaseInsensitive))
                {
                    flagUse=true;
                }
                else
                {
                    flagUse=false;
                }
            }
        }
        if(ui->comboProcess->currentIndex()==1)
        {
            if(proc->uidVector.at(i)>=500)
            {
                flagUse=true;
                /// 加入过滤器
                if(filterText!="")
                {
                    if(itemCmd->text().contains(filterText,Qt::CaseInsensitive))
                    {
                        flagUse=true;
                    }
                    else
                    {
                        flagUse=false;
                    }
                }
            }
            else
            {
                flagUse=false;
            }
        }
        if(ui->comboProcess->currentIndex()==2)
        {
            if(proc->statVector.at(i)=="R")
            {
                flagUse=true;
                /// 加入过滤器
                if(filterText!="")
                {
                    if(itemCmd->text().contains(filterText,Qt::CaseInsensitive))
                    {
                        flagUse=true;
                    }
                    else
                    {
                        flagUse=false;
                    }
                }
            }
            else
            {
                flagUse=false;
            }
        }

        if(flagUse==false)
        {
            delete itemPid;
            delete itemCmd;
            delete itemStat;
            delete itemNice;
            delete itemStartTime;
            delete itemWchan;
            delete itemWhichCpu;
            delete itemMem;
            delete itemPmem;
            delete itemSleepAvg;
            delete itemStack;
            delete itemIoread;
            delete itemIowrite;
            delete itemPcpu;
            delete itemWcpu;
            delete itemCmdLine;
            delete itemUid;
            delete itemUsrName;
        }
        if(flagUse==true)   //只有通过才添加这个进程
        {
            int countItem=0;
            if(processID==aPid.toInt())
            {
                selectRow=countRow;        //按照pid来定位选择的行
            }

            if(pid2==true)
            {
                model->setItem(countRow,countItem,itemPid);
                countItem++;
            }
            if(cmd2==true)
            {
                model->setItem(countRow,countItem,itemCmd);
                countItem++;
            }
            if(stat2==true)
            {
                model->setItem(countRow,countItem,itemStat);
                countItem++;
            }
            if(nice2==true)
            {
                model->setItem(countRow,countItem,itemNice);
                countItem++;
            }
            if(startTime2==true)
            {
                model->setItem(countRow,countItem,itemStartTime);
                countItem++;
            }
            if(wchan2==true)
            {
                model->setItem(countRow,countItem,itemWchan);
                countItem++;
            }
            if(whichCpu2==true)
            {
                model->setItem(countRow,countItem,itemWhichCpu);
                countItem++;
            }
            if(mem2==true)
            {
                model->setItem(countRow,countItem,itemMem);
                countItem++;
            }
            if(pmem2==true)
            {
                model->setItem(countRow,countItem,itemPmem);
                countItem++;
            }
            if(sleepAvg2==true)
            {
                model->setItem(countRow,countItem,itemSleepAvg);
                countItem++;
            }
            if(stackSize2==true)
            {
                model->setItem(countRow,countItem,itemStack);
                countItem++;
            }
            if(ioRead2==true)
            {
                model->setItem(countRow,countItem,itemIoread);
                countItem++;
            }
            if(ioWrite2==true)
            {
                model->setItem(countRow,countItem,itemIowrite);
                countItem++;
            }
            if(pcpu2==true)
            {
                model->setItem(countRow,countItem,itemPcpu);
                countItem++;
            }
            if(wcpu2==true)
            {
                model->setItem(countRow,countItem,itemWcpu);
                countItem++;
            }
            if(cmdLine2==true)
            {
                model->setItem(countRow,countItem,itemCmdLine);
                countItem++;
            }
            if(uid2==true)
            {
                model->setItem(countRow,countItem,itemUid);
                countItem++;
            }
            if(usrName2==true)
            {
                model->setItem(countRow,countItem,itemUsrName);
                countItem++;
            }

            /// 手动删除没有用到的item
            if(pid2==false)
            {
                delete itemPid;
            }
            if(cmd2==false)
            {
                delete itemCmd;
            }
            if(stat2==false)
            {
                delete itemStat;
            }
            if(nice2==false)
            {
                delete itemNice;
            }
            if(startTime2==false)
            {
                delete itemStartTime;
            }
            if(wchan2==false)
            {
                delete itemWchan;
            }
            if(whichCpu2==false)
            {
                delete itemWhichCpu;
            }
            if(mem2==false)
            {
                delete itemMem;
            }
            if(pmem2==false)
            {
                delete itemPmem;
            }
            if(sleepAvg2==false)
            {
                delete itemSleepAvg;
            }
            if(stackSize2==false)
            {
                delete itemStack;
            }
            if(ioRead2==false)
            {
                delete itemIoread;
            }
            if(ioWrite2==false)
            {
                delete itemIowrite;
            }
            if(pcpu2==false)
            {
                delete itemPcpu;
            }
            if(wcpu2==false)
            {
                delete itemWcpu;
            }
            if(cmdLine2==false)
            {
                delete itemCmdLine;
            }
            if(uid2==false)
            {
                delete itemUid;
            }
            if(usrName2==false)
            {
                delete itemUsrName;
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
    delete model;       //减少一大半的内存泄露

    proc->pidVector.clear();            //pid
    proc->cmdVector.clear();        //进程名
    proc->statVector.clear();        // 状态
    proc->niceVector.clear();
    proc->starttimeVector.clear();
    proc->wchanVector.clear();       //与下文wchan_str相匹配
    proc->whichcpuVector.clear();
    proc->memVector.clear();       //内存使用量
    proc->pmemVector.clear();                    //内存使用百分比
    proc->slpavgVector.clear();       //睡眠百分比
    proc->stackVector.clear();       //栈空间
    proc->ioreadVector.clear();
    proc->iowriteVector.clear();
    proc->pcpuVector.clear();                    //cpu使用百分比
    proc->wcpuVector.clear();                    //cpu使用30s内百分比
    proc->cmdlineVector.clear();
    proc->uidVector.clear();
    proc->usernameVector.clear();

    proc->originIoreadVector.clear();
    proc->originIowriteVector.clear();
    proc->originMemVector.clear();
    proc->originStarttimeVector.clear();
}

void qjdProcessMainWindow::autoRefresh()
{
    machineRefresh=true;
    vectorClear();
    proc->refresh();        //refresh 就开始泄漏了
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


// 处理选择的项目
void qjdProcessMainWindow::handleChooseField()
{
    options->handleCheck();
    hasOptions=true;
    ui->tblMain->resizeColumnsToContents();
}

void qjdProcessMainWindow::on_tblMain_pressed(QModelIndex index)
{
    index = ui->tblMain->indexAt(ui->tblMain->viewport()->mapFromGlobal(QCursor::pos()));
    selectRow=index.row();
    selectCol=index.column();

    //获取所在行的pid
    processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
    processName=ui->tblMain->model()->index(selectRow,1).data().toString();
    if(processName=="qjdprocess")
        selfID=processID;
    else selfID=0;
}

void qjdProcessMainWindow::showContextMenuTblMain(QPoint )
{
    if(menu)
    {
        menu->exec(QCursor::pos());
    }
}

//void qjdProcessMainWindow::prepareToShowTableJob()
//{
//    //    qDebug()<<"IN 1";
//    rightClick=true;
//}

//void qjdProcessMainWindow::showContextMenuTableJob(int a,int b)
//{
    //    qDebug()<<"IN 2";
    //    savedRow=a;
//    if(menu2 && a>2 && rightClick==true)
//    {
//        menu2->exec(QCursor::pos());
//    }
//    rightClick=false;
//}

void qjdProcessMainWindow::prepareToShowLog()
{
    //    qDebug()<<"IN 1";
    rightClick2=true;
}

void qjdProcessMainWindow::showContextMenuHistoryTable(int a,int b)
{
    //    qDebug()<<"IN 2";
    //    savedRow=a;
    if(rightClick2==true)
    {
        menuShowLog->exec(QCursor::pos());
    }
    rightClick2=false;
}

void qjdProcessMainWindow::showActiveLogMenu()
{
    if(menuActiveLogBrowser)
    {
        menuActiveLogBrowser->exec(QCursor::pos());
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
    if(processID==selfID)
        QMessageBox::information(this,"Attention","Don`t even try it");
    else
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
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPid==1)
                {
                    if(proc->pidVector[j] < proc->pidVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPid==2)
                {
                    if(proc->pidVector[j] > proc->pidVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // Process Name 排序
    if(flagSortProcessName==1 || flagSortProcessName==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortProcessName==1)
                {
                    if(proc->cmdVector[j] < proc->cmdVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortProcessName==2)
                {
                    if(proc->cmdVector[j] > proc->cmdVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // state 排序
    if(flagSortState==1 || flagSortState==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortState==1)
                {
                    if(proc->statVector[j] < proc->statVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortState==2)
                {
                    if(proc->statVector[j] > proc->statVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // nice 排序
    if(flagSortNice==1 || flagSortNice==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortNice==1)
                {
                    if(proc->niceVector[j] < proc->niceVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortNice==2)
                {
                    if(proc->niceVector[j] > proc->niceVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // starttime 排序
    if(flagSortStartTime==1 || flagSortStartTime==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortStartTime==1)
                {
                    if(proc->originStarttimeVector[j] < proc->originStarttimeVector[j+1])
                    {
                        swapInt(proc->originStarttimeVector[j],proc->originStarttimeVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortStartTime==2)
                {
                    if(proc->originStarttimeVector[j] > proc->originStarttimeVector[j+1])
                    {
                        swapInt(proc->originStarttimeVector[j],proc->originStarttimeVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // wchan 排序
    if(flagSortWchan==1 || flagSortWchan==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortWchan==1)
                {
                    if(proc->wchanVector[j] < proc->wchanVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortWchan==2)
                {
                    if(proc->wchanVector[j] > proc->wchanVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // which cpu 排序
    if(flagSortCpu==1 || flagSortCpu==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortCpu==1)
                {
                    if(proc->whichcpuVector[j] < proc->whichcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortCpu==2)
                {
                    if(proc->whichcpuVector[j] > proc->whichcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // memory 排序
    if(flagSortMemory==1 || flagSortMemory==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortMemory==1)
                {
                    if(proc->originMemVector[j] < proc->originMemVector[j+1])
                    {
                        swapInt(proc->originMemVector[j],proc->originMemVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortMemory==2)
                {
                    if(proc->originMemVector[j] > proc->originMemVector[j+1])
                    {
                        swapInt(proc->originMemVector[j],proc->originMemVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // memory(%) 排序
    if(flagSortPMemory==1 || flagSortPMemory==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPMemory==1)
                {
                    if(proc->pmemVector[j] < proc->pmemVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPMemory==2)
                {
                    if(proc->pmemVector[j] > proc->pmemVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // sleep(%) 排序
    if(flagSortSleep==1 || flagSortSleep==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortSleep==1)
                {
                    if(proc->slpavgVector[j] < proc->slpavgVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortSleep==2)
                {
                    if(proc->slpavgVector[j] > proc->slpavgVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // stack 排序
    if(flagSortStack==1 || flagSortStack==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortStack==1)
                {
                    if(proc->stackVector[j] < proc->stackVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortStack==2)
                {
                    if(proc->stackVector[j] > proc->stackVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // IO_READ 排序
    if(flagSortIoRead==1 || flagSortIoRead==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortIoRead==1)
                {
                    if(proc->originIoreadVector[j] < proc->originIoreadVector[j+1])
                    {
                        swapInt(proc->originIoreadVector[j],proc->originIoreadVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortIoRead==2)
                {
                    if(proc->originIoreadVector[j] > proc->originIoreadVector[j+1])
                    {
                        swapInt(proc->originIoreadVector[j],proc->originIoreadVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // IO_WRITE 排序
    if(flagSortIoWrite==1 || flagSortIoWrite==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortIoWrite==1)
                {
                    if(proc->originIowriteVector[j] < proc->originIowriteVector[j+1])
                    {
                        swapInt(proc->originIowriteVector[j],proc->originIowriteVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortIoWrite==2)
                {
                    if(proc->originIowriteVector[j] > proc->originIowriteVector[j+1])
                    {
                        swapInt(proc->originIowriteVector[j],proc->originIowriteVector[j+1],temp);

                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // %CPU 排序
    if(flagSortPCPU==1 || flagSortPCPU==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortPCPU==1)
                {
                    if(proc->pcpuVector[j] < proc->pcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortPCPU==2)
                {
                    if(proc->pcpuVector[j] > proc->pcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // %WCPU 排序
    if(flagSortWCPU==1 || flagSortWCPU==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortWCPU==1)
                {
                    if(proc->wcpuVector[j] < proc->wcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortWCPU==2)
                {
                    if(proc->wcpuVector[j] > proc->wcpuVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // cmdline 排序
    if(flagSortCMDLine==1 || flagSortCMDLine==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortCMDLine==1)
                {
                    if(proc->cmdlineVector[j] < proc->cmdlineVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortCMDLine==2)
                {
                    if(proc->cmdlineVector[j] > proc->cmdlineVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // uid 排序
    if(flagSortUid==1 || flagSortUid==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortUid==1)
                {
                    if(proc->uidVector[j] < proc->uidVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortUid==2)
                {
                    if(proc->uidVector[j] > proc->uidVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
            }
        }
    }

    // username 排序
    if(flagSortUsrName==1 || flagSortUsrName==2)
    {
        for(int i=0;i<proc->pidVector.size();i++)
        {
            t=proc->pidVector.size()-i-1;
            for(int j=0;j<t;j++)
            {
                if(flagSortUsrName==1)
                {
                    if(proc->usernameVector[j] < proc->usernameVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
                    }
                }
                if(flagSortUsrName==2)
                {
                    if(proc->usernameVector[j] > proc->usernameVector[j+1])
                    {
                        swapInt(proc->pidVector[j],proc->pidVector[j+1],temp);
                        swapInt(proc->niceVector[j],proc->niceVector[j+1],temp);
                        swapInt(proc->whichcpuVector[j],proc->whichcpuVector[j+1],temp);
                        swapInt(proc->pmemVector[j],proc->pmemVector[j+1],temp);
                        swapInt(proc->slpavgVector[j],proc->slpavgVector[j+1],temp);
                        swapInt(proc->stackVector[j],proc->stackVector[j+1],temp);
                        swapInt(proc->pcpuVector[j],proc->pcpuVector[j+1],temp);
                        swapInt(proc->wcpuVector[j],proc->wcpuVector[j+1],temp);
                        swapInt(proc->uidVector[j],proc->uidVector[j+1],temp);
                        swapString(proc->cmdVector[j],proc->cmdVector[j+1],tempS);
                        swapString(proc->statVector[j],proc->statVector[j+1],tempS);
                        swapString(proc->starttimeVector[j],proc->starttimeVector[j+1],tempS);
                        swapString(proc->wchanVector[j],proc->wchanVector[j+1],tempS);
                        swapString(proc->memVector[j],proc->memVector[j+1],tempS);
                        swapString(proc->ioreadVector[j],proc->ioreadVector[j+1],tempS);
                        swapString(proc->iowriteVector[j],proc->iowriteVector[j+1],tempS);
                        swapString(proc->cmdlineVector[j],proc->cmdlineVector[j+1],tempS);
                        swapString(proc->usernameVector[j],proc->usernameVector[j+1],tempS);
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
    vectorClear();
    proc->refresh();
    headerSort();
    setSortData();
    setData();
}

bool qjdProcessMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        //        qDebug("Ate key press %d", keyEvent->key());
        keyPress(keyEvent);
        return true;
    }
    /// 如果需要鼠标滚轮自定义，要重新实现tableview，然后才能监听事件
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
        processName=ui->tblMain->model()->index(selectRow,1).data().toString();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_Down:
        selectRow=selectRow+1;
        if(selectRow>=model->rowCount())
            selectRow=model->rowCount()-1;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        processName=ui->tblMain->model()->index(selectRow,1).data().toString();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_PageUp:
        selectRow=selectRow-5;
        if(selectRow<=0)
            selectRow=0;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        processName=ui->tblMain->model()->index(selectRow,1).data().toString();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;

    case Qt::Key_PageDown:
        selectRow=selectRow+5;
        if(selectRow>=model->rowCount())
            selectRow=model->rowCount()-1;
        processID=ui->tblMain->model()->index(selectRow,0).data().toInt();
        processName=ui->tblMain->model()->index(selectRow,1).data().toString();
        ui->tblMain->setCurrentIndex(ui->tblMain->model()->index(selectRow,0));
        break;
    }
}

//  shit 这里也要用到这个文件。。。 通用性太差
void qjdProcessMainWindow::setFirstActiveTableData()
{
    qDebug()<<"setFirstActiveTableData";
    QVector<QString> pname;
    bool isPname=false;
    fp.setFileName("/home/xtf/pathFile.index");
    fp.open(QFile::ReadOnly);

    QXmlStreamReader stream(&fp);
    while (!stream.atEnd())
    {
        int a= stream.readNext();

        if(a==4)
        {
            QString name=stream.name().toString();
            if(name=="Process_Name")
            {
                isPname=true;
            }
        }
        if(a==6)
        {
            QString text=stream.text().toString();
            if(isPname==true)
            {
                pname<<text;
                isPname=false;
            }
        }
    }
    if (stream.hasError())
    {
        qDebug()<<"do error handling";
    }
    fp.close();

    QVector<QString> tempCmd;
    QVector<int> tempPname;
    for(int i=0;i<proc->cmdVector.size();i++)
    {
        for(int j=0;j<pname.size();j++)
        {
            if(proc->cmdVector.at(i).contains(pname[j]))
            {
                // 返回及时作业中和历史记录中同名作业,会返回很多，而且可能不同名字
                tempCmd<<proc->cmdVector.at(i);
                tempPname<<j;
            }
        }
    }

    // 使用hash，确保进程对应最新的日志
    hashActive.clear();
//    ui->activeArguBrowser->clear();
    for(int i=0;i<tempCmd.size();i++)
    {
        hashActive[tempCmd[i]]=tempPname[i];
    }
    /// 设置完毕后才调用设置表格数据
    setActiveTableData();
}

///  读取公共文件
void qjdProcessMainWindow::setHistoryTableData()
{
    qDebug()<<"setHistoryTableData();";
    /// 会发现很多同名的作业，如何区分？
    // 由于是append，所以为读取到最后一个为准
    /// TODO:文件变大之后非常缓慢，函数需要分离,遍历的事情只要做一边
    /// 这个设置公共文件的路径想办法弄一下
    pubFile.setFileName(importFileName);
    pubFile.open(QFile::ReadOnly);

    isPnameJob=false;
    isPriPathJob=false;
    isArgPathJob=false;
    isLogPathJob=false;
    isStimeJob=false;
    isRecordJob=false;
    tempRecordJob.clear();
    pnameJob.clear();
    priPathJob.clear();
    argPathJob.clear();
    logPathJob.clear();
    stimeJob.clear();

    QXmlStreamReader stream(&pubFile);
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
        if(a==4)
        {
            /// 无法读取后面的链接
            QString name=stream.name().toString();
            if(name=="Record")
            {
                isRecordJob=true;
            }
            if(name=="Process_Name")
            {
                isPnameJob=true;
            }
            if(name=="Private_File_Path")
            {
                isPriPathJob=true;
            }
            if(name=="Argument_File_Path")
            {
                isArgPathJob=true;
            }
            if(name=="Log_File_Path")
            {
                isLogPathJob=true;
            }
            if(name=="Start_Time")
            {
                isStimeJob=true;
            }
        }
        if(a==5)
        {
        }
        if(a==6)
        {
            QString text=stream.text().toString();
            if(isRecordJob==true)
            {
                tempRecordJob<<text;
                isRecordJob=false;
            }
            if(isPnameJob==true)
            {
                pnameJob<<text;
                isPnameJob=false;
            }
            if(isPriPathJob==true)
            {
                priPathJob<<text;
                isPriPathJob=false;
            }
            if(isArgPathJob==true)
            {
                argPathJob<<text;
                isArgPathJob=false;
            }
            if(isLogPathJob==true)
            {
                logPathJob<<text;
                isLogPathJob=false;
            }
            if(isStimeJob==true)
            {
                stimeJob<<text;
                isStimeJob=false;
            }
        }
    }
    if (stream.hasError())
    {
        qDebug()<<"do error handling";
    }
    pubFile.close();
    //    qDebug()<<pnameJob<<priPathJob<<argPathJob<<stimeJob;

    /// ------------------------------------------------------------------------------------------------------------------- ///
    // 设置行数，不设置会显示不出
    historyTableRowNumber=priPathJob.size();
    ui->historyTable->setRowCount(historyTableRowNumber);

    for(int i=0;i<priPathJob.size();i++)
    {
        QTableWidgetItem *itemPname = new QTableWidgetItem(pnameJob[i]);
        QTableWidgetItem *itemStime = new QTableWidgetItem(stimeJob[i]);
        ui->historyTable->setItem(i,0,itemPname);
        ui->historyTable->setItem(i,1,itemStime);

        priFile.setFileName(priPathJob[i]);
        //        qDebug()<<priFile.fileName();
        if(!priFile.open(QFile::ReadOnly))
            qDebug()<<"open failure";

        /// 需要点击才显示相关参数
        /// 显示相关信息
        statementJob="";
        progressJob="";
        curProgressJob="";
        allProgressJob="";
        ltimeJob="";
        endtimeJob="";

        isCurrentTimeJob=false;
        isStatementJob=false;
        isCurrentProgressJob=false;
        isWholeProgressJob=false;

        QXmlStreamReader stream(&priFile);
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
            if(a==4)
            {
                /// 无法读取后面的链接
                QString name=stream.name().toString();
                if(name=="Current_Time")
                {
                    isCurrentTimeJob=true;
                }
                if(name=="Statement")
                {
                    isStatementJob=true;
                }
                if(name=="Current_Progress")
                {
                    isCurrentProgressJob=true;
                }
                if(name=="Whole_Progress")
                {
                    isWholeProgressJob=true;
                }
            }
            if(a==5)
            {
            }
            if(a==6)
            {
                QString text=stream.text().toString();
                if(isCurrentTimeJob==true)
                {
                    endtimeJob=text;
                    isCurrentTimeJob=false;
                }
                if(isStatementJob==true)
                {
                    statementJob=text;
                    isStatementJob=false;
                }
                if(isCurrentProgressJob==true)
                {
                    curProgressJob=text;
                    isCurrentProgressJob=false;
                }
                if(isWholeProgressJob==true)
                {
                    allProgressJob=text;
                    isWholeProgressJob=false;
                }
            }
        }
        if (stream.hasError())
        {
            qDebug()<<"do error handling";
        }
        QTableWidgetItem *itemEndTime = new QTableWidgetItem(endtimeJob);
        QProgressBar *itemPgbar=new QProgressBar();
        QTableWidgetItem *itemStatement=new QTableWidgetItem(statementJob);
        itemPgbar->setMaximum(allProgressJob.toInt());
        itemPgbar->setValue(curProgressJob.toInt());
        ui->historyTable->setItem(i,2,itemEndTime);
        ui->historyTable->setCellWidget(i,3,itemPgbar);
        ui->historyTable->setItem(i,4,itemStatement);

        priFile.close();
    }
    ui->historyTable->resizeColumnsToContents();
}

void qjdProcessMainWindow::on_historyTable_clicked(QModelIndex index)
{
    if(willRemoveRecord==true)
    {
        willRemoveRecord=false;
        return;
    }
    if(willRemoveRecord==false)
    {
        /// ---------------------显示参数文件-------------------- ///
        selectRowNum=index.row();
        //    setHistoryTableArguments();
        fHisArgu.setFileName(argPathJob[selectRowNum]);
        QString groupName=argPathJob[selectRowNum].left(argPathJob[selectRowNum].indexOf(".arg"));
        ui->labelArguFileName->setText(groupName);
        if(!fHisArgu.open(QFile::ReadOnly))
            qDebug()<<"open histable argu failure";
        else
            qDebug()<<"open histable argu success";
        /// 需要点击才显示相关参数
        // TODO:现在仍然是全部显示，没有任何的解析，最好通过解析参数文件，自行生成界面！！！！
        bool isArgu=false;
        QVector<QString> arguName;
        QVector<QString> arguMents;
        QXmlStreamReader stream(&fHisArgu);
        while (!stream.atEnd())
        {
            int a= stream.readNext();

            if(a==4)
            {
                /// 无法读取后面的链接
                //            qDebug()<<"a=4 stream.name"<<stream.name();      // 难办，开头项也是在这里被读出，不太好处理
                QString name=stream.name().toString();
                if(name.isEmpty()==false && name.isNull()==false)
                {
                    isArgu=true;
                    arguName<<name;
                }
            }
            if(a==6)
            {
                QString text=stream.text().toString();
                if(isArgu==true)
                {
                    isArgu=false;
                    arguMents<<text;
                }
            }
        }
        if (stream.hasError())
        {
            qDebug()<<"do error handling"<<stream.errorString();
        }

        QByteArray hisArgu;
        for(int i=0;i<arguName.size();i++)
        {
            hisArgu.append(arguName[i]);
            hisArgu.append(":");
            hisArgu.append(arguMents[i]);
            hisArgu.append("\n");
        }
        ui->historyArguBrowser->setText(hisArgu);

        fHisArgu.close();

        //    ui->btnRestart->setEnabled(true);   //开启restart

        /// --------------------显示日志文件-------------------------- ///
        showFile.setFileName(logPathJob[selectRowNum]);
        if(!showFile.open(QFile::ReadOnly))
            qDebug()<<"show File open failed";

        QByteArray a=showFile.readAll();

        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QString string = codec->toUnicode(a);

        ui->historyLogBrowser->setText(string);

        showFile.close();
    }
}

void qjdProcessMainWindow::on_activeTable_clicked(QModelIndex index)
{
    fActArgu.setFileName(argPathJob[hashActive.value(hashActive.keys().at(index.row()))]);
    if(!fActArgu.open(QFile::ReadOnly))
        qDebug()<<"open act argu failure";
    else
        qDebug()<<"open act argu success";
    /// 需要点击才显示相关参数
    // TODO:现在仍然是全部显示，没有任何的解析，最好通过解析参数文件，自行生成界面！！！！
    //    QByteArray argu=fActArgu.readAll();
    //    ui->activeArguBrowser->setText(argu);

    bool isArgu=false;
    QVector<QString> arguName;
    QVector<QString> arguMents;
    QXmlStreamReader stream(&fActArgu);
    while (!stream.atEnd())
    {
        int a= stream.readNext();

        if(a==4)
        {
            /// 无法读取后面的链接
            //            qDebug()<<"a=4 stream.name"<<stream.name();      // 难办，开头项也是在这里被读出，不太好处理
            QString name=stream.name().toString();
            if(name.isEmpty()==false && name.isNull()==false)
            {
                isArgu=true;
                arguName<<name;
            }
        }
        if(a==6)
        {
            QString text=stream.text().toString();
            if(isArgu==true)
            {
                isArgu=false;
                arguMents<<text;
            }
        }
    }
    if (stream.hasError())
    {
        qDebug()<<"do error handling"<<stream.errorString();
    }

    QByteArray actArgu;
    for(int i=0;i<arguName.size();i++)
    {
        actArgu.append(arguName[i]);
        actArgu.append(":");
        actArgu.append(arguMents[i]);
        actArgu.append("\n");
    }
    ui->activeArguBrowser->setText(actArgu);
    fActArgu.close();

    /// --------------------显示日志文件-------------------------- ///
    showFile.setFileName(logPathJob[hashActive.value(hashActive.keys().at(index.row()))]);
    if(!showFile.open(QFile::ReadOnly))
        qDebug()<<"show File open failed";

    savedOpendActiveLogFileName=showFile.fileName();  // 保存一下，用来刷新
    QByteArray a=showFile.readAll();

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString string = codec->toUnicode(a);

    ui->activeLogBrowser->setText(string);

    showFile.close();
}

void qjdProcessMainWindow::setActiveTableData()
{
    qDebug()<<"setActiveTableData";
    // 显示名称，开始时间，参数，剩余时间，进度
    QVector<QString> cmdKeys;
    QVector<QString> activeStime;

    ui->activeTable->setRowCount(hashActive.keys().size());
    for(int i=0;i<hashActive.keys().size();i++)
    {
        cmdKeys.append(hashActive.keys().at(i));
        /// 错误多发地带
        qDebug()<<stimeJob.size()<<hashActive.value(cmdKeys[i]);
        qDebug()<<stimeJob[hashActive.value(cmdKeys[i])];

        activeStime.append(stimeJob[hashActive.value(cmdKeys[i])]);
        QTableWidgetItem *itemActivePname = new QTableWidgetItem(cmdKeys[i]);
        QTableWidgetItem *itemActiveStime = new QTableWidgetItem(activeStime[i]);
        ui->activeTable->setItem(i,0,itemActivePname);
        ui->activeTable->setItem(i,1,itemActiveStime);

        fActive.setFileName(priPathJob[hashActive.value(cmdKeys[i])]);
        //        qDebug()<<fActive.fileName();
        if(!fActive.open(QFile::ReadOnly))
            qDebug()<<"open failure";

        /// 显示相关信息
        statementJob="";
        progressJob="";
        curProgressJob="";
        allProgressJob="";
        ltimeJob="";

        isCurrentTimeJob=false;
        isStatementJob=false;
        isCurrentProgressJob=false;
        isWholeProgressJob=false;
        isLeftTimeJob=false;

        QXmlStreamReader stream(&fActive);
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
                //                qDebug()<<"a=4 stream.name"<<stream.name();      // 难办，开头项也是在这里被读出，不太好处理
                QString name=stream.name().toString();
                if(name=="Current_Time")
                {
                    isCurrentTimeJob=true;
                }
                if(name=="Statement")
                {
                    isStatementJob=true;
                }
                if(name=="Current_Progress")
                {
                    isCurrentProgressJob=true;
                }
                if(name=="Whole_Progress")
                {
                    isWholeProgressJob=true;
                }
                if(name=="Left_Time")
                {
                    isLeftTimeJob=true;
                }
            }
            if(a==5)
            {
                //                qDebug()<<"a=5 stream.name"<<stream.name();
            }
            if(a==6)
            {
                //                qDebug()<<"a=6 stream.text"<<stream.text();
                QString text=stream.text().toString();
                if(isCurrentTimeJob==true)
                {
                    endtimeJob=text;
                    isCurrentTimeJob=false;
                }
                if(isStatementJob==true)
                {
                    statementJob=text;
                    isStatementJob=false;
                }
                if(isCurrentProgressJob==true)
                {
                    curProgressJob=text;
                    isCurrentProgressJob=false;
                }
                if(isWholeProgressJob==true)
                {
                    allProgressJob=text;
                    isWholeProgressJob=false;
                }
                if(isLeftTimeJob==true)
                {
                    ltimeJob=text;
                    isLeftTimeJob=false;
                }
            }
        }
        if (stream.hasError())
        {
            qDebug()<<"do error handling"<<stream.errorString();
        }

        fActive.close();

        QTableWidgetItem *itemActiveStat = new QTableWidgetItem(statementJob);
        QTableWidgetItem *itemActiveLtime = new QTableWidgetItem(ltimeJob);

        QProgressBar *itemActiveProgress=new QProgressBar();
        itemActiveProgress->setMaximum(allProgressJob.toInt());
        itemActiveProgress->setValue(curProgressJob.toInt());

        ui->activeTable->setItem(i,2,itemActiveStat);
        ui->activeTable->setCellWidget(i,3,itemActiveProgress);
        ui->activeTable->setItem(i,4,itemActiveLtime);
    }
    ui->activeTable->resizeColumnsToContents();
}

void qjdProcessMainWindow::closeEvent(QCloseEvent *)
{
    //备用
    if(pubFile.isOpen())
        pubFile.close();
    if(priFile.isOpen())
        priFile.close();
    if(fHisArgu.isOpen())
        fHisArgu.close();
    if(fActArgu.isOpen())
        fActArgu.close();
    if(fActive.isOpen())
        fActive.close();
}

//void qjdProcessMainWindow::on_actionStart_Process_triggered()
//{
    //    startTask=new qjdStartTask();
    //    startTask->setAttribute(Qt::WA_DeleteOnClose);  //添加这句以备删除,程序仍然会崩溃

    //    ui->stackedWidgetPage->setCurrentIndex(1);
    //    ui->tableSys->setCurrentCell(1,0);
    ////    ui->tabWidgetJob->insertTab(2,startTask,"Start Task");
    ////    ui->tabWidgetJob->setCurrentIndex(2);
    //    connect(startTask,SIGNAL(sigCloseStartTask()),this,SLOT(closeTabStartTask()));
//}

void qjdProcessMainWindow::on_historyTable_cellDoubleClicked(int row, int column)
{
    qDebug()<<row<<column;
}

void qjdProcessMainWindow::refreshTable()
{
    /// 刷新界面，比如新增加的作业，去除完成的不在运行作业
    // 其实就是更新All Table和hashActive
    historyTableRowNumber=0;
    pnameJob.clear();
    priPathJob.clear();
    stimeJob.clear();
    setHistoryTableData();
    ui->historyTable->resizeColumnsToContents();
    ui->activeTable->resizeColumnsToContents();

    setHistoryTableData();
    setFirstActiveTableData();
}

void qjdProcessMainWindow::filterProcess()
{
    /// 筛选进程名称
    filterText=filterEdit->text();
    // 拿此text与cmd作对比,重新setData，有才显示,如果没全部不显示
    machineRefresh=true;
    vectorClear();
    proc->refresh();        //refresh 就开始泄漏了
    if(flagSort==true)
    {
        headerSort();
        setSortData();
    }
    setData();
}

void qjdProcessMainWindow::closeOption()
{
    handleChooseField();
    machineRefresh=true;
    vectorClear();
    proc->refresh();        //refresh 就开始泄漏了
    if(flagSort==true)
    {
        headerSort();
        setSortData();
    }
    setData();

    ui->tblMain->resizeColumnsToContents();
    ui->tableSys->setCurrentCell(0,0);
    ui->stackedWidgetPage->setCurrentIndex(0);
}

void qjdProcessMainWindow::on_tableSys_itemClicked(QTableWidgetItem* item)
{
    if(item->row()==0)
    {
        ui->stackedWidgetPage->setCurrentIndex(0);
        ui->actionSystem_Monitor->setChecked(true);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(false);

        /// 完成选项的选择
        handleChooseField();
        machineRefresh=true;
        vectorClear();
        proc->refresh();        //refresh 就开始泄漏了
        if(flagSort==true)
        {
            headerSort();
            setSortData();
        }
        setData();
    }
    if(item->row()==1)
    {
        ui->stackedWidgetPage->setCurrentIndex(optionsIndex);
        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(true);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(false);    }
}

void qjdProcessMainWindow::on_actionSystemProcess_triggered(bool check)
{
    if(check==true || check==false)
    {
        qDebug()<<"check sys true or false";
        ui->actionSystemProcess->setChecked(true);
        ui->actionJobControl->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(0);
        ui->stackedWidgetPage->setCurrentIndex(0);
        ui->tableSys->setCurrentCell(0,0);

        ui->actionSystem_Monitor->setChecked(true);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(false);
    }
}

void qjdProcessMainWindow::on_actionJobControl_triggered(bool check)
{
    if(check==true || check==false)
    {
        qDebug()<<"check job true or false";
        ui->actionJobControl->setChecked(true);
        ui->actionSystemProcess->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(1);
        ui->stackedWidgetPage->setCurrentIndex(1);
        ui->tableJob->setCurrentCell(0,0);

        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(true);
        ui->actionActive_Task->setChecked(false);
    }
}

void qjdProcessMainWindow::on_tableJob_itemClicked(QTableWidgetItem* item)
{
    int rowToShow=item->row();
    qDebug()<<ui->tableJob->currentRow()<<item->row();
    if(item->row()==2)
    {
        rowToShow=ui->tableJob->currentRow();  //保证横线不被错误索引显示
    }
    if(item->row()==1)
    {
        timerA->start(3000);    //开始了就不暂停了
    }
    ui->stackedWidgetPage->setCurrentIndex(hashLog.value(rowToShow));
    if(rowToShow==0)
    {
        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(true);
        ui->actionActive_Task->setChecked(false);
    }
    if(rowToShow==1)
    {
        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(true);
    }
}

//void qjdProcessMainWindow::saveLog()
//{
//    qDebug()<<"save log";
//    /// 直接有方法，不需要手工复制
//    QString saveFileName=QFileDialog::getSaveFileName(this,tr("Save As..."), "/home/xtf", tr("Log Files (*.log *.txt )"));
////    QString originFileName=hashFileName.value(ui->tableJob->currentItem()->text());

//    if(!QFile::copy(originFileName,saveFileName))
//        qDebug()<<"Save As... Failed";
//}

//void qjdProcessMainWindow::refreshLog()
//{
//    /// 更新原有showLog，产生一定的麻烦,不如关闭再开来的实在
//    // 使源程序自带刷新？
//    qDebug()<<"TODO:refresh log";
//    //    ui->stackedWidgetPage->currentWidget()->
//}

//void qjdProcessMainWindow::closeLog()
//{
//    /// 仅仅是关闭窗口而已
//    qDebug()<<"close log";
//    ui->stackedWidgetPage->removeWidget(ui->stackedWidgetPage->currentWidget());
//    // 关闭之后的后续工作
//    //    hashLog.remove(ui->tableJob->currentRow());
// 不能删除，因为之前的对应关系还会正确，这是由于直接把widget remove掉了，而不是hide
//    ui->tableJob->removeRow(ui->tableJob->currentRow());  //移除被关闭的项目

//    if(ui->tableJob->currentRow()<=2)
//    {
//        ui->stackedWidgetPage->setCurrentIndex(2);
//    }
//}

void qjdProcessMainWindow::deleteLog()
{
    qDebug()<<"del Log";
    // 此处需要弹出对话框，表示谨慎删除
    QMessageBox msgBox;
    msgBox.setText("The record you choose will be deleted.");
    msgBox.setInformativeText("Do you really want to delete it?");
    msgBox.setIcon(QMessageBox::Warning);
    /// 暂时不使用保存，请选择导出功能
    msgBox.setStandardButtons(/*QMessageBox::Save |*/ QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret)
    {
        //    case QMessageBox::Save:
        //        qDebug()<<"save this record";  //保存文件
        //        break;
    case QMessageBox::Ok:
        deleteRecord();  //删除文件
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        return;
        break;
    default:
        qDebug()<<" should never be reached";
        break;
    }


}

void qjdProcessMainWindow::deleteRecord()
{
    int index=ui->historyTable->currentRow();
    /// 删除一条记录的所有文件
    /// 1. 删除相关的3个log文件
    if(!QFile::remove(priPathJob.at(index)))
        qDebug()<<"remove pri file failed, may be it dosen`t existed at all";
    if(!QFile::remove(argPathJob.at(index)))
        qDebug()<<"remove pri file failed, may be it dosen`t existed at all";
    if(!QFile::remove(logPathJob.at(index)))
        qDebug()<<"remove pri file failed, may be it dosen`t existed at all";

    /// 2.删除公共文件中的记录(麻烦，需要解析xml，判断位置)
    // 2.1 删除相关记录
    pnameJob.remove(index);
    priPathJob.remove(index);
    argPathJob.remove(index);
    logPathJob.remove(index);
    stimeJob.remove(index);

    // 2.2 准备重写文件
    pubFile.setFileName(importFileName);
    if(!pubFile.open(QFile::WriteOnly))
        qDebug()<<"pubFile open failed";

    QXmlStreamWriter writer(&pubFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();            //写个开头，不过在已经有开头的文件不需要再写一边开头

    // start end element不能随便使用，使用了便成为整个xml文件的开头和结束
    /// TODO：考虑中间插入节点
    writer.writeStartElement("Path_File");

    for(int i=0;i<pnameJob.size();i++)
    {
        writer.writeStartElement("Record");
        writer.writeTextElement("Process_Name",pnameJob[i]);
        writer.writeTextElement("Private_File_Path",priPathJob[i]);
        writer.writeTextElement("Argument_File_Path",argPathJob[i]);
        writer.writeTextElement("Log_File_Path",logPathJob[i]);
        writer.writeTextElement("Start_Time",stimeJob[i]);
        writer.writeEndElement();
    }
    writer.writeEndDocument();
    pubFile.close();

    /// 3.删除此条table中的记录,界面中的应该最后删除，等一切链接断开之后再做删除
    willRemoveRecord=true;
    setHistoryTableData();  // 最后整个刷新一下table
}

void qjdProcessMainWindow::saveRecord()
{
    /// 保存什么捏？
}

/// 导入索引文件
void qjdProcessMainWindow::on_actionImport_triggered()
{
    importFileName=QFileDialog::getOpenFileName(this,tr("Import index file"), "/home/xtf", tr("Index Files (*.index)"));
    qDebug()<<importFileName;
    setHistoryTableData();  // 最后整个刷新一下table
}
/// 菜单切换显示
// 切换多次会存在显示问题。。。
void qjdProcessMainWindow::on_actionSystem_Monitor_triggered(bool checked)
{
    if(checked==true)
    {
        ui->actionSystemProcess->setChecked(true);
        ui->actionJobControl->setChecked(false);

        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(0);
        ui->tableSys->setCurrentCell(0,0);
        ui->stackedWidgetPage->setCurrentIndex(0);
    }
    if(checked==false)
    {
        ui->actionSystem_Monitor->setChecked(true);
    }
}

void qjdProcessMainWindow::on_actionChoose_Field_triggered(bool checked)
{
    if(checked==true)
    {
        ui->actionSystemProcess->setChecked(true);
        ui->actionJobControl->setChecked(false);

        ui->actionSystem_Monitor->setChecked(false);
        ui->actionAll_task->setChecked(false);
        ui->actionActive_Task->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(0);
        ui->tableSys->setCurrentCell(1,0);
        ui->stackedWidgetPage->setCurrentIndex(optionsIndex);
    }
    if(checked==false)
    {
        ui->actionChoose_Field->setChecked(true);
    }
}

void qjdProcessMainWindow::on_actionAll_task_triggered(bool checked)
{
    if( checked==true)
    {
        ui->actionSystemProcess->setChecked(false);
        ui->actionJobControl->setChecked(true);

        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(false);
        ui->actionActive_Task->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(1);
        ui->tableJob->setCurrentCell(0,0);
        ui->stackedWidgetPage->setCurrentIndex(1);
    }
    if(checked==false)
    {
        ui->actionAll_task->setChecked(true);
    }
}

void qjdProcessMainWindow::on_actionActive_Task_triggered(bool checked)
{
    if( checked==true)
    {
        ui->actionSystemProcess->setChecked(false);
        ui->actionJobControl->setChecked(true);

        ui->actionSystem_Monitor->setChecked(false);
        ui->actionChoose_Field->setChecked(false);
        ui->actionAll_task->setChecked(false);

        ui->stackedWidgetTable->setCurrentIndex(1);
        ui->tableJob->setCurrentCell(1,0);
        ui->stackedWidgetPage->setCurrentIndex(2);
    }
    if(checked==false)
    {
        ui->actionActive_Task->setChecked(true);
    }
}



void qjdProcessMainWindow::on_actionExport_triggered()
{
    /// 导出所有日志文件
    QString saveToDirName=QFileDialog::getExistingDirectory(this, tr("Choose a Directory to Export Index & ALL Log Files"),
                                                            "/home/xtf",
                                                            QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if(saveToDirName=="")
    {
        QMessageBox::warning(this, "Choose One",  "You must choose a dir to export...");
    }
    else
    {
        QDir saveToDir;
        saveToDir.setPath(saveToDirName);
        /// 处理索引文件
        QString exportFileName=saveToDir.absoluteFilePath(importFileName.right(importFileName.size()-1-importFileName.lastIndexOf("/")));

        // 处理日志文件目录地址
        QVector<QString> priPathCopy;
        QVector<QString> argPathCopy;
        QVector<QString> logPathCopy;
        for(int i=0;i<priPathJob.size();i++)
        {
            priPathCopy<<saveToDir.absoluteFilePath(priPathJob[i].right(priPathJob[i].size()-1-priPathJob[i].lastIndexOf("/")));
            argPathCopy<<saveToDir.absoluteFilePath(argPathJob[i].right(argPathJob[i].size()-1-argPathJob[i].lastIndexOf("/")));
            logPathCopy<<saveToDir.absoluteFilePath(logPathJob[i].right(logPathJob[i].size()-1-logPathJob[i].lastIndexOf("/")));
        }
        pubFile.setFileName(exportFileName);
        if(!pubFile.open(QFile::WriteOnly))
            qDebug()<<"pubFile open failed";

        QXmlStreamWriter writer(&pubFile);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();            //写个开头，不过在已经有开头的文件不需要再写一边开头

        // start end element不能随便使用，使用了便成为整个xml文件的开头和结束
        /// TODO：考虑中间插入节点
        writer.writeStartElement("Path_File");

        for(int i=0;i<pnameJob.size();i++)
        {
            writer.writeStartElement("Record");
            writer.writeTextElement("Process_Name",pnameJob[i]);
            writer.writeTextElement("Private_File_Path",priPathCopy[i]);
            writer.writeTextElement("Argument_File_Path",argPathCopy[i]);
            writer.writeTextElement("Log_File_Path",logPathCopy[i]);
            writer.writeTextElement("Start_Time",stimeJob[i]);
            writer.writeEndElement();
        }
        writer.writeEndDocument();
        pubFile.close();

        // 开始复制日志文件
        bool failed=false;
        for(int i=0;i<priPathJob.size();i++)
        {
            if(!QFile::copy(priPathJob[i],priPathCopy[i]))
            {
                qDebug()<<"Copy priPath file failed"<<priPathJob[i];
                failed=true;
            }
            if(!QFile::copy(argPathJob[i],argPathCopy[i]))
            {
                qDebug()<<"Copy argPath file failed"<<argPathJob[i];
                failed=true;
            }
            if(!QFile::copy(logPathJob[i],logPathCopy[i]))
            {
                qDebug()<<"Copy logPath file failed"<<logPathJob[i];
                failed=true;
            }
        }
        if(failed==false)
        {
            QMessageBox::information(this, "Success",  "Export log file success complete");
        }
        if(failed==true)
        {
            QMessageBox::warning(this, "Something happend in the exec",
                                 "May be there were already have some same name file existed in the dir that can`t be overwrited!");
        }
    }
}

void qjdProcessMainWindow::refreshActiveLog()
{
    ui->activeLogBrowser->clear();

    showFile.setFileName(savedOpendActiveLogFileName);
    if(!showFile.open(QFile::ReadOnly))
        qDebug()<<"show File open failed";

    QByteArray a=showFile.readAll();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString string = codec->toUnicode(a);
    ui->activeLogBrowser->setText(string);
    showFile.close();
}
