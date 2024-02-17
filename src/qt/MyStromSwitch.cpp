/**
 * @file MyStromSwitch.cpp
 * @brief Définition de la classe MyStromSwitch
 */

#include "MyStromSwitch.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkDatagram>

/**
 * @brief Constructeur de la classe MyStromSwitch
 * @fn MyStromSwitch::MyStromSwitch
 * @param parent nullptr pour la fenêtre comme fenêtre principale de l'application
 */
MyStromSwitch::MyStromSwitch(QWidget* parent) :
    QWidget(parent), accesReseau(new QNetworkAccessManager(this)), udpSocket(new QUdpSocket(this)),
    decouverte(false)
{
    qDebug() << Q_FUNC_INFO;
    chargerParametres();
    initialiserGUI();
    initialiserSocketUDP();
    initialiserSignauxSlots();
}

/**
 * @brief Destructeur de la classe MyStromSwitch
 *
 * @fn MyStromSwitch::~MyStromSwitch
 * @details Libère les ressources de l'application
 */
MyStromSwitch::~MyStromSwitch()
{
    enregistrerParametres();
    qDebug() << Q_FUNC_INFO;
}

/**
 * @brief Charge les paramètres de l'application
 *
 * @fn MyStromSwitch::chargerParametres
 * @details Charge les paramètres à partir du fichier config.ini
 */
void MyStromSwitch::chargerParametres()
{
    QSettings parametres(CONFIGURATION_APPLICATION, QSettings::IniFormat);
    adresseIPSwitch = parametres.value("Switch/adresseIPSwitch").toString();
    qDebug() << Q_FUNC_INFO << "adresseIPSwitch" << adresseIPSwitch;
}

/**
 * @brief Enregistre les paramètres de l'application
 *
 * @fn MyStromSwitch::enregistrerParametres
 * @details Enregistre les paramètres à partir du fichier config.ini
 */
void MyStromSwitch::enregistrerParametres()
{
    QSettings parametres(CONFIGURATION_APPLICATION, QSettings::IniFormat);
    parametres.setValue(QString("Switch") + "/adresseIPSwitch", adresseIPSwitch);
    qDebug() << Q_FUNC_INFO << "adresseIPSwitch" << adresseIPSwitch;
}

/**
 * @brief Initialise l'interface graphique
 *
 * @fn MyStromSwitch::initialiserGUI
 * @details Génère la page graphique d'interface
 */
void MyStromSwitch::initialiserGUI()
{
    initialiserWidgets();
    initialiserFenetrePrincipale();
}

/**
 * @brief Initialise les widgets
 *
 * @fn MyStromSwitch::initialiserWidgets
 * @details Initialise les widgets de l'IHM.
 */
void MyStromSwitch::initialiserWidgets()
{
    layoutPrincipal = new QVBoxLayout;
    layoutLabels    = new QVBoxLayout;
    layoutValeurs   = new QHBoxLayout;
    layoutBoutons   = new QHBoxLayout;

    boutonDecouvrir = new QPushButton("Découvrir", this);
    boutonLire      = new QPushButton("Lire", this);
    boutonEteindre  = new QPushButton("Éteindre", this);
    boutonAllumer   = new QPushButton("Allumer", this);

    if(!adresseIPSwitch.isEmpty())
    {
        boutonLire->setEnabled(true);
        boutonEteindre->setEnabled(true);
        boutonAllumer->setEnabled(true);
    }
    else
    {
        boutonLire->setEnabled(false);
        boutonEteindre->setEnabled(false);
        boutonAllumer->setEnabled(false);
    }

    titre = new QLabel(this);
    titre->setText("myStrom Switch");
    urlRequete  = new QLabel(this);
    reponseEtat = new QLabel(this);
    // reponseJson = new QLabel(this);
    reponseJson = new QTextEdit(this);
    reponseJson->setReadOnly(true);

    power = new QLabel(this);
    power->setText("-- W");
    Ws = new QLabel(this);
    Ws->setText("-- W");
    relay = new QLabel(this);
    relay->setText("??");
    temperature = new QLabel(this);
    temperature->setText("--.- °C");
}

/**
 * @brief Initialise la socket d'écoute UDP
 *
 * @fn MyStromSwitch::initialiserSocketUDP
 * @details Initialise la socket d'écoute UDP pour la détection des appareils myStrom
 */
