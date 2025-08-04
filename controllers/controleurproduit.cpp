#include "controleurproduit.h"
#include <QDebug>

ControleurProduit::ControleurProduit(QObject *parent, ModeleProduit* modeleProduit) :
    QObject(parent),
    m_modeleProduit(modeleProduit)
{
    if(!m_modeleProduit) {
        qCritical() << "Modele produit non initialisé, fonctionnalité incomplet";
    }
}

//----------- PRODUIT -------------

void ControleurProduit::remplirLigneProduit(QStandardItemModel *tableModel, const Produit &produit)
{
    QList<QStandardItem*> ligneItems;

    QStandardItem* idItem = new QStandardItem(QString::number(produit.id_produit));
    idItem->setData(produit.id_produit, Qt::UserRole + 1); //stocker l'ID dans UserRole + 1
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);//Faire en sorte que l'id n'est pas modifiable
    idItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QStandardItem* nomItem = new QStandardItem(produit.nom_produit);
    nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);
    nomItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QStandardItem* quantiteStockItem = new QStandardItem(QString::number(produit.quantite_stock_en_unite_base, 'f', 3));
    quantiteStockItem->setFlags(quantiteStockItem->flags() & ~Qt::ItemIsEditable);
    quantiteStockItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QStandardItem* uniteBaseItem = new QStandardItem(produit.unite_base);
    uniteBaseItem->setFlags(uniteBaseItem->flags() & ~Qt::ItemIsEditable);
    uniteBaseItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QStandardItem* remiseItem = new QStandardItem(QString(QString::number(produit.remise_pourcentage, 'f', 2) + "%"));
    remiseItem->setFlags(remiseItem->flags() & ~Qt::ItemIsEditable);
    remiseItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);


    ligneItems << idItem << nomItem << uniteBaseItem  << remiseItem << quantiteStockItem ;
    tableModel->appendRow(ligneItems);
}


bool ControleurProduit::chargerProduitsDansTableModel(QStandardItemModel* tableModel, const QString& motCle)
{
    if (!m_modeleProduit) return false;
    if (!tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID" << "Nom de produit" << "Unité de base" << "Remise"  << "Quantité en stock (en unité de base)";
    tableModel->setHorizontalHeaderLabels(headers);


    QList<Produit> produits;
    if (motCle.isEmpty()) {
        produits = m_modeleProduit->getAllProduit();
    } else {
        produits = m_modeleProduit->rechercherProduit(motCle);
    }


    foreach (const Produit& produit, produits) {
         remplirLigneProduit(tableModel, produit);
    }
    return true;
}

Produit ControleurProduit::getProduitDepuisModel(int id_produit)
{
    if (!m_modeleProduit) return Produit(); // Return invalied produit
    return m_modeleProduit->getProduitById(id_produit);
}

QList<UniteVenteProduit> ControleurProduit::getUniteVenteProduitPourProduit(int id_produit)
{
    if (!m_modeleProduit) return QList<UniteVenteProduit>();
    return m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
}

bool ControleurProduit::ajouterProduit(Produit& donneeProduit, QList<UniteVenteProduit>& unites)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->ajouterProduit(donneeProduit, unites);
}

bool ControleurProduit::modifierProduit(const Produit& donneeProduit)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->modifierProduit(donneeProduit);
}

bool ControleurProduit::supprimerProduit(int id_produit)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->supprimerProduit(id_produit);
}

bool ControleurProduit::nomProduitExiste(QString nom, int id_produit) {
    return m_modeleProduit->nomProduitExiste(nom, id_produit);
}




//------------ UNITE DE VENTE ----------------------

void ControleurProduit::remplirLigneUniteVente(QStandardItemModel* tableModel, const UniteVenteProduit& unite)
{
    QList<QStandardItem*> ligneItems;
    QStandardItem* idItem = new QStandardItem(QString::number(unite.id_unite));
    idItem->setData(unite.id_unite, Qt::UserRole + 1); // stocker l'ID
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

    QStandardItem* nomItem = new QStandardItem(unite.nom_unite);
    nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);

    QStandardItem* facteurConversionItem = new QStandardItem(QString::number(unite.facteur_de_conversion_vers_base, 'f', 4));
    facteurConversionItem->setFlags(facteurConversionItem->flags() & ~Qt::ItemIsEditable);
    facteurConversionItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QStandardItem* prixUniteItem = new QStandardItem(QString::number(unite.prix_par_unite, 'f', 2));
    prixUniteItem->setFlags(prixUniteItem->flags() & ~Qt::ItemIsEditable);
    prixUniteItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ligneItems << idItem << nomItem << facteurConversionItem << prixUniteItem ;
    tableModel->appendRow(ligneItems);
}


bool ControleurProduit::chargerUnitesVenteProduitDansTableModel(QStandardItemModel* tableModel, int id_produit)
{
    if (!m_modeleProduit) return false;
    if (!tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID" << "Nom" << "Facteur de conversion vers base" << "Prix par unité";
    tableModel->setHorizontalHeaderLabels(headers);

    QList<UniteVenteProduit> unites = m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);

    foreach (const UniteVenteProduit& unite, unites) {
        remplirLigneUniteVente(tableModel, unite);
    }
    return true;
}


bool ControleurProduit::ajouterUniteVenteProduit(UniteVenteProduit& donneeUnite)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->ajouterUniteVenteProduit(donneeUnite);
}

bool ControleurProduit::modifierUniteVenteProduit(const UniteVenteProduit& donneeUnite)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->modifierUniteVenteProduit(donneeUnite);
}

bool ControleurProduit::supprimerUniteVenteProduit(int id_unite)
{
    if (!m_modeleProduit) return false;
    return m_modeleProduit->supprimerUniteVenteProduit(id_unite);
}


//-------------- STOCK --------------------

bool ControleurProduit::reapprovisionnerStockProduit(int id_produit, double nouveauQuantiteStockEnUniteDeBase)
{
    if (!m_modeleProduit) return false;
    if (nouveauQuantiteStockEnUniteDeBase <= 0) {
        qWarning() << "Quantité à ajouter doit être positive.";
        return false;
    }

    double stockActuel = m_modeleProduit->getStockProduitEnUniteDeBase(id_produit);
    if (id_produit == -1 && stockActuel == 0.0) {

        Produit p = m_modeleProduit->getProduitById(id_produit);
        if(p.id_produit == -1) {
            qWarning() << "Produit introuvable pour le reapprovisionnement:" << id_produit;
            return false;
        }
    }

    double nouveauStock = stockActuel + nouveauQuantiteStockEnUniteDeBase;
    return m_modeleProduit->reapprovisionnerStockProduit(id_produit, nouveauStock);
}

double ControleurProduit::getStockProduitEnUniteDeBase(int id_produit)
{
    if(!m_modeleProduit) return 0.0;
    return m_modeleProduit->getStockProduitEnUniteDeBase(id_produit);
}
























