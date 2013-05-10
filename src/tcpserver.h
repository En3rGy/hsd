#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QString>

class QTcpSocket;
class QTcpServer;
class QSettings;

/** @class CTcpServer
  * @brief Server to provide the eibd TCP/IP interface.
  * @author T. Paul
  * @date 2013
  */
class CTcpServer : public QObject
{
    Q_OBJECT
public:
    CTcpServer(QObject *parent = 0);
    ~CTcpServer();

    /** @brief Starts listeing on the passed port.
      * @param p_nPort Port to listen on, e.g. 6027
      */
    void listen( const uint & p_nPort );

    /** @brief Starts listening on the port given in the config file, 6720 by default.
      */
    void listen( void );

signals:
    /** @brief Signal to force setting an value via GIRA Home server
      * @param p_sEibAddr KNX/EIB address to set, e.g. 3/6/39
      * @param p_nVal Value to set.
      */
    void signal_setEibAdress( const QString & p_sEibAddr, const int & p_nVal );

public slots:
    void solt_newConnection( void );
    void slot_startRead( void );
    void slot_disconnected( void );

    /** @brief Slot to be called when data should be send via the eibd interface
      * @param p_sEibGroup Eib address to be set, e.g. 11/1/100
      * @param p_sValue Value to be set.
      */
    void slot_groupWrite( const QString & p_sEibGroup, const QString & p_sValue );

private:
    QTcpSocket * m_pTcpSocket;
    QTcpServer * m_pTcpServer;
    uint         m_nPort;
    QSettings  * m_pSettings;
};

#endif // TCPSERVER_H
