#ifndef HSD_H
#define HSD_H

#include <QObject>

class CTcpServer;
class CTcpClient;

/** @class CHsd
  * @brief Programm controller
  * @author T. Paul
  * @date 2013
  */
class CHsd : public QObject
{
    Q_OBJECT
public:
    explicit CHsd(QObject *parent = 0);
    virtual ~CHsd( void );

    void setLogLevel( const uint & p_unLogLevel );

    /** @brief Starts the programm components
      *
      * Connecting to home server, calling group adresses from homeserver,
      * starting listening for eibd messages.
      */
    void startService( void );

    void callHsXML(  ) const;

    static void stopService( void );
    static void setRemoteLogLevel( const int & p_nLogLevel );

signals:
    
public slots:

private:
    CTcpServer * m_pTcpServer;
    CTcpClient * m_pTcpClient; 
};

#endif // HSD_H
