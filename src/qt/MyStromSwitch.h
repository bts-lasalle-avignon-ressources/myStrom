#ifndef MyStromSwitch_H
#define MyStromSwitch_H

#include <QtWidgets>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QList>
#include <QUdpSocket>

/**
 * @def CONFIGURATION_APPLICATION
 * @brief Le nom du fichier de configuration
 */
#define CONFIGURATION_APPLICATION "config.ini"

/**
 * @def PORT_UDP_MYSTROM
 * @brief Le port pour la détection des appareils myStrom
 */
#define PORT_UDP_MYSTROM 7979

/**
 * @class MyStromSwitch
 * @brief La fenêtyre principale de l'application
 */
class MyStromSwitch : public QWidget
{
    Q_OBJECT

  private:
    // Widgets
    // Les layouts
    QVBoxLayout* layoutPrincipal;
    QVBoxLayout* layoutLabels;
    QHBoxLayout* layoutValeurs;
    QHBoxLayout* layoutBoutons;
    // Les boutons
    QPushButton* boutonDecouvrir;
    QPushButton* boutonLire;
    QPushButton* boutonEteindre;
    QPushButton* boutonAllumer;
    // Les labels
    QLabel* titre;
    QLabel* urlRequete;
    QLabel* reponseEtat;
    // QLabel* reponseJson;
    QTextEdit* reponseJson;
    QLabel*    power;
    QLabel*    Ws;
    QLabel*    relay;
    QLabel*    temperature;
    // Communication HTTP
    QNetworkAccessManager* accesReseau;
    QNetworkReply*         reponseReseau;
    QString                requeteApi;
    QString                adresseIPSwitch;
    // Communication UDP
    QUdpSocket* udpSocket;
    bool        decouverte;

    void chargerParametres();
    void enregistrerParametres();

  public:
    MyStromSwitch(QWidget* parent = nullptr);
    ~MyStromSwitch();

    void initialiserGUI();
    void initialiserWidgets();
    void initialiserSocketUDP();
    void initialiserSignauxSlots();
    void initialiserFenetrePrincipale();

  signals:
    void nouvelEtat();

  public slots:
    void decouvrirSwitch();
    void lireEtat();
    void eteindre();
    void allumer();
    void traiterReponseSwitch(QNetworkReply* reponseStation);
    void receptionnerDatagrammes();
};

#endif // MyStromSwitch_H
