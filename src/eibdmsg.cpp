#include <QDebug>

#include "eibdmsg.h"
#include "model.h"
#include <QStringList>
#include "groupaddress.h"
#include "QsLog.h"
#include "koxml.h"
#include <qmath.h>

CEibdMsg::CEibdMsg()
    : m_eMsgType( enuMsgType_undef )
    , m_nMsgSize( -1 )
{
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

CEibdMsg::CEibdMsg(const QByteArray & p_grByteArray)
    : m_eMsgType( enuMsgType_undef )
    , m_nMsgSize( -1 )
{
    setEibdMsg( p_grByteArray );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setEibdMsg(const QByteArray &p_grByteArray)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    QByteArray grMsg;

    QLOG_DEBUG() << QObject::tr("Received message: ").toStdString().c_str() << printASCII( p_grByteArray ).toStdString().c_str();

    // check if 1st 2 byte contain package length
    if ( p_grByteArray.size() < 2 ) {
        QLOG_WARN() << QObject::tr("Received message too short. Message was").toStdString().c_str() << printASCII( p_grByteArray ).toStdString().c_str();
        return;
    }

    if ( p_grByteArray.at( 1 ) < char( 0x20 ) ) { // size info
        m_eMsgType    = enuMsgType_msgSize;
        QString sSize = QString::number( p_grByteArray.at( 1 ) );
        m_nMsgSize    = ( int ) sSize.toDouble();
        QLOG_DEBUG() << QObject::tr("Msg interpreted as size info. Awaiting message with size").toStdString().c_str() << m_nMsgSize;

        if ( p_grByteArray.size() == 2 ) {
            return;
        }
    }

    if ( m_nMsgSize != -1 ) {
        grMsg.append( p_grByteArray.mid( 2, m_nMsgSize ) ); // removing size info

        if ( grMsg.size() > m_nMsgSize ) {
            QLOG_WARN() << QObject::tr("Message longer than indicated; truncating. Msg was:").toStdString().c_str() << printASCII( p_grByteArray ).toStdString().c_str();
        }
    }
    else {
        m_nMsgSize = p_grByteArray.size();
        grMsg.append( p_grByteArray );
    }

    // check different msg types
    QByteArray grMsgType = grMsg.mid( 0, 2 );

    if ( equals( grMsgType, CModel::g_uzEibAck, 2 ) ) {
        m_eMsgType = enuMsgType_connect;
    }
    else if ( ( grMsg.length() == 2 ) && ( equals( grMsgType, CModel::g_uzEibAck, 2 )) == false ) {
        m_eMsgType = enuMsgType_msgSize;
        m_nMsgSize = ( int ) QString::number( p_grByteArray.at( 1 ) ).toDouble();
    }
    else if ( equals( grMsgType, CModel::g_uzEIB_APDU_PACKET, 2 ) ) {
        m_eMsgType = enuMsgType_EIB_APDU_PACKET;

        // paylaod is within byte 2, 3
        // Check 1st payload byte
        uchar szPayload0 = grMsg.data()[2] & 0x03; // 0000 0011
        uchar szPayload1 = grMsg.data()[3] & 0xC0; // 1100 0000

        if ( szPayload0 == 0x00 ) {
            // Check 2nd payload byte
            if ( szPayload1 == 0x00 ) {
                m_eAPDUType = enuAPDUType_A_GroupValue_Read_PDU;
            }
            else if ( szPayload1 == 0x40 ) {
                // A_GroupValue_Response_PDU
                m_eAPDUType = enuAPDUType_undef;
            }
            else if ( szPayload1 == 0x80 ) {
                setEib1( grMsg.data()[3] );
                m_eAPDUType = enuAPDUType_A_GroupValue_Write_PDU;
            }
            else if ( szPayload1 == 0xC0 ) {
                // A_IndividualAddress_Write_PDU
                m_eAPDUType = enuAPDUType_undef;
            }
        }
        else if ( szPayload0 == 0xC0 ) {
            m_eAPDUType = enuAPDUType_undef;
        }
        else {
            m_eAPDUType = enuAPDUType_undef;
        }
    }
    else if ( equals( grMsgType, CModel::g_uzEIB_OPEN_GROUPCON, 2 ) ) {
        m_eMsgType = enuMsgType_EIB_OPEN_GROUPCON;
        setEibAddress( grMsg.mid( 2, 2 ) );
    }
    else if ( equals( grMsgType, CModel::g_uzEIB_OPEN_T_GROUP, 2 ) ) {
        m_eMsgType = enuMsgType_EIB_OPEN_T_GROUP;
        setEibAddress( grMsg.mid( 2, 2 ) );
    }
    else if ( equals( grMsgType, CModel::g_uzEIB_GROUP_PACKET, 2 ) ) {
        m_eMsgType = enuMsgType_EIB_GROUP_PACKET;
        setEibAddress( grMsg.mid( 2, 2 ) );

        // Process data
        QByteArray grData;
        grData.append( grMsg.mid( 5, grMsg.size() - 5 ) ); // skipping byte 4 which is 00

        // e.g. EIS1 = 1 Bit
        if ( grData.size() == 1 ) {
            setEib1( grMsg.at( 5 ) );
        }

        //
        else if ( grData.size() == 2 ) {
            //    ERROR 2016-12-31T14:49:55.283 Unknown DTP of data bytes in EIB message: "00 27 13 0c 00 80 cc"
            //    ERROR 2016-12-31T16:29:33.342 Unknown DTP of data bytes in EIB message: "00 27 1a 10 00 80 00"
            //    ERROR 2016-12-31T20:58:45.778 Unknown DTP of data bytes in EIB message: "00 27 12 02 00 81 00 06 00 27 12 03 00 81"
            //    ERROR 2016-12-31T20:58:55.018 Unknown DTP of data bytes in EIB message: "00 27 12 02 00 81 00 06 00 27 12 03 00 81"
            //    ERROR 2016-12-31T23:11:50.557 Unknown DTP of data bytes in EIB message: "00 27 1a 10 00 80 ff"
            //    ERROR 2016-12-31T23:11:51.847 Unknown DTP of data bytes in EIB message: "00 27 1a 10 00 80 00"

            setDTP5( grData.mid( 1, 1 ) ); // skipping 80 byte
        }

        // F_16 = DPT 9.001 resp. DPT_Value_Temp resp. 2-octet float value
        else if ( grData.size() == 3 ) {
            setDTP9_001( grData.mid( 1, 2 ) ); // skipping 1st byte
        }
        else {
            QLOG_ERROR() << QObject::tr( "Unknown DTP of data bytes in EIB message:" ).toStdString().c_str() << printASCII( grMsg );
        }
    } // else if ( grMsgType == QByteArray( * CModel::g_uzEIB_GROUP_PACKET, 2 ) )
    else {
        QLOG_WARN() << QObject::tr("Received unknown message").toStdString().c_str() << printASCII( grMsg );
    }

    m_nMsgSize = -1; // reset info
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const CEibdMsg::enuMsgType &CEibdMsg::getType() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    return m_eMsgType;
}

const CEibdMsg::enuAPDUType &CEibdMsg::getAPDUType() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    return m_eAPDUType;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const QString &CEibdMsg::getDestAddressKnx() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    return m_sDstAddrKnx;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const QVariant & CEibdMsg::getValue( bool * p_pHasValue ) const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( p_pHasValue != NULL )
    {
        * p_pHasValue = ! m_grValue.isNull();
    }
    return m_grValue;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

QByteArray CEibdMsg::getResponse( bool * p_pHasResponse )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QByteArray grResponse;

    bool bHasResponde = false;
    bool bPrependLengthInfo = true;

    switch( m_eMsgType ) {
    case enuMsgType_connect: {
        grResponse.append( CModel::g_uzEibAck[0] );
        grResponse.append( CModel::g_uzEibAck[1] );
        bHasResponde = true;
        bPrependLengthInfo = false; // fhem accepts no size info for this msg
        break;
    }
    case enuMsgType_EIB_OPEN_GROUPCON: {
        grResponse.append( CModel::g_uzEIB_OPEN_GROUPCON[0] );
        grResponse.append( CModel::g_uzEIB_OPEN_GROUPCON[1] );
        bHasResponde = true;
        bPrependLengthInfo = false; // fhem accepts no size info for this msg
        break;
    }
    case enuMsgType_EIB_GROUP_PACKET: {
        bHasResponde = false;
        bPrependLengthInfo = false;
        break;
    }
    case enuMsgType_EIB_OPEN_T_GROUP: {
        grResponse.append( char( 0x00 ) ); ///@todo check length
        grResponse.append( char( 0x02) );
        grResponse.append( CModel::g_uzEIB_OPEN_T_GROUP[ 0 ] );
        grResponse.append( CModel::g_uzEIB_OPEN_T_GROUP[ 1 ] );
        bHasResponde = true;
        bPrependLengthInfo = false; /// @todo check if correct
        break;
    }
    case enuMsgType_EIB_APDU_PACKET: {
        if ( m_eAPDUType == enuAPDUType_A_GroupValue_Read_PDU ) {

            // response
            uchar szByte0 = 0x00;
            uchar szByte1 = 0x40; // 0100 0000

            bHasResponde       = true;
            bPrependLengthInfo = false; /// @todo check if correct

            uchar szData = 0x00;
            float fVal = m_grValue.toFloat();
            if ( fVal == 1.0 ) {
                szData = 0x01;
            }
            else if ( fVal == 0.0 ) {
                szData = 0x00;
            }
            else {
                QLOG_WARN() << QObject::tr( "Value is not bool. Feature not implemented yet. Providing 0x00." ).toStdString().c_str();
                bHasResponde = true;
            }

            szByte1 = szByte1 | ( szData & 0x3f ); // 0011 1111

            grResponse.append( szByte0 );
            grResponse.append( szByte1 );
        }
        else if ( m_eAPDUType == enuAPDUType_A_GroupValue_Write_PDU ) {
            bHasResponde = false;
        }
        break;
    }
    default: {
        bHasResponde = false;

    }
    }

    if ( p_pHasResponse != NULL ) {
        * p_pHasResponse = bHasResponde;
    }

    int nLength = grResponse.length();

    if ( ( bPrependLengthInfo == true ) && ( nLength > 0 ) ) {
        char szLength0 = 0x00;
        char szLength1 = nLength;
        grResponse.prepend( szLength1 );
        grResponse.prepend( szLength0 );
    }

    return grResponse;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

QByteArray CEibdMsg::getMessage(const QString &p_sSrcAddr, const QString &p_sDestAddr, const QVariant &p_grData)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    // byte  0: 0x00
    // byte  1: Length w/o byte 1 + 2
    // byte  2: 0x00
    // byte  3: 0x27
    // byte  4: src addr
    // byte  5: src addr
    // byte  6: dest addr
    // byte  7: dest addr
    // byte  8: 0x00
    // byte  9: action  0x80: 'write'; 0x40: 'response'; 0x00: 'read';
    // byte 10: data if not in 8

    QByteArray grMsg;

    grMsg.append( char( 0x00 ) ); // index 0
    grMsg.append( char( 0x08 ) ); // index 1
    grMsg.append( char( 0x00 ) ); // index 2
    grMsg.append( char( 0x27 ) ); // index 3

    CGroupAddress grSrcAddr;
    CGroupAddress grDestAddr;

    grSrcAddr.setKNXString( p_sSrcAddr );
    grDestAddr.setKNXString( p_sDestAddr );

    grMsg.append( grSrcAddr.toHex() );   // index 4 + 5
    grMsg.append( grDestAddr.toHex() );  // index 6 + 7

    grMsg.append( char( 0x00 ) ); // index 8

    bool bOk;
    float fVal = p_grData.toFloat( & bOk );

    if ( bOk == false )
    {
        QLOG_ERROR() << QObject::tr("Received non float value. Transmission aborted.").toStdString().c_str() << p_grData.toString();
        return QByteArray();
    }

    CKoXml::enuDPT eDPT = CKoXml::getInstance()->getGaDPT( p_sDestAddr );

    switch( eDPT ) {
    case CKoXml::enuDPT_DPT1: {
        int nVal = (int) fVal;
        char szData = nVal;// QString::number( nVal ).toAscii();
        szData = szData | 0x80;
        grMsg.append( szData ); // index 9 & 10
        break;
    }

    case CKoXml::enuDPT_DPT5_DPT6: {
        // e.g. "00 27 1a 10 00 80 ff"
        grMsg.append( char( 0x80 ) ); // index 8
        quint8 unVal = static_cast< quint8 >( fVal );
        grMsg.append( unVal );
        grMsg.replace(1, quint8( 0x09 )); // correction of msg length
        break;
    }

    default: {
        QLOG_WARN() << QObject::tr("Requested DPT not supported. EIS / Value was").toStdString().c_str()
                    << CKoXml::getInstance()->getGaFormat( p_sDestAddr ).toStdString().c_str()
                    << " / "
                    << p_grData;
        break;
    }
    }

    return grMsg;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

int CEibdMsg::getMsgDataSize() const
{
    return m_nMsgSize;
}

void CEibdMsg::setValue(const float &p_fVal)
{
    m_grValue = p_fVal;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool CEibdMsg::isNatural(const float & p_fNumber)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QStringList grStringList;

    QString sNumer = QString::number( p_fNumber );

    if ( sNumer.contains( "." ) == true )
    {
        grStringList = sNumer.split( '.', QString::SkipEmptyParts );

        if ( grStringList.size() != 2 )
        {
            return false;
        }

        if ( grStringList.at( 1 ).toInt() != 0 )
        {
            return false;
        }

        return true;
    }
    else
    {
        bool bRet;
        sNumer.toInt( & bRet );

        return bRet;
    }
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

bool CEibdMsg::equals(const QByteArray &p_grByteArray, const uchar * p_grCharArr, const int &p_nLength)
{
    for ( int i = 0; i < p_nLength; ++ i) {
        if ( p_grByteArray.data()[ i ] != p_grCharArr[ i ] ) {
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setEib1(const uchar & p_szData)
{
    uchar szData = p_szData;
    szData = szData & 0x7f; // 0x7f = 0111 1111
    m_grValue.setValue( static_cast< int >( szData ) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setDTP5(const QByteArray &p_grData)
{
    quint8 unData = static_cast< quint8 >( p_grData.at( 0 ) );
    m_grValue.setValue( static_cast< int >( unData ) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setDTP9_001(const QByteArray & p_grData)
{
    // DPT 9.001 DPT_Value_Temp is a 2-octet float value.
    // The format is MEEE EMMM   MMMM MMMM (16 bits). The value is then 0,01 x M x 2^E. The mantissa (M) is coded two's complement.
    // If I calculated correctly, $193D is 25,36 °C.

    uchar szTemp;
    uchar szM[2];
    uchar szE;

    // save top bit for sigend int
    szTemp = p_grData.at( 0 ) & 0x80; // 0x80 = 1000 0000

    szM[ 0 ] = p_grData.at( 0 ) & 0x07; // 0x87 = 0000 0111
    szM[ 1 ] = p_grData.at( 1 ); // & 0xFF; // 0xFF = 1111 1111

    QByteArray grM;
    grM.append( szM[ 0 ] );
    grM.append( szM[ 1 ] );

    szE    = p_grData.at( 0 ) & 0x78; // 0x78 = 0111 1000
    szE   >>= 3; // shift bits to the right: 0xxx x000 >> 0000 xxxx
    int nE = static_cast< int >( szE );

    // transfer M bit representation to int representation
    int nM = 0;
    nM |= grM.at( 0 );
    nM <<= 8;
    nM |= grM.at( 1 );

    // resepct sign via two’s complement notation: negative, if highest bit is 1
    if ( szTemp == 0x80 )
    {
        nM *= -1;
    }

    // Calculate DPT 9.001 resp. DPT_Value_Temp resp. 2-octet float value
    float fValue = 0.01 * nM * qPow( 2, nE );
    m_grValue.setValue( fValue );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setEibAddress(const QByteArray &p_grData)
{
    CGroupAddress grGA;
    grGA.setHex( p_grData );
    m_sDstAddrKnx = grGA.toKNXString();
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

QString CEibdMsg::printASCII( const QByteArray & p_grByteArray)
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QString sResult;
    QString sBuffer;
    for ( int i = 0; i < p_grByteArray.length(); i++ )
    {
        sBuffer = QString::number( uchar( p_grByteArray.at( i ) ), 16 );
        if ( sBuffer.length() == 1 )
        {
            sBuffer = "0" + sBuffer;
        }

        sResult += sBuffer;

        if ( i < p_grByteArray.length() - 1 )
        {
            sResult += " ";
        }
    }

    return sResult;
}
