#include <QByteArray>
#include <QCoreApplication>
#include <QHostAddress>
#include <QUdpSocket>
#include <QDebug>
#include <QDateTime>
#include <QFile>

//#define TEST_UNPACH
#define TIME_WIDTH 13
#define TEST_SEND_PACKED_DATA

bool testUnpack(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        while (!file.atEnd())
        {
            char len[sizeof(uint32_t)] = {0};
            file.read(len, sizeof(uint32_t));
            uint32_t iLen = *(uint32_t*)(&len);

            char time[TIME_WIDTH+1] = {0};
            file.read(time, TIME_WIDTH);
            QByteArray gzip = file.read(iLen-TIME_WIDTH);
            QString gzipFileName = QString(time) + ".gz";
            QFile fileGzip(gzipFileName);
            if (fileGzip.open(QFile::WriteOnly))
            {
                fileGzip.write(gzip, iLen-13);
                fileGzip.close();
                qDebug()<<gzipFileName<<"end";
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
    for(int i=0; i<17000; ++i)
    {
#ifdef TEST_SEND_PACKED_DATA
        QByteArray msg = QString("market.btcusdt.depth.step0,%1,17671.09,2,17671.640000,1")
                .arg(QString::number(QDateTime::currentMSecsSinceEpoch())).toLatin1();
        qus.writeDatagram(msg, QHostAddress(ip), port);
        qDebug() << "--- Sender ---"<<msg<< endl;
#else
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
#endif
    }
#endif

    return a.exec();
}
