#include "tcpserver.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QSettings>
#include <QDebug>
#include "model.h"


CTcpServer::CTcpServer(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pTcpServer( NULL )
  , m_pSettings( NULL )
{
    m_pSettings  = new QSettings();
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( solt_newConnection()) );
}

CTcpServer::~CTcpServer()
{
    m_pTcpServer->close();
}

void CTcpServer::listen( const uint p_nPort )
{
    m_nPort = p_nPort;

    if ( m_pTcpServer->isListening() == true )
    {
        m_pTcpServer->close();
    }

    m_pTcpServer->listen( QHostAddress::Any, m_nPort );
}

void CTcpServer::listen()
{
    m_nPort = m_pSettings->value( CModel::g_sKey_HsdPort, 6720 ).value< qint16 >();
    listen( m_nPort );
}

void CTcpServer::solt_newConnection()
{
    m_pTcpSocket = m_pTcpServer->nextPendingConnection();

    connect( m_pTcpSocket, SIGNAL(readyRead()), this, SLOT( slot_startRead() ) );
}

void CTcpServer::slot_startRead()
{
    QByteArray grDatagram;
    grDatagram = m_pTcpSocket->readAll();

    QString sString( grDatagram.data() );

    qDebug() << "IN: " << grDatagram.length() << grDatagram;

    if ( grDatagram.data() == QByteArray( "\0" ) )
    {
        qDebug() << "Out:" << grDatagram.length() << grDatagram;
        m_pTcpSocket->write( grDatagram );
    }

    emit signal_receivedMessage( sString );

    // m_pTcpSocket->close();
}
