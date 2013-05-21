#include "hsd.h"
#include "tcpclient.h"
#include "tcpserver.h"
#include "model.h"
#include <QSettings>
#include "QsLog.h"
#include <QsLogDest.h>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QDateTime>

CHsd::CHsd(QObject *parent) :
    QObject(parent)
  , m_pTcpServer( NULL )
  , m_pTcpClient( NULL )
{
    m_pTcpServer = new CTcpServer( this );
    m_pTcpClient = new CTcpClient( this );

    QVariant grLogLevel = CModel::getInstance()->getValue( CModel::g_sKey_LogLevel, uint( 2 ) );
    uint unLogLevel = grLogLevel.toUInt();

    //QDir grLogPath = QDir::currentPath() + "/../var";
    QDir grLogPath = QCoreApplication::applicationDirPath() + "/../var";

    if ( grLogPath.exists() == false )
    {
        grLogPath.mkdir( grLogPath.absolutePath() );
    }

    QString sFileName = "hsd_" + QDateTime::currentDateTime().toString( "yyyy-MM-dd_hhmmss" /*Qt::ISODate*/ ) + ".log";
    sFileName.replace( ":", "" );

    QsLogging::Logger & grLogger = QsLogging::Logger::instance();

    m_pFileDestPtr = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeFileDestination(
                        grLogPath.absoluteFilePath( sFileName ) ) );

    m_pDebugDestPtr = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeDebugOutputDestination() );

    grLogger.addDestination( m_pFileDestPtr.data() );
    grLogger.addDestination( m_pDebugDestPtr.data() );
    grLogger.setLoggingLevel( ( QsLogging::Level ) unLogLevel );

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

void CHsd::setLogLevel(const uint &p_unLogLevel)
{
    QsLogging::Logger & grLogger = QsLogging::Logger::instance();
    grLogger.setLoggingLevel( ( QsLogging::Level ) p_unLogLevel );
}

void CHsd::startService()
{
    m_pTcpClient->getGaXml();
    m_pTcpServer->listen();
    m_pTcpClient->initConnection();
}

void CHsd::callHsXML() const
{
    m_pTcpClient->getGaXml();
}

void CHsd::stopService()
{
    QLOG_INFO() << "Sending STOP signal.";
    QString sDestAddr = "127.0.0.1";
    int     nPort     = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort ).toInt();
    QString sData     = CModel::g_sExitMessage;
    QByteArray grData;
    grData.append( sData );
    m_pTcpClient->sendData( sDestAddr, nPort, grData );
}
