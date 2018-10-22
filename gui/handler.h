#ifndef HANDLER_H
#define HANDLER_H

#include<QFile>
#include<QPair>
#include<QString>
#include<QTcpServer>
#include<QTcpSocket>

struct Command{
    QString text;
    int argc;
};

class handler
{
public:
    handler();
    void InstRequest(QString inst);
    QString InstResponse();
    QPair<QTcpServer*, QString> CmdPositve(QString cmd);
    QPair<QTcpSocket*, QString> CmdPassive(QString cmd);
    void DataRequest(QTcpSocket* socket, QFile* file);
    void DataResponse(QTcpSocket* socket, QFile* file);
private:
    QTcpSocket *socket;
    bool passive;
};

#endif // HANDLER_H
