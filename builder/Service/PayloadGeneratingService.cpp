

#include "PayloadGeneratingService.h"
#include "QDebug"

PayloadGeneratingService::PayloadGeneratingService(QObject* parent)
    : QObject(parent)
{
}

bool PayloadGeneratingService::checkTgBotToken(const QString& botToken, const QString& chatId)
{
    auto manager = new QNetworkAccessManager(this);
    QObject::connect(manager, &QNetworkAccessManager::finished,
        this, [&](QNetworkReply* reply)
        {
            if(reply->error())
            {
                qDebug() << "Error: " << reply->errorString();
                return;
            }

            const auto answer = reply->readAll();

            qDebug() << "Reply: " << answer;
        });

    return true;
}

void PayloadGeneratingService::selectIco(const QString& pathToIco)
{
    qDebug() << "PathToIco: " << pathToIco;
}
