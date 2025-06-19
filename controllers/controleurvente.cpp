#include "controleurvente.h"


#include <QDebug>


ControleurVente::ControleurVente(QObject *parent, ModeleClient* modeleClient, ModeleProduit* modeleProduit, ModeleVente* modeleVente) :
    QObject(parent),
    m_modeleClient(modeleClient),
    m_modeleProduit(modeleProduit),
    m_modeleVente(modeleVente),
    m_pourcentageRemise(0.0),
    m_montantRemiseManuel(0.0),
    m_dossierFacture("D:/facture")
{
    if(!m_modeleClient || !m_modeleProduit || !m_modeleVente) {
        qCritical() << "Le controleur vente manque de un ou plusieurs modele, la fonctionnalité est incomplet";
    }

    reinitialiserVente();

}



//--------- Gestion client ----------------------

QList<Client>  ControleurVente::rechercherClient(const QString& motCle)
{
    if (!m_modeleClient) return QList<Client>();
    return m_modeleClient->rechercherClient(motCle);
}

Client ControleurVente::getClientById(int id_client)
{
    if (!m_modeleClient) return Client();
    return m_modeleClient->getClientById(id_client);
}

void ControleurVente::setClientCourant(const Client& client)
{
    m_clientCourant = client;
}




//--------- Gestion produit ----------------------

QList<Produit> ControleurVente::rechercherProduit(const QString& motCle)
{
    if(!m_modeleProduit) return QList<Produit>();
    return m_modeleProduit->rechercherProduit(motCle);
}

Produit ControleurVente::getProduitById(int id_produit)
{
    if (!m_modeleProduit) return Produit();
    return m_modeleProduit->getProduitById(id_produit);
}

QList<UniteVenteProduit> ControleurVente::getUniteVenteProduitPourProduit(int id_produit)
{
    if (!m_modeleProduit) return QList<UniteVenteProduit>();
    return m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
}





//--------- Gestion panier ----------------------

bool ControleurVente::stockProduitSuffisant(int id_produit, int id_unite, double quantite){


    Produit produit = m_modeleProduit->getProduitById(id_produit);
    if (produit.id_produit == -1) {
        qWarning() << "Produit introuvable:" << id_produit;
        return false;
    }


    UniteVenteProduit unite;
    QList<UniteVenteProduit> unites = m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
    bool uniteTrouve = false;

    foreach (const UniteVenteProduit& u, unites) {
        if (u.id_unite == id_unite) {
            unite = u;
            uniteTrouve = true;
            break;
        }
    }

    if (!uniteTrouve) {
        qWarning() << "Unité introuvable:" << id_unite << "pour le produit ID:" << id_produit;
        return false;
    }

    double stockDemandeEnUniteBase = quantite * unite.facteur_de_conversion_vers_base;
    if (produit.quantite_stock_en_unite_base < stockDemandeEnUniteBase) {
        return false;
    }

    return  true;
}

bool ControleurVente::ajouterElementAuPanier(int id_produit, int id_unite, double quantite)
{
    if (!m_modeleProduit) return false;
    if (quantite <= 0) {
        qWarning() << "Quantité doit être positif.";
        return false;
    }

    Produit produit = m_modeleProduit->getProduitById(id_produit);
    if (produit.id_produit == -1) {
        qWarning() << "Produit introuvable:" << id_produit;
        return false;
    }

    UniteVenteProduit unite;
    QList<UniteVenteProduit> unites = m_modeleProduit->getUniteVenteProduitPourProduit(id_produit);
    bool uniteTrouve = false;

    foreach (const UniteVenteProduit& u, unites) {
        if (u.id_unite == id_unite) {
            unite = u;
            uniteTrouve = true;
            break;
        }
    }

    if (!uniteTrouve) {
        qWarning() << "Unité introuvable:" << id_unite << "pour le produit ID:" << id_produit;
        return false;
    }

    double stockDemandeEnUniteBase = quantite * unite.facteur_de_conversion_vers_base;
    if (produit.quantite_stock_en_unite_base < stockDemandeEnUniteBase) {
        return false;
    }

    AffichageElementPanier elementPanier;
    elementPanier.id_produit = id_produit;
    elementPanier.id_unite_vente = id_unite;
    elementPanier.quantite_vendu = quantite;
    elementPanier.prix_de_vente = unite.prix_par_unite;
    elementPanier.subtotal_composant = unite.prix_par_unite * quantite;


    elementPanier.nom_produit = produit.nom_produit;
    elementPanier.nom_unite = unite.nom_unite;

    m_elementsPanier.append(elementPanier);
    calculerEtEmettreTotals();
    //emit panierAChange();
    return true;
}

