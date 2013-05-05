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
        a.setApplicationVersion( "0.0.3" );

        QSettings::setPath( QSettings::IniFormat, QSettings::SystemScope, "../etc/hsd.ini" );


        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
