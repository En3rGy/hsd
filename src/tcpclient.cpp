#include "tcpclient.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QHostAddress>
#include <QTextCodec>
#include <QSettings>
#include <koxml.h>
#include <model.h>
#include "groupaddress.h"

const QChar   CTcpClient::m_sMsgEndChar    = '\0';
const QString CTcpClient::m_sSeperatorChar = "|";


//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpClient::CTcpClient(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pWebRequestTcpSocket( NULL )
  , m_pSettings( NULL )
{
    m_pSettings = new QSettings( CModel::g_sSettingsPath, QSettings::IniFormat );
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "Windows-1252" ) );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpClient::~CTcpClient()
{
    if ( m_pTcpSocket != NULL )
    {
        m_pTcpSocket->close();
    }

    m_pSettings->sync();

    delete m_pSettings;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::send(const QString & p_sAction , const QString &p_sGA, const QString &p_sValue)
{
    if ( m_pTcpSocket == NULL )
    {
        qDebug() << "Init connection first";
        return;
    }

    if ( m_pTcpSocket->state() != QAbstractSocket::ConnectedState )
    {
        qDebug() << "Not connected";
        return;
    }

    CGroupAddress grGA;
    grGA.setKNXString( p_sGA );

    int     nVal     = grGA.toHSRepresentation();
    QString sMessage = p_sAction
                        + m_sSeperatorChar
                        + QString::number( nVal )
                        + m_sSeperatorChar
                        + p_sValue
                        + m_sMsgEndChar;

    QByteArray grArray;
    grArray.append( sMessage );
    grArray.append( m_sMsgEndChar );

    int nRet = m_pTcpSocket->write( grArray );
    if ( m_pTcpSocket->waitForBytesWritten() == false )
    {
        qDebug() << "Timoeout while sending";
        return;
    }

    if ( nRet == -1 )
    {
        qDebug() << m_pTcpSocket->errorString() << Q_FUNC_INFO;
    }
    else
    {
        qDebug() << "Send" << nRet << "byte:" << grArray;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_startRead()
{
    QByteArray grDatagram;

    grDatagram = m_pTcpSocket->readAll();

    QString sString( grDatagram.data() );

    QString sType;
    QString sIntGA;
    QString sGA;
    QString sValue;

    splitString( sString, sType, sIntGA, sValue );

    CGroupAddress grGA;
    grGA.setHS( sIntGA.toInt() );

    sGA = grGA.toKNXString();

    qDebug() << "Received via HS interface:" << sGA << "\tValue:" << sValue;

    if ( grGA.isValid() == true )
    {
        emit signal_receivedMessage( sGA, sValue );
    }
//    else
//    {
//        qDebug() << sGA << "is not valid.";
//    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_webRequestReadFinished()
{
    m_grWebRequestData.append( m_pWebRequestTcpSocket->readAll() );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_webRequestClosed()
{
    m_pWebRequestTcpSocket->close();

    // Interprete XmlFile
    CKoXml::getInstance()->setXml( m_grWebRequestData );

    m_grWebRequestData.clear();
}

void CTcpClient::slot_setEibAdress(const QString &p_sEibAddr, const int &p_nVal)
{
    send( "1", p_sEibAddr, QString::number( p_nVal ) );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::splitString(const QString &p_sIncoming, QString &p_sType, QString &p_sGA, QString &p_sValue)
{
    QVector<QString> grGAVec;
    QString sTemp;

    for ( int i = 0; i < p_sIncoming.length(); i++ )
    {
        if ( p_sIncoming.at( i ) == '|' )
        {
            grGAVec.push_back( sTemp );
            sTemp.clear();
        }
        else
        {
            sTemp = sTemp + p_sIncoming.at( i );
        }
    }
    grGAVec.push_back( sTemp );

    if ( grGAVec.count() == 3 )
    {
        p_sType  = grGAVec.at( 0 );
        p_sGA    = grGAVec.at( 1 );
        p_sValue = grGAVec.at( 2 );
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::initConnection( const QString &p_sPass)
{
    if ( m_pTcpSocket == NULL )
    {
        m_pTcpSocket = new QTcpSocket( this );
        connect( m_pTcpSocket, SIGNAL( readyRead()), this, SLOT( slot_startRead() ) );
    }

    QVariant grHsIp = m_pSettings->value( CModel::g_sKey_HSIP );
    if ( grHsIp.isNull() == true )
    {
        m_pSettings->setValue( CModel::g_sKey_HSIP, QString( "192.168.143.11" ) );
        grHsIp.setValue( QString( "192.168.143.11" ) );
    }
    QHostAddress grHostAddress( grHsIp.toString() );


    QVariant grHsGwPort = m_pSettings->value( CModel::g_sKey_HSGwPort );
    if ( grHsGwPort.isNull() == true )
    {
        m_pSettings->setValue( CModel::g_sKey_HSGwPort, uint( 7003 ) );
        grHsGwPort.setValue( uint( 7003 ) );
    }
    uint unPort = grHsGwPort.toUInt();

    qDebug() << grHostAddress << unPort;

    m_pTcpSocket->connectToHost( grHostAddress, unPort);
    if( m_pTcpSocket->waitForConnected( 2000 ) )
    {
        qDebug() << "Connection with HS established" ;

        QByteArray grArray;
        grArray.append( p_sPass );
        grArray.append( m_sMsgEndChar );

        qDebug() << "Sending to HS: Initialization message." ;
        int nRet = m_pTcpSocket->write( grArray );

        if ( nRet == -1 )
        {
            qDebug() << "Error reported while sending" ;
        }
        else
        {
            qDebug() << "Send" << nRet << "byte: " << grArray;
        }
    }
    else
    {
        qDebug() << m_pTcpSocket->errorString() << Q_FUNC_INFO;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::getGaXml()
{
    // Load Xml file form HS
    QVariant grHsIp     = m_pSettings->value( CModel::g_sKey_HSIP );
    if ( grHsIp.isNull() == true )
    {
        m_pSettings->setValue( CModel::g_sKey_HSIP, "192.168.143.11" );
        grHsIp.setValue( QString( "192.168.143.11" ) );
    }
    QString sHsIp = grHsIp.toString();

    QVariant grHsPort = m_pSettings->value( CModel::g_sKey_HSWebPort );
    if ( grHsPort.isNull() == true )
    {
        m_pSettings->setValue( CModel::g_sKey_HSWebPort, uint( 80 ) );
        grHsPort.setValue( uint( 80 ) );
    }
    uint unHsPort = grHsPort.toUInt();


    QString sWebRequest = "GET /hscl?sys/cobjects.xml HTTP/1.0\r\n\r\n";

    if ( m_pWebRequestTcpSocket == NULL )
    {
        m_pWebRequestTcpSocket = new QTcpSocket( this );
        connect( m_pWebRequestTcpSocket, SIGNAL( readChannelFinished()), this, SLOT( slot_webRequestReadFinished()) );
        connect( m_pWebRequestTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_webRequestClosed() ) );
    }

    if ( m_pWebRequestTcpSocket->state() == QTcpSocket::ConnectedState )
    {
        qDebug() << "Web request ongoing";
        return;
    }

    m_pWebRequestTcpSocket->connectToHost( sHsIp, unHsPort );
    if ( m_pWebRequestTcpSocket->waitForConnected( 2000 ) == true )
    {
        m_pWebRequestTcpSocket->write( sWebRequest.toAscii() );

        if ( m_pWebRequestTcpSocket->waitForBytesWritten() == true )
        {
        }
        else
        {
            qDebug() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
            return;
        }
    }
    else
    {
        qDebug() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
        return;
    }
}
