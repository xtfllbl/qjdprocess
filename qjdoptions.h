#ifndef QJDOPTIONS_H
#define QJDOPTIONS_H

#include <QDialog>
#include <ui_qjdoptions.h>
namespace Ui {
    class qjdoptions;
}

class qjdoptions : public QDialog {
    Q_OBJECT
public:
    qjdoptions(QWidget *parent = 0);
    ~qjdoptions();
    Ui::qjdoptions *ui;

    bool cmd;
    bool cmdLine;
    bool ioRead;
    bool ioWrite;
    bool mem;
    bool nice;
    bool pcpu;
    bool pid;
    bool pmem;
    bool stat;
    bool sleepAvg;
    bool stackSize;
    bool startTime;
    bool uid;
    bool usrName;
    bool wcpu;
    bool wchan;
    bool whichCpu;

    int countCol;


protected:
    void changeEvent(QEvent *e);

signals:
    void sigCloseTab();

public slots:
    void on_btnApplyField_clicked();
    void handleCheck();
};

#endif // QJDOPTIONS_H
