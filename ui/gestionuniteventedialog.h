#ifndef GESTIONUNITEVENTEDIALOG_H
#define GESTIONUNITEVENTEDIALOG_H

#include <QDialog>

#include "models/produit.h"

namespace Ui {
class GestionUniteVenteDialog;
}

class GestionUniteVenteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GestionUniteVenteDialog(QWidget *parent = 0);
    ~GestionUniteVenteDialog();
    void setDonneeUniteVente(const UniteVenteProduit& unite);
    UniteVenteProduit getDonneeUniteVente();

private slots:
    void on_bouton_enregistrer_clicked();

private:
    Ui::GestionUniteVenteDialog *ui;
    void accept();
};

#endif // GESTIONUNITEVENTEDIALOG_H
