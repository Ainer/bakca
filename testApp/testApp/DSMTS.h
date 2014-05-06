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
    int metric;
    QTime timestamp;
};


struct TransportAddress {
    QString url;
    TransportAddressProperties properties;

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
    bool parseUrlPacket(const QByteArray msg);
    bool parseNotifyPacket(const char* msg);
    void sendMulticastNotifyPacket();
    QList<AgentInfo> platformAgents;

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    QHostAddress groupAddress;

    QList<AgentInfo> forwardedAgents;
};


class MessageTransportService: public QObject
{
    Q_OBJECT
public:
    Tufao::HttpServer server;
    QNetworkAccessManager *manager;
    //TcpClient tcpClient;
    //TcpServer tcpServer;
    MessageTransportService(QObject *parent = 0);
    void writeHttpNotify(const QList<AgentInfo> agentsToBeNotified , const QMap<QString, QString> recipients, const QString sender);
    void writeHttpMessage(const QMap<QString, QString> recipients, const QString sender, QByteArray msg);
    void sendHttp(const QByteArray msg, const QString targetAgent, bool notify);
    QMap<QString, AgentInfo> processHttpNotify(QByteArray data);


public slots:
    void handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response);
signals:
    void httpNotifyReceived(QMap<QString,AgentInfo>);


};

class Platform: public QObject
{
    Q_OBJECT
public:
    MessageTransportService mts;
    DiscoveryService ds;
    Platform(QObject *parent = 0);

public slots:
    void forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents);
};



#endif // DSMTS_H
