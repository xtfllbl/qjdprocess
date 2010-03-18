#include "qjdtable.h"
#include <QTableView>
#include <QDebug>

qjdTable::qjdTable(QWidget *parent) :
    QTableView(parent)
{
}

void qjdTable::wheelEvent(QWheelEvent *wheelEvent)
{
    qDebug()<<wheelEvent;


}
