#-------------------------------------------------
#
# Project created by QtCreator 2025-05-26T13:23:49
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gestion-magasin
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    database/databasemanager.cpp \
    ui/gestionproduitdialog.cpp \
    ui/gestionuniteventedialog.cpp \
    ui/gestionclientdialog.cpp \
    ui/reapprovisionnerstockdialog.cpp \
    models/modeleclient.cpp \
    models/modeleproduit.cpp \
    models/modelevente.cpp \
    models/modelerapport.cpp \
    controllers/controleurclient.cpp \
    controllers/controleurproduit.cpp \
    controllers/controleurvente.cpp \
    controllers/controleurrapport.cpp \
    ui/horizontaltabstyle.cpp

HEADERS  += mainwindow.h \
    database/databasemanager.h \
    ui/gestionproduitdialog.h \
    ui/gestionuniteventedialog.h \
    ui/gestionclientdialog.h \
    ui/reapprovisionnerstockdialog.h \
    models/modeleclient.h \
    models/client.h \
    models/produit.h \
    models/modeleproduit.h \
    models/vente.h \
    models/modelevente.h \
    models/modelerapport.h \
    controllers/controleurclient.h \
    controllers/controleurproduit.h \
    controllers/controleurvente.h \
    controllers/controleurrapport.h \
    ui/horizontaltabstyle.h

FORMS    += mainwindow.ui \
    gestionproduitdialog.ui \
    gestionuniteventedialog.ui \
    gestionclientdialog.ui \
    reapprovisionnerstockdialog.ui

RESOURCES += \
    resources/resources.qrc
