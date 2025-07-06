#include "modelevente.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

ModeleVente::ModeleVente(QObject *parent, ModeleProduit* modeleProduit) :
    QObject(parent),
    m_modeleProduit(modeleProduit)
{
    //constructeur
}

UniteVenteProduit ModeleVente::getUniteVenteProduitById(int id_unite)
{
    UniteVenteProduit unite;
    if (id_unite <=0) return unite;

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_unite, id_produit, nom_unite, facteur_de_conversion_vers_base, prix_par_unite "
                  "FROM UnitesVenteProduit WHERE id_unite = :id_unite");
    query.bindValue(":id_unite", id_unite);

    if (query.exec() && query.next()) {
        unite.id_unite = query.value("id_unite").toInt();
        unite.id_produit = query.value("id_produit").toInt();
        unite.nom_unite = query.value("nom_unite").toString();
        unite.facteur_de_conversion_vers_base = query.value("facteur_de_conversion_vers_base").toDouble();
        unite.prix_par_unite = query.value("prix_par_unite").toDouble();
    } else if (query.lastError().isValid()) {
        qWarning() << "Echec de recupération de l'unité de vente par ID:" << query.lastError().text();
    }
    return unite;
}


bool ModeleVente::enregistrerVente(Vente& donneeVente, QList<ComposantVente>& composantsVente)
{
    if (!m_modeleProduit) {
        qWarning() << "ModeleVente::ModeleProduit non initialisé, ne peut pas enregistrer une vente";
        return false;
    }
    if (composantsVente.isEmpty()) {
        qWarning() << "Ne peut pas enregistrer une vente avec aucun composant de vente";
        return false;
    }

    QSqlDatabase db = DatabaseManager::database();
    if (!db.transaction()) {
        qWarning() << "Echec lors du debut de transaction:" << db.lastError().text();
        return false;
    }



    //INSERTION DANS LA TABLE DE VENTE


    QSqlQuery venteQuery(db);
    venteQuery.prepare("INSERT INTO Ventes (id_client, date_heure_vente, montant_total, montant_remise) "
                      "VALUES (:id_client, :date_heure_vente, :montant_total, :montant_remise)");
    venteQuery.bindValue(":id_client", donneeVente.id_client > 0 ? QVariant(donneeVente.id_client) : QVariant(QVariant::Int));
    venteQuery.bindValue(":date_heure_vente", donneeVente.date_heure_vente.toString(Qt::ISODate));
    venteQuery.bindValue(":montant_total", donneeVente.montant_total);
    venteQuery.bindValue(":montant_remise", donneeVente.montant_remise);

    if (!venteQuery.exec()) {
        qWarning() << "Echec lors de l'insertion dans la table vente:" << venteQuery.lastError().text();
        db.rollback();
        return false;
    }
    donneeVente.id_vente = venteQuery.lastInsertId().toInt();



    //INSERTION DANS LA TABLE ComposantsVente et mettre à jour le stock de chaque composant

    for (int i = 0; i < composantsVente.size(); i++) {
        ComposantVente& composant = composantsVente[i]; // Accès par reference pour modifier l'original
        composant.id_vente = donneeVente.id_vente;

        UniteVenteProduit uniteVente = getUniteVenteProduitById(composant.id_unite_vente);
        if (uniteVente.id_unite == -1) {
            qWarning() << "Echec lors de la recuperation de l'unité de vente: " << composant.id_unite_vente;
            db.rollback();
            return false;
        }

        if (composant.id_produit == 0 || composant.id_produit == -1) {
            composant.id_produit = uniteVente.id_produit;
        } else if (composant.id_produit != uniteVente.id_produit) {
             qWarning() << "Composant.id_produit et UniteVenteProduit.id_produit ne correspondent pas";
             db.rollback();
             return false;
        }
        if (composant.id_produit <= 0) {
            qWarning() << "Identifiant produit invalide pour le composant de vente";
            db.rollback();
            return false;
        }


        QSqlQuery composantQuery(db);
        composantQuery.prepare("INSERT INTO ComposantsVente (id_vente, id_produit, id_unite_vente, quantite_vendu, prix_de_vente, subtotal_composant) "
                          "VALUES (:id_vente, :id_produit, :id_unite_vente, :quantite_vendu, :prix_de_vente, :subtotal_composant)");
        composantQuery.bindValue(":id_vente", composant.id_vente);
        composantQuery.bindValue(":id_produit", composant.id_produit);
        composantQuery.bindValue(":id_unite_vente", composant.id_unite_vente);
        composantQuery.bindValue(":quantite_vendu", composant.quantite_vendu);
        composantQuery.bindValue(":prix_de_vente", composant.prix_de_vente);
        composantQuery.bindValue(":subtotal_composant", composant.subtotal_composant);

        if (!composantQuery.exec()) {
            qWarning() << "Echec d'insertion de composant de vente: " << composantQuery.lastError().text();
            db.rollback();
            return false;
        }
        composant.id_unite_vente = composantQuery.lastInsertId().toInt(); // Mise à jour de l'identifiant dans la liste original

        // Mise à jour de stock
        double quantiteEnUniteDeBase = composant.quantite_vendu * uniteVente.facteur_de_conversion_vers_base;
        double stockActuel = m_modeleProduit->getStockProduitEnUniteDeBase(composant.id_produit);
        double nouveauStock = stockActuel - quantiteEnUniteDeBase;

        if (!m_modeleProduit->reapprovisionnerStockProduit(composant.id_produit, nouveauStock)) {
            qWarning() << "Echec de mise à jour de stock du produit ID:" << composant.id_produit;
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qWarning() << "Echec de commit de transaction: " << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;

}


Vente ModeleVente::getVenteById(int id_vente)
{
    Vente vente;
    if (id_vente <= 0) return vente;

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise "
                  "FROM Ventes WHERE id_vente = :id_vente");
    query.bindValue(":id_vente", id_vente);

    if (query.exec() && query.next()) {
        vente.id_vente = query.value("id_vente").toInt();
        vente.id_client = query.value("id_client").isNull() ? -1 : query.value("id_client").toInt();
        vente.date_heure_vente = QDateTime::fromString(query.value("date_heure_vente").toString(), Qt::ISODate);
        vente.montant_total = query.value("montant_total").toDouble();
        vente.montant_remise = query.value("montant_remise").toDouble();

    } else if (query.lastError().isValid()) {
        qWarning() << "Echec de recuperation de la vente: " << query.lastError().text();
    }
    return vente;
}


QList<ComposantVente> ModeleVente::getComposantsVentePourVente(int id_vente)
{
    QList<ComposantVente> composants;
    if (id_vente <= 0) return composants;

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_composant_vente, id_vente, id_produit, id_unite_vente, quantite_vendu, prix_de_vente, subtotal_composant "
                  "FROM ComposantsVente WHERE id_vente = :id_vente");
    query.bindValue(":id_vente", id_vente);

    if (query.exec()) {
        while (query.next()) {
            ComposantVente composant;
            composant.id_composant_vente = query.value("id_composant_vente").toInt();
            composant.id_vente = query.value("id_vente").toInt();
            composant.id_produit = query.value("id_produit").toInt();
            composant.id_unite_vente = query.value("id_unite_vente").toInt();
            composant.quantite_vendu = query.value("quantite_vendu").toDouble();
            composant.prix_de_vente = query.value("prix_de_vente").toDouble();
            composant.subtotal_composant = query.value("subtotal_composant").toDouble();
            composants.append(composant);
        }
    } else {
        qWarning() << "Echec de recuperation de composant de vente pour la vente ID " << id_vente << ":" << query.lastError().text();
    }
    return composants;
}


