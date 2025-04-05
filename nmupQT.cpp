#include <QApplication>
#include <QTcpSocket>
#include <QDebug>

void scanPort(const QString& ip, int port) {
    QTcpSocket socket;
    socket.connectToHost(ip, port);
    if (socket.waitForConnected(500)) {
        qDebug() << "Port" << port << "is OPEN!";
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    for(int i = 0;i<=30000;i++)scanPort("127.0.0.1",i);
    return a.exec();
}
