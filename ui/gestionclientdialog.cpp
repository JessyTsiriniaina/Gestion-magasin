#include "gestionclientdialog.h"
#include "ui_gestionclientdialog.h"
#include "shadoweffect.h"

#include <QDebug>
#include <QMessageBox>

GestionClientDialog::GestionClientDialog(QWidget *parent, ControleurClient* controleur, int id_client) :
    QDialog(parent),
    ui(new Ui::GestionClientDialog),
    m_controleurClient(controleur),
    m_id_client(id_client)
{
    ui->setupUi(this);

    if (!m_controleurClient) {
        qCritical() << "Controlleur client est null";
        QMessageBox::critical(this, "Erreur", "Controlleur client non disponible. Dialog ne peut pas fonctionner");

        ui->bouton_enregistrer->setEnabled(false);
        return;
    }

    if (m_id_client != 0) { // Client existant
        setWindowTitle("Modifier client");
        chargerDonneeClient();
    } else { // Nouveau client
        setWindowTitle("Ajouter nouveau client");
    }

    ui->groupBox_details_client->setGraphicsEffect(ShadowEffect::createShadow(this));

    connect(ui->bouton_annuler,
            &QPushButton::clicked,
            this,
            &GestionClientDialog::close
    );
}

GestionClientDialog::~GestionClientDialog()
{
    delete ui;
}

void GestionClientDialog::chargerDonneeClient()
{
    if (!m_controleurClient || m_id_client == 0) return;

    Client client = m_controleurClient->getClientDepuisModel(m_id_client);
    if (client.id_client == -1) {
        QMessageBox::critical(this, "Erreur de chargement", "Ne peut pas charger les informations du client.");
        ui->input_nom_client->setEnabled(false);
        ui->input_telephone_client->setEnabled(false);
        ui->bouton_enregistrer->setEnabled(false);
        return;
    }

    ui->input_nom_client->setText(client.nom_client);
    ui->input_telephone_client->setText(client.telephone_client);
}

void GestionClientDialog::on_bouton_enregistrer_clicked()
{
    if (!m_controleurClient) {
        QMessageBox::critical(this, "Erreur", "Controleur client non disponible.");
        return;
    }

    QString nom = ui->input_nom_client->text().trimmed();
    QString telephone = ui->input_telephone_client->text().trimmed();

    if (nom.isEmpty()) {
        QMessageBox::warning(this, "Entrée invalide", "Nom client ne peut pas être vide.");
        ui->input_nom_client->setFocus();
        return;
    }

    if(!m_controleurClient->isTelephoneValide(telephone)){
        QMessageBox::warning(this, "Numéro de telephone invalide", "Veuillez entrer un numéro de téléphone valide");
        ui->input_telephone_client->setFocus();
        return;
    }

    Client donneeClient;
    donneeClient.nom_client = nom;
    donneeClient.telephone_client = telephone;

    bool reussi = false;
    if (m_id_client == 0) { // nouveau client
        reussi = m_controleurClient->ajouterClient(donneeClient);
        if (reussi) {
            QMessageBox::information(this, "Client enregistré", "Nouveau client enregistré avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur d'enregistrement", "Echec lors de l'enregistrement de nouveau client");
        }
    } else { //  client existant
        donneeClient.id_client = m_id_client;
        reussi = m_controleurClient->modifierClient(donneeClient);
        if (reussi) {
            QMessageBox::information(this, "Modification enregistré", "Information du client a été mis à jour avec succès.");
        } else {
            QMessageBox::critical(this, "Erreur de modification", "Echec lors de la mise à jour de l'information du client.");
        }
    }

    if (reussi) {
        accept();
    }
}
