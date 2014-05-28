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


//INLINE METHODS

inline QJsonObject fromProperties(TransportAddressProperties props, bool isPlatformAgent){
    QJsonObject object;
    QJsonValue value;
    value = isPlatformAgent ? 0 : props.metric;
    object.insert(METRIC, value);
    value = isPlatformAgent ? QTime::currentTime().addSecs(30).toString() : props.validUntil.toString();
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

inline void inserAgents(QXmlStreamWriter *writer, AgentInfo info, QString recipient){
    writer->writeStartElement("agent");

    writer->writeTextElement(NAME, info.desription.name);


    writer->writeStartElement(FLAGS);
    foreach (QString flag, info.desription.flags)
    {
        writer->writeTextElement("flag", flag);
    }
    writer->writeEndElement(); //flags


    writer->writeStartElement(SERVICES);
    foreach (QString service, info.desription.services)
    {
        writer->writeTextElement("service", service);
    }
    writer->writeEndElement(); //services

    writer->writeStartElement(TRANSPORT_ADDRESSES);
    for (auto it2 = info.transportAddresses.constBegin();
         it2 != info.transportAddresses.constEnd(); ++it2){
        if (it2.value().origins.contains(recipient))
            continue;
        writer->writeStartElement("route");
        writer->writeTextElement(METRIC, QString::number(it2.value().metric));
        writer->writeTextElement(VALID_UNTIL, it2.value().validUntil.toString());
        writer->writeTextElement("transportAddress", MY_ADDRESS + "/forwardedAgents");
        writer->writeTextElement("origins", it2.value().origins.join(" "));
        writer->writeEndElement(); // route
    }
    writer->writeEndElement(); // transportAddresses
    writer->writeEndElement(); //agent
}


// ///////////////////////////////////////////////////////// DISCOVERY SERVICE /////////////////////////////////////////////////////////


//DISCOVERY SERVICE PRIVATE
void DiscoveryService::handleDatagram(QByteArray data){
    QJsonDocument doc = QJsonDocument::fromBinaryData(data);
    QVariantMap message = doc.object().toVariantMap();
    if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Notify])
        parseNotifyPacket(message);
    else if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Hello])
		m_platform->handleStatusMessage(MessageType::Hello, message["address"].toString(), true);
    else if (message["type"].toString() == MESSAGE_TYPE_STRINGS[MessageType::Bye])
		m_platform->handleStatusMessage(MessageType::Bye, message["address"].toString(), true);
    return;
}

void DiscoveryService::writeStatusMessage(QString type){
    QVariantMap msg;
    msg["type"] = QVariant(type);
    msg["address"] = QVariant(MY_ADDRESS);
    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(msg));
    m_udpSocket->writeDatagram(doc.toBinaryData(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);
}

//DISCOVERY SERVICE PUBLIC

DiscoveryService::DiscoveryService(Platform *platform)
    : QObject(platform)
    , m_platform(platform)
{
    m_groupAddress = QHostAddress(MULTICAST_ADDRESS);

    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(QHostAddress::AnyIPv4, MULTICAST_PORT, QUdpSocket::ShareAddress);
    m_udpSocket->joinMulticastGroup(m_groupAddress);
    //udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(0));

    connect(m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));
    writeStatusMessage("hello");
    //sendMulticastNotifyPacket();
}
DiscoveryService::~DiscoveryService(){
    writeStatusMessage("bye");
}

