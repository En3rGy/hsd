#include "model.h"
#include <QSettings>


const QString CModel::g_sKey_HSIP       = "HSIP";
const QString CModel::g_sKey_HSWebPort  = "HSWebPort";
const QString CModel::g_sKey_HSGwPort   = "HSGwPort";
const QString CModel::g_sKey_HsdPort    = "HsdPort";

const uchar   CModel::g_uzEibGroupPacket [2]  = { 0x00, 0x27 };
const uchar   CModel::g_uzEibOpenGroupCon [5] = { 0x00, 0x26, 0x00, 0x00,0x00 };
const uchar   CModel::g_uzEibOn               = 0x81;
const uchar   CModel::g_uzEibOff              = 0x80;
const uchar   CModel::g_uzEibAck [2]          = { 0x00, 0x05 };


CModel::CModel(QObject *parent) :
    QObject(parent)
{
}
