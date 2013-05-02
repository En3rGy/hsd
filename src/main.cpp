#include <QCoreApplication>
#include "tcpclient.h"
#include <QDebug>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.0.2" );

        CTcpClient grTcpClient;

        if ( argc == 1 )
        {
            grTcpClient.initConnection( "192.168.143.11", 7003 );
            grTcpClient.send( "1", "2/2/15", "1" );

            return EXIT_SUCCESS;
        }
        else if ( argc == 6 )
        {
            grTcpClient.initConnection( argv[1], atoi( argv[2] ) );
            grTcpClient.send( argv[3], argv[4], argv[5] );

            return EXIT_SUCCESS;
        }
        else if ( argc == 7 )
        {
            grTcpClient.initConnection( argv[1], atoi( argv[2] ), argv[3] );
            grTcpClient.send( argv[4], argv[5], argv[6] );

            return EXIT_SUCCESS;
        }
        else
        {
            qDebug() << "hsd [ip] [port] [opt:pass] [action] [GA] [value]";
            qDebug() << "\t[ip]\t IP adress of homeserver";
            qDebug() << "\t[port]\t Port of homeserver KO-Gateway";
            qDebug() << "\t[opt:pass]\t Password of homeserver KO-Gateway (optional)";
            qDebug() << "\t[action]\t Action to be performed";
            qDebug() << "\t[GA]\t Group adress to be called";
            qDebug() << "\t[value]\t Value to be set";

            return EXIT_SUCCESS;
        }


        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
