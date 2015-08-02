#ifndef DISPLAY_OUTPUT_H
#define DISPLAY_OUTPUT_H

#include <QWidget>

namespace Ui {
class display_output;
}

class display_output : public QWidget
{
    Q_OBJECT

public:
    explicit display_output(QWidget *parent = 0);
    ~display_output();

signals:
    void deselect_checkbox_display_output();

public slots:
    void recei_fro_disp_output_hide_or_show(bool state);
    void recei_fro_disp_output_disp(QString str);

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::display_output *ui;
};

#endif // DISPLAY_OUTPUT_H
