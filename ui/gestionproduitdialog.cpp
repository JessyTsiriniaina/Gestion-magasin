#include "gestionproduitdialog.h"
#include "ui_gestionproduitdialog.h"

#include "ui/gestionuniteventedialog.h"


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
        chargerDonneeClient();
        ui->spin_box_stock_initial->setEnabled(false);
        ui->spin_box_stock_initial->setToolTip("Stock pour un produit existant est geré par \"Reapprovisionner stock\".");
    } else { // nouveau produit
        setWindowTitle("Ajouter nouveau produit");
        m_produitCourant = Produit();
        m_produitCourant.id_produit = 0; // Explicitement
        ui->spin_box_stock_initial->setEnabled(true);
    }

    remplirTableUnitesVente();

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

void GestionProduitDialog::chargerDonneeClient()
{
    if (!m_controleurProduit || m_id_produit == 0) return;

    m_produitCourant = m_controleurProduit->getProduitDepuisModel(m_id_produit);
    if (m_produitCourant.id_produit == -1) {
        QMessageBox::critical(this, "Erreur de chargement", "Ne peut pas charger les données du produit.");
        ui->input_nom_produit->setEnabled(false);
        ui->input_unite_base->setEnabled(false);
        ui->groupBox_unite_vente->setEnabled(false);
        ui->bouton_enregistrer->setEnabled(false);
        return;
    }

    ui->input_nom_produit->setText(m_produitCourant.nom_produit);
    ui->input_unite_base->setText(m_produitCourant.unite_base);


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



















//RENOMMMER LES VARIABLES






void GestionProduitDialog::on_bouton_ajouter_unite_clicked()
{
    if (!m_controleurProduit) {
        QMessageBox::critical(this, "Erreur", "Controlleur produit non disponible.");
        return;
    }

    GestionUniteVenteDialog unitDialog(this);
    if (unitDialog.exec() == QDialog::Accepted) {
        UniteVenteProduit newUnit = unitDialog.getDonneeUniteVente();

        if (m_id_produit == 0) { //Unité pour un nouveau produit, sera enregistré avec le produit
            newUnit.id_unite = -(m_unitesVenteCourant.size() + 1); //id temporaire

            newUnit.id_produit = 0;
            m_unitesVenteCourant.append(newUnit);
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité ajouté", "Unité de vente pour le produit est enregistré.");
        } else { // Unité pour un produit existant
            newUnit.id_produit = m_id_produit;
            if (m_controleurProduit->ajouterUniteVenteProduit(newUnit)) {
                m_unitesVenteCourant.append(newUnit);
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

    QModelIndexList selectedIndexes = ui->table_view_unite_vente->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "Modifier unité", "Veuillez selectionner un unité à modifer.");
        return;
    }
    int selectedRow = selectedIndexes.first().row();
    int unitIdInTable = m_unitesVenteTableModel->item(selectedRow, 0)->data(Qt::UserRole + 1).toInt();

    int unitIndexInList = -1;
    for(int i=0; i < m_unitesVenteCourant.size(); i++) {
        if(m_unitesVenteCourant[i].id_unite == unitIdInTable) {
            unitIndexInList = i;
            break;
        }
    }

    if (unitIndexInList == -1) {
        QMessageBox::critical(this, "Erreur", "Unité selectionné introuvable dans la liste");
        return;
    }

    UniteVenteProduit& unitToEditRef = m_unitesVenteCourant[unitIndexInList];

    GestionUniteVenteDialog unitDialog(this);
    unitDialog.setDonneeUniteVente(unitToEditRef);

    if (unitDialog.exec() == QDialog::Accepted) {
        UniteVenteProduit updatedUnitDataFromDialog = unitDialog.getDonneeUniteVente();

        if (m_id_produit == 0) {

            unitToEditRef.nom_unite = updatedUnitDataFromDialog.nom_unite;
            unitToEditRef.facteur_de_conversion_vers_base = updatedUnitDataFromDialog.facteur_de_conversion_vers_base;
            unitToEditRef.prix_par_unite = updatedUnitDataFromDialog.prix_par_unite;
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité modifié", "Modification unité de vente pris en compte");
        } else {
            updatedUnitDataFromDialog.id_unite = unitToEditRef.id_unite;
            updatedUnitDataFromDialog.id_produit = m_id_produit;
            if (m_controleurProduit->modifierUniteVenteProduit(updatedUnitDataFromDialog)) {
                // Update the unit in m_currentSellingUnits to reflect changes
                unitToEditRef.nom_unite = updatedUnitDataFromDialog.nom_unite;
                unitToEditRef.facteur_de_conversion_vers_base = updatedUnitDataFromDialog.facteur_de_conversion_vers_base;
                unitToEditRef.prix_par_unite = updatedUnitDataFromDialog.prix_par_unite;
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

    QModelIndexList selectedIndexes = ui->table_view_unite_vente->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "Supprimer unité", "Veuillez selectionner un unité à supprimer.");
        return;
    }
    int selectedRow = selectedIndexes.first().row();
    int unitIdInTable = m_unitesVenteTableModel->item(selectedRow, 0)->data(Qt::UserRole + 1).toInt();

    if (QMessageBox::question(this, "Confirmer suppression", "Vous voulez vraimenet supprimer l'unité?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return; //annuler la suppression
    }


    bool foundAndRemoved = false;
    for (int i = 0; i < m_unitesVenteCourant.size(); i++) {
        if (m_unitesVenteCourant[i].id_unite == unitIdInTable) {
            if (m_id_produit != 0) {
                if (unitIdInTable > 0) {
                    if (!m_controleurProduit->supprimerUniteVenteProduit(unitIdInTable)) {
                        QMessageBox::warning(this, "Erreur de suppression unité", "Echec de suppression de l'unité dans la base de données.");
                        return;
                    }
                }
            }
            m_unitesVenteCourant.removeAt(i);
            foundAndRemoved = true;
            remplirTableUnitesVente();
            QMessageBox::information(this, "Unité supprimé", "Unité de vente supprimé avec succès.");
            break;
        }
    }

    if(!foundAndRemoved && unitIdInTable > 0){
         QMessageBox::critical(this, "Erreur", "Unité (ID: " + QString::number(unitIdInTable) + ") introuvable dans la liste local pour la suppression.");
    }
}

void GestionProduitDialog::on_table_view_unite_vente_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    on_bouton_modifier_unite_clicked();
}
