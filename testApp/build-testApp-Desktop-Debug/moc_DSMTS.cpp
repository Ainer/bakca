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
    QByteArrayData data[3];
    char stringdata[43];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_DiscoveryService_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_DiscoveryService_t qt_meta_stringdata_DiscoveryService = {
    {
QT_MOC_LITERAL(0, 0, 16),
QT_MOC_LITERAL(1, 17, 23),
QT_MOC_LITERAL(2, 41, 0)
    },
    "DiscoveryService\0processPendingDatagrams\0"
    "\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DiscoveryService[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void DiscoveryService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DiscoveryService *_t = static_cast<DiscoveryService *>(_o);
        switch (_id) {
        case 0: _t->processPendingDatagrams(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
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
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
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
QT_MOC_LITERAL(1, 24, 18),
QT_MOC_LITERAL(2, 43, 0),
QT_MOC_LITERAL(3, 44, 4),
QT_MOC_LITERAL(4, 49, 13),
QT_MOC_LITERAL(5, 63, 25),
QT_MOC_LITERAL(6, 89, 7),
QT_MOC_LITERAL(7, 97, 26),
QT_MOC_LITERAL(8, 124, 8),
QT_MOC_LITERAL(9, 133, 19),
QT_MOC_LITERAL(10, 153, 20)
    },
    "MessageTransportService\0httpNotifyReceived\0"
    "\0data\0handleRequest\0Tufao::HttpServerRequest&\0"
    "request\0Tufao::HttpServerResponse&\0"
    "response\0httpNotifyReadyRead\0"
    "httpMessageReadyRead\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MessageTransportService[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       4,    2,   37,    2, 0x0a,
       9,    0,   42,    2, 0x0a,
      10,    0,   43,    2, 0x0a,

 // signals: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,

 // slots: parameters
    QMetaType::Bool, 0x80000000 | 5, 0x80000000 | 7,    6,    8,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MessageTransportService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MessageTransportService *_t = static_cast<MessageTransportService *>(_o);
        switch (_id) {
        case 0: _t->httpNotifyReceived((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: { bool _r = _t->handleRequest((*reinterpret_cast< Tufao::HttpServerRequest(*)>(_a[1])),(*reinterpret_cast< Tufao::HttpServerResponse(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: _t->httpNotifyReadyRead(); break;
        case 3: _t->httpMessageReadyRead(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MessageTransportService::*_t)(QByteArray );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MessageTransportService::httpNotifyReceived)) {
                *result = 0;
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
    if (!strcmp(_clname, "Tufao::AbstractHttpServerRequestHandler"))
        return static_cast< Tufao::AbstractHttpServerRequestHandler*>(const_cast< MessageTransportService*>(this));
    return QObject::qt_metacast(_clname);
}

int MessageTransportService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void MessageTransportService::httpNotifyReceived(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_Platform_t {
    QByteArrayData data[4];
    char stringdata[38];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_Platform_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_Platform_t qt_meta_stringdata_Platform = {
    {
QT_MOC_LITERAL(0, 0, 8),
QT_MOC_LITERAL(1, 9, 21),
QT_MOC_LITERAL(2, 31, 0),
QT_MOC_LITERAL(3, 32, 4)
    },
    "Platform\0forwardHttpNotifyToDs\0\0data\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Platform[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x0a,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,

       0        // eod
};

void Platform::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Platform *_t = static_cast<Platform *>(_o);
        switch (_id) {
        case 0: _t->forwardHttpNotifyToDs((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
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
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
