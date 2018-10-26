#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget){
    ui->setupUi(this);
    this->rowSelected = -1;
    this->ui->stackedWidget->setCurrentIndex(0);
    this->handler = new Handler;
    this->model = new QStandardItemModel(this->ui->tableView_list);
    this->model->setHorizontalHeaderLabels(QStringList()<<QString("Name")<<QString("Last Edited")<<QString("Type")<<QString("Size"));
    this->ui->tableView_list->setModel(this->model);
    this->setAcceptDrops(false);

    this->fileMenu = new QMenu(this->ui->tableView_list);
    QAction *actionSaveFile = fileMenu->addAction("Save");
    QAction *actionRenameFile = fileMenu->addAction("Rename");
    QAction *actionDeleteFile = fileMenu->addAction("Delete");
    connect(actionSaveFile, &QAction::triggered, this, &Widget::save_file);
    connect(actionDeleteFile, &QAction::triggered, this, &Widget::delete_file);
    connect(actionRenameFile, &QAction::triggered, this, &Widget::rename_file);

    this->directoryMenu = new QMenu(this->ui->tableView_list);
    QAction *actionRenameDirectory = directoryMenu->addAction("Rename");
    QAction *actionDeleteDirectory = directoryMenu->addAction("Delete");
    connect(actionDeleteDirectory, &QAction::triggered, this, &Widget::delete_directory);
    connect(actionRenameDirectory, &QAction::triggered, this, &Widget::rename_directory);

    this->emptyMenu = new QMenu(this->ui->tableView_list);
    QAction *actionCreateFolder = emptyMenu->addAction("Create folder");
    QAction *actionSyst = emptyMenu->addAction("System information");
    connect(actionCreateFolder, &QAction::triggered, this, &Widget::create_folder);
    connect(actionSyst, &QAction::triggered, this, &Widget::show_syst);

    connect(this->handler, &Handler::get_listdata, this, &Widget::show_listdata);
    connect(this->handler, &Handler::get_response, this, &Widget::show_response);
    connect(this->ui->tableView_list, &TableView::directory_clicked, this, &Widget::change_directory);
    connect(this->ui->tableView_list, &TableView::menu_called, this, &Widget::show_menu);

    this->buttonGroupMode = new QButtonGroup(this);
    this->buttonGroupMode->addButton(this->ui->radioButton_passive);
    this->buttonGroupMode->addButton(this->ui->radioButton_positive);
    this->buttonGroupType = new QButtonGroup(this);
    this->buttonGroupType->addButton(this->ui->radioButton_ascii);
    this->buttonGroupType->addButton(this->ui->radioButton_binary);

    this->ui->radioButton_binary->setChecked(true);
    this->ui->radioButton_passive->setChecked(true);

    this->fileIcon = QIcon(":/icon/res/default_file.png");
    this->rootIcon = QIcon(":/icon/res/root_directory.png");
    this->dirIcon = QIcon(":/icon/res/default_directory.png");
}

Widget::~Widget(){
    delete ui;
}

void Widget::show_response(QString response){
    this->ui->textEdit_response->append(response.trimmed());
}

void Widget::show_listdata(QString response){
    this->model->setRowCount(0);
    QStringList stringList = response.split("\r\n");
    for(QString string: stringList){
        if(string.size() == 0)
            continue;
        QRegularExpression regex("([drwxbcl-]{10})\\s+?(\\d+)\\s+?(\\w+)\\s+?(\\w+)\\s+?(\\d+)\\s+?([a-zA-Z]{3}\\s\\d+\\s\\d+:\\d+)\\s+?(\\S+)");
        QRegularExpressionMatch match = regex.match(string);
        QList<QStandardItem*> itemList;
        if(match.captured(7) == ".")
            continue;
        QStandardItem *itemFileName = nullptr;
        QStandardItem *itemEditTime = new QStandardItem(match.captured(6));
        QString fileType;
        switch(match.captured(1).at(0).toLatin1()){
        case '-':
            fileType = "regular file";
            break;
        case 'b':
            fileType = "block special file";
            break;
        case 'c':
            fileType = "character special file";
            break;
        case 'd':
            fileType = "directory";
            break;
        case 'l':
            fileType = "symbolic link";
            break;
        default:
            fileType = "unknown type";
            break;
        }
        QString fileName = match.captured(7);
        if(fileName == "..")
            itemFileName = new QStandardItem(this->rootIcon, fileName);
        else if(fileType == "directory")
            itemFileName = new QStandardItem(this->dirIcon, fileName);
        else
            itemFileName = new QStandardItem(this->fileIcon, fileName);
        QStandardItem *itemFileType = new QStandardItem(fileType);
        QStandardItem *itemFileSize = new QStandardItem(match.captured(5));
        itemList.append(itemFileName);
        itemList.append(itemEditTime);
        itemList.append(itemFileType);
        itemList.append(itemFileSize);
        this->model->appendRow(itemList);
    }
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
    this->update_pathedit();
    this->update_tableview();
    this->ui->stackedWidget->setCurrentIndex(1);
    this->setAcceptDrops(true);
}

