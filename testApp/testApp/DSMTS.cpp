#include <QtNetwork/QtNetwork>
#include <Tufao/Headers>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "DSMTS.h"

const QString MULTICAST_ADDRESS = "239.255.43.21";
const QString MY_ADDRESS = "http://158.195.212.98:22222";
const QString LOOPBACK = "127.0.0.1";
const int MULTICAST_PORT = 45454;

const QString NAME = "name";
const QString SERVICES = "services";
const QString FLAGS = "flags";
const QString TRANSPORT_ADDRESSES = "transportAddresses";
const QString URL = "url";
const QString METRIC = "metric";
const QString VALID_UNTIL = "validUntil";


// ///////////////////////////////////////////////////////// DISCOVERY SERVICE /////////////////////////////////////////////////////////

DiscoveryService::DiscoveryService(Platform *platform)
    : QObject(platform)
    , platform(platform)
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

inline QJsonObject fromProperties(TransportAddressProperties props){
    QJsonObject object;
    QJsonValue value;
    value = props.metric;
    object.insert(METRIC, value);
    value = props.validUntil.toString();
    object.insert(VALID_UNTIL, value);

    return object;
}

inline TransportAddressProperties fromJsonObject(QJsonObject object){
    TransportAddressProperties props;
    QJsonValue value;
    value = object[METRIC];
    props.metric = value.toInt();
    value = object[VALID_UNTIL];
    props.validUntil = QTime::fromString(value.toString());
}

bool DiscoveryService::parseNotifyPacket(QByteArray msg) // TODO CHANGE TO LIST
{

    // CHANGE AFTER TEST
    platform->platformAgents.clear();

    AgentInfo ai;
    // qDebug(msg);
    QJsonDocument doc = QJsonDocument::fromBinaryData(msg);
    QVariantMap message = doc.object().toVariantMap();
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        QVariantMap aiMap = it.value().toMap();
        ai.desription.name = aiMap[NAME].toString();
        if (platform->platformAgents.contains(ai.desription.name)){
            ++it;
            continue;
        }
        QVariantMap tmpMap = aiMap[TRANSPORT_ADDRESSES].toMap();
        for (QVariantMap::const_iterator it2 = tmpMap.constBegin(); it2 != tmpMap.constEnd(); ++it2){
            QVariantMap tmpMap2 = it2.value().toMap();
            TransportAddressProperties props;
            props.metric = tmpMap2[METRIC].toInt();
            props.validUntil = QTime::fromString(tmpMap2[VALID_UNTIL].toString());
            props.sourceDs = this;
            ai.transportAddresses[it2.key()] = props;
        }
        ai.desription.services = aiMap[SERVICES].toStringList();
        ai.desription.flags = aiMap[FLAGS].toStringList();

        ++it;



    }
    return true;
}

void DiscoveryService::sendMulticastNotifyPacket()
{
    //change

    QVariantMap agentInfo;
    QVariantMap agent;

    QHash<QString, AgentInfo>::const_iterator it;

    for (it = platform->platformAgents.constBegin(); it != platform->platformAgents.constEnd(); ++it){
        qDebug() << it.key();
        QVariantMap addresses;
        QHash<QString, TransportAddressProperties>::const_iterator it2 = it.value().transportAddresses.constBegin();
        while (it2 != it.value().transportAddresses.constEnd()){
            addresses[it2.key()] = fromProperties(it2.value()).toVariantMap();
            ++it2;
        }
        if (addresses.empty())
            continue;
        agent[NAME] = QVariant(it.value().desription.name);
        agent[SERVICES] = QVariant(it.value().desription.services);
        agent[FLAGS] = QVariant(it.value().desription.flags);
        agent[TRANSPORT_ADDRESSES] =  addresses;
        agentInfo[it.value().desription.name] = agent;
    }

    // ADD GW INFO && forwardedAgents if GW

    if (platform->gateway)
        for (it = platform->forwardedAgents.constBegin(); it != platform->forwardedAgents.constEnd(); ++it){
            QVariantMap addresses;
            QHash<QString, TransportAddressProperties>::const_iterator it2 = it.value().transportAddresses.constBegin();
            while (it2 != it.value().transportAddresses.constEnd()){
                if (it2.value().sourceDs != this)
                    addresses[MY_ADDRESS + "/routedAgents"] = fromProperties(it2.value()).toVariantMap();
                ++it2;
            }
            if (addresses.empty())
                continue;
            agent[NAME] = QVariant(it.value().desription.name);
            agent[SERVICES] = QVariant(it.value().desription.services);
            agent[FLAGS] = QVariant(it.value().desription.flags);
            agent[TRANSPORT_ADDRESSES] =  addresses;
            agentInfo[it.value().desription.name] = agent;
        }

    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    qDebug() << doc.toJson();

    udpSocket->writeDatagram(doc.toBinaryData(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);

}

// ///////////////////////////////////////////////////////// MESSAGE TRANSPORT SERVICE /////////////////////////////////////////////////////////

MessageTransportService::MessageTransportService(Platform *platform)
    : QObject(platform)
    , platform(platform)
{

    connect(&server, SIGNAL(requestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)),
            this,
            SLOT(handleRequest(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)));


    //tcpClient.connectToHost(My_ADDRESS, 1024);


    server.listen(QHostAddress::Any, 22222);
}

