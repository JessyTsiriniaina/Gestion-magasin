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



/*

if (element == CE_TabBarTabLabel) {
        QStyleOptionTab opt(*qstyleoption_cast<const QStyleOptionTab *>(option));
        opt.shape = QTabBar::RoundedNorth;  // ← texte horizontal

        QProxyStyle::drawControl(element, &opt, painter, widget);
    }
    else if (element == CE_TabBarTab) {
        QStyleOptionTab opt(*qstyleoption_cast<const QStyleOptionTab *>(option));
        opt.shape = QTabBar::RoundedNorth;

        QRect rect = opt.rect;
        QPainterPath path;
        path.addRoundedRect(rect.adjusted(1, 1, -1, -1), 10, 10);

        // Couleur de fond
        QColor fillColor = (opt.state & QStyle::State_Selected) ? QColor("#ffffff") : QColor("#e0e0e0");
        painter->fillPath(path, fillColor);

        // Bordure si sélectionné
        if (opt.state & QStyle::State_Selected) {
            QPen pen(QColor("#0078d7"), 2); // bordure bleue
            painter->setPen(pen);
        } else {
            painter->setPen(Qt::NoPen);
        }

        painter->drawPath(path);

        // Appelle le dessin du label (texte + icône)
        QProxyStyle::drawControl(CE_TabBarTabLabel, &opt, painter, widget);
    }
    else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }





*/
