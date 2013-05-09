#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QString>

class QTcpSocket;
class QTcpServer;
class QSettings;

class CTcpServer : public QObject
{
    Q_OBJECT
public:
    CTcpServer(QObject *parent = 0);
    ~CTcpServer();

    void listen( const uint p_nPort );
    void listen( void );

signals:
    void signal_setEibAdress( const QString & p_sEibAddr, const int & p_nVal );

public slots:
    void solt_newConnection( void );
    void slot_startRead( void );
    void slot_disconnected( void );
    void slot_groupWrite( const QString & p_sEibGroup, const QString & p_sValue );

private:
    QTcpSocket * m_pTcpSocket;
    QTcpServer * m_pTcpServer;
    qint16       m_nPort;
    QSettings  * m_pSettings;
};

#endif // TCPSERVER_H
