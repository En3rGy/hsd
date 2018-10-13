#include "koxml.h"
#include <QDebug>
#include "QsLog.h"

CKoXmlGC g_grKoXmlGarbageCollector;

CKoXml * CKoXml::m_pInstance = nullptr;

CKoXml *CKoXml::getInstance()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_pInstance == nullptr )
    {
        m_pInstance = new CKoXml();
    }

    return m_pInstance;
}

void CKoXml::setXml( const QByteArray & p_grKoXml)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    int     nIndex = 0;
    cobject grCobject;

    while ( p_grKoXml.indexOf( "<cobject ", nIndex ) != -1 )
    {
        nIndex = p_grKoXml.indexOf( "<cobject ", nIndex ) + 9;

        grCobject.sId       = copy( p_grKoXml, "id", nIndex );
        grCobject.sUsed     = copy( p_grKoXml, "used", nIndex );
        grCobject.sType     = copy( p_grKoXml, "type", nIndex );
        grCobject.sPath     = copy( p_grKoXml, "path", nIndex );
        grCobject.sFmt      = copy( p_grKoXml, "fmt", nIndex );
        grCobject.sFmtex    = copy( p_grKoXml, "fmtex", nIndex );
        grCobject.sName     = copy( p_grKoXml, "name", nIndex );
        grCobject.sRem      = copy( p_grKoXml, "rem", nIndex );
        grCobject.sInit     = copy( p_grKoXml, "init", nIndex );
        grCobject.sMin      = copy( p_grKoXml, "min", nIndex );
        grCobject.sMax      = copy( p_grKoXml, "max", nIndex );
        grCobject.sStep     = copy( p_grKoXml, "step", nIndex );
        grCobject.sList     = copy( p_grKoXml, "list", nIndex );
        grCobject.sGa       = copy( p_grKoXml, "ga", nIndex );
        grCobject.sGanum    = copy( p_grKoXml, "ganum", nIndex );
        grCobject.sCogws    = copy( p_grKoXml, "cogws", nIndex );
        grCobject.sCogwr    = copy( p_grKoXml, "cogwr", nIndex );
        grCobject.sScan     = copy( p_grKoXml, "scan", nIndex );
        grCobject.sSbc      = copy( p_grKoXml, "sbc", nIndex );
        grCobject.sRead     = copy( p_grKoXml, "read", nIndex );
        grCobject.sTransmit = copy( p_grKoXml, "transmit", nIndex );

        m_ssGA2NameMap.insert( grCobject.sGa, grCobject );

        QLOG_DEBUG() << grCobject.sGa << "\t" << grCobject.sName << "\t" << grCobject.sFmt;
    }

    QLOG_INFO() << QObject::tr("Received available GAs from HS. No of received elements:").toStdString().c_str() << m_ssGA2NameMap.size();

    if ( m_ssGA2NameMap.size() == 0 ) {
        QLOG_WARN() << QObject::tr( "NO GAs were received. Remeber to transfer your project via Data+Images+Sounds to transfer XML also." );
    }
}

CKoXml::CKoXml()
{
    QLOG_TRACE() << Q_FUNC_INFO;
}

QString CKoXml::copy( const QByteArray &p_grData, const QString & p_sTag, const int &p_nStartIndex )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QString sBeginStr = p_sTag + "=\"";
    int nStartIndex = p_grData.indexOf( sBeginStr, p_nStartIndex ) + sBeginStr.length();

    if ( ( nStartIndex >= p_grData.length() ) || ( p_nStartIndex == -1 ) )
    {
        return QString();
    }

    int nEndIndex = p_grData.indexOf( "\" ", nStartIndex );
    if ( ( nEndIndex >= p_grData.length() ) || ( nEndIndex == -1 ) )
    {
        return QString();
    }

    return p_grData.mid( nStartIndex, nEndIndex - nStartIndex );
}

CKoXmlGC::~CKoXmlGC()
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( CKoXml::m_pInstance != nullptr )
    {
        delete CKoXml::m_pInstance;
        CKoXml::m_pInstance = nullptr;
    }
}

QString CKoXml::getGaName(const QString &p_sGA)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    cobject grValue = m_ssGA2NameMap.value( p_sGA );

    if ( grValue.sName.isEmpty() == true )
    {
        return QObject::tr( "Name of GA is unknown" ).toStdString().c_str();
    }

    return grValue.sName;
}

QString CKoXml::getGaFormat(const QString &p_sGA)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    cobject grValue = m_ssGA2NameMap.value( p_sGA );

    if ( grValue.sFmt.isEmpty() == true )
    {
        return QString( QObject::tr( "Format of " ) + p_sGA + QObject::tr( " is unknown" ) ).toStdString().c_str();
    }

    return grValue.sFmt;
}

CKoXml::enuDPT CKoXml::getGaDPT(const QString &p_sGA)
{
    QString sFmt = getGaFormat( p_sGA );

    if ( sFmt == "EIS6_8BIT" ) {
        return enuDPT_DPT5_001;
    }
    else if ( ( sFmt == "NONEIS_8BIT_RTR" ) ||
              ( sFmt == "EIS5_16BIT" ) ) {
        return enuDPT_DPT9;
    }
    else if ( sFmt == "EIS2+EIS6_8BIT" ) {
        return enuDPT_DPT5_004;
    }
    else if ( ( sFmt == "EIS?_4BIT" ) ||
              ( sFmt == "EIS?_8BIT" ) ||
              ( sFmt == "EIS?_14BYTE" ) ||
              ( sFmt == "EIS?_DALI" ) ||
              ( sFmt == "EIS?_SRO" ) ) {
        return enuDPT_undef;
    }
    else if ( ( sFmt == "EIS10_16BIT_UNSIGNED" ) ||
              ( sFmt == "EIS10_16BIT_SIGNED" ) ) {
        return enuDPT_DPT7_DPT8;
    }
    else if ( ( sFmt == "EIS11_32BIT_UNSIGNED" ) ||
              ( sFmt == "EIS11_32BIT_SIGNED" ) ) {
        return enuDPT_DPT12_DPT13;
    }
    else if ( sFmt == "EIS3_3BYTE_TIME" ) {
        return enuDPT_DPT10;
    }
    else if ( sFmt == "EIS4_3BYTE_DATE" ) {
        return enuDPT_DPT11;
    }
    else if ( sFmt == "EIS9_4BYTE" ) {
        return enuDPT_DPT14;
    }
    else if ( sFmt == "EIS8_2BIT" ) {
        return enuDPT_DPT2;
    }
    else if ( sFmt == "EIS1+EIS2+EIS7_1BIT" ) {
        return enuDPT_DPT1;
    }
    else {
        return enuDPT_undef;
    }
}
