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

private slots:
};

#endif // QJDSHOWLOG_H
