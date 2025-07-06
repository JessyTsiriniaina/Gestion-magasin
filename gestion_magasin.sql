

PRAGMA foreign_keys=ON;
BEGIN TRANSACTION;

CREATE TABLE Produits (
    id_produit INTEGER PRIMARY KEY AUTOINCREMENT,
    nom_produit TEXT NOT NULL UNIQUE,
    unite_base TEXT NOT NULL,
    quantite_stock_en_unite_base REAL NOT NULL DEFAULT 0.0
);



CREATE TABLE UnitesVenteProduit (
    id_unite INTEGER PRIMARY KEY AUTOINCREMENT,
    id_produit INTEGER NOT NULL, 
    nom_unite TEXT NOT NULL, 
    facteur_de_conversion_vers_base REAL NOT NULL, 
    prix_par_unite REAL NOT NULL, 
    FOREIGN KEY (id_produit) REFERENCES Produits(id_produit) ON DELETE CASCADE
);



CREATE TABLE Clients (
    id_client INTEGER PRIMARY KEY AUTOINCREMENT, 
    nom_client TEXT NOT NULL, 
    telephone_client TEXT NULLABLE
);


CREATE TABLE Ventes (
    id_vente INTEGER PRIMARY KEY AUTOINCREMENT, 
    id_client INTEGER NULLABLE, 
    date_heure_vente TEXT NOT NULL, 
    montant_total REAL NOT NULL, 
    montant_remise REAL DEFAULT 0.0, 
    FOREIGN KEY (id_client) REFERENCES Clients(id_client) ON DELETE SET NULL
);


CREATE TABLE ComposantsVente (
    id_composant_vente INTEGER PRIMARY KEY AUTOINCREMENT, 
    id_vente INTEGER NOT NULL, 
    id_produit INTEGER NOT NULL,   --REDUNDANT??
    id_unite_vente INTEGER NOT NULL, 
    quantite_vendu REAL NOT NULL, 
    prix_de_vente REAL NOT NULL, 
    subtotal_composant REAL NOT NULL, 
    FOREIGN KEY (id_vente) REFERENCES Ventes(id_vente) ON DELETE CASCADE, 
    FOREIGN KEY (id_produit) REFERENCES Produits(id_produit) ON DELETE RESTRICT, 
    FOREIGN KEY (id_unite_vente) REFERENCES UnitesVenteProduit(id_unite) ON DELETE RESTRICT
);

COMMIT;
