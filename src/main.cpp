#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"
#include "QsLog.h"
#include "model.h"
#include <QTranslator>

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
        a.setApplicationVersion( "0.4.1" );

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
                    qDebug() << QObject::tr( "hsd provides the eibd TCP/IP interface to access the KNX bus via the GIRA Homeserver KO-Gateway.\n" )
                             << QObject::tr( "\nConfigure settings in [hsd]/etc/hsd.ini.")
                             << QObject::tr( "\nLog file is written to [hsd]/var/hsd.log\n")
                             << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                             << QObject::tr( "\nSettings file options:")
                             << "\n" << CModel::g_sKey_HsdPort << QObject::tr( "\tPort, where hsd listens for eibd messages.")
                             << "\n" << CModel::g_sKey_HSGwPort << QObject::tr( "\tGIRA Homeserver KO-Gateway port")
                             << "\n" << CModel::g_sKey_HSIP << QObject::tr( "\tIP address of GIRA Homeserver")
                             << "\n" << CModel::g_sKey_HSWebPort << QObject::tr( "\tPort of GIRA Homeserver web server")
                             << "\n" << CModel::g_sKey_LogLevel << QObject::tr( "\tHsd log level (see below)")
                             << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                             << QObject::tr( "\nCommand line options:\n")
                             << "\nhsd [option]"
                             << QObject::tr( "\n--?\tThis help page")
                             << QObject::tr( "\n-lx\tx=0 (TraceLevel),")
                             << QObject::tr( "\n\t =1 (DebugLevel),")
                             << QObject::tr( "\n\t =2 (InfoLevel),")
                             << QObject::tr( "\n\t =3 (WarnLevel),")
                             << QObject::tr( "\n\t =4 (ErrorLevel, default),")
                             << QObject::tr( "\n\t =5 (FatalLevel)" );

                    return EXIT_SUCCESS;
                }
                else if ( strcmp( argv[ i ], "-l0" ) == 0 )
                {
                    grHsd.setLogLevel( 0 );
                    break;
                }
                else if ( strcmp( argv[ i ], "-l1" ) == 0 )
                {
                    grHsd.setLogLevel( 1 );
                    break;
                }
                else if ( strcmp( argv[ i ], "-l2" ) == 0 )
                {
                    grHsd.setLogLevel( 2 );
                    break;
                }
                else if ( strcmp( argv[ i ], "-l3" ) == 0 )
                {
                    grHsd.setLogLevel( 3 );
                    break;
                }
                else if ( strcmp( argv[ i ], "-l4" ) == 0 )
                {
                    grHsd.setLogLevel( 4 );
                    break;
                }
                else if ( strcmp( argv[ i ], "-l5" ) == 0)
                {
                    grHsd.setLogLevel( 5 );
                    break;
                }
            }
        }
        else
        {
            qDebug() << QObject::tr( "Program starting. Use \"hsd --?\" for help" );
        }

        grHsd.startService();

        return a.exec();
    }
    catch( ... )
    {
        //QLOG_FATAL() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
