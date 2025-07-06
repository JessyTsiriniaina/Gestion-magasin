#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

struct Client {
    int id_client;
    QString nom_client;
    QString telephone_client;

    Client() : id_client(-1) {} //constructeur par defaut qui initialise l'ID du client à -1
};

#endif // CLIENT_H
