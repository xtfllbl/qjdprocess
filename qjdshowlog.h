#ifndef QJDSHOWLOG_H
#define QJDSHOWLOG_H

#include <QDialog>
#include <QFile>

namespace Ui {
    class qjdShowLog;
}

class qjdShowLog : public QDialog
{
    Q_OBJECT

public:
    explicit qjdShowLog(QWidget *parent = 0);
    ~qjdShowLog();
    void showLog(QString);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::qjdShowLog *ui;

    QFile showFile;
signals:
//    void sigCloseLog(); //无法解决多重对象问题
private slots:

private slots:
//    void on_btnClose_clicked();
};

#endif // QJDSHOWLOG_H
