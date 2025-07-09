#ifndef GESTIONPRODUITDIALOG_H
#define GESTIONPRODUITDIALOG_H

#include <QDialog>

#include "gestionuniteventedialog.h"
#include "controllers/controleurproduit.h"

namespace Ui {
class GestionProduitDialog;
}

class GestionProduitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GestionProduitDialog(QWidget *parent = 0, ControleurProduit* controleur = 0, int id_produit = 0);
    ~GestionProduitDialog();

private slots:
    void on_bouton_ajouter_unite_clicked();

    void on_bouton_enregistrer_clicked();

    void on_bouton_modifier_unite_clicked();

    void on_bouton_supprimer_unite_clicked();

    void on_table_view_unite_vente_doubleClicked(const QModelIndex &index);

private:
    Ui::GestionProduitDialog *ui;
    GestionUniteVenteDialog *m_gestionUniteVenteDialog;
    ControleurProduit *m_controleurProduit;
    int m_id_produit;
    Produit m_produitCourant;

    QList<UniteVenteProduit> m_unitesVenteCourant; // Liste des unités pour le produit dans ce dialog
    QStandardItemModel* m_unitesVenteTableModel;

    void chargerDonneeProduit();
    void remplirTableUnitesVente();
};

#endif // GESTIONPRODUITDIALOG_H
