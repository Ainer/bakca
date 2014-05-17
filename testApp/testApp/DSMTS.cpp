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

int number = 0;




//TODO
//HELLO/BYE message on startup and finish and handling +handling
//loading/saving persistent GW list +
//Beautify
//
//
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
    writeStatusMessage("Hello");
    //sendMulticastNotifyPacket();
}
DiscoveryService::~DiscoveryService(){
    writeStatusMessage("Bye");
}

bool DiscoveryService::saveGWtoFile(){
    QFile saveFile(QStringLiteral("GWAgents.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonDocument saveDoc(QJsonArray::fromStringList(platform->gatewayAgents));
    qDebug() << saveDoc.toJson();
    saveFile.write(saveDoc.toJson());
    return true;
}

bool DiscoveryService::loadGWfromFile(){
    QFile loadFile(QStringLiteral("GWAgents.json"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    for (int i = 0; i < loadDoc.array().count(); ++i){
        if (!platform->gatewayAgents.contains(loadDoc.array()[i].toString()))
            platform->gatewayAgents.append(loadDoc.array()[i].toString());
    }

    qDebug() << platform->gatewayAgents;
    return true;
}

void DiscoveryService::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        handleDatagram(datagram);
    }
}

inline QJsonObject fromProperties(TransportAddressProperties props, bool isPlatformAgent){
    QJsonObject object;
    QJsonValue value;
    value = props.metric;
    object.insert(METRIC, value);
    if (isPlatformAgent)
        value = QTime::currentTime().addSecs(30).toString();
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
    return props;
}

void DiscoveryService::handleDatagram(QByteArray data){
    QJsonDocument doc = QJsonDocument::fromBinaryData(data);
    QVariantMap message = doc.object().toVariantMap();
    if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Notify])
        parseNotifyPacket(message);
    else if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Hello])
        return;
    else if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Bye])
        return;
    return;
}

bool DiscoveryService::parseNotifyPacket(QVariantMap msg) // TODO CHANGE TO LIST
{
    AgentInfo ai;
    // qDebug(msg);

    QVariantMap message = msg;
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        if (it.key() == "gwAgents"){
            foreach(QVariant agent, it.value().toList()){
                if (!platform->gatewayAgents.contains(agent.toString())){
                    platform->gatewayAgents.append(agent.toString());
                }
            }
            ++it;
            continue;
        } else if (it.key() == "type"){
            ++it;
            continue;
        } else {
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
    }
    return true;
}

