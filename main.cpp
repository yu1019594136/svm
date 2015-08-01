#include "widget.h"
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QTextCodec> //添加头文件
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create(QString("Cleanlooks"))); // styleName 就是上图所列

    QApplication::setPalette(QApplication::style()->standardPalette()); // 选择风格默认颜色

//    qDebug() << QStyleFactory::keys();

    QApplication a(argc, argv);

    Widget w;

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8")); //使程序中可以使用中文

//    w.setAutoFillBackground(true);
//    QPalette palette;
//    QPixmap pixmap("widget_backimage.png");
//    palette.setBrush(QPalette::Window, QBrush(pixmap));
//    w.setPalette(palette);


    /* 给窗体添加滚动条，注意应该先添加ui的布局，再来调用setWidget(&w)——摘自QScrollArea文档 */
    QScrollArea *pArea;
    pArea = new QScrollArea;
    pArea->setWidget(&w);
    pArea->resize(880, 700);//确定初始滑动条显示大小
    pArea->setWidgetResizable(true);
    pArea->show();

    return a.exec();
}