void MyStromSwitch::initialiserSocketUDP()
{
    // Attachement locale de la socket UDP
    // udpSocket->bind(QHostAddress::LocalHost, PORT_UDP_MYSTROM);
    // ou
    // udpSocket->bind(QHostAddress::Any, PORT_UDP_MYSTROM); // ie "0.0.0.0"

    if(udpSocket->bind(QHostAddress::AnyIPv4, PORT_UDP_MYSTROM))
    {
        qDebug() << Q_FUNC_INFO;
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "erreur bind";
    }
}

/**
 * @brief Initialise les signaux et les slots
 *
 * @fn MyStromSwitch::initialiserSignauxSlots
 * @details Initialise les connects des signaux et des slots
 */
void MyStromSwitch::initialiserSignauxSlots()
{
    connect(boutonDecouvrir, SIGNAL(clicked(bool)), this, SLOT(decouvrirSwitch()));
    connect(boutonLire, SIGNAL(clicked(bool)), this, SLOT(lireEtat()));
    connect(boutonEteindre, SIGNAL(clicked(bool)), this, SLOT(eteindre()));
    connect(boutonAllumer, SIGNAL(clicked(bool)), this, SLOT(allumer()));
    connect(this, SIGNAL(nouvelEtat()), this, SLOT(lireEtat()));
    connect(accesReseau,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(traiterReponseSwitch(QNetworkReply*)));
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(receptionnerDatagrammes()));
}

/**
 * @brief Initialise la fenêtre principale
 * @fn MyStromSwitch::initialiserFenetrePrincipale
 */
void MyStromSwitch::initialiserFenetrePrincipale()
{
    layoutLabels->addWidget(titre);
    layoutLabels->addWidget(urlRequete);
    layoutLabels->addWidget(reponseEtat);
    layoutLabels->addWidget(reponseJson);
    layoutLabels->addStretch();
    layoutValeurs->addStretch();
    layoutValeurs->addWidget(power);
    layoutValeurs->addStretch();
    layoutValeurs->addWidget(Ws);
    layoutValeurs->addStretch();
    layoutValeurs->addWidget(relay);
    layoutValeurs->addStretch();
    layoutValeurs->addWidget(temperature);
    layoutValeurs->addStretch();
    layoutBoutons->addWidget(boutonDecouvrir);
    layoutBoutons->addWidget(boutonLire);
    layoutBoutons->addWidget(boutonEteindre);
    layoutBoutons->addWidget(boutonAllumer);
    layoutPrincipal->addLayout(layoutLabels);
    layoutPrincipal->addLayout(layoutValeurs);
    layoutPrincipal->addLayout(layoutBoutons);

    setLayout(layoutPrincipal);
    // QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    // resize(screenGeometry.width(), screenGeometry.height());
    resize(820, 820);
}

void MyStromSwitch::decouvrirSwitch()
{
    qDebug() << Q_FUNC_INFO;
    decouverte = true;
}

void MyStromSwitch::lireEtat()
{
    qDebug() << Q_FUNC_INFO;
    QString         api = "http://" + adresseIPSwitch + "/report";
    QUrl            url = QUrl(api);
    QNetworkRequest requeteGet;
    requeteGet.setUrl(url);
    requeteGet.setRawHeader("Content-Type", "application/json");
    requeteGet.setRawHeader(QByteArray("Accept"), QByteArray("application/json"));
    qDebug() << Q_FUNC_INFO << "url" << url;
    requeteApi = api;
    urlRequete->setText(api);
    /*QSslConfiguration conf = requeteGet.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    requeteGet.setSslConfiguration(conf);*/
    accesReseau->get(requeteGet);
}

void MyStromSwitch::eteindre()
{
    qDebug() << Q_FUNC_INFO;
    // @see http://[IP]/toggle
    QString         api = "http://" + adresseIPSwitch + "/relay?state=0";
    QNetworkRequest requeteGet;
    QUrl            url = QUrl(api);
    requeteGet.setUrl(url);
    requeteGet.setRawHeader("Content-Type", "application/json");
    requeteGet.setRawHeader(QByteArray("Accept"), QByteArray("application/json"));
    qDebug() << Q_FUNC_INFO << "url" << url;
    requeteApi = api;
    urlRequete->setText(api);
    /*QSslConfiguration conf = requeteGet.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    requeteGet.setSslConfiguration(conf);*/
    accesReseau->get(requeteGet);
}

