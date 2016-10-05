#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QString>

class QUdpSocket;

class CUdpManager : public QObject
{
    Q_OBJECT
public:
    explicit CUdpManager(QObject *parent = 0);

    void sendData(const QByteArray &p_grData , const QString &p_sHostAdress = "192.168.143.11", const quint16 p_unPort = 2001 );

    void listen( const uint p_nPort );

signals:
    void signal_receivedMessage( QString );

public slots:
    void solt_readPendingDatagrams( void );

private:
    QUdpSocket * m_pUdpSocket;
    qint16       m_nPort;
};

#endif // UDPMANAGER_H
