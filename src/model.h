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
    static const QString g_sKey_HSGwPort; ///< Port of KO-Gateway
    
signals:
    
public slots:
    
};

#endif // MODEL_H
