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
    Notify, StandardMessage, Bye
};
const QStringList MESSAGE_TYPE_STRINGS = {"notify", "standardMessage", "Bye"};



struct TransportAddressProperties {
public:
    int metric;
    QTime timestamp;
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


/*
class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);

public slots:
    void connectToHost(QString host, int port);
    void writeData(QByteArray data);
    void writeData2();

private slots:
    QByteArray IntToArray(qint32 source);

private:
    QTcpSocket *socket;
};

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

signals:
    void dataReceived(QByteArray data);

private slots:
    void newConnection();
    void disconnected();
    void readyRead();
    void readData(QByteArray data);
    qint32 ArrayToInt(QByteArray source);

private:
    QTcpServer *server;
    QHash<QTcpSocket*, QByteArray*> buffers; //We need a buffer to store data until block has completely received
    QHash<QTcpSocket*, qint32*> sizes; //We need to store the size to verify if a block has received completely
};

*/

class DiscoveryService: public QObject
{
    Q_OBJECT

private:
    QUdpSocket *udpSocket;
    QHostAddress groupAddress;
    QHash<QString, AgentInfo> forwardedAgents;
    QHash<QString, AgentInfo> platformAgents;
    void updateAgents(QHash<QString, AgentInfo> newAgents);
public:
    DiscoveryService(QObject *parent = 0);

    void setPlatformAgents(QHash<QString, AgentInfo> agents){platformAgents = agents;}

    bool parseUrlPacket(const QByteArray msg);
    bool parseNotifyPacket(QByteArray msg);
    void sendMulticastNotifyPacket();

signals:
    void forwardedAgentsUpdated(QHash<QString, AgentInfo>);

private slots:
    void processPendingDatagrams();

public slots:
    void processXmlNotify(QByteArray data);


};


class MessageTransportService: public QObject
{
    Q_OBJECT

private:
    Tufao::HttpServer server;
    QNetworkAccessManager *manager;
    void sendHttp(const QByteArray msg, const QString targetAgent, MessageType type);
    void processHttpMessage(QByteArray data);

public:
    //TcpClient tcpClient;
    //TcpServer tcpServer;
    QHash<QString, AgentInfo> platformAgents;
    QHash<QString, AgentInfo> forwardedAgents;
    MessageTransportService(QObject *parent = 0);
    void writeHttpNotify(const QList<AgentInfo> agentsToBeNotified , const QHash<QString, QString> recipients, const QString sender);
    void writeHttpMessage(const QHash<QString, QString> recipients, const QString sender, QByteArray msg, MessageType type);

private slots:
    void handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);

signals:
    void needAgentList();
    void messageReady(QStringList, QByteArray);
    void notifyMessageReady(QByteArray);

};

class Platform: public QObject
{
    Q_OBJECT
public:
    MessageTransportService mts;
    DiscoveryService ds;
    Platform(QObject *parent = 0);
    QHash<QString, AgentInfo> platformAgents;
    QHash<QString, AgentInfo> forwardedAgents;

private slots:
    void updateforwardedAgents(QHash<QString, AgentInfo> agents){forwardedAgents = agents;}
    void sendAgentListToMts();
    void handleAgentMessage(QStringList recipients, QByteArray msg);

};



#endif // DSMTS_H
