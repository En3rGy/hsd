#ifndef MODEL_H
#define MODEL_H

#include <QString>

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
    CModel( void );

    static const QString g_sKey_HSIP;
    static const QString g_sKey_HSWebPort;
    static const QString g_sKey_HSGwPort; ///< Port of HS KO-Gateway
    static const QString g_sKey_HsdPort;  ///< Port of HSD IP interface
    static const QString g_sSettingsPath;

    static const uchar   g_uzEibGroupPacket [2];
    static const uchar   g_uzEibOpenGroupCon [5];
    static const uchar   g_uzEibOn;
    static const uchar   g_uzEibOff;
    static const uchar   g_uzEibAck [2];    
};

#endif // MODEL_H
