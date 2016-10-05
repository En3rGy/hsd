#include "NetworkSender.h"
#include "ui_NetworkSender.h"
#include "udpmanager.h"
#include "tcpmanager.h"
#include <QDebug>

CNetworkSender::CNetworkSender(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CNetworkSender)
{
    ui->setupUi(this);

    connect( ui->pushButton_send, SIGNAL(clicked()), this, SLOT( slot_sendMsg()) );
}



CNetworkSender::~CNetworkSender()
{
    delete ui;
}

void CNetworkSender::slot_sendMsg()
{
    QString sMsgText = ui->textEdit_msg->toPlainText();
    QString sIp      = ui->lineEdit_receiverIp->text();
    int     nPort    = ui->lineEdit_RecevierPort->text().toInt();

    sMsgText.simplified();
    sMsgText.replace( " ", "" );

    QByteArray grData;

    if ( ( sMsgText.length() % 2 ) != 0 ) {
        qDebug() << "Length of text is odd, should be even. Aborting";
        return;
    }

    for ( int i = 0; i < sMsgText.length(); ++i ) {
        if ( i % 2 != 0 ) {
            continue;
        }
        QString sBuf = sMsgText.mid( i, 2 );
        grData.append( stringToHex( sBuf ) );
    }

    if ( ui->radioButton_tcp->isChecked() == true )
    {
        CTcpManager grTcpManager( this );
        grTcpManager.sendData( grData, sIp, nPort );
    }
    else if ( ui->radioButton_udp->isChecked() == true )
    {
        CUdpManager grUdpManager( this );

        grUdpManager.sendData( grData, sIp, nPort );
    }
}

QByteArray CNetworkSender::stringToHex(const QString & p_sString )
{
    bool ok;
    ushort unParsedValue = p_sString.toUShort( &ok, 16 );
    if (!ok) {
        qDebug() << "Error parsing" << p_sString;
        return QByteArray();
    }

    char szVal = ( char ) unParsedValue;
    QByteArray grRet;
    grRet.append( szVal ); // = QByteArray::number( unParsedValue );
    return grRet;
}

QString CNetworkSender::printASCII( const QByteArray & p_grByteArray)
{
    QString sResult;
    QString sBuffer;
    for ( int i = 0; i < p_grByteArray.length(); i++ )
    {
        sBuffer = QString::number( uchar( p_grByteArray.at( i ) ), 16 );
        if ( sBuffer.length() == 1 )
        {
            sBuffer = "0" + sBuffer;
        }

        sResult += sBuffer;

        if ( i < p_grByteArray.length() - 1 )
        {
            sResult += " ";
        }
    }

    return sResult;
}
