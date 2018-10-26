#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QFile file(":/style/res/style.css");
    file.open(QIODevice::ReadOnly);
    a.setStyleSheet(file.readAll());
    file.close();
    Widget w;
    w.show();
    return a.exec();
}
