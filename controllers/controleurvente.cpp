#include "controleurvente.h"


#include <QDebug>


ControleurVente::ControleurVente(QObject *parent, ModeleClient* modeleClient, ModeleProduit* modeleProduit, ModeleVente* modeleVente) :
    QObject(parent),
    m_modeleClient(modeleClient),
    m_modeleProduit(modeleProduit),
    m_modeleVente(modeleVente),
    m_pourcentageRemise(0.0),
    m_montantRemiseManuel(0.0)
{
    if(!m_modeleClient || !m_modeleProduit || !m_modeleVente) {
        qCritical() << "Le controleur vente manque de un ou plusieurs modele, la fonctionnalité est incomplet";
    }

    reinitialiserVente();

}



//--------- Gestion client ----------------------

QList<Client>  ControleurVente::rechercherClient(const QString& motCle)
{
    if (!m_modeleClient) return QList<Client>();
    return m_modeleClient->rechercherClient(motCle);
}

Client ControleurVente::getClientById(int id_client)
{
    if (!m_modeleClient) return Client();
    return m_modeleClient->getClientById(id_client);
}

void ControleurVente::setClientCourant(const Client& client)
{
    m_clientCourant = client;
}

void ControleurVente::viderClient()
{
    m_clientCourant = Client();
    m_clientCourant.id_client = -1;
}




//--------- Gestion produit ----------------------

QList<Produit> ControleurVente::rechercherProduit(const QString& motCle)
{
    if(!m_modeleProduit) return QList<Produit>();
    return m_modeleProduit->rechercherProduit(motCle);
}

Produit ControleurVente::getProduitById(int id_produit)
{
    if (!m_modeleProduit) return Produit();
    return m_modeleProduit->getProduitById(id_produit);
}

QList<UniteVenteProduit> ControleurVente::getUniteVenteProduitPourProduit(int id_produit)
{
    if (!m_modeleProduit) return QList<UniteVenteProduit>();
    return m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
}





//--------- Gestion panier ----------------------

bool ControleurVente::stockProduitSuffisant(int id_produit, int id_unite, double quantite){


    Produit produit = m_modeleProduit->getProduitById(id_produit);
    if (produit.id_produit == -1) {
        qWarning() << "Produit introuvable:" << id_produit;
        return false;
    }


    UniteVenteProduit unite;
    QList<UniteVenteProduit> unites = m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
    bool uniteTrouve = false;

    foreach (const UniteVenteProduit& u, unites) {
        if (u.id_unite == id_unite) {
            unite = u;
            uniteTrouve = true;
            break;
        }
    }

    if (!uniteTrouve) {
        qWarning() << "Unité introuvable:" << id_unite << "pour le produit ID:" << id_produit;
        return false;
    }

    double stockDemandeEnUniteBase = quantite * unite.facteur_de_conversion_vers_base;
    if (produit.quantite_stock_en_unite_base < stockDemandeEnUniteBase) {
        return false;
    }

    return  true;
}

bool ControleurVente::ajouterElementAuPanier(int id_produit, int id_unite, double quantite)
{
    if (!m_modeleProduit) return false;
    if (quantite <= 0) {
        qWarning() << "Quantité doit être positif.";
        return false;
    }

    Produit produit = m_modeleProduit->getProduitById(id_produit);
    if (produit.id_produit == -1) {
        qWarning() << "Produit introuvable:" << id_produit;
        return false;
    }

    UniteVenteProduit unite;
    QList<UniteVenteProduit> unites = m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
    bool uniteTrouve = false;

    foreach (const UniteVenteProduit& u, unites) {
        if (u.id_unite == id_unite) {
            unite = u;
            uniteTrouve = true;
            break;
        }
    }

    if (!uniteTrouve) {
        qWarning() << "Unité introuvable:" << id_unite << "pour le produit ID:" << id_produit;
        return false;
    }

    double stockDemandeEnUniteBase = quantite * unite.facteur_de_conversion_vers_base;
    if (produit.quantite_stock_en_unite_base < stockDemandeEnUniteBase) {
        return false;
    }

    AffichageElementPanier elementPanier;
    elementPanier.id_produit = id_produit;
    elementPanier.id_unite_vente = id_unite;
    elementPanier.quantite_vendu = quantite;
    elementPanier.prix_de_vente = unite.prix_par_unite;
    elementPanier.subtotal_composant = unite.prix_par_unite * quantite;


    elementPanier.nom_produit = produit.nom_produit;
    elementPanier.nom_unite = unite.nom_unite;

    m_elementsPanier.append(elementPanier);
    calculerEtEmettreTotals();
    //emit panierAChange();
    return true;
}

