#include <QtNetwork/QtNetwork>
#include <Tufao/Headers>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "DSMTS.h"

const QString MULTICAST_ADDRESS = "239.255.43.21";
const QString My_ADDRESS = "158.195.212.98";
const QString LOOPBACK = "127.0.0.1";
const int MULTICAST_PORT = 45454;

const QString NAME = "name";
const QString SERVICES = "services";
const QString FLAGS = "flags";
const QString TRANSPORT_ADDRESSES = "transportAddresses";
const QString URL = "url";
const QString METRIC = "metric";
const QString TIMESTAMP = "timestamp";


// ///////////////////////////////////////////////////////// DISCOVERY SERVICE /////////////////////////////////////////////////////////

DiscoveryService::DiscoveryService(QObject *parent) : QObject(parent)
{
    groupAddress = QHostAddress(MULTICAST_ADDRESS);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, MULTICAST_PORT, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(groupAddress);
    //udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(0));

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));

    //sendMulticastNotifyPacket();
}

void DiscoveryService::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        parseNotifyPacket(datagram);
    }
}

bool DiscoveryService::parseUrlPacket(const QByteArray msg)
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

bool DiscoveryService::parseNotifyPacket(QByteArray msg) // TODO CHANGE TO LIST
{
    AgentInfo ai;
    // qDebug(msg);
    QJsonDocument doc = QJsonDocument::fromBinaryData(msg);
    QVariantMap message = doc.object().toVariantMap();
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        QVariantMap aiMap = it.value().toMap();
        ai.desription.name = aiMap[NAME].toString();
        ai.desription.services = aiMap[SERVICES].toStringList();
        ai.desription.flags = aiMap[FLAGS].toStringList();
        ai.transportAddresses = aiMap[TRANSPORT_ADDRESSES].toMap();
        ai.timeStamp = aiMap[TIMESTAMP].toTime();
        ++it;


    }
    return true;
}

void DiscoveryService::sendMulticastNotifyPacket()  // TODO CHANGE TO LIST
{
    QVariantMap agentInfo;
    QVariantMap agent;

    QHash<QString, AgentInfo>::const_iterator it;

    for (it = platformAgents.constBegin(); it != platformAgents.constEnd(); ++it){
        agent[NAME] = QVariant(it.value().desription.name);
        agent[SERVICES] = QVariant(it.value().desription.services);
        agent[FLAGS] = QVariant(it.value().desription.flags);
        agent[TRANSPORT_ADDRESSES] =  it.value().transportAddresses;
        agent[TIMESTAMP] = QVariant(it.value().timeStamp);
        agentInfo[it.value().desription.name] = agent;
    }

    // ADD GW INFO && forwardedAgents if GW

    //ALL PLATFORM AGENTS HAVE THE SAME ADDRESS?

    for (it = forwardedAgents.constBegin(); it != forwardedAgents.constEnd(); ++it){
        agent[NAME] = QVariant(it.value().desription.name);
        agent[SERVICES] = QVariant(it.value().desription.services);
        agent[FLAGS] = QVariant(it.value().desription.flags);
        agent[TRANSPORT_ADDRESSES] = qVariantFromValue(it.value().transportAddresses);
        agentInfo[it.value().desription.name] = agent;
    }




    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    //qDebug() << doc.toJson();

    udpSocket->writeDatagram(doc.toBinaryData(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);

}

// ///////////////////////////////////////////////////////// MESSAGE TRANSPORT SERVICE /////////////////////////////////////////////////////////

MessageTransportService::MessageTransportService(QObject *parent) : QObject(parent)
{

    connect(&server, SIGNAL(requestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)),
            this,
            SLOT(handleRequest(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)));

    //tcpClient.connectToHost(My_ADDRESS, 1024);


    server.listen(QHostAddress::Any, 22222);
}