QList<Vente> ModeleVente::getAllVente()
{
    QList<Vente> ventes;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise "
                  "FROM Ventes ORDER BY date_heure_vente DESC");

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.id_vente = query.value("id_vente").toInt();
            vente.id_client = query.value("id_client").isNull() ? -1 : query.value("id_client").toInt();
            vente.date_heure_vente = QDateTime::fromString(query.value("date_heure_vente").toString(), Qt::ISODate);
            vente.montant_total = query.value("montant_total").toDouble();
            vente.montant_remise = query.value("montant_remise").toDouble();
            ventes.append(vente);
        }
    } else {
        qWarning() << "Echec de recuperation de tous les ventes: " << query.lastError().text();
    }
    return ventes;
}


QList<Vente> ModeleVente::getVenteParPlageDeDate(const QDateTime &dateDebut, const QDateTime &dateFin)
{
    QList<Vente> ventes;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise FROM Ventes "
                  "WHERE date(date_heure_vente) >= date(:dateDebut) AND date(date_heure_vente) <= date(:dateFin) ORDER BY date_heure_vente DESC");
    query.bindValue(":dateDebut", dateDebut.toString(Qt::ISODate));
    query.bindValue(":dateFin", dateFin.toString(Qt::ISODate));

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.id_vente = query.value("id_vente").toInt();
            vente.id_client = query.value("id_client").isNull() ? -1 : query.value("id_client").toInt();
            vente.date_heure_vente = QDateTime::fromString(query.value("date_heure_vente").toString(), Qt::ISODate);
            vente.montant_total = query.value("montant_total").toDouble();
            vente.montant_remise = query.value("montant_remise").toDouble();
            ventes.append(vente);
        }
    } else {
        qWarning() << "Echec de recuperation de vente par plage d'adresse: " << query.lastError().text();
    }
    return ventes;
}


QList<Vente> ModeleVente::getVenteByClient(int id_client)
{
    QList<Vente> ventes;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise FROM Ventes "
                  "WHERE id_client = :id_client ORDER BY date_heure_vente DESC");
    query.bindValue(":id_client", id_client);

    if (query.exec()) {
        while (query.next()) {
            Vente vente;
            vente.id_vente = query.value("id_vente").toInt();
            vente.id_client = query.value("id_client").toInt();
            vente.date_heure_vente = QDateTime::fromString(query.value("date_heure_vente").toString(), Qt::ISODate);
            vente.montant_total = query.value("montant_total").toDouble();
            vente.montant_remise = query.value("montant_remise").toDouble();
            ventes.append(vente);
        }
    } else {
        qWarning() << "Echec de recuperation de vente par client: " << query.lastError().text();
    }
    return ventes;
}
