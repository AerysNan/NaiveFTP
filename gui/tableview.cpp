#include "tableview.h"

TableView::TableView(QWidget *parent) : QTableView(parent){}

void TableView::mousePressEvent(QMouseEvent *event){
    setCurrentIndex(QModelIndex());
    QTableView::mousePressEvent(event);
    QModelIndex index = this->currentIndex();
    if(event->button() == Qt::LeftButton)
        return;
    if(event->button() == Qt::RightButton)
        emit this->menu_called(index.row());
}

void TableView::mouseDoubleClickEvent(QMouseEvent *event){
    QModelIndex index = this->currentIndex();
    if(index.row() < 0 && index.column() < 0)
        return;
    if(event->button() == Qt::RightButton)
        return;
    if(event->button() == Qt::LeftButton)
        emit this->directory_clicked(index.row());
}
