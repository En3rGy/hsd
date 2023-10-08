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
  * <p>Incomming message from HS:<br>
  * <ul>
  * <li>CTcpClient::slot_startRead</li>
  * <li><li>CGroupAddress</li></li>
  * <li>CTcpServer::slot_sendToEibdClient</li>
  * <li><li>CEibdMsg</li></li>
  * <li><li><li>CGroupAddress</li></li></li>
  * </ul>
  */


/// @todo move args to model as static string
///
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
             << "\n"   << "-l x\t" << QObject::tr( "Setting the log level x=0 (TraceLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=1 (DebugLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=2 (InfoLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=3 (WarnLevel),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=4 (ErrorLevel, default),").toStdString().c_str()
             << "\n\t" << QObject::tr( "=5 (FatalLevel)" ).toStdString().c_str()
             << "\n"   << "-rl x\t" << QObject::tr( "Setting the log level remotely for a running hsd service").toStdString().c_str()
             << "\n"   << "-v\t" << QObject::tr( "Printing program version" ).toStdString().c_str()
             << "\n"   << "-c [a]\t" << QObject::tr( "Printing adress convertion, e.g. hsd -c 4200 returns:" ).toStdString().c_str()
             << "\n\t" << "\"KNX: 8/2/0, HEX: 4200, HS: 16896\""
             << "\n\t" << QObject::tr( "HS adress representation requires to be given with at least 5 digits, e.g. by a leading zero: 01234" )
             << "\n"   << "-E\t" << QObject::tr( "Exit running hsd instances." ).toStdString().c_str();
}

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setOrganizationName( "PImp" );
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.6.0" );

        QString sLocale = QLocale::system().name();

        QTranslator grTranslator;
        grTranslator.load( QString(":hsd_") + sLocale);
        a.installTranslator( & grTranslator);

        int nLogLevel = -1;
        QList< QString > grArgsList;

        if ( argc >= 2 )
        {
            for ( int i = 0; i < argc; i++ )
            {
                grArgsList.push_back( argv[ i ] );
            }
        }

        if ( grArgsList.contains( "-?" ) == true )
        {
            printHelpPage();
            return EXIT_SUCCESS;
        }

        if ( grArgsList.contains( "-v") == true )
        {
            grArgsList.removeAt( grArgsList.indexOf( "-v" ) );
            qDebug() << QCoreApplication::applicationName().toStdString().c_str()
                     << QCoreApplication::applicationVersion().toStdString().c_str();
            return EXIT_SUCCESS;
        }

        if ( grArgsList.contains( "-c" ) == true )
        {
            int nIndex = grArgsList.indexOf( "-c" );

            if ( grArgsList.size() > nIndex + 1 )
            {
                QString sAddr( grArgsList.at( nIndex + 1 ) );

                CGroupAddress grAddr;
                grAddr.setAddress( sAddr );
                qDebug() << grAddr.toString().toStdString().c_str();
                return EXIT_SUCCESS;
            }
        }

        if ( grArgsList.contains( "-E" ) == true )
        {
            CHsd::stopService();
            return EXIT_SUCCESS;
        }


        if ( grArgsList.contains( "-l" ) == true )
        {
            int nIndex = grArgsList.indexOf( "-l" );

            if ( grArgsList.size() > nIndex + 1 )
            {
                bool bOk;
                int  nArgLevel = grArgsList.at( nIndex + 1 ).toInt( & bOk );

                if ( bOk == true )
                {
                    if ( nArgLevel >= 0 && nLogLevel <= 5 )
                    {
                        nLogLevel = nArgLevel;
                        grArgsList.removeAt( nIndex + 1 );
                        grArgsList.removeAt( nIndex );
                    }
                }
                else
                {
                    qDebug() << QObject::tr( "Received set log level command line argument but the passed log level is not correct:" ).toStdString().c_str()
                             << grArgsList.at( nIndex + 1);
                }
            }
            else
            {
                qDebug() << QObject::tr( "Received set log level command line argument (\"-l\") but the level was missing" ).toStdString().c_str()
                         << grArgsList.at( nIndex + 1);
            }
        }

        if ( grArgsList.contains( "-rl" ) == true )
        {
            int nIndex = grArgsList.indexOf( "-rl" );

            if ( grArgsList.size() > nIndex + 1 )
            {
                bool bOk;
                int  nArgLevel = grArgsList.at( nIndex + 1 ).toInt( & bOk );

                if ( bOk == true )
                {
                    if ( nArgLevel >= 0 && nLogLevel <= 5 )
                    {
                        CHsd::setRemoteLogLevel( nArgLevel );
                        return EXIT_SUCCESS;
                    }
                }
            }
        }

        if ( grArgsList.empty() == false )
        {
            printHelpPage();
            return EXIT_SUCCESS;
        }

        CHsd grHsd;

        if ( nLogLevel >= 0 )
        {
            grHsd.setLogLevel( nLogLevel );
        }

        QLOG_INFO() << a.applicationName().toStdString().c_str()
                    << a.applicationVersion().toStdString().c_str()
                    << a.applicationFilePath().toStdString().c_str();

        grHsd.startService();

        return a.exec();
    } // try

    catch( ... )
    {
        QLOG_FATAL() << QObject::tr( "Uncought Exception" ).toStdString().c_str() << Q_FUNC_INFO;
    }
}
