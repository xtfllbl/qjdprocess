#ifndef QJDTASK_H
#define QJDTASK_H

#include <QDialog>
#include "ui_qjdtask.h"
#include <QFile>

namespace Ui {
    class qjdTask;
}

class qjdTask : public QDialog {
    Q_OBJECT
public:
    explicit qjdTask(QWidget *parent = 0);
    ~qjdTask();
    Ui::qjdTask *ui;

    void setTableData();
//    void setLabelData();
protected:
    void changeEvent(QEvent *e);

private:
    QFile fp;
    QFile fp2;
    QVector<QString> pname;
    QVector<QString> path;
    QVector<QString> arguments;
    QVector<QString> stime;
    QString statement;
    QString progress;
    QString curProgress;
    QString allProgress;
    QString ltime;

    int historyTableRowNumber;
    int historyTableColNumber;
    int selectRowNum;


private slots:
    void on_historyTable_clicked(QModelIndex index);
    void setTablePrivateData();
};

#endif // QJDTASK_H
