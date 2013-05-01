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

    /**
     * @brief Send a message to the KO-Gateway
     * @param p_sAction Type of message
     * @param p_sGA Group Adress, e.g. 2/2/15
     * @param p_sValue Value to set, e.g. 1
     */
    void send( const QString & p_sAction, const QString  & p_sGA, const QString & p_sValue );

    /**
     * @brief Initialization of connection to HS
     * @param p_sHostAddress Host address of Homeserver, e.g. 192.168.1.2
     * @param p_unPort Port of KO-Gateway on HS, usually 7003
     * @param p_sPass Password for KO-Gateway, defined in Experte software
     */
    void initConnection( const QString &p_sHostAddress, const quint16 &p_unPort, const QString & p_sPass = "" );

signals:
    /**
     * @brief Signal emitted by incomming message
     * @param p_sGA Updated Group Adress, e.g. 2/2/1
     * @param p_sValue New value, e.g. 0
     */
    void signal_receivedMessage( const QString  & p_sGA, const QString & p_sValue );

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
