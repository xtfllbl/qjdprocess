#include "qjdstarttask.h"
#include "ui_qjdstarttask.h"
#include <QDebug>

// not using any more
/// 提供个人便利
qjdStartTask::qjdStartTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdStartTask)
{
    ui->setupUi(this);

    process = new QProcess(this);
    connect(process,SIGNAL(error(QProcess::ProcessError)),this,SLOT(processError(QProcess::ProcessError)));
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readOutput()));

    // 设置窗口有最小化和关闭按钮
    setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
}

qjdStartTask::~qjdStartTask()
{
    delete ui;
}

void qjdStartTask::changeEvent(QEvent *e)
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

void qjdStartTask::on_btnBrowse_pressed()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,tr("Open Process"), "/home/xtf");
    ui->lineFileName->setText(fileName);
}

void qjdStartTask::on_btnStart_pressed()
{
    QString fileName=ui->lineFileName->text();
    process->start(fileName);
    ui->lblStat->setText("Already Start");
}

void qjdStartTask::on_btnStop_pressed()
{
    if(process->isOpen())
        process->close();
    ui->lblStat->setText("Stoped");
}

void qjdStartTask::processError(QProcess::ProcessError error)
{
    // 处理进程错误
    // SIGTERM,SIGKILL都表现为crashed
    qDebug()<<"error Number:"<<error;
    if(error==0)
        qDebug()<<"The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
    if(error==1)
        qDebug()<<"The process crashed some time after starting successfully.";
    if(error==2)
        qDebug()<<"The last waitFor...() function timed out. The state of QProcess is unchanged, and you can try calling waitFor...() again.";
    if(error==3)
        qDebug()<<"An error occurred when attempting to read from the process. For example, the process may not be running..";
    if(error==4)
        qDebug()<<"An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.";
    /// 未知错误就是被强行终止了的后果
    // 所谓继续执行，就是在认为暂停作业之后再手动开启的正常情况，kill掉的算意外~~~~~~~~~~~~
    if(error==5)
        qDebug()<<"An unknown error occurred. This is the default return value of error().";
}

//可读取相关信息，但是一旦Kill，就不保证了
void qjdStartTask::readOutput()
{
    qDebug()<<"has sth 2 read~~~~~";
//    qDebug()<<"stdout::"<<process->readAllStandardOutput();
    qDebug()<<"stderr::"<<process->readAllStandardError();
}

void qjdStartTask::on_btnClose_pressed()
{
    emit sigCloseStartTask();
}
