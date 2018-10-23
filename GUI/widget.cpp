#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget){
    ui->setupUi(this);
    this->ui->stackedWidget->setCurrentIndex(0);
    this->handler = new Handler;
    this->model = new QStandardItemModel(this->ui->tableView_list);
    this->model->setHorizontalHeaderLabels(QStringList()<<QString("Name")<<QString("Last Edited")<<QString("Type")<<QString("Size"));
    this->ui->tableView_list->setModel(this->model);
    connect(this->handler, &Handler::get_listdata, this, &Widget::show_listdata);
    connect(this->handler, &Handler::get_response, this, &Widget::show_response);
}

Widget::~Widget(){
    delete ui;
}

void Widget::show_response(QString response){
    this->ui->textEdit_response->append(response);
}

void Widget::show_listdata(QString response){
    this->model->clear();

}

void Widget::login_failed(QString reason){
    QMessageBox::critical(this, "Error", reason, QMessageBox::Ok);
    this->ui->lineEdit_ip->clear();
    this->ui->lineEdit_password->clear();
    this->ui->lineEdit_port->clear();
    this->ui->lineEdit_username->clear();
}

void Widget::on_pushButton_login_clicked(){
    this->ui->textEdit_response->clear();
    QString serverIP = this->ui->lineEdit_ip->text().trimmed();
    QString serverPort = this->ui->lineEdit_port->text().trimmed();
    QString username = this->ui->lineEdit_username->text().trimmed();
    QString password = this->ui->lineEdit_password->text().trimmed();
    this->handler->socket->connectToHost(serverIP, serverPort.toInt(),QTcpSocket::ReadWrite);
    if(!this->handler->socket->waitForConnected(5000)){
        this->login_failed("Failed to connect to host!");
        return;
    }
    QString response = this->handler->inst_response();
    this->show_response(response);
    this->handler->inst_request("user " + username);
    response = this->handler->inst_response();
    this->show_response(response);
    if(!response.startsWith("331")){
        this->login_failed("Failed to verify username!");
        return;
    }
    this->handler->inst_request("pass " + password);
    response = this->handler->inst_response();
    this->show_response(response);
    if(!response.startsWith("230")){
        this->login_failed("Failed to verify password!");
        return;
    }
    QMessageBox::information(this, "Information", "Login success!", QMessageBox::Ok);
    this->update_tableview();
    this->ui->stackedWidget->setCurrentIndex(1);
}

void Widget::on_pushButton_quit_clicked(){
    this->close();
}

void Widget::closeEvent(QCloseEvent *e){
    this->handler->inst_request("quit");
    QString response = this->handler->inst_response();
    if(response.startsWith("221")){
        QMessageBox::information(this, "Information", response, QMessageBox::Ok);
        this->handler->socket->close();
        this->close();
        e->accept();
        return;
    }
    this->show_response(response);
    e->ignore();
}

void Widget::on_radioButton_passive_clicked(){
    this->handler->passive = true;
}

void Widget::on_radioButton_positive_clicked(){
    this->handler->passive = false;
}

void Widget::update_tableview(){
    if(this->handler->passive){
        QPair<QTcpSocket*, QString> p = this->handler->cmd_passive("list");
        if(!p.second.startsWith("150"))
            return;
        p.first->waitForConnected(5000);
        this->handler->data_response(p.first, nullptr);
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    else{
        QPair<QTcpServer*, QString> p = this->handler->cmd_positve("list");
        if(!p.second.startsWith("150")){
            p.first->close();
            return;
        }
        QTcpSocket* transportSocket = p.first->nextPendingConnection();

    }
}
