#ifndef GESTIONCLIENTDIALOG_H
#define GESTIONCLIENTDIALOG_H

#include <QDialog>

#include "models/client.h"
#include "controllers/controleurclient.h"

namespace Ui {
class GestionClientDialog;
}

class GestionClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GestionClientDialog(QWidget *parent = 0, ControleurClient* controleur = 0, int id_client = 0);
    ~GestionClientDialog();

private slots:
    void on_bouton_enregistrer_clicked();

private:
    Ui::GestionClientDialog *ui;
    ControleurClient* m_controleurClient;
    int m_id_client; //0 si c'est un nouveau client

    void chargerDonneeClient();
};

#endif // GESTIONCLIENTDIALOG_H
