#ifndef DIALOG_INTERFACE_STYLE_H
#define DIALOG_INTERFACE_STYLE_H

#include <QDialog>

namespace Ui {
class Dialog_interface_style;
}

class Dialog_interface_style : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_interface_style(QWidget *parent = 0);
    ~Dialog_interface_style();

private slots:
    void on_pushButton_clicked();

public slots:
    void changeStyle(QString str);

private:
    Ui::Dialog_interface_style *ui;
};

#endif // DIALOG_INTERFACE_STYLE_H
