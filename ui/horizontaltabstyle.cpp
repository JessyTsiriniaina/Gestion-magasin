#include "horizontaltabstyle.h"

QSize HorizontalTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                           const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_TabBarTab)
        return QSize(200, 50);
    return s;
}

void HorizontalTabStyle::drawControl(ControlElement element, const QStyleOption *option,
                                     QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel) {
        QStyleOptionTab opt(*qstyleoption_cast<const QStyleOptionTab *>(option));

        opt.shape = QTabBar::RoundedNorth;
        QProxyStyle::drawControl(element, &opt, painter, widget);
    } else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

