#include "gestionproduitdialog.h"
#include "ui_gestionproduitdialog.h"

#include "ui/gestionuniteventedialog.h"
#include "shadoweffect.h"


#include <QStandardItemModel>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QLocale>

GestionProduitDialog::GestionProduitDialog(QWidget *parent, ControleurProduit* controleur, int id_produit) :
    QDialog(parent),
    ui(new Ui::GestionProduitDialog),
    m_controleurProduit(controleur),
    m_id_produit(id_produit)
{
    ui->setupUi(this);

    if (!m_controleurProduit) {
        qCritical() << "Controleur produit est null";

        QMessageBox::critical(this, "Erreur", "Controleur produit non disponible. Dialog ne peut pas fonctionner");
        ui->bouton_enregistrer->setEnabled(false); //
        ui->groupBox_unite_vente->setEnabled(false);
        ui->groupBox_detail_produit->setEnabled(false);
        return;
    }

    m_gestionUniteVenteDialog = new GestionUniteVenteDialog(this);


    m_unitesVenteTableModel = new QStandardItemModel(this);
    ui->table_view_unite_vente->setModel(m_unitesVenteTableModel);

    if (m_id_produit != 0) { // Produit existant
        setWindowTitle("Modifier produit");
        chargerDonneeProduit();
        ui->spin_box_stock_initial->setEnabled(false);
        ui->spin_box_stock_initial->setToolTip("Stock pour un produit existant est geré par \"Reapprovisionner stock\".");
    } else { // nouveau produit
        setWindowTitle("Ajouter nouveau produit");
        m_produitCourant = Produit();
        m_produitCourant.id_produit = 0; // Explicitement
        ui->spin_box_stock_initial->setEnabled(true);
    }

    remplirTableUnitesVente();


    ui->groupBox_detail_produit->setGraphicsEffect(ShadowEffect::createShadow(this));
    ui->groupBox_unite_vente->setGraphicsEffect(ShadowEffect::createShadow(this));

    connect(ui->bouton_annuler,
            &QPushButton::clicked,
            this,
            &GestionProduitDialog::close
    );

}

GestionProduitDialog::~GestionProduitDialog()
{
    delete ui;
}

void GestionProduitDialog::chargerDonneeProduit()
{
    if (!m_controleurProduit || m_id_produit == 0) return;

    m_produitCourant = m_controleurProduit->getProduitDepuisModel(m_id_produit);
    if (m_produitCourant.id_produit == -1) {
        QMessageBox::critical(this, "Erreur de chargement", "Ne peut pas charger les données du produit.");
        ui->input_nom_produit->setEnabled(false);
        ui->input_unite_base->setEnabled(false);
        ui->spin_box_remise_pourcentage->setEnabled(false);
        ui->groupBox_unite_vente->setEnabled(false);
        ui->bouton_enregistrer->setEnabled(false);
        return;
    }

    ui->input_nom_produit->setText(m_produitCourant.nom_produit);
    ui->input_unite_base->setText(m_produitCourant.unite_base);
    ui->spin_box_remise_pourcentage->setValue(m_produitCourant.remise_pourcentage);

    m_unitesVenteCourant = m_controleurProduit->getUniteVenteProduitPourProduit(m_id_produit);
}


void GestionProduitDialog::remplirTableUnitesVente()
{
    if (!m_controleurProduit) return;

    m_unitesVenteTableModel->clear();

    QStringList headers;
    headers << "ID" << "Nom Unité" << "Facteur vers Base" << "Prix unitaire";

    m_unitesVenteTableModel->setHorizontalHeaderLabels(headers);


    foreach (const UniteVenteProduit& unite, m_unitesVenteCourant) {
        QList<QStandardItem*> ligneItems;

        QStandardItem* idItem = new QStandardItem(QString::number(unite.id_unite));
        idItem->setData(unite.id_unite, Qt::UserRole + 1);
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        idItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        QStandardItem* nomItem = new QStandardItem(unite.nom_unite);
        nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);

        QStandardItem* facteurItem = new QStandardItem(QString::number(unite.facteur_de_conversion_vers_base, 'f', 4));
        facteurItem->setFlags(facteurItem->flags() & ~Qt::ItemIsEditable);
        facteurItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QStandardItem* prixItem = new QStandardItem(QLocale(QLocale::French, QLocale::Madagascar).toCurrencyString(unite.prix_par_unite, "Ar"));
        prixItem->setFlags(prixItem->flags() & ~Qt::ItemIsEditable);
        prixItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        ligneItems << idItem << nomItem << facteurItem << prixItem;
        m_unitesVenteTableModel->appendRow(ligneItems);
    }
    ui->table_view_unite_vente->resizeColumnsToContents();
}


