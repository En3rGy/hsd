#include "koxml.h"
#include <QDebug>

CKoXmlGC g_grKoXmlGarbageCollector;

CKoXml * CKoXml::m_pInstance = NULL;

CKoXml *CKoXml::getInstance()
{
    if ( m_pInstance == NULL )
    {
        m_pInstance = new CKoXml();
    }

    return m_pInstance;
}

void CKoXml::setXml(QByteArray & p_grKoXml)
{
    int     nIndex = 0;
    cobject grCobject;

    while ( p_grKoXml.indexOf( "<cobject ", nIndex ) != -1 )
    {
        nIndex = p_grKoXml.indexOf( "<cobject ", nIndex ) + 9;

        grCobject.sId       = copy( p_grKoXml, "id", nIndex );
        grCobject.sUsed     = copy( p_grKoXml, "used", nIndex );
        grCobject.sType     = copy( p_grKoXml, "type", nIndex );
        grCobject.sPath     = copy( p_grKoXml, "path", nIndex );
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

        //qDebug() << nIndex << "\t" << grCobject.sGa << "\t" << grCobject.sName;
    }

    qDebug() << "Received " << m_ssGA2NameMap.size() << "elements from HS.";
}

CKoXml::CKoXml()
{
}

QString CKoXml::copy(QByteArray &p_grData, const QString & p_sTag, const int &p_nStartIndex )
{
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
    if ( CKoXml::m_pInstance != NULL )
    {
        delete CKoXml::m_pInstance;
        CKoXml::m_pInstance = NULL;
    }
}

QString CKoXml::getGaName(const QString &p_sGA)
{
    cobject grValue = m_ssGA2NameMap.value( p_sGA );

    if ( grValue.sName.isEmpty() == true )
    {
        return QObject::tr( "Unknown" );
    }

    return grValue.sName;
}
