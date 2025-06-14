#include "gestionuniteventedialog.h"
#include "ui_gestionuniteventedialog.h"

#include <QMessageBox>

GestionUniteVenteDialog::GestionUniteVenteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GestionUniteVenteDialog)
{
    ui->setupUi(this);

    connect(ui->bouton_annuler,
            &QPushButton::clicked,
            this,
            &GestionUniteVenteDialog::close
    );
}

GestionUniteVenteDialog::~GestionUniteVenteDialog()
{
    delete ui;
}

void GestionUniteVenteDialog::setDonneeUniteVente(const UniteVenteProduit& unite)
{
    ui->input_nom_unite->setText(unite.nom_unite);
    ui->spin_box_conversion_unite_base->setValue(unite.facteur_de_conversion_vers_base);
    ui->spin_box_prix_unitaire->setValue(unite.prix_par_unite);
}


UniteVenteProduit GestionUniteVenteDialog::getDonneeUniteVente()
{
    UniteVenteProduit unite;
    unite.nom_unite = ui->input_nom_unite->text().trimmed();
    unite.facteur_de_conversion_vers_base = ui->spin_box_conversion_unite_base->value();
    unite.prix_par_unite = ui->spin_box_prix_unitaire->value();
    return unite;
}


//Override pour ajouter des validations
void GestionUniteVenteDialog::accept()
{
    if (ui->input_nom_unite->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur d'entrée", "Nom unité ne peut pas être vide");
        return;
    }
    if (ui->spin_box_conversion_unite_base->value() <= 0) {
        QMessageBox::warning(this, "Erreur d'entrée", "Facteur de conversion doit être superieur à 0");
        return;
    }
    if (ui->spin_box_prix_unitaire->value() < 0) {
        QMessageBox::warning(this, "IErreur d'entrée", "Prix unitaire ne peut pas être negatif");
        return;
    }
    QDialog::accept();
}

void GestionUniteVenteDialog::on_bouton_enregistrer_clicked()
{
    accept();
}
