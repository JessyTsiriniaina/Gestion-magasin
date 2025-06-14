#ifndef CONTROLEURRAPPORT_H
#define CONTROLEURRAPPORT_H

#include <QObject>
#include <QDate>
#include <QStandardItemModel>
#include <QString>
#include <QList>

#include "models/modeleproduit.h"
#include "models/modelerapport.h"
#include "models/modeleclient.h"

class ControleurRapport : public QObject
{
    Q_OBJECT
public:
    explicit ControleurRapport(QObject *parent = 0, ModeleRapport* modeleRapport = 0, ModeleClient* modeleClient = 0, ModeleProduit* modeleProduit = 0);


    bool genererHistoriqueDeVente(const QDate& dateDebut, const QDate& dateFin, QStandardItemModel* tableModel);
    bool genererHistoriqueDeVenteMensuel(int annee, int mois, QStandardItemModel* tableModel);
    bool genererHistoriqueDeVenteParClient(int id_client, const QDate& dateDebut, const QDate& dateFin, QStandardItemModel* tableModel);

    bool genererRapportProduitPlusVendu(const QDate& dateDebut, const QDate& dateFin, int limite, QStandardItemModel* tableModel);
    bool genererRapportProduitEnRuptureStock(double seuil, QStandardItemModel* tableModel);


    QList<Client> rechercherClient(const QString& motCle);

signals:

public slots:


private:
    ModeleRapport* m_modeleRapport;
    ModeleClient* m_modeleClient;
    ModeleProduit* m_modeleProduit;

    QString getNomClient(int id_client);

    void remplirLigneVente(QStandardItemModel* tableModel, const Vente& vente);
    void remplirProduitPlusVenduLigne(QStandardItemModel* tableModel, const ProduitPlusVenduInfo& produitInfo);
    void remplirProduitEnRuptureStockLigne(QStandardItemModel* tableModel, const ProduitEnRuptureStockInfo& produitInfo);

    void remplirVenteMensuelLigne(QStandardItemModel* tableModel, const QString& moisDeAnnee, double totalVentes);

};

#endif // CONTROLEURRAPPORT_H
