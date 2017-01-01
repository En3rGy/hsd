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

    m_pFileDestPtr  = QsLogging::DestinationFactory::MakeFileDestination( grLogPath.absoluteFilePath( sFileName ) );
    m_pDebugDestPtr = QsLogging::DestinationFactory::MakeDebugOutputDestination();

    grLogger.addDestination( m_pFileDestPtr );
    grLogger.addDestination( m_pDebugDestPtr );
    grLogger.setLoggingLevel( ( QsLogging::Level ) unLogLevel );

    qDebug() << QCoreApplication::applicationName().toStdString().c_str() << ": " << tr( "Writing Logfile to:" ).toStdString().c_str() << grLogPath.absoluteFilePath( sFileName );
    QLOG_INFO() << tr( "Writing Logfile to:" ).toStdString().c_str() << grLogPath.absoluteFilePath( sFileName );

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
    QsLogging::Logger & grLogger = QsLogging::Logger::instance();
    grLogger.setLoggingLevel( ( QsLogging::Level ) p_unLogLevel );
}

void CHsd::startService()
{
    qDebug() << QCoreApplication::applicationName().toStdString().c_str() << ": " << tr( "Log level is:" ).toStdString().c_str() << QsLogging::Logger::instance().loggingLevel();
    QLOG_INFO() << tr( "Log level is:" ).toStdString().c_str() << QsLogging::Logger::instance().loggingLevel();

    while ( m_pTcpClient->getGaXml() == false ) {
        QCoreApplication::processEvents();
    }
    m_pTcpServer->listen();
    m_pTcpClient->initConnection();
}

void CHsd::callHsXML() const
{
    m_pTcpClient->getGaXml();
}

void CHsd::stopService()
{
    qDebug() << QCoreApplication::applicationName().toStdString().c_str() << ": " << tr( "Sending STOP signal." ).toStdString().c_str();
    QLOG_INFO() << tr( "Sending STOP signal." ).toStdString().c_str();

    QString sDestAddr = "127.0.0.1";
    int     nPort     = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort ).toInt();
    QString sData     = CModel::g_sExitMessage;
    QByteArray grData;
    grData.append( sData );

    CTcpClient::sendData( sDestAddr, nPort, grData );
}

void CHsd::setRemoteLogLevel(const int &p_nLogLevel)
{
    qDebug() << QCoreApplication::applicationName().toStdString().c_str() << ": " << tr( "Sending remote log level" ).toStdString().c_str() << p_nLogLevel;
    QLOG_INFO() << tr( "Sending remote log level" ).toStdString().c_str() << p_nLogLevel;

    QString sDestAddr = "127.0.0.1";
    int     nPort     = CModel::getInstance()->getValue( CModel::g_sKey_HsdPort ).toInt();
    QString sData     = CModel::g_sLogLevelMessage;
    QByteArray grData;
    grData.append( sData );
    grData.append( p_nLogLevel );

    CTcpClient::sendData( sDestAddr, nPort, grData );
}
