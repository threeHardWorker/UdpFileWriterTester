#include <QByteArray>
#include <QCoreApplication>
#include <QHostAddress>
#include <QUdpSocket>
#include <QDebug>
#include <QDateTime>
#include <QFile>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 3)
    {
        qDebug()<<"exaple:testUdpClient 127.0.0.1 8899"<<endl;
        return -1;
    }

    QString ip = argv[0];
    qint16 port = QString(argv[1]).toShort();

    QUdpSocket qus;
    for(int i=0; i<200; ++i)
    {
        QByteArray msg = "market.BTC-USDT.depth.step01";
        msg = msg.leftJustified(64, ' ', true);
        QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
        msg += timestamp;
        qDebug() << "--- Sender ---"<<msg<< endl;
        QString fileName = a.applicationDirPath() + "/test.zip";
        QFile file(fileName);
        if (file.open(QFile::ReadOnly))
        {
            msg += file.readAll();
            qus.writeDatagram(msg, QHostAddress(ip), port);
        }
        else
        {
            qCritical()<<"open file:"<<fileName<<"error"<<endl;
            break;
        }
    }

    return a.exec();
}
