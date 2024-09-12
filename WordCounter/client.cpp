#include "client.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>

constexpr int CHUNK_SIZE = 64 * 1024; // 64 KB

Client::Client(const QString& filename, QObject *parent)
    : QObject(parent), _filename(filename)
{
    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &Client::OnConnected);
    connect(_socket, &QTcpSocket::readyRead, this, &Client::OnReadyRead);
    connect(_socket, &QTcpSocket::disconnected, this, &Client::OnDisconnected);
}

bool Client::OpenFile() 
{
    _file.setFileName(_filename);
    if (!_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Client: Couldn`t open file:" << _filename;
        _socket->disconnectFromHost();
        return false;
    }
    return true;
}

void Client::OnConnected()
{
    qInfo() << "Client: Connected to server";
   
    if (OpenFile())
        ContinueWriting();
}

void Client::ContinueWriting()
{
    while (!_file.atEnd()) {
        QByteArray chunk = _file.read(CHUNK_SIZE);
        if (chunk.isEmpty()) {
            break;
        }
        _socket->write(chunk);
        _socket->flush();
    }

    _file.close();
    _socket->write(QByteArray("EOF"));
    qInfo() << "Client: Finished sending file content to server";
}

void Client::OnReadyRead()
{
    QByteArray data = _socket->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject result = doc.object();

    qInfo() << "Client: Analysis result:";
    qInfo() << "Client: Total words:" << result["total_words"];
    qInfo() << "Client: Unique words:" << result["unique_words"];
    qInfo() << "Client: Longest unique sequence:" << result["longest_unique_sequence"];
    _socket->disconnectFromHost();
}
