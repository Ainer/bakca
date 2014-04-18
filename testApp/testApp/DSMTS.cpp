#include <QtNetwork/QtNetwork>

#include "DSMTS.h"

const QString MULTICAST_ADDRESS = "239.255.43.21";
const int MULTICAST_PORT = 45454;

DiscoveryService::DiscoveryService(QObject *parent) : QObject(parent)
{
    groupAddress = QHostAddress(MULTICAST_ADDRESS);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, MULTICAST_PORT, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(groupAddress);
    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(0));

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
    QString tmpString = QString::fromUtf8(msg).replace(' ', "");
    QByteArray array;
    char* buffer;

    QStringList agentList = tmpString.split('\n', QString::SkipEmptyParts);
    foreach(QString agent, agentList){
        QStringList infoSections = agent.split(';');
        AgentDescription tmpDesc;
        TransportAddress tmpAdd;
        AgentInfo tmpInfo;
        if (infoSections[0] != "")
            tmpDesc.name = infoSections[0];
        if (infoSections[1] != "")
            tmpDesc.flags = infoSections[1].split('&');
        if (infoSections[2] != "")
            tmpDesc.services = infoSections[2].split('&');
        if (infoSections[3] != "")
            tmpAdd.url = infoSections[3];
        tmpInfo.address = tmpAdd;
        tmpInfo.desription = tmpDesc;
        agents.push_back(tmpInfo);
    }
    for (int i=0; i < agents.count(); ++i)
    {

        array = agents[i].desription.name.toUtf8();
        buffer = array.data();
        qDebug(buffer);
        array = agents[i].address.url.toUtf8();
        buffer = array.data();
        qDebug(buffer);

    }
    return true;
}

void DiscoveryService::SendNotifyPacket()
{
    QByteArray msg;  //= "adamko;true&false&true&false;count&subtract;http://ahoj\nstefan;false&false;;udp://caukos";
    foreach(AgentInfo agent, PlatformAgents){
        msg += agent.desription.name +';'
                + agent.desription.flags.join('&') + ';'
                + agent.desription.services.join('&') + ';'
                + agent.address.url + '\n';
    }

    udpSocket->writeDatagram(msg.data(), msg.length(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);
}
