#include "tcpclient.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QHostAddress>
#include <QTextCodec>
#include <QSettings>
#include <koxml.h>
#include <model.h>

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
    m_pSettings = new QSettings();
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

    int     nVal     = convertGA( p_sGA );
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
        qDebug() << m_pTcpSocket->errorString();
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
    sGA = convertGA( sIntGA.toInt() );

    qDebug() << "Received HS GA update:" << sGA << "\tValue:" << sValue;

    emit signal_receivedMessage( sGA, sValue );
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

int CTcpClient::convertGA(const QString &p_sGA)
{
    QVector<QString> grGAVec;
    QString sTemp;

    for ( int i = 0; i < p_sGA.length(); i++ )
    {
        if ( p_sGA.at( i ) == '/' )
        {
            grGAVec.push_back( sTemp );
            sTemp.clear();
        }
        else
        {
            sTemp = sTemp + p_sGA.at( i );
        }
    }
    grGAVec.push_back( sTemp );

    if ( grGAVec.count() == 3 )
    {
        int nX = grGAVec.at( 0 ).toInt();
        int nY = grGAVec.at( 1 ).toInt();
        int nZ = grGAVec.at( 2 ).toInt();
        int nConvert = nX * 2048 + nY * 256 + nZ;

        return nConvert;
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

QString CTcpClient::convertGA(const int &p_nGA)
{
    // int nConvert = nX * 2048 + nY * 256 + nZ;

    int nX = p_nGA / 2048;
    int nY = ( p_nGA - ( nX * 2048 ) ) / 256;
    int nZ = ( p_nGA - ( nX * 2048 ) - ( nY * 256 ) );

    QString sGA = QString::number( nX ) + "/" + QString::number( nY ) + "/" + QString::number( nZ );

    return sGA;
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

    QHostAddress grHostAddress( m_pSettings->value( CModel::g_sKey_HSIP, "192.168.143.11" ).value< QString >() );

    m_pTcpSocket->connectToHost( grHostAddress, m_pSettings->value( CModel::g_sKey_HSGwPort, "7003" ).value< uint >() );
    if( m_pTcpSocket->waitForConnected( 2000 ) )
    {
        qDebug() << "Connection established" ;

        QByteArray grArray;
        grArray.append( p_sPass );
        grArray.append( m_sMsgEndChar );

        qDebug() << "Sending initialization message." ;
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
        qDebug() << m_pTcpSocket->errorString() ;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpClient::getGaXml()
{
    // Load Xml file form HS
    QString sHsIp       = m_pSettings->value( CModel::g_sKey_HSIP, "192.168.143.11" ).value< QString >();
    QString sHsPort     = m_pSettings->value( CModel::g_sKey_HSWebPort, "80" ).value< QString >();
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

    m_pWebRequestTcpSocket->connectToHost( sHsIp, sHsPort.toInt() );
    if ( m_pWebRequestTcpSocket->waitForConnected( 2000 ) == true )
    {
        m_pWebRequestTcpSocket->write( sWebRequest.toAscii() );

        if ( m_pWebRequestTcpSocket->waitForBytesWritten() == true )
        {
        }
        else
        {
            qDebug() << m_pWebRequestTcpSocket->errorString();
            return;
        }
    }
    else
    {
        qDebug() << m_pWebRequestTcpSocket->errorString();
        return;
    }
}
