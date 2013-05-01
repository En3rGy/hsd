#include <QCoreApplication>
#include "tcpmanager.h"
#include <QDebug>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        QCoreApplication a(argc, argv);
        a.setApplicationName( "hsd" );
        a.setApplicationVersion( "0.0.1" );

        CTcpManager grTcpManager;

        grTcpManager.initConnection( "192.168.143.11", 7003 );

//        if ( argc != 4 )
//        {
//            std::getline( std::cin, std::string() );

//            grTcpManager.send( "1", "2/2/15", "1" );

//            std::getline( std::cin, std::string() );

//            grTcpManager.send( "1", "2/2/15", "0" );

//        }
//        else
//        {
//            grTcpManager.send( argv[1], argv[2], argv[3] );
//        }

        return a.exec();
    }
    catch( ... )
    {
        qDebug() << "Uncought Exception" << Q_FUNC_INFO;
    }
}
