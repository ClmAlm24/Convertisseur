#include "conversion.h"
#include <QPainter>
#include <QWidget>
#include <QResizeEvent>
#include <QVector>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QColor>
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>

Conversion::Conversion(QWidget *parent) : QWidget(parent)
{
    // Définir la taille initiale du widget (vous pouvez ajuster les valeurs selon vos besoins)
    //resize(500, 500);
    // Initialisez conversionsTableWidget
    conversionsTableWidget = new QTableWidget(0, 4, this);
    // Configurez le QTableWidget selon vos besoins, par exemple en ajoutant les en-têtes de colonnes
    conversionsTableWidget->setHorizontalHeaderLabels({"De", "A", "Valeur Entrée", "Valeur De Sortie"});
    conversionsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Rendre les en-têtes de colonnes non modifiables
    conversionsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);


    // Définissez la taille et la position du QTableWidget si nécessaire
    //conversionsTableWidget->resize(200, 100);
    conversionsTableWidget->move(400,80);

    setupDatabase();
    setupUI();
    connectSignalsSlots();
    loadConversions();
}
void Conversion::setupUI()
{
    // Créez les widgets nécessaires pour l'interface utilisateur
    // Par exemple, créez un QLabel pour afficher le résultat de la conversion
    conversionsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    resultLabel = new QLabel("0.0", this);
    // Créez des QComboBox pour les unités de conversion
    fromUnitComboBox = new QComboBox(this);
    toUnitComboBox = new QComboBox(this);
    // Créez un QLineEdit pour l'entrée de la valeur à convertir
    inputLineEdit = new QLineEdit(this);
    // Créez un QPushButton pour effectuer la conversion
    convertButton = new QPushButton("Convertir", this);
    resetButton = new QPushButton("Réinitialiser", this);
    convertButton->setObjectName("convertButton");
    resetButton->setObjectName("resetButton");


    // Créez un QTableWidget pour afficher les conversions précédentes
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    conversionsTableWidget->setSizePolicy(sizePolicy);
    // Configurez les QComboBox avec les options d'unités de conversion
    QStringList unitOptions = {"Ligne", "Pouce", "Pied", "Verge", "Mile", "Millimètre", "Centimètre", "Mètre", "Kilomètre"};
    fromUnitComboBox->addItems(unitOptions);
    toUnitComboBox->addItems(unitOptions);

    // Disposez les widgets dans une mise en page appropriée (par exemple, QVBoxLayout)
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Ajoutez le tableau d'affichage au layout et étirez-le pour qu'il prenne tout l'espace disponible
    mainLayout->addWidget(conversionsTableWidget);
    //mainLayout->addWidget(conversionsTableWidget, 10, Qt::AlignCenter); // Utilisez Qt::AlignCenter pour centrer le tableau
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(fromUnitComboBox);
    mainLayout->addWidget(inputLineEdit);
    mainLayout->addWidget(toUnitComboBox);
   mainLayout->addWidget(convertButton);
   mainLayout->addWidget(resetButton);



    mainLayout->setStretchFactor(conversionsTableWidget, 1);
    //setMaximumWidth(400);
    setLayout(mainLayout);    // Connectez les signaux et les slots pour gérer les interactions utilisateur
    //QFile styleFile(":/style/style.css");*/
    QFile styleFile(":assets/style/style.css");

    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());

    // Appliquer le style CSS à l'ensemble de l'application
    setStyleSheet(styleSheet);// Connectez les signaux et les slots pour gérer les interactions utilisateur
}

void Conversion::connectSignalsSlots()
{
    // Connectez les signaux et les slots pour gérer les interactions utilisateur
    connect(convertButton, &QPushButton::clicked, this, &Conversion::onConvertButtonClicked);
    connect(resetButton, &QPushButton::clicked, this, &Conversion::onResetButtonClicked);
}

