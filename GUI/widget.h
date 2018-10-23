#ifndef WIDGET_H
#define WIDGET_H

#include<QWidget>
#include<QEventLoop>
#include<QCloseEvent>
#include<QMessageBox>
#include<QStandardItem>
#include<QStandardItemModel>
#include"handler.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
private:
    Ui::Widget *ui;
    Handler* handler;
    QStandardItemModel* model;
    void login_failed(QString reason);
    void closeEvent(QCloseEvent *e);
    void update_tableview();
private slots:
    void show_response(QString response);
    void show_listdata(QString response);
    void on_pushButton_login_clicked();
    void on_pushButton_quit_clicked();
    void on_radioButton_passive_clicked();
    void on_radioButton_positive_clicked();
};

#endif // WIDGET_H
