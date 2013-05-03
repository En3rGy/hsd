#ifndef KOXML_H
#define KOXML_H

#include <QByteArray>
#include <QMap>
#include <QString>

class CKoXml
{
public:
    static CKoXml * getInstance();

    void setXml( QByteArray & p_grKoXml );
    QString getGaName( const QString & p_sGA );

    struct cobject
    {
        QString sId;
        QString sUsed;
        QString sType;
        QString sPath;
        QString sFmt;
        QString sFmtex;
        QString sName;
        QString sRem;
        QString sInit;
        QString sMin;
        QString sMax;
        QString sStep;
        QString sList;
        QString sGa; ///< Group adress, e.g. 1/1/15
        QString sGanum;
        QString sCogws;
        QString sCogwr;
        QString sScan;
        QString sSbc;
        QString sRead;
        QString sTransmit;
    };

protected:
    CKoXml();

    QString copy(       QByteArray & p_grData,
                  const QString    & p_sTag,
                  const int        & p_nStartIndex );

    static CKoXml * m_pInstance;
    QMap< QString, cobject > m_ssGA2NameMap;

    friend class CKoXmlGC;
};

class CKoXmlGC
{
public:
    virtual ~CKoXmlGC();
};

#endif // KOXML_H
