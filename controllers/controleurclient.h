#ifndef CONTROLEURCLIENT_H
#define CONTROLEURCLIENT_H

#include <QObject>
#include <QStandardItemModel>
#include "models/modeleclient.h"


class ControleurClient : public QObject
{
    Q_OBJECT
public:
    explicit ControleurClient(QObject *parent = 0, ModeleClient* modeleClient = 0);

    bool chargerClientsDansTableModel(QStandardItemModel* tableModel, const QString& motCle = "");

    Client getClientDepuisModel(int id_client);

    bool ajouterClient(Client& donneeClient);
    bool modifierClient(const Client& donneeClient);
    bool supprimerClient(int id_client);

signals:

public slots:


private:
    ModeleClient* m_modeleClient;
    void remplirLigneClient(QStandardItemModel* tableModel, const Client& client);
};

#endif // CONTROLEURCLIENT_H
