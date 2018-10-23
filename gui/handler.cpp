#include "handler.h"

Handler::Handler(){
    this->passive = true;
    this->socket = new QTcpSocket;
    QTime time = QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
}

void Handler::inst_request(QString inst){
    QString data = inst.trimmed() + QString("\r\n");
    this->socket->write(data.toStdString().c_str());
}

QString Handler::inst_response(){
    this->socket->waitForReadyRead(-1);
    QByteArray array = this->socket->readAll();
    return QString(array).trimmed();
}

QPair<QTcpServer*, QString> Handler::cmd_positve(QString cmd){
    QHostAddress hostAddress = this->socket->localAddress();
    QString ip = hostAddress.toString().split(".").join(",");
    while(true) {
        int port = qrand()%45535+20000;
        QTcpServer* server = new QTcpServer;
        if(server->listen(QHostAddress::Any, port)){
            QString message = QString("port " + ip + "," + QString(port>>8) + ","  + QString(port & 0xff));
            this->inst_request(message);
            emit this->get_response(this->inst_response());
            this->inst_request(cmd);
            QString response = this->inst_response();
            emit this->get_response(this->inst_response());
            if(response.startsWith("150"))
                return QPair<QTcpServer*, QString>(nullptr, response);
            return QPair<QTcpServer*, QString>(server, response);
        }
    }
}

QPair<QTcpSocket*, QString> Handler::cmd_passive(QString cmd){
    this->inst_request(QString("pasv"));
    QString response = this->inst_response();
    emit this->get_response(response);
    QRegularExpression regex("\\((\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)\\)");
    QRegularExpressionMatch match = regex.match(response);
    int h1 = match.captured(1).toInt();
    int h2 = match.captured(2).toInt();
    int h3 = match.captured(3).toInt();
    int h4 = match.captured(4).toInt();
    int p1 = match.captured(5).toInt();
    int p2 = match.captured(6).toInt();
    QString url = QString(h1) + "." + QString(h2) + "." + QString(h3) + "." + QString(h4);
    QTcpSocket* transportSocket = new QTcpSocket;
    transportSocket->connectToHost(url, (p1<<8)+p2, QTcpSocket::ReadWrite);
    this->inst_request(cmd);
    response = this->inst_response();
    emit this->get_response(response);
    return QPair<QTcpSocket*, QString>(transportSocket, response);
}

void Handler::data_request(QTcpSocket* tranportSocket, QFile* file){
    while(true) {
        char buffer[BUFSIZ];
        if(file->read(buffer, BUFSIZ) <= 0){
            tranportSocket->write(buffer, qstrlen(buffer));
            return;
        }
        tranportSocket->write(buffer, BUFSIZ);
    }
}

void Handler::data_response(QTcpSocket* socket, QFile *file){
    QString string = "";
    while(true){
        char buffer[BUFSIZ];
        if(socket->read(buffer, BUFSIZ) <= 0){
            if(file != nullptr)
                file->write(buffer, qstrlen(buffer));
            else
                string += buffer;
                emit this->get_listdata(string);
            return;
        }
        if(file != nullptr)
            file->write(buffer, BUFSIZ);
        else
            string += buffer;
    }
}
