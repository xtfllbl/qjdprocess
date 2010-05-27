#ifndef QJDPROCESSMAINWINDOW_H
#define QJDPROCESSMAINWINDOW_H

#include <QMainWindow>
#include "qjdproc.h"
#include "qjdoptions.h"
#include <QTimer>
#include <signal.h>
#include <errno.h>
#include <QStandardItemModel>
#include <QKeyEvent>

#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QtXml/QXmlStreamReader>
#include <qjdstarttask.h>

namespace Ui {
    class qjdProcessMainWindow;
}

class qjdProcessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    qjdProcessMainWindow(QWidget *parent = 0);
    ~qjdProcessMainWindow();

    void setData();
    void setSortData();
    void vectorClear();
    void flagClear();

    qjdoptions *options;
    Proc *proc;
    qjdStartTask *startTask;

    QTimer *timer;
    QTimer *reportTimer;
    QMenu *menu;
    QString colName;
    QString colNameSave;
    QStandardItemModel *model;

    QAction *actStop;
    QAction *actCon;
    QAction *actTer;
    QAction *actKill;
    QAction *actHan;
    QAction *actView;

    QString aPid;
    QString aCmd;
    QString aStat;
    QString aNice;
    QString aStartTime;
    QString aWchan;
    QString aWhichCpu;
    QString aMem;
    QString aPmem;
    QString aSleepAvg;
    QString aStack;
    QString aIoread;
    QString aIowrite;
    QString aPcpu;
    QString aWcpu;
    QString aCmdLine;  //cmdlineVector
    QString aUid;
    QString aUsrName;

    QStandardItem *itemPid;
    QStandardItem *itemCmd;
    QStandardItem *itemStat;
    QStandardItem *itemNice;
    QStandardItem *itemStartTime;
    QStandardItem *itemWchan;
    QStandardItem *itemWhichCpu;
    QStandardItem *itemMem;
    QStandardItem *itemPmem;
    QStandardItem *itemSleepAvg;
    QStandardItem *itemStack;
    QStandardItem *itemIoread;
    QStandardItem *itemIowrite;
    QStandardItem *itemPcpu;
    QStandardItem *itemWcpu;
    QStandardItem *itemCmdLine;
    QStandardItem *itemUid;
    QStandardItem *itemUsrName;

    int colNumberSave;
    bool machineRefresh;

    int addFlag;
    int selectCol;
    int selectRow;
    int processID;
    int selfID;
    QString processName;
    bool flagUse;
    bool flagFirstRun;
    bool flagSort;

    int flagSortPid;
    int flagSortProcessName;
    int flagSortState;
    int flagSortNice;
    int flagSortStartTime;
    int flagSortWchan;
    int flagSortCpu;
    int flagSortMemory;
    int flagSortPMemory;
    int flagSortSleep;
    int flagSortStack;
    int flagSortIoRead;
    int flagSortIoWrite;
    int flagSortPCPU;
    int flagSortWCPU;
    int flagSortCMDLine;
    int flagSortUid;
    int flagSortUsrName;

    int flagSortPidSave;
    int flagSortProcessNameSave;
    int flagSortStateSave;
    int flagSortNiceSave;
    int flagSortStartTimeSave;
    int flagSortWchanSave;
    int flagSortCpuSave;
    int flagSortMemorySave;
    int flagSortPMemorySave;
    int flagSortSleepSave;
    int flagSortStackSave;
    int flagSortIoReadSave;
    int flagSortIoWriteSave;
    int flagSortPCPUSave;
    int flagSortWCPUSave;
    int flagSortCMDLineSave;
    int flagSortUidSave;
    int flagSortUsrNameSave;

    bool cmd2;
    bool cmdLine2;
    bool ioRead2;
    bool ioWrite2;
    bool mem2;
    bool nice2;
    bool pcpu2;
    bool pid2;
    bool pmem2;
    bool stat2;
    bool sleepAvg2;
    bool stackSize2;
    bool startTime2;
    bool uid2;
    bool usrName2;
    bool wcpu2;
    bool wchan2;
    bool whichCpu2;

    int colNum;

    bool hasOptions;

    QFile fp;
    QString path;
    QString arguments;
    QString stime;
    QString statement;
    QString progress;
    QString curProgress;
    QString allProgress;
    QString ltime;
    bool reportIsShow;

    QHash<QString, int> hashActive;

public slots:
    void setActiveTableData();

signals:

protected:
    void changeEvent(QEvent *e);

private:
    Ui::qjdProcessMainWindow *ui;

    QFile pubFile;
    QFile priFile;
    QFile argFile;

    QFile fHisArgu;
    QFile fActArgu;
    QFile fActive;

    QTimer *timerA;
    QVector<QString> priPathJob;
    QVector<QString> argPathJob;
    QVector<QString> stimeJob;
    QVector<QString> pnameJob;
    bool isPnameJob;
    bool isPriPathJob;
    bool isArgPathJob;
    bool isStimeJob;

    bool isCurrentTimeJob;
    bool isStatementJob;
    bool isCurrentProgressJob;
    bool isWholeProgressJob;
    bool isLeftTimeJob;

    QVector<QString> argumentsJob;
    QString statementJob;
    QString progressJob;
    QString curProgressJob;
    QString allProgressJob;
    QString ltimeJob;
    QString endtimeJob;

    int historyTableRowNumber;
    int historyTableColNumber;
    int selectRowNum;
    int selectRowNumA;

private slots:
    void on_historyTable_cellDoubleClicked(int row, int column);
    void on_tableChoose_cellClicked(int row, int column);
    void on_btnChooseField_pressed();
    void on_actionStart_Process_triggered();
    void keyPress(QKeyEvent *);
    void on_tblMain_pressed(QModelIndex );
    void handleChooseField();
    void on_actionExit_triggered();
    void autoRefresh();
    void showContextMenu(QPoint);
    void killProcess();
    void terProcess();
    void hanProcess();
    void stopProcess();
    void conProcess();
    void send_to_selected(int);
    void sendsig(int, int);
    void headerSort();
    void headerHandle(int);
    bool eventFilter(QObject *obj, QEvent *event);
    void setFirstActiveTableData();

/// -----------------------------------------
    void on_activeTable_clicked(QModelIndex index);
    void on_tabWidgetJob_selected(QString );
    void on_historyTable_clicked(QModelIndex index);
    void setHistoryTableData();
    void closeEvent(QCloseEvent *);

    void closetab();
    void refreshTable();
};
#endif // QJDPROCESSMAINWINDOW_H
