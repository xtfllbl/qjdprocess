#ifndef QJDFILTERLINEEDIT_H
#define QJDFILTERLINEEDIT_H

#include <QWidget>
#include <QLineEdit>
class qjdFilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit qjdFilterLineEdit(QWidget *parent = 0);

signals:

public slots:

private:
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
};

#endif // QJDFILTERLINEEDIT_H
