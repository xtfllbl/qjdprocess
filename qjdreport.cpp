#include "qjdreport.h"
#include "ui_qjdreport.h"
#include "qdebug.h"
qjdReport::qjdReport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdReport)
{
    ui->setupUi(this);
}

qjdReport::~qjdReport()
{
    delete ui;
}

void qjdReport::changeEvent(QEvent *e)
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

void qjdReport::closeEvent(QCloseEvent *event)
{
    delete ui;
}
