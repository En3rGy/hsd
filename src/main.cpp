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
        }
        else if ( argc == 5 )
        {
            grTcpClient.initConnection( "192.168.143.11", 7003 );
            grTcpClient.send( argv[1], argv[2], argv[3] );
        }
        else if ( argc == 6 )
        {
            grTcpClient.initConnection( "192.168.143.11", 7003 );
            grTcpClient.send( argv[1], argv[2], argv[3] );
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
        }


        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
