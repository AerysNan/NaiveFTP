#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include<QTableView>
#include<QMouseEvent>
#include<QMessageBox>

class TableView : public QTableView{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);
private:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
signals:
    void directory_clicked(int row);
    void menu_called(int row);
};

#endif // TABLEVIEW_H
