#ifndef WIDGET_H
#define WIDGET_H

#include<QMenu>
#include<QWidget>
#include<QMimeData>
#include<QEventLoop>
#include<QCloseEvent>
#include<QMessageBox>
#include<QFileDialog>
#include<QInputDialog>
#include<QButtonGroup>
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
    Handler *handler;
    QStandardItemModel *model;
    QMenu *fileMenu;
    QMenu *directoryMenu;
    QMenu *emptyMenu;
    QIcon rootIcon;
    QIcon dirIcon;
    QIcon fileIcon;
    QButtonGroup *buttonGroupType;
    QButtonGroup *buttonGroupMode;
    int rowSelected;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void login_failed(QString reason);
    void closeEvent(QCloseEvent *e);
    void update_tableview();
    void update_pathedit();
private slots:
    void show_response(QString response);
    void show_listdata(QString response);
    void on_pushButton_login_clicked();
    void on_pushButton_quit_clicked();
    void on_radioButton_passive_clicked();
    void on_radioButton_positive_clicked();
    void show_menu(int row);
    void change_directory(int row);
    void save_file();
    void rename_file();
    void delete_file();
    void rename_directory();
    void delete_directory();
    void create_folder();
    void show_syst();
    void on_radioButton_binary_clicked();
    void on_radioButton_ascii_clicked();
    void on_pushButton_upload_clicked();
};

#endif // WIDGET_H