bool DiscoveryService::saveGWtoFile(){
    QFile saveFile(QStringLiteral("GWAgents.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonDocument saveDoc(QJsonArray::fromStringList(m_platform->m_gatewayAgents));
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
        if (!m_platform->m_gatewayAgents.contains(loadDoc.array()[i].toString()))
            m_platform->m_gatewayAgents.append(loadDoc.array()[i].toString());
    }

    qDebug() << m_platform->m_gatewayAgents;
    return true;
}

bool DiscoveryService::parseNotifyPacket(QVariantMap msg) // TODO CHANGE TO LIST
{
    AgentInfo ai;
    // qDebug(msg);

    QVariantMap message = msg;
    auto it = message.constBegin();
    while(it != message.constEnd()){
        if (it.key() == "gwAgents"){
            foreach(QVariant agent, it.value().toList()){
                if (!m_platform->m_gatewayAgents.contains(agent.toString())){
                    m_platform->m_gatewayAgents.append(agent.toString());
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
            if (m_platform->m_platformAgents.contains(ai.desription.name)){
                ++it;
                continue;
            }
            QVariantMap tmpMap = aiMap[TRANSPORT_ADDRESSES].toMap();
            for (auto it2 = tmpMap.constBegin(); it2 != tmpMap.constEnd(); ++it2){
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



//DISCOVERY SERVICE PRIVATE SLOTS
void DiscoveryService::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        handleDatagram(datagram);
    }
}

//DISCOVERY SERVICE PUBLIC SLOTS

void DiscoveryService::sendMulticastNotifyPacket()
{
    if (m_platform->m_platformAgents.empty() && m_platform->m_forwardedAgents.empty() && m_platform->m_gatewayAgents.empty())
        return;

    QVariantMap agentInfo;
    QVariantMap agent;

    for (auto it = m_platform->m_platformAgents.constBegin(); it != m_platform->m_platformAgents.constEnd(); ++it){
        QVariantMap addresses;
        auto it2 = it.value().transportAddresses.constBegin();
        while (it2 != it.value().transportAddresses.constEnd()){
            addresses[it2.key()] = fromProperties(it2.value(), true).toVariantMap();
            ++it2;
        }
        if (addresses.empty()) //do not announce agents with no new addresses
            continue;
        agent[NAME] = QVariant(it.value().desription.name);
        agent[SERVICES] = QVariant(it.value().desription.services);
        agent[FLAGS] = QVariant(it.value().desription.flags);
        agent[TRANSPORT_ADDRESSES] =  addresses;
        agentInfo[it.value().desription.name] = agent;
    }

    if (m_platform->m_gateway){

        for (auto it = m_platform->m_forwardedAgents.constBegin(); it != m_platform->m_forwardedAgents.constEnd(); ++it){
            QVariantMap addresses;
            auto it2 = it.value().transportAddresses.constBegin();
            while (it2 != it.value().transportAddresses.constEnd()){
                if (it2.value().sourceDs != this)
                    addresses[MY_ADDRESS] = fromProperties(it2.value(), false).toVariantMap();
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
        foreach (QString agent, m_platform->m_gatewayAgents)
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

    m_udpSocket->writeDatagram(doc.toBinaryData(), QHostAddress(MULTICAST_ADDRESS), MULTICAST_PORT);
}



// ///////////////////////////////////////////////////////// MESSAGE TRANSPORT SERVICE /////////////////////////////////////////////////////////

//MESSAGE TRANSPORT SERVICE PRIVATE

void MessageTransportService::sendHttp(const QByteArray msg, const QString targetAgent)
{
    QUrl url(targetAgent);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "envelope/xml");
    manager->post(request, msg);
    qDebug() << "POSLAL SOM REQUEST NA " << targetAgent;

}

void MessageTransportService::processXmlNotify(QByteArray data, QString sender){

    // change it to announce only agents

    QHash<QString, AgentInfo> agents;
    QDomDocument doc;
    doc.setContent(data);
    QDomElement element = doc.documentElement();
    QDomNodeList nodeList = element.elementsByTagName("agent");

    for(int ii = 0;ii < nodeList.count(); ii++)
    {
        AgentInfo info;
        bool isLocal = false;

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
                QDomNodeList taNodeList = peData.toElement().childNodes();
                for (int i = 0; i < taNodeList.length(); ++i){
                    qDebug() << "route" << i;
                    QDomNode metricNode = taNodeList.item(i).toElement().firstChild();
                    QDomNode validUntilNode = metricNode.nextSibling();
                    QDomNode addressNode = validUntilNode.nextSibling();
                    if (metricNode.toElement().tagName() == METRIC && validUntilNode.toElement().tagName() == VALID_UNTIL
                            && addressNode.toElement().tagName() == "transportAddress"){
                        info.transportAddresses[addressNode.toElement().text()].metric = metricNode.toElement().text().toInt();
                        info.transportAddresses[addressNode.toElement().text()].validUntil = QTime::fromString(validUntilNode.toElement().text());
                    }
                    QString text = taNodeList.item(i).toElement().namedItem("origins").toElement().text();
                    info.transportAddresses[addressNode.toElement().text()].origins = text.split(' ', QString::SplitBehavior::SkipEmptyParts);
                    info.transportAddresses[addressNode.toElement().text()].origins.append(sender);
                    //qDebug() << info.transportAddresses[addressNode.toElement().text()].route;
                }
            }

            pEntries = pEntries.nextSibling();
        }
        //find existence

		//skontroluj, ci neexistuje s metrikou 0, v takom pripade neukladaj
        foreach(TransportAddressProperties props, agents[info.desription.name].transportAddresses.values())
            if (props.metric == 0)
                isLocal = true;
        if (isLocal)
            continue;
        agents[info.desription.name].desription = info.desription;
        auto it = info.transportAddresses.begin();
        bool metricExists = false;
        while (it != info.transportAddresses.end()){
            if (agents[info.desription.name].transportAddresses[it.key()].origins == it.value().origins){
                agents[info.desription.name].transportAddresses[it.key()] = it.value();
                metricExists = true;
            }
            if (it == info.transportAddresses.constEnd() && !metricExists){
                agents[info.desription.name].transportAddresses.insertMulti(it.key(), it.value());
            }
            ++it;
        }

        nodeList = element.elementsByTagName("GWAgent");
        for (int i = 0; i < nodeList.count(); ++i){
            if (!m_platform->m_gatewayAgents.contains(nodeList.at(i).toElement().text()))
                m_platform->m_gatewayAgents.append(nodeList.at(i).toElement().text());
        }
    }
}

//MESSAGE TRANSPORT SEVICE PUBLIC
MessageTransportService::MessageTransportService(Platform *platform)
    : QObject(platform)
    , m_platform(platform)
{

    connect(&m_server, SIGNAL(requestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)),
            this,
            SLOT(handleRequest(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)));

    m_server.listen(QHostAddress::Any, 22222);
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
            sendHttp(data, currentURL);
        } else {
            foreach(QString address, recipients.values())
                sendHttp(data, address);
            return;
        }
        //qDebug() << currentURL;
    }
}

void MessageTransportService::writeHttpStatusMessage(QString type){
    if (!m_platform->m_gateway)
        return;
    QByteArray data;
    QXmlStreamWriter *writer = new QXmlStreamWriter(&data);
    writer->setAutoFormatting(true);

    writer->writeStartDocument();
    writer->writeTextElement("type", type);
    writer->writeTextElement("address", MY_ADDRESS);
    writer->writeEndDocument();

    QHash<QString, QString> recipients;
    foreach(QString gw, m_platform->m_gatewayAgents){
        recipients.insert(gw, gw);
    }
    qDebug() << data;

    writeHttpMessage(recipients, MY_ADDRESS, data, MessageType::Hello);
}

MessageTransportService::~MessageTransportService(){
    writeHttpStatusMessage("bye");
}

//MESSAGE TRANSPORT SERVICE PRIVATE SLOTS

void MessageTransportService::handleRequest(Tufao::HttpServerRequest &request, Tufao::HttpServerResponse &response){
    //TODO PROCESS MSG

    qDebug() << "my address: " << request.socket().localAddress().toString();
    qDebug() << "peer address: " << request.socket().peerAddress().toString();

    connect(&request, &Tufao::HttpServerRequest::end,
    this, [this,&request, &response]() {
    processHttpMessage(request, response);
    });
}

void MessageTransportService::processHttpMessage(Tufao::HttpServerRequest &request, Tufao::HttpServerResponse &response){
    QByteArray data = request.readBody();
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
        if (!m_platform->m_gatewayAgents.contains(sender))
            m_platform->m_gatewayAgents.append(sender);
        processXmlNotify(msg, sender);
        response.writeHead(Tufao::HttpResponseStatus::OK);//presun na spravne miesto po spracovani message
        response.headers().replace("Content-Type", "text/plain");
        response.end(":)");
        return;
	}
	else if (type == MessageType::Hello || type == MessageType::Bye){
		m_platform->handleStatusMessage(type, sender, false);
        response.writeHead(Tufao::HttpResponseStatus::OK);//presun na spravne miesto po spracovani message
        response.headers().replace("Content-Type", "text/plain");
        response.end(":)");
		return;
	}


    node = element.namedItem("recipients");
    if (!node.isNull()){
        QDomNodeList nodeList = node.toElement().childNodes();
        for (int i = nodeList.length()-1; i >= 0; --i){
            if (m_platform->m_platformAgents.contains(nodeList.item(i).toElement().text())){
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

        for(auto it = m_platform->m_forwardedAgents[agent].transportAddresses.constBegin();
            it != m_platform->m_forwardedAgents[agent].transportAddresses.constEnd(); ++it){

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
    response.writeHead(Tufao::HttpResponseStatus::OK);//presun na spravne miesto po spracovani message
    response.headers().replace("Content-Type", "text/plain");
    response.end(":)");

    if (type == MessageType::StandardMessage)
        emit messageReady(myAgents, msg);

}

//MESSAGE TRANSPORT SERVICE PUBLIC SLOTS


void MessageTransportService::writeHttpNotify(){

    QByteArray data;
    QXmlStreamWriter *writer = new QXmlStreamWriter(&data);
    writer->setAutoFormatting(true);

    QHash<QString, QString> recipients;
    foreach(QString address, m_platform->m_gatewayAgents)
        recipients[address] = address;

    qDebug() << "PISEM SPRAVU";

    if (m_platform->m_platformAgents.empty() && m_platform->m_forwardedAgents.empty() && m_platform->m_gatewayAgents.empty())
        //if all agents empty, no need to notify Notify when only gw not empty?
        return;

    foreach(QString gw, recipients.keys()){

        writer->writeStartDocument();
        writer->writeStartElement("notifyInfo");
        if (!m_platform->m_platformAgents.empty() || !m_platform->m_forwardedAgents.empty()){
            writer->writeStartElement("agents");

            foreach(AgentInfo info, m_platform->m_platformAgents){
                inserAgents(writer, info, gw);
            }

            foreach(AgentInfo info, m_platform->m_forwardedAgents)
                inserAgents(writer, info, gw);

            writer->writeEndElement(); //agents
        }


        if (!m_platform->m_gatewayAgents.empty()){
            writer->writeStartElement("GWInfo");

            foreach(QString agent, m_platform->m_gatewayAgents)
                writer->writeTextElement("GWAgent", agent);

            writer->writeEndElement(); // GWInfo
        }
        writer->writeEndElement(); //notifyInfo
        writer->writeEndDocument();
        qDebug() << data;
        QHash<QString, QString> recipient;
        recipient.insert(gw, gw);
        writeHttpMessage(recipient  //GW agents
                         ,MY_ADDRESS, data, MessageType::Notify);//prehod na spravne miesto
    }
}
// ///////////////////////////////////////////////////////// Platform /////////////////////////////////////////////////////////
//Platform PUBLIC
Platform::Platform(QObject *parent)
    : QObject(parent)
    , m_mts(this)
    , m_ds(this)
{
    m_ds.loadGWfromFile();
    m_validationTimer = new QTimer(this);
    m_validationTimer->setInterval(5000);
    m_validationTimer->setSingleShot(false);
    m_validationTimer->start();

    m_localNetworkNotificationTimer = new QTimer(this);
    m_localNetworkNotificationTimer->setInterval(10000);
    m_localNetworkNotificationTimer->setSingleShot(false);
    m_localNetworkNotificationTimer->start();

    m_forwardedAgentsNotificationTimer = new QTimer(this);
    m_forwardedAgentsNotificationTimer->setInterval(60000);
    m_forwardedAgentsNotificationTimer->setSingleShot(false);
    m_forwardedAgentsNotificationTimer->start();


    //VALIDATIONTIME CONNECTION
    connect(m_validationTimer, SIGNAL(timeout()), this, SLOT(eraseInvalidTransportAddresses()));
    connect(m_localNetworkNotificationTimer, SIGNAL(timeout()), &m_ds, SLOT(sendMulticastNotifyPacket()));
    connect(m_forwardedAgentsNotificationTimer, SIGNAL(timeout()), &m_mts, SLOT(writeHttpNotify()));

    //MTS CONNECTIONS
    connect(&m_mts, SIGNAL(messageReady(QStringList,QByteArray)), this, SLOT(handleAgentMessage(QStringList,QByteArray)));
    m_mts.writeHttpStatusMessage("hello");
}

void Platform::handleStatusMessage(MessageType type, QString address, bool ds){
    if (type == MessageType::Hello){
        if (!m_gatewayAgents.contains(address) && !ds)
            m_gatewayAgents << address;
		if (ds)
            m_ds.sendMulticastNotifyPacket();
		else
            m_mts.writeHttpNotify();
    } else if (type == MessageType::Bye && !ds){
        if (m_gatewayAgents.contains(address))
            m_gatewayAgents.removeAll(address);
        auto it = m_forwardedAgents.begin();
        while (it != m_forwardedAgents.end()){
            auto it2 = it.value().transportAddresses.begin();
            while (it2 != it.value().transportAddresses.end()){
                if (it2.key().contains(address)){
                    it.value().transportAddresses.remove(it2.key());
                } else {
                    ++it2;
                }
            }
            if (it.value().transportAddresses.empty()){
                m_forwardedAgents.erase(it);
            } else {
                ++it;
            }
        }
    }
}

//Platform PRIVATE SLOTS

void Platform::handleAgentMessage(QStringList recipients, QByteArray msg){

}

void Platform::eraseInvalidTransportAddresses(){
    auto it = m_forwardedAgents.begin();
    while (it != m_forwardedAgents.end()){
        auto it2 = it.value().transportAddresses.begin();
        while (it2 != it.value().transportAddresses.end()){
            if (it2.value().validUntil < QTime::currentTime()){
                it.value().transportAddresses.remove(it2.key());
            } else {
                ++it2;
            }
        }
        if (it.value().transportAddresses.empty()){
            m_forwardedAgents.erase(it);
        } else {
            ++it;
        }
    }
}
























