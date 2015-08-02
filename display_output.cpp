#include "display_output.h"
#include "ui_display_output.h"
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>
#include <QColor>

display_output::display_output(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::display_output)
{
    ui->setupUi(this);
}

display_output::~display_output()
{
    delete ui;
}

void display_output::recei_fro_disp_output_hide_or_show(bool state)
{
    if(state)
    {
        this->show();
    }
    else
    {
        this->hide();
    }

}
void display_output::recei_fro_disp_output_disp(QString str)
{
    if(str == "clear()")//如果发过来的是字符串"clear()"，那么将文本浏览器清空
        ui->textBrowser->clear();
    else
        ui->textBrowser->append(str);
}

//隐藏窗口
void display_output::on_pushButton_3_clicked()
{
    this->hide();

    //发送信号给widget取消《显示输出》勾选
    emit deselect_checkbox_display_output();
}

//清空历史输出
void display_output::on_pushButton_4_clicked()
{
    ui->textBrowser->clear();
}

//字体颜色
void display_output::on_pushButton_2_clicked()
{
    QColor color = QColorDialog::getColor(Qt::red, this, tr("颜色对话框"));
    ui->textBrowser->setTextColor(color);
}
//文本背景颜色
void display_output::on_pushButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::red, this, tr("颜色对话框"));
    ui->textBrowser->setTextBackgroundColor(color);
}

//字体
void display_output::on_pushButton_5_clicked()
{
    // 标记是否按下了“OK”按钮
    bool ok;

    // 获得选择的字体
    QFont font = QFontDialog::getFont(&ok,this);

    // 如果按下“OK”按钮，那么让“字体对话框”按钮使用新字体
    // 如果按下“Cancel”按钮，那么输出信息
    if(ok) ui->textBrowser->setFont(font);
//    else qDebug() <<tr("没有选择字体！");

}
