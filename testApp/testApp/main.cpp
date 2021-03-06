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

    /*ahoj.desription.name = "filip";
   // ahoj.desription.flags << "nie" << "ano" << "nie";
    ahoj.desription.services << "0" << "1" << "1";

    props.metric = 3;
    props.validUntil = QTime::currentTime().addSecs(180);
    //props.sourceDs = &platform.ds;
    ahoj.transportAddresses["http://127.0.0.1:22222/filip"] = props;
    props.sourceDs = nullptr;*/
/*

    QHash<QString, AgentInfo> agents;
    QList<AgentInfo> toBeNotified;

    agents.insert(ahoj.desription.name,ahoj);
    toBeNotified.append(ahoj);
    ahoj.transportAddresses.clear();

    props.metric = 1;
    props.validUntil = QTime::currentTime().addSecs(180);
    props.sourceDs = &platform.m_ds;
    props.origins << "http://127.0.0.1:22222" << "http://128.0.0.1:22452";

    ahoj.desription.name = "magda";
    ahoj.desription.flags << "GW" << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("http://158.195.212.98:22222")] = props;

    agents.insert(ahoj.desription.name,ahoj);
    toBeNotified.append(ahoj);
    //platform.m_gatewayAgents << "http://158.195.212.98:22222";

    QHash<QString, QString> to;
    to["magda"] = "http://109.230.49.193:22222";

    platform.m_mts.writeHttpMessage(to, "misko", "Toto je sprava", MessageType::StandardMessage);


/*
    platform.m_platformAgents = agents;

    hash.insert("milos", "http://ahoj.net");
    hash.insert("roman", "http://127.0.0.1:22222");
    hash.insert("jaroslav", "http://127.0.0.1:22222");
    hash.insert("filip", "http://127.0.0.1:22222");


    platform.m_ds.saveGWtoFile();
    //platform.mts.writeHttpMessage(hash, "Adko", "Toto je message");


    agents.clear();
    ahoj.transportAddresses.clear();

    props.metric = 2;
    props.validUntil = QTime::currentTime().addSecs(180);

    ahoj.desription.name = "roman";
    ahoj.desription.flags << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("http://ahoj.sk")] = props;
    agents.insert(ahoj.desription.name, ahoj);

    ahoj.transportAddresses.clear();

    props.metric = 4;
    props.validUntil = QTime::currentTime().addSecs(180);

    ahoj.desription.name = "jaroslav";
    ahoj.desription.flags << "ano" << "ano";
    ahoj.desription.services << "1" << "1" << "1";
    ahoj.transportAddresses[QString("http://nie.sk")] = props;
    agents.insert(ahoj.desription.name, ahoj);

    platform.m_forwardedAgents = agents;*/

    return a.exec();
}
