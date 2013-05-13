#include "model.h"
#include <QsLog.h>

const QString CModel::g_sKey_HSIP         = "HSIP";
const QString CModel::g_sKey_HSWebPort    = "HSWebPort";
const QString CModel::g_sKey_HSGwPort     = "HSGwPort";
const QString CModel::g_sKey_HsdPort      = "HsdPort";
const QString CModel::g_sKey_LogLevel     = "LogLevel";
const QString CModel::g_sKey_HSGwPassword = "HSGwPass";
const QString CModel::g_sSettingsPath     = "../etc/hsd.ini";

const uchar   CModel::g_uzEibGroupPacket [2]  = { 0x00, 0x27 };
const uchar   CModel::g_uzEibOpenGroupCon [5] = { 0x00, 0x26, 0x00, 0x00,0x00 };
const uchar   CModel::g_uzEibOn               = 0x81;
const uchar   CModel::g_uzEibOff              = 0x80;
const uchar   CModel::g_uzEibAck [2]          = { 0x00, 0x05 };

CModelGC g_grCModelGarbageCollector;

CModel * CModel::m_pInstance = NULL;

CModel::CModel()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pSettings = new QSettings( CModel::g_sSettingsPath, QSettings::IniFormat );
}

CModel *CModel::getInstance()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_pInstance == NULL )
    {
        m_pInstance = new CModel();
    }

    return m_pInstance;
}

CModel::~CModel()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pSettings->sync();
    delete m_pSettings;
}

QVariant CModel::getValue(const QString &p_sKey, const QVariant &p_grDefaultValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    if ( m_pSettings->contains( p_sKey ) == false )
    {
        m_pSettings->setValue( p_sKey, p_grDefaultValue );
        return p_grDefaultValue;
    }
    else
    {
        return m_pSettings->value( p_sKey );
    }
}

void CModel::setValue(const QString &p_sKey, const QVariant &p_grValue)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    m_pSettings->setValue( p_sKey, p_grValue );
}


CModelGC::~CModelGC()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( CModel::m_pInstance != NULL )
    {
        delete CModel::m_pInstance;
        CModel::m_pInstance = NULL;
    }
}
