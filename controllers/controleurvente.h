#ifndef CONTROLEURVENTE_H
#define CONTROLEURVENTE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QList>
#include <QVariantMap>


#include "models/client.h"
#include "models/produit.h"
#include "models/vente.h"
#include "models/modeleclient.h"
#include "models/modeleproduit.h"
#include "models/modelevente.h"


//Pour afficher des information supplémentaire sur les elements du panier
struct AffichageElementPanier : public ComposantVente {
    QString nom_produit;
    QString nom_unite;
};




class ControleurVente : public QObject
{
    Q_OBJECT
public:
    explicit ControleurVente(QObject *parent = 0, ModeleClient* modeleClient = 0, ModeleProduit* modeleProduit = 0, ModeleVente* modeleVente = 0);



    //--------- Gestion client ----------------------
    QList<Client>  rechercherClient(const QString& motCle);
    Client getClientById(int id_client);
    void setClientCourant(const Client& client);
    void viderClient();


    //--------- Gestion produit ----------------------
    QList<Produit> rechercherProduit(const QString& motCle);
    Produit getProduitById(int id_produit);
    QList<UniteVenteProduit> getUniteVenteProduitPourProduit(int id_produit);

    //--------- Gestion panier ----------------------
    bool stockProduitSuffisant(int id_produit, int id_unite, double quantite);
    bool ajouterElementAuPanier(int id_produit, int id_unite, double quantite);
    bool supprimerElementDuPanier(int indexElementPanier);
    const QList<AffichageElementPanier>& getElementsPanier() const;


    //--------- Total et remise ----------------------
    void setPourcentageRemise(double pourcentage);
    double getPourcentageRemise() const;
    void setMontantRemise(double montant);
    double getMontantRemise() const;

    //--------- Finalisation ----------------------
    bool finaliserVente(Vente& donneeVenteComplet);
    bool genererFacture();



    void reinitialiserVente();

signals:
    void totalsAChange(const QVariantMap& totals);

public slots:


private:
    ModeleClient* m_modeleClient;
    ModeleProduit* m_modeleProduit;
    ModeleVente* m_modeleVente;

    Client m_clientCourant;

    double m_pourcentageRemise;
    double m_montantRemiseManuel;

    QList<AffichageElementPanier> m_elementsPanier;


    void calculerEtEmettreTotals();

};

#endif // CONTROLEURVENTE_H
