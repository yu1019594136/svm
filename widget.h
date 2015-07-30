#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QScrollArea>

/* 数据处理线程 */
#include "thread_data_proc.h"

/* 支持向量机相关头文件 */
#include "svm.h"
#include "svm-scale.h"
#include "svm-predict.h"
#include "svm-train.h"
#include "common.h"
#include "qcommon.h"
#include "grid_search.h"
#include "aco_search.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    /* 参数重置，重置对象包括界面的控件和程序中的变量 */
    void set_parameters_to_default();

    /* 从界面读取全部参数保存到程序变量中，此函数应该和int write_parameters_to_file(char *savefile)配合使用将界面参数记录到文件 */
    void read_parameters_from_interface();

    /* 将程序变量参数的值记录到文件svm_parameter.txt，此函数应该和void read_parameters_from_interface()配合使用将界面参数记录到文件 */
    int write_parameters_to_file(char *savefile);

    /* 从文件读取全部参数直接保存到界面控件中 */
    int read_parameters_from_file(char *savefile);

    /* 解析各个结构体（SVM_ALL_INPUT_FILEPATH_ON_INTERFACE、SVM_FILE_SOURCE）中的数据，
     * 并将最后的算法能够接受的数据填写到SVM_ALL_FILEPATH、PARA_SVM_SCALE、PARA_SVM_TRAIN、
     * PARA_SVM_PREDICT、PARA_GRID_SEARCH、PARA_ACO_SEARCH中 */
    int parse_svm_parameters();

    /* 配置文件路径变量 */
    int filepath_config(SVM_ALL_FILEPATH *svm_all_filepath);

signals:
    void send_to_datapro_svm_task(SVM_TASK svm_task_para);

public slots:
    void recei_from_datapro_display_output(QString str);

    void recei_fro_datapro_task_done();

private slots:
    void y_scaling_l_u_hide_show(int state);

    void scale_module_hide_show(int state);
    void train_module_hide_show(int state);
    void test_module_hide_show(int state);
    void grid_search_module_hide_show(int state);
    void aco_search_module_hide_show(int state);
    void display_output_hide_show(int state);

    void restore_filename_enable(bool state);
    void train_data_scaled_filepath(bool state);
    void model_filepath(bool state);
    void test_data_scaled_filepath(bool state);

    void svm_type_para_enbale(int index);
    void kernel_type_para_enbale(int index);

    void on_pushButton_11_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

private:
    Ui::Widget *ui;
    QScrollArea *pArea;

    /* 数据处理线程 */
    DataProcessThread *dataprocess_thread;

    SVM_TASK svm_task;
    SVM_ALL_INPUT_FILEPATH_ON_INTERFACE svm_all_filepath_on_interface;

};

#endif // WIDGET_H
