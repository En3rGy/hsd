#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"
#include "QsLog.h"
#include "model.h"
#include <QTranslator>
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
    qDebug() << QObject::tr( "hsd provides the eibd TCP/IP interface to access the KNX bus via the GIRA Homeserver KO-Gateway." )
             << "\n\n" << QObject::tr( "Configure settings in [hsd]/etc/hsd.ini.").toLatin1()
             << "\n"   << QObject::tr( "Log file is written to [hsd]/var/hsd.log").toLatin1()
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Settings file options:")
             << "\n" << CModel::g_sKey_HsdPort   << "\t" << QObject::tr( "Port, where hsd listens for eibd messages.").toLatin1()
             << "\n" << CModel::g_sKey_HSGwPort  << "\t" << QObject::tr( "GIRA Homeserver KO-Gateway port").toLatin1()
             << "\n" << CModel::g_sKey_HSIP      << "\t" << QObject::tr( "IP address of GIRA Homeserver").toLatin1()
             << "\n" << CModel::g_sKey_HSWebPort << "\t" << QObject::tr( "Port of GIRA Homeserver web server").toLatin1()
             << "\n" << CModel::g_sKey_LogLevel  << "\t" << QObject::tr( "Hsd log level (see below)").toLatin1()
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Command line options:" )
             << "\n\nhsd [option]"
             << "\n"   << "-?\t" << QObject::tr( "This help page").toLatin1()
             << "\n"   << "-lx\t" << QObject::tr( "Setting the log level x=0 (TraceLevel),").toLatin1()
             << "\n\t" << QObject::tr( "=1 (DebugLevel),").toLatin1()
             << "\n\t" << QObject::tr( "=2 (InfoLevel),").toLatin1()
             << "\n\t" << QObject::tr( "=3 (WarnLevel),").toLatin1()
             << "\n\t" << QObject::tr( "=4 (ErrorLevel, default),").toLatin1()
             << "\n\t" << QObject::tr( "=5 (FatalLevel)" ).toLatin1()
             << "\n"   << "-v\t" << QObject::tr( "Printing program version" ).toLatin1()
             << "\n"   << "-c [a]\t" << QObject::tr( "Printing adress convertion, e.g. hsd -c 4200 returns:" ).toLatin1()
             << "\n\t" << "\"KNX: 8/2/0, HEX: 4200, HS: 16896\""
             << "\n"   << "-E\t" << QObject::tr( "Exit running hsd instances." ).toLatin1();
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

        CHsd grHsd;

        if ( argc >= 2 )
        {
            for ( int i = 0; i < argc; i++ )
            {
                if ( strcmp( argv[ i ], "-?" ) == 0 )
                {
                    printHelpPage();
                    return EXIT_SUCCESS;
                }
                else if ( strcmp( argv[ i ], "-l0" ) == 0 )
                {
                    grHsd.setLogLevel( 0 );
                }
                else if ( strcmp( argv[ i ], "-l1" ) == 0 )
                {
                    grHsd.setLogLevel( 1 );
                }
                else if ( strcmp( argv[ i ], "-l2" ) == 0 )
                {
                    grHsd.setLogLevel( 2 );
                }
                else if ( strcmp( argv[ i ], "-l3" ) == 0 )
                {
                    grHsd.setLogLevel( 3 );
                }
                else if ( strcmp( argv[ i ], "-l4" ) == 0 )
                {
                    grHsd.setLogLevel( 4 );
                }
                else if ( strcmp( argv[ i ], "-l5" ) == 0)
                {
                    grHsd.setLogLevel( 5 );
                }
                else if ( strcmp( argv[ i ], "-c") == 0 )
                {
                    if ( argc > ( i + 1) )
                    {
                        QString sAddr( argv[ i + 1 ] );

                        grHsd.callHsXML();


                        CGroupAddress grAddr;
                        grAddr.setAddress( sAddr );
                        qDebug() << grAddr.toString();
                        return EXIT_SUCCESS;
                    }
                    else
                    {
                        printHelpPage();
                        return EXIT_SUCCESS;
                    }
                }
                else if ( strcmp( argv[ i ], "-v") == 0 )
                {
                    qDebug() << QCoreApplication::applicationName() << QCoreApplication::applicationVersion();
                    return EXIT_SUCCESS;
                }

                else if ( strcmp( argv[ i ], "-E") == 0 )
                {
                    grHsd.stopService();
                    return EXIT_SUCCESS;
                }

            }
        }
        else
        {
        }

        grHsd.startService();

        return a.exec();
    }
    catch( ... )
    {
        QLOG_FATAL() << QObject::tr( "Uncought Exception" ).toStdString().c_str() << Q_FUNC_INFO;
    }
}


