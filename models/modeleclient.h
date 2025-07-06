#ifndef MODELECLIENT_H
#define MODELECLIENT_H

#include <QObject>
#include <QString>
#include <QList>


#include "database/databasemanager.h"
#include "client.h"

class ModeleClient : public QObject
{
    Q_OBJECT
public:
    explicit ModeleClient(QObject *parent = 0);


    bool ajouterClient(Client& donneeClient);
    bool modifierClient(const Client& donneeClient);
    bool supprimerClient(int id_client);
    Client getClientById(int id_client);
    QList<Client> getAllClient();
    QList<Client> rechercherClient(const QString& motCle);

signals:

public slots:

};

#endif // MODELECLIENT_H
