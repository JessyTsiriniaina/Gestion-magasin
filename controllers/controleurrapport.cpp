#include "controleurrapport.h"

#include <QStringList>
#include <QLocale>

ControleurRapport::ControleurRapport(QObject *parent, ModeleRapport* modeleRapport, ModeleClient* modeleClient, ModeleProduit* modeleProduit) :
    QObject(parent),
    m_modeleRapport(modeleRapport),
    m_modeleClient(modeleClient),
    m_modeleProduit(modeleProduit)
{
    //constructeur
}


QString ControleurRapport::getNomClient(int id_client)
{
    if (id_client <= 0 || !m_modeleClient) return "Client temporaire";
    Client client = m_modeleClient->getClientById(id_client);
    return (client.id_client != -1) ? client.nom_client : "Client temporaire/Inconnu";
}



void ControleurRapport::remplirLigneVente(QStandardItemModel *tableModel, const Vente &vente)
{
    QList<QStandardItem*> elementsLigne;
    elementsLigne << (new QStandardItem(QString::number(vente.id_vente)));
    elementsLigne << (new QStandardItem(vente.date_heure_vente.toString(Qt::DefaultLocaleShortDate)));
    elementsLigne << (new QStandardItem(getNomClient(vente.id_client)));

    QStandardItem* montantTotalElement = new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(vente.montant_total, "Ar"));
    montantTotalElement->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << montantTotalElement;

    QStandardItem* elementRemise = new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(vente.montant_remise, "Ar"));
    elementRemise->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << elementRemise;

    double subTotal = vente.montant_total + vente.montant_remise;
    QStandardItem* subTotalElement = new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(subTotal, "Ar"));
    subTotalElement->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << subTotalElement;

    tableModel->appendRow(elementsLigne);
}



void ControleurRapport::remplirProduitPlusVenduLigne(QStandardItemModel *tableModel, const ProduitPlusVenduInfo &produitInfo)
{
    QList<QStandardItem*> elementsLigne;
    elementsLigne << (new QStandardItem(QString::number(produitInfo.produit.id_produit)));
    elementsLigne << (new QStandardItem(produitInfo.produit.nom_produit));

    QStandardItem* quantiteElement = new QStandardItem(QString::number(produitInfo.totalQuantiteVenduEnUniteBase, 'f', 3));
    quantiteElement->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << quantiteElement;

    elementsLigne << (new QStandardItem(produitInfo.produit.unite_base));

    tableModel->appendRow(elementsLigne);
}



void ControleurRapport::remplirProduitEnRuptureStockLigne(QStandardItemModel *tableModel, const ProduitEnRuptureStockInfo &produitInfo)
{
    QList<QStandardItem*> elementsLigne;
    elementsLigne << (new QStandardItem(QString::number(produitInfo.produit.id_produit)));
    elementsLigne << (new QStandardItem(produitInfo.produit.nom_produit));

    QStandardItem* stockElement = new QStandardItem(QString::number(produitInfo.produit.quantite_stock_en_unite_base, 'f', 3));
    stockElement->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << stockElement;

    elementsLigne << (new QStandardItem(produitInfo.produit.unite_base));

    tableModel->appendRow(elementsLigne);
}


void ControleurRapport::remplirVenteMensuelLigne(QStandardItemModel *tableModel, const QString &moisDeAnnee, double totalVentes)
{
    QList<QStandardItem*> elementsLigne;
    elementsLigne << (new QStandardItem(moisDeAnnee));

    QStandardItem* montantTotalElement = new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(totalVentes, "Ar"));
    montantTotalElement->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    elementsLigne << montantTotalElement;

    tableModel->appendRow(elementsLigne);
}




QList<Client> ControleurRapport::rechercherClient(const QString &motCle)
{
    if (!m_modeleClient) return QList<Client>();
    return m_modeleClient->rechercherClient(motCle);
}


