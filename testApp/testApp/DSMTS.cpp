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

    sendMulticastNotifyPacket();
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

bool DiscoveryService::parseNotifyPacket(const char *msg) // TODO CHANGE TO LIST
{
    AgentInfo ai;
    // qDebug(msg);
    QJsonDocument doc = QJsonDocument::fromJson(msg);
    QVariantMap message = doc.object().toVariantMap();
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        QVariantMap aiHash = it.value().toMap();
        ai.desription.name = aiHash[NAME].toString();
        ai.desription.services = aiHash[SERVICES].toStringList();
        ai.desription.flags = aiHash[FLAGS].toStringList();
        ai.address.url = aiHash[URL].toString();
        ai.address.properties.metric = aiHash[METRIC].toInt();
        ai.address.properties.timestamp = aiHash[METRIC].toTime();
        if (forwardedAgents.indexOf(ai) < 0)
            forwardedAgents.append(ai);
        else
            forwardedAgents[forwardedAgents.indexOf(ai)] = ai;
        ++it;
    }


    return true;
}

void DiscoveryService::sendMulticastNotifyPacket()  // TODO CHANGE TO LIST
{
    QVariantMap agentInfo;
    QVariantMap agent;

    //const namiesto name, services...


    for (int i = 0; i < platformAgents.length(); ++i){
        agent[NAME] = QVariant(platformAgents[i].desription.name);
        agent[SERVICES] = QVariant(platformAgents[i].desription.services);
        agent[FLAGS] = QVariant(platformAgents[i].desription.flags);
        agent[URL] = QVariant(platformAgents[i].address.url);
        agent[METRIC] = QVariant(platformAgents[i].address.properties.metric+1);
        agent[TIMESTAMP] = QVariant(platformAgents[i].address.properties.timestamp);
        agentInfo[platformAgents[i].desription.name] = agent;
    }

    for (int i = 0; i < forwardedAgents.length(); ++i){
        agent[NAME] = QVariant(forwardedAgents[i].desription.name);
        agent[SERVICES] = QVariant(forwardedAgents[i].desription.services);
        agent[FLAGS] = QVariant(forwardedAgents[i].desription.flags);
        agent[URL] = QVariant(forwardedAgents[i].address.url);
        agent[METRIC] = QVariant(forwardedAgents[i].address.properties.metric+1);
        agentInfo[forwardedAgents[i].desription.name] = agent;
    }

    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    //qDebug(doc.toJson());
    udpSocket->writeDatagram(doc.toJson(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);

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

    if (request.url().path().contains("/AgentNotify")){
        response.writeHead(Tufao::HttpResponseStatus::OK);
        response.headers().replace("Content-Type", "text/plain");
        response.end("AgentList Received");
        QMap<QString, AgentInfo>::const_iterator it;
        agents = processHttpNotify(request.readBody());
        qDebug() << "DOSTAL SOM REQUEST";
        for (it = agents.constBegin(); it != agents.constEnd(); ++it){
            qDebug() << it.value().desription.name;
            qDebug() << it.value().desription.flags;
            qDebug() << it.value().desription.services;
            qDebug() << it.value().address.url;
            qDebug() << it.value().address.properties.metric;
        }


        emit httpNotifyReceived(agents);
    } else {

        //TODO PROCESS MSG
        response.writeHead(Tufao::HttpResponseStatus::NO_RESPONSE);
        response.headers().replace("Content-Type", "text/plain");
        response.end(":(");
    }


}

void MessageTransportService::writeHttpNotify(const QList<AgentInfo> agentsToBeNotified,
                                              const QMap<QString, QString> recipients, const QString sender){
    QMap<QString, QString> rec = recipients;
    QString currentURL = "";
    QMap<QString, QString>::const_iterator it;

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


            writer.writeTextElement(URL, info.address.url);
            writer.writeTextElement(METRIC, QString::number(++info.address.properties.metric));

            writer.writeEndElement(); //agent

        }
        writer.writeEndElement(); //agents
        writer.writeEndElement(); //envelope
        writer.writeEndDocument();
        foreach(QString key, keysToRemove)
            rec.remove(key);
        keysToRemove.clear();

        //qDebug() << data;
        sendHttp(data, currentURL, true); //TODO MESSAGE


    }


}

void MessageTransportService::writeHttpMessage(const QMap<QString, QString> recipients, const QString sender, QByteArray msg)
{
    QMap<QString, QString> rec = recipients;
    QString currentURL = "";
    QMap<QString, QString>::const_iterator it = rec.constBegin();



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

        //qDebug() << data;

        foreach(QString key, keysToRemove)
            rec.remove(key);
        keysToRemove.clear();
        sendHttp(data, currentURL, false); //TODO MESSAGE
        //qDebug() << currentURL;
    }
}

void MessageTransportService::sendHttp(const QByteArray msg, const QString targetAgent, bool Notify)
{
    QUrl url(Notify ? targetAgent + "/AgentNotify" : targetAgent);
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
            }else if(tagNam == URL) {
                info.address.url = peData.text();
            }else if(tagNam == METRIC) {
                info.address.properties.metric = peData.text().toInt();
            }

            pEntries = pEntries.nextSibling();
        }

        agents[info.desription.name] = info;
    }
    return agents;

}

// ///////////////////////////////////////////////////////// PLATFORM /////////////////////////////////////////////////////////

Platform::Platform(QObject *parent) : QObject(parent)
{
    qDebug() << "Platforma";
    //connect(&MTS, SIGNAL(httpNotifyReceived(QByteArray)), this, SLOT(forwardHttpNotifyToDs(QMap<QString, AgentInfo>)));
}

void Platform::forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents)
{
    //todo
    return;
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
























