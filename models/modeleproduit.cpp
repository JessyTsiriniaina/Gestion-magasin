#include "modeleproduit.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

ModeleProduit::ModeleProduit(QObject *parent) :
    QObject(parent)
{
    //constructeur
}

bool ModeleProduit::nomProduitExiste(const QString& nom, int idProduitCourant)
{
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_produit FROM Produits WHERE nom_produit = :nom AND id_produit != :idProduitCourant");
    query.bindValue(":nom", nom);
    query.bindValue(":idProduitCourant", idProduitCourant);

    if (!query.exec() && query.next()) {
       return true;
    }
    return false;
}


bool ModeleProduit::ajouterProduit(Produit &donneeProduit, QList<UniteVenteProduit> &unites)
{

    if (nomProduitExiste(donneeProduit.nom_produit)) {
        qWarning() << "Echec. Nom produit" << donneeProduit.nom_produit << "existe déjà.";
        return false;
    }

    QSqlDatabase db = DatabaseManager::database();
    if (!db.transaction()) {
        qWarning() << "Debut de transaction echoué:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Produits (nom_produit, unite_base, quantite_stock_en_unite_base) "
                  "VALUES (:nom_produit, :unite_base, :quantite_stock_en_unite_base)");
    query.bindValue(":nom_produit", donneeProduit.nom_produit);
    query.bindValue(":unite_base", donneeProduit.unite_base);
    query.bindValue(":quantite_stock_en_unite_base", donneeProduit.quantite_stock_en_unite_base);

    if (!query.exec()) {
        qWarning() << "Echec d'ajout du produit:" << query.lastError().text();
        db.rollback();
        return false;
    }

    donneeProduit.id_produit = query.lastInsertId().toInt();

    //Maintenant, on va inserer les unités de vente associés à la base de données aussi
    for (int i = 0; i < unites.size(); i++) {
        UniteVenteProduit& unite = unites[i]; // accès par reférence pour modifier l'original ( et non une copie )
        unite.id_produit = donneeProduit.id_produit;
        QSqlQuery uniteQuery(db);
        uniteQuery.prepare("INSERT INTO UnitesVenteProduit (id_produit, nom_unite, facteur_de_conversion_vers_base, prix_par_unite) "
                          "VALUES (:id_produit, :nom_unite, :facteur_de_conversion_vers_base, :prix_par_unite)");
        uniteQuery.bindValue(":id_produit", unite.id_produit);
        uniteQuery.bindValue(":nom_unite", unite.nom_unite);
        uniteQuery.bindValue(":facteur_de_conversion_vers_base", unite.facteur_de_conversion_vers_base);
        uniteQuery.bindValue(":prix_par_unite", unite.prix_par_unite);

        if (!uniteQuery.exec()) {
            qWarning() << "Echec lors de l'ajout de l'unité de vente" << unite.nom_unite << ":" << uniteQuery.lastError().text();
            db.rollback();
            return false;
        }
        unite.id_unite = uniteQuery.lastInsertId().toInt(); // Mettre à jour l'ID dans la liste original
    }




    if (!db.commit()) {
        qWarning() << "Echec lors du commit des transactions dans l'ajout de produit:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;

}


bool ModeleProduit::modifierProduit(const Produit& donneeProduit)
{
    if (donneeProduit.id_produit <= 0) {
        qWarning() << "Echec. ID produit invalide.";
        return false;
    }
    if (nomProduitExiste(donneeProduit.nom_produit, donneeProduit.id_produit)) {
        qWarning() << "Echec de modification. " << donneeProduit.nom_produit << "exite déjà";
        return false;
    }

    QSqlQuery query(DatabaseManager::database());
    query.prepare("UPDATE Produits SET nom_produit = :nom_produit, unite_base = :unite_base, "
                  "quantite_stock_en_unite_base = :quantite_stock_en_unite_base "
                  "WHERE id_produit = :id_produit");
    query.bindValue(":nom_produit", donneeProduit.nom_produit);
    query.bindValue(":unite_base", donneeProduit.unite_base);
    query.bindValue(":quantite_stock_en_unite_base", donneeProduit.quantite_stock_en_unite_base);
    query.bindValue(":id_produit", donneeProduit.id_produit);

    if (!query.exec()) {
        qWarning() << "Echec de modification:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}


bool ModeleProduit::supprimerProduit(int id_produit)
{

    QSqlDatabase db = DatabaseManager::database();

    if (!db.transaction()) {
        qWarning() << "Debut de transaction echoué:" << db.lastError().text();
        return false;
    }

    QSqlQuery supprUniteQuery(db);
    supprUniteQuery.prepare("DELETE FROM UnitesVenteProduit WHERE id_produit = :id_produit");
    supprUniteQuery.bindValue(":id_produit", id_produit);

    if(!supprUniteQuery.exec()) {
        qWarning() << "Echec de suppression des unité de vente associés: " << supprUniteQuery.lastError().text();
        return false;
    }


    QSqlQuery query(db);
    query.prepare("DELETE FROM Produits WHERE id_produit = :id_produit");
    query.bindValue(":id_produit", id_produit);

    if (!query.exec()) {
        qWarning() << "Echec de suppression de produit: " << query.lastError().text();
        return false;
    }

    int ligne_modifiee = query.numRowsAffected();

    if (!db.commit()) {
        qWarning() << "Echec lors du commit des transactions dans la suppression de produit:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return ligne_modifiee > 0;
}


Produit ModeleProduit::getProduitById(int id_produit)
{
    Produit produit;
    if (id_produit <= 0) return produit; // Invalide produit

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_produit, nom_produit, unite_base, quantite_stock_en_unite_base "
                  "FROM Produits WHERE id_produit = :id_produit");
    query.bindValue(":id_produit", id_produit);

    if (query.exec() && query.next()) {
        produit.id_produit = query.value("id_produit").toInt();
        produit.nom_produit = query.value("nom_produit").toString();
        produit.unite_base = query.value("unite_base").toString();
        produit.quantite_stock_en_unite_base = query.value("quantite_stock_en_unite_base").toDouble();
    } else if (query.lastError().isValid()) {
        qWarning() << "Echec de recuperation du produit:" << query.lastError().text();
    }
    return produit;
}


QList<Produit> ModeleProduit::getAllProduit()
{
    QList<Produit> produits;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT * FROM Produits ORDER BY nom_produit ASC");

    if (query.exec()) {
        while (query.next()) {
            Produit produit;
            produit.id_produit = query.value("id_produit").toInt();
            produit.nom_produit = query.value("nom_produit").toString();
            produit.unite_base = query.value("unite_base").toString();
            produit.quantite_stock_en_unite_base = query.value("quantite_stock_en_unite_base").toDouble();
            produits.append(produit);
        }
    } else {
        qWarning() << "Echec de recuperation de tous les produits:" << query.lastError().text();
    }
    return produits;
}


bool ModeleProduit::ajouterUniteVenteProduit(UniteVenteProduit &donneeUnite)
{

    if (donneeUnite.id_produit <= 0) {
         qWarning() << "id_produit invalide pour ajouter un unité de vente.";
         return false;
    }


    QSqlQuery query(DatabaseManager::database());
    query.prepare("INSERT INTO UnitesVenteProduit (id_produit, nom_unite, facteur_de_conversion_vers_base, prix_par_unite) "
                  "VALUES (:id_produit, :nom_unite, :facteur_de_conversion_vers_base, :prix_par_unite)");
    query.bindValue(":id_produit", donneeUnite.id_produit);
    query.bindValue(":nom_unite", donneeUnite.nom_unite);
    query.bindValue(":facteur_de_conversion_vers_base", donneeUnite.facteur_de_conversion_vers_base);
    query.bindValue(":prix_par_unite", donneeUnite.prix_par_unite);

    if (!query.exec()) {
        qWarning() << "Echec lors de l'ajout de l'unité de mesure:" << query.lastError().text();
        return false;
    }
    donneeUnite.id_unite = query.lastInsertId().toInt();
    return true;
}


bool ModeleProduit::modifierUniteVenteProduit(const UniteVenteProduit& donneeUnite)
{
    if (donneeUnite.id_unite <= 0) {
        qWarning() << "id_unite invalide pour modification.";
        return false;
    }

    QSqlQuery query(DatabaseManager::database());
    query.prepare("UPDATE UnitesVenteProduit SET nom_unite = :nom_unite, "
                  "facteur_de_conversion_vers_base = :facteur_de_conversion_vers_base, prix_par_unite = :prix_par_unite "
                  "WHERE id_unite = :id_unite");
    query.bindValue(":nom_unite", donneeUnite.nom_unite);
    query.bindValue(":facteur_de_conversion_vers_base", donneeUnite.facteur_de_conversion_vers_base);
    query.bindValue(":prix_par_unite", donneeUnite.prix_par_unite);
    query.bindValue(":id_unite", donneeUnite.id_unite);

    if (!query.exec()) {
        qWarning() << "Echec de modification de l'unité de mesure:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}


bool ModeleProduit::supprimerUniteVenteProduit(int id_unite)
{
    QSqlQuery query(DatabaseManager::database());
    query.prepare("DELETE FROM UnitesVenteProduit WHERE id_unite = :id_unite");
    query.bindValue(":id_unite", id_unite);

    if (!query.exec()) {
        qWarning() << "Echec de suppression de l'unite de mesure " << id_unite << ":" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}


QList<UniteVenteProduit> ModeleProduit::getUniteVenteProduitPourProduit(int id_produit)
{
    QList<UniteVenteProduit> unites;
    if (id_produit <= 0) return unites; //unites vide

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_unite, id_produit, nom_unite, facteur_de_conversion_vers_base, prix_par_unite "
                  "FROM UnitesVenteProduit WHERE id_produit = :id_produit ORDER BY nom_unite ASC");
    query.bindValue(":id_produit", id_produit);

    if (query.exec()) {
        while (query.next()) {
            UniteVenteProduit unite;
            unite.id_unite = query.value("id_unite").toInt();
            unite.id_produit = query.value("id_produit").toInt();
            unite.nom_unite = query.value("nom_unite").toString();
            unite.facteur_de_conversion_vers_base = query.value("facteur_de_conversion_vers_base").toDouble();
            unite.prix_par_unite = query.value("prix_par_unite").toDouble();
            unites.append(unite);
        }
    } else {
        qWarning() << "Echec de recuperation des unites de mesure du produit" << query.lastError().text();
    }
    return unites;
}


double ModeleProduit::getStockProduitEnUniteDeBase(int id_produit)
{
    double stock = 0.0;
    if (id_produit <= 0) return stock;

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT quantite_stock_en_unite_base FROM Produits WHERE id_produit = :id_produit");
    query.bindValue(":id_produit", id_produit);

    if (query.exec() && query.next()) {
        stock = query.value(0).toDouble();
    } else if (query.lastError().isValid()) {
        qWarning() << "Echec de recuperation de quantité de stock pour le produit: " << id_produit << ":" << query.lastError().text();
    }
    return stock;
}


bool ModeleProduit::reapprovisionnerStockProduit(int id_produit, double nouveauQuantiteStockEnUniteDeBase)
{
    if (id_produit <= 0) {
        qWarning() << "Invalide ID produit pour la mise à jour de stock.";
        return false;
    }
    QSqlQuery query(DatabaseManager::database());
    query.prepare("UPDATE Produits SET quantite_stock_en_unite_base = :quantite_stock_en_unite_base "
                  "WHERE id_produit = :id_produit");
    query.bindValue(":quantite_stock_en_unite_base", nouveauQuantiteStockEnUniteDeBase);
    query.bindValue(":id_produit", id_produit);

    if (!query.exec()) {
        qWarning() << "Echec de mise à jour de stock produit:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}



QList<Produit> ModeleProduit::rechercherProduit(const QString &motCle)
{
    QList<Produit> produits;
    QSqlQuery query(DatabaseManager::database());

    QString motSimilaire = "%" + motCle + "%";
    query.prepare("SELECT id_produit, nom_produit, unite_base, quantite_stock_en_unite_base FROM Produits "
                  "WHERE nom_produit LIKE :motCle ");
    query.bindValue(":motCle", motSimilaire);

    if (query.exec()) {
        while (query.next()) {
            Produit produit;
            produit.id_produit = query.value("id_produit").toInt();
            produit.nom_produit = query.value("nom_produit").toString();
            produit.unite_base = query.value("unite_base").toString();
            produit.quantite_stock_en_unite_base = query.value("quantite_stock_en_unite_base").toDouble();
            produits.append(produit);
        }
    } else {
        qWarning() << "Echec de recherhe produit: " << query.lastError().text();
    }
    return produits;
}










