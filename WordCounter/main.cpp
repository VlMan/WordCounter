#include <QtCore/QCoreApplication>
#include <QTimer>

#include "server.h"
#include "client.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) 
    {
        qWarning() << "Enter a filename.";
        return -1;
    }

    const QString &filename = argv[1];

    if (!QFile::exists(filename)) 
    {
        qWarning() << "The file does not exist.";
        return -1;
    }

    QScopedPointer<Server> server(new Server);
    QScopedPointer<Client> client(new Client(filename));

    // Wait until server to start
    QTimer::singleShot(100, client.data(), &Client::Start);

    QObject::connect(client.data(), &Client::Finished, &a, &QCoreApplication::quit);

    return a.exec();
}
