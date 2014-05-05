#include <QCoreApplication>
#include <Tufao/HttpServer>
#include <QtCore/QUrl>
#include <Tufao/HttpServerRequest>
#include <Tufao/Headers>
#include "DSMTS.h"

using namespace Tufao;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QMap<QString, QString> map;
    Platform platform;

    platform.DS.SendMulticastNotifyPacket();

    AgentInfo ahoj;

    ahoj.desription.name = "filip";
    ahoj.desription.flags << "nie" << "ano" << "neger";
    ahoj.desription.services << "0" << "1" << "1";
    ahoj.address.url = "http://niekto.sk";
    ahoj.address.metric = 2;

    QList<AgentInfo> agents;
    agents.append(ahoj);

    ahoj.desription.name = "magda";
    ahoj.desription.flags << "nie" << "ano" << "neger";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.address.url = "http://158.195.212.98:8080/AgentNotify";
    ahoj.address.metric = 5;

    agents.append(ahoj);




    map.insert("milos", "http://ahoj.sk/notify");
    map.insert("marek", "http://ahoj.sk/notify");
    map.insert("maros", "http://158.195.212.98:8080/AgentNotify");
    platform.MTS.writeHttpNotifyMessage(agents, map, "Adko");

    return a.exec();
}