void Widget::on_pushButton_quit_clicked(){
    this->close();
}

void Widget::closeEvent(QCloseEvent *e){
    this->handler->inst_request("quit");
    QString response = this->handler->inst_response();
    if(response.startsWith("221")){
        QMessageBox::information(this, "Information", response.trimmed(), QMessageBox::Ok);
        this->handler->socket->close();
        this->close();
        e->accept();
        return;
    }
    this->show_response(response);
    e->ignore();
}

void Widget::on_radioButton_passive_clicked(){
    qDebug() << "Set mode passive";
    this->handler->passive = true;
}

void Widget::on_radioButton_positive_clicked(){
    qDebug() << "Set mode positive";
    this->handler->passive = false;
}

void Widget::update_tableview(){
    if(this->handler->passive){
        QPair<QTcpSocket*, QString> p = this->handler->cmd_passive("list");
        if(!p.second.startsWith("150"))
            return;
        this->handler->data_response(p.first, nullptr);
        qDebug() << "List data transfer finished";
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    else{
        QPair<QTcpServer*, QString> p = this->handler->cmd_positve("list");
        if(!p.second.startsWith("150")){
            p.first->close();
            return;
        }
        p.first->waitForNewConnection(-1);
        QTcpSocket* transportSocket = p.first->nextPendingConnection();
        this->handler->data_response(transportSocket, nullptr);
        transportSocket->close();
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
}

void Widget::change_directory(int row){
    QString fileType = this->model->item(row, 2)->text();
    if(fileType != "directory")
        return;
    QString directoryName = this->model->item(row, 0)->text();
    this->handler->inst_request("cwd " + directoryName);
    QString response = this->handler->inst_response();
    this->show_response(response);
    this->update_pathedit();
    this->update_tableview();
}

void Widget::update_pathedit(){
    this->handler->inst_request("pwd");
    QString response = this->handler->inst_response();
    this->show_response(response);
    QRegularExpression regex("\"(.*?)\"");
    QRegularExpressionMatch match = regex.match(response);
    QString path = match.captured(1);
    this->ui->lineEdit_path->setText(path);
}

void Widget::show_menu(int row){
    this->rowSelected = row;
    if(row == -1)
        this->emptyMenu->exec(QCursor::pos());
    else{
        QString fileType = this->model->item(row, 2)->text();
        if(fileType == "directory")
            this->directoryMenu->exec(QCursor::pos());
        else if(fileType == "regular file")
            this->fileMenu->exec(QCursor::pos());
    }
}

void Widget::rename_directory(){
    bool ok = false;
    QString directoryName= this->model->item(this->rowSelected, 0)->text();
    QString newName = QInputDialog::getText(this, tr("Rename directory"), tr("Please enter new directory"), QLineEdit::Normal, QString(), &ok);
    if(!ok || newName.isEmpty())
        return;
    this->handler->inst_request("rnfr " + directoryName);
    this->show_response(this->handler->inst_response());
    this->handler->inst_request("rnto " + newName);
    this->show_response(this->handler->inst_response());
    this->update_tableview();
}

void Widget::delete_directory(){
    QString directoryName= this->model->item(this->rowSelected, 0)->text();
    this->handler->inst_request("rmd " + directoryName);
    this->show_response(this->handler->inst_response());
    this->update_tableview();
}
void Widget::create_folder(){
    bool ok = false;
    QString folderName = QInputDialog::getText(this, tr("Create"), tr("<p style=\"font-family: Consolas;\">Please enter folder name</p>"), QLineEdit::Normal, QString(), &ok);
    if(!ok || folderName.isEmpty())
        return;
    this->handler->inst_request("mkd " + folderName);
    this->show_response(this->handler->inst_response());
    this->update_tableview();
}
void Widget::show_syst(){
    this->handler->inst_request("syst");
    QString response = this->handler->inst_response();
    this->show_response(response);
    QMessageBox::information(this, "information", response.trimmed().mid(4), QMessageBox::Ok);
}
void Widget::save_file(){
    QString fileName = this->model->item(this->rowSelected, 0)->text();
    QString saveName = QFileDialog::getSaveFileName(this, tr("Save File"));
    if(saveName.isEmpty())
        return;
    QFile file(saveName);
    file.open(QIODevice::ReadWrite);
    if(this->handler->passive){
        QPair<QTcpSocket*, QString> p = this->handler->cmd_passive("retr " + fileName);
        if(!p.second.startsWith("150"))
            return;
        this->handler->data_response(p.first, &file);
        qDebug() << "Retr data transfer finished";
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    else{
        QPair<QTcpServer*, QString> p = this->handler->cmd_positve("retr " + fileName);
        if(!p.second.startsWith("150")){
            p.first->close();
            return;
        }
        p.first->waitForNewConnection(-1);
        QTcpSocket* transportSocket = p.first->nextPendingConnection();
        this->handler->data_response(transportSocket, &file);
        transportSocket->close();
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    file.close();
}

void Widget::delete_file(){
    QString fileName = this->model->item(this->rowSelected, 0)->text();
    this->handler->inst_request("dele " + fileName);
    this->show_response(this->handler->inst_response());
    this->update_tableview();
}
void Widget::rename_file(){
    bool ok = false;
    QString fileName= this->model->item(this->rowSelected, 0)->text();
    QString newName = QInputDialog::getText(this, tr("Rename file"), tr("Please enter new name"), QLineEdit::Normal, QString(), &ok);
    if(!ok || newName.isEmpty())
        return;
    this->handler->inst_request("rnfr " + fileName);
    this->show_response(this->handler->inst_response());
    this->handler->inst_request("rnto " + newName);
    this->show_response(this->handler->inst_response());
    this->update_tableview();
}

void Widget::on_radioButton_binary_clicked(){
    this->handler->inst_request("type I");
    this->show_response(this->handler->inst_response());
}

void Widget::on_radioButton_ascii_clicked(){
    this->handler->inst_request("type A");
    this->show_response(this->handler->inst_response());
}

void Widget::on_pushButton_upload_clicked(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Upload file"), tr("."));
    if(fileName.isEmpty())
        return;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if(this->handler->passive){
        QPair<QTcpSocket*, QString> p = this->handler->cmd_passive("stor " + fileName.section('/', -1, -1));
        if(!p.second.startsWith("150"))
            return;
        this->handler->data_request(p.first, &file);
        qDebug() << "Stor data transfer finished";
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    else{
        QPair<QTcpServer*, QString> p = this->handler->cmd_positve("stor " + fileName.section('/', -1, -1));
        if(!p.second.startsWith("150")){
            p.first->close();
            return;
        }
        p.first->waitForNewConnection(-1);
        QTcpSocket* transportSocket = p.first->nextPendingConnection();
        this->handler->data_request(transportSocket, &file);
        transportSocket->close();
        p.first->close();
        this->show_response(this->handler->inst_response());
    }
    file.close();
    this->update_tableview();
}

void Widget::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void Widget::dropEvent(QDropEvent *event){
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    for(QUrl url: urls) {
        QString fileName = url.toLocalFile();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        if(this->handler->passive){
            QPair<QTcpSocket*, QString> p = this->handler->cmd_passive("stor " + fileName.section('/', -1, -1));
            if(!p.second.startsWith("150"))
                return;
            this->handler->data_request(p.first, &file);
            qDebug() << "Stor data transfer finished";
            p.first->close();
            this->show_response(this->handler->inst_response());
        }
        else{
            QPair<QTcpServer*, QString> p = this->handler->cmd_positve("stor " + fileName.section('/', -1, -1));
            if(!p.second.startsWith("150")){
                p.first->close();
                return;
            }
            p.first->waitForNewConnection(-1);
            QTcpSocket* transportSocket = p.first->nextPendingConnection();
            this->handler->data_request(transportSocket, &file);
            transportSocket->close();
            p.first->close();
            this->show_response(this->handler->inst_response());
        }
        file.close();
    }
    this->update_tableview();
}

