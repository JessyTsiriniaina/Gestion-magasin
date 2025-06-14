#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>


class DatabaseManager
{
private:
    QString m_connectionName;
    QSqlDatabase m_db;

public:
    DatabaseManager(const QString& path, const QString& connectionName);
    ~DatabaseManager();
    //file:///D:/documents/work/c_cpp/cpp/Gestion_magasin/gestion_magasin.db
    static QSqlDatabase database(const QString& path="gestion_magasin.db", const QString& connectionName="main_connection");
    bool openDatabase();
};


#endif
