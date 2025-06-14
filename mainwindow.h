#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

#include "ui/gestionclientdialog.h"
#include "ui/gestionproduitdialog.h"
#include "ui/reapprovisionnerstockdialog.h"




#include "models/modeleclient.h"
#include "models/modeleproduit.h"
#include "models/modelevente.h"
#include "models/modelerapport.h"




#include "controllers/controleurclient.h"
#include "controllers/controleurproduit.h"
#include "controllers/controleurvente.h"
#include "controllers/controleurrapport.h"



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_mainTabWidget_currentChanged(int index);

    //Gestion vente
     void on_input_recherche_produit_textChanged(const QString &arg1);
     void on_input_rechercher_client_textChanged(const QString &arg1);
     void on_combo_box_produit_disponible_currentIndexChanged(int index);
     void on_combo_box_liste_client_currentIndexChanged(int index);
     void gererChangementTotals(const QVariantMap& totals);


    //Gestion client
    void on_bouton_ajouter_client_clicked();
    void on_input_rechercher_client_2_textChanged(const QString &arg1);
    void on_bouton_supprimer_client_clicked();
    void on_bouton_modifier_client_clicked();



    //Gestion Produit
    void on_bouton_ajouter_produit_clicked();
    void on_bouton_modifier_produit_clicked();
    void on_bouton_supprimer_produit_clicked();
    void on_input_rechercher_produit_textChanged(const QString &arg1);
    void on_bouton_reapprovisionner_produit_clicked();




    void on_bouton_ajouter_au_panier_clicked();

    void on_spin_box_taux_remise_valueChanged(double arg1);

    void on_bouton_supprimer_du_panier_clicked();

    void on_input_montant_remise_textChanged(const QString &arg1);

    void on_bouton_annuler_clicked();

    void on_bouton_finaliser_clicked();

    void on_input_recherche_client_textChanged(const QString &arg1);

    void on_boutton_generer_rapport_clicked();

    void on_radio_button_plage_date_clicked();

    void on_radio_button_par_mois_clicked();

    void on_radio_button_par_client_clicked();

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent *);

    void remplirProduitDisponible(const QList<Produit>& produits);
    void remplirUniteDeMesure(int id_produit);
    void remplirListeClient(QList<Client>& clients) ;


    void configurerClientTab();
    void configurerProduitTab();
    void configurerPanierTable();
    void configurerRapportTab();

    int getIdClientSelectionneDansTableClient();
    int getIdProduitSelectionneDansTableProduit();



    void raffraichirPanierTableView();
    void chargerTableClient();
    void chargerTableProduit();




    //Dialog
    GestionClientDialog *m_gestion_client_dialog;
    GestionProduitDialog *m_gestion_produit_dialog;

    //Gestion client
    ModeleClient *m_modeleClient;
    ControleurClient *m_controleurClient;
    QStandardItemModel *m_clientTableModel;

    //Gestion produit
    ModeleProduit *m_modeleProduit;
    ControleurProduit *m_controleurProduit;
    QStandardItemModel *m_produitTableModel;


    //Gestion vente
    ModeleVente *m_modeleVente;
    ControleurVente *m_controleurVente;
    QStandardItemModel *m_panierTableModel;


    //Gestion rapport
    ModeleRapport* m_modeleRapport;
    ControleurRapport* m_controleurRapport;
    QStandardItemModel* m_rapportTableModel;

    QList<Client> m_clientTrouve;
    QList<Produit> m_produitTrouve;

    bool m_modifieMontantRemiseDirectement;

};

#endif // MAINWINDOW_H
