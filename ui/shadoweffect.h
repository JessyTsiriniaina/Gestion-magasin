#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H

#include <QGraphicsDropShadowEffect>

class ShadowEffect
{
public:
    ShadowEffect();
    static QGraphicsDropShadowEffect *createShadow(QObject* parent = 0);
};

#endif // SHADOWEFFECT_H
