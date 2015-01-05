#include "tcpclient.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QHostAddress>
#include <QTextCodec>
#include <koxml.h>
#include <model.h>
#include "QsLog.h"
#include <QCoreApplication>
#include <QTimer>
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
  , m_bReceviedXML( false )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    //QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "Windows-1252" ) );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpClient::~CTcpClient()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_pTcpSocket != NULL )
    {
        m_pTcpSocket->close();
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::send(const QString & p_sAction , const QString &p_sGA, const QVariant &p_grValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_pTcpSocket == NULL )
    {
        QLOG_WARN() << QObject::tr("Connection with HS not yet initialized.");
        return;
    }

    if ( m_pTcpSocket->state() != QAbstractSocket::ConnectedState )
    {
        QLOG_WARN() << QObject::tr("Not connected with HS.").toStdString().c_str();

        if ( initConnection() == false )
        {
            QLOG_WARN() << QObject::tr("Connection could not re-establish. Discarding message.").toStdString().c_str();
            return;
        }
    }

    CGroupAddress grGA;
    grGA.setKNXString( p_sGA );

    int     nVal     = grGA.toHSRepresentation();
    QString sMessage = p_sAction
                        + m_sSeperatorChar
                        + QString::number( nVal )
                        + m_sSeperatorChar
                        + QString::number( p_grValue.toFloat() )
                        + m_sMsgEndChar;

    QByteArray grArray;
    grArray.append( sMessage );
    grArray.append( m_sMsgEndChar );

    int nRet = m_pTcpSocket->write( grArray );
    if ( m_pTcpSocket->waitForBytesWritten() == false )
    {
        QLOG_ERROR() << m_pTcpSocket->errorString() << QObject::tr("while communicating with HS.").toStdString().c_str();
        return;
    }

    if ( nRet == -1 )
    {
        QLOG_ERROR() << m_pTcpSocket->errorString() << QObject::tr("while communicating with HS.").toStdString().c_str();
    }
    else
    {
        QLOG_DEBUG() << QObject::tr("Send message to HS:").toStdString().c_str() << grArray << QObject::tr("Message size in byte:").toStdString().c_str() << nRet;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_startRead()
{
    QLOG_TRACE() << Q_FUNC_INFO;
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

    QLOG_DEBUG() << QObject::tr("Received via HS interface:").toStdString().c_str()
                 << sGA
                 << QObject::tr("Value:").toStdString().c_str()
                 << sValue;

    if ( grGA.isValid() == true )
    {
        emit signal_receivedMessage( sGA, sValue );
    }
    else
    {
        // Report invalid GA just once
        if ( m_grInvlGAList.contains( grGA.toKNXString() ) == false )
        {
            m_grInvlGAList.push_back( grGA.toKNXString() );

            QLOG_ERROR() << QObject::tr( "GA is not valid. Message is not processed further. GA was:" ).toStdString().c_str() << sGA;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_webRequestReadFinished()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_grWebRequestData.append( m_pWebRequestTcpSocket->readAll() );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_webRequestClosed()
{
    QLOG_DEBUG() << tr("Web request closed").toStdString().c_str();
    QLOG_TRACE() << Q_FUNC_INFO;

    QLOG_INFO() << QObject::tr( "Connection to peer closed:" ).toStdString().c_str()
                << m_pWebRequestTcpSocket->peerAddress().toString()
                << QObject::tr( ":" ).toStdString().c_str()
                << m_pWebRequestTcpSocket->peerPort();

    m_pWebRequestTcpSocket->close();

    // Interprete XmlFile
    CKoXml::getInstance()->setXml( m_grWebRequestData );

    m_grWebRequestData.clear();

    m_bReceviedXML = true;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_setEibAdress(const QString &p_sEibAddr, const QVariant &p_grVal )
{
    QLOG_TRACE() << Q_FUNC_INFO;

    send( "1", p_sEibAddr, p_grVal.toString() );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_disconnected()
{
    QLOG_WARN() << tr("Disconnected from Homeserver.").toStdString().c_str();

    initConnection();
}

void CTcpClient::slot_reconnect()
{
    QLOG_INFO() << tr( "Trying to reconnect to HS." ).toStdString().c_str();
    initConnection();
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::splitString(const QString &p_sIncoming, QString &p_sType, QString &p_sGA, QString &p_sValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QVector<QString> grGAVec;
    QString sTemp;

    for ( int i = 0; i < p_sIncoming.length(); i++ )
    {
        if ( p_sIncoming.at( i ) == m_sSeperatorChar.at( 0 ) )
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

bool CTcpClient::initConnection()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_pTcpSocket == NULL )
    {
        m_pTcpSocket = new QTcpSocket( this );
        connect( m_pTcpSocket, SIGNAL( readyRead()), this, SLOT( slot_startRead() ) );
        connect( m_pTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_disconnected()));
    }

    QVariant grHsIp = CModel::getInstance()->getValue( CModel::g_sKey_HSIP, QString( "192.168.143.11" )  );
    QHostAddress grHostAddress( grHsIp.toString() );


    QVariant grHsGwPort = CModel::getInstance()->getValue( CModel::g_sKey_HSGwPort, uint( 7003 ) );
    uint unPort = grHsGwPort.toUInt();

    QLOG_DEBUG() << grHostAddress.toString() << unPort;

    m_pTcpSocket->connectToHost( grHostAddress, unPort);
    if( m_pTcpSocket->waitForConnected( 2000 ) )
    {
        QLOG_INFO() << QObject::tr("Connection with HS established").toStdString().c_str();

        QByteArray grArray;

        QVariant grHsGwPass = CModel::getInstance()->getValue( CModel::g_sKey_HSGwPassword, QString( "" ) );
        QString sPass = grHsGwPass.toString();

        grArray.append( sPass );
        grArray.append( m_sMsgEndChar );

        QLOG_INFO() << QObject::tr("Sending to HS: Initialization message.").toStdString().c_str();
        int nRet = m_pTcpSocket->write( grArray );

        if ( nRet == -1 )
        {
            QLOG_ERROR() << m_pTcpSocket->errorString();
            int nTimeout_ms = CModel::getInstance()->getValue( CModel::g_sKey_PauseTilHSReconnect, 30000 ).toInt();
            QTimer::singleShot( nTimeout_ms, this, SLOT( slot_reconnect()) );
            return false;
        }
        else
        {
            QLOG_DEBUG() << QObject::tr("Send message to HS:").toStdString().c_str() << grArray << QObject::tr("Message size in byte:").toStdString().c_str() << nRet;
        }
    }
    else
    {
        QLOG_ERROR() << m_pTcpSocket->errorString() << tr( "trying to init communication with HS." ).toStdString().c_str();
        int nTimeout_ms = CModel::getInstance()->getValue( CModel::g_sKey_PauseTilHSReconnect, 30000 ).toInt();
        QTimer::singleShot( nTimeout_ms, this, SLOT( slot_reconnect()) );
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::getGaXml()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    // Load Xml file form HS
    QVariant grHsIp = CModel::getInstance()->getValue( CModel::g_sKey_HSIP, QString( "192.168.143.11" ) );
    QString  sHsIp  = grHsIp.toString();

    QVariant grHsPort = CModel::getInstance()->getValue( CModel::g_sKey_HSWebPort, uint( 80 ) );
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
        QLOG_WARN() << QObject::tr("HS web request already ongoing").toStdString().c_str();
        return;
    }

    m_pWebRequestTcpSocket->connectToHost( sHsIp, unHsPort );
    if ( m_pWebRequestTcpSocket->waitForConnected( 2000 ) == true )
    {
        m_pWebRequestTcpSocket->write( sWebRequest.toLatin1() );

        if ( m_pWebRequestTcpSocket->waitForBytesWritten() == true )
        {
            QLOG_DEBUG() << tr("Wrote to HS").toStdString().c_str();

            m_bReceviedXML = false;

            while ( m_bReceviedXML == false )
            {
                QCoreApplication::processEvents();
            }
        }
        else
        {
            QLOG_ERROR() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
            return;
        }
    }
    else
    {
        QLOG_ERROR() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
        return;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::sendData(const QString &p_sDestAddr, const int &p_nPort, const QByteArray &p_grData)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    QTcpSocket grSocket;

    QHostAddress grHostAddress( p_sDestAddr );
    grSocket.connectToHost( grHostAddress, p_nPort );

    if( grSocket.waitForConnected( 2000 ) )
    {
        int nRet = grSocket.write( p_grData );

        if ( grSocket.waitForBytesWritten( 2000 ) )
        {
            QLOG_DEBUG() << tr( "Send" ) << nRet <<tr( "byte:" ).toStdString().c_str() << p_grData << " = " << QString( p_grData ) << tr("to").toStdString().c_str() << p_sDestAddr << ":" << p_nPort;
        }
        else
        {
            QLOG_ERROR() << grSocket.errorString() << tr( "while trying to send" ).toStdString().c_str() << p_grData << tr( "to").toStdString().c_str() << p_sDestAddr << ":" << p_nPort;
        }
    }
    else
    {
        QLOG_ERROR() << grSocket.errorString() << tr( "while trying to connect to" ).toStdString().c_str() << p_sDestAddr << ":" << p_nPort;
    }
}
