#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>

class QTcpSocket;
class QTcpServer;
class QSettings;

class CTcpClient : public QObject
{
    Q_OBJECT
public:
    CTcpClient(QObject *parent = 0);
    ~CTcpClient();

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
    void initConnection( const QString & p_sPass = "" );

    void getGaXml( void );

signals:
    /**
     * @brief Signal emitted by incomming message
     * @param p_sGA Updated Group Adress, e.g. 2/2/1
     * @param p_sValue New value, e.g. 0
     */
    void signal_receivedMessage( const QString  & p_sGA, const QString & p_sValue );

public slots:
    void slot_startRead( void );
    void slot_webRequestReadFinished( void );
    void slot_webRequestClosed( void );

private:
    static int     convertGA( const QString & p_sGA );
    static QString convertGA( const int & p_nGA );

    void splitString( const QString & p_sIncoming, QString & p_sType, QString & p_sGA, QString & p_sValue );

    QTcpSocket * m_pTcpSocket;
    QTcpSocket * m_pWebRequestTcpSocket;
    qint16       m_nPort;
    QByteArray   m_grWebRequestData;
    QSettings  * m_pSettings;

    static const QChar   m_sMsgEndChar;
    static const QString m_sSeperatorChar;
};

#endif // TCPCLIENT_H