double Conversion::convertValue(double inputValue, const QString& fromUnit, const QString& toUnit)
{
    // Liste des unités supportées avec leurs facteurs de conversion par rapport à l'unité de base (pieds)
    // Vous pouvez ajouter ou modifier les unités de conversion selon vos besoins
    QHash<QString, double> conversionFactors = {
        {"Ligne", 1.0 / 144.0},
        {"Pouce", 1.0 / 12.0},
        {"Pied", 1.0},
        {"Verge", 3.0},
        {"Mile", 5280.0},
        {"Millimètre", 1.0 / 304.8},
        {"Centimètre", 1.0 / 30.48},
        {"Mètre", 1.0 / 0.3048},
        {"Kilomètre", 1.0 / 0.0003048}
    };

    // Vérifier si les unités de conversion sont présentes dans la liste
    if (!conversionFactors.contains(fromUnit) || !conversionFactors.contains(toUnit))
    {
        qDebug() << "Unité de conversion non prise en charge.";
        return 0.0;
    }

    // Convertir en pieds comme unité de base
    double inputValueInFeet = inputValue * conversionFactors.value(fromUnit);

    // Convertir en unité de destination
    double outputValue = inputValueInFeet / conversionFactors.value(toUnit);

    return outputValue;
}

void Conversion::onConvertButtonClicked()
{
    // Récupérer la valeur d'entrée et les unités sélectionnées dans l'interface
    bool conversionOK;
    double inputValue = inputLineEdit->text().toDouble(&conversionOK);
    if (!conversionOK)
    {
        // Gérer le cas où la valeur d'entrée n'est pas un nombre valide
        // Par exemple, afficher un message d'erreur à l'utilisateur
        qDebug() << "Erreur : valeur d'entrée invalide.";
        return;
    }

    QString fromUnit = fromUnitComboBox->currentText();
    QString toUnit = toUnitComboBox->currentText();

    // Effectuez la conversion en utilisant la logique appropriée
    double result = convertValue(inputValue, fromUnit, toUnit);

    // Mettez à jour le résultat dans l'interface
    resultLabel->setText(QString::number(result));

    // Enregistrez la conversion dans la base de données
    saveConversion(fromUnit, toUnit, inputValue, result);

    // Rechargez les conversions précédentes depuis la base de données pour les afficher dans l'interface
    loadConversions();
}

void Conversion::onResetButtonClicked()
{
    int reply = frenchYesNoMsgBox("Attention !", "Vous voulez vraiment tout supprimer ?");
    if(reply == QMessageBox::Yes) {
        resetDatabase();
        loadConversions();
    }
}


void Conversion::saveConversion(const QString& fromUnit, const QString& toUnit, double inputValue, double outputValue)
{
    QSqlQuery query;
    QString insertQuery = "INSERT INTO conversions (from_unit, to_unit, input_value, output_value) "
                          "VALUES (:fromUnit, :toUnit, :inputValue, :outputValue)";
    query.prepare(insertQuery);
    query.bindValue(":fromUnit", fromUnit);
    query.bindValue(":toUnit", toUnit);
    query.bindValue(":inputValue", inputValue);
    query.bindValue(":outputValue", outputValue);

    if (!query.exec())
    {
        qDebug() << "Erreur lors de l'enregistrement de la conversion : " << query.lastError().text();
    }
}

void Conversion::loadConversions()

