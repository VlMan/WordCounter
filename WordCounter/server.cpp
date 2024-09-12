#include "server.h"

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

static QJsonObject AnalyzeText(const QString& text);
static QJsonArray FindLongestUniqueSequence(const QStringList& words);

Server::Server(QObject *parent)
{
    _server = new QTcpServer(this);
    connect(_server, &QTcpServer::newConnection, this, &Server::HandleNewConnection);

    if (!_server->listen(QHostAddress::Any, 6000)) {
        qWarning() << "Server: Server could not start!";
    }
    else {
        qInfo() << "Server: Server started on port 6000";
    }
}

void Server::HandleNewConnection()
{
    QTcpSocket* client_socket = _server->nextPendingConnection();
    connect(client_socket, &QTcpSocket::readyRead, this, &Server::HandleReadyRead);
    connect(client_socket, &QTcpSocket::disconnected, client_socket, &QTcpSocket::deleteLater);
    qInfo() << "Server: New client connected";
}

void Server::HandleReadyRead()
{
    QTcpSocket* client_socket = qobject_cast<QTcpSocket*>(sender());
    if (!client_socket) return;

    qInfo() << "Server: Bytes recived:" << client_socket->bytesAvailable();

    const QByteArray recieved_data = client_socket->readAll();

    static QByteArray buffer = "";

    buffer.append(recieved_data);

    if (buffer.endsWith("EOF")) {
        buffer.chop(3);
        QString text = QString::fromUtf8(buffer);
        qInfo() << "Server: Received complete text from client, length:" << text.length();
        QJsonObject result = AnalyzeText(text);

        QJsonDocument doc(result);
        client_socket->write(doc.toJson());
        client_socket->flush();
        qInfo() << "Server: Sent analysis result to client";

        buffer.clear();
    }
}

static QJsonObject AnalyzeText(const QString& text) 
{
    QRegularExpression word_regex(R"(\b\w+(['`]\w+)?\b)");
    QRegularExpressionMatchIterator it = word_regex.globalMatch(text.toLower());
    QStringList words;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        words << match.captured(0);
    }
    const int total_words = words.size();

    QSet<QString> uniqueWords(words.begin(), words.end());
    const int unique_word_count = uniqueWords.size();

    const QJsonArray longest_unique_sequence = FindLongestUniqueSequence(words);

    QJsonObject result;
    result["total_words"] = total_words;
    result["unique_words"] = unique_word_count;
    result["longest_unique_sequence"] = longest_unique_sequence;
    return result;
}

static QJsonArray FindLongestUniqueSequence(const QStringList& words) 
{
    int max_length = 0;
    int start = 0;
    QHash<QString, int> word_indices;
    int max_start = 0;

    for (int i = 0; i < words.size(); ++i) {
        const QString& word = words[i];

        if (word_indices.contains(word) && word_indices[word] >= start) {
            start = word_indices[word] + 1;
        }

        word_indices[word] = i;

        if (i - start + 1 > max_length) {
            max_length = i - start + 1;
            max_start = start;
        }
    }

    const QString phrase = words.mid(max_start, max_length).join(" ");

    QJsonArray result;
    result.append(phrase);
    result.append(max_length);

    return result;
}
