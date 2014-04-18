#ifndef DSMTS_GLOBAL_H
#define DSMTS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DSMTS_LIBRARY)
#  define DSMTSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DSMTSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DSMTS_GLOBAL_H
