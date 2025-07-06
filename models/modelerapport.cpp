#include "modelerapport.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

ModeleRapport::ModeleRapport(QObject *parent) :
    QObject(parent)
{
}

QDateTime ModeleRapport::dateVersDateTimeDebut(const QDate &date)
{
    return QDateTime(date, QTime(0, 0, 0));
}

QDateTime ModeleRapport::dateVersDateTimeFin(const QDate &date)
{
    return QDateTime(date, QTime(23, 59, 59, 999));
}


QList<Vente> ModeleRapport::getVenteParPlageDeDate(const QDate &dateDebut, const QDate &dateFin)
{
    QList<Vente> ventes;

    if (!dateDebut.isValid() || !dateFin.isValid() || dateDebut > dateFin) {
        qWarning() << "Plage de date invalide";
        return ventes;
    }
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise FROM Ventes "
                  "WHERE date(date_heure_vente) >= date(:dateDebut) AND date(date_heure_vente) <= date(:dateFin) " // Utiliser date() pour SQLite comparaison
                  "ORDER BY date_heure_vente DESC");
    query.bindValue(":dateDebut", dateVersDateTimeDebut(dateDebut).toString(Qt::ISODate)); // Bind date et heure complet pour la securité, query utilise date()
    query.bindValue(":dateFin", dateVersDateTimeFin(dateFin).toString(Qt::ISODate));

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
        qWarning() << "Erreur de recuperation de vente par plage de date:" << query.lastError().text();
    }
    return ventes;
}

QList<Vente> ModeleRapport::getVenteByClientDansPlageDeDate(int id_client, const QDate &dateDebut, const QDate &dateFin)
{
    QList<Vente> ventes;
    if (id_client < 0) {
         qWarning() << "Invalide ID client" << id_client;
        return ventes;
    }
    if (!dateDebut.isValid() || !dateFin.isValid() || dateDebut > dateFin) {
        qWarning() << "Plage de date invalide";
        return ventes;
    }

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_vente, id_client, date_heure_vente, montant_total, montant_remise FROM Ventes "
                  "WHERE (id_client = :id_client OR (:id_client <= 0 AND id_client IS NULL)) "
                  "AND date(date_heure_vente) >= date(:dateDebut) AND date(date_heure_vente) <= date(:dateFin) "
                  "ORDER BY date_heure_vente DESC");
    query.bindValue(":id_client", id_client);
    query.bindValue(":dateDebut", dateVersDateTimeDebut(dateDebut).toString(Qt::ISODate));
    query.bindValue(":dateFin", dateVersDateTimeFin(dateFin).toString(Qt::ISODate));

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
        qWarning() << "Erreur de recuperation de vente par client par plage de date:" << query.lastError().text();
    }
    return ventes;
}

QMap<QString, double> ModeleRapport::getVenteMensuel(int annee)
{
    QMap<QString, double> ventesMensuel;
    if (annee <= 0) {
        qWarning() << "annee invalide " << annee;
        return ventesMensuel;
    }
    QSqlQuery query(DatabaseManager::database());

    // Using strftime which is SQLite specific.
    query.prepare("SELECT strftime('%Y-%m', date_heure_vente) AS mois_de_vente, SUM(montant_total) AS grand_total_mensuel " // SUM(montant_total) est la somme de (subtotal - discount)
                  "FROM Ventes "
                  "WHERE strftime('%Y', date_heure_vente) = :annee "
                  "GROUP BY mois_de_vente ORDER BY mois_de_vente ASC");
    query.bindValue(":annee", QString::number(annee));

    if (query.exec()) {
        while (query.next()) {
            ventesMensuel.insert(query.value("mois_de_vente").toString(), query.value("grand_total_mensuel").toDouble());
        }
    } else {
        qWarning() << "Echec de recuperation de vente mensuel:" << query.lastError().text();
    }
    return ventesMensuel;
}


QList<ProduitPlusVenduInfo> ModeleRapport::getProduitsPlusVendu(const QDate &dateDebut, const QDate &dateFin, int limite)
{
    QList<ProduitPlusVenduInfo> plusVendu;
    if (!dateDebut.isValid() || !dateFin.isValid() || dateDebut > dateFin) {
        qWarning() << "Plage de date invalide";
        return plusVendu;
    }
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT p.id_produit, p.nom_produit, p.unite_base, p.quantite_stock_en_unite_base, "
        "SUM(cv.quantite_vendu * uvp.facteur_de_conversion_vers_base) AS total_vendu_en_unite_base "
        "FROM Ventes v "
        "JOIN ComposantsVente cv ON v.id_vente = cv.id_vente "
        "JOIN UnitesVenteProduit uvp ON cv.id_unite_vente = uvp.id_unite "
        "JOIN Produits p ON cv.id_produit = p.id_produit "
        "WHERE date(v.date_heure_vente) >= date(:dateDebut) AND date(v.date_heure_vente) <= date(:dateFin) "
        "GROUP BY p.id_produit, p.nom_produit, p.unite_base, p.quantite_stock_en_unite_base "
        "ORDER BY total_vendu_en_unite_base DESC "
        "LIMIT :limite"
    );

    query.bindValue(":dateDebut", dateVersDateTimeDebut(dateDebut).toString(Qt::ISODate));
    query.bindValue(":dateFin", dateVersDateTimeFin(dateFin).toString(Qt::ISODate));
    query.bindValue(":limite", limite);

    if (query.exec()) {
        while (query.next()) {
            ProduitPlusVenduInfo info;
            info.produit.id_produit = query.value(0).toInt();
            info.produit.nom_produit = query.value(1).toString();
            info.produit.unite_base = query.value(2).toString();
            info.produit.quantite_stock_en_unite_base = query.value(3).toDouble();
            info.totalQuantiteVenduEnUniteBase = query.value(4).toDouble();
            plusVendu.append(info);
        }
    } else {
        qWarning() << "Erreur recuperation produit plus vendu: " << query.lastError().text();
    }
    return plusVendu;
}



QList<ProduitEnRuptureStockInfo> ModeleRapport::getProduitEnRuptureStock(double seuilEnUniteBase)
{
    QList<ProduitEnRuptureStockInfo> produitsEnRupture;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT id_produit, nom_produit, unite_base, quantite_stock_en_unite_base "
                  "FROM Produits "
                  "WHERE quantite_stock_en_unite_base <= :seuil "
                  "ORDER BY quantite_stock_en_unite_base ASC");
    query.bindValue(":seuil", seuilEnUniteBase);

    if (query.exec()) {
        while (query.next()) {
            ProduitEnRuptureStockInfo info;
            info.produit.id_produit = query.value("id_produit").toInt();
            info.produit.nom_produit = query.value("nom_produit").toString();
            info.produit.unite_base = query.value("unite_base").toString();
            info.produit.quantite_stock_en_unite_base = query.value("quantite_stock_en_unite_base").toDouble();
            produitsEnRupture.append(info);
        }
    } else {
        qWarning() << "Echec de recuperation des produits en rupture de stock:" << query.lastError().text();
    }
    return produitsEnRupture;
}
