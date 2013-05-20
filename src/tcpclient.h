#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>

class QTcpSocket;
class QTcpServer;
class QSettings;

/** @class CTcpClient
  * @brief Class for managing communication with GIRA homeserver.
  * @author T. Paul
  * @date 2013
  */

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
    void initConnection( void );

    /** @brief Calls the group adress via a HS xml interface.
      */
    void getGaXml( void );

    /**
     * @brief Sends a message to a specific host
     * @param p_sDestAddr Destination address, e.g. www.url.com
     * @param p_nPort Destination port, e.g. 80
     * @param p_grData Data to be transmitted.
     */
    static void sendData( const QString & p_sDestAddr, const int & p_nPort, const QByteArray & p_grData );

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

    /** @brief Send a write action to GIRA Home Server
      * @param p_sEibAddr KNX/EIB address to write, e.g. 1/5/30
      * @param p_nVal Value to set
      */
    void slot_setEibAdress( const QString & p_sEibAddr, const int & p_nVal );

private:
    /** @brief Splits the incomming string from HS to sub data.
      * @param[in] p_sIncoming Incoming HS string.
      * @param[out] p_sType Type of message
      * @param[out] p_sGA Group adress
      * @param[out] p_sValue value of GA.
      */
    void splitString( const QString & p_sIncoming, QString & p_sType, QString & p_sGA, QString & p_sValue );

    QTcpSocket * m_pTcpSocket;
    QTcpSocket * m_pWebRequestTcpSocket;
    qint16       m_nPort;
    QByteArray   m_grWebRequestData;

    static const QChar   m_sMsgEndChar;
    static const QString m_sSeperatorChar;
};

#endif // TCPCLIENT_H
