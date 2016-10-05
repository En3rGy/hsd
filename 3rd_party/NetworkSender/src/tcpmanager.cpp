#include "tcpmanager.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QDebug>
#include "NetworkSender.h"

CTcpManager::CTcpManager(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pTcpServer( NULL )
{
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( solt_newConnection()) );
}

CTcpManager::~CTcpManager()
{
    m_pTcpServer->close();
}

void CTcpManager::sendData(const QByteArray &p_grData, const QString &p_sHostAdress, const quint16 p_unPort)
{

    QHostAddress grHostAdress( p_sHostAdress );
    QTcpSocket grTcpSocket;

    qDebug() << "# Send hex " << CNetworkSender::printASCII( p_grData ) << " to " << p_sHostAdress << "Port " << p_unPort << Q_FUNC_INFO;
    grTcpSocket.connectToHost( grHostAdress, p_unPort );
    if ( grTcpSocket.waitForConnected() )
    {
        grTcpSocket.write( p_grData );
        if ( grTcpSocket.waitForBytesWritten() )
        {
        }
        else
        {
            qDebug () << "Timout sending data";
        }
        grTcpSocket.disconnect();
        if ( grTcpSocket.waitForDisconnected() )
        {
            qDebug() << "Data sent succsefully, connection closed again.";
        }
        else
        {
            qDebug () << "Timout disconnecting";
        }
    }
    else
    {
        qDebug() << "Timeout connecting to host: " << p_sHostAdress << ":" << p_unPort;
    }
}

void CTcpManager::listen( const uint p_nPort )
{
    m_nPort = p_nPort;

    if ( m_pTcpServer->isListening() == true )
    {
        m_pTcpServer->close();
    }

    m_pTcpServer->listen( QHostAddress::Any, m_nPort );
}

void CTcpManager::solt_newConnection()
{
    m_pTcpSocket = m_pTcpServer->nextPendingConnection();

    connect( m_pTcpSocket, SIGNAL(readyRead()), this, SLOT( slot_startRead() ) );
}

void CTcpManager::slot_startRead()
{
    QByteArray grDatagram;
    grDatagram = m_pTcpSocket->readAll();

    QString sString( grDatagram.data() );

    emit signal_receivedMessage( sString );

    m_pTcpSocket->close();
}
