#ifndef MODELEVENTE_H
#define MODELEVENTE_H

#include <QObject>
#include "vente.h"
#include <QList>
#include <QDateTime>
#include <QString>

#include "modeleproduit.h"
#include "database/databasemanager.h"



class ModeleVente : public QObject
{
    Q_OBJECT
public:
    explicit ModeleVente(QObject *parent = 0, ModeleProduit* modeleProduit = 0);

    bool enregistrerVente(Vente& donneeVente, QList<ComposantVente>& composantsVente);
    Vente getVenteById(int id_vente); //sans ses composants
    QList<ComposantVente> getComposantsVentePourVente(int id_vente);
    QList<Vente> getAllVente();
    QList<Vente> getVenteParPlageDeDate(const QDateTime& dateDebut, const QDateTime& dateFin);
    QList<Vente> getVenteByClient(int id_client);


signals:

public slots:

private:
    ModeleProduit* m_modeleProduit;

    UniteVenteProduit getUniteVenteProduitById(int id_unite);

};

#endif // MODELEVENTE_H
