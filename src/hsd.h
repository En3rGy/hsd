#ifndef HSD_H
#define HSD_H

#include <QObject>

class CTcpServer;
class CTcpClient;

class CHsd : public QObject
{
    Q_OBJECT
public:
    explicit CHsd(QObject *parent = 0);
    virtual ~CHsd( void );

    void startService( void );
    
signals:
    
public slots:

private:
    CTcpServer * m_pTcpServer;
    CTcpClient * m_pTcpClient;
    
};

#endif // HSD_H
