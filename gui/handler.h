#ifndef HANDLER_H
#define HANDLER_H

#include<QTime>
#include<QFile>
#include<QPair>
#include<QString>
#include<QTcpServer>
#include<QTcpSocket>
#include<QByteArray>
#include<QStringList>
#include<QRegularExpression>

struct Command{
    QString text;
    int argc;
};

class Handler : public QObject{
    Q_OBJECT
public:
    Handler();
    void inst_request(QString inst);
    QString inst_response();
    QPair<QTcpServer*, QString> cmd_positve(QString cmd);
    QPair<QTcpSocket*, QString> cmd_passive(QString cmd);
    void data_request(QTcpSocket* transportSocket, QFile* file);
    void data_response(QTcpSocket* transportSocket, QFile* file);
    QTcpSocket *socket;
    bool passive;
signals:
    void get_response(QString response);
    void get_listdata(QString response);
};

#endif // HANDLER_H
