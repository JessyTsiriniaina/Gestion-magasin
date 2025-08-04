#include "databasemanager.h"
#include <QDebug>
#include <QSqlError>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

DatabaseManager::DatabaseManager(const QString& connectionName)
    : m_connectionName(connectionName),
      m_db(QSqlDatabase::addDatabase("QSQLITE", connectionName))
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir().mkpath(dataLocation);
    QString dbPath = QDir(dataLocation).filePath("gestion_magasin.db");
    if(!QFile::exists(dbPath)) {
        QFile::copy(":/db/db/gestion_magasin.db", dbPath);
        QFile::setPermissions(dbPath, QFile::ReadOwner | QFile::WriteOwner);
    }

    m_db.setDatabaseName(dbPath);
}

DatabaseManager::~DatabaseManager()
{
    if(m_db.isOpen()) {
        m_db.close();
        qDebug() << "Instance de base de données fermé";
    }
}

QSqlDatabase DatabaseManager::database(const QString& connectionName)
{
    if (!QSqlDatabase::contains(connectionName)) {
        DatabaseManager* newInstance = new DatabaseManager(connectionName);

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
