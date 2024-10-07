

#include "PayloadGeneratingService.h"
#include "QDebug"

PayloadGeneratingService::PayloadGeneratingService(QObject* parent)
    : QObject(parent)
{
}

void PayloadGeneratingService::selectIco(const QString& PathToIco)
{
    qDebug() << "PathToIco: " << PathToIco;
}
