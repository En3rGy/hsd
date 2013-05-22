#ifndef EIBDMSG_H
#define EIBDMSG_H

#include <QByteArray>
#include <QString>
#include <QVariant>

/** @class CEibdMsg
  * @brief Handler of eibd messages
  * @author T. Paul
  * @date 2013
  *
  * Helper class for dealing with incomming eibd messages. Providing replys for incomming messages.
  */
class CEibdMsg
{
public:
    enum enuMsgType
    {
          enuMsgType_connect         ///< Request for connection
        , enuMsgType_openGroupSocket ///< Request for opening a group socket
        , enuMsgType_simpleWrite     ///< Request to write a value to an EIB / KNX address
        , enuMsgType_msgSize         ///< Message to propagate the size of the next message
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

    /// @brief Converts a byte array to a string showing the hex values
    /// @param[in] p_grByteArray byte array to print
    /// @return Hex code of byte array, e.g. "03 ff 00 8a"
    static QString printASCII(const QByteArray &p_grByteArray);

    /**
     * @brief getMessage returns a simplewrite for setting a value on an EIB/KNX address
     * @param p_sSrcAddr Address of sender, e.g. 1.4.15
     * @param p_sDestAddr Address of destination, e.g. 5/5/35
     * @param p_grData Data to write to the destination address
     * @return Message as byte array
     */
    static QByteArray getMessage( const QString & p_sSrcAddr, const QString & p_sDestAddr, const QVariant & p_grData );

    /**
     * @brief Returns the submitted size of the data package.
     * @return Size of data package.
     */
    int getMsgDataSize(  ) const;

protected:

    /** @brief Checks if the passed double is a natural number
      * @param p_dNumber Number to check
      * @return True if the parameter is a natural number: 1.0 -> true, 1.1 -> false
      */
    static bool isNatural( const double & p_dNumber );

    enuMsgType m_eMsgType;
    QString    m_sSrcAddr; ///< EIB address of message sender.
    QString    m_sDstAddr; ///< EIB address of message receiver.

    QVariant   m_grValue;  ///< Value of EIB message.

    int        m_nMsgSize; ///< Size of data part of message
};

#endif // EIBDMSG_H
