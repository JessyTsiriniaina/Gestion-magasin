#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database/databasemanager.h"

#include <QDebug>
#include <QMessageBox>
#include <QModelIndexList>
#include <QDialog>
#include <QLocale>
#include <QVariantMap>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_modifieMontantRemiseDirectement(false)
{

    ui->setupUi(this);



    ui->mainTabWidget->setCurrentIndex(0); //Ouvre l'application à la tab vente (pseudo home)
    ui->combo_box_liste_client->clear();
    ui->combo_box_liste_client->addItem("Client temporaire", QVariant(0));


    QSqlDatabase db = DatabaseManager::database("gestion_magasin.db", "main_connection");
    if (!db.isValid() || !db.isOpen()) {
         QMessageBox::critical(this, "Erreur base de données", "Echec lors de la connexion à la base de données");
         return;
    }





    //Initialisation des modeles
    m_modeleClient = new ModeleClient(this);
    m_modeleProduit = new ModeleProduit(this);
    m_modeleVente = new ModeleVente(this);
    m_modeleRapport = new ModeleRapport(this);






    //Initialisation des contrôleurs
    m_controleurClient = new ControleurClient(this, m_modeleClient);
    m_controleurProduit = new ControleurProduit(this, m_modeleProduit);
    m_controleurVente = new ControleurVente(this, m_modeleClient, m_modeleProduit, m_modeleVente);
    m_controleurRapport = new ControleurRapport(this, m_modeleRapport, m_modeleClient, m_modeleProduit);







    //Configurer les tabs
    configurerClientTab();
    configurerProduitTab();
    configurerPanierTable();
    configurerRapportTab();






    //Charger les données dans les tables
    chargerTableClient();
    chargerTableProduit();



    connect(m_controleurVente, SIGNAL(totalsAChange(QVariantMap)), this, SLOT(gererChangementTotals(QVariantMap)));
}

MainWindow::~MainWindow()
{
    delete ui;
}





//---------- Utilité en relation avec la tab VENTE -------------


//Recherche client

void MainWindow::remplirListeClient(QList<Client>& clients)
{
    ui->combo_box_liste_client->clear();
    ui->combo_box_liste_client->addItem("Client temporaire", 0);

    foreach (const Client& client, clients) {
        ui->combo_box_liste_client->addItem(client.nom_client + " (ID: " + QString::number(client.id_client) + ")", QVariant::fromValue(client.id_client));
    }
    if (!clients.isEmpty()) {
        ui->combo_box_liste_client->setCurrentIndex(1);
    }

}

// Recherche et ajout produit

void MainWindow::remplirProduitDisponible(const QList<Produit>& produits)
{
    ui->combo_box_produit_disponible->clear();

    foreach (const Produit& produit, produits) {
        ui->combo_box_produit_disponible->addItem(produit.nom_produit + " (ID: " + QString::number(produit.id_produit) + ")", QVariant::fromValue(produit.id_produit));
    }
    if (!produits.isEmpty()) {
        on_combo_box_produit_disponible_currentIndexChanged(0);
    } else {
        ui->combo_box_produit_disponible->clear();
    }
}


void MainWindow::remplirUniteDeMesure(int id_produit)
{
    ui->combo_box_unite_de_mesure->clear();
    if (!m_controleurVente || id_produit <= 0) return;

    QList<UniteVenteProduit> unites = m_controleurVente->getUniteVenteProduitPourProduit(id_produit);

    foreach (const UniteVenteProduit& unite, unites) {
        QString text = QString("%1 (%2 l'unité)")
                                .arg(unite.nom_unite)
                                .arg(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(unite.prix_par_unite, "Ar"));
        ui->combo_box_unite_de_mesure->addItem(text, QVariant::fromValue(unite.id_unite));
    }
}


//Panier

void MainWindow::configurerPanierTable()
{
    m_panierTableModel = new QStandardItemModel(this);
    ui->table_view_panier->setModel(m_panierTableModel);
}

