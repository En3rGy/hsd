#ifndef NETWORKSENDER_H
#define NETWORKSENDER_H

#include <QWidget>

namespace Ui {
class CNetworkSender;
}

class CNetworkSender : public QWidget
{
    Q_OBJECT
    
public:
    explicit CNetworkSender(QWidget *parent = 0);
    ~CNetworkSender();

    static QString printASCII( const QByteArray & p_grByteArray);
    static QByteArray stringToHex( const QString & p_sString );

public slots:
    void slot_sendMsg( void );
    
private:
    Ui::CNetworkSender *ui;
};

#endif // NETWORKSENDER_H
