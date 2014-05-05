#ifndef DSMTS_H
#define DSMTS_H

#include <QtNetwork/QtNetwork>
#include <QtWidgets/QDialog>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerResponse>
#include <Tufao/AbstractHttpServerRequestHandler>
#include <QNetworkReply>

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
    bool ParseUrlPacket(const char* msg);
    bool ParseNotifyPacket(const char* msg);
    void SendMulticastNotifyPacket();
    QMap<QString, QString>& getUrlFromName(QStringList names);
    QString getUrlFromName(QString name);

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

/////fill Qlists with indexes


class MessageTransportService: public QObject, public Tufao::AbstractHttpServerRequestHandler
{
    Q_OBJECT
public:
    Tufao::HttpServer server;
    //TcpClient tcpClient;
    //TcpServer tcpServer;
    MessageTransportService(QObject *parent = 0);
    void writeHttpNotifyMessage(const QList<AgentInfo> agentsToBeNotified, const QMap<QString, QString> recipients, const QString sender);
    void sendMessage(const QMap<QString, QString> recipients, const QByteArray msg, const QString sender);
    void sendHttpNotify(const QByteArray msg, const QString targetAgent);
    QMap<QString, AgentInfo> processHttpNotify(QByteArray data);

public slots:
    bool handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response) override;
    void httpNotifyReadyRead();
    void httpMessageReadyRead();

private:
    QNetworkReply *reply;
signals:
    void httpNotifyReceived(QByteArray data);


};

class Platform: public QObject
{
    Q_OBJECT
public:
    MessageTransportService MTS;
    DiscoveryService DS;
    Platform(QObject *parent = 0);

public slots:
    void forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents);
};



#endif // DSMTS_H