void MessageTransportService::handleRequest(Tufao::HttpServerRequest &request, Tufao::HttpServerResponse &response){
    //TODO PROCESS MSG
    m_request = &request;
    connect (&request , SIGNAL(end()), this, SLOT(processHttpMessage()));
    qDebug() << "my address: " << request.socket().localAddress().toString();
    qDebug() << "peer address: " << request.socket().peerAddress().toString();
    response.writeHead(Tufao::HttpResponseStatus::NO_RESPONSE);
    response.headers().replace("Content-Type", "text/plain");
    response.end(":(");
}

inline void inserAgents(QXmlStreamWriter *writer, AgentInfo info, bool forwarded){
    writer->writeStartElement("agent");

    writer->writeTextElement(NAME, info.desription.name);


    writer->writeStartElement(FLAGS);
    foreach (QString flag, info.desription.flags)
    {
        writer->writeTextElement("flag", flag);
    }
    writer->writeEndElement();


    writer->writeStartElement(SERVICES);
    foreach (QString service, info.desription.services)
    {
        writer->writeTextElement("service", service);
    }
    writer->writeEndElement();

    writer->writeStartElement("routes");
    for (QHash<QString, TransportAddressProperties>::const_iterator it2 = info.transportAddresses.constBegin();
         it2 != info.transportAddresses.constEnd(); ++it2){
        writer->writeStartElement("route");
        writer->writeTextElement(METRIC, QString::number(it2.value().metric));
        writer->writeTextElement(VALID_UNTIL, it2.value().validUntil.toString());
        writer->writeTextElement("transportAddress", forwarded ? MY_ADDRESS + "/forwardedAgents" : it2.key());
        writer->writeEndElement(); // route
    }
    writer->writeEndElement(); // routes
    writer->writeEndElement(); //agent
}

void MessageTransportService::writeHttpNotify(){

    QByteArray data;
    QXmlStreamWriter *writer = new QXmlStreamWriter(&data);
    writer->setAutoFormatting(true);

    qDebug() << "PISEM SPRAVU";

    writer->writeStartDocument();
    writer->writeStartElement("agents");

    foreach(AgentInfo info, platform->platformAgents)
        inserAgents(writer, info, false);

    foreach(AgentInfo info, platform->forwardedAgents)
        inserAgents(writer, info, true);



    writer->writeEndElement(); //agents
    writer->writeEndDocument();

    qDebug() << data;

    QHash<QString, QString> recipients;
    foreach(QString address, platform->gatewayAgents)
        recipients[address] = address;
    qDebug() << recipients.keys();

    writeHttpMessage(recipients  //GW agents
                     ,MY_ADDRESS, data, MessageType::Notify);//prehod na spravne miesto
}

void MessageTransportService::writeHttpMessage(const QHash<QString, QString> recipients, const QString sender, QByteArray msg,
                                               MessageType type)
{
    QHash<QString, QString> rec = recipients;
    QString currentURL = "";
    QHash<QString, QString>::const_iterator it = rec.constBegin();
    if (rec.empty() || sender.isEmpty() || msg.isEmpty())
        return;


    while(!rec.empty()){

        it = rec.constBegin();
        currentURL = it.value();
        QStringList keysToRemove;

        QByteArray data;
        QXmlStreamWriter writer(&data);

        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("envelope");
        writer.writeTextElement("messageType", MESSAGE_TYPE_STRINGS[type]);
        writer.writeTextElement("sender", sender);

        if (type == MessageType::StandardMessage){
            writer.writeStartElement("recipients");

            for (;it != rec.constEnd(); ++it){
                if (it.value() == currentURL){
                    writer.writeTextElement("recipient", it.key());
                    keysToRemove.append(it.key());
                }
            }

            writer.writeEndElement(); //recipients
        }

        writer.writeStartElement("message");

        writer.writeCDATA(msg);

        writer.writeEndElement(); //message

        writer.writeEndElement(); //envelope
        writer.writeEndDocument();

        //qDebug() << data;
        if (type == MessageType::StandardMessage){
            foreach(QString key, keysToRemove)
                rec.remove(key);
            keysToRemove.clear();
            sendHttp(data, currentURL, type);
        } else {
            foreach(QString address, recipients.values())
               sendHttp(data, address, type);
            return;
        }
        //qDebug() << currentURL;
    }
}

