#ifndef GROUPADDRESS_H
#define GROUPADDRESS_H

#include <QString>

class CGroupAddress
{
public:
    CGroupAddress();

    QByteArray toHex( void ) const;
    QString    toKNXString( const QString & p_sSeparator = "/" ) const;
    int        toHSRepresentation( void ) const;

    void setHex( const QByteArray & p_grHexAddr );
    void setKNXString( const QString & p_sStrAddr );
    void setHS( const int & p_nHSAddr );

    bool isValid( void ) const;

protected:
    uint m_unMainAddr;
    uint m_unMiddAddr;
    uint m_unLowAddr;
};

#endif // GROUPADDRESS_H
