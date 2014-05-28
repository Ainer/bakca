#ifndef DSMTS_H
#define DSMTS_H

#include <QtNetwork/QtNetwork>
#include <QtWidgets/QDialog>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerResponse>
#include <Tufao/AbstractHttpServerRequestHandler>
#include <QNetworkReply>



enum MessageType{
    Notify, StandardMessage, Hello, Bye
};
const QStringList MESSAGE_TYPE_STRINGS = {"notify", "standardMessage", "hello", "bye"};

class Platform;

class DiscoveryService;

struct TransportAddressProperties {
public:
    int metric;
    QTime validUntil;
    DiscoveryService *sourceDs;
    QStringList origins;
};
Q_DECLARE_METATYPE(TransportAddressProperties)

/*
struct TransportAddress{
    QString url;
};
*/

struct AgentDescription {
    QString name;
    QStringList services;
    QStringList flags;
};


class AgentInfo{
public:
    QHash<QString, TransportAddressProperties> transportAddresses;
    AgentDescription desription;
    bool operator==(const AgentInfo &ai) const {
        return this->desription.name == ai.desription.name; //name should be unique, thus comparting two agents based on name
    }
};

class DiscoveryService: public QObject
{
    Q_OBJECT

private:
    Platform *m_platform;
    QUdpSocket *m_udpSocket;
    QHostAddress m_groupAddress; //odstran a uprav v kode
    void handleDatagram(QByteArray data); 
    void writeStatusMessage(QString type);
    void writeStatusMessage();
public:
    DiscoveryService(Platform *platform);
    ~DiscoveryService();
    bool parseNotifyPacket(QVariantMap msg);
    bool saveGWtoFile();
	bool loadGWfromFile();
private slots:
    void processPendingDatagrams();
public slots:
    void sendMulticastNotifyPacket();
};

class MessageTransportService: public QObject
{
    Q_OBJECT

private:
    Platform *m_platform;
    Tufao::HttpServer m_server;
    Tufao::HttpServerRequest *m_request;
	Tufao::HttpServerResponse *m_response;
    void sendHttp(const QByteArray msg, const QString targetAgent);
    void processXmlNotify(QByteArray data, QString sender);


public:
    MessageTransportService(Platform *platform);
    ~MessageTransportService();
    void writeHttpMessage(const QHash<QString, QString> recipients, const QString sender, QByteArray msg, MessageType type);
    void writeHttpStatusMessage(QString type);

private slots:
    void handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);
    void processHttpMessage(Tufao::HttpServerRequest &request, Tufao::HttpServerResponse &response);
public slots:
    void writeHttpNotify();

signals:
    void messageReady(QStringList, QByteArray);
    void notifyMessageReady(QByteArray);

};

class Platform: public QObject
{
    Q_OBJECT

private:
    QTimer *m_validationTimer;
    QTimer *m_localNetworkNotificationTimer;
    QTimer *m_forwardedAgentsNotificationTimer;
public:
    bool m_gateway = true;
    MessageTransportService m_mts;
    DiscoveryService m_ds;
    Platform(QObject *parent = 0);
    QHash<QString, AgentInfo> m_platformAgents;
    QHash<QString, AgentInfo> m_forwardedAgents;
    QStringList m_gatewayAgents;

    void handleStatusMessage(MessageType type, QString address, bool ds);

private slots:
    void handleAgentMessage(QStringList recipients, QByteArray msg);
    void eraseInvalidTransportAddresses();

};



#endif // DSMTS_H
