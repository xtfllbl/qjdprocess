#ifndef QJDTASK_H
#define QJDTASK_H

#include <QDialog>
#include "ui_qjdtask.h"
#include <QFile>
#include <QTimer>

namespace Ui {
    class qjdTask;
}

class qjdTask : public QDialog {
    Q_OBJECT
public:
    explicit qjdTask(QWidget *parent = 0);
    ~qjdTask();
    Ui::qjdTask *ui;
    QHash<QString, int> hashActive;
signals:
    void sigRefresh();
public slots:
    void setActiveTableData();
protected:
    void changeEvent(QEvent *e);

private:
    QFile fp;
    QFile fp2;
    QFile fHisArgu;
    QFile fActArgu;
    QFile fActive;

    QTimer *timerA;
//    bool hasTimerA;
    QVector<QString> pname;
    QVector<QString> path;
    QVector<QString> arguments;
    QVector<QString> stime;
    QString statement;
    QString progress;
    QString curProgress;
    QString allProgress;
    QString ltime;
    QString endtime;

    int historyTableRowNumber;
    int historyTableColNumber;
    int selectRowNum;
    int selectRowNumA;

    void closeEvent(QCloseEvent *);

private slots:
    void on_btnRefresh_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_activeTable_clicked(QModelIndex index);
    void on_tabWidget_selected(QString );
    void on_historyTable_clicked(QModelIndex index);
    void setHistoryTableData();
    void setHistoryTablePrivateData();
    void headerHandleH(int);
    void headerHandleA(int);
};

#endif // QJDTASK_H
