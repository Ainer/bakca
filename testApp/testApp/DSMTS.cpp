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

    SendMulticastNotifyPacket();
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
    AgentInfo ai;
    // qDebug(msg);
    QJsonDocument doc = QJsonDocument::fromJson(msg);
    QVariantMap message = doc.object().toVariantMap();
    QVariantMap::const_iterator it = message.constBegin();
    while(it != message.constEnd()){
        QVariantMap aiHash = it.value().toMap();
        ai.desription.name = aiHash["name"].toString();
        ai.desription.services = aiHash["services"].toStringList();
        ai.desription.flags = aiHash["flags"].toStringList();
        ai.address.url = aiHash["url"].toString();
        ai.address.metric = aiHash["metric"].toInt();
        int tmpindex = Agents.indexOf(ai);
        if (tmpindex > -1){
            Agents[tmpindex] = ai;
            tmpindex = -1;
        } else {
            Agents.push_back(ai);
            LocalAgents.push_back(Agents.length()-1);
        }

        ++it;
    }


    return true;
}

void DiscoveryService::SendMulticastNotifyPacket()
{
    QVariantMap agentInfo;
    QVariantMap agent;


    for (int i = 0; i < PlatformAgents.length(); ++i){
        agent["name"] = QVariant(Agents[PlatformAgents[i]].desription.name);
        agent["services"] = QVariant(Agents[PlatformAgents[i]].desription.services);
        agent["flags"] = QVariant(Agents[PlatformAgents[i]].desription.flags);
        agent["url"] = QVariant(Agents[PlatformAgents[i]].address.url);
        agent["metric"] = QVariant(Agents[PlatformAgents[i]].address.metric+1);
        agentInfo[Agents[PlatformAgents[i]].desription.name] = agent;
    }

    for (int i = 0; i < RemoteAgents.length(); ++i){
        agent["name"] = QVariant(Agents[RemoteAgents[i]].desription.name);
        agent["services"] = QVariant(Agents[RemoteAgents[i]].desription.services);
        agent["flags"] = QVariant(Agents[RemoteAgents[i]].desription.flags);
        agent["url"] = QVariant(Agents[RemoteAgents[i]].address.url);
        agent["metric"] = QVariant(Agents[RemoteAgents[i]].address.metric+1);
        agentInfo[Agents[RemoteAgents[i]].desription.name] = agent;
    }

    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(agentInfo));
    //qDebug(doc.toJson());
    udpSocket->writeDatagram(doc.toJson(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);

}


QMap<QString, QString> DiscoveryService::getUrlFromName(QStringList names){
    QMap<QString, QString> output;
    foreach(QString name, names){
        foreach(AgentInfo ai, Agents){
            if (ai.desription.name == name)
                output[name] = ai.address.url;
        }
    }
    return output;
}

QString DiscoveryService::getUrlFromName(QString name){
    foreach(AgentInfo ai, Agents){
        if (ai.desription.name == name)
            return ai.address.url;
    }
    return "";
}

// ///////////////////////////////////////////////////////// MESSAGE TRANSPORT SERVICE /////////////////////////////////////////////////////////

MessageTransportService::MessageTransportService(QObject *parent) : QObject(parent)
{

    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     this, &MessageTransportService::handleRequest);

    //tcpClient.connectToHost(My_ADDRESS, 1024);


    server.listen(QHostAddress::Any, 8080);
}

bool MessageTransportService::handleRequest(Tufao::HttpServerRequest &request,
                                            Tufao::HttpServerResponse &response){

    if (request.url().path().contains("/AgentNotify")){
        response.writeHead(Tufao::HttpResponseStatus::OK);
        response.headers().replace("Content-Type", "text/plain");
        response.end("AgentList Received");
        QMap<QString, AgentInfo>::const_iterator it;
        QMap<QString, AgentInfo> agents = processHttpNotify(request.readBody());
        for (it = agents.constBegin(); it != agents.constEnd(); ++it){
            qDebug() << it.value().desription.name;
            qDebug() << it.value().desription.flags;
            qDebug() << it.value().desription.services;
            qDebug() << it.value().address.url;
            qDebug() << it.value().address.metric;
        }
        //qDebug() << request.readBody();
    } else {
        response.writeHead(Tufao::HttpResponseStatus::NO_RESPONSE);
        response.headers().replace("Content-Type", "text/plain");
        response.end(":(");
    }
    emit httpNotifyReceived(request.readBody());

    return true;

}

