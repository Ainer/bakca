#include <QCoreApplication>
#include "DSMTS.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DiscoveryService DS;
    return a.exec();
}
