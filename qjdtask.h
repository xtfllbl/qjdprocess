#ifndef QJDTASK_H
#define QJDTASK_H

#include <QDialog>
#include "ui_qjdtask.h"
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QtXml/QXmlStreamReader>
#include <qjdstarttask.h>

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
    qjdStartTask *startTask;

    QFile pubFile;
    QFile priFile;
    QFile argFile;

    QFile fHisArgu;
    QFile fActArgu;
    QFile fActive;

    QTimer *timerA;
//    bool hasTimerA;
    QVector<QString> priPath;
    QVector<QString> argPath;
    QVector<QString> stime;
    QVector<QString> pname;
    bool isPname;
    bool isPriPath;
    bool isArgPath;
    bool isStime;

    bool isCurrentTime;
    bool isStatement;
    bool isCurrentProgress;
    bool isWholeProgress;
    bool isLeftTime;
//    bool isStime;
    QVector<QString> arguments;
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
    void on_btnStart_clicked();
    void on_btnClose_clicked();
    void on_activeTable_clicked(QModelIndex index);
    void on_tabWidget_selected(QString );
    void on_historyTable_clicked(QModelIndex index);
    void setHistoryTableData();
//    void setHistoryTableArguments();
    void headerHandleH(int);
    void headerHandleA(int);

    void restartProgress();
    void handleError(QProcess::ProcessError);

};

#endif // QJDTASK_H