bool ControleurVente::supprimerElementDuPanier(int indexElementPanier)
{
    if (indexElementPanier < 0 || indexElementPanier >= m_elementsPanier.size()) {
        return false;
    }
    m_elementsPanier.removeAt(indexElementPanier);
    calculerEtEmettreTotals();
    //emit panierAChange();
    return true;
}


const QList<AffichageElementPanier>& ControleurVente::getElementsPanier() const
{
    return m_elementsPanier;
}



//--------- Total et remise ----------------------

void ControleurVente::calculerEtEmettreTotals()
{
    double subtotal = 0.0;

    foreach (const AffichageElementPanier& element, m_elementsPanier) {
        subtotal += element.subtotal_composant;
    }

    double montantRemiseActuel = 0.0;
    if (m_pourcentageRemise > 0) {
        montantRemiseActuel = (subtotal * m_pourcentageRemise) / 100.0;
        m_montantRemiseManuel = montantRemiseActuel; //Mettre à jour le montant manuel si le pourcentage est utilisé
    } else {
        montantRemiseActuel = m_montantRemiseManuel;
        // S'assurer que le montant de remise ne depasse pas le subtotal
        if (montantRemiseActuel > subtotal) montantRemiseActuel = subtotal;
        if (montantRemiseActuel < 0) montantRemiseActuel = 0;
        m_montantRemiseManuel = montantRemiseActuel; // mise à jour
    }

    double grandTotal = subtotal - montantRemiseActuel;

    QVariantMap totals;
    totals["subtotal"] = subtotal;
    totals["montantRemise"] = montantRemiseActuel;
    totals["grandTotal"] = grandTotal;
    totals["pourcentageRemise"] = m_pourcentageRemise;

    emit totalsAChange(totals);
}



void ControleurVente::setPourcentageRemise(double pourcentage)
{
    if (pourcentage < 0) pourcentage = 0;
    if (pourcentage > 100) pourcentage = 100;
    m_pourcentageRemise = pourcentage;
    m_montantRemiseManuel = 0;
    calculerEtEmettreTotals();
}

double ControleurVente::getPourcentageRemise() const
{
    return m_pourcentageRemise;
}

void ControleurVente::setMontantRemise(double montant)
{

    if (m_pourcentageRemise == 0) {
        if (montant < 0) montant = 0;
        m_montantRemiseManuel = montant;

        calculerEtEmettreTotals();
    } else {

        qDebug() << "Ne peut pas éditer manualement le montant de remise si le pourcentage est activé";

        calculerEtEmettreTotals();
    }
}

double ControleurVente::getMontantRemise() const
{

    double subtotal = 0.0;

    foreach (const AffichageElementPanier& element, m_elementsPanier) {
        subtotal += element.subtotal_composant;
    }

    if (m_pourcentageRemise > 0) {
        return qRound(((subtotal * m_pourcentageRemise) / 100.0) * 100.0) / 100.0;
    }

    return qMin(m_montantRemiseManuel, subtotal);
}



//--------- Finalisation ----------------------

bool ControleurVente::finaliserVente(Vente& donneeVenteComplet)
{
    if (!m_modeleVente) return false;
    if (m_elementsPanier.isEmpty()) {
        qWarning() << "Ne peut pas finaliser une vente vide";
        return false;
    }

    Vente enregistrementVente;
    enregistrementVente.id_client = (m_clientCourant.id_client > 0) ? m_clientCourant.id_client : -1;
    enregistrementVente.date_heure_vente = QDateTime::currentDateTime();

    double subtotal = 0.0;

    foreach(const AffichageElementPanier& elementPanier, m_elementsPanier) {
        subtotal += elementPanier.subtotal_composant;
    }

    double montantRemiseFinal = getMontantRemise();
    enregistrementVente.montant_total = subtotal - montantRemiseFinal;
    enregistrementVente.montant_remise = montantRemiseFinal;


    QList<ComposantVente> composantsVente;

    foreach (const AffichageElementPanier& elementPanier, m_elementsPanier) {
        ComposantVente composant;
        composant.id_produit = elementPanier.id_produit;
        composant.id_unite_vente = elementPanier.id_unite_vente;
        composant.quantite_vendu = elementPanier.quantite_vendu;
        composant.prix_de_vente = elementPanier.prix_de_vente;
        composant.subtotal_composant = elementPanier.subtotal_composant;
        composantsVente.append(composant);
    }

    if (m_modeleVente->enregistrerVente(enregistrementVente, composantsVente)) {
        donneeVenteComplet = enregistrementVente;
        Client client= m_modeleClient->getClientById(donneeVenteComplet.id_client);

        if(!genererFacture(donneeVenteComplet, client))
            qDebug() << "Erreur lors de la generation de facture";

        return true;


    }
    return false;
}


