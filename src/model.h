#ifndef MODEL_H
#define MODEL_H

#include <QObject>

class CModel : public QObject
{
    Q_OBJECT
public:
    explicit CModel(QObject *parent = 0);

    static const QString g_sKey_HSIP;
    static const QString g_sKey_HSWebPort;
    static const QString g_sKey_HSGwPort; ///< Port of HS KO-Gateway
    static const QString g_sKey_HsdPort;  ///< Port of HSD IP interface

    static const uchar   g_uzEibGroupPacket [2];
    static const uchar   g_uzEibOpenGroupCon [5];
    static const uchar   g_uzEibOn;
    static const uchar   g_uzEibOff;
    static const uchar   g_uzEibAck [2];
    
signals:
    
public slots:
    
};

#endif // MODEL_H
