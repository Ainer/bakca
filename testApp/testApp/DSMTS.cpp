#include <QtNetwork/QtNetwork>
#include <Tufao/Headers>

#include "DSMTS.h"

const QString MULTICAST_ADDRESS = "239.255.43.21";
const int MULTICAST_PORT = 45454;

DiscoveryService::DiscoveryService(QObject *parent) : QObject(parent)
{
    groupAddress = QHostAddress(MULTICAST_ADDRESS);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, MULTICAST_PORT, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(groupAddress);
    //udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(0));

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));

    SendNotifyPacket();
}

void DiscoveryService::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        ParseNotifyPacket(datagram.data());
    }
}

bool DiscoveryService::ParseUrlPacket(const char* msg)
{
    TransportAddress tmpAddress;
    QString tmpString = QString::fromUtf8(msg);
    for (int i = 0; i < tmpString.length(); ++i)
    {
        if (tmpString.indexOf("://") != -1){
            tmpAddress.url = tmpString.left(tmpString.indexOf("://"));
        }
        QByteArray array = tmpString.left(tmpString.indexOf("://")).toUtf8();
        char* buffer = array.data();
        qDebug(buffer);
        array = tmpString.right(tmpString.indexOf("://")).toUtf8();
        buffer = array.data();
        qDebug(buffer);
    }
    return true;
}

bool DiscoveryService::ParseNotifyPacket(const char *msg)
{
    AgentInfo ai;
    qDebug(msg);
    QJsonDocument doc = QJsonDocument::fromJson(msg);
    QVariantMap message = doc.object().toVariantMap();
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        QVariantMap aiHash = it.value().toMap();
        ai.desription.name = aiHash["name"].toString();
        qDebug() << ai.desription.name;
        ai.desription.services = aiHash["services"].toStringList();
        ai.desription.flags = aiHash["flags"].toStringList();
        ai.address.url = aiHash["url"].toString();
        qDebug() << ai.address.url;
        ai.address.metric = aiHash["metric"].toInt();
        int tmpindex = Agents.indexOf(ai);
        if (tmpindex > -1){
            Agents[tmpindex] = ai;
            tmpindex = -1;
        } else {
            Agents.push_back(ai);
            LocalAgents.push_back(Agents.length()-1);
        }

        ++it;
    }


    return true;
}

void DiscoveryService::SendNotifyPacket()
{
    QVariantMap agentInfo;
    QVariantMap agent;

    foreach(AgentInfo ai, Agents){
        agent["name"] = QVariant(ai.desription.name);
        agent["services"] = QVariant(ai.desription.services);
        agent["flags"] = QVariant(ai.desription.flags);
        agent["url"] = QVariant(ai.address.url);
        agent["metric"] = QVariant(ai.address.metric+1);
        agentInfo[ai.desription.name] = agent;
    }

    agent["name"] = QVariant("ahoj");
    agent["services"] = QVariantList() << "count" << "subtract";
    agent["flags"] = QVariantList() << "0" << "1" << "0";
    agent["url"] = QVariant("http://ahojnegre");
    agent["metric"] = QVariant(5);
    agentInfo["martin"] = agent;

    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    //qDebug(doc.toJson());
    udpSocket->writeDatagram(doc.toJson(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);

}

MessageTransportService::MessageTransportService(QObject *parent) : QObject(parent)
{
    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     this, &MessageTransportService::handleRequest);

    server.listen(QHostAddress::Any, 8080);
}

bool MessageTransportService::handleRequest(Tufao::HttpServerRequest &request,
                                            Tufao::HttpServerResponse &response){


    response.writeHead(Tufao::HttpResponseStatus::OK);
    response.headers().replace("Content-Type", "text/plain");
    response.end("Done");

}

void MessageTransportService::sendRequest(const QList<QString> recipients){

}





















