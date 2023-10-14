#include "model.h"
#include <QtLogging>
#include <QtDebug>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

const QString CModel::g_sKey_HSIP         = "HSIP";
const QString CModel::g_sKey_HSWebPort    = "HSWebPort";
const QString CModel::g_sKey_HSGwPort     = "HSGwPort";
const QString CModel::g_sKey_HsdPort      = "HsdPort";
const QString CModel::g_sKey_LogLevel     = "LogLevel";
const QString CModel::g_sKey_HSGwPassword = "HSGwPass";
const QString CModel::g_sKey_LogPerDate   = "LogPerDate";
const QString CModel::g_sExitMessage      = "Shutdown hsd please";
const QString CModel::g_sLogLevelMessage  = "Set hsd log level please";
const QString CModel::g_sKey_PauseTilHSReconnect = "TimeoutForReconnectHS_ms";

const char   CModel::g_uzEIB_OPEN_T_GROUP [2]  = { 0x00, 0x22 };
const char   CModel::g_uzEIB_APDU_PACKET [2]   = { 0x00, 0x25 };
const char   CModel::g_uzEIB_OPEN_GROUPCON [5] = { 0x00, 0x26 }; // { 0x00, 0x26, 0x00, 0x00,0x00 };
const char   CModel::g_uzEIB_GROUP_PACKET [2]  = { 0x00, 0x27 };

const char   CModel::g_uzEibOn                 = 0x81;
const char   CModel::g_uzEibOff                = 0x80;
const char   CModel::g_uzEibAck [2]            = { 0x00, 0x05 };

QString CModel::g_sSettingsPath     = "../etc/hsd.ini";
QString CModel::g_sLogFilePath      = "../var/hsd.log";

CModelGC g_grCModelGarbageCollector;

CModel * CModel::m_pInstance = nullptr;

CModel::CModel()
{
    QDir grSettingsPath= QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString sSettingsPath = grSettingsPath.absoluteFilePath("hsd.ini");
    if (! grSettingsPath.exists()) {
        grSettingsPath.mkpath(grSettingsPath.absolutePath());
    }

    m_pSettings = new QSettings( sSettingsPath, QSettings::IniFormat );

    QVariant grLogLevel = getValue( g_sKey_LogLevel, uint( 2 ) );
    QDir grLogPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    if ( grLogPath.exists() == false ) {
        grLogPath.mkdir( grLogPath.absolutePath() );
    }

    QString sFileName = "hsd.log";
    if ( getValue( CModel::g_sKey_LogPerDate, false ).toBool() ) {
        sFileName = "hsd_" + QDateTime::currentDateTime().toString( "yyyy-MM-dd_hhmmss" /*Qt::ISODate*/ ) + ".log";
    }
    sFileName.replace( ":", "" );

    CModel::g_sLogFilePath = grLogPath.absoluteFilePath(sFileName);

    qInfo() << QObject::tr( "Settings file used:" ) << m_pSettings->fileName();
    qInfo() << QObject::tr( "Writing Logfile to:" ) << CModel::g_sLogFilePath << " with log level " << grLogLevel;
}

CModel::CModel(const CModel &p_grModel)
{
    m_pSettings = p_grModel.m_pSettings;
}

CModel *CModel::getInstance()
{
    if ( m_pInstance == NULL )
        m_pInstance = new CModel();

    return m_pInstance;
}

CModel::~CModel()
{
    m_pSettings->sync();
    delete m_pSettings;
}

QVariant CModel::getValue(const QString &p_sKey, const QVariant &p_grDefaultValue)
{
    if ( m_pSettings->contains( p_sKey ) == false )
    {
        m_pSettings->setValue( p_sKey, p_grDefaultValue );
        qWarning() << QObject::tr( "Requested settings key not found in config file. Using default value." )
                   << QObject::tr( "Key was:" ) << p_sKey
                   << QObject::tr( "Default value is:" ) << p_grDefaultValue;
        return p_grDefaultValue;
    }
    else
    {
        return m_pSettings->value( p_sKey );
    }
}

void CModel::setValue(const QString &p_sKey, const QVariant &p_grValue)
{
    m_pSettings->setValue( p_sKey, p_grValue );
}

void CModel::logCSV(const QString &p_sTo, const QString &p_sFrom, const QString &p_sGA, const QString & p_sVal, const QString &p_sMsg, const QString &p_sRawMsg)
{
    QString sSep = " ; ";
    qDebug() << sSep
             << p_sTo << sSep
             << p_sFrom << sSep
             << p_sGA << sSep
             << p_sVal << sSep
             << p_sMsg << sSep
             << p_sRawMsg;
}


CModelGC::~CModelGC()
{
    if ( CModel::m_pInstance != NULL )
    {
        delete CModel::m_pInstance;
        CModel::m_pInstance = NULL;
    }
}
