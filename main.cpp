#include <QByteArray>
#include <QCoreApplication>
#include <QHostAddress>
#include <QUdpSocket>
#include <QDebug>
#include <QDateTime>
#include <QFile>

//#define TEST_UNPACH

bool testUnpack(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        while (!file.atEnd())
        {
            int i = sizeof(uint32_t);
            char len[sizeof(uint32_t)+1] = {0};
            file.read(len, sizeof(uint32_t));
            uint32_t iLen = *(uint32_t*)(&len);

            char time[13] = {0};
            file.read(time, 13);
            QByteArray gzip = file.read(iLen-13);
            QFile fileGzip(QString(time) + ".gz");
            if (fileGzip.open(QFile::WriteOnly))
            {
                fileGzip.write(gzip, iLen-13);
                fileGzip.close();
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef TEST_UNPACH
    testUnpack(QCoreApplication::applicationDirPath() + "/2020-11-15");
#else
    if (argc < 3)
    {
        qDebug()<<"exaple:testUdpClient 127.0.0.1 8899"<<endl;
        return -1;
    }

    QString ip = argv[1];
    qint16 port = QString(argv[2]).toShort();

    QUdpSocket qus;
    for(int i=0; i<5; ++i)
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
#endif

    return a.exec();
}