void MessageTransportService::handleRequest(Tufao::HttpServerRequest &request, Tufao::HttpServerResponse &response){
    QMap<QString, AgentInfo> agents;
    QByteArray data = request.readBody();
    if (data == "")
        return;

    if (request.url().path().contains("/AgentNotify")){
        response.writeHead(Tufao::HttpResponseStatus::OK);
        response.headers().replace("Content-Type", "text/plain");
        response.end("AgentList Received");
        QMap<QString, AgentInfo>::const_iterator it;
        agents = processHttpNotify(data);

        qDebug() << "DOSTAL SOM REQUEST";
        /* for (it = agents.constBegin(); it != agents.constEnd(); ++it){
            qDebug() << it.value().desription.name;
            qDebug() << it.value().desription.flags;
            qDebug() << it.value().desription.services;
        }*/

        //SEND LOCAL NOTIFY AFTER PROCESS


        emit httpNotifyReceived(agents);
    } else {

        //TODO PROCESS MSG
        response.writeHead(Tufao::HttpResponseStatus::NO_RESPONSE);
        response.headers().replace("Content-Type", "text/plain");
        response.end(":(");
        processHttpMessage(data);
    }


}

//TRY SIGNAL DATA()

void MessageTransportService::writeHttpNotify(const QList<AgentInfo> agentsToBeNotified,
                                              const QHash<QString, QString> recipients, const QString sender){

    QHash<QString, QString> rec = recipients;
    QHash<QString, QString>::const_iterator it;
    QString currentURL = "";

    while(!rec.empty()){
        QByteArray data;
        QXmlStreamWriter writer(&data);
        writer.setAutoFormatting(true);

        qDebug() << "PISEM SPRAVU";
        it = rec.constBegin();
        currentURL = it.value();
        QStringList keysToRemove;

        writer.writeStartDocument();
        writer.writeStartElement("envelope");
        writer.writeTextElement("sender", sender);
        writer.writeStartElement("recipients");

        for (;it != rec.constEnd(); ++it){
            if (it.value() == currentURL){
                writer.writeTextElement("recipient", it.key());
                keysToRemove.append(it.key());
            }
        }

        writer.writeEndElement(); //recipients

        writer.writeStartElement("agents");
        foreach(AgentInfo info, agentsToBeNotified)
        {
            writer.writeStartElement("agent");

            writer.writeTextElement(NAME, info.desription.name);


            writer.writeStartElement(FLAGS);
            foreach (QString flag, info.desription.flags)
            {
                writer.writeTextElement("flag", flag);
            }
            writer.writeEndElement();


            writer.writeStartElement(SERVICES);
            foreach (QString service, info.desription.services)
            {
                writer.writeTextElement("service", service);
            }
            writer.writeEndElement();

            writer.writeStartElement(TRANSPORT_ADDRESSES);
            for (QVariantMap::const_iterator it2 = info.transportAddresses.constBegin(); it2 != info.transportAddresses.constEnd(); ++it2){
                writer.writeStartElement("transportAddress");
                writer.writeTextElement(URL, it2.key());
                writer.writeTextElement(METRIC, it2.value().toString());
                writer.writeEndElement();//transportAddress
            }
            writer.writeEndElement();//transportAddresses

            writer.writeTextElement(TIMESTAMP, info.timeStamp.toString());

            writer.writeEndElement(); //agent

        }
        writer.writeEndElement(); //agents
        writer.writeEndElement(); //envelope
        writer.writeEndDocument();
        foreach(QString key, keysToRemove)
            rec.remove(key);
        keysToRemove.clear();

        // qDebug() << data;
        sendHttp(data, currentURL, true); //TODO MESSAGE


    }


}

