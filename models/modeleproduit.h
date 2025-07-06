#ifndef MODELEPRODUIT_H
#define MODELEPRODUIT_H

#include <QObject>
#include <QList>
#include <QString>

#include "database/databasemanager.h"
#include "produit.h"


class ModeleProduit : public QObject
{
    Q_OBJECT
public:
    explicit ModeleProduit(QObject *parent = 0);

    bool ajouterProduit(Produit& donneeProduit, QList<UniteVenteProduit>& unites);
    bool modifierProduit(const Produit& donneeProduit);
    bool supprimerProduit(int id_produit);
    Produit getProduitById(int id_produit);

    QList<Produit> getAllProduit();




    bool ajouterUniteVenteProduit(UniteVenteProduit& donneeUnite);
    bool modifierUniteVenteProduit(const UniteVenteProduit& donneUnite);
    bool supprimerUniteVenteProduit(int id_unite);
    QList<UniteVenteProduit> getUniteVenteProduitPourProduit(int id_produit);


    double getStockProduitEnUniteDeBase(int id_produit);
    bool reapprovisionnerStockProduit(int id_produit, double nouveauQuantiteStockEnUniteDeBase);

    QList<Produit> rechercherProduit(const QString &motCle);


signals:

public slots:

private:
    bool  nomProduitExiste(const QString& nom, int idProduitCourant = -1);

};

#endif // MODELEPRODUIT_H
