#include "qjdfilterlineedit.h"
#include <QDebug>
qjdFilterLineEdit::qjdFilterLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    setCursor(Qt::IBeamCursor);
    setMaximumWidth(150);
    setText("Filter");

    QPalette palette1;
    QBrush brush5(QColor(186, 186, 186, 255));
    brush5.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Active, QPalette::Text, brush5);
    palette1.setBrush(QPalette::Inactive, QPalette::Text, brush5);
    QBrush brush6(QColor(125, 133, 163, 255));
    brush6.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Disabled, QPalette::Text, brush6);
    setPalette(palette1);
}

void qjdFilterLineEdit::focusInEvent(QFocusEvent *event)
{
    /// 继承原有效果，以此为基础添加新效果
    QLineEdit::focusInEvent(event);
    if(text()=="Filter")
    {
        setText("");
    }
    QPalette palette1;
    QBrush brush5(QColor(0, 0, 0, 255));
    brush5.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Active, QPalette::Text, brush5);
    palette1.setBrush(QPalette::Inactive, QPalette::Text, brush5);
    QBrush brush6(QColor(125, 133, 163, 255));
    brush6.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Disabled, QPalette::Text, brush6);
    setPalette(palette1);
}

void qjdFilterLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    if(text()=="")
    {
        QPalette palette1;
        QBrush brush5(QColor(186, 186, 186, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Text, brush5);
        palette1.setBrush(QPalette::Inactive, QPalette::Text, brush5);
        QBrush brush6(QColor(125, 133, 163, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Disabled, QPalette::Text, brush6);
        setPalette(palette1);

        setText("Filter");
    }
}