bool ControleurVente::supprimerElementDuPanier(int indexElementPanier)
{
    if (indexElementPanier < 0 || indexElementPanier >= m_elementsPanier.size()) {
        return false;
    }
    m_elementsPanier.removeAt(indexElementPanier);
    calculerEtEmettreTotals();
    //emit panierAChange();
    return true;
}


const QList<AffichageElementPanier>& ControleurVente::getElementsPanier() const
{
    return m_elementsPanier;
}



//--------- Total et remise ----------------------

void ControleurVente::calculerEtEmettreTotals()
{
    double subtotal = 0.0;

    foreach (const AffichageElementPanier& element, m_elementsPanier) {
        subtotal += element.subtotal_composant;
    }

    double montantRemiseActuel = 0.0;
    if (m_pourcentageRemise > 0) {
        montantRemiseActuel = (subtotal * m_pourcentageRemise) / 100.0;
        m_montantRemiseManuel = montantRemiseActuel; //Mettre à jour le montant manuel si le pourcentage est utilisé
    } else {
        montantRemiseActuel = m_montantRemiseManuel;
        // S'assurer que le montant de remise ne depasse pas le subtotal
        if (montantRemiseActuel > subtotal) montantRemiseActuel = subtotal;
        if (montantRemiseActuel < 0) montantRemiseActuel = 0;
        m_montantRemiseManuel = montantRemiseActuel; // mise à jour
    }

    double grandTotal = subtotal - montantRemiseActuel;

    QVariantMap totals;
    totals["subtotal"] = subtotal;
    totals["montantRemise"] = montantRemiseActuel;
    totals["grandTotal"] = grandTotal;
    totals["pourcentageRemise"] = m_pourcentageRemise;

    emit totalsAChange(totals);
}



void ControleurVente::setPourcentageRemise(double pourcentage)
{
    if (pourcentage < 0) pourcentage = 0;
    if (pourcentage > 100) pourcentage = 100;
    m_pourcentageRemise = pourcentage;
    m_montantRemiseManuel = 0;
    calculerEtEmettreTotals();
}

double ControleurVente::getPourcentageRemise() const
{
    return m_pourcentageRemise;
}

void ControleurVente::setMontantRemise(double montant)
{

    if (m_pourcentageRemise == 0) {
        if (montant < 0) montant = 0;
        m_montantRemiseManuel = montant;

        calculerEtEmettreTotals();
    } else {

        qDebug() << "Ne peut pas éditer manualement le montant de remise si le pourcentage est activé";

        calculerEtEmettreTotals();
    }
}

double ControleurVente::getMontantRemise() const
{

    double subtotal = 0.0;

    foreach (const AffichageElementPanier& element, m_elementsPanier) {
        subtotal += element.subtotal_composant;
    }

    if (m_pourcentageRemise > 0) {
        return qRound(((subtotal * m_pourcentageRemise) / 100.0) * 100.0) / 100.0;
    }

    return qMin(m_montantRemiseManuel, subtotal);
}



//--------- Finalisation ----------------------

bool ControleurVente::finaliserVente(Vente& donneeVenteComplet)
{
    if (!m_modeleVente) return false;
    if (m_elementsPanier.isEmpty()) {
        qWarning() << "Ne peut pas finaliser une vente vide";
        return false;
    }

    Vente enregistrementVente;
    enregistrementVente.id_client = (m_clientCourant.id_client > 0) ? m_clientCourant.id_client : -1;
    enregistrementVente.date_heure_vente = QDateTime::currentDateTime();

    double subtotal = 0.0;

    foreach(const AffichageElementPanier& elementPanier, m_elementsPanier) {
        subtotal += elementPanier.subtotal_composant;
    }

    double montantRemiseFinal = getMontantRemise();
    enregistrementVente.montant_total = subtotal - montantRemiseFinal;
    enregistrementVente.montant_remise = montantRemiseFinal;


    QList<ComposantVente> composantsVente;

    foreach (const AffichageElementPanier& elementPanier, m_elementsPanier) {
        ComposantVente composant;
        composant.id_produit = elementPanier.id_produit;
        composant.id_unite_vente = elementPanier.id_unite_vente;
        composant.quantite_vendu = elementPanier.quantite_vendu;
        composant.prix_de_vente = elementPanier.prix_de_vente;
        composant.subtotal_composant = elementPanier.subtotal_composant;
        composantsVente.append(composant);
    }

    if (m_modeleVente->enregistrerVente(enregistrementVente, composantsVente)) {
        donneeVenteComplet = enregistrementVente;

        return true;
    }
    return false;
}


bool ControleurVente::genererFacture()
{

}






void ControleurVente::reinitialiserVente()
{
    m_elementsPanier.clear();
    viderClient();
    m_pourcentageRemise = 0.0;
    m_montantRemiseManuel = 0.0;

    calculerEtEmettreTotals();
}
