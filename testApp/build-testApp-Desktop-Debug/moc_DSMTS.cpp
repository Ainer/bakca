/****************************************************************************
** Meta object code from reading C++ file 'DSMTS.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../testApp/DSMTS.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DSMTS.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DiscoveryService_t {
    QByteArrayData data[5];
    char stringdata[91];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_DiscoveryService_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_DiscoveryService_t qt_meta_stringdata_DiscoveryService = {
    {
QT_MOC_LITERAL(0, 0, 16),
QT_MOC_LITERAL(1, 17, 22),
QT_MOC_LITERAL(2, 40, 0),
QT_MOC_LITERAL(3, 41, 24),
QT_MOC_LITERAL(4, 66, 23)
    },
    "DiscoveryService\0forwardedAgentsUpdated\0"
    "\0QHash<QString,AgentInfo>\0"
    "processPendingDatagrams\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DiscoveryService[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       4,    0,   27,    2, 0x08,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void DiscoveryService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DiscoveryService *_t = static_cast<DiscoveryService *>(_o);
        switch (_id) {
        case 0: _t->forwardedAgentsUpdated((*reinterpret_cast< QHash<QString,AgentInfo>(*)>(_a[1]))); break;
        case 1: _t->processPendingDatagrams(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DiscoveryService::*_t)(QHash<QString,AgentInfo> );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DiscoveryService::forwardedAgentsUpdated)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject DiscoveryService::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DiscoveryService.data,
      qt_meta_data_DiscoveryService,  qt_static_metacall, 0, 0}
};


const QMetaObject *DiscoveryService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DiscoveryService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DiscoveryService.stringdata))
        return static_cast<void*>(const_cast< DiscoveryService*>(this));
    return QObject::qt_metacast(_clname);
}

int DiscoveryService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DiscoveryService::forwardedAgentsUpdated(QHash<QString,AgentInfo> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_MessageTransportService_t {
    QByteArrayData data[11];
    char stringdata[175];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MessageTransportService_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MessageTransportService_t qt_meta_stringdata_MessageTransportService = {
    {
QT_MOC_LITERAL(0, 0, 23),
QT_MOC_LITERAL(1, 24, 13),
QT_MOC_LITERAL(2, 38, 0),
QT_MOC_LITERAL(3, 39, 12),
QT_MOC_LITERAL(4, 52, 18),
QT_MOC_LITERAL(5, 71, 13),
QT_MOC_LITERAL(6, 85, 25),
QT_MOC_LITERAL(7, 111, 7),
QT_MOC_LITERAL(8, 119, 26),
QT_MOC_LITERAL(9, 146, 8),
QT_MOC_LITERAL(10, 155, 18)
    },
    "MessageTransportService\0needAgentList\0"
    "\0messageReady\0notifyMessageReady\0"
    "handleRequest\0Tufao::HttpServerRequest&\0"
    "request\0Tufao::HttpServerResponse&\0"
    "response\0processHttpMessage\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MessageTransportService[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06,
       3,    2,   40,    2, 0x06,
       4,    1,   45,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       5,    2,   48,    2, 0x08,
      10,    0,   53,    2, 0x08,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList, QMetaType::QByteArray,    2,    2,
    QMetaType::Void, QMetaType::QByteArray,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    QMetaType::Void,

       0        // eod
};

void MessageTransportService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MessageTransportService *_t = static_cast<MessageTransportService *>(_o);
        switch (_id) {
        case 0: _t->needAgentList(); break;
        case 1: _t->messageReady((*reinterpret_cast< QStringList(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 2: _t->notifyMessageReady((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 3: _t->handleRequest((*reinterpret_cast< Tufao::HttpServerRequest(*)>(_a[1])),(*reinterpret_cast< Tufao::HttpServerResponse(*)>(_a[2]))); break;
        case 4: _t->processHttpMessage(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MessageTransportService::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MessageTransportService::needAgentList)) {
                *result = 0;
            }
        }
        {
            typedef void (MessageTransportService::*_t)(QStringList , QByteArray );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MessageTransportService::messageReady)) {
                *result = 1;
            }
        }
        {
            typedef void (MessageTransportService::*_t)(QByteArray );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MessageTransportService::notifyMessageReady)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject MessageTransportService::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MessageTransportService.data,
      qt_meta_data_MessageTransportService,  qt_static_metacall, 0, 0}
};


const QMetaObject *MessageTransportService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MessageTransportService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MessageTransportService.stringdata))
        return static_cast<void*>(const_cast< MessageTransportService*>(this));
    return QObject::qt_metacast(_clname);
}

int MessageTransportService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void MessageTransportService::needAgentList()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MessageTransportService::messageReady(QStringList _t1, QByteArray _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MessageTransportService::notifyMessageReady(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_Platform_t {
    QByteArrayData data[6];
    char stringdata[76];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Platform_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Platform_t qt_meta_stringdata_Platform = {
    {
QT_MOC_LITERAL(0, 0, 8),
QT_MOC_LITERAL(1, 9, 18),
QT_MOC_LITERAL(2, 28, 0),
QT_MOC_LITERAL(3, 29, 10),
QT_MOC_LITERAL(4, 40, 3),
QT_MOC_LITERAL(5, 44, 30)
    },
    "Platform\0handleAgentMessage\0\0recipients\0"
    "msg\0eraseInvalidTransportAddresses\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Platform[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x08,
       5,    0,   29,    2, 0x08,

 // slots: parameters
    QMetaType::Void, QMetaType::QStringList, QMetaType::QByteArray,    3,    4,
    QMetaType::Void,

       0        // eod
};

void Platform::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Platform *_t = static_cast<Platform *>(_o);
        switch (_id) {
        case 0: _t->handleAgentMessage((*reinterpret_cast< QStringList(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 1: _t->eraseInvalidTransportAddresses(); break;
        default: ;
        }
    }
}

const QMetaObject Platform::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Platform.data,
      qt_meta_data_Platform,  qt_static_metacall, 0, 0}
};


const QMetaObject *Platform::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Platform::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Platform.stringdata))
        return static_cast<void*>(const_cast< Platform*>(this));
    return QObject::qt_metacast(_clname);
}

int Platform::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
