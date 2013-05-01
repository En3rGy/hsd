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

    void send( const QString & p_sAction, const QString  & p_sGA, const QString & p_sValue );

    void initConnection( const QString &p_sHostAddress, const quint16 &p_unPort );

signals:
    void signal_receivedMessage( const QString  & p_sGA, const QString & p_sValueg );

public slots:
    void slot_startRead( void );

private:
    static int     convertGA( const QString & p_sGA );
    static QString convertGA( const int & p_nGA );

    void splitString( const QString & p_sIncoming, QString & p_sType, QString & p_sGA, QString & p_sValue );

    QTcpSocket * m_pTcpSocket;
    QTcpServer * m_pTcpServer;
    qint16       m_nPort;

    static const QChar   m_sMsgEndChar;
    static const QString m_sSeperatorChar;
};

#endif // UDPMANAGER_H
