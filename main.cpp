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

int findPos(const char *data, uint32_t dataLength, char c)
{
    char *ptr = (char*)data;
    for (uint32_t i=0; i<dataLength; i++)
    {
       if (*ptr == c)
       {
          return i;
       }
       else if (*ptr == '\0')
       {
          return i-1;
       }

       ptr++;
    }
    return -1;
}

char* parsePackage(const char *pack,
                   char **nameEndPtr,
                   char **dataEndPtr)
{
    char *ptr = (char*)pack;
   if (strlen(ptr) == 0)
    {
        printf("end parse string\n");
        return NULL;
    }

    int nameEndPos = findPos(ptr, strlen(ptr), ',');
    if (nameEndPos == -1)
    {
        printf("error:can not find name(,):%s\n", ptr);
        return NULL;
    }

    ptr += nameEndPos;//name end
    *nameEndPtr = ptr;
    ptr += 1; //,

    int dataEndPos = findPos(ptr, strlen(ptr), '\n');
    if (dataEndPos == -1)
    {
        printf("error:can not find end(\n):%s\n", ptr);
        return NULL;
    }

    ptr += dataEndPos;
    *dataEndPtr = ptr;
    ptr += 1;

    return ptr;
}

void test()
{
    QByteArray msg = "";
        for(int j=0; j<170; ++j)
        {
            msg += QString("market.btcusdt.depth.step%1,%2,17671.09,2,17671.640000,%3\n")
                    .arg(j%5)
                    .arg(QString::number(QDateTime::currentMSecsSinceEpoch()))
                    .arg(j/5).toLatin1();
        }

    char *nameEndPtr = NULL;
    char *dataEndPtr = NULL;

    char *ptr = msg.data();
    char name[64] = {0};
    char data[128] = {0};
    int index = 0;
    while(ptr != NULL)
    {
        char *tmp = parsePackage(ptr, &nameEndPtr, &dataEndPtr);
        if (!tmp)
        {
            break;
        }

        memset(name, 0, sizeof(name));
        int nameLen = nameEndPtr - ptr;
        memcpy(name, ptr, nameEndPtr-ptr);

        memset(data, 0, sizeof(data));
        int dataLen = dataEndPtr - nameEndPtr - 1;
        memcpy(data, nameEndPtr+1, dataEndPtr-nameEndPtr-1);

        printf("index:%d,name:%s,data:%s\n", index+1, name, data);
        //qDebug()<<index+1<<",name:"<<name<<",data:"<<data<<endl;
        index++;
        ptr = tmp;
    }
}

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

#ifdef TEST_UNPACH
    testUnpack(QCoreApplication::applicationDirPath() + "/2020-11-15");
#else
    if (argc < 3)
    {
        qDebug()<<"exaple:testUdpClient 127.0.0.1 8899"<<endl;
        return -1;
    }

//    test();
//    return 0;


    QString ip = argv[1];
    qint16 port = QString(argv[2]).toShort();

    QUdpSocket qus;
    int len = 0;
    int i=0;
    while(1)
    //for(int i=0; i<100000; ++i)
    {
#ifdef TEST_SEND_PACKED_DATA
        QByteArray msg = QString("market.btcusdt.depth.step%1,(%2)%3,17671.09,2,17671.640000,%4")
                .arg(i%5)
                .arg(i)
                .arg(QString::number(QDateTime::currentMSecsSinceEpoch()))
                .arg(i/5).toLatin1();
        qus.writeDatagram(msg, QHostAddress(ip), port);
        qDebug() << "--- Sender ---"<<msg<< endl;

        len += msg.length();
        if (len>5*1024*1024)
        {
            qDebug()<<"end buffer:"<<i;
            break;
        }
        i++;
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

    return 0;
    //return a.exec();
}