void MainWindow::raffraichirPanierTableView()
{
    if (!m_controleurVente) return;
    m_panierTableModel->clear();
    QStringList headers;
    headers << "Nom produit" << "Unite" << "Qte" << "Prix/Unité" << "Subtotal";
    m_panierTableModel->setHorizontalHeaderLabels(headers);

    const QList<AffichageElementPanier>& elementsPanier = m_controleurVente->getElementsPanier();

    foreach (const AffichageElementPanier& element, elementsPanier) {
        QList<QStandardItem*> elementsLigne;
        elementsLigne.append(new QStandardItem(element.nom_produit));
        elementsLigne.append(new QStandardItem(element.nom_unite));
        elementsLigne.append(new QStandardItem(QString::number(element.quantite_vendu, 'f', 3)));
        elementsLigne.append(new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(element.prix_de_vente, "Ar")));
        elementsLigne.append(new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(element.subtotal_composant, "Ar")));



        m_panierTableModel->appendRow(elementsLigne);
    }
    ui->table_view_panier->resizeColumnsToContents();
}




//Totals

void MainWindow::gererChangementTotals(const QVariantMap& totals)
{
    QLocale locale(QLocale::French, QLocale::Madagascar);
    ui->input_subtotal->setText(locale.toCurrencyString(totals.value("subtotal", 0.0).toDouble(), "Ar"));

    double pourcentageRemise = totals.value("pourcentageRemise", 0.0).toDouble();
    double montantRemise = totals.value("montantRemise", 0.0).toDouble();

    bool spinBoxPourcentageRemiseBloque = ui->spin_box_taux_remise->blockSignals(true);
    bool inputMontantRemiseBloque = ui->input_montant_remise->blockSignals(true);

    if (pourcentageRemise > 0) {
        ui->spin_box_taux_remise->setValue(pourcentageRemise);
        ui->input_montant_remise->setReadOnly(true);
        ui->input_montant_remise->setText(locale.toCurrencyString(montantRemise, "Ar"));
    } else {
        ui->spin_box_taux_remise->setValue(0.0);
        ui->input_montant_remise->setReadOnly(false);

        if (!m_modifieMontantRemiseDirectement ||
            ui->input_montant_remise->text().isEmpty() ||
            QLocale(QLocale::French, QLocale::Madagascar).toDouble(ui->input_montant_remise->text()) != montantRemise) {
             ui->input_montant_remise->setText(locale.toCurrencyString(montantRemise, "Ar"));
        }
    }

    ui->spin_box_taux_remise->blockSignals(spinBoxPourcentageRemiseBloque);
    ui->input_montant_remise->blockSignals(inputMontantRemiseBloque);

    ui->input_grand_total->setText(locale.toCurrencyString(totals.value("grandTotal", 0.0).toDouble(), "Ar"));
}





//---------- Utilité en relation avec la tab RAPPORT -------------


void MainWindow::configurerRapportTab()
{
    m_rapportTableModel = new QStandardItemModel(this);
    ui->table_view_resultat_rapport->setModel(m_rapportTableModel);
    ui->combo_box_mois->addItem("Tous les ans", QVariant(0));
    ui->combo_box_selectionner_client->addItem("Client temporaire", QVariant(0));
    for (int i = 1; i <= 12; ++i) {
        ui->combo_box_mois->addItem(QLocale(QLocale(QLocale::French, QLocale::Madagascar)).monthName(i), i);
    }

    // Set default dates
    QDate dateCourant = QDate::currentDate();
    QDate premierJourDuMoisPrecedent = dateCourant.addMonths(-1);
    premierJourDuMoisPrecedent.setDate(premierJourDuMoisPrecedent.year(), premierJourDuMoisPrecedent.month(), 1);

    ui->date_edit_de->setDate(premierJourDuMoisPrecedent);
    ui->date_edit_a->setDate(dateCourant);
    ui->date_edit_depuis->setDate(premierJourDuMoisPrecedent);
    ui->date_edit_jusque->setDate(dateCourant);
    ui->spin_box_annee->setRange(2000, dateCourant.year());
    ui->spin_box_annee->setValue(QDate::currentDate().year());



    on_radio_button_plage_date_clicked();
}





