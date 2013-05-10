#include "tcpserver.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QSettings>
#include <QDebug>
#include <QBitArray>
#include "eibdmsg.h"
#include "model.h"
#include "koxml.h"
#include "eibdmsg.h"

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::CTcpServer(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pTcpServer( NULL )
  , m_pSettings( NULL )
{
    m_pSettings  = new QSettings( CModel::g_sSettingsPath, QSettings::IniFormat );
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( solt_newConnection()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::~CTcpServer()
{
    m_pTcpServer->close();

    m_pSettings->sync();
    delete m_pSettings;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::listen( const uint &p_nPort )
{
    m_nPort = p_nPort;

    if ( m_pTcpServer->isListening() == true )
    {
        m_pTcpServer->close();
    }

    m_pTcpServer->listen( QHostAddress::Any, m_nPort );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::listen()
{
    QVariant grPort = m_pSettings->value( CModel::g_sKey_HsdPort );
    if ( grPort.isNull() == true )
    {
        m_pSettings->setValue( CModel::g_sKey_HsdPort, uint( 6720 ) );
        grPort.setValue( uint( 6720 ) );
    }
    m_nPort = grPort.value< qint16>();
    listen( m_nPort );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::solt_newConnection()
{
    m_pTcpSocket = m_pTcpServer->nextPendingConnection();

    connect( m_pTcpSocket, SIGNAL(readyRead()), this, SLOT( slot_startRead() ) );
    connect( m_pTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_disconnected()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_startRead()
{
    QByteArray grDatagram;
    grDatagram = m_pTcpSocket->readAll();

    CEibdMsg grMsg( grDatagram );

    switch ( grMsg.getType() )
    {
    case CEibdMsg::enuMsgType_connect:
    {
        qDebug() << "Received via eibd interface: Connection request. Granted.";
        m_pTcpSocket->write( grMsg.getResponse() );
    }
        break;

    case CEibdMsg::enuMsgType_openGroupSocket:
    {
        qDebug() << "Received via eibd interface: openGroupSocket request. Granted.";
        m_pTcpSocket->write( grMsg.getResponse() );
    }
        break;

    case CEibdMsg::enuMsgType_simpleWrite:
    {
        qDebug() << "Received via eibd interface: simpleWrite request" << grMsg.getDestAddress() << grMsg.getValue() << ". Forwarded.";
        emit signal_setEibAdress( grMsg.getDestAddress(), grMsg.getValue().toInt() );
        /// @todo Process values others than int.
    }
        break;

    default:
    {
        qDebug() << "Received via eibd interface: Unknown request: " << CEibdMsg::printASCII( grDatagram );
    }
    }
}

void CTcpServer::slot_disconnected()
{
    qDebug() << "Disconnected from eibd client";
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_groupWrite(const QString &p_sEibGroup, const QString &p_sValue)
{
    if ( m_pTcpSocket == NULL )
    {
        return;
    }

    QByteArray grMsg = CEibdMsg::getMessage( "", p_sEibGroup, p_sValue.toDouble() );

    if ( grMsg.isEmpty() == true )
    {
        return;
    }

    if ( m_pTcpSocket->state() != QTcpSocket::ConnectedState )
    {
        qDebug() << "No eibd client connected to hsd server. Discarding incomming EIB/KNX update.";
        return;
    }

    qDebug() << "Sending via eibd interface" << CEibdMsg::printASCII( grMsg );
    m_pTcpSocket->write( grMsg );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////