void MessageTransportService::writeHttpMessage(const QHash<QString, QString> recipients, const QString sender, QByteArray msg)
{
    QHash<QString, QString> rec = recipients;
    QString currentURL = "";
    QHash<QString, QString>::const_iterator it = rec.constBegin();



    while(!rec.empty()){

        it = rec.constBegin();
        currentURL = it.value();
        QStringList keysToRemove;

        QByteArray data;
        QXmlStreamWriter writer(&data);

        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("envelope");
        writer.writeTextElement("sender", sender);
        writer.writeStartElement("recipients");

        for (;it != rec.constEnd(); ++it){
            if (it.value() == currentURL){
                writer.writeTextElement("recipient", it.key());
                keysToRemove.append(it.key());
            }
        }

        writer.writeEndElement(); //recipients

        writer.writeStartElement("message");

        writer.writeCDATA(msg);

        writer.writeEndElement(); //message

        writer.writeEndElement(); //envelope
        writer.writeEndDocument();

        qDebug() << data;

        foreach(QString key, keysToRemove)
            rec.remove(key);
        keysToRemove.clear();
        sendHttp(data, currentURL, false); //TODO MESSAGE
        //qDebug() << currentURL;
    }
}

void MessageTransportService::sendHttp(const QByteArray msg, const QString targetAgent, bool Notify)
{
    QUrl url(Notify ? targetAgent /*+ "/AgentNotify"*/ : targetAgent);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "envelope/xml");
    manager->post(request, msg);
    qDebug() << "POSLAL SOM REQUEST";
}

QMap<QString, AgentInfo> MessageTransportService::processHttpNotify(QByteArray data){
    QMap<QString, AgentInfo> agents;
    QDomDocument doc;
    doc.setContent(data);
    QDomElement element = doc.documentElement();
    QDomNodeList nodeList = element.elementsByTagName("agent");

    for(int ii = 0;ii < nodeList.count(); ii++)
    {
        AgentInfo info;

        // get the current one as QDomElement
        QDomElement el = nodeList.at(ii).toElement();

        QDomNode pEntries = el.firstChild();
        while(!pEntries.isNull()) {
            QDomElement peData = pEntries.toElement();
            QString tagNam = peData.tagName();

            if(tagNam == NAME) {
                info.desription.name = peData.text();
            }else if(tagNam == SERVICES) {
                for(int i = 0; i < peData.childNodes().count(); ++i){
                    info.desription.services << peData.childNodes().at(i).toElement().text();
                }
            }else if(tagNam == FLAGS) {
                for(int i = 0; i < peData.childNodes().count(); ++i){
                    info.desription.flags << peData.childNodes().at(i).toElement().text();
                }
            }else if(tagNam == TRANSPORT_ADDRESSES) {
                QDomNodeList taNodeList = peData.childNodes();
                for (int i = 0; i < taNodeList.length(); ++i){
                    QDomNode node = taNodeList.item(i).toElement().firstChild();
                    if (node.toElement().tagName() == URL && node.nextSibling().toElement().tagName() == METRIC)
                        info.transportAddresses[node.toElement().text()] = node.nextSibling().toElement().text().toInt();
                }

            }else if(tagNam == TIMESTAMP) {
                info.timeStamp =   QTime::fromString(peData.text());
            }

            pEntries = pEntries.nextSibling();
        }

        agents[info.desription.name] = info;
    }
    return agents;
}

void MessageTransportService::processHttpMessage(QByteArray data){
    emit needAgentList();

    QDomDocument doc;
    QStringList myAgents;
    QStringList forwardAgents;

    doc.setContent(data);


    QDomElement element = doc.documentElement();
    QDomNode node = element.namedItem("recipients");
    if (!node.isNull()){
        QDomNodeList nodeList = node.toElement().childNodes();
        for (int i = nodeList.length()-1; i >= 0; --i){
            if (platformAgents.contains(nodeList.item(i).toElement().text())){
                myAgents << nodeList.item(i).toElement().text();
            } else {
                forwardAgents << nodeList.item(i).toElement().text();
            }
        }
    }

    qDebug() << "My agents: " << myAgents;
    qDebug() << "Remote agents: " << forwardAgents;

    QByteArray msg;
    QString sender;
    QString url;
    int minMetric = 999;

    node = element.namedItem("sender");
    sender = node.toElement().text();
    node = element.namedItem("message");
    msg = node.toElement().firstChild().toCDATASection().nodeValue().toUtf8();

    QHash<QString, QString> recipients;
    foreach(QString agent, forwardAgents){

        for(QVariantMap::const_iterator it = remoteAgents[agent].transportAddresses.constBegin();
            it != remoteAgents[agent].transportAddresses.constEnd(); ++it){
            qDebug() << it.key();
            qDebug() << it.value();

            if (it.value().toInt() < minMetric){
                url = it.key();
                minMetric = it.value().toInt();
            }
        }


        recipients[agent] = url;
        url = "";
        minMetric = 999;
    }
    writeHttpMessage(recipients, sender, msg);




    emit messageReady(myAgents, msg);
}