//---------- Utilité en relation avec la tab CLIENT -------------

void MainWindow::configurerClientTab()
{
    m_clientTableModel = new QStandardItemModel(this);
    ui->tableView_clients->setModel(m_clientTableModel);
}


void MainWindow::chargerTableClient()
{
    if (!m_controleurClient || !m_clientTableModel) {
        qWarning() << "Controlleur client ou client table model non initialisé.";
        return;
    }
    m_controleurClient->chargerClientsDansTableModel(m_clientTableModel, ui->input_rechercher_client_2->text());
}

int MainWindow::getIdClientSelectionneDansTableClient()
{
    QModelIndexList ligneSelectionne = ui->tableView_clients->selectionModel()->selectedRows();
    if (ligneSelectionne.isEmpty()) return -1;
    QModelIndex idIndex = m_clientTableModel->index(ligneSelectionne.first().row(), 0); // ID est dans la première colonne
    return m_clientTableModel->data(idIndex, Qt::UserRole + 1).toInt(); // ID est stocké dans UserRole+1
}





//---------- Utilité en relation avec la tab PRODUIT -------------

void MainWindow::configurerProduitTab()
{
    m_produitTableModel = new QStandardItemModel(this);
    ui->tableView_produits->setModel(m_produitTableModel);
}

void MainWindow::chargerTableProduit()
{
    if (!m_controleurProduit || !m_produitTableModel) {
        qWarning() << "Controlleur produit ou produit table model non initialisé.";
        return;
    }
    m_controleurProduit->chargerProduitsDansTableModel(m_produitTableModel, ui->input_rechercher_produit->text());
}

int MainWindow::getIdProduitSelectionneDansTableProduit()
{
    QModelIndexList ligneSelectionne = ui->tableView_produits->selectionModel()->selectedRows();
    if (ligneSelectionne.isEmpty()) return -1;
    QModelIndex idIndex = m_produitTableModel->index(ligneSelectionne.first().row(), 0);
    return m_produitTableModel->data(idIndex, Qt::UserRole + 1).toInt();
}








//---------------- GERER LES CHANGEMENTS DE TAB --------------------------------------

void MainWindow::on_mainTabWidget_currentChanged(int index)
{
    // 0 = Vente, 1 = Rapport, 2 = Client, 3 = Produit
    if (index == 0) {
        //a faire
    } else if (index == 1) {
        //a faire
    } else if (index == 2) {
        chargerTableClient();
    } else if (index == 3) {
        chargerTableProduit();
    }
}







//------------------------------- SLOT --------------------------------------


//-------------- VENTE TAB SLOT -----------------


//Recherche client

void MainWindow::on_input_rechercher_client_textChanged(const QString &arg1)
{
    if (!m_controleurVente) return;
    m_clientTrouve = m_controleurVente->rechercherClient(arg1);
    remplirListeClient(m_clientTrouve);
}

void MainWindow::on_combo_box_liste_client_currentIndexChanged(int index)
{
    if (!m_controleurVente) return;
    int id_client = ui->combo_box_liste_client->itemData(index).toInt();
    if(id_client > 0) {
        foreach(const Client& client, m_clientTrouve) {
            if(id_client == client.id_client) {
                m_controleurVente->setClientCourant(client);
            }
        }
    }
}

//Recherche et ajout produit

void MainWindow::on_input_recherche_produit_textChanged(const QString &arg1)
{
    if (!m_controleurVente) return;
    m_produitTrouve = m_controleurVente->rechercherProduit(arg1);
    remplirProduitDisponible(m_produitTrouve);
}


