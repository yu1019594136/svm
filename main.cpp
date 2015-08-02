#include "widget.h"
#include "login_dialog.h"
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QTextCodec> //添加头文件
#include <QLineEdit>

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create(QString("Cleanlooks"))); // styleName 就是上图所列

    QApplication::setPalette(QApplication::style()->standardPalette()); // 选择风格默认颜色

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8")); //使程序中可以使用中文

    QApplication app(argc, argv);

    Widget svm_gui_widget;
    Login_dialog login;

    /* 给窗体添加滚动条，注意应该先添加ui的布局，再来调用setWidget(&w)——摘自QScrollArea文档 */
    QScrollArea *pArea;
    pArea = new QScrollArea;
    pArea->setWidget(&svm_gui_widget);
    pArea->resize(880, 700);//确定初始滑动条显示大小
    pArea->setWidgetResizable(true);

    if(login.exec() == QDialog::Accepted)
    {
        pArea->show();
        return app.exec();
    }
    else
        return 0;
}
