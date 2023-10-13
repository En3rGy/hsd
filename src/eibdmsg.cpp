#include <QDebug>

#include "eibdmsg.h"
#include "model.h"
#include <QStringList>
#include "groupaddress.h"
#include <QtLogging>
#include "koxml.h"
#include <qmath.h>
#include <QtEndian>
#include <QBitArray>
#include <QDataStream>

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

QList<QByteArray> CEibdMsg::splitMessages(QByteArray &p_grMessages)
{
    // find start index of 0x0027
    QList < int > grIdxList;
    for ( int i = 0; i < p_grMessages.size(); ++i ) {

        // split at 00 027; check for length byte before
        if ( i < p_grMessages.size() - 1 ) {
            if ( p_grMessages.at( i ) == 0x00 and p_grMessages.at( i + 1 ) == 0x27 ) {

                // note 2 length byte if avialble
                if ( i > 1 ) {
                    grIdxList.append( i - 2 );
                }
                else {
                    grIdxList.append( i );
                }
            }
        }
    }

    // copy identified messages
    QList<QByteArray> grRetList;
    QByteArray grMsg;
    for( int i = 0; i < grIdxList.size(); ++i ) {
        int nIdx = grIdxList.at( i );
        int nNextIdx;
        if ( i < grIdxList.size() - 1 ) {
            nNextIdx = grIdxList.at( i + 1 );
        }
        else {
            nNextIdx = p_grMessages.size();
        }
        grMsg = p_grMessages.mid( nIdx, nNextIdx - nIdx );
        grRetList.append( grMsg );
    }

    if ( grIdxList.isEmpty() ) {
        grRetList.append( p_grMessages );
    }

    return grRetList;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setEibdMsg(const QByteArray &p_grByteArray, const QString &p_sGA)
{
    QByteArray grMsg;

    // check if 1st 2 byte contain package length
    if ( p_grByteArray.size() < 2 ) {
        qWarning() << QObject::tr("Received message too short. Message was") << printASCII( p_grByteArray );
        return;
    }

    if ( p_grByteArray.at( 1 ) < char( 0x20 ) ) { // size info
        m_eMsgType    = enuMsgType_msgSize;
        QString sSize = QString::number( p_grByteArray.at( 1 ) );
        m_nMsgSize    = static_cast< int >( sSize.toDouble() );

        if ( p_grByteArray.size() == 2 ) {
            //QLOG_DEBUG() << QObject::tr("Msg interpreted as size info. Awaiting message with size") << m_nMsgSize;
            return;
        }
    }

    if ( m_nMsgSize != -1 ) {
        grMsg.append( p_grByteArray.mid( 2, m_nMsgSize ) ); // removing size info

        if ( grMsg.size() > m_nMsgSize ) {
            qWarning() << QObject::tr("Message longer than indicated; truncating. Msg was:") << printASCII( p_grByteArray );
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
        m_nMsgSize = static_cast< int >( QString::number( p_grByteArray.at( 1 ) ).toDouble() );
    }
    else if ( equals( grMsgType, CModel::g_uzEIB_APDU_PACKET, 2 ) ) {
        m_eMsgType = enuMsgType_EIB_APDU_PACKET;

        // paylaod is within byte 2, 3
        // Check 1st payload byte
        uchar szPayload0 = grMsg.data()[2] & 0x03; // 0000 0011
        uchar szPayload1 = grMsg.data()[3] & 0xC0; // 1100 0000

        // GroupValue Write    = XXXX XX00 10VV VVVV (2 bytes)
        // GroupValue Read     = XXXX XX00 00XX XXXX  (2 bytes)
        // GroupValue Response = XXXX XX00 01VV VVVV  (2 bytes)

        if ( szPayload0 == 0x00 ) {
            // Check 2nd payload byte
            if ( szPayload1 == 0x00 ) {
                /* A GroupValue Read  This APDU is sent, to tell a device, that it should send the
                 * current values of the group object in a A GroupValue Response.  The format
                 * it XXXX XX00 00XX XXXX  (2 bytes).
                 */
                m_eAPDUType = enuAPDUType_A_GroupValue_Read_PDU;
            }
            else if ( szPayload1 == 0x40 ) {
                /* A GroupValue Response  This APDU is used to answer a A GroupValue Read
                 * request.  The format is XXXX XX00 01VV VVVV  (2 bytes).  If the datatype
                 * to transmit is between 1 and 6 bit longs,  the lower 6 bits (V ) contain the
                 * value.  If the value is 1 (or more) bytes long, all V s are zero and the value is
                 * appended after the two bytes.
                 */
                m_eAPDUType = enuAPDUType_undef;
            }
            else if ( szPayload1 == 0x80 ) {
                /*
                A GroupValue Write  This APDU is used to update a group object (ie.  switch a
                light on). The format is XXXX XX00 10VV VVVV (2 bytes). If the datatype
                to transmit is between 1 and 6 bit longs,  the lower 6 bits (V ) contain the
                value.  If the value is 1 (or more) bytes long, all V s are zero and the value is
                appended after the two bytes.
                */

                if ( grMsg.size() == 4 ) {
                    setEib1( grMsg.data()[3] & 0x3F );
                }
                else {
                    QByteArray grPayload = grMsg.mid( 4 );

                    if ( grPayload.size() == 1 ) {
                        switch ( CKoXml::getInstance()->getGaDPT( p_sGA ) ) {
                        case CKoXml::enuDPT_DPT5_001: {
                            setDTP5_001( grPayload );
                            break;
                        }
                        case CKoXml::enuDPT_DPT5_004: {
                            setDTP5_004( grPayload );
                            break;
                        }
                        default: {
                            break;
                        }
                        }
                    }
                    else if ( grPayload.size() == 2 ) {
                        setDTP9_001( grPayload );
                    }
                    else if ( grPayload.size() == 4 ) {
                        setDTP3( grPayload );
                    }
                    else {
                        qCritical() << QObject::tr( "DTP of payload could not be identified.")
                                     << "Msg:" << CEibdMsg::printASCII( p_grByteArray )
                                     << "Payload:" << CEibdMsg::printASCII( grPayload );
                    }
                }
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

        CKoXml::enuDPT eDpt = CKoXml::getInstance()->getGaDPT( getDestAddressKnx() );

        switch( eDpt ) {
        case CKoXml::enuDPT_undef: {
            qWarning() << QObject::tr("DPT/EIS of GA unknown") << getDestAddressKnx();
            break;
        }
        case CKoXml::enuDPT_DPT1: {
            setEib1( grMsg.at( 5 ) );
            break;
        }
        case CKoXml::enuDPT_DPT5_001: {
            setDTP5_001( grData.mid( 1, 1 ) ); // skipping 80 byte
            break;
        }
        case CKoXml::enuDPT_DPT5_004: {
            setDTP5_004( grData.mid( 1, 1 ) ); // skipping 80 byte
            break;
        }
            //case CKoXml::enuDPT_DPT3: {break;}
        case CKoXml::enuDPT_DPT9: {
            setDTP9_001( grData.mid( 1, 2 ) ); // skipping 1st byte
            break;
        }
        case CKoXml::enuDPT_DPT16 : {
            setDtp16( grData.mid( 1, 14 ) ); ///@todo skipping 1st byte is ok?
            break;
        }
        default: {
            qCritical() << QObject::tr( "Unknown DTP of data bytes in EIB message:" ) << printASCII( grMsg );
        }
        } // switch
    } // else if ( grMsgType == QByteArray( * CModel::g_uzEIB_GROUP_PACKET, 2 ) )
    else {
        qWarning() << QObject::tr("Received unknown message") << printASCII( grMsg );
    }

    m_nMsgSize = -1; // reset info
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const CEibdMsg::enuMsgType &CEibdMsg::getType() const
{
    return m_eMsgType;
}

const CEibdMsg::enuAPDUType &CEibdMsg::getAPDUType() const
{
    return m_eAPDUType;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const QString &CEibdMsg::getDestAddressKnx() const
{
    return m_sDstAddrKnx;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

const QVariant & CEibdMsg::getValue( bool * p_pHasValue ) const
{
    if ( p_pHasValue != nullptr )
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
        bPrependLengthInfo = true; // fhem accepts no size info for this msg
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
            char szByte0 = 0x00;
            char szByte1 = 0x40; // 0100 0000

            bHasResponde       = true;
            bPrependLengthInfo = false; /// @todo check if correct

            uchar szData = 0x00;
            float fVal = m_grValue.toFloat();
            if ( fVal == 1.0f ) {
                szData = 0x01;
            }
            else if ( fVal == 0.0f ) {
                szData = 0x00;
            }
            else {
                qWarning() << QObject::tr( "Value is not bool. Feature not implemented yet. Providing 0x00." );
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

    if ( p_pHasResponse != nullptr ) {
        * p_pHasResponse = bHasResponde;
    }

    int nLength = grResponse.length();

    if ( ( bPrependLengthInfo == true ) && ( nLength > 0 ) ) {
        char szLength0 = 0x00;
        char szLength1 = static_cast< char >( nLength );
        grResponse.prepend( szLength1 );
        grResponse.prepend( szLength0 );
    }

    return grResponse;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

QByteArray CEibdMsg::getMessage(const QString &p_sSrcAddr, const QString &p_sDestAddr, const QVariant &p_grData, const QByteArray &p_grByteMsg)
{
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
        qCritical() << QObject::tr("Received non float value. Transmission aborted.") << p_grData.toString();
        return QByteArray();
    }

    CKoXml::enuDPT eDPT = CKoXml::getInstance()->getGaDPT( p_sDestAddr );

    switch( eDPT ) {
    case CKoXml::enuDPT_DPT1: {
        int nVal = static_cast< int >( fVal );
        char szData = static_cast< char >( nVal );// QString::number( nVal ).toAscii();
        szData = szData | 0x80;
        grMsg.append( szData ); // index 9 & 10
        break;
    }

    case CKoXml::enuDPT_DPT5_004: {
        // e.g. "00 27 1a 10 00 80 ff"
        grMsg.append( char( 0x80 ) ); // index 8
        quint8 nVal = static_cast< quint8 >( fVal );
        grMsg.append( nVal );
        grMsg[ 1 ] = 0x09; // correction of msg length
        break;
    }

    case CKoXml::enuDPT_DPT5_001: {
        // e.g. "00 27 1a 10 00 80 ff"
        grMsg.append( char( 0x80 ) ); // index 8
        quint8 unVal = static_cast< quint8 >( fVal * 2.55f );
        grMsg.append( unVal );
        grMsg[ 1 ] = 0x09; // correction of msg length
        break;
    }

    case CKoXml::enuDPT_DPT3: {
        qint32 nVal = static_cast< qint32 >( fVal );
        QByteArray grVal;
        QDataStream in( & grVal, QIODeviceBase::ReadWrite );
        in << nVal;
        grMsg.append( grVal );

        /// @todo check if 0x00 before value is required
        grMsg[ 1 ] = 0x0B; // correction of msg length
        qDebug() << printASCII( grMsg ) << Q_FUNC_INFO;
        break;
    }

    case CKoXml::enuDPT_DPT9: {

        // 00 08 00 27 25 00 00 80 8A 24

        grMsg.append( char( 0x80 ) ); // index 8

        qint16 nRes = 0;

        //        Implement  FloatValue = (0,01*M)*2^(E) with MEEE EMMM MMMM MMMM

        //        Step 1: Calculate the mantissa
        //        Due to the resolution of 0.01, the value to be coded must be multiplied by 100: 30 x 100 =
        //        3000

        qint16 nE = 0;
        qint16 nM = static_cast< qint16 >( fVal * 100.0f );

        //        Step 2: Check if exponent is required

        //        Mantissa is 11 bits, range is from + 2047 to -2048.
        //        3000 is larger, therefore exponent is required.
        //        Which exponent? 21 = 2 is sufficient as 3000 : 2 = 1500, and this number can be coded in
        //        the mantissa.

        if ( nM > -2048 and nM < 2048 ) {
            /// @todo get exponent
            qWarning() << "Values nM < -2048 and nM > 2048 not supported for DTP9 yet.";
        }

        //        Step 3: Code the mantissa:

        //        Value:       1024     512      256      128       64       32      16       8        4        2        1
        //        Number:        1         0          1         1          1         0        1        1        1        0        0


        //        If the number is negative, then create a two’s complement!

        //        Output value:  101 1101 1100

        //        Invert:              010 0010 0011
        //        +1                                         1
        //        -------------------------------------------------
        //                                010 0010 0100

        if ( nM < 0 ) {
            /// @todo get two's complement

            nRes |= 0x01;
            nRes = nRes << 7;

            qWarning() << "Values nM < 0 not supported for DTP9 yet.";
        }

        //        Step 4: Code sign and exponent
        //        Number is negative, therefore the S bit = 1
        //        Exponent = 1, coded in four bits = 0001

        //        Step 5: Final result

        //        -30 = 1   0001   010 0010 0100

        //       MEEE EMMM MMMM MMMM

        nE = nE & 0x000F;
        nE = nE << 11;

        nRes |= nE;

        nM = nM & 0x07FF;

        nRes |= nM;

        QByteArray grVal;
        QDataStream in( & grVal, QIODeviceBase::ReadWrite );
        in << nRes;
        grMsg.append( grVal );

        //grMsg.remove( 4, 2 ); // remove src address

        /// @todo check if 0x00 before value is required
        grMsg[ 1 ] = 0x0A; // correction of msg length
        qDebug() << printASCII( grMsg ) << Q_FUNC_INFO;

        break;
    }

    case CKoXml::enuDPT_DPT16: {
        /// @todo
        qCritical() << QObject::tr( "DPT14 14-byte text not implemented yet!" );
        break;
    }

    default: {
        qWarning() << QObject::tr("Requested DPT not supported. EIS / Value: \"")
                    << CKoXml::getInstance()->getGaFormat( p_sDestAddr )
                    << " / "
                    << p_grData << "\"\n"
                    << "Original Message:" << printASCII( p_grByteMsg );
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
    QStringList grStringList;

    QString sNumer = QString::number( p_fNumber );

    if ( sNumer.contains( "." ) == true )
    {
        grStringList = sNumer.split( '.', Qt::SkipEmptyParts );

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

bool CEibdMsg::equals(const QByteArray &p_grByteArray, const char * p_grCharArr, const int &p_nLength)
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

void CEibdMsg::setDTP3(const QByteArray &p_grData)
{
    QDataStream out( p_grData );
    qint32 nData;
    out >> nData;;
    m_grValue.setValue( nData );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setDTP5_001(const QByteArray &p_grData)
{
    quint8 unData = static_cast< quint8 >( p_grData.at( 0 ) );
    m_grValue.setValue( static_cast< int >( unData * 100.0f / 255.0f ) );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setDTP5_004(const QByteArray &p_grData)
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
    // The format is MEEE EMMM MMMM MMMM (16 bits). The value is then 0,01 x M x 2^E. The mantissa (M) is coded two's complement.
    // If I calculated correctly, $193D is 25,36 °C.

    //    A temperature value of - 30 degrees C can be calculated according DPT 9.001 as follows:
    //    Step 1: Calculate the mantissa
    //    Due to the resolution of 0.01, the value to be coded must be multiplied by 100: 30 x 100 = 3000
    //
    //    Step 2: Check if exponent is required
    //    Mantissa is 11 bits, range is from + 2047 to -2048.
    //    3000 is larger, therefore exponent is required.
    //    Which exponent? 2 1 = 2
    //    is sufficient as 3000 : 2 = 1500, and this number can be coded in
    //    the mantissa.
    //
    //    Step 3: Code the mantissa:
    //    Value:  1024 512 256 128 64 32 16 8 4 2 1
    //    Number:    1   0   1   1  1  0 1  1 1 0 0
    //
    //    If the number is negative, then create a two’s complement!
    //    Output value:  101 1101 1100
    //    Invert:        010 0010 0011
    //    +1                         1
    //    -------------------------------------------------
    //                   010 0010 0100
    //
    //    Step 4: Code sign and exponent
    //    Number is negative, therefore the S bit = 1
    //    Exponent = 1, coded in four bits = 0001
    //
    //    Step 5: Final result
    //    -30 = 1 0001 010 0010 0100 => 1000 1010 0010 0100 => 0x8A 0x24

    // Test if input values are correct
    qint16 nIn = 0;
    const char * szIn = p_grData.data();
    nIn |= ( szIn[0] << 8 );
    nIn |= szIn[ 1 ];
    // OK: qDebug() << printASCII( p_grData ) << "sould be 8A 24" << Q_FUNC_INFO;

    // endian check
    qint16 nTest  = 1;
    char * szTest = reinterpret_cast< char * > ( & nTest );

    bool bLE = false;
    if ( szTest[0] == 0x01 ) { // litlle endian
        bLE = true;
    }

    if ( bLE == false ) {
        qCritical() << "Big Endian not supported!" << Q_FUNC_INFO;
        return;
    }

    // start processing dtp 9
    uchar szSign;
    uchar szE;

    // save top bit for sigend int
    szSign = p_grData.at( 0 ) & 0x80; // 0x80 = 1000 0000

    // transfer M bit representation to int representation
    // MEEE EMMM MMMM MMMM
    qint16 nM  = 0;
    char * pzM = reinterpret_cast< char * > ( & nM );

    nM = nIn & 0x07FF;

    //qDebug() << " 0000 0MMM MMMM MMMM";
    //qDebug() << "In:      " << printBin( nIn ) << "sould be 1000 1010 0010 0100" << Q_FUNC_INFO;
    //qDebug() << "Mantissa:" << printBin( nM ) << " should be 0000 0010 0010 0100";


    // in case of negative value, revert two component representation
    if ( szSign == 0x80 ) {
        // substract 1 by adding the two complement of 1
        nM = bDecr( nM );

        // invert value
        pzM[ 0 ] = ~pzM[ 0 ];
        pzM[ 1 ] = ~pzM[ 1 ];

        nM &= 2047; // limit to mantisa

        nM *= -1;
    }

    //qDebug() << "Restr.:  " << printBin( nM ) << " should be 0000 0101 1101 1100";


    szE    = p_grData.at( 0 ) & 0x78; // 0x78 = 0111 1000
    szE   >>= 3; // shift bits to the right: 0xxx x000 >> 0000 xxxx
    int nE = static_cast< int >( szE );

    // Calculate DPT 9.001 resp. DPT_Value_Temp resp. 2-octet float value
    float fValue = 0.01 * nM * qPow( 2, nE );
    m_grValue.setValue( fValue );
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void CEibdMsg::setDtp16(const QByteArray &p_grData)
{
    QString sMsg =  QString::fromUtf8( p_grData );
    m_grValue.setValue( sMsg );
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

qint16 CEibdMsg::bDecr(qint16 p_nN1)
{
    // increment by adding 0xFFFF (two complement of 0x0001

    QBitArray grBits( 16 );
    bool bCarryBit = false;

    for ( int i = 0; i < 16; ++i ) {

        // read int 2 bit array
        grBits.setBit( i, (( p_nN1 >> i ) & 0x0001) == 0x0001 ); // bit[0] most right; right to left; little endian

        // add 1 to each bit
        if( grBits.testBit( i ) == true ) {
            // sum = 2 -> 0 + carry bit
            if ( bCarryBit == false ) {
                grBits.setBit( i, false ); // take 1 over
                bCarryBit = true;
            }
            // sum = 3 -> 1 + carry bit
            else {
                grBits.setBit( i, true ); // take 1 over
                bCarryBit = true;
            }
        }
        // current bit = 0
        else {
            if ( bCarryBit == false ) { // sum = 1 -> 1, no carry bit
                grBits.setBit( i, true ); // take 1 over
                bCarryBit = false;
            }
            else { // sum = 2 -> 0, carry bit
                grBits.setBit( i, false ); // take 1 over
                bCarryBit = true;
            }
        } // else

    }// for 0..15

    // put bit array 2 int
    qint16 nRetVal = 0;
    for ( int i = 0; i < 16; ++i ) { // bit[0] most right
        if ( grBits.testBit( 15 - i ) == true ) {
            nRetVal = nRetVal | ( 0x0001 << (15 - i ) );
        }
    }

    return nRetVal;
}

QString CEibdMsg::printBin(qint16 p_nNo)
{
    QString sRes;
    for ( int i = 0; i < 16; ++i ) {
        if ( ( i % 4 == 0 ) && ( i > 0 ) ) {
            sRes += " ";
        }
        if ( ( ( p_nNo >> (15 - i )) & 0x0001 ) == 0x001 ) {
            sRes += "1";
        }
        else {
            sRes += "0";
        }
    }
    return sRes;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

QString CEibdMsg::printASCII( const QByteArray & p_grByteArray)
{
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