// ///////////////////////////////////////////////////////// PLATFORM /////////////////////////////////////////////////////////

Platform::Platform(QObject *parent) : QObject(parent)
{
    //qDebug() << "Platforma";
    //connect(&MTS, SIGNAL(httpNotifyReceived(QByteArray)), this, SLOT(forwardHttpNotifyToDs(QMap<QString, AgentInfo>)));
    connect(&mts, SIGNAL(httpMessageReceived(QByteArray)), this, SLOT(handleHttpMessage(QByteArray)));
    connect(&mts, SIGNAL(needAgentList()), this, SLOT(sendAgentListToMts()));
    connect(&mts, SIGNAL(messageReady(QStringList,QByteArray)), this, SLOT(handleAgentMessage(QStringList,QByteArray)));
}

void Platform::handleHttpMessage(QByteArray msg){

}

void Platform::forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents)
{
    //todo
    return;
}

void Platform::sendAgentListToMts(){
    mts.platformAgents = platformAgents;
    mts.remoteAgents = remoteAgents;
}

void Platform::handleAgentMessage(QStringList recipients, QByteArray msg){
    qDebug() << recipients;
    qDebug() << msg;
}



































/*
// ///////////////////////////////////////////////////////// TCP Client /////////////////////////////////////////////////////////



TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), SLOT(writeData2()));
}

void TcpClient::connectToHost(QString host, int port)
{
    socket->connectToHost(host, port);
}

void TcpClient::writeData(QByteArray data)
{
    data = "ahoj";
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->write(IntToArray(data.size())); //write size of data
        socket->write(data); //write the data itself
    }
}

QByteArray TcpClient::IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

void TcpClient::writeData2(){
    writeData("ahoj");
}

// ///////////////////////////////////////////////////////// TCP SERVER /////////////////////////////////////////////////////////

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    connect(this, SIGNAL(dataReceived(QByteArray)), SLOT(readData(QByteArray)));
    //qDebug() << "Listening:" << server->listen(QHostAddress::Any, 1024);
}

void TcpServer::newConnection()
{
    while(server->hasPendingConnections())
    {
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        QByteArray *buffer = new QByteArray();
        qint32 *s = new qint32;
        *s = 0;
        buffers.insert(socket, buffer);
        sizes.insert(socket, s);
    }
}

void TcpServer::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *size = sizes.value(socket);
    socket->deleteLater();
    delete buffer;
    delete size;
}

void TcpServer::readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *s = sizes.value(socket);
    qint32 size = *s;
    while(socket->bytesAvailable() > 0)
    {
        buffer->append(socket->readAll());
        while ((buffer->count() >= 4 && size == 0) || (buffer->count() >= size && size > 0)) //While can process data, process it
        {
           // qDebug() << "Reading...";
            if (buffer->count() >= 4 && size == 0) //if size of data has received completely, then store it on our global variable
            {
                size = ArrayToInt(buffer->mid(0, 4));
                *s = size;
                buffer->remove(0, 4);
            }
            if (buffer->count() >= size && size > 0) // If data has received completely, then emit our SIGNAL with the data
            {
                QByteArray data = buffer->mid(0, size);
                buffer->remove(0, size);
                size = 0;
                *s = size;
                emit dataReceived(data);
            }
        }
    }
}

void TcpServer::readData(QByteArray data){
    //qDebug() << data;
    QDomDocument doc;
    if (doc.setContent(data))
       // qDebug() << "precitane";
}


qint32 TcpServer::ArrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}

*/
























