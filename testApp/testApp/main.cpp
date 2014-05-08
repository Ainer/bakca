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

    QHash<QString, QString> hash;
    Platform platform;

    TransportAddressProperties props;



    AgentInfo ahoj;

    ahoj.desription.name = "filip";
    ahoj.desription.flags << "nie" << "ano" << "nie";
    ahoj.desription.services << "0" << "1" << "1";

    props.metric = 3;
    props.timestamp = QTime::currentTime().addSecs(10);
    ahoj.transportAddresses["127.0.0.1:22222"] = props;

    ahoj.transportAddresses.clear();

    QHash<QString, AgentInfo> agents;
    QList<AgentInfo> toBeNotified;
    agents.insert(ahoj.desription.name,ahoj);
    toBeNotified.append(ahoj);

    props.metric = 5;
    props.timestamp = QTime::currentTime().addSecs(11);

    ahoj.desription.name = "magda";
    ahoj.desription.flags << "GW" << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("128.0.0.1:22452")] = props;

    agents.insert(ahoj.desription.name,ahoj);
    toBeNotified.append(ahoj);






    hash.insert("milos", "http://ahoj.net");
    hash.insert("roman", "http://127.0.0.1:22222");
    hash.insert("jaroslav", "http://127.0.0.1:22222");
    hash.insert("filip", "http://127.0.0.1:22222");

    platform.mts.writeHttpNotify(toBeNotified, hash, "Adko");
    //platform.mts.writeHttpMessage(hash, "Adko", "Toto je message");

    platform.ds.setPlatformAgents(agents);
    platform.platformAgents = agents;

    agents.clear();
    ahoj.transportAddresses.clear();

    props.metric = 2;
    props.timestamp = QTime::currentTime().addSecs(15);

    ahoj.desription.name = "roman";
    ahoj.desription.flags << "GW" << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("http://ahoj.sk")] = props;
    agents.insert(ahoj.desription.name, ahoj);

    ahoj.transportAddresses.clear();

    props.metric = 4;
    props.timestamp = QTime::currentTime().addSecs(22);

    ahoj.desription.name = "jaroslav";
    ahoj.desription.flags << "GW" << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("http://nie.sk")] = props;
    agents.insert(ahoj.desription.name, ahoj);

    //platform.forwardedAgents = agents;

    return a.exec();
}