void MainWindow::on_combo_box_produit_disponible_currentIndexChanged(int index)
{
    if (!m_controleurVente || index < 0 || index >= ui->combo_box_produit_disponible->count()) {
        ui->combo_box_unite_de_mesure->clear();
        return;
    }
    int id_produit = ui->combo_box_produit_disponible->itemData(index).toInt();
    remplirUniteDeMesure(id_produit);
}



//Panier

void MainWindow::on_bouton_ajouter_au_panier_clicked()
{
    if (!m_controleurVente) {
        QMessageBox::critical(this, "Erreur", "Controleur vente non disponible.");
        return;
    }

    int produitComboIndex = ui->combo_box_produit_disponible->currentIndex();
    if (produitComboIndex < 0) {
        QMessageBox::warning(this, "Erreur d'entréé", "Veuillez sélectionner un produit.");
        ui->combo_box_produit_disponible->setFocus();
        return;
    }
    int id_produit = ui->combo_box_produit_disponible->itemData(produitComboIndex).toInt();

    int uniteComboIndex = ui->combo_box_unite_de_mesure->currentIndex();
    if (uniteComboIndex < 0) {
        QMessageBox::warning(this, "Erreur d'entréé", "Veuillez sélectionner un unite de mesure pour le produit.");
        ui->combo_box_unite_de_mesure->setFocus();
        return;
    }
    int id_unite = ui->combo_box_unite_de_mesure->itemData(uniteComboIndex).toInt();

    double quantite = ui->spin_box_quantite->value();

    if (quantite <= 0) {
        QMessageBox::warning(this, "Erreur d'entrée", "Quantité doit être superieur ou égal à 0.");
        ui->spin_box_quantite->setFocus();
        return;
    }

    if(!m_controleurVente->stockProduitSuffisant(id_produit, id_unite, quantite)) {
        QMessageBox::warning(this, "Stock insuffisant", "La quantité du produit dans le stock est inferieur à celui demandé.");
        ui->spin_box_quantite->setFocus();
        return;
    }
    if (!m_controleurVente->ajouterElementAuPanier(id_produit, id_unite, quantite)) {
        QMessageBox::warning(this, "Echec d'ajout au panier", "Ne peut pas ajouter le produit sélectionné au panier");
    }
    raffraichirPanierTableView();
}



void MainWindow::on_input_montant_remise_textChanged(const QString &arg1)
{
    if (!m_controleurVente) return;

    if (ui->spin_box_taux_remise->value() == 0 && !ui->input_montant_remise->isReadOnly()) {
        m_modifieMontantRemiseDirectement = true;
        bool ok;

        QString valeur_input = ui->input_montant_remise->text();
        if(valeur_input.endsWith("Ar"))
            valeur_input.chop(2);

        double montant = QLocale(QLocale::French, QLocale::Madagascar).toDouble(valeur_input, &ok);

        if (ok) {
            m_controleurVente->setMontantRemise(montant);
        } else if (ui->input_montant_remise->text().trimmed().isEmpty()) {
            // si champs vide, traiter comme si c'est 0  remise
            m_controleurVente->setMontantRemise(0.0);
        } else {

        }
    }
}



void MainWindow::on_bouton_supprimer_du_panier_clicked()
{
    if (!m_controleurVente) {
        QMessageBox::critical(this, "Erreur", "Controleur vente non disponible.");
        return;
    }
    QModelIndexList ligneSelectionne = ui->table_view_panier->selectionModel()->selectedRows();
    if (ligneSelectionne.isEmpty()) {
        QMessageBox::information(this, "Supprimer element", "Veuillez selectionner un element du panier à supprimer.");
        return;
    }

    if(!m_controleurVente->supprimerElementDuPanier(ligneSelectionne.first().row())) {
        QMessageBox::warning(this, "Erreur", "Echec de suppression de l'element du panier");
    }
    raffraichirPanierTableView();
}