{
    // Effacez toutes les lignes existantes dans le QTableWidget
    conversionsTableWidget->clearContents();
    conversionsTableWidget->setRowCount(0);

    // Exécutez la requête SQL pour sélectionner toutes les données de la table "conversions"
    QSqlQuery query("SELECT DISTINCT * FROM conversions");
    QSet<QString> uniqueRows;
    int rowCount = 0; // Compteur de lignes pour la couleur de fond alternée
    // Parcourez les résultats de la requête en utilisant une boucle while
    while (query.next())
       {
           // Récupérez les valeurs des colonnes "from_unit", "to_unit", "input_value" et "output_value"
           QString fromUnit = query.value("from_unit").toString();
           QString toUnit = query.value("to_unit").toString();
           double inputValue = query.value("input_value").toDouble();
           double outputValue = query.value("output_value").toDouble();
           // Vérifiez si cette ligne existe déjà dans la structure temporaire
           QString rowKey = fromUnit + "|" + toUnit + "|" + QString::number(inputValue) + "|" + QString::number(outputValue);
           if (!uniqueRows.contains(rowKey))
           {
               // Insérez les valeurs dans le QTableWidget uniquement si elles ne sont pas en double
               int row = conversionsTableWidget->rowCount();
               conversionsTableWidget->insertRow(row);

               QTableWidgetItem* fromItem = new QTableWidgetItem(fromUnit);
               QTableWidgetItem* toItem = new QTableWidgetItem(toUnit);
               QTableWidgetItem* inputItem = new QTableWidgetItem(QString::number(inputValue));
               QTableWidgetItem* outputItem = new QTableWidgetItem(QString::number(outputValue));

               conversionsTableWidget->setItem(row, 0, fromItem);
               conversionsTableWidget->setItem(row, 1, toItem);
               conversionsTableWidget->setItem(row, 2, inputItem);
               conversionsTableWidget->setItem(row, 3, outputItem);

               // Ajoutez la ligne à la structure temporaire pour éviter les doublons
               if (rowCount % 2 == 0)
                          {
                              conversionsTableWidget->item(row, 0)->setBackground(QBrush(QColor("#e6e6e6")));
                              conversionsTableWidget->item(row, 1)->setBackground(QBrush(QColor("#e6e6e6")));
                              conversionsTableWidget->item(row, 2)->setBackground(QBrush(QColor("#e6e6e6")));
                              conversionsTableWidget->item(row, 3)->setBackground(QBrush(QColor("#e6e6e6")));
                          }

                          // Ajouter la ligne à la structure temporaire pour éviter les doublons
                          uniqueRows.insert(rowKey);

                          rowCount++;           }
       }


    // Affichez le contenu réel du QTableWidget (optionnel)
    for (int row = 0; row < conversionsTableWidget->rowCount(); ++row)
    {
        for (int col = 0; col < conversionsTableWidget->columnCount(); ++col)
        {
            QTableWidgetItem* item = conversionsTableWidget->item(row, col);
            if (item)
            {
                qDebug() << "Row:" << row << ", Column:" << col << ", Value:" << item->text();
            }
        }
    }
}
int Conversion::frenchYesNoMsgBox(QString const& title, QString const& text)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton* ouiButton =  msgBox.addButton("Oui", QMessageBox::YesRole);
    msgBox.addButton("Non", QMessageBox::NoRole);
    msgBox.setDefaultButton(ouiButton);
    msgBox.exec();
    if(msgBox.clickedButton() == ouiButton) return QMessageBox::Yes;
    else return QMessageBox::No;
}

void Conversion::resetDatabase()
{
    QSqlQuery query;
    QString deleteQuery = "DELETE FROM conversions";
    if (!query.exec(deleteQuery))
    {
        qDebug() << "Erreur lors de la suppression de la conversion : " << query.lastError().text();
    }
}

void Conversion::setupDatabase()
{
    // Configuration de la base de données


    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
   database.setDatabaseName("conversion.db");

    // Ouvrez la base de données
    database.open();

    if (!database.open())
    {
        qDebug() << "Impossible d'ouvrir la base de données.";
        return;
    }

    // Créez la table si elle n'existe pas déjà
    if (!createTable())
    {
        qDebug() << "Erreur lors de la création de la table.";
    }
    createTable();
}

bool Conversion::createTable()
{
    QSqlQuery query;
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS conversions ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                               "from_unit TEXT, "
                               "to_unit TEXT, "
                               "input_value REAL, "
                               "output_value REAL)";

    if (!query.exec(createTableQuery))
    {
        qDebug() << "Erreur lors de la création de la table : " << query.lastError().text();
        return false;
    }

    return true;
}

