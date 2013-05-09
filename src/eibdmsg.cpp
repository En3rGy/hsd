#include <QDebug>

#include "eibdmsg.h"
#include "model.h"
#include <QStringList>

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

    uchar uszSize [ 2 ];
    uszSize[ 0 ] = p_grByteArray.at( 0 );
    uszSize[ 1 ] = p_grByteArray.at( 1 );

    int nSize = ( int ) uszSize;

    if ( p_grByteArray.size() - 2 == nSize )
    {
        grMsg.append( p_grByteArray.mid( 2, p_grByteArray.size() - 2 ) );
        qDebug() << "Received message length bytes" << nSize;
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
                m_sDstAddr = hex2eib( grEibAdr );

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

                if ( grData.size() == 2 )
                {
                    uchar szData = grMsg.at( 5 );
                    szData = szData & 0x7f; // 0x7f = 0111 1111
                    m_grValue.setValue( ( int ) szData );
                }
                else if ( grData.size() > 2 )
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
    // my ($head, $src, $dst,$data) = unpack("nnnxa*", $buf);
    // quint16 $head == 0x0027
    // quint16 $src == hex eib address
    // quint16 $dst == hex eib address
    // quint8 = 0 == '\0'
    // $data = 0x?0... | DATA, having ? = ACPI ('read' => 0, 'reply' => 1, 'write' => 2)

    QByteArray grMsg;

    grMsg.append( CModel::g_uzEibGroupPacket[ 0 ] ); // quint16
    grMsg.append( CModel::g_uzEibGroupPacket[ 1 ] );

    // litte end 0000 0000 0010 0111
    // big end   1110 0100 0000 0000

    /// @todo fhem crashes not beeing able to identify 0x0027???

    QByteArray grSrc =  eib2hex( p_sSrcAddr );
    QByteArray grDest = eib2hex( p_sDestAddr );

    grMsg.append( grSrc );
    grMsg.append( grDest );

    grMsg.append( '\0' );
    grMsg.append( 0x02 ); // ACPI write

    grMsg.append( p_grData.toByteArray() );   // value / data

    return grMsg;
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

QString CEibdMsg::hex2eib( QByteArray & p_grHexAddr )
{
    // eib 2 hex: The most significant Bit is always zero, followed by 4 bits for the
    // maingroup, 3 bits for the middlegroup and 8 bits for the subgrou

    // 0hhh hmmm

    if ( p_grHexAddr.length() != 2 )
    {
        qDebug() << "Wrong length" << Q_FUNC_INFO;
        return QString();
    }

    uchar szData [ 3 ];
    szData[0] = (uchar) p_grHexAddr.at(0);
    szData[1] = (uchar) p_grHexAddr.at(0);
    szData[2] = (uchar) p_grHexAddr.at(1);

    szData[0] = szData[0] << 1;
    QString sMainAddr   = QString::number( ( szData[0] >> 4 ) & 0xf );

    QString sMiddleAddr = QString::number( ( szData[1] & 0x7 ) );
    QString sUnderAddr  = QString::number( szData[2] );
    QString sSeparator = "/";

    QString sGA = sMainAddr + sSeparator + sMiddleAddr + sSeparator + sUnderAddr;
    return sGA;
}

QByteArray CEibdMsg::eib2hex(const QString &p_sEibAddr)
{
    // eib 2 hex: The most significant Bit is always zero, followed by 4 bits for the
    // maingroup, 3 bits for the middlegroup and 8 bits for the subgrou
    // 0hhh hmmm


    QByteArray  grRetArray;
    QStringList grAddrList;
    QString     sSep;

    if ( p_sEibAddr.contains( "/" ) )
    {
        sSep = "/";
    }
    else if ( p_sEibAddr.contains( "." ) )
    {
        sSep = ".";
    }
    else
    {
        return grRetArray;
    }

    grAddrList = p_sEibAddr.split( sSep, QString::SkipEmptyParts );

    if ( grAddrList.size() != 3 )
    {
        qDebug() << "ERROR: " << p_sEibAddr << "not of kind a/b/c. Aborting." << Q_FUNC_INFO;
        return grRetArray;
    }

    uchar szHexAddr [2];

    uchar szHAddr = grAddrList.at( 0 ).toUInt();
    uchar szMAddr = grAddrList.at( 1 ).toUInt();
    uchar szUAddr = grAddrList.at( 2 ).toUInt();

    // 0000 0000 == 0x00
    // 1111 1111 == 0xff

    szHAddr = szHAddr << 3;
    szHexAddr[ 0 ] = szHAddr | szMAddr;
    szHexAddr[ 1 ] = szUAddr;

    grRetArray.append( (char * ) & szHexAddr, sizeof( szHexAddr ) );

    return grRetArray;
}
