#ifndef PRODUIT_H
#define PRODUIT_H

#include <QString>

struct Produit {
    int id_produit;
    QString nom_produit;
    QString unite_base;
    double quantite_stock_en_unite_base;
    double remise_pourcentage;

    Produit() : id_produit(-1), quantite_stock_en_unite_base(0.0), remise_pourcentage(0.0) {}
};


struct UniteVenteProduit {
    int id_unite;
    int id_produit;
    QString nom_unite;
    double facteur_de_conversion_vers_base;
    double prix_par_unite;

    UniteVenteProduit() : id_unite(-1),
                          id_produit(-1),
                          facteur_de_conversion_vers_base(0.0),
                          prix_par_unite(0.0)
                       {}
};

#endif // PRODUIT_H
