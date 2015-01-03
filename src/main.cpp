#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"
#include "QsLog.h"
#include "model.h"
#include <QTranslator>
#include <QList>
#include "groupaddress.h"
#include "eibdmsg.h"
#include "model.h"

/** @mainpage
  * <p>The programm provides parts of the eibd TCP/IP interface to communicate
  * with the GIRA Homeserver KO-Gateway.</p>
  * <p>CTcpClient manages communication with the GIRA Homeserver. CTcpServer
  * provides the eibd TCP/IP interface.</p>
  */


void printHelpPage( void )
{
    qDebug() << QObject::tr( "hsd provides the eibd TCP/IP interface to access the KNX bus via the GIRA Homeserver KO-Gateway." ).toStdString().c_str()
             << "\n\n" << QObject::tr( "Configure settings in [hsd]/etc/hsd.ini.").toStdString().c_str()
             << "\n"   << QObject::tr( "Log file is written to [hsd]/var/hsd.log").toStdString().c_str()
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Settings file options:").toStdString().c_str()
             << "\n" << CModel::g_sKey_HsdPort   << "\t" << QObject::tr( "Port, where hsd listens for eibd messages.").toStdString().c_str()
             << "\n" << CModel::g_sKey_HSGwPort  << "\t" << QObject::tr( "GIRA Homeserver KO-Gateway port").toStdString().c_str()
             << "\n" << CModel::g_sKey_HSIP      << "\t" << QObject::tr( "IP address of GIRA Homeserver").toStdString().c_str()
             << "\n" << CModel::g_sKey_HSWebPort << "\t" << QObject::tr( "Port of GIRA Homeserver web server").toStdString().c_str()
             << "\n" << CModel::g_sKey_LogLevel  << "\t" << QObject::tr( "Hsd log level (see below)").toStdString().c_str()
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Command line options:" ).toStdString().c_str()
             << "\n\nhsd [option]"
             << "\n"   << "-?\t" << QObject::tr( "This help page").toStdString().c_str()
             << "\n"   << "-lx\t" << QObject::tr( "Setting the log level x=0 (TraceLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=1 (DebugLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=2 (InfoLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=3 (WarnLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=4 (ErrorLevel, default),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=5 (FatalLevel)" ).toStdString().c_str()
             << "\n"   << "-v\t" << QObject::tr( "Printing program version" ).toStdString().c_str()
             << "\n"   << "-c [a]\t" << QObject::tr( "Printing adress convertion, e.g. hsd -c 4200 returns:" ).toStdString().c_str()
             << "\n\t" << "\"KNX: 8/2/0, HEX: 4200, HS: 16896\""
             << "\n"   << "-E\t" << QObject::tr( "Exit running hsd instances." ).toStdString().c_str();
}

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setOrganizationName( "PImp" );
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.4.9" );

        QString sLocale = QLocale::system().name();

        QTranslator grTranslator;
        grTranslator.load( QString(":hsd_") + sLocale);
        a.installTranslator( & grTranslator);

        int nLogLevel = -1;
        QList< QString > grArgsList;
        bool bValidArg = true;

        if ( argc >= 2 )
        {
            bValidArg = false;
            for ( int i = 0; i < argc; i++ )
            {
                grArgsList.push_back( argv[ i ] );
            }
        }

        if ( grArgsList.contains( "-?" ) == true )
        {
            bValidArg = true;
            printHelpPage();
            return EXIT_SUCCESS;
        }
        if ( grArgsList.contains( "-l0" ) == true )
        {
            bValidArg = true;
            nLogLevel = 0;
        }
        else if ( grArgsList.contains( "-l1" )  == true )
        {
            bValidArg = true;
            nLogLevel = 1;
        }
        else if ( grArgsList.contains( "-l2" ) == true )
        {
            bValidArg = true;
            nLogLevel = 2;
        }
        else if ( grArgsList.contains( "-l3" ) == true )
        {
            bValidArg = true;
            nLogLevel = 3;
        }
        else if ( grArgsList.contains( "-l4" ) == true )
        {
            bValidArg = true;
            nLogLevel = 4;
        }
        else if ( grArgsList.contains( "-l5" ) == true)
        {
            bValidArg = true;
            nLogLevel = 5;
        }

        if ( grArgsList.contains( "-v") == true )
        {
            bValidArg = true;
            qDebug() << QCoreApplication::applicationName().toStdString().c_str()
                     << QCoreApplication::applicationVersion().toStdString().c_str();
            return EXIT_SUCCESS;
        }

        if ( grArgsList.contains( "-c" ) == true )
        {
            int nIndex = grArgsList.indexOf( "-c" );

            if ( grArgsList.size() > nIndex + 1 )
            {
                bValidArg = true;
                QString sAddr( grArgsList.at( nIndex + 1 ) );

                // grHsd.callHsXML();

                CGroupAddress grAddr;
                grAddr.setAddress( sAddr );
                qDebug() << grAddr.toString().toStdString().c_str();
                return EXIT_SUCCESS;
            }
        }

        CHsd grHsd;

        if ( bValidArg == false )
        {
            printHelpPage();
            return EXIT_SUCCESS;
        }

        if ( grArgsList.contains( "-E" ) == true )
        {
            bValidArg = true;
            grHsd.stopService();
            return EXIT_SUCCESS;
        }

        if ( nLogLevel >= 0 )
        {
            grHsd.setLogLevel( nLogLevel );
        }

        grHsd.startService();

        return a.exec();
    } // try

    catch( ... )
    {
        QLOG_FATAL() << QObject::tr( "Uncought Exception" ).toStdString().c_str() << Q_FUNC_INFO;
    }
}