void MessageTransportService::sendHttp(const QByteArray msg, const QString targetAgent, MessageType type)
{
    QUrl url(type == MessageType::Notify ? targetAgent + "/AgentNotify" : targetAgent);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "envelope/xml");
    manager->post(request, msg);
    qDebug() << "POSLAL SOM REQUEST NA " << targetAgent;

}

void MessageTransportService::processXmlNotify(QByteArray data){

    // change it to announce only agents

    QHash<QString, AgentInfo> agents;
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
                        info.transportAddresses[node.toElement().text()].metric = node.nextSibling().toElement().text().toInt();
                }

            }else if(tagNam == VALID_UNTIL) {
                for(QHash<QString, TransportAddressProperties>::iterator it = info.transportAddresses.begin();
                    it != info.transportAddresses.end(); ++it){
                    it.value().validUntil = QTime::fromString(peData.text());
                }
            }

            pEntries = pEntries.nextSibling();
        }

        agents[info.desription.name] = info;
    }
}



void MessageTransportService::processHttpMessage(){

    // if notify, don't forward
    QByteArray data = m_request->readBody();
    qDebug() << data;
    QDomDocument doc;
    QStringList myAgents;
    QStringList forwardAgents;
    MessageType type;

    doc.setContent(data);


    QDomElement element = doc.documentElement();
    QDomNode node = element.namedItem("messageType");
    type =  (MessageType)MESSAGE_TYPE_STRINGS.indexOf(node.toElement().text());

    QByteArray msg;
    node = element.namedItem("message");
    msg = node.toElement().firstChild().toCDATASection().nodeValue().toUtf8();

    if (type == MessageType::Notify){
        processXmlNotify(msg);
        return;
    }


    node = element.namedItem("recipients");
    if (!node.isNull()){
        QDomNodeList nodeList = node.toElement().childNodes();
        for (int i = nodeList.length()-1; i >= 0; --i){
            if (platform->platformAgents.contains(nodeList.item(i).toElement().text())){
                myAgents << nodeList.item(i).toElement().text();
            } else {
                forwardAgents << nodeList.item(i).toElement().text();
            }
        }
    }

    qDebug() << "My agents: " << myAgents;
    qDebug() << "Remote agents: " << forwardAgents;

    QString sender;


    node = element.namedItem("sender");
    sender = node.toElement().text();


    QHash<QString, QString> recipients;
    foreach(QString agent, forwardAgents){
        QString url;
        int minMetric = 999;

        for(QHash<QString, TransportAddressProperties>::const_iterator it = platform->forwardedAgents[agent].transportAddresses.constBegin();
            it != platform->forwardedAgents[agent].transportAddresses.constEnd(); ++it){

            if (it.value().metric < minMetric){
                url = it.key();
                minMetric = it.value().metric;
            }
        }
        recipients[agent] = url;
        url = "";
        minMetric = 999;
    }
    writeHttpMessage(recipients, sender, msg, type);

    if (type == MessageType::StandardMessage)
        emit messageReady(myAgents, msg);

}

// ///////////////////////////////////////////////////////// PLATFORM /////////////////////////////////////////////////////////

Platform::Platform(QObject *parent)
    : QObject(parent)
    , mts(this)
    , ds(this)
{
    validationTimer = new QTimer(this);
    validationTimer->setInterval(5000);
    validationTimer->setSingleShot(false);

    localNetworkNotificationTimer = new QTimer(this);
    localNetworkNotificationTimer->setInterval(10000);
    localNetworkNotificationTimer->setSingleShot(false);

    forwardedAgentsNotificationTimer = new QTimer(this);
    forwardedAgentsNotificationTimer->setInterval(600000);
    localNetworkNotificationTimer->setSingleShot(false);


    //VALIDATIONTIME CONNECTION
    connect(validationTimer, SIGNAL(timeout()), this, SLOT(eraseInvalidTransportAddresses()));

    //MTS CONNECTIONS
    connect(&mts, SIGNAL(messageReady(QStringList,QByteArray)), this, SLOT(handleAgentMessage(QStringList,QByteArray)));

    ds.sendMulticastNotifyPacket();
}

void Platform::handleAgentMessage(QStringList recipients, QByteArray msg){
    //Give to Agent

    //qDebug() << recipients;
    //    qDebug() << msg;
}

void Platform::eraseInvalidTransportAddresses(){
    QHash<QString, AgentInfo>::iterator it = forwardedAgents.begin();
    while (it != forwardedAgents.end()){
        QHash<QString, TransportAddressProperties>::iterator it2 = it.value().transportAddresses.begin();
        while (it2 != it.value().transportAddresses.end()){
            if (it2.value().validUntil < QTime::currentTime()){
                it.value().transportAddresses.erase(it2);
            } else {
                ++it2;
            }
        }
        if (it.value().transportAddresses.empty()){
            forwardedAgents.erase(it);
        } else {
            ++it;
        }
    }
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
























