#include "tcpserver.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QDebug>
#include <QBitArray>
#include "eibdmsg.h"
#include "model.h"
// #include "koxml.h"
#include "eibdmsg.h"
#include <QtLogging>
#include <QtDebug>
#include <QTime>
#include <QCoreApplication>
#include <groupaddress.h>

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::CTcpServer(QObject *parent) :
    QObject(parent)
  , m_pReplyTcpSocket( nullptr )
  , m_pTcpServer( nullptr )
  , m_nSizeOfNextMsg( -1 )
{
    m_pTcpServer = new QTcpServer( this );

    connect( m_pTcpServer, SIGNAL( newConnection()), SLOT( slot_newConnection()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CTcpServer::~CTcpServer()
{
    m_pTcpServer->close();
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

    bool bSuccess = m_pTcpServer->listen( QHostAddress::Any, m_nPort );

    if ( bSuccess == true )
    {
        qDebug() << tr( "Start listening on eibd interface." ).toStdString().c_str();
    }
    else
    {
        qDebug() << tr( "An Error occured while starting to listen in eibd interface:" ).toStdString().c_str() << m_pTcpServer->errorString();
    }
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::listen()
{
    QVariant grPort = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort, uint( 6720 ) );
    m_nPort = grPort.value< qint16>();
    listen( m_nPort );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_newConnection()
{
    QTcpSocket * pInEibdSocket = m_pTcpServer->nextPendingConnection();
    m_grConnectionMap.insert( pInEibdSocket, CEibdMsg() );

    QString sFrom = "eibd://" + pInEibdSocket->peerAddress().toString() + ":" + QString::number( pInEibdSocket->peerPort() );
    CModel::getInstance()->logCSV( "hsd", sFrom, "", "", "New connection", "");

    connect( pInEibdSocket, SIGNAL( readyRead() ), this, SLOT( slot_startRead() ) );
    connect( pInEibdSocket, SIGNAL( disconnected() ), this, SLOT( slot_disconnected()) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CTcpServer::slot_startRead()
{
    QByteArray grDatagram;

    QTcpSocket * pInEibdSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pInEibdSocket == nullptr ) {
        return;
    }

    grDatagram = pInEibdSocket->readAll();

    if ( grDatagram.isEmpty() == true ) {
        qDebug() << tr( "Eibd in: Incomming message triggered but nothing received. Aborting." ).toStdString().c_str();
        return;
    }

    if ( ( m_nSizeOfNextMsg > 0 ) && ( grDatagram.size() < m_nSizeOfNextMsg ) ) {
        qDebug() << "Shortening message to previous submitted length. Loosing:" << CEibdMsg::printASCII( grDatagram.mid( m_nSizeOfNextMsg, grDatagram.size() - m_nSizeOfNextMsg ) );
        grDatagram = grDatagram.mid( 0, m_nSizeOfNextMsg );
    }

    QList< QByteArray > grDatagrammList = CEibdMsg::splitMessages( grDatagram );

    QString sEibdCon = "eibd://" + pInEibdSocket->peerAddress().toString() + ":" + QString::number( pInEibdSocket->peerPort() );

    foreach( QByteArray grDataMsg, grDatagrammList ) {
        CEibdMsg grMsg( grDataMsg );

        switch ( grMsg.getType() ) {
        case CEibdMsg::enuMsgType_connect: {
            CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), grMsg.getValue().toString(), "Connection request.", CEibdMsg::printASCII( grDataMsg ));
            write( pInEibdSocket, grMsg.getResponse() );
            break;
        }

        case CEibdMsg::enuMsgType_EIB_OPEN_GROUPCON: {
           CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), grMsg.getValue().toString(), "EIB_OPEN_GROUPCON", CEibdMsg::printASCII( grDataMsg ));
            write( pInEibdSocket, grMsg.getResponse() );

            m_grConnectionMap[ pInEibdSocket ] = grMsg;
            m_pReplyTcpSocket = pInEibdSocket;

            // Provide history data
            foreach( QString sGA, CModel::getInstance()->m_grGAState.keys() ) {
                bool bSuccess = false;
                float fVal = CModel::getInstance()->m_grGAState.value( sGA ).toFloat( & bSuccess );
                if ( bSuccess ) {
                    QByteArray grHistMsg = CEibdMsg::getMessage( "", sGA, fVal );
                    write( pInEibdSocket, grHistMsg );
                }
            }
            break;
        }

        case CEibdMsg::enuMsgType_EIB_GROUP_PACKET:
        {
            CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), grMsg.getValue().toString(), "EIB_GROUP_PACKET", CEibdMsg::printASCII( grDataMsg ));
            emit signal_sendToHs( grMsg.getDestAddressKnx(), grMsg.getValue() );
            break;
        }

        case CEibdMsg::enuMsgType_msgSize:
        {
            CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), QString::number ( grMsg.getMsgDataSize() ), "MSG SIZE Package", CEibdMsg::printASCII( grDataMsg ));
            m_nSizeOfNextMsg = grMsg.getMsgDataSize();
            break;
        }
        case CEibdMsg::enuMsgType_EIB_OPEN_T_GROUP:
        {
            CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), grMsg.getValue().toString(), "EIB_OPEN_T_GROUP", CEibdMsg::printASCII( grDataMsg ) );
            write( pInEibdSocket, grMsg.getResponse() );

            m_grConnectionMap[ pInEibdSocket ] = grMsg;
            break;
        }
        case CEibdMsg::enuMsgType_EIB_APDU_PACKET:
        {
            CEibdMsg grFormerMsg = m_grConnectionMap.value( pInEibdSocket );

            if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Write_PDU ) {
                grMsg.setEibdMsg( grDataMsg, grFormerMsg.getDestAddressKnx() );
                CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), grMsg.getValue().toString(), "EIB_APDU_PACKET WRITE", CEibdMsg::printASCII( grDataMsg ));
                emit signal_sendToHs( grFormerMsg.getDestAddressKnx(), grMsg.getValue() );
            }
            else if ( grMsg.getAPDUType() == CEibdMsg::enuAPDUType_A_GroupValue_Read_PDU ) {
                bool     bRet;
                QVariant grVal = CModel::getInstance()->m_grGAState.value( grFormerMsg.getDestAddressKnx() );
                float    fVal  = grVal.toFloat( & bRet );

                if ( bRet == false ) {
                    qWarning() << tr( "Failure while trying to read GA state. Trying to read float. GA stat is" ) << grVal;
                }

                /// @todo Implement read from HS.

                QByteArray grEibdMsg = CEibdMsg::getMessage( "", grFormerMsg.getDestAddressKnx(), fVal, grDataMsg );
                CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), QString::number ( grMsg.getMsgDataSize() ), "EIB_APDU_PACKET READ", CEibdMsg::printASCII( grDataMsg ));

                write( pInEibdSocket, grEibdMsg );
            }
            else {
                QString sLogMsg;

                sLogMsg = tr( "Eibd in: From: " )
                        + pInEibdSocket->peerAddress().toString()
                        + ":" + QString::number( pInEibdSocket->peerPort() )
                        +  tr( " Message: " ) + CEibdMsg::printASCII( grDatagram );

                qWarning() << sLogMsg.toStdString().c_str() << QObject::tr("Unknown EIB_APDU_PACKET. Discarding. ")
                           << grFormerMsg.getDestAddressKnx();
            }
            break;
        }
        case CEibdMsg::enuMsgType_EIB_RESET_CONNECTION: {
            CModel::getInstance()->logCSV( "hsd", sEibdCon, grMsg.getDestAddressKnx(), QString::number ( grMsg.getMsgDataSize() ), "EIB_RESET_CONNECTION", CEibdMsg::printASCII( grDataMsg) );

            break;
        }

        default:
        {
            if ( QString( grDatagram ) == CModel::g_sExitMessage ) {
                qInfo() << QObject::tr( "Reveived EXIT programm message via eibd interface. Shutting down." ).toStdString().c_str();
                QCoreApplication::exit();
            }
            else if ( QString( grDatagram.mid( 0, CModel::g_sLogLevelMessage.length() - 1 ) ) == CModel::g_sLogLevelMessage ) {
                grDatagram.remove( 0, CModel::g_sLogLevelMessage.length() -1  );
                int nLogLevel = grDatagram.toInt();

                qInfo() << QObject::tr( "Reveived Set Log Level message via eibd interface. Setting log level to:" ) << nLogLevel;
                // QsLogging::Logger::instance().setLoggingLevel( static_cast< QsLogging::Level >( nLogLevel ) );
                qDebug() << QObject::tr( "New log level is" );
            }
            else {
                qWarning() << QObject::tr("Eibd in: Unknown request:") << CEibdMsg::printASCII( grDatagram );
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
    QTcpSocket * pTcpSocket = dynamic_cast< QTcpSocket * >( sender() );
    if ( pTcpSocket == nullptr ) {
        CModel::getInstance()->logCSV( "hsd", "", "", "", QObject::tr("Disconnected from eibd client."), "" );
        return;
    }

    CModel::getInstance()->logCSV( "hsd",
                                  QString( "eibd://" + pTcpSocket->peerAddress().toString() + ":" + QString::number( pTcpSocket->peerPort() ) ),
                                  "", "", QObject::tr("Disconnected from eibd client."), "" );

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
    if ( m_grConnectionMap.isEmpty() == true ) {
        qCritical() << tr( "eibd interface not yet connected. Discarding message:" ).toStdString().c_str() << p_sEibGroup << p_sValue;
        return;
    }

    QByteArray grMsg = CEibdMsg::getMessage( "", p_sEibGroup, p_sValue.toFloat() );

    if ( grMsg.isEmpty() == true ) {
        qCritical() << QObject::tr( "No payload data could be obtained. Skippig message processing." ).toStdString().c_str();
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
    if ( p_pTcpSocket == nullptr ) {
        return -1;
    }

    if ( p_grData.isEmpty() == true ) {
        qWarning() << QObject::tr( "Send request but nothing to send available." );
        return -1;
    }

    if ( p_pTcpSocket->state() == QAbstractSocket::UnconnectedState ) {
        qCritical() << QObject::tr( "Socket not connected. Not sending." ).toStdString().c_str()
                     << p_pTcpSocket->peerAddress() << ":" << p_pTcpSocket->peerPort();
        return -1;
    }

    qint64 nDataWritten = p_pTcpSocket->write( p_grData );

    CGroupAddress grAddr;
    grAddr.setHex( p_grData.mid(6, 2) );
    QString  sEibdCon = "eibd://" + p_pTcpSocket->peerAddress().toString() + ":" + QString::number( p_pTcpSocket->peerPort() );
    CModel::getInstance()->logCSV( sEibdCon, "hsd", grAddr.toKNXString(), "", "", CEibdMsg::printASCII( p_grData ));

    return nDataWritten;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////
