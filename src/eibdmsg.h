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
          enuMsgType_connect         ///< Request for connection
        , enuMsgType_openGroupSocket ///< Request for opening a group socket
        , enuMsgType_simpleWrite     ///< Request to write a value to an EIB / KNX address
        , enuMsgType_undef           ///< Default error value
    };

    /// @brief default cto.
    CEibdMsg();

    /// @brief Constructor
    /// @param p_grByteArray message
    CEibdMsg( const QByteArray & p_grByteArray );

    /// @brief Type of message
    /// @return enuMsgType
    const enuMsgType & getType( void ) const;

    /// @brief Returns the destination adress of the message
    /// @return QString with KNX adress, e.g. "2/4/15"
    const QString & getDestAddress( void ) const;

    /// @brief Returns the value of the message
    /// @param[out] p_pHasValue True if value is available
    const QVariant & getValue( bool * p_pHasValue = NULL ) const;

    /// @brief Returns the response to the given message.
    /// @param[out] p_pHasResponse True if response is available
    /// @return An empty byte array in case of no neccessary response .
    QByteArray getResponse( bool * p_pHasResponse = NULL );

protected:
    /// @brief Converts a byte array to a string showing the hex values
    /// @param[in] p_grByteArray byte array to print
    /// @return Hex code of byte array, e.g. "03 ff 00 8a"
    QString printASCII(const QByteArray &p_grByteArray);

    /// @brief Converting a hex EIB/KNX address to string
    ///
    /// Conversion:<br>
    /// Having a 2 byte bit sequence: aaaa abbb cccc cccc<br>
    /// Turns to an EIB / KNX address like a/b/c<br>
    /// 0001 1001 0000 0010 equals 3/1/2
    ///
    /// @param p_grHexAddr 2 byte representation of a/b/c address.
    /// @return QString with KNX adress, e.g. "2/4/15"
    QString hex2eib( QByteArray & p_grHexAddr );

    enuMsgType m_eMsgType;
    QString    m_sSrcAddr; ///< EIB address of message sender.
    QString    m_sDstAddr; ///< EIB address of message receiver.

    QVariant   m_grValue;  ///< Value of EIB message.
};

#endif // EIBDMSG_H
