#ifndef QJDSTARTTASK_H
#define QJDSTARTTASK_H
#include <QFileDialog>
#include <QProcess>
#include <QDialog>

namespace Ui {
    class qjdStartTask;
}

class qjdStartTask : public QDialog
{
    Q_OBJECT

public:
    explicit qjdStartTask(QWidget *parent = 0);
    ~qjdStartTask();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::qjdStartTask *ui;

    QProcess *process;
private slots:
    void on_btnStop_pressed();
    void on_btnStart_pressed();
    void on_btnBrowse_pressed();

    void readOutput();
    void processError(QProcess::ProcessError error);
};

#endif // QJDSTARTTASK_H
