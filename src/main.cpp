#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.4.0" );

        QSettings::setPath( QSettings::IniFormat, QSettings::SystemScope, "../etc/hsd.ini" );

        CHsd grHsd;
        grHsd.startService();

        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
