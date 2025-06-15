#include "shadoweffect.h"

ShadowEffect::ShadowEffect()
{

}

QGraphicsDropShadowEffect * ShadowEffect::createShadow(QObject* parent)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(parent);
    effect->setBlurRadius(6);
    effect->setXOffset(0);
    effect->setYOffset(1);
    effect->setColor(QColor(0, 0, 0, 50));
    return effect;
}
