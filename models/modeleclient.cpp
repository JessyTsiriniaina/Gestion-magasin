#include "modeleclient.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

ModeleClient::ModeleClient(QObject *parent) :
    QObject(parent)
{
    //constructeur
}



bool ModeleClient::ajouterClient(Client &donneeClient)
{
    if(donneeClient.nom_client.isEmpty()) {
        qWarning() << "Le nom du client ne peut pas être vide";
        return false;
    }

    QSqlQuery query(DatabaseManager::database());

    query.prepare("INSERT INTO Clients (nom_client, telephone_client) VALUES (:nom, :telephone)");
    query.bindValue(":nom", donneeClient.nom_client);
    query.bindValue(":telephone",
                    donneeClient.telephone_client.isEmpty()?
                        QVariant(QVariant::String) :
                        donneeClient.telephone_client
                    );

    if (!query.exec()) {
        qWarning() << "Echec lors de l'ajout du client " << donneeClient.nom_client << ":" << query.lastError().text();
        return false;
    }

    donneeClient.id_client = query.lastInsertId().toInt();
    return true;
}



bool ModeleClient::modifierClient(const Client &donneeClient)
{

    if(donneeClient.id_client <= 0) {
        qWarning() << "Identifiant invalide pour modifier client";
        return false;
    }

    if(donneeClient.nom_client.isEmpty()) {
        qWarning() << "Le nom du client ne peut pas être vide pour la modification";
        return false;
    }

    QSqlQuery query(DatabaseManager::database());
    query.prepare("UPDATE Clients SET nom_client = :nom, telephone_client = :telephone WHERE id_client = :id_client");

    query.bindValue(":nom", donneeClient.nom_client);

    query.bindValue(":telephone",
                    donneeClient.telephone_client.isEmpty() ?
                        QVariant(QVariant::String) :
                        donneeClient.telephone_client
                    );

    query.bindValue(":id_client", donneeClient.id_client);

    if (!query.exec()) {
        qWarning() << "Echec de modification du client ID: " << donneeClient.id_client << ":" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}


bool ModeleClient::supprimerClient(int id_client)
{

    QSqlQuery query(DatabaseManager::database());
    query.prepare("DELETE FROM Clients WHERE id_client = :id_client");
    query.bindValue(":id_client", id_client);

    if (!query.exec()) {
        qWarning() << "Echec de suppression du client ID: " << id_client << ":" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}



Client ModeleClient::getClientById(int id_client)
{

    Client client;
    if (id_client <= 0) return client; // un client invalide

    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT * FROM Clients WHERE id_client = :id_client");
    query.bindValue(":id_client", id_client);

    if (query.exec() && query.next()) {
        client.id_client = query.value("id_client").toInt();
        client.nom_client = query.value("nom_client").toString();
        client.telephone_client = query.value("telephone_client").toString();
    } else if (query.lastError().isValid()) {
        qWarning() << "Echec lors de la recupération du client par ID:" << query.lastError().text();
    }
    return client;
}


QList<Client> ModeleClient::getAllClient()
{
    QList<Client> clients;
    QSqlQuery query(DatabaseManager::database());
    query.prepare("SELECT * FROM Clients ORDER BY nom_client ASC");

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.id_client = query.value("id_client").toInt();
            client.nom_client = query.value("nom_client").toString();
            client.telephone_client = query.value("telephone_client").toString();
            clients.append(client);
        }
    } else {
        qWarning() << "Echec de recupération de tous les clients:" << query.lastError().text();
    }
    return clients;
}


QList<Client> ModeleClient::rechercherClient(const QString &motCle)
{
    QList<Client> clients;
    QSqlQuery query(DatabaseManager::database());

    QString motSimilaire = "%" + motCle + "%";
    query.prepare("SELECT id_client, nom_client, telephone_client FROM Clients "
                  "WHERE nom_client LIKE :motCle OR telephone_client LIKE :motCle ORDER BY nom_client ASC");
    query.bindValue(":motCle", motSimilaire);

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.id_client = query.value("id_client").toInt();
            client.nom_client = query.value("nom_client").toString();
            client.telephone_client = query.value("telephone_client").toString();
            clients.append(client);
        }
    } else {
        qWarning() << "Echec de recherhe client: " << query.lastError().text();
    }
    return clients;
}




