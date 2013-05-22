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
             << "\n\n" << QObject::tr( "Configure settings in [hsd]/etc/hsd.ini.")
             << "\n"   << QObject::tr( "Log file is written to [hsd]/var/hsd.log")
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Settings file options:")
             << "\n" << CModel::g_sKey_HsdPort   << "\t" << QObject::tr( "Port, where hsd listens for eibd messages.")
             << "\n" << CModel::g_sKey_HSGwPort  << "\t" << QObject::tr( "GIRA Homeserver KO-Gateway port")
             << "\n" << CModel::g_sKey_HSIP      << "\t" << QObject::tr( "IP address of GIRA Homeserver")
             << "\n" << CModel::g_sKey_HSWebPort << "\t" << QObject::tr( "Port of GIRA Homeserver web server")
             << "\n" << CModel::g_sKey_LogLevel  << "\t" << QObject::tr( "Hsd log level (see below)")
             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
             << "\n" << QObject::tr( "Command line options:" )
             << "\n\nhsd [option]"
             << "\n"   << "--?\t" << QObject::tr( "This help page")
             << "\n"   << "-lx\t" << QObject::tr( "Setting the log level x=0 (TraceLevel),")
             << "\n\t" << QObject::tr( "=1 (DebugLevel),")
             << "\n\t" << QObject::tr( "=2 (InfoLevel),")
             << "\n\t" << QObject::tr( "=3 (WarnLevel),")
             << "\n\t" << QObject::tr( "=4 (ErrorLevel, default),")
             << "\n\t" << QObject::tr( "=5 (FatalLevel)" )
             << "\n"   << "-v\t" << QObject::tr( "Printing program version" )
             << "\n"   << "-c [a]\t" << QObject::tr( "Printing adress convertion, e.g. hsd -c 4200 returns:" )
             << "\n\t" << "\"KNX: 8/2/0, HEX: 4200, HS: 16896\""
             << "\n"   << "-E\t" << QObject::tr( "Exit running hsd instances." );
}

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setOrganizationName( "PImp" );
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.4.6" );

        QString sLocale = QLocale::system().name();

        QTranslator grTranslator;
        grTranslator.load( QString(":hsd_") + sLocale);
        a.installTranslator( & grTranslator);

        CHsd grHsd;

        if ( argc >= 2 )
        {
            for ( int i = 0; i < argc; i++ )
            {
                if ( strcmp( argv[ i ], "--?" ) == 0 )
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
            qDebug() << QObject::tr( "Program starting. Use \"hsd --?\" for help." )
                     << "\n"
                     << QObject::tr( "Press CTRL+C to quit." )
                     << "\n\n";
        }

        grHsd.startService();

        return a.exec();
    }
    catch( ... )
    {
        QLOG_FATAL() << "Uncought Exception" << Q_FUNC_INFO;
    }
}


