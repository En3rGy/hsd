#include <QString>
#include <QtTest>
#include <eibdmsg.h>
// #include <QsLog.h>
#include <tcpclient.h>
#include <groupaddress.h>
#include <koxml.h>

class Hsd_testTest : public QObject
{
    Q_OBJECT

public:
    Hsd_testTest();

private Q_SLOTS:
    void testBinDecr();
    void testPrintBin();
    void testMultMsg();
    void testOpenGroupCon();
    void testSplitHSString();
    void testFromHS();
    void testAPDUMsgTest();
    void testDtp9();
};

Hsd_testTest::Hsd_testTest()
{

}

void Hsd_testTest::testDtp9()
{
    QString sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS5_16BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"4/5/0\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    // 00 08 00 27 25 00 00 80 8A 24
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

    /// vice versa

    sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS5_16BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"0/4/5\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    // CKoXml::enuDPT_DPT9

    QByteArray grMsg3 = CEibdMsg::getMessage( "", "0/4/5", "8.3" );

    qDebug() << "eibd msg:" << CEibdMsg::printASCII( grMsg3 );

    CGroupAddress grAddr;
    grAddr.setHex( grMsg3.mid(6,2) );
    qDebug() << grAddr.toKNXString();

    QVERIFY( grAddr.toKNXString() == "0/4/5" );

    QByteArray grRes;
    grRes.append( 0x03 );
    grRes.append( 0x3e );

    QVERIFY( grMsg3.mid(10, 2) == grRes );
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

void Hsd_testTest::testMultMsg()
{
    QByteArray grTestData;
    QDataStream in( & grTestData, QIODevice::ReadWrite);

    in << 0x002725000080044c;
    in << 0x0008002725050080;
    in << 0x06f4000800272506;
    in << 0x00800667;

    // 00 27 25 00 00 80 04 4c || 00 08 00 27 25 05 00 80 06 f4 || 00 08 00 27 25 06 00 80 06 67

    QList< QByteArray > grResList = CEibdMsg::splitMessages( grTestData );

    QVERIFY( grResList.size() == 3 );

    QByteArray grRes1;
    QDataStream in1( & grRes1, QIODevice::ReadWrite );
    in1 << 0x002725000080044c;

    QByteArray grRes2;
    QDataStream in2( & grRes2, QIODevice::ReadWrite );
    in2 << 0x00080027;
    in2 << 0x25050080;
    grRes2.append( 0x06 );
    grRes2.append( (char) 0xf4 );

    QByteArray grRes3;
    QDataStream in3( & grRes3, QIODevice::ReadWrite );
    in3 << 0x00080027;
    in3 << 0x25060080;
    grRes3.append( 0x06 );
    grRes3.append( 0x67 );

    QVERIFY( grRes1 == grResList.at( 0 ) );
    QVERIFY( grRes2 == grResList.at( 1 ) );
    QVERIFY( grRes3 == grResList.at( 2 ) );

    QByteArray grTestData2;
    QDataStream inA( & grTestData2, QIODevice::ReadWrite );
    grResList.clear();
    inA << 0x00050026;
    grTestData2.append( (char) 0x00 );
    grTestData2.append( (char) 0x00 );
    grTestData2.append( (char) 0x00 );

    grResList = CEibdMsg::splitMessages( grTestData2 );

    qDebug() << "Orig" << CEibdMsg::printASCII( grTestData2 );
    foreach ( QByteArray grDat, grResList ) {
        qDebug() << "Array:" << CEibdMsg::printASCII( grDat );
    }

    QVERIFY( grResList.size() == 1 );
}

void Hsd_testTest::testOpenGroupCon()
{
    // 00 05 00 26 00 00 00

    QByteArray grTestData;
    QDataStream in( & grTestData, QIODevice::ReadWrite);

    in << 0x00050026;
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x00 );

    qDebug() << CEibdMsg::printASCII( grTestData );

    CEibdMsg grMsg( grTestData );

    QVERIFY( grMsg.getType() == CEibdMsg::enuMsgType_EIB_OPEN_GROUPCON );

}

void Hsd_testTest::testSplitHSString()
{
    QString sHSStr = "1|12295|0.0";

    QString sType;
    QString sGA;
    QString sValue;

    CTcpClient grTcpClient;

    grTcpClient.splitString( sHSStr, sType, sGA, sValue );

    QVERIFY( sType == "1" );
    QVERIFY( sGA == "12295" );
    QVERIFY( sValue == "0.0" );

}

