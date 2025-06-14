#ifndef CONTROLEURPRODUIT_H
#define CONTROLEURPRODUIT_H

#include <QObject>
#include <QStandardItemModel>
#include <QList>

#include "models/modeleproduit.h"


class ControleurProduit : public QObject
{
    Q_OBJECT
public:
    explicit ControleurProduit(QObject *parent = 0, ModeleProduit* modeleProduit = 0);

    bool chargerProduitsDansTableModel(QStandardItemModel* tableModel, const QString& motCle = "");

    Produit getProduitDepuisModel(int id_produit);
    QList<UniteVenteProduit> getUniteVenteProduitPourProduit(int id_produit);
    bool ajouterProduit(Produit& donneeProduit, QList<UniteVenteProduit>& unites);
    bool modifierProduit(const Produit& donneeProduit);
    bool supprimerProduit(int id_produit);


    bool chargerUnitesVenteProduitDansTableModel(QStandardItemModel* tableModel, int id_produit);
    bool ajouterUniteVenteProduit(UniteVenteProduit& donneeUnite);
    bool modifierUniteVenteProduit(const UniteVenteProduit& donneeUnite);
    bool supprimerUniteVenteProduit(int id_unite);

    bool reapprovisionnerStockProduit(int id_produit, double nouveauQuantiteStockEnUniteDeBase);
    double getStockProduitEnUniteDeBase(int id_produit);

signals:

public slots:

private:
    ModeleProduit* m_modeleProduit;

    void remplirLigneProduit(QStandardItemModel* tableModel, const Produit& produit);
    void remplirLigneUniteVente(QStandardItemModel* tableModel, const UniteVenteProduit& unite);

};

#endif // CONTROLEURPRODUIT_H
