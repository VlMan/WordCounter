#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QFile>

class Client : public QObject
{
    Q_OBJECT

public:
    Client(const QString& filename, QObject* parent = nullptr);

    void Start()
    {
        qInfo() << "Client: Connecting to server...";
        _socket->connectToHost("localhost", 6000);
    }

private slots:
    void OnConnected();

    void OnReadyRead();

    void OnDisconnected()
    {
        qInfo() << "Client: Disconnected from server";
        emit Finished();
    }

signals:
    void Finished();

private:
    void ContinueWriting();
    bool OpenFile();

    QTcpSocket* _socket;
    QString _filename;
    QFile _file;
};