void DiscoveryService::sendMulticastNotifyPacket()
{
    if (platform->platformAgents.empty() && platform->forwardedAgents.empty() && platform->gatewayAgents.empty())
        return;

    QVariantMap agentInfo;
    QVariantMap agent;

    QHash<QString, AgentInfo>::const_iterator it;

    for (it = platform->platformAgents.constBegin(); it != platform->platformAgents.constEnd(); ++it){
        QVariantMap addresses;
        QHash<QString, TransportAddressProperties>::const_iterator it2 = it.value().transportAddresses.constBegin();
        while (it2 != it.value().transportAddresses.constEnd()){
            addresses[it2.key()] = fromProperties(it2.value(), true).toVariantMap();
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

    if (platform->gateway){

        for (it = platform->forwardedAgents.constBegin(); it != platform->forwardedAgents.constEnd(); ++it){
            QVariantMap addresses;
            QHash<QString, TransportAddressProperties>::const_iterator it2 = it.value().transportAddresses.constBegin();
            while (it2 != it.value().transportAddresses.constEnd()){
                if (it2.value().sourceDs != this)
                    addresses[MY_ADDRESS + "/routedAgents"] = fromProperties(it2.value(), false).toVariantMap();
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

        QVariantList gwAgents;
        foreach (QString agent, platform->gatewayAgents)
            gwAgents.append(QVariant(agent));
        if (!gwAgents.empty())
            agentInfo["gwAgents"] = gwAgents;
        agentInfo["type"] = "notify";
    }

    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    //qDebug() << doc.toJson();
    qDebug() << "================================" << "Local" << number << "================================";
    number++;

    udpSocket->writeDatagram(doc.toBinaryData(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);
}

void DiscoveryService::writeStatusMessage(QString type){
    QVariantMap msg;
    msg["type"] = QVariant(type);
    msg["address"] = QVariant(MY_ADDRESS);
    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(msg));
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
    response.writeHead(Tufao::HttpResponseStatus::OK);
    response.headers().replace("Content-Type", "text/plain");
    response.end(":)");
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

    if (platform->platformAgents.empty() && platform->forwardedAgents.empty() && platform->gatewayAgents.empty())
        //if all agents empty, no need to notify Notify when only gw not empty?
        return;

    writer->writeStartDocument();
    writer->writeStartElement("notifyInfo");
    if (!platform->platformAgents.empty() || !platform->forwardedAgents.empty()){
        writer->writeStartElement("agents");

        foreach(AgentInfo info, platform->platformAgents)
            inserAgents(writer, info, false);

        foreach(AgentInfo info, platform->forwardedAgents)
            inserAgents(writer, info, true);

        writer->writeEndElement(); //agents
    }


    if (!platform->gatewayAgents.empty()){
        writer->writeStartElement("GWInfo");

        foreach(QString agent, platform->gatewayAgents)
            writer->writeTextElement("GWAgent", agent);

        writer->writeEndElement(); // GWInfo
    }
    writer->writeEndElement(); //notifyInfo
    writer->writeEndDocument();

    //qDebug() << data;

    QHash<QString, QString> recipients;
    foreach(QString address, platform->gatewayAgents)
        recipients[address] = address;
    //qDebug() << recipients.keys();

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
            }else if(tagNam == "routes") {
                QDomNodeList taNodeList = peData.toElement().childNodes();
                for (int i = 0; i < taNodeList.length(); ++i){
                    QDomNode metricNode = taNodeList.item(i).toElement().firstChild();
                    QDomNode validUntilNode = metricNode.nextSibling();
                    QDomNode addressNode = validUntilNode.nextSibling();
                    if (metricNode.toElement().tagName() == METRIC && validUntilNode.toElement().tagName() == VALID_UNTIL
                            && addressNode.toElement().tagName() == "transportAddress")
                        info.transportAddresses[addressNode.toElement().text()].metric = metricNode.toElement().text().toInt();
                    info.transportAddresses[addressNode.toElement().text()].validUntil = QTime::fromString(validUntilNode.toElement().text());
                }
            }

            pEntries = pEntries.nextSibling();
        }

        agents[info.desription.name] = info;

        nodeList = element.elementsByTagName("GWAgent");
        for (int i = 0; i < nodeList.count(); ++i){
            if (!platform->gatewayAgents.contains(nodeList.at(i).toElement().text()))
                platform->gatewayAgents.append(nodeList.at(i).toElement().text());
        }
    }
}



void MessageTransportService::processHttpMessage(){


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

    QString sender;

    node = element.namedItem("sender");
    sender = node.toElement().text();

    // if notify, don't forward
    if (type == MessageType::Notify){
        if (!platform->gatewayAgents.contains(sender))
            platform->gatewayAgents.append(sender);
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
    ds.loadGWfromFile();
    validationTimer = new QTimer(this);
    validationTimer->setInterval(5000);
    validationTimer->setSingleShot(false);
    validationTimer->start();

    localNetworkNotificationTimer = new QTimer(this);
    localNetworkNotificationTimer->setInterval(10000);
    localNetworkNotificationTimer->setSingleShot(false);
    localNetworkNotificationTimer->start();

    forwardedAgentsNotificationTimer = new QTimer(this);
    forwardedAgentsNotificationTimer->setInterval(60000);
    forwardedAgentsNotificationTimer->setSingleShot(false);
    forwardedAgentsNotificationTimer->start();


    //VALIDATIONTIME CONNECTION
    connect(validationTimer, SIGNAL(timeout()), this, SLOT(eraseInvalidTransportAddresses()));
    connect(localNetworkNotificationTimer, SIGNAL(timeout()), &ds, SLOT(sendMulticastNotifyPacket()));
    connect(forwardedAgentsNotificationTimer, SIGNAL(timeout()), &mts, SLOT(writeHttpNotify()));

    //MTS CONNECTIONS
    connect(&mts, SIGNAL(messageReady(QStringList,QByteArray)), this, SLOT(handleAgentMessage(QStringList,QByteArray)));
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
                it.value().transportAddresses.remove(it2.key());
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
























