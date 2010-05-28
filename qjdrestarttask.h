#ifndef QJDRESTARTTASK_H
#define QJDRESTARTTASK_H

#include <QDialog>

namespace Ui {
    class qjdRestartTask;
}

class qjdRestartTask : public QDialog
{
    Q_OBJECT

public:
    explicit qjdRestartTask(QWidget *parent = 0);
    ~qjdRestartTask();

protected:
    void changeEvent(QEvent *e);

signals:
    void sigCloseRestartTask();
private:
    Ui::qjdRestartTask *ui;

private slots:
    void on_btnClose_clicked();
};

#endif // QJDRESTARTTASK_H