bool ControleurVente::genererFacture(const Vente& donneeVenteComplet, const Client& client)
{
    QTextDocument facture;
    QTextCursor cursor(&facture);

    QTextCharFormat normalFormat;
    normalFormat.setFont(QFont("Outfit", 10));

    QTextCharFormat boldFormat;
    boldFormat.setFont(QFont("Outfit", 10, QFont::Bold));

    QTextCharFormat titreFormat;
    titreFormat.setFont(QFont("Outfit", 16, QFont::Bold));

    cursor.movePosition(QTextCursor::Start);
    cursor.insertBlock();

    cursor.insertText("FACTURE\n", titreFormat);
    cursor.insertBlock();
    cursor.insertText("{Nom du magasin}\n", boldFormat);
    cursor.insertText("{Adresse}\n", normalFormat);
    cursor.insertText("{E-mail}\n", normalFormat);
    cursor.insertText("{Telephone}\n\n", normalFormat);


    cursor.insertText("Client :\n", boldFormat);
    if(client.id_client < 1)
        cursor.insertText(QString("Anonyme/temporaire\n\n"), normalFormat);
    else {
        cursor.insertText(QString(client.nom_client + "\n"), normalFormat);
        cursor.insertText(QString("Téléphone: " + client.telephone_client + "\n\n"), normalFormat);
    }

    QString numeroFacture = QString::number(donneeVenteComplet.id_vente);
    QString dateHeureFacture = donneeVenteComplet.date_heure_vente.toString("dd/MM/yyyy à hh:mm");
    cursor.insertText(QString("Facture n° : " + numeroFacture + "\n"), normalFormat);
    cursor.insertText(QString("Date et heure: %1\n\n").arg(dateHeureFacture), normalFormat);


    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(5);
    tableFormat.setAlignment(Qt::AlignCenter);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));

    QStringList headers;
    headers << "Nom du produit" << "Unité" << "Prix unitaire" << "Quantité"  << "Subtotal";

    QTextTable *table = cursor.insertTable(m_elementsPanier.count() + 1, headers.count(), tableFormat);


    for (int col = 0; col < headers.size(); col++) {
        QTextTableCell cellule = table->cellAt(0, col);
        QTextCursor cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(headers[col], boldFormat);
    }


    int ligne = 1;
    double total = 0;
    foreach(const AffichageElementPanier& element, m_elementsPanier) {
        int col = 0;

        QTextTableCell cellule = table->cellAt(ligne, col++);
        QTextCursor cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(element.nom_produit, normalFormat);

        cellule = table->cellAt(ligne, col++);
        cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(element.nom_unite , normalFormat);

        cellule = table->cellAt(ligne, col++);
        cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(QString(QString::number(element.prix_de_vente) + " Ar"), normalFormat);

        cellule = table->cellAt(ligne, col++);
        cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(QString::number(element.quantite_vendu), normalFormat);

        cellule = table->cellAt(ligne, col++);
        cellCursor = cellule.firstCursorPosition();
        cellCursor.insertText(QString(QString::number(element.subtotal_composant) + " Ar"), normalFormat);

        ligne++;
        total += element.subtotal_composant;
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();


    cursor.insertText(QString("\nTotal: " + QString::number(total) + " Ar\n"), normalFormat);
    cursor.insertText(QString("Remise: " + QString::number(donneeVenteComplet.montant_remise) + " Ar\n"), normalFormat);
    cursor.insertText(QString("Grand total: " + QString::number(donneeVenteComplet.montant_total) + " Ar\n"), boldFormat);


    cursor.insertText(QString("\n\n\n Merci beaucoup! A bientôt!"), normalFormat);


    // Générer le PDF
    QPrinter printer(QPrinter::HighResolution);

    QDir().mkpath(m_dossierFacture);
    QString cheminFichier = m_dossierFacture + "/" + QString::number(donneeVenteComplet.id_vente) + ".pdf";

    if (!cheminFichier.isEmpty()) {
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(cheminFichier);
        facture.print(&printer);
        qDebug() << "Facture générée :" << cheminFichier;

        QDesktopServices::openUrl(QUrl::fromLocalFile(cheminFichier));

        return true;
    }

    return false;
}






void ControleurVente::reinitialiserVente()
{
    m_elementsPanier.clear();
    m_pourcentageRemise = 0.0;
    m_montantRemiseManuel = 0.0;

    calculerEtEmettreTotals();
}
