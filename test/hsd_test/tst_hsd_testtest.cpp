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
    void testDtp9();
    void testBinDecr();
    void testPrintBin();
};

Hsd_testTest::Hsd_testTest()
{

}

void Hsd_testTest::testDtp9()
{
    QByteArray grBtMsg;
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x08 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x27 );
    grBtMsg.append( 0x25 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( (char) 0x00 );
    grBtMsg.append( 0x80 );
    grBtMsg.append( 0x8A );
    grBtMsg.append( 0x24 );

    CEibdMsg grMsg( grBtMsg );
    QString sMsg = "Value was " + QString::number(grMsg.getValue().toFloat());
    QVERIFY2( grMsg.getValue().toFloat() == -30.0f, sMsg.toStdString().c_str() );

    grBtMsg.insert( 8, 0x19 );
    grBtMsg.insert( 9, 0x3D );
    grBtMsg.remove( 10, 2 );

    CEibdMsg grMsg2( grBtMsg );

    QVERIFY( grMsg2.getDestAddressKnx() == "4/5/0" );
    sMsg = "Value was " + QString::number(grMsg2.getValue().toFloat());
    QVERIFY2( grMsg2.getValue().toFloat() == 25.36f, sMsg.toStdString().c_str() );
}

void Hsd_testTest::testBinDecr()
{
    qint16 nRes = CEibdMsg::bDecr( 2 );
    QVERIFY( nRes == 1 );

    nRes = CEibdMsg::bDecr( -30 );
    QString sRes = "Result was " + QString::number( nRes );
    QVERIFY2( nRes == -31, sRes.toStdString().c_str() );
}

void Hsd_testTest::testPrintBin()
{
    QString sRes = CEibdMsg::printBin( 1 );
    QVERIFY( sRes == "0000 0000 0000 0001" );

    sRes = CEibdMsg::printBin( 19871 );
    QVERIFY( sRes == "0100 1101 1001 1111" );
}

QTEST_APPLESS_MAIN(Hsd_testTest)

#include "tst_hsd_testtest.moc"
