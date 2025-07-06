#include "reapprovisionnerstockdialog.h"
#include "ui_reapprovisionnerstockdialog.h"


#include <QMessageBox>
#include <QDebug>

ReapprovisionnerStockDialog::ReapprovisionnerStockDialog(QWidget *parent, ControleurProduit* controleur, const Produit& produit) :
    QDialog(parent),
    ui(new Ui::ReapprovisionnerStockDialog),
    m_produit(produit),
    m_controleurProduit(controleur)
{
    ui->setupUi(this);

    if (!m_controleurProduit) {
        qCritical() << "Controleur produit est null";
        QMessageBox::critical(this, "Erreur", "Controleur produit non disponible.");
        ui->bouton_ok->setEnabled(false);
        return;
    }

    if (m_produit.id_produit <= 0) {
        qCritical() << "Produit invalide.";
        QMessageBox::critical(this, "Erreur", "Produit invalide fourni.");
        ui->bouton_ok->setEnabled(false);
        return;
    }

    setWindowTitle("Reapprovisionner stock: " + m_produit.nom_produit);
    ui->label_nom_produit->setText(m_produit.nom_produit);
    ui->label_unite_de_base->setText(m_produit.unite_base);
    ui->spin_box_quantite_ajouter->setFocus();





    connect(ui->bouton_annuler,
                &QPushButton::clicked,
                this,
                &ReapprovisionnerStockDialog::close
    );
}

ReapprovisionnerStockDialog::~ReapprovisionnerStockDialog()
{
    delete ui;
}


void ReapprovisionnerStockDialog::on_bouton_ok_clicked()
{
    if (!m_controleurProduit) {
        QMessageBox::critical(this, "Erreur", "Controleur produit non disponible.");
        return;
    }

    double quantite = ui->spin_box_quantite_ajouter->value();
    if (quantite <= 0) {
        QMessageBox::warning(this, "Erreur d'entrée", "Quantité à ajouter doit être superieur ou égal à 0.");
        return;
    }

    if (m_controleurProduit->reapprovisionnerStockProduit(m_produit.id_produit, quantite)) {
        QMessageBox::information(this, "Stock ajouté", QString("%1 %2 de %3 ajouté avec succès.")
                                 .arg(quantite)
                                 .arg(m_produit.unite_base)
                                 .arg(m_produit.nom_produit));
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", QString("Echec de reapprovisionnement de stock %1.").arg(m_produit.nom_produit));
    }
}
