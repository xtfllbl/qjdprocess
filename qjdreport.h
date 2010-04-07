#ifndef QJDREPORT_H
#define QJDREPORT_H

#include <QDialog>
#include "ui_qjdreport.h"

namespace Ui {
    class qjdreport;
}

class qjdreport : public QDialog {
    Q_OBJECT
public:
    explicit qjdreport(QWidget *parent = 0);
    ~qjdreport();
    Ui::qjdreport *ui;

protected:
    void changeEvent(QEvent *e);

private:

    void closeEvent(QCloseEvent *);
};

#endif // QJDREPORT_H
