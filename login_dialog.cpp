#include "login_dialog.h"
#include "ui_login_dialog.h"
#include <QMessageBox>

Login_dialog::Login_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login_dialog)
{
    ui->setupUi(this);
    ui->lineEdit->setEchoMode(QLineEdit::Password);
}

Login_dialog::~Login_dialog()
{
    delete ui;
}

void Login_dialog::on_pushButton_clicked()
{
    if(ui->lineEdit->text() == "TJUIRASENOSE")
    {
        accept();
    }
    else
    {
        // 提示对话框
        //int ret = QMessageBox::information(this, tr("提示对话框"), tr("\n\t口令错误！\n\n\t密码提示：TJUIRASENOSE\t\n"), QMessageBox::Ok);
        //if(ret == QMessageBox::Ok) qDebug()<<tr("提示！");

        QMessageBox msgBox;
        msgBox.setText(tr("口令错误!\t\t\t"));
        msgBox.setInformativeText(tr("^_^密码提示：TJUIRASENOSE"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
    }
}
