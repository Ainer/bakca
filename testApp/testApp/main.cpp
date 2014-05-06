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

    platform.ds.sendMulticastNotifyPacket();

    AgentInfo ahoj;

    ahoj.desription.name = "filip";
    ahoj.desription.flags << "nie" << "ano" << "nie";
    ahoj.desription.services << "0" << "1" << "1";
    ahoj.address.url = "http://niekto.sk";
    ahoj.address.properties.metric = 2;

    QList<AgentInfo> agents;
    agents.append(ahoj);

    ahoj.desription.name = "magda";
    ahoj.desription.flags << "nie" << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.address.url = "http://158.195.212.98";
    ahoj.address.properties.metric = 5;

    agents.append(ahoj);




    map.insert("milos", "http://ahoj.sk");
    map.insert("marek", "http://nie.sk");
    map.insert("maros", "http://158.195.212.98:22222");

    platform.mts.writeHttpNotify(agents, map, "Adko");
    //platform.mts.writeHttpMessage(map, "Adko", "Niggermaniak Marian");

    return a.exec();
}
