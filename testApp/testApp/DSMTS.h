#ifndef DSMTS_H
#define DSMTS_H

#include <QtNetwork/QtNetwork>
#include <QtWidgets/QDialog>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerResponse>
#include <Tufao/AbstractHttpServerRequestHandler>

struct TransportAddress {
      QString url;
      int metric;
};

struct AgentDescription {
      QString name;
      QStringList services;
      QStringList flags;
};

class AgentInfo{
public:
      TransportAddress address;
      AgentDescription desription;
      bool operator==(const AgentInfo &ai) const {
        return this->desription.name == ai.desription.name; //name should be unique, thus comparting two agents based on name
      }
};


class DiscoveryService: public QObject
{
        Q_OBJECT
public:
    DiscoveryService(QObject *parent = 0);
    bool ParseUrlPacket(const char* msg);
    bool ParseNotifyPacket(const char* msg);
    void SendNotifyPacket();

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    QHostAddress groupAddress;
    QList<int> PlatformAgents;
    QList<int> RemoteAgents;
    QList<int> LocalAgents;
    QList<AgentInfo> Agents;
};

class MessageTransportService: public QObject, public Tufao::AbstractHttpServerRequestHandler
{
    Q_OBJECT
public:
    Tufao::HttpServer server;
    MessageTransportService(QObject *parent = 0);
    void sendRequest(const QList<QString> recipients);

public slots:
    bool handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response) override;

private:
    QAbstractSocket *sock;


};

#endif // DSMTS_H
