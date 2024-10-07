#ifndef PAYLOADGENERATINGSERVICE_H
#define PAYLOADGENERATINGSERVICE_H

#include <QObject>
#include <QtQuick>

class PayloadGeneratingService : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PayloadGeneratingService(QObject* parent = nullptr);

public slots:
    void selectIco(const QString& PathToIco);
};



#endif //PAYLOADGENERATINGSERVICE_H
