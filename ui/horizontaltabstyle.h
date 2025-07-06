#ifndef HORIZONTALTABSTYLE_H
#define HORIZONTALTABSTYLE_H


#include <QProxyStyle>
#include <QStyleOptionTab>
#include <QPainter>

class HorizontalTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;

    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
};

#endif // HORIZONTALTABSTYLE_H
