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
#include <QTime>
#include <QCoreApplication>

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::CTcpServer(QObject *parent) :
    QObject(parent)
  , m_pReplyTcpSocket( nullptr )
  , m_pTcpServer( nullptr )
  , m_nSizeOfNextMsg( -1 )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( slot_newConnection()) );
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

    bool bSuccess = m_pTcpServer->listen( QHostAddress::Any, m_nPort );

    if ( bSuccess == true )
    {
        QLOG_DEBUG() << tr( "Start listening on eibd interface." ).toStdString().c_str();
    }
    else
    {
        QLOG_ERROR() << tr( "An Error occured while starting to listen in eibd interface:" ).toStdString().c_str() << m_pTcpServer->errorString();
    }
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

void CTcpServer::slot_newConnection()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QTcpSocket * pTcpSocket = m_pTcpServer->nextPendingConnection();
    m_grSocketMap.insert( pTcpSocket, CEibdMsg() );

    QLOG_DEBUG() << tr( "New connection via eibd interface:" ).toStdString().c_str()
                 << pTcpSocket->peerAddress().toString().toStdString().c_str()
                 << ":"
                 << pTcpSocket->peerPort();

    connect( pTcpSocket, SIGNAL( readyRead() ), this, SLOT( slot_startRead() ) );
    connect( pTcpSocket, SIGNAL( disconnected() ), this, SLOT( slot_disconnected()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_startRead()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QByteArray grDatagram;

    QTcpSocket * pTcpSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pTcpSocket == nullptr ) {
        return;
    }

    grDatagram = pTcpSocket->readAll();

    if ( grDatagram.isEmpty() == true ) {
        QLOG_DEBUG() << tr( "Incomming message triggered but nothing received. Aborting." ).toStdString().c_str();
        return;
    }

    QLOG_DEBUG() << tr( "Received via eibd Interface." ).toStdString().c_str()
                 << tr( "Client:" ).toStdString().c_str()
                 << pTcpSocket->peerAddress().toString().toStdString().c_str() << ":" << pTcpSocket->peerPort()
                 << tr( "Message:" ).toStdString().c_str()
                 << CEibdMsg::printASCII( grDatagram );

    if ( ( m_nSizeOfNextMsg > 0 ) && ( grDatagram.size() <= m_nSizeOfNextMsg ) ) {
        QLOG_DEBUG() << "Shortening message to previous submitted length. Loosing:" << CEibdMsg::printASCII( grDatagram.mid( m_nSizeOfNextMsg, grDatagram.size() - m_nSizeOfNextMsg ) );
        grDatagram = grDatagram.mid( 0, m_nSizeOfNextMsg );
    }

    QList< QByteArray > grDatagrammList = CEibdMsg::splitMessages( grDatagram );

    foreach( QByteArray grDataMsg, grDatagrammList ) {

        CEibdMsg grMsg( grDataMsg );

        switch ( grMsg.getType() ) {
        case CEibdMsg::enuMsgType_connect: {
            QLOG_INFO() << QObject::tr("Received via eibd interface: Connection request. Granted.").toStdString().c_str();
            write( pTcpSocket, grMsg.getResponse() );
            break;
        }

        case CEibdMsg::enuMsgType_EIB_OPEN_GROUPCON: {
            QLOG_INFO() << QObject::tr("Received via eibd interface: EIB_OPEN_GROUPCON ").toStdString().c_str() << grMsg.getDestAddressKnx() << QObject::tr( ". Granting.").toStdString().c_str();

            write( pTcpSocket, grMsg.getResponse() );

            m_grSocketMap[ pTcpSocket ] = grMsg;
            m_pReplyTcpSocket = pTcpSocket;
            break;
        }

        case CEibdMsg::enuMsgType_EIB_GROUP_PACKET:
        {
            QLOG_DEBUG() << QObject::tr("Received via eibd interface: EIB_GROUP_PACKET request").toStdString().c_str() << grMsg.getDestAddressKnx() << grMsg.getValue() << QObject::tr(". Forwarded.");
            emit signal_sendToHs( grMsg.getDestAddressKnx(), grMsg.getValue() );
            break;
        }

        case CEibdMsg::enuMsgType_msgSize:
        {
            QLOG_DEBUG() << QObject::tr("Received via eibd interface: message size").toStdString().c_str() << grMsg.getMsgDataSize();
            m_nSizeOfNextMsg = grMsg.getMsgDataSize();
            break;
        }
        case CEibdMsg::enuMsgType_EIB_OPEN_T_GROUP:
        {
            QLOG_INFO() << QObject::tr("Received via eibd interface: EIB_OPEN_T_GROUP ").toStdString().c_str() << grMsg.getDestAddressKnx() << QObject::tr( ". Granted.").toStdString().c_str();
            write( pTcpSocket, grMsg.getResponse() );

            m_grSocketMap[ pTcpSocket ] = grMsg;
            break;
        }
        case CEibdMsg::enuMsgType_EIB_APDU_PACKET:
        {
            CEibdMsg grFormerMsg = m_grSocketMap.value( pTcpSocket );
            QLOG_INFO() << QObject::tr("Received via eibd interface: EIB_APDU_PACKET. Assigning it to ").toStdString().c_str() << grFormerMsg.getDestAddressKnx() << QObject::tr( ". Granted.").toStdString().c_str();

            if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Write_PDU ) {
                emit signal_sendToHs( grFormerMsg.getDestAddressKnx(), grMsg.getValue() );
            }
            else if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Read_PDU ) {
                bool bRet;
                QVariant grVal = CModel::getInstance()->m_grGAState.value( grFormerMsg.getDestAddressKnx() );
                float    fVal  = grVal.toFloat( & bRet );

                if ( bRet == false ) {
                    QLOG_WARN() << tr( "Failure while trying to read GA state. Trying to read float. GA stat is" ) << grVal;
                }

                QByteArray grEibdMsg = CEibdMsg::getMessage( "", grFormerMsg.getDestAddressKnx(), fVal );
                write( m_pReplyTcpSocket, grEibdMsg );
            }
            else {
                QLOG_WARN() << QObject::tr("Unknown EIB_APDU_PACKET. Discarding.").toStdString().c_str();
            }
            break;
        }
        case CEibdMsg::enuMsgType_EIB_RESET_CONNECTION: {
            QLOG_INFO() << QObject::tr("Received via eibd interface: EIB_RESET_CONNECTION ").toStdString().c_str() << grMsg.getDestAddressKnx() << QObject::tr( ". Granted.").toStdString().c_str();
            break;
        }

        default:
        {
            if ( QString( grDatagram ) == CModel::g_sExitMessage ) {
                QLOG_INFO() << QObject::tr( "Reveived EXIT programm message via eibd interface. Shutting down." ).toStdString().c_str();
                QCoreApplication::exit();
            }
            else if ( QString( grDatagram.mid( 0, CModel::g_sLogLevelMessage.length() - 1 ) ) == CModel::g_sLogLevelMessage ) {
                grDatagram.remove( 0, CModel::g_sLogLevelMessage.length() -1  );
                int nLogLevel = grDatagram.toInt();

                QLOG_INFO() << QObject::tr( "Reveived Set Log Level message via eibd interface. Setting log level to:" ).toStdString().c_str() << nLogLevel;
                QsLogging::Logger::instance().setLoggingLevel( static_cast< QsLogging::Level >( nLogLevel ) );
                QLOG_DEBUG() << QObject::tr( "New log level is" ).toStdString().c_str() << QsLogging::Logger::instance().loggingLevel();
            }
            else {
                QLOG_WARN() << QObject::tr("Received via eibd interface: Unknown request:").toStdString().c_str() << CEibdMsg::printASCII( grDatagram );
            }
        }
        }

        if ( grMsg.getType() != CEibdMsg::enuMsgType_msgSize ) {
            m_nSizeOfNextMsg = -1;
        }
    }
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_disconnected()
{
    QLOG_TRACE() << Q_FUNC_INFO;

    QLOG_INFO() << QObject::tr("Disconnected from eibd client.").toStdString().c_str();

    QTcpSocket * pTcpSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pTcpSocket == nullptr ) {
        return;
    }

    QLOG_DEBUG() << tr( "Connection was:" ).toStdString().c_str()
                 << pTcpSocket->peerAddress().toString().toStdString().c_str() << ":" << pTcpSocket->peerPort()
                 << tr( "Socket state is").toStdString().c_str() << pTcpSocket->state();

    m_grSocketMap.remove( pTcpSocket );
    if ( m_pReplyTcpSocket == pTcpSocket ) {
        m_pReplyTcpSocket = nullptr;
    }
    pTcpSocket->deleteLater();
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_sendToEibdClient(const QString &p_sEibGroup, const QString &p_sValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    if ( m_grSocketMap.isEmpty() == true ) {
        QLOG_ERROR() << tr( "eibd interface not yet connected. Discarding message:" ).toStdString().c_str() << p_sEibGroup << p_sValue;
        return;
    }

    if ( m_pReplyTcpSocket == nullptr ) {
        QLOG_ERROR() << tr( "eibd client not available to recieve data. Discarding message:" ).toStdString().c_str() << p_sEibGroup << p_sValue;
        return;
    }

    QByteArray grMsg = CEibdMsg::getMessage( "", p_sEibGroup, p_sValue.toFloat() );

    if ( grMsg.isEmpty() == true ) {
        QLOG_ERROR() << QObject::tr( "No payload data could be obtained. Skippig message processing." ).toStdString().c_str();
        return;
    }

    if ( m_pReplyTcpSocket->state() != QTcpSocket::ConnectedState ) {
        QLOG_DEBUG() << QObject::tr( "No eibd client connected to hsd server. Discarding incomming EIB/KNX update." ).toStdString().c_str();
        //return;
    }

    write( m_pReplyTcpSocket, grMsg );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

qint64 CTcpServer::write( QTcpSocket * p_pTcpSocket, const QByteArray &p_grData )
{
    QLOG_TRACE() << Q_FUNC_INFO;

    if ( p_pTcpSocket == nullptr ) {
        return -1;
    }

    if ( p_grData.isEmpty() == true ) {
        QLOG_WARN() << QObject::tr( "Send request but nothing to send available." );
        return -1;
    }

    if ( p_pTcpSocket->state() == QAbstractSocket::UnconnectedState ) {
        QLOG_ERROR() << QObject::tr( "Socket is unconnected." );
        return -1;
    }

    qint64 nDataWritten = p_pTcpSocket->write( p_grData );

    QLOG_DEBUG() << QObject::tr( "Sending to eibd client:" ).toStdString().c_str()
                 << p_pTcpSocket->peerAddress().toString().toStdString().c_str() << ":"
                 << p_pTcpSocket->peerPort()
                 << CEibdMsg::printASCII( p_grData )
                 << QObject::tr( "Bytes written:" ).toStdString().c_str()
                 << nDataWritten;

    return nDataWritten;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////
