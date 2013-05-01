#include "tcpmanager.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QHostAddress>
#include <QTextCodec>


const QChar   CTcpManager::m_sMsgEndChar    = '\0';
const QString CTcpManager::m_sSeperatorChar = "|";


//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpManager::CTcpManager(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pTcpServer( NULL )
{
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "Windows-1252" ) );

    m_pTcpServer = new QTcpServer( this );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

CTcpManager::~CTcpManager()
{
    m_pTcpSocket->close();
    m_pTcpServer->close();
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpManager::send(const QString & p_sAction , const QString &p_sGA, const QString &p_sValue)
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

    if ( nRet == -1 )
    {
        qDebug() << "Error reported while sending" ;
    }
    else
    {
        qDebug() << "Send" << nRet << "byte:" << grArray;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

void CTcpManager::slot_startRead()
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

    qDebug() << "Received GA: " << sGA << "Value: " << sValue;

    emit signal_receivedMessage( sGA, sValue );
}

//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////

int CTcpManager::convertGA(const QString &p_sGA)
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

QString CTcpManager::convertGA(const int &p_nGA)
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

void CTcpManager::splitString(const QString &p_sIncoming, QString &p_sType, QString &p_sGA, QString &p_sValue)
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

void CTcpManager::initConnection(const QString &p_sHostAddress, const quint16 &p_unPort, const QString &p_sPass)
{
    if ( m_pTcpSocket == NULL )
    {
        m_pTcpSocket = new QTcpSocket( this );
        connect( m_pTcpSocket, SIGNAL( readyRead()), this, SLOT( slot_startRead() ) );
    }

    QHostAddress grHostAddress( p_sHostAddress );

    m_pTcpSocket->connectToHost( grHostAddress, p_unPort );
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
