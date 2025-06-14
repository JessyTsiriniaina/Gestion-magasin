#ifndef VENTE_H
#define VENTE_H

#include <QString>
#include <QDateTime>

struct Vente {
    int id_vente;
    int id_client;
    QDateTime  date_heure_vente;
    double montant_total;
    double montant_remise;

    Vente(): id_vente(-1), id_client(-1), montant_total(0.0), montant_remise(0.0) {}
};



struct ComposantVente {
    int id_composant_vente;
    int id_vente;
    int id_produit;
    int id_unite_vente;
    double quantite_vendu;
    double prix_de_vente;
    double subtotal_composant;

    ComposantVente(): id_composant_vente(-1),
                      id_vente(-1),
                      id_produit(-1),
                      id_unite_vente(-1),
                      quantite_vendu(0.0),
                      prix_de_vente(0.0),
                      subtotal_composant(0.0)
    {}
};

#endif // VENTE_H
