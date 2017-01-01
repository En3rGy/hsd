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
          enuMsgType_connect          ///< Request for connection
        , enuMsgType_EIB_OPEN_GROUPCON  ///< Request for opening a group socket
        , enuMsgType_EIB_GROUP_PACKET      ///< Request to write a value to an EIB / KNX address
        , enuMsgType_msgSize          ///< Message to propagate the size of the next message
        , enuMsgType_undef            ///< Default error value
        , enuMsgType_EIB_OPEN_T_GROUP
        , enuMsgType_EIB_RESET_CONNECTION
        , enuMsgType_EIB_APDU_PACKET

    };

    enum enuAPDUType {
        enuAPDUType_undef
        , enuAPDUType_A_GroupValue_Write_PDU
        , enuAPDUType_A_GroupValue_Read_PDU
    };

    /// @brief default cto.
    CEibdMsg();

    /// @brief Constructor
    /// @param p_grByteArray message
    CEibdMsg( const QByteArray & p_grByteArray );

    /**
     * @brief Set for an incomming message via the Eib Interface
     * @param p_grByteArray
     */
    void setEibdMsg( const QByteArray & p_grByteArray );

    /// @brief Type of message
    /// @return enuMsgType
    const enuMsgType & getType( void ) const;

    /**
     * @brief getAPDUType
     * @return enuAPDUType
     */
    const enuAPDUType & getAPDUType( void ) const;

    /// @brief Returns the destination adress of the message
    /// @return QString with KNX adress, e.g. "2/4/15"
    const QString & getDestAddressKnx( void ) const;

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

    void setValue( const float & p_fVal );

protected:

    /** @brief Checks if the passed double is a natural number
      * @param p_dNumber Number to check
      * @return True if the parameter is a natural number: 1.0 -> true, 1.1 -> false
      */
    static bool isNatural(const float &p_fNumber );

    /**
     * @brief Perfoms a hex compare of the QByteArray and char *.
     *
     * One has to take care that length of p_grByteArray and p_grCharArr have at least a length on p_nLength!
     *
     * @param p_grByteArray
     * @param p_grCharArr
     * @param p_nLength
     * @return
     */
    bool equals( const QByteArray & p_grByteArray, const uchar * p_grCharArr, const int & p_nLength );

    /**
     * @brief Interprets an Eib1 Dtp (1 bit) and stores the result
     * @param p_szData
     */
    void setEib1( const uchar & p_szData );

    void setDTP5( const QByteArray &p_grData );

    /**
     * @brief Interprets an 2 byte DTP9.001 message and stores the result
     * @param p_grData 2 byte DTP9.001 message
     */
    void setDTP9_001( const QByteArray &p_grData );

    void setEibAddress( const QByteArray &p_grData );

    enuMsgType  m_eMsgType;
    enuAPDUType m_eAPDUType;
    QString     m_sSrcAddr; ///< EIB address of message sender.
    QString     m_sDstAddrKnx; ///< EIB address of message receiver.

    QVariant    m_grValue;  ///< Value of EIB message.

    int         m_nMsgSize; ///< Size of data part of message
};

#endif // EIBDMSG_H
