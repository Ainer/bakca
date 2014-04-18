#ifndef DSMTS_H
#define DSMTS_H

#include <QtNetwork/QtNetwork>
#include <QtWidgets/QDialog>

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
    QVector<AgentInfo> agents;
    QList<AgentInfo> PlatformAgents;
};

#endif // DSMTS_H
