#include "qjdrestarttask.h"
#include "ui_qjdrestarttask.h"

/// 遗弃－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－///
qjdRestartTask::qjdRestartTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdRestartTask)
{
    ui->setupUi(this);
}

qjdRestartTask::~qjdRestartTask()
{
    delete ui;
}

void qjdRestartTask::changeEvent(QEvent *e)
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

void qjdRestartTask::on_btnClose_clicked()
{
    emit sigCloseRestartTask();
}
