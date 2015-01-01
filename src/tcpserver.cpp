#include "tcpserver.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QDebug>
#include <QBitArray>
#include "eibdmsg.h"
#include "model.h"
#include "koxml.h"
#include "eibdmsg.h"
#include "QsLog.h"
#include <QCoreApplication>

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::CTcpServer(QObject *parent) :
    QObject(parent)
  , m_pTcpSocket( NULL )
  , m_pTcpServer( NULL )
  , m_nSizeOfNextMsg( -1 )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( solt_newConnection()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::~CTcpServer()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pTcpServer->close();
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::listen( const uint &p_nPort )
{
    QLOG_TRACE() << Q_FUNC_INFO;
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
    QLOG_TRACE() << Q_FUNC_INFO;
    QVariant grPort = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort, uint( 6720 ) );
    m_nPort = grPort.value< qint16>();
    listen( m_nPort );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::solt_newConnection()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pTcpSocket = m_pTcpServer->nextPendingConnection();

    connect( m_pTcpSocket, SIGNAL(readyRead()), this, SLOT( slot_startRead() ) );
    connect( m_pTcpSocket, SIGNAL( disconnected()), this, SLOT( slot_disconnected()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_startRead()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QByteArray grDatagram;
    grDatagram = m_pTcpSocket->readAll();

    QLOG_DEBUG() << "Received via eibd Interface:" << CEibdMsg::printASCII( grDatagram );

    if ( ( m_nSizeOfNextMsg > 0 ) && ( grDatagram.size() <= m_nSizeOfNextMsg ) )
    {
        QLOG_DEBUG() << "Shortening message to previous submitted length. Loosing:" << CEibdMsg::printASCII( grDatagram.mid( m_nSizeOfNextMsg, grDatagram.size() - m_nSizeOfNextMsg ) );
        grDatagram = grDatagram.mid( 0, m_nSizeOfNextMsg );
    }

    CEibdMsg grMsg( grDatagram );

    switch ( grMsg.getType() )
    {
    case CEibdMsg::enuMsgType_connect:
    {
        QLOG_INFO() << QObject::tr("Received via eibd interface: Connection request. Granted.");
        m_pTcpSocket->write( grMsg.getResponse() );
    }
        break;

    case CEibdMsg::enuMsgType_openGroupSocket:
    {
        QLOG_INFO() << QObject::tr("Received via eibd interface: openGroupSocket request. Granted.");
        m_pTcpSocket->write( grMsg.getResponse() );
    }
        break;

    case CEibdMsg::enuMsgType_simpleWrite:
    {
        QLOG_DEBUG() << QObject::tr("Received via eibd interface: simpleWrite request") << grMsg.getDestAddress() << grMsg.getValue() << QObject::tr(". Forwarded.");
        emit signal_setEibAdress( grMsg.getDestAddress(), grMsg.getValue().toString() );
        /// @todo Process values others than int.
    }
        break;

    case CEibdMsg::enuMsgType_msgSize:
    {
        QLOG_DEBUG() << QObject::tr("Received via eibd interface: message size") << grMsg.getMsgDataSize();
        m_nSizeOfNextMsg = grMsg.getMsgDataSize();
        break;
    }

    default:
    {
        if ( QString( grDatagram ) == CModel::g_sExitMessage )
        {
            QLOG_INFO() << QObject::tr( "Reveived EXIT programm message via eibd interface. Shutting down." );
            QCoreApplication::exit();
        }
        else
        {
            QLOG_WARN() << QObject::tr("Received via eibd interface: Unknown request:") << CEibdMsg::printASCII( grDatagram ) << "=" << QString( grDatagram );
        }
    }
    }

    if ( grMsg.getType() != CEibdMsg::enuMsgType_msgSize )
    {
        m_nSizeOfNextMsg = -1;
    }
}

void CTcpServer::slot_disconnected()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QLOG_INFO() << QObject::tr("Disconnected from eibd client");
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_groupWrite(const QString &p_sEibGroup, const QString &p_sValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;
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
        QLOG_DEBUG() << QObject::tr( "No eibd client connected to hsd server. Discarding incomming EIB/KNX update." );
        return;
    }

    QLOG_DEBUG() << QObject::tr("Sending via eibd interface") << CEibdMsg::printASCII( grMsg );
    m_pTcpSocket->write( grMsg );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////
