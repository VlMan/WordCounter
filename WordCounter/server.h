#pragma once

#include <QObject>
#include <QTcpServer>

class Server : public QObject
{
    Q_OBJECT

public:
    Server(QObject* parent = nullptr);

private slots:
    void HandleNewConnection();

    void HandleReadyRead();

private:
    QTcpServer* _server;
};
