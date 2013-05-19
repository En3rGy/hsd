#ifndef GROUPADDRESS_H
#define GROUPADDRESS_H

#include <QString>

/** @class CGroupAddress
  * @brief Helper class for dealing and convertion between different adress representations.
  * @author T. Paul
  * @date 2013
  */
class CGroupAddress
{
public:
    CGroupAddress();

    /** @breif Returns the hex representation of an EIB/KNX address
      *
      * Conversion:<br>
      * Having a 2 byte sequence: aaaa abbb cccc cccc<br>
      * Turns to an EIB / KNX address like a/b/c<br>
      * 0001 1001 0000 0010 equals 3/1/2
      *
      * @return QByteArray containing the hex representation.
      */
    QByteArray toHex( void ) const;

    /** @brief Returning a readable EIB/KNX address, e.g. 3/5/18
      * @param p_sSeparator defining the seperator, e.g. "/" or "." resulting in 3/5/15 or 3.5.15
      * @return QString containing the address
      */
    QString toKNXString( const QString & p_sSeparator = "/" ) const;

    /** @brief Returning the HS intferace representation of an EIB/KNX address
      *
      * Having the adrees a/b/c the conversion formular is:<br>
      * a * 2048 + b * 256 + c
      *
      * @return HS representation of address.
      */
    int toHSRepresentation( void ) const;

    /** @brief Setter for hex address representation
      *
      * The hex representation is two byte long and follows this scheme: <br>
      * aaaa abbb cccc cccc for a/b/c.
      *
      * @param p_grHexAddr
      */
    void setHex( const QByteArray & p_grHexAddr );

    /** @brief Setter for string address representation, e.g. 3/5/18
      * @param p_sStrAddr Address as string, "/" and "." are allowed seperators.
      */
    void setKNXString( const QString & p_sStrAddr );

    /** @brief Setter for HS address representation.
      * @param p_nHSAddr HS address given by a * 2048 + b * 256 + c for a/b/c.
      */
    void setHS( const int & p_nHSAddr );

    /** @brief Setter for string representation of adress in EIB, HEX or HS form
      * @param p_sAddress
      */
    void setAddress( const QString & p_sAddress );

    /** @brief Check if the EIB/KNX address is valid.
      *
      * Basically, the number ranges are checked for a/b/c with<br>
      * 0 <= a <= 15, 0 <= b <= 15, 0 <= c <= 256
      * @return bool true if address is within ranges, otherwise false.
      */
    bool isValid( void ) const;

    QString toString( void ) const;

protected:
    uint m_unMainAddr; ///< a in a/b/c
    uint m_unMiddAddr; ///< b in a/b/c
    uint m_unLowAddr;  ///< c in a/b/c
};

#endif // GROUPADDRESS_H
