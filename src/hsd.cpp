#include "hsd.h"
#include <QElapsedTimer>
#include "tcpclient.h"
#include "tcpserver.h"
#include "model.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>
#include <QtLogging>
#include <QStandardPaths>

CHsd::CHsd(QObject *parent) :
    QObject(parent)
  , m_pTcpServer( nullptr )
  , m_pTcpClient( nullptr )
{
    m_pTcpServer = new CTcpServer( this );
    m_pTcpClient = new CTcpClient( this );

    connect ( m_pTcpServer,
              SIGNAL(signal_sendToHs(QString,QVariant)),
              m_pTcpClient,
              SLOT( slot_sendToHs(QString,QVariant)));

    connect( m_pTcpClient,
             SIGNAL( signal_receivedMessage(QString,QString)),
             m_pTcpServer,
             SLOT( slot_sendToEibdClient(QString,QString)) );
}

CHsd::~CHsd()
{
    delete m_pTcpServer;
    delete m_pTcpClient;
}

void CHsd::setLogLevel(const uint &p_unLogLevel)
{
    Q_UNUSED(p_unLogLevel)
}

void CHsd::startService()
{
    qDebug() << QCoreApplication::applicationName() << ": " << tr( "Log level is:" );
    qInfo() << tr( "Log level is:" );

    m_pTcpServer->listen();

    QElapsedTimer grInterval;
    grInterval.start();
    bool  bRes = false;
    while ( bRes == false ) {
        QCoreApplication::processEvents();
        if ( grInterval.elapsed() > 1000 ) {
            bRes = m_pTcpClient->getGaXml();
            grInterval.restart();
        }
    }

    m_pTcpClient->initConnection();
}

void CHsd::callHsXML() const
{
    m_pTcpClient->getGaXml();
}

void CHsd::stopService()
{
    qDebug() << QCoreApplication::applicationName() << ": " << tr( "Sending STOP signal." );
    qInfo() << tr( "Sending STOP signal." );

    QString sDestAddr = "127.0.0.1";
    int     nPort     = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort ).toInt();
    QString sData     = CModel::g_sExitMessage;
    QByteArray grData;
    grData.append( sData.toLatin1() );

    CTcpClient::sendData( sDestAddr, nPort, grData );
}

void CHsd::setRemoteLogLevel(const int &p_nLogLevel)
{
    qDebug() << QCoreApplication::applicationName() << ": " << tr( "Sending remote log level" ) << p_nLogLevel;
    qInfo() << tr( "Sending remote log level" ) << p_nLogLevel;

    QString sDestAddr = "127.0.0.1";
    int     nPort     = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort ).toInt();
    QString sData     = CModel::g_sLogLevelMessage;
    QByteArray grData;
    grData.append(sData.toLatin1());
    grData.append(static_cast< char > ( p_nLogLevel ));

    CTcpClient::sendData(sDestAddr, nPort, grData);
}
