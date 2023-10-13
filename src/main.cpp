#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include "hsd.h"
#include "model.h"
#include <QTranslator>
#include <QtLogging>
#include <QtDebug>
#include <QList>
#include <iostream>
#include "groupaddress.h"
#include "model.h"
#include "qdir.h"
#include "qlocale.h"

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


void logMsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stdout, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stdout, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stdout, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}


/// @todo move args to model as static string
///
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
             << "\n"   << "-?\t" << QObject::tr( "This help page")
             << "\n"   << "-l x\t" << QObject::tr( "Setting the log level x=0 (TraceLevel),")
             << "\n\t" << QObject::tr( "=1 (DebugLevel),")
             << "\n\t" << QObject::tr( "=2 (InfoLevel),")
             << "\n\t" << QObject::tr( "=3 (WarnLevel),")
             << "\n\t" << QObject::tr( "=4 (ErrorLevel, default),")
             << "\n\t" << QObject::tr( "=5 (FatalLevel)" )
             << "\n"   << "-rl x\t" << QObject::tr( "Setting the log level remotely for a running hsd service")
             << "\n"   << "-v\t" << QObject::tr( "Printing program version" )
             << "\n"   << "-c [a]\t" << QObject::tr( "Printing adress convertion, e.g. hsd -c 4200 returns:" )
             << "\n\t" << "\"KNX: 8/2/0, HEX: 4200, HS: 16896\""
             << "\n\t" << QObject::tr( "HS adress representation requires to be given with at least 5 digits, e.g. by a leading zero: 01234" )
             << "\n"   << "-E\t" << QObject::tr( "Exit running hsd instances." );
}

int main(int argc, char *argv[])
{
    qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}");
    qInstallMessageHandler(logMsgOutput);

    try
    {
        QCoreApplication a(argc, argv);
        a.setOrganizationName( "PImp" );
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.6.0" );

        QTranslator grTranslator;
        QString sTranslation = QString("hsd_") + QLocale::system().name();
        QDir grDir = QDir::current();

        if (!grTranslator.load(sTranslation)) {
            std::cout << "Did not find translation data. Exit!";
            return EXIT_FAILURE;
        }
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
            qDebug() << QCoreApplication::applicationName()
                     << QCoreApplication::applicationVersion();
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
                qDebug() << grAddr.toString();
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
                    qDebug() << QObject::tr( "Received set log level command line argument but the passed log level is not correct:" )
                             << grArgsList.at( nIndex + 1);
                }
            }
            else
            {
                qDebug() << QObject::tr( "Received set log level command line argument (\"-l\") but the level was missing" )
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

        qInfo() << a.applicationName() << a.applicationVersion() << a.applicationFilePath();

        grHsd.startService();

        return a.exec();
    } // try

    catch( ... )
    {
        qCritical() << QObject::tr( "Uncought Exception" ) << Q_FUNC_INFO;
    }
}
