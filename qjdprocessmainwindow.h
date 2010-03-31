#ifndef QJDPROCESSMAINWINDOW_H
#define QJDPROCESSMAINWINDOW_H

#include <QMainWindow>
#include <qjdproc.h>
#include <QTimer>
#include <qjdoptions.h>
#include <signal.h>
#include <errno.h>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <qjdtable.h>

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
    qjdTable *qjdtable;

    QTimer *timer;
    QMenu *menu;
    QString colName;
    QString colNameSave;
    QStandardItemModel *model;

    QAction *actStop;
    QAction *actCon;
    QAction *actTer;
    QAction *actKill;
    QAction *actHan;

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
protected:
    void changeEvent(QEvent *e);
private:
    Ui::qjdProcessMainWindow *ui;
private slots:
    void on_actionStopRefresh_triggered(bool );
    void on_actionManualRefresh_triggered();
    void keyPress(QKeyEvent *);
    void on_tblMain_pressed(QModelIndex );
    void on_actionChoose_Field_triggered(bool );
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
//   void setTable();

};
#endif // QJDPROCESSMAINWINDOW_H
