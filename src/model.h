#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QSettings>

/** @class CModel
  * @brief Definition of constants.
  * @author T. Paul
  * @date 2013
  *
  * Not realy a model.
  */
class CModel
{
public:
    static CModel * getInstance( void );
    virtual ~CModel( void );

    static const QString g_sKey_HSIP;
    static const QString g_sKey_HSWebPort;
    static const QString g_sKey_HSGwPort; ///< Settings key for port of HS KO-Gateway
    static const QString g_sKey_HsdPort;  ///< Settings key for port of HSD IP interface
    static const QString g_sKey_HSGwPassword;  ///< Setings key for password for HS KO-Gateway
    static const QString g_sSettingsPath;
    static const QString g_sKey_LogLevel;
    static const QString g_sKey_LogPerDate;
    static const QString g_sKey_PauseTilHSReconnect; ///< Settings key for timeout until re-connection try to HS.
    static const QString g_sExitMessage;
    static const QString g_sLogLevelMessage;

    static const char   g_uzEIB_OPEN_T_GROUP [2];
    static const char   g_uzEIB_APDU_PACKET [2];
    static const char   g_uzEIB_OPEN_GROUPCON [5];
    static const char   g_uzEIB_GROUP_PACKET [2];
    static const char   g_uzEibOn;
    static const char   g_uzEibOff;
    static const char   g_uzEibAck [2];

    QVariant getValue( const QString & p_sKey, const QVariant & p_grDefaultValue = QVariant() );
    void     setValue( const QString & p_sKey, const QVariant & p_grValue );

    QMap< QString, QVariant > m_grGAState;

    static void logCSV( const QString & p_sTo, const QString & p_sFrom, const QString & p_sGA, const QString & p_sVal, const QString & p_sMsg, const QString & p_sRawMsg );

protected:
    CModel( void );
	CModel( const CModel & p_grModel );

    static CModel    * m_pInstance;
    QSettings * m_pSettings;

    friend class CModelGC;
};

class CModelGC
{
public:
    virtual ~CModelGC( void );
};

#endif // MODEL_H
