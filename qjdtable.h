#ifndef QJDTABLE_H
#define QJDTABLE_H

#include <QObject>
#include <QTableView>

class qjdTable : public QTableView
{
    Q_OBJECT
public:
    qjdTable(QWidget *parent = 0);

signals:

public slots:
    void wheelEvent(QWheelEvent *);
};

#endif // QJDTABLE_H
