#ifndef REAPPROVISIONNERSTOCKDIALOG_H
#define REAPPROVISIONNERSTOCKDIALOG_H

#include <QDialog>

#include "controllers/controleurproduit.h"
#include "models/modeleproduit.h"


namespace Ui {
class ReapprovisionnerStockDialog;
}

class ReapprovisionnerStockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReapprovisionnerStockDialog(QWidget *parent = 0, ControleurProduit* controleur = 0, const Produit& produit = Produit());
    ~ReapprovisionnerStockDialog();

private slots:
    void on_bouton_ok_clicked();

private:
    Ui::ReapprovisionnerStockDialog *ui;
    Produit m_produit;
    ControleurProduit* m_controleurProduit;
};

#endif // REAPPROVISIONNERSTOCKDIALOG_H
