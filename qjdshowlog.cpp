#include "qjdshowlog.h"
#include "ui_qjdshowlog.h"
#include <QDebug>
#include <QTextCodec>

/// 接收参数，显示文件
qjdShowLog::qjdShowLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qjdShowLog)
{
    ui->setupUi(this);
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB18030"));
}

qjdShowLog::~qjdShowLog()
{
    delete ui;
}

void qjdShowLog::changeEvent(QEvent *e)
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

void qjdShowLog::showLog(QString fileName)
{
    showFile.setFileName(fileName);
    if(!showFile.open(QFile::ReadOnly))
        qDebug()<<"show File open failed";

    QByteArray a=showFile.readAll();

//    QByteArray encodedString = "...";
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString string = codec->toUnicode(a);

    ui->labelFullPath->setText(fileName);
    ui->logBrowser->setText(string);

}

//void qjdShowLog::on_btnClose_clicked()
//{
//    /// 哪个发出的,无法解决多重对象的判别以及对象删除的问题
////    emit sigCloseLog();
//}
