#ifndef QJDREPORT_H
#define QJDREPORT_H

#include <QDialog>

namespace Ui {
    class qjdReport;
}

class qjdReport : public QDialog {
    Q_OBJECT
public:
    explicit qjdReport(QWidget *parent = 0);
    ~qjdReport();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::qjdReport *ui;

    void closeEvent(QCloseEvent *);
};

#endif // QJDREPORT_H