void MessageTransportService::sendMessage(const QMap<QString, QString> recipients, const QByteArray msg, const QString sender){

    /*
    QMap<QString, QString> rec = recipients;
    QString currentURL = "";
    QMap<QString, QString>::const_iterator it;
    /*
    <message>
    <content-type>message/xml</content-type>
    <data>&lt;?xml version="1.0".... </data>

    while(!rec.empty()){
        it = rec.constBegin();
        currentURL = it.value();


        QDomDocument doc;
        QDomProcessingInstruction instr = doc.createProcessingInstruction(
                            "xml", "version='1.0' encoding='UTF-8'");
        doc.appendChild(instr);
        QDomElement element = doc.createElement("envelope");
        doc.appendChild(element);

        QDomElement messageElement = doc.createElement("message");
//vyriesit XML


        QDomElement newElement;



        addElement(doc, element, "sender", sender);
        newElement = doc.createElement("recipients");
        element.appendChild(newElement);

        for (;it != rec.end(); ++it){
            if (it.value() == currentURL){
                addElement(doc, newElement, "recipient", it.key());
                rec.remove(it.key());
                if (rec.empty())
                    break;
                it = rec.begin();
            }
        }
        qDebug(doc.toByteArray());

    }
    */

}

void MessageTransportService::writeHttpNotifyMessage(const QList<AgentInfo> agentsToBeNotified, const QMap<QString, QString> recipients, const QString sender)
{
    QMap<QString, QString> rec = recipients;
    QString currentURL = "";
    QMap<QString, QString>::const_iterator it = rec.constBegin();

    QByteArray data;
    QXmlStreamWriter writer(&data);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("envelope");
    writer.writeTextElement("sender", sender);
    writer.writeStartElement("recipients");
    for (;it != rec.constEnd(); ++it){
        writer.writeTextElement("recipient", it.key());
    }
    writer.writeEndElement(); //recipients

    writer.writeStartElement("agents");
    foreach(AgentInfo info, agentsToBeNotified)
    {
        writer.writeStartElement("agent");

        writer.writeTextElement("name", info.desription.name);


        writer.writeStartElement("flags");
        foreach (QString flag, info.desription.flags)
        {
            writer.writeTextElement("flag", flag);
        }
        writer.writeEndElement();


        writer.writeStartElement("services");
        foreach (QString service, info.desription.services)
        {
            writer.writeTextElement("service", service);
        }
        writer.writeEndElement();


        writer.writeTextElement("url", info.address.url);
        writer.writeTextElement("metric", QString::number(++info.address.metric));

        writer.writeEndElement();

    }

    writer.writeEndElement();
    writer.writeEndDocument();

    while(!rec.empty()){
        it = rec.constBegin();
        currentURL = it.value();
        QStringList keysToRemove;
        for (;it != rec.constEnd(); ++it){
            if (it.value() == currentURL)
                keysToRemove.append(it.key());
        }
        foreach(QString key, keysToRemove)
            rec.remove(key);
        keysToRemove.clear();
        sendHttpNotify(data, currentURL);
        //qDebug() << currentURL;
    }
    //qDebug() << data;
}

void MessageTransportService::sendHttpNotify(const QByteArray msg, const QString targetAgent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url(targetAgent + "/AgentNotify");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "envelope/xml");

    reply = manager->post(request, msg);

    connect(reply, SIGNAL(readyRead()), this, SLOT(httpNotifyReadyRead()));
}

QMap<QString, AgentInfo> MessageTransportService::processHttpNotify(QByteArray data){
    QMap<QString, AgentInfo> agents;
    QDomDocument doc;
    !doc.setContent(data);
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

            if(tagNam == "name") {
                info.desription.name = peData.text();
            }else if(tagNam == "services") {
                for(int i = 0; i < peData.childNodes().count(); ++i){
                    info.desription.services << peData.childNodes().at(i).toElement().text();
                }
            }else if(tagNam == "flags") {
                for(int i = 0; i < peData.childNodes().count(); ++i){
                    info.desription.flags << peData.childNodes().at(i).toElement().text();
                }
            }else if(tagNam == "url") {
                info.address.url = peData.text();
            }else if(tagNam == "metric") {
                info.address.metric = peData.text().toInt();
            }

            pEntries = pEntries.nextSibling();
        }

        agents[info.desription.name] = info;
    }
    return agents;

}

void MessageTransportService::httpNotifyReadyRead(){

    QByteArray data;

    if (reply->url().path().contains("/AgentNotify")){



        //qDebug() << doc.toByteArray();
    }else {
        data = reply->readAll();
    }
}

void MessageTransportService::httpMessageReadyRead(){

}

// ///////////////////////////////////////////////////////// PLATFORM /////////////////////////////////////////////////////////

Platform::Platform(QObject *parent) : QObject(parent)
{
    connect(&MTS, SIGNAL(httpNotifyReceived(QByteArray)), this, SLOT(forwardHttpNotifyToDs(QByteArray)));
}

void Platform::forwardHttpNotifyToDs(QMap<QString, AgentInfo> agents)
{
    qDebug() << agents;

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
























