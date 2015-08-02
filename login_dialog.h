#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>

namespace Ui {
class Login_dialog;
}

class Login_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Login_dialog(QWidget *parent = 0);
    ~Login_dialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Login_dialog *ui;
};

#endif // LOGIN_DIALOG_H
