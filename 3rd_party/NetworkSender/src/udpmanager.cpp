#include "udpmanager.h"
#include <QUdpSocket>
#include <QVector>
#include "NetworkSender.h"

CUdpManager::CUdpManager(QObject *parent) :
    QObject(parent)
  , m_pUdpSocket( NULL )
{
}

void CUdpManager::listen( const uint p_nPort )
{
    m_nPort = p_nPort;

    if ( m_pUdpSocket != NULL )
    {
        delete m_pUdpSocket;
        m_pUdpSocket = NULL;
    }

    m_pUdpSocket = new QUdpSocket();

    connect( m_pUdpSocket, SIGNAL( readyRead()), this, SLOT( solt_readPendingDatagrams()));
    m_pUdpSocket->bind( p_nPort );
}

void CUdpManager::solt_readPendingDatagrams()
{
    while (m_pUdpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_pUdpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_pUdpSocket->readDatagram( datagram.data(), datagram.size(), &sender, &senderPort);

        QString sString( datagram.data() );

        emit signal_receivedMessage( sString );
    }
}

void CUdpManager::sendData( const QByteArray & p_grData, const QString & p_sHostAdress, const quint16 p_unPort )
{
    QHostAddress grHostAdress( p_sHostAdress );
    QUdpSocket grUdpSocket;

    qDebug() << "# Send hex " << CNetworkSender::printASCII( p_grData ) << " to " << p_sHostAdress << "Port " << p_unPort << Q_FUNC_INFO;
    grUdpSocket.writeDatagram( p_grData, grHostAdress, p_unPort );
}

