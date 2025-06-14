#include "databasemanager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlDatabase>


/*QSqlDatabase DatabaseManager::database(const QString& path, const QString& connectionName)
{
    QSqlDatabase db;
    if(QSqlDatabase::contains(connectionName)) {
        return QSqlDatabase::database(connectionName);
    }
    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(path);

    if(db.open()) {
        qDebug() << "Connexion reussi";
        return db;
    }
    qDebug() << "Connexion echoué";
    return QSqlDatabase();

}*/

DatabaseManager::DatabaseManager(const QString& path, const QString& connectionName)
    : m_connectionName(connectionName),
      m_db(QSqlDatabase::addDatabase("QSQLITE", connectionName))
{
    m_db.setDatabaseName(path);
}

DatabaseManager::~DatabaseManager()
{
    if(m_db.isOpen()) {
        m_db.close();
        qDebug() << "Instance de base de données fermé";
    }
}

QSqlDatabase DatabaseManager::database(const QString& path, const QString& connectionName)
{
    if (!QSqlDatabase::contains(connectionName)) {
        DatabaseManager* newInstance = new DatabaseManager(path, connectionName);

        if (!newInstance->openDatabase()) {
            qDebug() << "Echec lors de l'ouverture de la base de données:" << connectionName;
            return QSqlDatabase(); // Renvoie un database invalide
        }
    }
    return QSqlDatabase::database(connectionName);
}


bool DatabaseManager::openDatabase()
{
    if (!m_db.isOpen()) {
        if (!m_db.open()) {
            qDebug() << "Erreur d'ouverture de la base de données" << m_db.databaseName()<< m_db.lastError().text();
            return false;
        }
        qDebug() << "Base de données ouvert avec succès.";
    }
    return true;
}
