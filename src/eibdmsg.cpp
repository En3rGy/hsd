#include <QDebug>

#include "eibdmsg.h"
#include "model.h"
#include <QStringList>
#include "groupaddress.h"

CEibdMsg::CEibdMsg()
{
}

CEibdMsg::CEibdMsg(const QByteArray & p_grByteArray)
{
    QByteArray grMsg;

    // check if 1st 2 byte contain package length
    if ( p_grByteArray.size() < 2 )
    {
        qDebug() << "Received too short message " << printASCII( p_grByteArray );
        return;
    }

    QString sSize = QString::number( p_grByteArray.at( 1 ) );
    int nSize = ( int ) sSize.toDouble();

    if ( p_grByteArray.size() - 2 == nSize )
    {
        grMsg.append( p_grByteArray.mid( 2, p_grByteArray.size() - 2 ) );
    }
    else
    {
        grMsg.append( p_grByteArray );
    }

    QString sString( grMsg.data() );

    switch ( grMsg.length() )
    {
    case 2: // connect
        if ( ( grMsg.data()[0] == CModel::g_uzEibAck[0] ) &&
             ( grMsg.data()[1] == CModel::g_uzEibAck[1] ) )
        {
            m_eMsgType = enuMsgType_connect;
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
        if ( grMsg.size() >= 6 )  // set request, e.g. 00 27 09 0f 00 80
        {

            if ( ( grMsg.data()[0] == CModel::g_uzEibGroupPacket[0] ) &&
                 ( grMsg.data()[1] == CModel::g_uzEibGroupPacket[1] ) )
            {
                m_eMsgType = enuMsgType_simpleWrite;

                QByteArray grEibAdr;
                grEibAdr.append( grMsg.data()[2]);
                grEibAdr.append( grMsg.data()[3]);

                CGroupAddress grGA;
                grGA.setHex( grEibAdr );
                m_sDstAddr = grGA.toKNXString();

                uchar szData = grMsg.at( 5 );
                szData = szData & 0x7f; // 0x7f = 0111 1111

                // If you need to send data with a data type bigger than 6 bit:
                // byte data[] = new byte[2 + <size of datatype in bytes>];
                // data[0] = 0;
                // data[1] =0x80;
                // data[2]=<first byte of datatype>;
                // data[3]=<second byte of datatype>;

                QByteArray grData;
                grData.append( grMsg.mid( 5, grMsg.size() - 5 ) );

                qDebug() << "Data field" << printASCII( grData );

                if ( grData.size() == 1 )
                {
                    uchar szData = grMsg.at( 5 );
                    szData = szData & 0x7f; // 0x7f = 0111 1111
                    m_grValue.setValue( ( int ) szData );
                }
                else if ( grData.size() > 1 )
                {
                    grData.remove( 0, 2 );
                    m_grValue.setValue( grData );
                }
            }
            break;

            qDebug() << "Received unknown message: " << printASCII( grMsg );
        }
    }
}

const CEibdMsg::enuMsgType &CEibdMsg::getType() const
{
    return m_eMsgType;
}

const QString &CEibdMsg::getDestAddress() const
{
    return m_sDstAddr;
}

const QVariant & CEibdMsg::getValue( bool * p_pHasValue ) const
{
    if ( p_pHasValue != NULL )
    {
        * p_pHasValue = ! m_grValue.isNull();
    }
    return m_grValue;
}

QByteArray CEibdMsg::getResponse( bool * p_pHasResponse )
{
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
    double dVal = p_grData.toDouble( & bOk );

    if ( isNatural( dVal ) == true )
    {
        int nVal = (int) dVal;
        if ( nVal <= 100 )
        {
            char szData = nVal;// QString::number( nVal ).toAscii();
            szData = szData | 0x80;
            grMsg.append( szData ); // index 9

            return grMsg;
        }
        else
        {
            qDebug() << "Can only forward positive natural numbers < 100, not" << p_grData;
        }
    }

    return QByteArray();

//    grMsg.append( char( 0x80 ) ); // index 9
//    grMsg.append( dVal );

//    quint8 nSize = grMsg.size() - 2;

//    grMsg[ 1 ] = nSize; // index 1

//    return grMsg;
}

bool CEibdMsg::isNatural(const double & p_dNumber)
{
    QStringList grStringList;

    QString sNumer = QString::number( p_dNumber );

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
    QString sResult;
    QString sBuffer;
    for ( int i = 0; i < p_grByteArray.length(); i++ )
    {
        sBuffer = QString::number( uchar( p_grByteArray.at( i ) ), 16 );
        if ( sBuffer.length() == 1 )
        {
            sBuffer = "0" + sBuffer;
        }

        sResult += sBuffer + " ";
    }

    return sResult;
}