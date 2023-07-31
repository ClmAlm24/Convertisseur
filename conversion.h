#ifndef CONVERSION_H
#define CONVERSION_H

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




class Conversion : public QWidget
{
    Q_OBJECT
public:
    explicit Conversion(QWidget *parent = nullptr);



protected:


private:
    //std::vector<std::vector<QString>> texts; // Tableau 2D de QString
    void setupUI();
    void connectSignalsSlots();
    double convertValue(double inputValue, const QString& fromUnit, const QString& toUnit);
    void saveConversion(const QString& fromUnit, const QString& toUnit, double inputValue, double outputValue);
    void loadConversions();
    void resetDatabase();

    int frenchYesNoMsgBox(QString const& title, QString const& text);


    // Fonctions pour créer et gérer la base de données
    void setupDatabase();
    bool createTable();

    QSqlDatabase database;
    QLabel* fromLabel;
    QLabel* toLabel;
    QComboBox* fromUnitComboBox;
    QComboBox* toUnitComboBox;
    QLineEdit* inputLineEdit;
    QLabel* resultLabel;
    QPushButton* convertButton;
    QPushButton* resetButton;

    // Les données du tableau de conversion
    double inputValue;
    QString fromUnit;
    QString toUnit;
    QTableWidget *conversionsTableWidget;

private slots:
    void onConvertButtonClicked();
    void onResetButtonClicked();





};

#endif // CONVERSION_H