void MainWindow::on_spin_box_taux_remise_valueChanged(double arg1)
{
    if (!m_controleurVente) return;
    m_modifieMontantRemiseDirectement = false;
    m_controleurVente->setPourcentageRemise(arg1);


    ui->input_montant_remise->setReadOnly(arg1 > 0);
    if(arg1 > 0) {
        m_modifieMontantRemiseDirectement = false;
    }
}

//Finaliser



void MainWindow::on_bouton_finaliser_clicked()
{
    if (!m_controleurVente) {
        QMessageBox::critical(this, "Erreur", "Controleur vente non disponible.");
        return;
    }

    if (m_controleurVente->getElementsPanier().isEmpty()) {
        QMessageBox::warning(this, "Vente vide", "Ne peut pas finaliser une vente vide. Veuillez ajouter des élements dans le panier.");
        return;
    }


    if (QMessageBox::question(this, "Confirmer la vente",
                              "Vous êtes sur de vouloir finaliser la vente?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    Vente donneeVenteComplet;
    if (m_controleurVente->finaliserVente(donneeVenteComplet)) {
        QMessageBox::information(this, "Vente enregistré", QString("Vente (ID: %1) enregistré avec succès!").arg(donneeVenteComplet.id_vente));
    } else {
        QMessageBox::critical(this, "Erreur", "Echec d'enregistement de vente.");
    }
}



void MainWindow::on_bouton_annuler_clicked()
{
    if (!m_controleurVente) return;
    ui->input_montant_remise->clear();
    ui->input_rechercher_client->clear();
    ui->input_recherche_produit->clear();
    ui->combo_box_liste_client->clear();
    ui->combo_box_produit_disponible->clear();
    m_controleurVente->reinitialiserVente();
    raffraichirPanierTableView();
}




//-------------- RAPPORT TAB SLOT -----------------



void MainWindow::on_input_recherche_client_textChanged(const QString &arg1)
{
    if (!m_controleurRapport) return;
    QList<Client> clients = m_controleurRapport->rechercherClient(arg1);
    ui->combo_box_selectionner_client->clear();
    ui->combo_box_selectionner_client->addItem("Client temporaire", QVariant(0));

    foreach (const Client& client, clients) {
        ui->combo_box_selectionner_client->addItem(client.nom_client + " (ID: " + QString::number(client.id_client) + ")", QVariant::fromValue(client.id_client));
    }

    if (!clients.isEmpty()) {
        ui->combo_box_selectionner_client->setCurrentIndex(1);
    }
}



void MainWindow::on_boutton_generer_rapport_clicked()
{
    if (!m_controleurRapport || !m_rapportTableModel) {
        QMessageBox::critical(this, "Erreur", "Modules de rapport non initialisé.");
        return;
    }

    int indexTypeRapport = ui->combo_box_type_rapport->currentIndex();
    m_rapportTableModel->clear();

    switch (indexTypeRapport) {
        case 0: // Historique de vente
        {
            if (ui->radio_button_plage_date->isChecked()) {
                QDate dateDebut = ui->date_edit_de->date();
                QDate dateFin = ui->date_edit_a->date();
                if (dateDebut > dateFin) {
                    QMessageBox::warning(this, "Erreur d'entrée", "Date debut doit être avant date fin.");
                    return;
                }
                m_controleurRapport->genererHistoriqueDeVente(dateDebut, dateFin, m_rapportTableModel);
            }

            else if (ui->radio_button_par_mois->isChecked()) {
                int annee = ui->spin_box_annee->value();
                int mois = ui->combo_box_mois->currentData().toInt(); // 0 pour tous les ans
                m_controleurRapport->genererHistoriqueDeVenteMensuel(annee, mois, m_rapportTableModel);
            }

            else if (ui->radio_button_par_client->isChecked()) {
                int id_client = ui->combo_box_selectionner_client->currentData().toInt();
                if (id_client < 0) {
                     QMessageBox::warning(this, "Erreur d'entrée", "Veuillez selectionner un client.");
                     return;
                }
                QDate dateDebut = ui->date_edit_de->date();
                QDate dateFin = ui->date_edit_a->date();
                 if (dateDebut > dateFin) {
                    QMessageBox::warning(this, "Erreur d'entrée", "Date debut doit être avant date fin.");
                    return;
                }
                m_controleurRapport->genererHistoriqueDeVenteParClient(id_client, dateDebut, dateFin, m_rapportTableModel);
            }
            break;
        }
        case 1: // Produit le plus vendu
        {
            QDate dateDebut = ui->date_edit_depuis->date();
            QDate dateFin = ui->date_edit_jusque->date();
            int limite = ui->spin_box_limite_vente->value();
            if (dateDebut > dateFin) {
                QMessageBox::warning(this, "Erreur d'entrée", "Date debut doit être avant date fin.");
                return;
            }
            m_controleurRapport->genererRapportProduitPlusVendu(dateDebut, dateFin, limite, m_rapportTableModel);
            break;
        }
        case 2: // Produit en rupture de stock
        {
            double seuil = ui->spin_box_seuil_stock->value();
            m_controleurRapport->genererRapportProduitEnRuptureStock(seuil, m_rapportTableModel);
            break;
        }
        default:
            QMessageBox::warning(this, "Erreur de generation de rapport", "Type de rapport non géré");
            break;
    }
    ui->table_view_resultat_rapport->resizeColumnsToContents();
}




void MainWindow::on_radio_button_plage_date_clicked()
{
    ui->date_edit_a->setEnabled(true);
    ui->date_edit_de->setEnabled(true);
    ui->spin_box_annee->setEnabled(false);
    ui->combo_box_mois->setEnabled(false);
    ui->input_recherche_client->setEnabled(false);
    ui->combo_box_selectionner_client->setEnabled(false);
}



void MainWindow::on_radio_button_par_mois_clicked()
{
    ui->date_edit_a->setEnabled(false);
    ui->date_edit_de->setEnabled(false);
    ui->spin_box_annee->setEnabled(true);
    ui->combo_box_mois->setEnabled(true);
    ui->input_recherche_client->setEnabled(false);
    ui->combo_box_selectionner_client->setEnabled(false);
}

void MainWindow::on_radio_button_par_client_clicked()
{
    ui->date_edit_a->setEnabled(true);
    ui->date_edit_de->setEnabled(true);
    ui->spin_box_annee->setEnabled(false);
    ui->combo_box_mois->setEnabled(false);
    ui->input_recherche_client->setEnabled(true);
    ui->combo_box_selectionner_client->setEnabled(true);
}





//-------------- CLIENT TAB SLOT -----------------

void MainWindow::on_input_rechercher_client_2_textChanged(const QString &text)
{
    chargerTableClient();
}

void MainWindow::on_bouton_ajouter_client_clicked()
{
    if(!m_controleurClient) return;
    m_gestion_client_dialog = new GestionClientDialog(this, m_controleurClient, 0);
    if(m_gestion_client_dialog->exec() == QDialog::Accepted) chargerTableClient();
}

void MainWindow::on_bouton_modifier_client_clicked()
{
    if(!m_controleurClient) return;
    int id_client = getIdClientSelectionneDansTableClient();
    if (id_client == -1) {
        QMessageBox::information(this, "Modifier client", "Veuillez selectionner un client.");
        return;
    }
    m_gestion_client_dialog = new GestionClientDialog(this, m_controleurClient, id_client);
    if (m_gestion_client_dialog->exec() == QDialog::Accepted) chargerTableClient();
}

void MainWindow::on_bouton_supprimer_client_clicked()
{
    if(!m_controleurClient) return;
    int id_client = getIdClientSelectionneDansTableClient();
    if (id_client == -1) {
        QMessageBox::information(this, "Supprimer client", "Veuillez selectionner le client à supprimer.");
        return;
    }
    Client clientASupprimer = m_controleurClient->getClientDepuisModel(id_client);
     if (clientASupprimer.id_client == -1) {
        QMessageBox::warning(this, "Supprimer client", "Ne peut pas recuperer les information du client.");
        return;
    }
    if (QMessageBox::question(this, "Confirmer la suppression",
                              QString("Vous êtes sûr de vouloir supprimer du client '%1'?").arg(clientASupprimer.nom_client),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_controleurClient->supprimerClient(id_client)) {
            QMessageBox::information(this, "Client supprimé", "Le client a été supprimé avec succès.");
            chargerTableClient();
        } else {
            QMessageBox::critical(this, "Erreur", "Echec de suppression de client.");
        }
    }
}



//-------------- PRODUIT TAB SLOT -----------------


void MainWindow::on_bouton_ajouter_produit_clicked()
{
    if(!m_controleurProduit) return;
    m_gestion_produit_dialog = new GestionProduitDialog(this, m_controleurProduit, 0);
    if(m_gestion_produit_dialog->exec() == QDialog::Accepted) chargerTableProduit();
}


void MainWindow::on_input_rechercher_produit_textChanged(const QString &arg1)
{
    chargerTableProduit();
}

void MainWindow::on_bouton_modifier_produit_clicked()
{
    if (!m_controleurProduit) return;
    int id_produit = getIdProduitSelectionneDansTableProduit();
    if (id_produit == -1) {
        QMessageBox::information(this, "Modifier produit", "Veuillez selectionner un produit à modifier.");
        return;
    }
    m_gestion_produit_dialog = new GestionProduitDialog(this, m_controleurProduit, id_produit);
    if (m_gestion_produit_dialog->exec() == QDialog::Accepted) chargerTableProduit();
}


void MainWindow::on_bouton_supprimer_produit_clicked()
{
    if (!m_controleurProduit) return;
    int id_produit = getIdProduitSelectionneDansTableProduit();
    if (id_produit == -1) {
        QMessageBox::information(this, "Supprimer produit", "Veuillez selectionner un produit à supprimer.");
        return;
    }
    Produit produitASupprimer = m_controleurProduit->getProduitDepuisModel(id_produit);
    if (produitASupprimer.id_produit == -1) {
         QMessageBox::warning(this, "Supprimer produit", "Ne peut pas charger les details du produit.");
         return;
    }
    if (QMessageBox::question(this, "Confirmer suppression",
                              QString("Voulez-vous vraiment supprimer le produit '%1'?").arg(produitASupprimer.nom_produit),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (m_controleurProduit->supprimerProduit(id_produit)) {
            QMessageBox::information(this, "Produit supprimé", "Produit supprimé avec succès.");
            chargerTableProduit();
        } else {
            QMessageBox::critical(this, "Erreur", "Echec lors de la suppression de produit.");
        }
    }
}


void MainWindow::on_bouton_reapprovisionner_produit_clicked()
{
    if (!m_controleurProduit) return;
    int id_produit = getIdProduitSelectionneDansTableProduit();
    if (id_produit == -1) {
        QMessageBox::information(this, "Reapprovisionner stock", "Veuillez selectionner un produit à reapprovisionner.");
        return;
    }
    Produit produit = m_controleurProduit->getProduitDepuisModel(id_produit);
    if (produit.id_produit == -1) {
        QMessageBox::warning(this, "Reapprovisionner stock", "Ne peut pas charger les details du produit.");
        return;
    }
    ReapprovisionnerStockDialog dialog(this, m_controleurProduit, produit);
    if (dialog.exec() == QDialog::Accepted) chargerTableProduit();
}

