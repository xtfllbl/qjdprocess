#include "qjdtable.h"
#include <QTableView>
#include <QDebug>

// 以备不时之需
qjdTable::qjdTable(QWidget *parent) :
    QTableView(parent)
{
}

void qjdTable::wheelEvent(QWheelEvent *wheelEvent)
{
    qDebug()<<wheelEvent;
}
