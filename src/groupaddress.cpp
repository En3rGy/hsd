#include <QStringList>
#include <QDebug>
#include "groupaddress.h"
#include "QsLog.h"
#include "koxml.h"

CGroupAddress::CGroupAddress()
    : m_unMainAddr( 0 )
    , m_unMiddAddr( 0 )
    , m_unLowAddr( 0 )
{
    QLOG_TRACE() << Q_FUNC_INFO;
}

QByteArray CGroupAddress::toHex() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    // eib 2 hex: The most significant Bit is always zero, followed by 4 bits for the
    // maingroup, 3 bits for the middlegroup and 8 bits for the subgrou
    // 0hhh hmmm
    QByteArray grRetArray;

    uchar szHexAddr [2];

    uchar szHAddr = m_unMainAddr; // grAddrList.at( 0 ).toUInt();
    uchar szMAddr = m_unMiddAddr; //grAddrList.at( 1 ).toUInt();
    uchar szUAddr = m_unLowAddr;  //grAddrList.at( 2 ).toUInt();

    // 0000 0000 == 0x00
    // 1111 1111 == 0xff

    szHAddr = szHAddr << 4;
    szHexAddr[ 0 ] = szHAddr | szMAddr;
    szHexAddr[ 1 ] = szUAddr;

    grRetArray.append( (char * ) & szHexAddr, sizeof( szHexAddr ) );

    return grRetArray;
}

QString CGroupAddress::toKNXString(const QString &p_sSeparator) const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QString sRetStr;

    sRetStr = QString::number( m_unMainAddr )
            + p_sSeparator
            + QString::number( m_unMiddAddr )
            + p_sSeparator
            + QString::number( m_unLowAddr );

    return sRetStr;
}

int CGroupAddress::toHSRepresentation() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    int nConvert = m_unMainAddr * 2048 + m_unMiddAddr * 256 + m_unLowAddr;
    return nConvert;
}

void CGroupAddress::setHex(const QByteArray & p_grHexAddr)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    // eib 2 hex: The most significant Bit is always zero, followed by 4 bits for the
    // maingroup, 3 bits for the middlegroup and 8 bits for the subgrou

    // 0hhh hmmm

    if ( p_grHexAddr.length() != 2 )
    {
        QLOG_ERROR() << QObject::tr( "Wrong length of hex address." ).toStdString().c_str();
        return;
    }

    uchar szData [ 3 ];
    szData[0] = (uchar) p_grHexAddr.at(0);
    szData[1] = (uchar) p_grHexAddr.at(0);
    szData[2] = (uchar) p_grHexAddr.at(1);

    szData[0] = szData[0] << 1;

    QString sMainAddr   = QString::number( ( szData[0] >> 4 ) & 0xf );
    QString sMiddleAddr = QString::number( ( szData[1] & 0x7 ) );
    QString sUnderAddr  = QString::number( szData[2] );

    m_unMainAddr = sMainAddr.toUInt();
    m_unMiddAddr = sMiddleAddr.toUInt();
    m_unLowAddr  = sUnderAddr.toUInt();
}

void CGroupAddress::setPlainHex(const QByteArray &p_grHexAddr)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    if ( p_grHexAddr.length() != 2 )
    {
        QLOG_ERROR() << QObject::tr( "Wrong length of hex address." ).toStdString().c_str();
        return;
    }

    // given hhhh mmmm ssss ssss

    uchar szData [ 3 ];
    szData[0] = (uchar) p_grHexAddr.at(0);
    szData[1] = (uchar) p_grHexAddr.at(0);
    szData[2] = (uchar) p_grHexAddr.at(1);

    //szData[0] = szData[0] << 1;

    QString sMainAddr   = QString::number( ( szData[0] >> 4 ) & 0xf );
    QString sMiddleAddr = QString::number( ( szData[1] & 0x8 ) );
    QString sUnderAddr  = QString::number( szData[2] );

    m_unMainAddr = sMainAddr.toUInt();
    m_unMiddAddr = sMiddleAddr.toUInt();
    m_unLowAddr  = sUnderAddr.toUInt();
}

void CGroupAddress::setKNXString(const QString &p_sStrAddr)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QString     sSep;
    QStringList grAddrList;

    if ( p_sStrAddr.contains( "/" ) )
    {
        sSep = "/";
    }
    else if ( p_sStrAddr.contains( "." ) )
    {
        sSep = ".";
    }
    else
    {
        return;
    }

    grAddrList = p_sStrAddr.split( sSep, QString::SkipEmptyParts );

    if ( grAddrList.size() != 3 )
    {
        QLOG_ERROR() << p_sStrAddr << QObject::tr("not of kind a/b/c. Aborting.").toStdString().c_str();
        return;
    }

    m_unMainAddr = grAddrList.at( 0 ).toUInt();
    m_unMiddAddr = grAddrList.at( 1 ).toUInt();
    m_unLowAddr  = grAddrList.at( 2 ).toUInt();
}

void CGroupAddress::setHS(const int & p_nHSAddr)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    // int nConvert = nX * 2048 + nY * 256 + nZ;

    m_unMainAddr = p_nHSAddr / 2049; // 2049 instead of 2048 is correct (but why?)
    m_unMiddAddr = ( p_nHSAddr - ( m_unMainAddr * 2048 ) ) / 256;
    m_unLowAddr  = ( p_nHSAddr - ( m_unMainAddr * 2048 ) - ( m_unMiddAddr * 256 ) );
}

void CGroupAddress::setAddress(const QString &p_sAddress)
{
    if ( p_sAddress.contains( "/" ) ) // EIB/KNX representation
    {
        qDebug() << QObject::tr( "Input is EIB/KNX representation" ).toStdString().c_str();

        setKNXString( p_sAddress );
    }
    else if ( p_sAddress.length() == 4 ) // HEX representation
    {
        qDebug() << QObject::tr( "Input is HEX representation" ).toStdString().c_str();

        QByteArray grHexAddr;

        grHexAddr.append( p_sAddress.mid( 0, 2 ).toShort( NULL, 16 ) );
        grHexAddr.append( p_sAddress.mid( 2, 2 ).toShort( NULL, 16 ) );
        setPlainHex( grHexAddr );
    }
    else // HS representation
    {
        qDebug() << QObject::tr( "Input is HS representation" ).toStdString().c_str();

        setHS( p_sAddress.toInt() );
    }
}

bool CGroupAddress::isValid() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( m_unMainAddr > 15 )
    {
        return false;
    }
    if ( m_unMiddAddr > 15 )
    {
        return false;
    }
    if ( m_unLowAddr > 256 )
    {
        return false;
    }

    return true;
}

QString CGroupAddress::toString() const
{
    QString sRet = "KNX: " + toKNXString()
            + ", HEX: " + toHex().toHex()
            + ", HS: " + QString::number( toHSRepresentation() );
           // + ", " + CKoXml::getInstance()->getGaName( toKNXString( ));

    return sRet;
}


