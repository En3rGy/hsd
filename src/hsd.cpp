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


    QObject::connect ( m_pTcpServer,
                       SIGNAL(signal_setEibAdress(QString,int)),
                       m_pTcpClient,
                       SLOT( slot_setEibAdress(QString,int)));
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
