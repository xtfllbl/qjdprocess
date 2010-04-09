#include "qjdreport.h"
#include "ui_qjdreport.h"

qjdreport::qjdreport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdreport)
{
    ui->setupUi(this);
}

qjdreport::~qjdreport()
{
    delete ui;
}

void qjdreport::changeEvent(QEvent *e)
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

void qjdreport::closeEvent(QCloseEvent *)
{
    hide();
}
