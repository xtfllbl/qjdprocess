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

private:
    Ui::qjdRestartTask *ui;
};

#endif // QJDRESTARTTASK_H
