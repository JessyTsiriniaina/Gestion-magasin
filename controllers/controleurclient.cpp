#include "controleurclient.h"
#include <QStandardItemModel>
#include <QDebug>

ControleurClient::ControleurClient(QObject *parent, ModeleClient* modeleClient) :
    QObject(parent), m_modeleClient(modeleClient)
{
    if(!m_modeleClient) {
        qCritical() << "Modele client non initialisé, fonctionnalité incomplet";
    }
}

void ControleurClient::remplirLigneClient(QStandardItemModel *tableModel, const Client &client)
{
    QList<QStandardItem*> ligneItems;

    QStandardItem* idItem = new QStandardItem(QString::number(client.id_client));
    idItem->setData(client.id_client, Qt::UserRole + 1); //stocker l'ID dans UserRole + 1
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable); //Faire en sorte que l'id n'est pas modifiable
    idItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QStandardItem* nomItem = new QStandardItem(client.nom_client);
    nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);

    QStandardItem* telephoneItem = new QStandardItem(client.telephone_client);
    telephoneItem->setFlags(telephoneItem->flags() & ~Qt::ItemIsEditable);
    telephoneItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    ligneItems << idItem << nomItem << telephoneItem;
    tableModel->appendRow(ligneItems);
}


bool ControleurClient::chargerClientsDansTableModel(QStandardItemModel *tableModel, const QString &motCle)
{
    if (!m_modeleClient) return false;
    if (!tableModel) return false;

    tableModel->clear();
    QStringList headers;
    headers << "ID" << "Nom" << "Téléphone";
    tableModel->setHorizontalHeaderLabels(headers);


    QList<Client> clients;
    if (motCle.isEmpty()) {
        clients = m_modeleClient->getAllClient();
    } else {
        clients = m_modeleClient->rechercherClient(motCle);
    }


    foreach (const Client& client, clients) {
         remplirLigneClient(tableModel, client);
    }
    return true;
}


Client ControleurClient::getClientDepuisModel(int id_client)
{
    if (!m_modeleClient) return Client(); // Return invalide client
    return m_modeleClient->getClientById(id_client);
}


bool ControleurClient::ajouterClient(Client& donneeClient)
{
    if (!m_modeleClient) return false;
    return m_modeleClient->ajouterClient(donneeClient);
}

bool ControleurClient::modifierClient(const Client& donneeClient)
{
    if (!m_modeleClient) return false;
    return m_modeleClient->modifierClient(donneeClient);
}

bool ControleurClient::supprimerClient(int customerId)
{
    if (!m_modeleClient) return false;
    return m_modeleClient->supprimerClient(customerId);
}

bool ControleurClient::isTelephoneValide(QString telephone)
{
    QRegularExpression regex("^$|^(\\+261|0)(20|32|33|34|37|38|39)\\d{7}$");
    return regex.match(telephone).hasMatch();
}











