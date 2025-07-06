#ifndef MODELERAPPORT_H
#define MODELERAPPORT_H

#include <QObject>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QList>
#include <QMap>

#include "vente.h"
#include "produit.h"
#include "database/databasemanager.h"

struct ProduitPlusVenduInfo {
    Produit produit;
    double totalQuantiteVenduEnUniteBase;

    ProduitPlusVenduInfo() : totalQuantiteVenduEnUniteBase(0.0) {}
};

struct ProduitEnRuptureStockInfo {
    Produit produit;
};


class ModeleRapport : public QObject
{
    Q_OBJECT
public:
    explicit ModeleRapport(QObject *parent = 0);


    QList<Vente> getVenteParPlageDeDate(const QDate& dateDebut, const QDate& dateFin);  //duplicated code???
    QList<Vente> getVenteByClientDansPlageDeDate(int id_client, const QDate& dateDebut, const QDate& dateFin);
    QMap<QString, double> getVenteMensuel(int annee);
    QList<ProduitPlusVenduInfo> getProduitsPlusVendu(const QDate& dateDebut, const QDate& dateFin, int limite = 10);
    QList<ProduitEnRuptureStockInfo> getProduitEnRuptureStock(double seuilEnUniteBase);

signals:

public slots:

private:
    //des fonctions qui convertent QDate en QDateTme
    QDateTime dateVersDateTimeDebut(const QDate& date);
    QDateTime dateVersDateTimeFin(const QDate& date);

};

#endif // MODELERAPPORT_H
