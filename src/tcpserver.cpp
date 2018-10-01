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
    QTcpSocket * pInEibdSocket = m_pTcpServer->nextPendingConnection();
    m_grConnectionMap.insert( pInEibdSocket, CEibdMsg() );

    QLOG_DEBUG() << tr( "Eibd in: New connection" ).toStdString().c_str()
                 << pInEibdSocket->peerAddress().toString().toStdString().c_str()
                 << ":" << pInEibdSocket->peerPort();

    connect( pInEibdSocket, SIGNAL( readyRead() ), this, SLOT( slot_startRead() ) );
    connect( pInEibdSocket, SIGNAL( disconnected() ), this, SLOT( slot_disconnected()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_startRead()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QByteArray grDatagram;

    QTcpSocket * pInEibdSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pInEibdSocket == nullptr ) {
        return;
    }

    grDatagram = pInEibdSocket->readAll();

    if ( grDatagram.isEmpty() == true ) {
        QLOG_DEBUG() << tr( "Eibd in: Incomming message triggered but nothing received. Aborting." ).toStdString().c_str();
        return;
    }

    QString sLogMsg;

    sLogMsg = tr( "Eibd in: From: " )
            + pInEibdSocket->peerAddress().toString()
            + ":" + QString::number( pInEibdSocket->peerPort() )
            +  tr( " Message: " ) + CEibdMsg::printASCII( grDatagram );

    if ( ( m_nSizeOfNextMsg > 0 ) && ( grDatagram.size() < m_nSizeOfNextMsg ) ) {
        QLOG_DEBUG() << "Shortening message to previous submitted length. Loosing:" << CEibdMsg::printASCII( grDatagram.mid( m_nSizeOfNextMsg, grDatagram.size() - m_nSizeOfNextMsg ) );
        grDatagram = grDatagram.mid( 0, m_nSizeOfNextMsg );
    }

    QList< QByteArray > grDatagrammList = CEibdMsg::splitMessages( grDatagram );

    foreach( QByteArray grDataMsg, grDatagrammList ) {

        sLogMsg = tr( "Eibd in: From: " )
                + pInEibdSocket->peerAddress().toString()
                + ":" + QString::number( pInEibdSocket->peerPort() )
                +  tr( " Message: " ) + CEibdMsg::printASCII( grDataMsg );

        CEibdMsg grMsg( grDataMsg );

        switch ( grMsg.getType() ) {
        case CEibdMsg::enuMsgType_connect: {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("Connection request. Granted.");
            write( pInEibdSocket, grMsg.getResponse() );
            break;
        }

        case CEibdMsg::enuMsgType_EIB_OPEN_GROUPCON: {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_OPEN_GROUPCON ")
                         << grMsg.getDestAddressKnx()
                         << QObject::tr( ". Granting.");

            write( pInEibdSocket, grMsg.getResponse() );

            m_grConnectionMap[ pInEibdSocket ] = grMsg;
            m_pReplyTcpSocket = pInEibdSocket;
            break;
        }

        case CEibdMsg::enuMsgType_EIB_GROUP_PACKET:
        {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_GROUP_PACKET request")
                         << grMsg.getDestAddressKnx()
                         << grMsg.getValue().toString()
                         << QObject::tr(". Forwarded.");
            emit signal_sendToHs( grMsg.getDestAddressKnx(), grMsg.getValue() );
            break;
        }

        case CEibdMsg::enuMsgType_msgSize:
        {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("Message size")
                         << QString::number ( grMsg.getMsgDataSize() );
            m_nSizeOfNextMsg = grMsg.getMsgDataSize();
            break;
        }
        case CEibdMsg::enuMsgType_EIB_OPEN_T_GROUP:
        {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_OPEN_T_GROUP ")
                         << grMsg.getDestAddressKnx()
                         << QObject::tr( ". Granted.");
            write( pInEibdSocket, grMsg.getResponse() );

            m_grConnectionMap[ pInEibdSocket ] = grMsg;
            break;
        }
        case CEibdMsg::enuMsgType_EIB_APDU_PACKET:
        {
            CEibdMsg grFormerMsg = m_grConnectionMap.value( pInEibdSocket );
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_APDU_PACKET. Assigning it to ")
                         << grFormerMsg.getDestAddressKnx();

            if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Write_PDU ) {
                QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_APDU_PACKET. Writing.");
                emit signal_sendToHs( grFormerMsg.getDestAddressKnx(), grMsg.getValue() );
            }
            else if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Read_PDU ) {
                bool     bRet;
                QVariant grVal = CModel::getInstance()->m_grGAState.value( grFormerMsg.getDestAddressKnx() );
                float    fVal  = grVal.toFloat( & bRet );

                if ( bRet == false ) {
                    QLOG_WARN() << tr( "Failure while trying to read GA state. Trying to read float. GA stat is" ) << grVal;
                }

                /// @todo Implement read from HS.

                QByteArray grEibdMsg = CEibdMsg::getMessage( "", grFormerMsg.getDestAddressKnx(), fVal, grDataMsg );
                QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_APDU_PACKET. Reading.");
                write( pInEibdSocket, grEibdMsg );
            }
            else {
                QLOG_WARN() << QObject::tr("Unknown EIB_APDU_PACKET. Discarding.").toStdString().c_str();
            }
            break;
        }
        case CEibdMsg::enuMsgType_EIB_RESET_CONNECTION: {
            QLOG_DEBUG() << sLogMsg.toStdString().c_str() << QObject::tr("EIB_RESET_CONNECTION ")
                         << grMsg.getDestAddressKnx()
                         << QObject::tr( ". Granted.");
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
                QLOG_WARN() << QObject::tr("Eibd in: Unknown request:").toStdString().c_str() << CEibdMsg::printASCII( grDatagram );
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

    QTcpSocket * pTcpSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pTcpSocket == nullptr ) {
        QLOG_WARN() << QObject::tr("Eibd in: Disconnected from eibd client.").toStdString().c_str();
        return;
    }

    QLOG_WARN() << QObject::tr("Eibd in: Disconnected from eibd client.").toStdString().c_str()
                << tr( "Connection was" ).toStdString().c_str()
                << pTcpSocket->peerAddress().toString().toStdString().c_str() << ":" << pTcpSocket->peerPort();

    m_grConnectionMap.remove( pTcpSocket );
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

    if ( m_grConnectionMap.isEmpty() == true ) {
        QLOG_ERROR() << tr( "eibd interface not yet connected. Discarding message:" ).toStdString().c_str() << p_sEibGroup << p_sValue;
        return;
    }

    QByteArray grMsg = CEibdMsg::getMessage( "", p_sEibGroup, p_sValue.toFloat() );

    if ( grMsg.isEmpty() == true ) {
        QLOG_ERROR() << QObject::tr( "No payload data could be obtained. Skippig message processing." ).toStdString().c_str();
        return;
    }

    // provide data to all connected eibd clients.
    foreach ( QTcpSocket * pEibdSocket, m_grConnectionMap.keys() ) {
        if ( pEibdSocket == nullptr ) {
            continue;
        }

        write( pEibdSocket, grMsg );
    }

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
        QLOG_ERROR() << QObject::tr( "Socket not connected. Not sending." ).toStdString().c_str()
                     << p_pTcpSocket->peerAddress() << ":" << p_pTcpSocket->peerPort();
        return -1;
    }

    qint64 nDataWritten = p_pTcpSocket->write( p_grData );

    QLOG_DEBUG() << QObject::tr( "Eibd out:" ).toStdString().c_str()
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