void GestionProduitDialog::on_bouton_enregistrer_clicked()
{
    if (!m_controleurProduit) {
         QMessageBox::critical(this, "Erreur", "Controleur produit non disponible");
         return;
    }

    //Valider les entrées
    if (ui->input_nom_produit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur d'entrée", "Nom produit ne peut pas être vide.");
        ui->input_nom_produit->setFocus();
        return;
    }
    if (ui->input_unite_base->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur d'entrée", "Unité de base ne peut pas être vide.");
        ui->input_unite_base->setFocus();
        return;
    }
    if (m_id_produit == 0 && ui->spin_box_stock_initial->value() < 0) {
         QMessageBox::warning(this, "Erreur d'entrée", "Stock initial ne peut pas être negatif.");
         ui->spin_box_stock_initial->setFocus();
         return;
    }

    if (m_id_produit == 0 && m_unitesVenteCourant.isEmpty()) {
         QMessageBox::warning(this, "Erreur", "Veuillez définir au moins un unité de vente pour le nouveau produit.");
         return;
    }


    m_produitCourant.nom_produit = ui->input_nom_produit->text().trimmed();
    m_produitCourant.unite_base = ui->input_unite_base->text().trimmed();
    m_produitCourant.remise_pourcentage = ui->spin_box_remise_pourcentage->value();

    if (m_id_produit == 0) { // Nouveau produit
        m_produitCourant.quantite_stock_en_unite_base = ui->spin_box_stock_initial->value();

        if (m_controleurProduit->ajouterProduit(m_produitCourant, m_unitesVenteCourant)) {
            QMessageBox::information(this, "Produit ajouté", "Nouveau produit enregistré avec succès.");
            accept();
        } else {
            QMessageBox::critical(this, "Erreur d'enregistrement", "Echec lors de l'enregistrement du produit");
        }
    } else { // Produit existant
        if (m_controleurProduit->modifierProduit(m_produitCourant)) {
             QMessageBox::information(this, "Modification enregistré", "Modification du produit pris en compte.");
             accept();
        } else {
            QMessageBox::critical(this, "Erreur de modification", "Echec lors de modification du produit");
        }
    }
}




void GestionProduitDialog::on_bouton_ajouter_unite_clicked()
{
    if (!m_controleurProduit) {
        QMessageBox::critical(this, "Erreur", "Controlleur produit non disponible.");
        return;
    }

    GestionUniteVenteDialog uniteDialog(this);
    if (uniteDialog.exec() == QDialog::Accepted) {
        UniteVenteProduit nouvelUnite = uniteDialog.getDonneeUniteVente();

        if (m_id_produit == 0) { //Unité pour un nouveau produit, sera enregistré avec le produit
            nouvelUnite.id_unite = -(m_unitesVenteCourant.size() + 1); //id temporaire

            nouvelUnite.id_produit = 0;
            m_unitesVenteCourant.append(nouvelUnite);
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité ajouté", "Unité de vente pour le produit est enregistré.");
        } else { // Unité pour un produit existant
            nouvelUnite.id_produit = m_id_produit;
            if (m_controleurProduit->ajouterUniteVenteProduit(nouvelUnite)) {
                m_unitesVenteCourant.append(nouvelUnite);
                remplirTableUnitesVente();
                QMessageBox::information(this, "Unité ajouté", "Unité de vente enregistré avec succès.");
            } else {
                QMessageBox::warning(this, "Erreur d'ajout unité", "Echec lors de l'enregistrement de lunité");
            }
        }
    }
}



