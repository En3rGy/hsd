#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"

/** @mainpage
  * <p>The programm provides parts of the eibd TCP/IP interface to communicate
  * with the GIRA Homeserver KO-Gateway.</p>
  * <p>CTcpClient manages communication with the GIRA Homeserver. CTcpServer
  * provides the eibd TCP/IP interface.</p>
  */

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setOrganizationName( "PImp" );
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.4.0" );

        CHsd grHsd;
        grHsd.startService();

        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
