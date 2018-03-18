#include <QString>
#include <QtTest>
#include <eibdmsg.h>
#include <QsLog.h>

class Hsd_testTest : public QObject
{
    Q_OBJECT

public:
    Hsd_testTest();

private Q_SLOTS:
    void testCase1();
};

Hsd_testTest::Hsd_testTest()
{

}

void Hsd_testTest::testCase1()
{
    //-30 = 1   0001 010 0010 0100

    QByteArray grBtMsg;
    // 00 08 00 27 25 00 00 80 86 52
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x08 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x27 );
    grBtMsg.append( 0x25 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x80 );
    grBtMsg.append( 0x8A );
    grBtMsg.append( 0xA4 );

    CEibdMsg grMsg( grBtMsg );
    //qDebug() << grMsg.getValue();
    QString sMsg = "Value was " + QString::number(grMsg.getValue().toFloat());
    QVERIFY2( grMsg.getValue().toFloat() == -30.0, sMsg.toStdString().c_str() );

    grBtMsg.insert( 8, 0x19 );
    grBtMsg.insert( 9, 0x3D );
    grBtMsg.remove( 10, 2 );

    CEibdMsg grMsg2( grBtMsg );

    QVERIFY( grMsg2.getDestAddressKnx() == "4/5/0" );
    sMsg = "Value was " + QString::number(grMsg2.getValue().toFloat());
    QVERIFY2( grMsg2.getValue().toFloat() == 25.36, sMsg.toStdString().c_str() );
}

QTEST_APPLESS_MAIN(Hsd_testTest)

#include "tst_hsd_testtest.moc"
