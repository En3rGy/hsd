#include <QDebug>

#include "eibdmsg.h"
#include "model.h"
#include <QStringList>
#include "groupaddress.h"
#include "QsLog.h"
#include <qmath.h>

CEibdMsg::CEibdMsg()
{
}

CEibdMsg::CEibdMsg(const QByteArray & p_grByteArray)
{
    QLOG_TRACE() << Q_FUNC_INFO;

    QByteArray grMsg;

    // check if 1st 2 byte contain package length
    if ( p_grByteArray.size() < 2 )
    {
        QLOG_WARN() << QObject::tr("Received too short message. Message was").toStdString().c_str() << printASCII( p_grByteArray );
        return;
    }

    QString sSize = QString::number( p_grByteArray.at( 1 ) );
    m_nMsgSize    = ( int ) sSize.toDouble();

    if ( p_grByteArray.size() - 2 == m_nMsgSize )
    {
        grMsg.append( p_grByteArray.mid( 2, p_grByteArray.size() - 2 ) );
    }
    else
    {
        grMsg.append( p_grByteArray );
    }

    switch ( grMsg.length() )
    {
    case 2: // connect
        if ( ( grMsg.data()[0] == CModel::g_uzEibAck[0] ) &&
             ( grMsg.data()[1] == CModel::g_uzEibAck[1] ) )
        {
            m_eMsgType = enuMsgType_connect;
        }
        else
        {
            m_eMsgType = enuMsgType_msgSize;
            m_nMsgSize = ( int ) QString::number( p_grByteArray.at( 1 ) ).toDouble();
        }
        break;

    case 5: // openGroupSocket
        if ( ( grMsg.data()[0] == CModel::g_uzEibOpenGroupCon[0] ) &&
             ( grMsg.data()[1] == CModel::g_uzEibOpenGroupCon[1] ) &&
             ( grMsg.data()[2] == CModel::g_uzEibOpenGroupCon[2] ) &&
             ( grMsg.data()[3] == CModel::g_uzEibOpenGroupCon[3] ) &&
             ( grMsg.data()[4] == CModel::g_uzEibOpenGroupCon[4] ) )
        {
            m_eMsgType = enuMsgType_openGroupSocket;
        }
        break;

    default:
        if ( grMsg.size() >= 6 )  // set request, e.g. (0) 00 (1) 27 (2) 09 (3) 0f (4) 00 (5) 80 (6) 08 (7) 73
        {

            // Determine message type via byte 0 + 1, e.g. 00 27

            if ( ( grMsg.data()[0] == CModel::g_uzEibGroupPacket[0] ) &&
                 ( grMsg.data()[1] == CModel::g_uzEibGroupPacket[1] ) )
            {
                m_eMsgType = enuMsgType_simpleWrite;


                // determine eib adress via byte 2 + 3, e.g. 09 0f for 0/9/15

                QByteArray grEibAdr;
                grEibAdr.append( grMsg.data()[2]);
                grEibAdr.append( grMsg.data()[3]);

                CGroupAddress grGA;
                grGA.setHex( grEibAdr );
                m_sDstAddr = grGA.toKNXString();

                // Process data

                QByteArray grData;
                grData.append( grMsg.mid( 5, grMsg.size() - 5 ) ); // skipping byte 4 which is 00

                if ( grData.size() == 1 ) // e.g. EIS1 = 1 Bit
                {
                    uchar szData = grMsg.at( 5 );
                    szData = szData & 0x7f; // 0x7f = 0111 1111
                    m_grValue.setValue( static_cast< int >( szData ) );
                }
                else if ( grData.size() == 3 ) // F_16 = DPT 9.001 resp. DPT_Value_Temp resp. 2-octet float value
                {
                    grData.remove( 0, 1 );

                    // DPT 9.001 DPT_Value_Temp is a 2-octet float value.
                    // The format is MEEE EMMM   MMMM MMMM (16 bits). The value is then 0,01 x M x 2^E. The mantissa (M) is coded two's complement.
                    // If I calculated correctly, $193D is 25,36 °C.

                    uchar szTemp;
                    uchar szM[2];
                    uchar szE;

                    // save top bit for sigend int
                    szTemp = grData.at( 0 ) & 0x80; // 0x80 = 1000 0000

                    szM[ 0 ] = grData.at( 0 ) & 0x07; // 0x87 = 0000 0111
                    szM[ 1 ] = grData.at( 1 ); // & 0xFF; // 0xFF = 1111 1111

                    QByteArray grM;
                    grM.append( szM[ 0 ] );
                    grM.append( szM[ 1 ] );

                    szE    = grData.at( 0 ) & 0x78; // 0x78 = 0111 1000
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
                else
                {
                    QLOG_ERROR() << QObject::tr( "Unknown DTP of data bytes in EIB message:" ).toStdString().c_str() << printASCII( grMsg );
                }

            break;
            }

            QLOG_INFO() << QObject::tr("Received unknown message").toStdString().c_str() << printASCII( grMsg );
        }
    }
}

const CEibdMsg::enuMsgType &CEibdMsg::getType() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    return m_eMsgType;
}

const QString &CEibdMsg::getDestAddress() const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    return m_sDstAddr;
}

const QVariant & CEibdMsg::getValue( bool * p_pHasValue ) const
{
    QLOG_TRACE() << Q_FUNC_INFO;
    if ( p_pHasValue != NULL )
    {
        * p_pHasValue = ! m_grValue.isNull();
    }
    return m_grValue;
}

QByteArray CEibdMsg::getResponse( bool * p_pHasResponse )
{
    QLOG_TRACE() << Q_FUNC_INFO;
    QByteArray grResponse;

    switch( m_eMsgType )
    {
    case enuMsgType_connect:
    {
        grResponse.append( ( const char * ) CModel::g_uzEibAck, 2 );
        if ( p_pHasResponse != NULL )
        {
            * p_pHasResponse = true;
        }
        break;
    }
    case enuMsgType_openGroupSocket:
    {
        const char szOpenGroupSocketAck [2] = { 0x00, 0x26 };
        grResponse.append( QByteArray( szOpenGroupSocketAck, 2 ) );
        if ( p_pHasResponse != NULL )
        {
            * p_pHasResponse = true;
        }
        break;
    }
    case enuMsgType_simpleWrite:
    {
        if ( p_pHasResponse != NULL )
        {
            * p_pHasResponse = false;
        }
        break;
    }
    default:
    {
        if ( p_pHasResponse != NULL )
        {
            * p_pHasResponse = false;
        }
    }
    }

    return grResponse;
}

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

    /// @todo Encode float values

    if ( isNatural( fVal ) != true )
    {
        QLOG_WARN() << QObject::tr("Forwarding float values vie eibd interface is not supportet, converting to int. Value was").toStdString().c_str() << p_grData.toString();
    }

    int nVal = (int) fVal;

    if ( nVal > 100 )
    {
        QLOG_WARN() << QObject::tr("Can only forward positive natural numbers < 100, setting value to 0. Value was").toStdString().c_str() << p_grData.toString();
        nVal = 0;
    }

    char szData = nVal;// QString::number( nVal ).toAscii();
    szData = szData | 0x80;
    grMsg.append( szData ); // index 9 & 10

    return grMsg;

    //    grMsg.append( char( 0x80 ) ); // index 9
    //    grMsg.append( dVal );

    //    quint8 nSize = grMsg.size() - 2;

    //    grMsg[ 1 ] = nSize; // index 1

    //    return grMsg;
}

int CEibdMsg::getMsgDataSize() const
{
    return m_nMsgSize;
}

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
