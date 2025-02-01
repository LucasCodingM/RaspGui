#include "openmeteoapi.h"
#include "customexception.h"

openMeteoAPI::openMeteoAPI(QObject *parent)
    : QObject{parent}
    , m_url("https://api.open-meteo.com/v1/"
            "forecast?latitude=48.112&longitude=-1.6743&current=temperature_2m&daily=weather_code&"
            "timezone=auto&forecast_days=3")
    , m_reply(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
    QObject::connect(m_networkManager,
                     &QNetworkAccessManager::finished,
                     this,
                     &openMeteoAPI::serviceRequestFinished);
}

void openMeteoAPI::serviceRequestFinished(QNetworkReply *networkReply)
{
    m_reply = networkReply;
    if (m_reply->error() != QNetworkReply::NoError) {
        THROW_CUSTOM_EXCEPTION(m_reply->errorString());
        return;
    }
    qDebug() << "Receive https reply";
    // Parse the JSON response
    QByteArray response = m_reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    m_jsonInfoWeather = doc.object();
    m_reply->deleteLater();
}

void openMeteoAPI::fetchWeather()
{
    m_networkManager->get(QNetworkRequest(m_url));
}

QJsonArray openMeteoAPI::getListDaysWeatherCode()
{
    if (m_jsonInfoWeather.empty()) {
        THROW_CUSTOM_EXCEPTION("Weather data not available");
        return QJsonArray();
    }
    return m_jsonInfoWeather["weather_code"].toArray();
}

double openMeteoAPI::getTemperature()
{
    if (m_jsonInfoWeather.empty()) {
        THROW_CUSTOM_EXCEPTION("Weather temperature not available");
        return 0;
    }
    return m_jsonInfoWeather["current"].toObject()["temperature_2m"].toDouble();
}
