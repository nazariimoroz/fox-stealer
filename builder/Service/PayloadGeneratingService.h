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

    Q_INVOKABLE bool checkTgBotToken(const QString& botToken
        , const QString& chatId);

public:
    void selectIco(const QString& pathToIco);
};



#endif //PAYLOADGENERATINGSERVICE_H
