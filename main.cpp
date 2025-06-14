#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFontDatabase fontDB;
    int fontId = fontDB.addApplicationFont(":/fonts/fonts/Outfit-Regular.ttf");

    if(fontId == -1) {
        qDebug() << "Erreur: impossible de charger la police Outfit Regular";
    }

    fontId = fontDB.addApplicationFont(":/fonts/fonts/Outfit-Bold.ttf");

    if(fontId == -1) {
        qDebug() << "Erreur: impossible de charger la police Outfit Bold";
    }


    QFont outfitFont("Outfit", 10);
    a.setFont(outfitFont);

    MainWindow w;


    w.show();

    return a.exec();
}
