#ifndef DSMTS_H
#define DSMTS_H

#include <QtNetwork/QtNetwork>
#include <QtWidgets/QDialog>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerResponse>
#include <Tufao/AbstractHttpServerRequestHandler>
#include <QNetworkReply>



struct TransportAddressProperties {
public:
    int metric;
    QTime timestamp;
};
Q_DECLARE_METATYPE(TransportAddressProperties)


struct TransportAddress{
    QString url;
};


struct AgentDescription {
    QString name;
    QStringList services;
    QStringList flags;
};


class AgentInfo{
public:
    QTime timeStamp;
    QVariantMap transportAddresses;
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
public:
    DiscoveryService(QObject *parent = 0);
    bool parseUrlPacket(const QByteArray msg);
    bool parseNotifyPacket(QByteArray msg);
    void sendMulticastNotifyPacket();
    QHash<QString, AgentInfo> platformAgents;

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    QHostAddress groupAddress;

    QHash<QString, AgentInfo> forwardedAgents;
};


class MessageTransportService: public QObject
{
    Q_OBJECT
public:
    //TcpClient tcpClient;
    //TcpServer tcpServer;
    QHash<QString, AgentInfo> platformAgents;
    QHash<QString, AgentInfo> remoteAgents;
    MessageTransportService(QObject *parent = 0);
    void writeHttpNotify(const QList<AgentInfo> agentsToBeNotified , const QHash<QString, QString> recipients, const QString sender);
    void writeHttpMessage(const QHash<QString, QString> recipients, const QString sender, QByteArray msg);

private slots:
    void handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);
signals:
    void httpNotifyReceived(QMap<QString,AgentInfo>);
    void httpMessageReceived(QByteArray);
    void needAgentList();
    void messageReady(QStringList, QByteArray);

private:
    Tufao::HttpServer server;
    QNetworkAccessManager *manager;
    void sendHttp(const QByteArray msg, const QString targetAgent, bool notify);
    QMap<QString, AgentInfo> processHttpNotify(QByteArray data);
    void processHttpMessage(QByteArray data);


};

class Platform: public QObject
{
    Q_OBJECT
public:
    MessageTransportService mts;
    DiscoveryService ds;
    Platform(QObject *parent = 0);
    QHash<QString, AgentInfo> platformAgents;
    QHash<QString, AgentInfo> remoteAgents;

public slots:
    void forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents);
    void handleHttpMessage(QByteArray msg);
    void sendAgentListToMts();
    void handleAgentMessage(QStringList recipients, QByteArray msg);
private:

};



#endif // DSMTS_H