void GestionProduitDialog::on_bouton_modifier_unite_clicked()
{
    if (!m_controleurProduit) {
        QMessageBox::critical(this, "Erreur", "Controlleur produit non disponible.");
        return;
    }

    QModelIndexList indexSelectionnee = ui->table_view_unite_vente->selectionModel()->selectedRows();
    if (indexSelectionnee.isEmpty()) {
        QMessageBox::information(this, "Modifier unité", "Veuillez selectionner un unité à modifer.");
        return;
    }
    int ligneSelectionnee = indexSelectionnee.first().row();
    int id_unite = m_unitesVenteTableModel->item(ligneSelectionnee, 0)->data(Qt::UserRole + 1).toInt();

    int indexUniteDansListe = -1;
    for(int i=0; i < m_unitesVenteCourant.size(); i++) {
        if(m_unitesVenteCourant[i].id_unite == id_unite) {
            indexUniteDansListe = i;
            break;
        }
    }

    if (indexUniteDansListe == -1) {
        QMessageBox::critical(this, "Erreur", "Unité selectionné introuvable dans la liste");
        return;
    }

    UniteVenteProduit& refUniteAModifier = m_unitesVenteCourant[indexUniteDansListe];

    GestionUniteVenteDialog uniteDialog(this);
    uniteDialog.setDonneeUniteVente(refUniteAModifier);

    if (uniteDialog.exec() == QDialog::Accepted) {
        UniteVenteProduit nouveauDonneeUnite = uniteDialog.getDonneeUniteVente();

        if (m_id_produit == 0) {

            refUniteAModifier.nom_unite = nouveauDonneeUnite.nom_unite;
            refUniteAModifier.facteur_de_conversion_vers_base = nouveauDonneeUnite.facteur_de_conversion_vers_base;
            refUniteAModifier.prix_par_unite = nouveauDonneeUnite.prix_par_unite;
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité modifié", "Modification unité de vente pris en compte");
        } else {
            nouveauDonneeUnite.id_unite = refUniteAModifier.id_unite;
            nouveauDonneeUnite.id_produit = m_id_produit;
            if (m_controleurProduit->modifierUniteVenteProduit(nouveauDonneeUnite)) {

                refUniteAModifier.nom_unite = nouveauDonneeUnite.nom_unite;
                refUniteAModifier.facteur_de_conversion_vers_base = nouveauDonneeUnite.facteur_de_conversion_vers_base;
                refUniteAModifier.prix_par_unite = nouveauDonneeUnite.prix_par_unite;
                remplirTableUnitesVente();
                QMessageBox::information(this, "Unité modifié", "Modification unité de vente pris en compte.");
            } else {
                QMessageBox::warning(this, "Erreur modification unité", "Erreur lors de la modification de l'unité");
            }
        }
    }
}




void GestionProduitDialog::on_bouton_supprimer_unite_clicked()
{
    if (!m_controleurProduit) {
        QMessageBox::critical(this, "Erreur", "Controlleur produit non disponible.");
        return;
    }

    QModelIndexList indexSelectionnee = ui->table_view_unite_vente->selectionModel()->selectedRows();
    if (indexSelectionnee.isEmpty()) {
        QMessageBox::information(this, "Supprimer unité", "Veuillez selectionner un unité à supprimer.");
        return;
    }
    int ligneSelectionnee = indexSelectionnee.first().row();
    int id_unite = m_unitesVenteTableModel->item(ligneSelectionnee, 0)->data(Qt::UserRole + 1).toInt();

    if (QMessageBox::question(this, "Confirmer suppression", "Vous voulez vraimenet supprimer l'unité?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return; //annuler la suppression
    }


    bool trouveEtSupprime = false;
    for (int i = 0; i < m_unitesVenteCourant.size(); i++) {
        if (m_unitesVenteCourant[i].id_unite == id_unite) {
            if (m_id_produit != 0) {
                if (id_unite > 0) {
                    if (!m_controleurProduit->supprimerUniteVenteProduit(id_unite)) {
                        QMessageBox::warning(this, "Erreur de suppression unité", "Echec de suppression de l'unité dans la base de données.");
                        return;
                    }
                }
            }
            m_unitesVenteCourant.removeAt(i);
            trouveEtSupprime = true;
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité supprimé", "Unité de vente supprimé avec succès.");
            break;
        }
    }

    if(!trouveEtSupprime && id_unite > 0){
         QMessageBox::critical(this, "Erreur", "Unité (ID: " + QString::number(id_unite) + ") introuvable dans la liste local pour la suppression.");
    }
}

void GestionProduitDialog::on_table_view_unite_vente_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_bouton_modifier_unite_clicked();
}