void MyStromSwitch::allumer()
{
    qDebug() << Q_FUNC_INFO;
    // @see http://[IP]/toggle
    QString         api = "http://" + adresseIPSwitch + "/relay?state=1";
    QNetworkRequest requeteGet;
    QUrl            url = QUrl(api);
    requeteGet.setUrl(url);
    requeteGet.setRawHeader("Content-Type", "application/json");
    requeteGet.setRawHeader(QByteArray("Accept"), QByteArray("application/json"));
    qDebug() << Q_FUNC_INFO << "url" << url;
    requeteApi = api;
    urlRequete->setText(api);
    /*QSslConfiguration conf = requeteGet.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    requeteGet.setSslConfiguration(conf);*/
    accesReseau->get(requeteGet);
}

/**
 * @brief Slot qui traite les réponses renvoyées par le pont Hue
 * @fn MyStromSwitch::traiterReponseHue
 */
void MyStromSwitch::traiterReponseSwitch(QNetworkReply* reponse)
{
    qDebug() << Q_FUNC_INFO << "requeteApi" << requeteApi;
    if(reponse->error() != QNetworkReply::NoError)
    {
        qDebug() << Q_FUNC_INFO << "erreur" << reponse->error();
        qDebug() << Q_FUNC_INFO << "erreur" << reponse->errorString();
        reponseEtat->setText(reponse->errorString());
        return;
    }
    QByteArray donneesReponse = reponse->readAll();
    qDebug() << Q_FUNC_INFO << "donneesReponse" << donneesReponse;
    qDebug() << Q_FUNC_INFO << "donneesReponse" << donneesReponse.size();
    if(donneesReponse.size())
        reponseJson->append(donneesReponse);

    QJsonDocument documentJson = QJsonDocument::fromJson(donneesReponse);

    if(requeteApi.contains("report"))
    {
        /*
            {"power":0,"Ws":0,"relay":true,"temperature":21.5}
        */
        QJsonObject json = documentJson.object();
        qDebug() << Q_FUNC_INFO << "power" << json["power"].toDouble();
        qDebug() << Q_FUNC_INFO << "Ws" << json["Ws"].toDouble();
        qDebug() << Q_FUNC_INFO << "relay" << json["relay"].toBool();
        qDebug() << Q_FUNC_INFO << "temperature" << json["temperature"].toDouble();
        power->setText(QString::number(json["power"].toDouble()) + " W");
        Ws->setText(QString::number(json["Ws"].toDouble()) + " W");
        if(json["relay"].toBool())
            relay->setText("ON");
        else
            relay->setText("OFF");
        temperature->setText(QString::number(json["temperature"].toDouble()) + " °C");
        requeteApi.clear();
    }
    else if(requeteApi.contains("relay"))
    {
        requeteApi = "http://" + adresseIPSwitch + "/report";
        emit nouvelEtat();
    }
}

void MyStromSwitch::receptionnerDatagrammes()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagramme = udpSocket->receiveDatagram();
        if(!decouverte)
            return;
        qDebug() << Q_FUNC_INFO << "emetteurAdresse" << datagramme.senderAddress().toString()
                 << "emetteurPort" << datagramme.senderPort();
        qDebug() << Q_FUNC_INFO << "destinationAdresse"
                 << datagramme.destinationAddress().toString() << "destinationPort"
                 << datagramme.destinationPort();
        qDebug() << Q_FUNC_INFO << "nbOctets" << datagramme.data().size() << "donneesDatagramme"
                 << datagramme.data().toHex();

        reponseJson->append("emetteurAdresse = " + datagramme.senderAddress().toString() +
                            " - emetteurPort = " + QString::number(datagramme.senderPort()));
        reponseJson->append(
          "destinationAdresse = " + datagramme.destinationAddress().toString() +
          " - destinationPort = " + QString::number(datagramme.destinationPort()));
        reponseJson->append("donneesDatagramme = " + datagramme.data().toHex() + " (longueur = " +
                            QString::number(datagramme.data().size()) + " octets)");

        if(datagramme.destinationAddress().toString() == "255.255.255.255" &&
           datagramme.destinationPort() == PORT_UDP_MYSTROM)
        {
            adresseIPSwitch = datagramme.senderAddress().toString();
            decouverte      = false;
            return;
        }
    }
}
