#ifndef EIBDMSG_H
#define EIBDMSG_H

#include <QByteArray>
#include <QString>
#include <QVariant>

class CEibdMsg
{
public:
    enum enuMsgType
    {
        enuMsgType_connect
        , enuMsgType_openGroupSocket
        , enuMsgType_simpleWrite
    };

    CEibdMsg();
    CEibdMsg( const QByteArray & p_grByteArray );

protected:
    QString printASCII(const QByteArray &p_grByteArray);
    QString hex2eib( QByteArray & p_grHexAddr );

    enuMsgType m_eMsgType;
    QString    m_sSrcAddr; ///< EIB address of message sender.
    QString    m_sDstAddr; ///< EIB address of message receiver.

    QVariant   m_grValue;  ///< Value of EIB message.
};

#endif // EIBDMSG_H
