#include "tcpclient.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QHostAddress>
// #include <QTextCodec>
#include <koxml.h>
#include <model.h>
#include <QtLogging>
#include <QCoreApplication>
#include <QTimer>
#include "groupaddress.h"
#include <QStringList>
#include <QtLogging>
#include <QtDebug>

const QChar   CTcpClient::m_sMsgEndChar    = '\0';
const QString CTcpClient::m_sSeperatorChar = "|";


//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpClient::CTcpClient(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( nullptr )
  , m_pWebRequestTcpSocket( nullptr )
  , m_bReceviedXML( false )
{

}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpClient::~CTcpClient()
{
    if ( m_pTcpSocket != nullptr )
    {
        m_pTcpSocket->close();
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::send(const QString & p_sAction , const QString &p_sGA, const QVariant & p_grValue)
{
    if ( m_pTcpSocket == nullptr )
    {
        qWarning() << QObject::tr("Connection with HS not yet initialized.");
        return;
    }

    if ( m_pTcpSocket->state() != QAbstractSocket::ConnectedState )
    {
        qWarning() << QObject::tr("Not connected with HS.");

        if ( initConnection() == false )
        {
            qWarning() << QObject::tr("Connection could not re-establish. Discarding message.");
            return;
        }
    }

    CGroupAddress grGA;
    grGA.setKNXString( p_sGA );

    int     nHsGA     = grGA.toHSRepresentation();
    QString sMessage = p_sAction
            + m_sSeperatorChar
            + QString::number( nHsGA )
            + m_sSeperatorChar
            + QString::number( p_grValue.toFloat() )
            + m_sMsgEndChar;

    QByteArray grArray;
    grArray.append( sMessage.toLatin1() );
    //grArray.append( m_sMsgEndChar ); ///< @todo check if necessary

    int nRet = m_pTcpSocket->write( grArray );
    if ( m_pTcpSocket->waitForBytesWritten() == false )
    {
        qCritical() << m_pTcpSocket->errorString() << QObject::tr("while communicating with HS.");
        return;
    }

    if ( nRet == -1 )
    {
        qCritical() << m_pTcpSocket->errorString() << QObject::tr("while communicating with HS.");
    }
    else
    {
        CModel::getInstance()->logCSV( "HS", "hsd", grGA.toKNXString(), "", "", sMessage );
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_startRead()
{
    QByteArray grDatagram;

    grDatagram = m_pTcpSocket->readAll();

    QList< QByteArray > grArrayList = grDatagram.split( '\0' );

    // remove last list entry indicating list end from hs
    if (grArrayList.last() == QByteArray(1, '\000'))
    {
        grArrayList.removeLast();
    }

    foreach ( QByteArray grMsgArray, grArrayList)
    {
        QString     sString( grMsgArray.data() );

        QString sType;
        QString sIntGA;
        QString sGA;
        QString sValue;

        splitString( sString, sType, sIntGA, sValue );

        //    Status    Beschreibung          Daten
        //    Zahl
        //    1        Wert setzen absolut    Float oder Text
        //    2        Wert setzen relativ    Float
        //    3        Step+                  leer
        //    4        Step-                  leer
        //    5        Liste+                 leer
        //    6        Liste-                 leer

        if ( sType == "99" ) // HS ping --> ignore
        {
            CModel::getInstance()->logCSV( "hsd", "HS", "", "", "Ping. No action required", grDatagram );
        }
        else  {
            /// @todo Respect different HS message types

            CGroupAddress grGA;
            grGA.setHS( sIntGA.toInt() );

            sGA = grGA.toKNXString();

            CModel::getInstance()->logCSV( "hsd", "HS", sGA, sValue, "", grMsgArray );

            if ( grGA.isValid() == true ) {
                CModel::getInstance()->m_grGAState.insert( sGA, sValue.toFloat());
                emit signal_receivedMessage( sGA, sValue );
            }
            else {
                // Report invalid GA just once
                if ( m_grInvlGAList.contains( grGA.toKNXString() ) == false ) {
                    m_grInvlGAList.push_back( grGA.toKNXString() );

                    qCritical() << QObject::tr( "GA is not valid. Message is not processed further. GA was:" ) << sGA;
                }
            }
        } //else
    } // foreach
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

void CTcpClient::slot_gaXmlWebRequestClosed()
{
    m_pWebRequestTcpSocket->close();

    qDebug() << tr("Web request asking HS for existing GAs is closed.");

    // Interprete XmlFile
    CKoXml::getInstance()->setXml( m_grWebRequestData );

    m_grWebRequestData.clear();

    m_bReceviedXML = true;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_sendToHs(const QString &p_sEibAddrKnx, const QVariant &p_grVal )
{
    send( "1", p_sEibAddrKnx, p_grVal );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_disconnected()
{
    qWarning() << tr("Disconnected from Homeserver.");

    initConnection();
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::slot_reconnect()
{
    qInfo() << tr( "Trying to reconnect to HS." );
    initConnection();
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
    if ( m_pTcpSocket == nullptr )
    {
        m_pTcpSocket = new QTcpSocket( this );
        connect( m_pTcpSocket, SIGNAL( readyRead()), this, SLOT( slot_startRead() ) );
        connect( m_pTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_disconnected()));
    }

    QVariant grHsIp = CModel::getInstance()->getValue( CModel::g_sKey_HSIP, QString( "192.168.143.11" )  );
    QHostAddress grHostAddress( grHsIp.toString() );


    QVariant grHsGwPort = CModel::getInstance()->getValue( CModel::g_sKey_HSGwPort, uint( 7003 ) );
    uint unPort = grHsGwPort.toUInt();

    m_pTcpSocket->connectToHost( grHostAddress, unPort);
    if( m_pTcpSocket->waitForConnected( 2000 ) )
    {
        QByteArray grArray;

        QVariant grHsGwPass = CModel::getInstance()->getValue( CModel::g_sKey_HSGwPassword, QString( "" ) );
        QString sPass = grHsGwPass.toString();

        grArray.append(sPass.toLatin1());
        grArray.append(m_sMsgEndChar.toLatin1());

        qDebug() << QObject::tr("HS out: Initialization message.");
        int nRet = m_pTcpSocket->write( grArray );

        if ( nRet == -1 )
        {
            qCritical() << m_pTcpSocket->errorString();
            int nTimeout_ms = CModel::getInstance()->getValue( CModel::g_sKey_PauseTilHSReconnect, 30000 ).toInt();
            QTimer::singleShot( nTimeout_ms, this, SLOT( slot_reconnect()) );
            return false;
        }
        else
        {
            qInfo() << QObject::tr("Connection with HS established");
        }
    }
    else
    {
        qCritical() << m_pTcpSocket->errorString() << tr( "trying to init communication with HS." );
        int nTimeout_ms = CModel::getInstance()->getValue( CModel::g_sKey_PauseTilHSReconnect, 30000 ).toInt();
        QTimer::singleShot( nTimeout_ms, this, SLOT( slot_reconnect()) );
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

bool CTcpClient::getGaXml()
{
    // Load Xml file form HS
    QVariant grHsIp = CModel::getInstance()->getValue( CModel::g_sKey_HSIP, QString( "192.168.143.11" ) );
    QString  sHsIp  = grHsIp.toString();

    QVariant grHsPort = CModel::getInstance()->getValue( CModel::g_sKey_HSWebPort, uint( 80 ) );
    uint unHsPort = grHsPort.toUInt();


    QString sWebRequest = "GET /hscl?sys/cobjects.xml HTTP/1.0\r\n\r\n";

    if ( m_pWebRequestTcpSocket == nullptr )
    {
        m_pWebRequestTcpSocket = new QTcpSocket( this );
        connect( m_pWebRequestTcpSocket, SIGNAL( readyRead()), this, SLOT( slot_webRequestReadFinished()) );
        connect( m_pWebRequestTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_gaXmlWebRequestClosed() ) );
    }

    if ( m_pWebRequestTcpSocket->state() == QTcpSocket::ConnectedState )
    {
        qWarning() << QObject::tr("HS web request already ongoing");
        return false;
    }

    m_pWebRequestTcpSocket->connectToHost( sHsIp, unHsPort );
    if ( m_pWebRequestTcpSocket->waitForConnected( 2000 ) == true )
    {
        m_pWebRequestTcpSocket->write( sWebRequest.toLatin1() );

        if ( m_pWebRequestTcpSocket->waitForBytesWritten() == true )
        {
            qDebug() << tr("Asked HS for existing GAs by sending: ") << sWebRequest;

            m_bReceviedXML = false;

            while ( m_bReceviedXML == false )
            {
                QCoreApplication::processEvents();
            }
        }
        else
        {
            qCritical() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
            return false;
        }
    }
    else
    {
        qCritical() << m_pWebRequestTcpSocket->errorString() << Q_FUNC_INFO;
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::sendData(const QString &p_sDestAddr, const int &p_nPort, const QByteArray &p_grData)
{
    QTcpSocket grSocket;

    QHostAddress grHostAddress( p_sDestAddr );
    grSocket.connectToHost( grHostAddress, p_nPort );

    if( grSocket.waitForConnected( 2000 ) )
    {
        int nRet = grSocket.write( p_grData );

        if ( grSocket.waitForBytesWritten( 2000 ) )
        {
            qDebug() << tr( "Send" ) << nRet <<tr( "byte:" ) << p_grData << " = " << QString( p_grData ) << tr("to") << p_sDestAddr << ":" << p_nPort;
        }
        else
        {
            qCritical() << grSocket.errorString() << tr( "while trying to send" ) << p_grData << tr( "to") << p_sDestAddr << ":" << p_nPort;
        }
    }
    else
    {
        qCritical() << grSocket.errorString() << tr( "while trying to connect to" ) << p_sDestAddr << ":" << p_nPort;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::closeConnection(const QString &p_sDestAddr, const int &p_nPort, const QByteArray &p_grData)
{
    // avoid reconnecting
    m_pTcpSocket->disconnect( SLOT( slot_disconnected()) );

    sendData( p_sDestAddr, p_nPort, p_grData );
}