bool ControleurRapport::genererHistoriqueDeVente(const QDate &dateDebut, const QDate &dateFin, QStandardItemModel *tableModel)
{
    if (!m_modeleRapport || !tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID Vente" << "Date" << "Client" << "Montant total" << "Remise" << "Subtotal";
    tableModel->setHorizontalHeaderLabels(headers);

    QList<Vente> ventes = m_modeleRapport->getVenteParPlageDeDate(dateDebut, dateFin);

    foreach (const Vente& vente, ventes) {
        remplirLigneVente(tableModel, vente);
    }

    return true;
}


bool ControleurRapport::genererHistoriqueDeVenteMensuel(int annee, int mois, QStandardItemModel *tableModel)
{
    if (!m_modeleRapport || !tableModel) return false;

    tableModel->clear();

    QStringList headers;
    headers << "Année-Mois" << "Total Vente";
    tableModel->setHorizontalHeaderLabels(headers);

    if (mois > 0 && mois <= 12) {

        QMap<QString, double> rapportMensuel = m_modeleRapport->getVenteMensuel(annee);
        QString moisCible = QString("%1-%2").arg(annee).arg(mois, 2, 10, QChar('0'));
        if (rapportMensuel.contains(moisCible)) {
            remplirVenteMensuelLigne(tableModel, moisCible, rapportMensuel.value(moisCible));
        }
    } else {

        QMap<QString, double> rapportMensuel = m_modeleRapport->getVenteMensuel(annee);

        for(QMap<QString, double>::const_iterator i = rapportMensuel.constBegin(); i != rapportMensuel.constEnd(); ++i) {
            remplirVenteMensuelLigne(tableModel, i.key(), i.value());
        }
    }
    return true;
}


bool ControleurRapport::genererHistoriqueDeVenteParClient(int id_client, const QDate &dateDebut, const QDate &dateFin, QStandardItemModel *tableModel)
{
    if (!m_modeleRapport || !tableModel) return false;
    if (id_client < 0) {
        qWarning("Rapport Controleur: invalide id_client");
        tableModel->clear();
        QStringList headers;
        headers << "Erreur";
        tableModel->setHorizontalHeaderLabels(headers);
        tableModel->appendRow(new QStandardItem("Veuillez selectionner un client valide"));
        return false;
    }

    tableModel->clear();
    QStringList headers;
    headers << "ID Vente" << "Date" << "Client" << "Montant total" << "Remise" << "Subtotal";
    tableModel->setHorizontalHeaderLabels(headers);

    QList<Vente> ventes = m_modeleRapport->getVenteByClientDansPlageDeDate(id_client, dateDebut, dateFin);

    foreach (const Vente& vente, ventes) {
        remplirLigneVente(tableModel, vente);
    }
    return true;
}


bool ControleurRapport::genererRapportProduitPlusVendu(const QDate &dateDebut, const QDate &dateFin, int limite, QStandardItemModel *tableModel)
{
    if (!m_modeleRapport || !tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID Produit" << "Nom produit" << "Total vendu (en unité de base)" << "Unité de base";
    tableModel->setHorizontalHeaderLabels(headers);

    QList<ProduitPlusVenduInfo> produits = m_modeleRapport->getProduitsPlusVendu(dateDebut, dateFin, limite);

    foreach (const ProduitPlusVenduInfo& info, produits) {
        remplirProduitPlusVenduLigne(tableModel, info);
    }
    return true;
}


bool ControleurRapport::genererRapportProduitEnRuptureStock(double seuil, QStandardItemModel *tableModel)
{
    if (!m_modeleRapport || !tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID Produit" << "Nom produit" << "Stock actuel" << "Unité de base";
    tableModel->setHorizontalHeaderLabels(headers);

    QList<ProduitEnRuptureStockInfo> produits = m_modeleRapport->getProduitEnRuptureStock(seuil);

    foreach (const ProduitEnRuptureStockInfo& info, produits) {
        remplirProduitEnRuptureStockLigne(tableModel, info);
    }
    return true;
}
