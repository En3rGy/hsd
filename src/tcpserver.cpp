#include "tcpserver.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QVector>
#include <QSettings>
#include <QDebug>
#include <QBitArray>
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
    qDebug() << "Received request: " << printASCII( grDatagram );

    QString sString( grDatagram.data() );

    switch ( grDatagram.length() )
    {
    case 2: // connect
        if ( ( grDatagram.data()[0] == CModel::g_uzEibAck[0] ) &&
             ( grDatagram.data()[1] == CModel::g_uzEibAck[1] ) )
        {
                qDebug() << "Request Connection";
                m_pTcpSocket->write( QByteArray( ( const char * ) CModel::g_uzEibAck, 2 ) );
        }
        break;

    case 5: // openGroupSocket
        if ( ( grDatagram.data()[0] == CModel::g_uzEibOpenGroupCon[0] ) &&
             ( grDatagram.data()[1] == CModel::g_uzEibOpenGroupCon[1] ) &&
             ( grDatagram.data()[2] == CModel::g_uzEibOpenGroupCon[2] ) &&
             ( grDatagram.data()[3] == CModel::g_uzEibOpenGroupCon[3] ) &&
             ( grDatagram.data()[4] == CModel::g_uzEibOpenGroupCon[4] ) )
        {
            qDebug() << "Request openGroupSocket";
            const char szOpenGroupSocketAck [2] = { 0x00, 0x26 };
            m_pTcpSocket->write( QByteArray( szOpenGroupSocketAck, 2 ) );
        }
        break;

    case 6: // set request, e.g. 00 27 09 0f 00 80
        if ( ( grDatagram.data()[0] == CModel::g_uzEibGroupPacket[0] ) &&
             ( grDatagram.data()[1] == CModel::g_uzEibGroupPacket[1] ) )
        {
            QByteArray grEibAdr;
            grEibAdr.append( grDatagram.data()[2]);
            grEibAdr.append( grDatagram.data()[3]);
            QString sEibAddr = hex2eib( grEibAdr );

            if ( (uchar) grDatagram.data()[5] == CModel::g_uzEibOff ) // off
            {
                qDebug() << "Set GA" << sEibAddr << "\toff";
                emit signal_setEibAdress( sEibAddr, 0 );
            }
            else if ( (uchar) grDatagram.data()[5] == CModel::g_uzEibOn ) // on
            {
                qDebug() << "Set GA" << sEibAddr << "\ton";
                emit signal_setEibAdress( sEibAddr, 1 );
            }
        }
        break;

    case 8: // re-send request, e.g. 00 06 00 27 11 0f 00 80
        if ( ( grDatagram.data()[0] == 0x00 ) &&
             ( grDatagram.data()[1] == 0x06 ) )
            if ( ( grDatagram.data()[2] == CModel::g_uzEibGroupPacket[0] ) &&
                 ( grDatagram.data()[3] == CModel::g_uzEibGroupPacket[1] ) )
            {
                QByteArray grEibAdr;
                grEibAdr.append( grDatagram.data()[4]);
                grEibAdr.append( grDatagram.data()[5]);
                QString sEibAddr = hex2eib( grEibAdr );

                if ( (uchar) grDatagram.data()[7] == CModel::g_uzEibOff ) // off
                {
                    qDebug() << "Set GA" << sEibAddr << "\toff";
                    emit signal_setEibAdress( sEibAddr, 0 );
                }
                else if ( (uchar) grDatagram.data()[7] == CModel::g_uzEibOn ) // on
                {
                    qDebug() << "Set GA" << sEibAddr << "\ton";
                    emit signal_setEibAdress( sEibAddr, 1 );
                }
            }

        break;

    default:
        qDebug() << "Received unknown request: " << printASCII( grDatagram );
    }



    emit signal_receivedMessage( sString );
}

QString CTcpServer::printASCII(QByteArray & p_grByteArray)
{
    QString sResult;
    for ( int i = 0; i < p_grByteArray.length(); i++ )
    {
        sResult += QString::number( uchar( p_grByteArray.at( i ) ), 16 ) + " ";
    }

    return sResult;
}

QString CTcpServer::hex2eib( QByteArray & p_grHexAddr )
{
    // eib 2 hex: The most significant Bit is always zero, followed by 4 bits for the
    // maingroup, 3 bits for the middlegroup and 8 bits for the subgrou

    // 0hhh hmmm

    if ( p_grHexAddr.length() != 2 )
    {
        qDebug() << "Wrong length" << Q_FUNC_INFO;
        return QString();
    }

    uchar szData [ 3 ];
    szData[0] = (uchar) p_grHexAddr.at(0);
    szData[1] = (uchar) p_grHexAddr.at(0);
    szData[2] = (uchar) p_grHexAddr.at(1);

    szData[0] = szData[0] << 1;
    QString sMainAddr   = QString::number( ( szData[0] >> 4 ) & 0xf );

    QString sMiddleAddr = QString::number( ( szData[1] & 0x7 ) );
    QString sUnderAddr  = QString::number( szData[2] );
    QString sSeparator = "/";

    QString sGA = sMainAddr + sSeparator + sMiddleAddr + sSeparator + sUnderAddr;
    return sGA;
}
