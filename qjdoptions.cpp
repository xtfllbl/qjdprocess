#include "qjdoptions.h"
#include "ui_qjdoptions.h"
#include <QDebug>
qjdoptions::qjdoptions(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::qjdoptions)
{
    ui->setupUi(this);
    cmd=true;
    cmdLine=false;
    ioRead=false;
    ioWrite=false;
    mem=true;
    nice=true;
    pcpu=true;
    pid=true;
    pmem=false;
    sleepAvg=false;
    stackSize=true;
    startTime=true;
    stat=true;
    uid=false;
    usrName=true;
    wcpu=true;
    wchan=false;
    whichCpu=true;

//    ui->chkCMD->setChecked(true);
//    ui->chkNice->setChecked(true);
//    ui->chkStartTime->setChecked(true);
//    ui->chkMem->setChecked(true);
//    ui->chkStackSize->setChecked(true);
//    ui->chkWhichCPU->setChecked(true);
//    ui->chkPCPU->setChecked(true);
//    ui->chkUsrName->setChecked(true);
//    ui->chkStat->setChecked(true);
//    countCol=10;
    setWindowFlags(Qt::ToolTip);
}

qjdoptions::~qjdoptions()
{
    delete ui;
}

void qjdoptions::changeEvent(QEvent *e)
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

void qjdoptions::on_btnApplyField_clicked()
{
    handleCheck();
}

void qjdoptions::handleCheck()
{
//    qDebug()<<"handle check";
    countCol=0;
    if(ui->chkCMD->isChecked()==true)
    {
        cmd=true;
        countCol++;
    }
    if(ui->chkCMD->isChecked()==false)      cmd=false;
    if(ui->chkCMDline->isChecked()==true)
    {
        cmdLine=true;
        countCol++;
    }
    if(ui->chkCMDline->isChecked()==false)      cmdLine=false;
    if(ui->chkIOREAD->isChecked()==true)
    {
        ioRead=true;
        countCol++;
    }
    if(ui->chkIOREAD->isChecked()==false)     ioRead =false;
    if(ui->chkIOWRITE->isChecked()==true)
    {
        ioWrite =true;
        countCol++;
    }
    if(ui->chkIOWRITE->isChecked()==false)    ioWrite  =false;
    if(ui->chkMem->isChecked()==true)
    {
        mem=true;
        countCol++;
    }
    if(ui->chkMem->isChecked()==false)     mem =false;
    if(ui->chkNice->isChecked()==true)
    {
        nice=true;
        countCol++;
    }
    if(ui->chkNice->isChecked()==false)      nice=false;
    if(ui->chkPCPU->isChecked()==true)
    {
        pcpu=true;
        countCol++;
    }
    if(ui->chkPCPU->isChecked()==false)     pcpu =false;
    if(ui->chkPID->isChecked()==true)
    {
        pid =true;
        countCol++;
    }
    if(ui->chkPID->isChecked()==false)      pid=false;
    if(ui->chkPMem->isChecked()==true)
    {
        pmem=true;
        countCol++;
    }
    if(ui->chkPMem->isChecked()==false)     pmem =false;
    if(ui->chkStat->isChecked()==true)
    {
        stat =true;
        countCol++;
    }
    if(ui->chkStat->isChecked()==false)    stat =false;
    if(ui->chkSleepAvg->isChecked()==true)
    {
        sleepAvg =true;
        countCol++;
    }
    if(ui->chkSleepAvg->isChecked()==false)    sleepAvg  =false;
    if(ui->chkStackSize->isChecked()==true)
    {
        stackSize  =true;
        countCol++;
    }
    if(ui->chkStackSize->isChecked()==false)   stackSize   =false;
    if(ui->chkStartTime->isChecked()==true)
    {
        startTime  =true;
        countCol++;
    }
    if(ui->chkStartTime->isChecked()==false)   startTime   =false;
    if(ui->chkUID->isChecked()==true)
    {
        uid=true;
        countCol++;
    }
    if(ui->chkUID->isChecked()==false)      uid=false;
    if(ui->chkUsrName->isChecked()==true)
    {
        usrName =true;
        countCol++;
    }
    if(ui->chkUsrName->isChecked()==false)    usrName  =false;
    if(ui->chkWchan->isChecked()==true)
    {
        wchan =true;
        countCol++;
    }
    if(ui->chkWchan->isChecked()==false)    wchan  =false;
    if(ui->chkWCPU->isChecked()==true)
    {
        wcpu=true;
        countCol++;
    }
    if(ui->chkWCPU->isChecked()==false)     wcpu =false;
    if(ui->chkWhichCPU->isChecked()==true)
    {
        whichCpu =true;
        countCol++;
    }
    if(ui->chkWhichCPU->isChecked()==false)    whichCpu  =false;

}
