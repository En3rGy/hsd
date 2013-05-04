#include <QCoreApplication>
#include "tcpclient.h"
#include "tcpserver.h"
#include <QDebug>
#include <string>
#include <iostream>
#include <QSettings>
#include <QObject>

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.0.2" );

        QSettings::setPath( QSettings::IniFormat, QSettings::SystemScope, "hsd.ini" );

        CTcpServer grTcpServer;
        CTcpClient grTcpClient;

        QObject::connect ( & grTcpServer,
                  SIGNAL(signal_setEibAdress(QString,int)),
                  & grTcpClient,
                  SLOT( slot_setEibAdress(QString,int)));

        if ( argc == 2 )
        {
            if ( QString( argv[ 1 ] ) == "-t")
            {
                grTcpClient.getGaXml();
                grTcpClient.initConnection();
                grTcpClient.send( "1", "2/2/15", "1" );

                return EXIT_SUCCESS;
            }
            else if ( QString( argv[ 1 ] ) == "-ga" )
            {
                grTcpClient.getGaXml();
            }
            else if ( QString( argv[ 1 ] ) == "-s" )
            {
                grTcpServer.listen();
                grTcpClient.initConnection();
            }

        }
        else if ( argc == 4 )
        {
            grTcpClient.getGaXml();
            grTcpClient.initConnection();
            grTcpClient.send( argv[1], argv[2], argv[3] );

            return EXIT_SUCCESS;
        }
        else if ( argc == 5 )
        {
            grTcpClient.getGaXml();
            grTcpClient.initConnection( argv[1] );
            grTcpClient.send( argv[2], argv[3], argv[4] );

            return EXIT_SUCCESS;
        }
        else
        {
            qDebug() << "hsd [opt:pass] [action] [GA] [value]";
            qDebug() << "\t[opt:pass]\t Password of homeserver KO-Gateway (optional)";
            qDebug() << "\t[action]\t Action to be performed";
            qDebug() << "\t[GA]\t Group adress to be called";
            qDebug() << "\t[value]\t Value to be set";
            qDebug() << "-t\t Test";
            qDebug() << "-ga\t Receive GAs from HS";
            qDebug() << "-s\t Start server";

            return EXIT_SUCCESS;
        }


        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
