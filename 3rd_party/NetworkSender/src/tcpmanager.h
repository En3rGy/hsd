#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include <QObject>
#include <QString>

class QTcpSocket;
class QTcpServer;

class CTcpManager : public QObject
{
    Q_OBJECT
public:
    CTcpManager(QObject *parent = 0);
    ~CTcpManager();

    void sendData(const QByteArray &p_grData, const QString &p_sHostAdress = "192.168.143.11", const quint16 p_unPort = 1000 );
    void listen( const uint p_nPort );

signals:
    void signal_receivedMessage( QString );

public slots:
    void solt_newConnection( void );
    void slot_startRead( void );

private:
    QTcpSocket * m_pTcpSocket;
    QTcpServer * m_pTcpServer;
    qint16       m_nPort;
};

#endif // UDPMANAGER_H
