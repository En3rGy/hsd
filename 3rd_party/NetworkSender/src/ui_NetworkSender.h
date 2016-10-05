/********************************************************************************
** Form generated from reading UI file 'NetworkSender.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NETWORKSENDER_H
#define UI_NETWORKSENDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CNetworkSender
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QRadioButton *radioButton_udp;
    QRadioButton *radioButton_tcp;
    QGroupBox *groupBox_2;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineEdit_receiverIp;
    QLabel *label_2;
    QLineEdit *lineEdit_RecevierPort;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_2;
    QTextEdit *textEdit_msg;
    QTextEdit *textEdit_reply;
    QPushButton *pushButton_send;

    void setupUi(QWidget *CNetworkSender)
    {
        if (CNetworkSender->objectName().isEmpty())
            CNetworkSender->setObjectName(QStringLiteral("CNetworkSender"));
        CNetworkSender->resize(400, 462);
        verticalLayout = new QVBoxLayout(CNetworkSender);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(CNetworkSender);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        radioButton_udp = new QRadioButton(groupBox);
        radioButton_udp->setObjectName(QStringLiteral("radioButton_udp"));
        radioButton_udp->setChecked(true);

        horizontalLayout->addWidget(radioButton_udp);

        radioButton_tcp = new QRadioButton(groupBox);
        radioButton_tcp->setObjectName(QStringLiteral("radioButton_tcp"));
        radioButton_tcp->setEnabled(true);

        horizontalLayout->addWidget(radioButton_tcp);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(CNetworkSender);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        formLayout = new QFormLayout(groupBox_2);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        lineEdit_receiverIp = new QLineEdit(groupBox_2);
        lineEdit_receiverIp->setObjectName(QStringLiteral("lineEdit_receiverIp"));

        formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_receiverIp);

        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        lineEdit_RecevierPort = new QLineEdit(groupBox_2);
        lineEdit_RecevierPort->setObjectName(QStringLiteral("lineEdit_RecevierPort"));

        formLayout->setWidget(1, QFormLayout::FieldRole, lineEdit_RecevierPort);


        verticalLayout->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(CNetworkSender);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        verticalLayout_2 = new QVBoxLayout(groupBox_3);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        textEdit_msg = new QTextEdit(groupBox_3);
        textEdit_msg->setObjectName(QStringLiteral("textEdit_msg"));

        verticalLayout_2->addWidget(textEdit_msg);

        textEdit_reply = new QTextEdit(groupBox_3);
        textEdit_reply->setObjectName(QStringLiteral("textEdit_reply"));
        textEdit_reply->setReadOnly(true);

        verticalLayout_2->addWidget(textEdit_reply);


        verticalLayout->addWidget(groupBox_3);

        pushButton_send = new QPushButton(CNetworkSender);
        pushButton_send->setObjectName(QStringLiteral("pushButton_send"));

        verticalLayout->addWidget(pushButton_send);


        retranslateUi(CNetworkSender);

        QMetaObject::connectSlotsByName(CNetworkSender);
    } // setupUi

    void retranslateUi(QWidget *CNetworkSender)
    {
        CNetworkSender->setWindowTitle(QApplication::translate("CNetworkSender", "CNetworkSender", 0));
        groupBox->setTitle(QApplication::translate("CNetworkSender", "Protokoll", 0));
        radioButton_udp->setText(QApplication::translate("CNetworkSender", "UDP", 0));
        radioButton_tcp->setText(QApplication::translate("CNetworkSender", "TCP", 0));
        groupBox_2->setTitle(QApplication::translate("CNetworkSender", "Receiver", 0));
        label->setText(QApplication::translate("CNetworkSender", "IP:", 0));
        label_2->setText(QApplication::translate("CNetworkSender", "Port:", 0));
        groupBox_3->setTitle(QApplication::translate("CNetworkSender", "Message", 0));
        pushButton_send->setText(QApplication::translate("CNetworkSender", "Send", 0));
    } // retranslateUi

};

namespace Ui {
    class CNetworkSender: public Ui_CNetworkSender {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NETWORKSENDER_H
