#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>


class DatabaseManager
{
private:
    QString m_connectionName;
    QSqlDatabase m_db;

    bool openDatabase();

public:
    DatabaseManager(const QString& connectionName);
    ~DatabaseManager();

    static QSqlDatabase database(const QString& connectionName="main_connection");
};


#endif
