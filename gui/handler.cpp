#include "handler.h"

Handler::Handler(){
    this->passive = true;
    this->socket = new QTcpSocket;
    QTime time = QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
}

void Handler::inst_request(QString inst){
    QString data = inst.trimmed() + "\r\n";
    this->socket->write(data.toStdString().c_str());
}

QString Handler::inst_response(){
    QString response = "";
    QByteArray testArray =this->socket->readLine();
    if(testArray.size()==0)
        this->socket->waitForReadyRead(-1);
    else{
        qDebug() << "Read command line from previous buffer" << testArray;
        response.append(testArray);
    }
    while(true){
        QByteArray array = this->socket->readLine();
        qDebug() << "Read command line:" << array;
        if(array.size() == 0 || array.at(3) == ' '){
            qDebug() << "Read command line finished.";
            response.append(array);
            return response;
        }
        response.append(array);
    }
}

QPair<QTcpServer*, QString> Handler::cmd_positve(QString cmd){
    QString ip;
    for(QHostAddress address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)){
            ip = address.toString();
            if(ip.section('.', -1, -1) == "1" || ip.section('.', 0, 0) == "169" || ip.section('.', 0, 0) == "192")
                continue;
            qDebug() << "Using IP address:" << address.toString();
            ip = ip.split('.').join(',');
            break;
        }
    }
    while(true) {
        int port = qrand()%45535+20000;
        QTcpServer* server = new QTcpServer;
        if(server->listen(QHostAddress::Any, port)){
            QString message = "port " + ip + "," + QString::number(port>>8) + ","  + QString::number(port & 0xff);
            this->inst_request(message);
            emit this->get_response(this->inst_response());
            this->inst_request(cmd);
            QString response = this->inst_response();
            emit this->get_response(response);
            return QPair<QTcpServer*, QString>(server, response);
        }
    }
}

QPair<QTcpSocket*, QString> Handler::cmd_passive(QString cmd){
    this->inst_request("pasv");
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
    QString url = QString::number(h1) + "." + QString::number(h2) + "." + QString::number(h3) + "." + QString::number(h4);
    QTcpSocket* transportSocket = new QTcpSocket;
    transportSocket->connectToHost(url, (p1<<8)+p2, QTcpSocket::ReadWrite);
    transportSocket->waitForConnected(-1);
    qDebug() << "Transport channel connected.";
    this->inst_request(cmd);
    qDebug() << "Command passive:" << cmd;
    response = this->inst_response();
    emit this->get_response(response);
    return QPair<QTcpSocket*, QString>(transportSocket, response);
}

void Handler::data_request(QTcpSocket* tranportSocket, QFile* file){
    while(true) {
        QByteArray array = file->readLine();
        if(array.size() <= 0)
            return;
        qDebug() << "===== Send array =====";
        qDebug() << array;
        tranportSocket->write(array);
        tranportSocket->waitForBytesWritten(-1);
    }
}

void Handler::data_response(QTcpSocket* transportSocket, QFile *file){
    QString string = "";
    while(true){
        transportSocket->waitForReadyRead(-1);
        QByteArray array = transportSocket->readLine();
        if(array.size() <= 0){
            qDebug() << "Data transport finished.";
            if(file == nullptr)
                emit this->get_listdata(string);
            return;
        }
        qDebug() << "===== Read array =====";
        qDebug() << array;
        if(file != nullptr)
            file->write(array);
        else
            string.append(array);
    }
}
