#include "model.h"
#include <QSettings>


const QString CModel::g_sKey_HSIP       = "HSIP";
const QString CModel::g_sKey_HSWebPort  = "HSWebPort";
const QString CModel::g_sKey_HSGwPort   = "HSGwPort";
const QString CModel::g_sKey_HsdPort    = "HsdPort";


CModel::CModel(QObject *parent) :
    QObject(parent)
{
}