void Hsd_testTest::testFromHS()
{
    QString sGA  = "12295";
    QString sVal = "0.0";

    CGroupAddress grGA;
    grGA.setHS( sGA.toInt() );
    sGA = grGA.toKNXString();

    QVERIFY( sGA == "6/0/7" );

    // <cobject id="32988" used="1" type="eib" path="06 Tore\0 Au&#223;enanlage\" fmt="EIS2+EIS6_8BIT" fmtex="integer" name="G TargetDoorState" rem="0" init="0" min="0" max="255" step="0" list="" ga="6/0/7" ganum="12295" cogws="1" cogwr="1" scan="0"  sbc="0"  read="1"  transmit="1" />

    QString sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS2+EIS6_8BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"6/0/7\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    // CKoXml::enuDPT_DPT3

    QByteArray grMsg = CEibdMsg::getMessage( "", sGA, sVal.toFloat() );

    qDebug() << "eibd msg:" << CEibdMsg::printASCII( grMsg );

    ///////////

    // Received via HS interface: 6/0/8 Value: 0.0 "2|12296|0.0"
    // Sending to eibd client: 127.0.0.1 : 44780 "00 08 00 27 00 00 30 08 00 80" Bytes written: 10

    //<cobject id="32995" used="1" type="eib" path="06 Tore\0 Au&#223;enanlage\" fmt="EIS1+EIS2+EIS7_1BIT" fmtex="integer" name="G RM ObstructionDetected" rem="0" init="0" min="0" max="1" step="0" list="" ga="6/0/8" ganum="12296" cogws="1" cogwr="1" scan="0"  sbc="0"  read="1"  transmit="1" />

    /// 2nd run

    sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS2+EIS6_8BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"6/0/6\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    //DEBUG 2018-10-10T21:47:26.057 " ; " hsd " ; " HS " ; " 6/0/6 " ; " 3.0 " ; "  " ; " 1|12294|3.0

    QString sHsMsg = "1|12294|3.0";

    QString sType;
    QString sIntGA;
    QString sGA2;
    QString sValue;

    CTcpClient::splitString( sHsMsg, sType, sIntGA, sValue );

    CGroupAddress grGA2;
    grGA2.setHS( sIntGA.toInt() );

    sGA2 = grGA2.toKNXString();

    QVERIFY( sGA2 == "6/0/6" );
    QVERIFY( sValue == "3.0" );
    QVERIFY( sValue.toFloat() == 3.0f );

    QByteArray grMsg2 = CEibdMsg::getMessage( "", sGA2, sValue.toFloat() );

    // 00 09 00 27 00 00 30 06 00 80 03

    qDebug() << CEibdMsg::printASCII( grMsg2 );

    QVERIFY( CEibdMsg::printASCII( grMsg2 ) == "00 09 00 27 00 00 30 06 00 80 03" );

}

void Hsd_testTest::testAPDUMsgTest()
{
    // Message: 00 05 00 25 00 80 84 "EIB_APDU_PACKET. Assigning it to " "9/0/5"

    QByteArray grTestData;
    QDataStream in( & grTestData, QIODevice::ReadWrite);

    QString sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS6_8BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"6/0/6\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    in << 0x00050025;
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x80 );
    grTestData.append( (char) 0x84 );

    qDebug() << CEibdMsg::printASCII( grTestData );

    CEibdMsg grMsg;
    grMsg.setEibdMsg( grTestData, "6/0/6" );

    qDebug() << grMsg.getValue();

    QVERIFY( grMsg.getValue().toInt() == 51 );


    // check different dpt


    sXml = "<cobject id=\"32988\" used=\"1\" type=\"eib\" path=\"06 Tore\\\\0 Au&#223;enanlage\\\\\" fmt=\"EIS2+EIS6_8BIT\" fmtex=\"integer\" name=\"G TargetDoorState\" rem=\"0\" init=\"0\" min=\"0\" max=\"255\" step=\"0\" list=\"\" ga=\"6/0/7\" ganum=\"12295\" cogws=\"1\" cogwr=\"1\" scan=\"0\"  sbc=\"0\"  read=\"1\"  transmit=\"1\" />";
    CKoXml::getInstance()->setXml( sXml.toUtf8() );

    grTestData.clear();
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x25 );
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x81 );

    CEibdMsg grMsg2;
    grMsg2.setEibdMsg( grTestData, "6/0/7" );

    qDebug() << grMsg2.getValue();
    QVERIFY( grMsg2.getValue().toInt() == 1 );

//    hsd                     ;  eibd://127.0.0.1:41318  ;         ;     ;  New connection         ;
//    hsd                     ;  eibd://127.0.0.1:41318  ;  6/0/7  ;     ;  EIB_OPEN_T_GROUP       ;  00 05 00 22 30 07 ff
//    ERROR 2018-10-13T18:26:44.063 Length of hex address not equals 2 byte. Length in byte was 0
//    eibd://127.0.0.1:41318  ;  hsd                     ;  0/0/0  ;     ;                         ;  00 02 00 22
//    hsd                     ;  eibd://127.0.0.1:41318  ;         ;  0  ;  EIB_APDU_PACKET WRITE  ;  00 05 00 25 00 80 01
//    HS                      ;  hsd                     ;  6/0/7  ;     ;                         ;  1|12295|0

    grTestData.clear();
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x05 );
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x25 );
    grTestData.append( (char) 0x00 );
    grTestData.append( (char) 0x80 );
    grTestData.append( (char) 0x01 );
    CEibdMsg grMsg3;
    grMsg3.setEibdMsg( grTestData, "6/0/7" );

    QVERIFY( grMsg3.getValue().toInt() == 1 );

}

QTEST_APPLESS_MAIN(Hsd_testTest)

#include "tst_hsd_testtest.moc"
