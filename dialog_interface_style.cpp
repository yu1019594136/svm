#include "dialog_interface_style.h"
#include "ui_dialog_interface_style.h"
#include <QStyleFactory>

Dialog_interface_style::Dialog_interface_style(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_interface_style)
{
    ui->setupUi(this);

    /* 列出所有支持的内部风格 */
    ui->comboBox->addItems(QStyleFactory::keys());
    connect(ui->comboBox, SIGNAL(activated(QString)), this, SLOT(changeStyle(QString)));
}

Dialog_interface_style::~Dialog_interface_style()
{
    delete ui;
}
//确定按钮
void Dialog_interface_style::on_pushButton_clicked()
{
    this->close();
}

void Dialog_interface_style::changeStyle(QString str)
{
    QApplication::setStyle(QStyleFactory::create(str));               // 更改风格
    QApplication::setPalette(QApplication::style()->standardPalette());     // 使用风格默认的颜色
}
