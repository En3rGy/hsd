#include "hsd.h"
#include "tcpclient.h"
#include "tcpserver.h"

CHsd::CHsd(QObject *parent) :
    QObject(parent)
  , m_pTcpServer( NULL )
  , m_pTcpClient( NULL )
{
    m_pTcpServer = new CTcpServer( this );
    m_pTcpClient = new CTcpClient( this );


    connect ( m_pTcpServer,
              SIGNAL(signal_setEibAdress(QString,int)),
              m_pTcpClient,
              SLOT( slot_setEibAdress(QString,int)));

    connect( m_pTcpClient,
             SIGNAL( signal_receivedMessage(QString,QString)),
             m_pTcpServer,
             SLOT( slot_groupWrite(QString,QString)) );
}

CHsd::~CHsd()
{
    delete m_pTcpServer;
    delete m_pTcpClient;
}

void CHsd::startService()
{
    m_pTcpClient->getGaXml();
    m_pTcpServer->listen();
    m_pTcpClient->initConnection();
}
