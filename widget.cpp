#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileDialog>
#include <QMetaType>
#include <stdio.h>

Q_DECLARE_METATYPE(SVM_TASK)
//Q_DECLARE_METATYPE(SVM_ALL_INPUT_FILEPATH_ON_INTERFACE)

/* 支持向量机相关变量参数，以下变量用于获取保存从界面去读取到的参数，这些变量将作为全局变量和数据处理线程共享 */
SVM_ALL_FILEPATH Svm_all_filepath;//配置所有文件路径相关的变量
PARA_SVM_SCALE Para_svm_scale;//缩放参数
PARA_SVM_TRAIN Para_svm_train;//训练参数
int *weight_label;//训练样本类别之间样本数量不平衡时可能需要对不同类别设置不同的权重，此时在weight_label和weight两
double *weight;   //个数组对应位置分别填入类别标签和权重,注意其数组长度必须和nr_weight相等!!!
PARA_SVM_PREDICT Para_svm_predict;//预测参数
PARA_GRID_SEARCH Para_grid_search;
PARA_ACO_SEARCH Para_aco_search;

SVM_FILE_SOURCE svm_file_source;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    svm_task.svm_scale_task = false;
    //svm_task.restore_filepath_source = SCALE_ACCORD_TO_PARA;
    svm_task.svm_train_task = false;
    svm_task.svm_predict_task = false;
    svm_task.svm_grid_search_task = false;
    svm_task.svm_aco_search_task = false;

    svm_file_source.restore_filepath_source = SCALE_ACCORD_TO_PARA;
    svm_file_source.model_filepath_source = FILE_FROM_LAST_TASK;
    svm_file_source.train_data_scaled_filepath_source = FILE_FROM_LAST_TASK;
    svm_file_source.test_data_scaled_filepath_source = FILE_FROM_LAST_TASK;
    svm_file_source.y_scaling_flag = Y_SCALING_NO;

    Svm_all_filepath.train_data_filepath = NULL;
    Svm_all_filepath.train_data_scaled_filepath = NULL;
    Svm_all_filepath.test_data_filepath = NULL;
    Svm_all_filepath.test_data_scaled_filepath = NULL;
    Svm_all_filepath.test_data_scaled_filepath_other_file = NULL;
    Svm_all_filepath.range_filepath = NULL;
    Svm_all_filepath.model_filepath = NULL;
    Svm_all_filepath.predict_accuracy_filepath = NULL;
    Svm_all_filepath.aco_search_result = NULL;
    Svm_all_filepath.grid_search_result = NULL;

    weight_label = NULL;
    weight = NULL;

    /* 注册元类型 */
    qRegisterMetaType <SVM_TASK>("SVM_TASK");
//    qRegisterMetaType <SVM_ALL_INPUT_FILEPATH_ON_INTERFACE>("SVM_ALL_INPUT_FILEPATH_ON_INTERFACE");

    /* 界面控件初始化 C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR */
    QStringList svm_type;
    svm_type.append("C_SVC");
    svm_type.append("NU_SVC");
    svm_type.append("ONE_CLASS");
    svm_type.append("EPSILON_SVR");
    svm_type.append("NU_SVR");
    ui->comboBox->addItems(svm_type);
    ui->comboBox->setEditable(false);
    ui->comboBox->setCurrentIndex(1);//C_SVM

    QStringList kernel_type;
    kernel_type.append("LINEAR");
    kernel_type.append("POLY");
    kernel_type.append("RBF");
    kernel_type.append("SIGMOID");
    kernel_type.append("PRECOMPUTED");
    ui->comboBox_2->addItems(kernel_type);
    ui->comboBox_2->setEditable(false);
    ui->comboBox_2->setCurrentIndex(1);//LINEAR

    /* 相关按钮的动作导致某些部件的隐藏和显示 */

    //功能模块选择中的复选框是否勾选决定相关功能模块是否显示
    connect(ui->checkBox_2, SIGNAL(stateChanged(int)), SLOT(scale_module_hide_show(int)));
    connect(ui->checkBox_3, SIGNAL(stateChanged(int)), SLOT(train_module_hide_show(int)));
    connect(ui->checkBox_4, SIGNAL(stateChanged(int)), SLOT(test_module_hide_show(int)));
    connect(ui->checkBox_6, SIGNAL(stateChanged(int)), SLOT(grid_search_module_hide_show(int)));
    connect(ui->checkBox_7, SIGNAL(stateChanged(int)), SLOT(aco_search_module_hide_show(int)));
    connect(ui->checkBox_8, SIGNAL(stateChanged(int)), SLOT(display_output_hide_show(int)));

    //缩放参数中的y_scaling复选框是否勾选决定相关参数部件是否使能
    connect(ui->checkBox, SIGNAL(stateChanged(int)), SLOT(y_scaling_l_u_hide_show(int)));
    //radioButton被选中时，restore_filename一行将被禁能
    connect(ui->radioButton, SIGNAL(toggled(bool)), SLOT(restore_filename_enable(bool)));
    //radioButton_3被选中时，svm_train.train_data_scaled_filepath:一行将被禁能
    connect(ui->radioButton_3, SIGNAL(toggled(bool)), SLOT(train_data_scaled_filepath(bool)));
    //radioButton_7被选中时，svm_predict.model_filepath一行将被禁能
    connect(ui->radioButton_7, SIGNAL(toggled(bool)), SLOT(model_filepath(bool)));
    //radioButton_5被选中时，test_data_scaled_filepath一行将被禁能
    connect(ui->radioButton_5, SIGNAL(toggled(bool)), SLOT(test_data_scaled_filepath(bool)));

    //combox的当前QString(C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR)将导致不需要参数配置项被禁能
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(svm_type_para_enbale(int)));
    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(kernel_type_para_enbale(int)));

    /* 将所有部件设置初始默认参数 */
    set_parameters_to_default();//该函数的调用必须放在最后

    /* 初始化一个数据处理线程，并启动 */
    dataprocess_thread = new DataProcessThread();

    /* dataproc线程的字符串输出到文本浏览器 */
    connect(dataprocess_thread, SIGNAL(send_to_textbrower_display_output(QString)), this, SLOT(recei_from_datapro_display_output(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(send_to_datapro_svm_task(SVM_TASK)), dataprocess_thread, SLOT(recei_fro_widget_svm_task(SVM_TASK)), Qt::QueuedConnection);
    connect(dataprocess_thread, SIGNAL(task_done()), this, SLOT(recei_fro_datapro_task_done()), Qt::QueuedConnection);
    dataprocess_thread->start();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::y_scaling_l_u_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->label_9->setEnabled(false);
        ui->label_10->setEnabled(false);
        ui->doubleSpinBox_3->setEnabled(false);
        ui->doubleSpinBox_4->setEnabled(false);

        svm_file_source.y_scaling_flag = Y_SCALING_NO;
    }
    else if(state == Qt::Checked)
    {
        ui->label_9->setEnabled(true);
        ui->label_10->setEnabled(true);
        ui->doubleSpinBox_3->setEnabled(true);
        ui->doubleSpinBox_4->setEnabled(true);

        svm_file_source.y_scaling_flag = Y_SCALING_YES;
    }
}
void Widget::scale_module_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->groupBox->hide();
        svm_task.svm_scale_task = false;
    }
    else if(state == Qt::Checked)
    {
        ui->groupBox->show();
        svm_task.svm_scale_task = true;
    }
}
void Widget::train_module_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->groupBox_2->hide();
        svm_task.svm_train_task = false;
    }
    else if(state == Qt::Checked)
    {
        ui->groupBox_2->show();
        svm_task.svm_train_task = true;
    }
}
void Widget::test_module_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->groupBox_3->hide();
        svm_task.svm_predict_task = false;
    }
    else if(state == Qt::Checked)
    {
        ui->groupBox_3->show();
        svm_task.svm_predict_task = true;
    }
}
void Widget::grid_search_module_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->groupBox_6->hide();
        if(ui->checkBox_7->checkState() == Qt::Unchecked)
        {
            ui->checkBox_3->setEnabled(true);
            ui->checkBox_4->setEnabled(true);
        }

        svm_task.svm_grid_search_task = false;
    }
    else if(state == Qt::Checked)//如果格点搜索复选框被选中，那么训练和预测复选框将被选中，然后禁能，表示训练和预测模块强制要求启用，不允许取消
    {
        ui->groupBox_6->show();
        ui->checkBox_3->setChecked(true);
        ui->checkBox_4->setChecked(true);

        ui->checkBox_3->setEnabled(false);
        ui->checkBox_4->setEnabled(false);

        svm_task.svm_grid_search_task = true;
    }
}
void Widget::aco_search_module_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->groupBox_7->hide();
        if(ui->checkBox_6->checkState() == Qt::Unchecked)
        {
            ui->checkBox_3->setEnabled(true);
            ui->checkBox_4->setEnabled(true);
        }

        svm_task.svm_aco_search_task  = false;

    }
    else if(state == Qt::Checked)//如果蚁群搜索复选框被选中，那么训练和预测复选框将被选中，然后禁能，表示训练和预测模块强制要求启用，不允许取消
    {
        ui->groupBox_7->show();
        ui->checkBox_3->setChecked(true);
        ui->checkBox_4->setChecked(true);

        ui->checkBox_3->setEnabled(false);
        ui->checkBox_4->setEnabled(false);

        svm_task.svm_aco_search_task  = true;
    }
}
void Widget::display_output_hide_show(int state)
{
    if(state == Qt::Unchecked)
    {
        ui->textBrowser->hide();
    }
    else if(state == Qt::Checked)
    {
        ui->textBrowser->show();
    }
}
/* 参数重置，重置对象包括界面的控件和程序中的变量 */
void Widget::set_parameters_to_default()
{
    /* 支持向量机相关数据变量的默认初始化 */
    Para_svm_scale.l = -1.0;
    Para_svm_scale.u = 1.0;
    Para_svm_scale.y_scaling = 0;
    Para_svm_scale.y_lower = 0.0;
    Para_svm_scale.y_upper = 0.0;
    Para_svm_scale.data_set = NULL;
    Para_svm_scale.save_filename = NULL;
    Para_svm_scale.restore_filename = NULL;
    Para_svm_scale.result_filename = NULL;

    Para_svm_train.svm_train_parameter.svm_type = C_SVC;//C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR,选择一个参数
    Para_svm_train.svm_train_parameter.kernel_type = RBF;//LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED, 选择一个参数
    Para_svm_train.svm_train_parameter.degree = 3;
    Para_svm_train.svm_train_parameter.gamma = 1;//0.1;//0代表默认
    Para_svm_train.svm_train_parameter.coef0 = 0;
    Para_svm_train.svm_train_parameter.nu = 0.5;//
    Para_svm_train.svm_train_parameter.cache_size = 100;
    Para_svm_train.svm_train_parameter.C = 	1;//1;
    Para_svm_train.svm_train_parameter.eps = 1e-3;
    Para_svm_train.svm_train_parameter.p = 0.1;
    Para_svm_train.svm_train_parameter.shrinking = 1;
    Para_svm_train.svm_train_parameter.probability = 0;
    Para_svm_train.svm_train_parameter.nr_weight = 0;//0表示不设置权重，n表示有n个类别要设置权重系数，n个类别以及对应的权重系数由本文件开头处的数组weight_label和weight设定,注意其数组长度必须和n相等
    Para_svm_train.svm_train_parameter.weight_label = NULL;//weight_label该数组在本文件开头处初始化
    Para_svm_train.svm_train_parameter.weight = NULL;//weight该数组在本文件开头处初始化
    Para_svm_train.quiet_mode = 0;//0: outputs;  !0: no outputs
    Para_svm_train.cross_validation = 0;// 5;// 0;//输入0表示不进行交叉验证，输入n表示进行n折交叉验证（注意n必须大于等于2）
    Para_svm_train.training_set_file = NULL;//Svm_all_filepath.train_data_scaled_filepath;
    Para_svm_train.model_file = NULL;//Svm_all_filepath.model_filepath;

    /* 配置预测参数 */
    Para_svm_predict.predict_probability = 0;//probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported
    Para_svm_predict.quiet_mode = 0;//0: outputs;  !0: no outputs
    Para_svm_predict.test_file = NULL;//Svm_all_filepath.test_data_scaled_filepath;
    Para_svm_predict.model_file = NULL;//Svm_all_filepath.model_filepath;
    Para_svm_predict.output_file = NULL;//Svm_all_filepath.predict_accuracy_filepath;

    /* 格点搜索参数配置 */
    Para_grid_search.d1_begin = -5;//底数为2，下同,搜索参数时第一个参数d1必须是核函数参数g，第二个参数d2是分类器参数(nu or C)
    Para_grid_search.d1_end = 5;//10;
    Para_grid_search.d1_step = 1;
    Para_grid_search.d2_begin = -5;//注意nu参数取值范围（0,1）开区间
    Para_grid_search.d2_end = -0.01;
    Para_grid_search.d2_step = -0.01;//1;
    Para_grid_search.v_fold = 4;
    Para_grid_search.flag_predict = 1;// 1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_grid_search.output_file = NULL;//Svm_all_filepath.grid_search_result;

    /* 蚁群搜索参数配置 */
    Para_aco_search.ants_amount = 1;
    Para_aco_search.x_aixs_lines = 1;
    Para_aco_search.y_aixs_values = 10;
    Para_aco_search.max_iterations = 150;
    Para_aco_search.pheromone_initial_value = 0.0;
    Para_aco_search.alpha = 0.0;
    Para_aco_search.beta = 0.0;
    Para_aco_search.rho = 0.0;
    Para_aco_search.Q = 0.0;
    Para_aco_search.flag_predict = 1;//1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_aco_search.v_fold = 4;
    Para_aco_search.MAX_TAU = 20.0;
    Para_aco_search.MIN_TAU = 0.0001;
    Para_aco_search.ERR = 10;
    Para_aco_search.best_path = NULL;
    Para_aco_search.output_file = NULL;//Svm_all_filepath.aco_search_result;
    //经验最优路径，小数点必须有2个，逗号必须有1个，其分割位置确定了搜索参数的哪些位;小数点前面没有数字表示，该参数仅仅搜索小数点后面的数字
    //每一个数位上的数字表达了最优路径


    /* 界面ui的初始化 */
    //隐藏五个功能模块
    ui->groupBox->hide();
    ui->groupBox_2->hide();
    ui->groupBox_3->hide();
    ui->groupBox_6->hide();
    ui->groupBox_7->hide();
    ui->textBrowser->hide();

    /* 五个功能复选框置为初始未选中状态 */
    ui->checkBox_2->setChecked(false);
    ui->checkBox_3->setChecked(false);
    ui->checkBox_4->setChecked(false);
    ui->checkBox_6->setChecked(false);
    ui->checkBox_7->setChecked(false);
    ui->checkBox_8->setChecked(false);

    //数据缩放模块默认参数
    ui->radioButton->setChecked(true);
    ui->lineEdit->setText(QString("*"));
    ui->lineEdit_2->setText(QString(""));
    ui->doubleSpinBox->setValue(Para_svm_scale.l);
    ui->doubleSpinBox_2->setValue(Para_svm_scale.u);
    ui->checkBox->setChecked(false);
    ui->doubleSpinBox_4->setValue(Para_svm_scale.y_lower);
    ui->doubleSpinBox_3->setValue(Para_svm_scale.y_upper);

    ui->label_9->setEnabled(false);
    ui->label_10->setEnabled(false);
    ui->doubleSpinBox_3->setEnabled(false);
    ui->doubleSpinBox_4->setEnabled(false);

    //训练模块默认参数
    ui->radioButton_3->setChecked(true);
    ui->lineEdit_5->setText("*");
    ui->radioButton_3->setChecked(true);
    ui->comboBox->setCurrentIndex(0);//C_SVM
    ui->comboBox_2->setCurrentIndex(0);//LINEAR
    ui->doubleSpinBox_5->setValue(Para_svm_train.svm_train_parameter.C);
    ui->spinBox->setValue(Para_svm_train.svm_train_parameter.degree);
    ui->doubleSpinBox_6->setValue(Para_svm_train.svm_train_parameter.p);
    ui->doubleSpinBox_8->setValue(Para_svm_train.svm_train_parameter.gamma);
    ui->doubleSpinBox_7->setValue(Para_svm_train.svm_train_parameter.nu);
    ui->doubleSpinBox_9->setValue(Para_svm_train.svm_train_parameter.coef0);
    ui->doubleSpinBox_10->setValue(Para_svm_train.svm_train_parameter.cache_size);
    ui->doubleSpinBox_11->setValue(Para_svm_train.svm_train_parameter.eps);
    ui->spinBox_2->setValue(Para_svm_train.svm_train_parameter.nr_weight);
    ui->lineEdit_6->setText("1:3,2:2,3:5(label_1:weight_1,)");
    ui->spinBox_3->setValue(Para_svm_train.svm_train_parameter.shrinking);
    ui->spinBox_4->setValue(Para_svm_train.svm_train_parameter.probability);
    ui->spinBox_5->setValue(Para_svm_train.cross_validation);
    ui->spinBox_6->setValue(Para_svm_train.quiet_mode);

    //预测模块默认参数
    ui->radioButton_7->setChecked(true);
    ui->radioButton_5->setChecked(true);
    ui->lineEdit_7->setText("*");
    ui->lineEdit_8->setText("*");
    ui->spinBox_7->setValue(Para_svm_predict.predict_probability);
    ui->spinBox_8->setValue(Para_svm_predict.quiet_mode);

    /* 格点搜索参数配置 */
    ui->doubleSpinBox_12->setValue(Para_grid_search.d1_begin);
    ui->doubleSpinBox_13->setValue(Para_grid_search.d1_step);
    ui->doubleSpinBox_14->setValue(Para_grid_search.d1_end);
    ui->doubleSpinBox_15->setValue(Para_grid_search.d2_begin);
    ui->doubleSpinBox_16->setValue(Para_grid_search.d2_step);
    ui->doubleSpinBox_17->setValue(Para_grid_search.d2_end);
    ui->spinBox_9->setValue(Para_grid_search.v_fold);
    ui->spinBox_10->setValue(Para_grid_search.flag_predict);

    /* 蚁群搜索参数配置 */
    ui->spinBox_11->setValue(Para_aco_search.ants_amount);
    ui->spinBox_12->setValue(Para_aco_search.x_aixs_lines);
    ui->spinBox_13->setValue(Para_aco_search.y_aixs_values);
    ui->spinBox_14->setValue(Para_aco_search.max_iterations);

    ui->doubleSpinBox_18->setValue(Para_aco_search.pheromone_initial_value);
    ui->doubleSpinBox_19->setValue(Para_aco_search.alpha);
    ui->doubleSpinBox_20->setValue(Para_aco_search.beta);
    ui->doubleSpinBox_21->setValue(Para_aco_search.rho);
    ui->doubleSpinBox_22->setValue(Para_aco_search.MAX_TAU);
    ui->doubleSpinBox_23->setValue(Para_aco_search.MIN_TAU);
    ui->doubleSpinBox_24->setValue(Para_aco_search.ERR);
    ui->doubleSpinBox_25->setValue(Para_aco_search.Q);
    ui->spinBox_15->setValue(Para_aco_search.flag_predict);
    ui->spinBox_16->setValue(Para_aco_search.v_fold);
    ui->lineEdit_9->setText("2.0000,0.020090");
}
//radioButton被选中时，restore_filename一行将被禁能
void Widget::restore_filename_enable(bool state)
{
    ui->lineEdit_4->setEnabled(!state);
    ui->pushButton_4->setEnabled(!state);

    ui->label_4->setEnabled(state);
    ui->label_5->setEnabled(state);
    ui->doubleSpinBox->setEnabled(state);
    ui->doubleSpinBox_2->setEnabled(state);

    ui->checkBox->setChecked(false);
    ui->checkBox->setEnabled(state);

    if(state)//
    {
        svm_file_source.restore_filepath_source = SCALE_ACCORD_TO_PARA;
        //svm_task.restore_filepath_source = svm_file_source.restore_filepath_source;
    }
    else
    {
        svm_file_source.restore_filepath_source = SCALE_ACCORD_TO_FILE;
        //svm_task.restore_filepath_source = svm_file_source.restore_filepath_source;
    }
}
//radioButton_3被选中时，svm_train.train_data_scaled_filepath:一行将被禁能
void Widget::train_data_scaled_filepath(bool state)
{
    ui->lineEdit_5->setEnabled(!state);
    ui->pushButton_5->setEnabled(!state);

    if(state)
        svm_file_source.train_data_scaled_filepath_source = FILE_FROM_LAST_TASK;
    else
        svm_file_source.train_data_scaled_filepath_source = FILE_FROM_OTHER_FILE;

}
//radioButton_7被选中时，svm_predict.model_filepath一行将被禁能
void Widget::model_filepath(bool state)
{
    ui->lineEdit_7->setEnabled(!state);
    ui->pushButton_6->setEnabled(!state);

    if(state)
        svm_file_source.model_filepath_source = FILE_FROM_LAST_TASK;
    else
        svm_file_source.model_filepath_source = FILE_FROM_OTHER_FILE;
}
//radioButton_5被选中时，test_data_scaled_filepath一行将被禁能
void Widget::test_data_scaled_filepath(bool state)
{
    ui->lineEdit_8->setEnabled(!state);
    ui->pushButton_7->setEnabled(!state);

    if(state)
        svm_file_source.test_data_scaled_filepath_source = FILE_FROM_LAST_TASK;
    else
        svm_file_source.test_data_scaled_filepath_source = FILE_FROM_OTHER_FILE;
}
//根据combox的当前QString(C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR)禁能一些不需要参数配置
void Widget::svm_type_para_enbale(int index)
{
    if(index == 0)//当前选择的是C_SVC, ,那么C有效
    {
        ui->doubleSpinBox_5->setEnabled(true);//C
        ui->doubleSpinBox_6->setEnabled(false);//p
        ui->doubleSpinBox_7->setEnabled(false);//nu
        ui->lineEdit_6->setEnabled(true);//weight and lable
        ui->spinBox_2->setEnabled(true);//nr_weight
    }
    else if(index == 1)//当前选择的是NU_SVC,
    {
        ui->doubleSpinBox_5->setEnabled(false);//C
        ui->doubleSpinBox_6->setEnabled(false);//p
        ui->doubleSpinBox_7->setEnabled(true);//nu
        ui->lineEdit_6->setEnabled(false);//weight and lable
        ui->spinBox_2->setEnabled(false);//nr_weigh
    }
    else if(index == 2)//当前选择的是ONE_CLASS,,
    {
        ui->doubleSpinBox_5->setEnabled(false);//C
        ui->doubleSpinBox_6->setEnabled(false);//p
        ui->doubleSpinBox_7->setEnabled(true);//nu
        ui->lineEdit_6->setEnabled(false);//weight and lable
        ui->spinBox_2->setEnabled(false);//nr_weigh

        /* 当选择ONE_CLASS
            -b probability_estimates: whether to predict probability
               estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported */
        Para_svm_predict.predict_probability = 0;
        ui->spinBox_7->setValue(Para_svm_predict.predict_probability);
    }
    else if(index == 3)//当前选择的是EPSILON_SVR
    {
        ui->doubleSpinBox_5->setEnabled(true);//C
        ui->doubleSpinBox_6->setEnabled(true);//p
        ui->doubleSpinBox_7->setEnabled(false);//nu
        ui->lineEdit_6->setEnabled(false);//weight and lable
        ui->spinBox_2->setEnabled(false);//nr_weigh
    }
    else if(index == 4)//当前选择的是NU_SVR,
    {
        ui->doubleSpinBox_5->setEnabled(true);//C
        ui->doubleSpinBox_6->setEnabled(false);//p
        ui->doubleSpinBox_7->setEnabled(true);//nu
        ui->lineEdit_6->setEnabled(false);//weight and lable
        ui->spinBox_2->setEnabled(false);//nr_weigh
    }
}
void Widget::kernel_type_para_enbale(int index)
{
    if(index == 0 || index == 4)//当前选择的是LINEAR
    {
        ui->spinBox->setEnabled(false);//degree
        ui->doubleSpinBox_8->setEnabled(false);//gamma
        ui->doubleSpinBox_9->setEnabled(false);//coef0
    }
    else if(index == 1)
    {
        ui->spinBox->setEnabled(true);//degree
        ui->doubleSpinBox_8->setEnabled(true);//gamma
        ui->doubleSpinBox_9->setEnabled(true);//coef0
    }
    else if(index == 2)
    {
        ui->spinBox->setEnabled(false);//degree
        ui->doubleSpinBox_8->setEnabled(true);//gamma
        ui->doubleSpinBox_9->setEnabled(false);//coef0
    }
    else if(index == 3)
    {
        ui->spinBox->setEnabled(false);//degree
        ui->doubleSpinBox_8->setEnabled(true);//gamma
        ui->doubleSpinBox_9->setEnabled(true);//coef0
    }

}
void Widget::on_pushButton_11_clicked()
{
    set_parameters_to_default();
    ui->textBrowser->clear();
    ui->textBrowser->append(tr("提示：界面参数已重置，历史输出被清空。"));
}
//程序退出按钮
void Widget::on_pushButton_13_clicked()
{
    dataprocess_thread->stop();
    while(dataprocess_thread->isRunning());//等待数据处理线程退出

    close();//关闭滑动条以及其内的所有部件

    QApplication *p;//退出应用程序
    p->quit();
}
//svm_scale.train_data_filepath:文件路劲选择
void Widget::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("缩放模块：请选择一个训练数据文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("训练数据文件路径:") + fileName);
    ui->lineEdit->setText(fileName);
}
//svm_scale.test_data_filepath:文件路劲选择
void Widget::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("缩放模块：请选择一个测试数据文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("测试数据文件路径:") + fileName);
    ui->lineEdit_2->setText(fileName);
}

void Widget::on_pushButton_4_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("缩放模块：请选择一个缩放规则文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("缩放规则文件路径:") + fileName);
    ui->lineEdit_4->setText(fileName);
}

void Widget::on_pushButton_5_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("训练模块：请选择一个训练样本文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("训练样本文件路径:") + fileName);
    ui->lineEdit_5->setText(fileName);
}

void Widget::on_pushButton_6_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("测试模块：请选择一个分类器模型文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("分类器模型文件路径:") + fileName);
    ui->lineEdit_7->setText(fileName);
}

void Widget::on_pushButton_7_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("测试模块：请选择一个测试样本文件"), tr("C:\\"),tr("文本文件(*)"));
    fileName.replace('/', "\\");
    ui->textBrowser->append(tr("测试样本文件路径:") + fileName);
    ui->lineEdit_8->setText(fileName);
}
//开始按钮槽函数
/*
    点击开始按钮后，GUI线程读取界面上的所有参数，并将其保存到临时文件svm_para.txt中，
由GUI线程发送信号给数据处理线程后，数据线程开始读取参数执行任务。
*/
void Widget::on_pushButton_12_clicked()
{
    QString num_temp;

    /* 读取全部的界面参数 */
    read_parameters_from_interface();

    /* 解析各个结构体（SVM_ALL_INPUT_FILEPATH_ON_INTERFACE、SVM_FILE_SOURCE）中的数据，
     * 并将最后的算法能够接受的数据填写到SVM_ALL_FILEPATH、PARA_SVM_SCALE、PARA_SVM_TRAIN、
     * PARA_SVM_PREDICT、PARA_GRID_SEARCH、PARA_ACO_SEARCH中 */
    if(parse_svm_parameters() == SUCCESS)
    {
        ui->textBrowser->append(tr("提示：参数检查基本通过，无设置错误。但不保证所有参数绝对正确！\n"));
        emit send_to_datapro_svm_task(svm_task);

        /* 禁能开始按钮一直到数据线程梳理数据完成 */
        ui->pushButton_12->setEnabled(false);
    }
    else
        ui->textBrowser->append(tr("错误：参数解析出错，请检查参数！\n"));

}

/* 读取全部的界面参数 */
void Widget::read_parameters_from_interface()
{
    /* 读取数据文件名称以及一些字符串形式的输入，这部分参数再输入到算法之前要根据svm_file_source结构体中的内容进行解析 */
    svm_all_filepath_on_interface.train_data_filepath = ui->lineEdit->displayText();
    svm_all_filepath_on_interface.test_data_filepath = ui->lineEdit_2->displayText();
    svm_all_filepath_on_interface.restore_filename = ui->lineEdit_4->displayText();
    svm_all_filepath_on_interface.train_data_scaled_filepath = ui->lineEdit_5->displayText();
    svm_all_filepath_on_interface.model_filepath = ui->lineEdit_7->displayText();
    svm_all_filepath_on_interface.test_data_scaled_filepath = ui->lineEdit_8->displayText();

    svm_all_filepath_on_interface.label_and_weight = ui->lineEdit_6->displayText();
    svm_all_filepath_on_interface.empiri_best_path = ui->lineEdit_9->displayText();

    /* 读取界面参数 */
    Para_svm_scale.l = ui->doubleSpinBox->value();
    Para_svm_scale.u = ui->doubleSpinBox_2->value();
    Para_svm_scale.y_scaling = 0;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_scale.y_lower = ui->doubleSpinBox_4->value();
    Para_svm_scale.y_upper = ui->doubleSpinBox_3->value();
    Para_svm_scale.data_set = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_scale.save_filename = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_scale.restore_filename = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_scale.result_filename = NULL;//输出结果文件用户无法设置，由程序内部设定

    Para_svm_train.svm_train_parameter.svm_type = ui->comboBox->currentIndex();//C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR,选择一个参数
    Para_svm_train.svm_train_parameter.kernel_type = ui->comboBox_2->currentIndex();//LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED, 选择一个参数
    Para_svm_train.svm_train_parameter.degree = ui->spinBox->value();
    Para_svm_train.svm_train_parameter.gamma = ui->doubleSpinBox_8->value();//0.1;//0代表默认
    Para_svm_train.svm_train_parameter.coef0 = ui->doubleSpinBox_9->value();
    Para_svm_train.svm_train_parameter.nu = ui->doubleSpinBox_7->value();//
    Para_svm_train.svm_train_parameter.cache_size = ui->doubleSpinBox_10->value();
    Para_svm_train.svm_train_parameter.C = 	ui->doubleSpinBox_5->value();//1;
    Para_svm_train.svm_train_parameter.eps = ui->doubleSpinBox_11->value();
    Para_svm_train.svm_train_parameter.p = ui->doubleSpinBox_6->value();
    Para_svm_train.svm_train_parameter.shrinking = ui->spinBox_3->value();
    Para_svm_train.svm_train_parameter.probability = ui->spinBox_4->value();
    Para_svm_train.svm_train_parameter.nr_weight = ui->spinBox_2->value();//0表示不设置权重，n表示有n个类别要设置权重系数，n个类别以及对应的权重系数由本文件开头处的数组weight_label和weight设定,注意其数组长度必须和n相等
    Para_svm_train.svm_train_parameter.weight_label = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_train.svm_train_parameter.weight = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_train.quiet_mode = ui->spinBox_6->value();//0: outputs;  !0: no outputs
    Para_svm_train.cross_validation = ui->spinBox_5->value();// 5;// 0;//输入0表示不进行交叉验证，输入n表示进行n折交叉验证（注意n必须大于等于2）
    Para_svm_train.training_set_file = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_train.model_file = NULL;//输出结果文件用户无法设置，由程序内部设定

    /* 配置预测参数 */
    Para_svm_predict.predict_probability = ui->spinBox_7->value();//probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported
    Para_svm_predict.quiet_mode = ui->spinBox_8->value();//0: outputs;  !0: no outputs
    Para_svm_predict.test_file = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_predict.model_file = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_svm_predict.output_file = NULL;//输出结果文件用户无法设置，由程序内部设定

    /* 格点搜索参数配置 */
    Para_grid_search.d1_begin = ui->doubleSpinBox_12->value();//底数为2，下同,搜索参数时第一个参数d1必须是核函数参数g，第二个参数d2是分类器参数(nu or C)
    Para_grid_search.d1_end = ui->doubleSpinBox_14->value();//10;
    Para_grid_search.d1_step = ui->doubleSpinBox_13->value();
    Para_grid_search.d2_begin = ui->doubleSpinBox_15->value();//注意nu参数取值范围（0,1）开区间
    Para_grid_search.d2_end = ui->doubleSpinBox_17->value();
    Para_grid_search.d2_step = ui->doubleSpinBox_16->value();//1;
    Para_grid_search.v_fold = ui->spinBox_9->value();
    Para_grid_search.flag_predict = ui->spinBox_10->value();// 1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_grid_search.output_file = NULL;//输出结果文件用户无法设置，由程序内部设定

    /* 蚁群搜索参数配置 */
    Para_aco_search.ants_amount = ui->spinBox_11->value();
    Para_aco_search.x_aixs_lines = ui->spinBox_12->value();
    Para_aco_search.y_aixs_values = ui->spinBox_13->value();
    Para_aco_search.max_iterations = ui->spinBox_14->value();
    Para_aco_search.pheromone_initial_value = ui->doubleSpinBox_18->value();
    Para_aco_search.alpha = ui->doubleSpinBox_19->value();
    Para_aco_search.beta = ui->doubleSpinBox_20->value();
    Para_aco_search.rho = ui->doubleSpinBox_21->value();
    Para_aco_search.Q = ui->doubleSpinBox_25->value();
    Para_aco_search.flag_predict = ui->spinBox_15->value();//1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_aco_search.v_fold = ui->spinBox_16->value();
    Para_aco_search.MAX_TAU = ui->doubleSpinBox_22->value();
    Para_aco_search.MIN_TAU = ui->doubleSpinBox_23->value();
    Para_aco_search.ERR = ui->doubleSpinBox_24->value();
    Para_aco_search.best_path = NULL;//此变量此处的设置无效，已由其他变量代为传递
    Para_aco_search.output_file = NULL;//输出结果文件用户无法设置，由程序内部设定
    //经验最优路径，小数点必须有2个，逗号必须有1个，其分割位置确定了搜索参数的哪些位;小数点前面没有数字表示，该参数仅仅搜索小数点后面的数字
    //每一个数位上的数字表达了最优路径
}

/* 将全部的界面参数记录到文件svm_parameter.txt */
int Widget::write_parameters_to_file(char *savefile)
{
    /* 1、记录将要执行的任务svm_task结构体， 还需要在数据处理线程中进行解析 */
    /* 2、记录数据文件来源信息svm_file_source结构体，还需要在数据处理线程中进行解析 */
    /* 3、记录界面中字符串形式的输入svm_all_filepath_on_interface结构体，主要是文件名参数，还需要在数据处理线程中进行解析 */
    /* 4、记录界面中所有整数以及浮点数控件的输入Para_svm_scale、Para_svm_train、Para_svm_predict、Para_grid_search、Para_aco_search */

    FILE *fp;
    QByteArray ba;
    char *str;

    if((fp = fopen(savefile, "w")) != NULL)
    {
        /* 1、记录将要执行的任务svm_task结构体 */
        fprintf(fp, "svm_task.svm_scale_task = %d\n", svm_task.svm_scale_task);
        fprintf(fp, "svm_task.svm_train_task = %d\n", svm_task.svm_train_task);
        fprintf(fp, "svm_task.svm_predict_task = %d\n", svm_task.svm_predict_task);
        fprintf(fp, "svm_task.svm_grid_search_task = %d\n", svm_task.svm_grid_search_task);
        fprintf(fp, "svm_task.svm_aco_search_task = %d\n\n", svm_task.svm_aco_search_task);

        /* 2、记录数据文件来源信息svm_file_source结构体 */
        fprintf(fp, "svm_file_source.y_scaling_flag = %d\n", svm_file_source.y_scaling_flag);
        fprintf(fp, "svm_file_source.restore_filepath_source = %d\n", svm_file_source.restore_filepath_source);
        fprintf(fp, "svm_file_source.train_data_scaled_filepath_source = %d\n", svm_file_source.train_data_scaled_filepath_source);
        fprintf(fp, "svm_file_source.model_filepath_source = %d\n", svm_file_source.model_filepath_source);
        fprintf(fp, "svm_file_source.test_data_scaled_filepath_source = %d\n\n", svm_file_source.test_data_scaled_filepath_source);

        /* 3、记录界面中字符串形式的输入svm_all_filepath_on_interface结构体，主要是文件名参数 */
        if(svm_all_filepath_on_interface.train_data_filepath.length() == 0)
        {
            svm_all_filepath_on_interface.train_data_filepath = "NULL";
        }
        ba = svm_all_filepath_on_interface.train_data_filepath.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.train_data_filepath = %s\n", str);

        if(svm_all_filepath_on_interface.test_data_filepath.length() == 0)
        {
            svm_all_filepath_on_interface.test_data_filepath = "NULL";
        }
        ba = svm_all_filepath_on_interface.test_data_filepath.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.test_data_filepath = %s\n", str);

        if(svm_all_filepath_on_interface.restore_filename.length() == 0)
        {
            svm_all_filepath_on_interface.restore_filename = "NULL";
        }
        ba = svm_all_filepath_on_interface.restore_filename.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.restore_filename = %s\n", str);

        if(svm_all_filepath_on_interface.train_data_scaled_filepath.length() == 0)
        {
            svm_all_filepath_on_interface.train_data_scaled_filepath = "NULL";
        }
        ba = svm_all_filepath_on_interface.train_data_scaled_filepath.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.train_data_scaled_filepath = %s\n", str);

        if(svm_all_filepath_on_interface.model_filepath.length() == 0)
        {
            svm_all_filepath_on_interface.model_filepath = "NULL";
        }
        ba = svm_all_filepath_on_interface.model_filepath.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.model_filepath = %s\n", str);

        if(svm_all_filepath_on_interface.test_data_scaled_filepath.length() == 0)
        {
            svm_all_filepath_on_interface.test_data_scaled_filepath = "NULL";
        }
        ba = svm_all_filepath_on_interface.test_data_scaled_filepath.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.test_data_scaled_filepath = %s\n", str);

        if(svm_all_filepath_on_interface.label_and_weight.length() == 0)
        {
            svm_all_filepath_on_interface.label_and_weight = "NULL";
        }
        ba = svm_all_filepath_on_interface.label_and_weight.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.label_and_weight = %s\n", str);

        if(svm_all_filepath_on_interface.empiri_best_path.length() == 0)
        {
            svm_all_filepath_on_interface.empiri_best_path = "NULL";
        }
        ba = svm_all_filepath_on_interface.empiri_best_path.toLatin1();
        str = ba.data();
        fprintf(fp, "svm_all_filepath_on_interface.empiri_best_path = %s\n\n", str);

        /* 4、记录界面中所有整数以及浮点数控件的输入Para_svm_scale、Para_svm_train、Para_svm_predict、Para_grid_search、Para_aco_search */
        fprintf(fp, "Para_svm_scale.l = %f\n", Para_svm_scale.l);
        fprintf(fp, "Para_svm_scale.u = %f\n", Para_svm_scale.u);
        fprintf(fp, "Para_svm_scale.y_scaling = %d\n", Para_svm_scale.y_scaling);
        fprintf(fp, "Para_svm_scale.y_lower = %f\n", Para_svm_scale.y_lower);
        fprintf(fp, "Para_svm_scale.y_upper = %f\n", Para_svm_scale.y_upper);
        fprintf(fp, "Para_svm_scale.data_set = %s\n", Para_svm_scale.data_set);
        fprintf(fp, "Para_svm_scale.save_filename = %s\n", Para_svm_scale.save_filename);
        fprintf(fp, "Para_svm_scale.restore_filename = %s\n", Para_svm_scale.restore_filename);
        fprintf(fp, "Para_svm_scale.result_filename = %s\n\n", Para_svm_scale.result_filename);

        fprintf(fp, "Para_svm_train.svm_train_parameter.svm_type = %d\n", Para_svm_train.svm_train_parameter.svm_type);
        fprintf(fp, "Para_svm_train.svm_train_parameter.kernel_type = %d\n", Para_svm_train.svm_train_parameter.kernel_type);
        fprintf(fp, "Para_svm_train.svm_train_parameter.degree = %d\n", Para_svm_train.svm_train_parameter.degree);
        fprintf(fp, "Para_svm_train.svm_train_parameter.gamma = %f\n", Para_svm_train.svm_train_parameter.gamma);
        fprintf(fp, "Para_svm_train.svm_train_parameter.coef0 = %f\n", Para_svm_train.svm_train_parameter.coef0);
        fprintf(fp, "Para_svm_train.svm_train_parameter.cache_size = %f\n", Para_svm_train.svm_train_parameter.cache_size);
        fprintf(fp, "Para_svm_train.svm_train_parameter.eps = %f\n", Para_svm_train.svm_train_parameter.eps);
        fprintf(fp, "Para_svm_train.svm_train_parameter.C = %f\n", Para_svm_train.svm_train_parameter.C);
        fprintf(fp, "Para_svm_train.svm_train_parameter.nr_weight = %d\n", Para_svm_train.svm_train_parameter.nr_weight);
        fprintf(fp, "Para_svm_train.svm_train_parameter.weight_label = %d\n", Para_svm_train.svm_train_parameter.weight_label);
        fprintf(fp, "Para_svm_train.svm_train_parameter.weight = %f\n", Para_svm_train.svm_train_parameter.weight);
        fprintf(fp, "Para_svm_train.svm_train_parameter.nu = %f\n", Para_svm_train.svm_train_parameter.nu);
        fprintf(fp, "Para_svm_train.svm_train_parameter.p = %f\n", Para_svm_train.svm_train_parameter.p);
        fprintf(fp, "Para_svm_train.svm_train_parameter.shrinking = %d\n", Para_svm_train.svm_train_parameter.shrinking);
        fprintf(fp, "Para_svm_train.svm_train_parameter.probability = %d\n", Para_svm_train.svm_train_parameter.probability);
        fprintf(fp, "Para_svm_train.quiet_mode = %d\n", Para_svm_train.quiet_mode);
        fprintf(fp, "Para_svm_train.cross_validation = %d\n", Para_svm_train.cross_validation);
        fprintf(fp, "Para_svm_train.training_set_file = %s\n", Para_svm_train.training_set_file);
        fprintf(fp, "Para_svm_train.model_file = %s\n\n", Para_svm_train.model_file);

        fprintf(fp, "Para_svm_predict.predict_probability = %d\n", Para_svm_predict.predict_probability);
        fprintf(fp, "Para_svm_predict.quiet_mode = %d\n", Para_svm_predict.quiet_mode);
        fprintf(fp, "Para_svm_predict.test_file = %s\n", Para_svm_predict.test_file);
        fprintf(fp, "Para_svm_predict.model_file = %s\n", Para_svm_predict.model_file);
        fprintf(fp, "Para_svm_predict.output_file = %s\n\n", Para_svm_predict.output_file);

        fprintf(fp, "Para_grid_search.d1_begin = %f\n", Para_grid_search.d1_begin);
        fprintf(fp, "Para_grid_search.d1_end = %f\n", Para_grid_search.d1_end);
        fprintf(fp, "Para_grid_search.d1_step = %f\n", Para_grid_search.d1_step);
        fprintf(fp, "Para_grid_search.d2_begin = %f\n", Para_grid_search.d2_begin);
        fprintf(fp, "Para_grid_search.d2_end = %f\n", Para_grid_search.d2_end);
        fprintf(fp, "Para_grid_search.d2_step = %f\n", Para_grid_search.d2_step);
        fprintf(fp, "Para_grid_search.v_fold = %d\n", Para_grid_search.v_fold);
        fprintf(fp, "Para_grid_search.flag_predict = %d\n", Para_grid_search.flag_predict);
        fprintf(fp, "Para_grid_search.output_file = %s\n\n", Para_grid_search.output_file);

        fprintf(fp, "Para_aco_search.ants_amount = %d\n", Para_aco_search.ants_amount);
        fprintf(fp, "Para_aco_search.x_aixs_lines = %d\n", Para_aco_search.x_aixs_lines);
        fprintf(fp, "Para_aco_search.y_aixs_values = %d\n", Para_aco_search.y_aixs_values);
        fprintf(fp, "Para_aco_search.max_iterations = %d\n", Para_aco_search.max_iterations);
        fprintf(fp, "Para_aco_search.pheromone_initial_value = %f\n", Para_aco_search.pheromone_initial_value);
        fprintf(fp, "Para_aco_search.alpha = %f\n", Para_aco_search.alpha);
        fprintf(fp, "Para_aco_search.beta = %f\n", Para_aco_search.beta);
        fprintf(fp, "Para_aco_search.rho = %f\n", Para_aco_search.rho);
        fprintf(fp, "Para_aco_search.Q = %f\n", Para_aco_search.Q);
        fprintf(fp, "Para_aco_search.flag_predict = %d\n", Para_aco_search.flag_predict);
        fprintf(fp, "Para_aco_search.v_fold = %d\n", Para_aco_search.v_fold);
        fprintf(fp, "Para_aco_search.MAX_TAU = %f\n", Para_aco_search.MAX_TAU);
        fprintf(fp, "Para_aco_search.MIN_TAU = %f\n", Para_aco_search.MIN_TAU);
        fprintf(fp, "Para_aco_search.ERR = %f\n", Para_aco_search.ERR);
        fprintf(fp, "Para_aco_search.best_path = %s\n", Para_aco_search.best_path);
        fprintf(fp, "Para_aco_search.output_file = %s\n\n", Para_aco_search.output_file);

        fclose(fp);
        fp = NULL;
    }
    else
    {
        ui->textBrowser->append(tr("错误：将界面参数写入到文件失败！"));
        return ERROR;
    }

    return SUCCESS;
}

//显示输出到文本浏览器-槽函数
void Widget::recei_from_datapro_display_output(QString str)
{
    ui->textBrowser->append(str);
}

//保存参数到文件，按钮槽函数
void Widget::on_pushButton_9_clicked()
{
    QByteArray ba;
    char *savefile;
    char temp_str[1024] = {0};
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, tr("保存参数文件到"), tr("C:\\svm_parameters.txt"), tr("文本文件 (*)"));

    fileName.replace('/', "\\");

    if(fileName != NULL)//获取的文件路劲为空，则表明用户取消了文件的保存
    {
        ba = fileName.toLatin1();
        savefile = ba.data();

        /* 读取全部的界面参数 */
        read_parameters_from_interface();

        /* 将全部的界面参数记录到文件svm_parameter.txt */
        if(write_parameters_to_file(savefile) == SUCCESS)
        {
            sprintf(temp_str, "提示：保存界面参数到文件成功，参数保存于 %s\n", savefile);
            ui->textBrowser->append(tr(temp_str));
        }
        else
        {
            ui->textBrowser->append(tr("错误：保存界面参数到文件出错！"));
        }
    }
    else
        ui->textBrowser->append(tr("提示：取消保存界面参数。"));
}

//从文件读取界面参数
void Widget::on_pushButton_10_clicked()
{
    QByteArray ba;
    char *savefile;
    char temp_str[1024] = {0};
    QString fileName = QFileDialog::getOpenFileName(this,tr("请选择一个参数文件"), tr("C:\\"),tr("文本文件(*)"));

    fileName.replace('/', "\\");

    if(fileName != NULL)
    {
        ba = fileName.toLatin1();
        savefile = ba.data();

        if(read_parameters_from_file(savefile) == SUCCESS)
        {
            sprintf(temp_str, "提示：从文件读取参数成功，参数来源于文件 %s\n", savefile);
            ui->textBrowser->append(tr(temp_str));
        }
        else
        {
            ui->textBrowser->append(tr("错误：从文件读取界面参数出错！"));
        }
    }
    else
        ui->textBrowser->append(tr("提示：取消从文件读取界面参数。"));

}

/* 从文件读取全部参数直接保存到界面控件中 */
int Widget::read_parameters_from_file(char *savefile)
{
    /* 1、读取svm_task结构体， 将其解析并设置到界面上 */
    /* 2、读取数据文件来源信息svm_file_source结构体，将其解析并设置到界面上 */
    /* 3、读取svm_all_filepath_on_interface结构体，主要是文件名参数，将其解析并设置到界面上 */
    /* 4、读取界面中所有整数以及浮点数控件的输入Para_svm_scale、Para_svm_train、Para_svm_predict、Para_grid_search、Para_aco_search，将其解析并设置到界面上 */

    FILE *fp;
    int temp_int = 0;
    float temp_float = 0.0;
    float temp_float_array[10] = {0.0};
    char temp_str[MAX_STR_LEN] = {0};

    if((fp = fopen(savefile, "r")) != NULL)
    {
        /* 1、读取svm_task结构体， 将其解析并设置到界面上 */
        fscanf(fp, "svm_task.svm_scale_task = %d\n", &temp_int);
        if(temp_int)
            ui->checkBox_2->setChecked(true);
        else
            ui->checkBox_2->setChecked(false);
        fscanf(fp, "svm_task.svm_train_task = %d\n", &temp_int);
        if(temp_int)
            ui->checkBox_3->setChecked(true);
        else
            ui->checkBox_3->setChecked(false);
        fscanf(fp, "svm_task.svm_predict_task = %d\n", &temp_int);
        if(temp_int)
            ui->checkBox_4->setChecked(true);
        else
            ui->checkBox_4->setChecked(false);
        fscanf(fp, "svm_task.svm_grid_search_task = %d\n", &temp_int);
        if(temp_int)
            ui->checkBox_6->setChecked(true);
        else
            ui->checkBox_6->setChecked(false);
        fscanf(fp, "svm_task.svm_aco_search_task = %d\n\n", &temp_int);
        if(temp_int)
            ui->checkBox_7->setChecked(true);
        else
            ui->checkBox_7->setChecked(false);

        /* 2、读取数据文件来源信息svm_file_source结构体，将其解析并设置到界面上 */
        fscanf(fp, "svm_file_source.y_scaling_flag = %d\n", &temp_int);
        if(temp_int == Y_SCALING_NO)
        {
            ui->checkBox->setChecked(false);
        }
        else if(temp_int == Y_SCALING_YES)
        {
            ui->checkBox->setChecked(true);
        }

        fscanf(fp, "svm_file_source.restore_filepath_source = %d\n", &temp_int);
        if(temp_int == SCALE_ACCORD_TO_PARA)
        {
            ui->radioButton->setChecked(true);
        }
        else if(temp_int == SCALE_ACCORD_TO_FILE)
        {
            ui->radioButton_2->setChecked(true);
        }

        fscanf(fp, "svm_file_source.train_data_scaled_filepath_source = %d\n", &temp_int);
        if(temp_int == FILE_FROM_LAST_TASK)
        {
            ui->radioButton_3->setChecked(true);
        }
        else if(temp_int == FILE_FROM_OTHER_FILE)
        {
            ui->radioButton_4->setChecked(true);
        }

        fscanf(fp, "svm_file_source.model_filepath_source = %d\n", &temp_int);
        if(temp_int == FILE_FROM_LAST_TASK)
        {
            ui->radioButton_7->setChecked(true);
        }
        else if(temp_int == FILE_FROM_OTHER_FILE)
        {
            ui->radioButton_8->setChecked(true);
        }

        fscanf(fp, "svm_file_source.test_data_scaled_filepath_source = %d\n\n", &temp_int);
        if(temp_int == FILE_FROM_LAST_TASK)
        {
            ui->radioButton_5->setChecked(true);
        }
        else if(temp_int == FILE_FROM_OTHER_FILE)
        {
            ui->radioButton_6->setChecked(true);
        }

        /* 3、读取svm_all_filepath_on_interface结构体，主要是文件名参数，将其解析并设置到界面上 */
        fscanf(fp, "svm_all_filepath_on_interface.train_data_filepath = %s\n", temp_str);
        ui->lineEdit->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.test_data_filepath = %s\n", temp_str);
        ui->lineEdit_2->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.restore_filename = %s\n", temp_str);
        ui->lineEdit_4->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.train_data_scaled_filepath = %s\n", temp_str);
        ui->lineEdit_5->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.model_filepath = %s\n", temp_str);
        ui->lineEdit_7->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.test_data_scaled_filepath = %s\n", temp_str);
        ui->lineEdit_8->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.label_and_weight = %s\n", temp_str);
        ui->lineEdit_6->setText(temp_str);

        fscanf(fp, "svm_all_filepath_on_interface.empiri_best_path = %s\n\n", temp_str);
        ui->lineEdit_9->setText(temp_str);

        /* 4、读取界面中所有整数以及浮点数控件的输入Para_svm_scale、Para_svm_train、Para_svm_predict、Para_grid_search、Para_aco_search，将其解析并设置到界面上 */
        fscanf(fp, "Para_svm_scale.l = %f\n", &temp_float);
        ui->doubleSpinBox->setValue(temp_float);

        fscanf(fp, "Para_svm_scale.u = %f\n", &temp_float);
        ui->doubleSpinBox_2->setValue(temp_float);

        fscanf(fp, "Para_svm_scale.y_scaling = %d\n", &temp_int);
        //此处的数据已由其他变量（svm_file_source）代为传递，

        fscanf(fp, "Para_svm_scale.y_lower = %f\n", &temp_float);
        ui->doubleSpinBox_4->setValue(temp_float);

        fscanf(fp, "Para_svm_scale.y_upper = %f\n", &temp_float);
        ui->doubleSpinBox_3->setValue(temp_float);

        fscanf(fp, "Para_svm_scale.data_set = %s\n", temp_str);
        //此处的数据已由其他变量（svm_file_source）代为传递，

        fscanf(fp, "Para_svm_scale.save_filename = %s\n", temp_str);
        //此处的数据已由其他变量（svm_file_source）代为传递，

        fscanf(fp, "Para_svm_scale.restore_filename = %s\n", temp_str);
        //此处的数据已由其他变量（svm_file_source）代为传递，

        fscanf(fp, "Para_svm_scale.result_filename = %s\n\n", temp_str);
        //输出结果文件路径不保存到界面

        fscanf(fp, "Para_svm_train.svm_train_parameter.svm_type = %d\n", &temp_int);
        ui->comboBox->setCurrentIndex(temp_int);
        fscanf(fp, "Para_svm_train.svm_train_parameter.kernel_type = %d\n", &temp_int);
        ui->comboBox_2->setCurrentIndex(temp_int);
        fscanf(fp, "Para_svm_train.svm_train_parameter.degree = %d\n", &temp_int);
        ui->spinBox->setValue(temp_int);
        fscanf(fp, "Para_svm_train.svm_train_parameter.gamma = %f\n", &temp_float);
        ui->doubleSpinBox_8->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.coef0 = %f\n", &temp_float);
        ui->doubleSpinBox_9->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.cache_size = %f\n", &temp_float);
        ui->doubleSpinBox_10->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.eps = %f\n", &temp_float);
        ui->doubleSpinBox_11->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.C = %f\n", &temp_float);
        ui->doubleSpinBox_5->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.nr_weight = %d\n", &temp_int);
        ui->spinBox_2->setValue(temp_int);
        fscanf(fp, "Para_svm_train.svm_train_parameter.weight_label = %d\n", &temp_int);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_svm_train.svm_train_parameter.weight = %f\n", temp_float_array);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_svm_train.svm_train_parameter.nu = %f\n", &temp_float);
        ui->doubleSpinBox_7->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.p = %f\n", &temp_float);
        ui->doubleSpinBox_6->setValue(temp_float);
        fscanf(fp, "Para_svm_train.svm_train_parameter.shrinking = %d\n", &temp_int);
        ui->spinBox_3->setValue(temp_int);
        fscanf(fp, "Para_svm_train.svm_train_parameter.probability = %d\n", &temp_int);
        ui->spinBox_4->setValue(temp_int);
        fscanf(fp, "Para_svm_train.quiet_mode = %d\n", &temp_int);
        ui->spinBox_6->setValue(temp_int);
        fscanf(fp, "Para_svm_train.cross_validation = %d\n", &temp_int);
        ui->spinBox_5->setValue(temp_int);
        fscanf(fp, "Para_svm_train.training_set_file = %s\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_svm_train.model_file = %s\n\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，

        fscanf(fp, "Para_svm_predict.predict_probability = %d\n", &temp_int);
        ui->spinBox_7->setValue(temp_int);
        fscanf(fp, "Para_svm_predict.quiet_mode = %d\n", &temp_int);
        ui->spinBox_8->setValue(temp_int);
        fscanf(fp, "Para_svm_predict.test_file = %s\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_svm_predict.model_file = %s\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_svm_predict.output_file = %s\n\n", temp_str);
        //输出结果文件路径不保存到界面

        fscanf(fp, "Para_grid_search.d1_begin = %f\n", &temp_float);
        ui->doubleSpinBox_12->setValue(temp_float);
        fscanf(fp, "Para_grid_search.d1_end = %f\n", &temp_float);
        ui->doubleSpinBox_14->setValue(temp_float);
        fscanf(fp, "Para_grid_search.d1_step = %f\n", &temp_float);
        ui->doubleSpinBox_13->setValue(temp_float);
        fscanf(fp, "Para_grid_search.d2_begin = %f\n", &temp_float);
        ui->doubleSpinBox_15->setValue(temp_float);
        fscanf(fp, "Para_grid_search.d2_end = %f\n", &temp_float);
        ui->doubleSpinBox_17->setValue(temp_float);
        fscanf(fp, "Para_grid_search.d2_step = %f\n", &temp_float);
        ui->doubleSpinBox_16->setValue(temp_float);
        fscanf(fp, "Para_grid_search.v_fold = %d\n", &temp_int);
        ui->spinBox_9->setValue(temp_int);
        fscanf(fp, "Para_grid_search.flag_predict = %d\n", &temp_int);
        ui->spinBox_10->setValue(temp_int);
        fscanf(fp, "Para_grid_search.output_file = %s\n\n", temp_str);
        //输出结果文件路径不保存到界面

        fscanf(fp, "Para_aco_search.ants_amount = %d\n", &temp_int);
        ui->spinBox_11->setValue(temp_int);
        fscanf(fp, "Para_aco_search.x_aixs_lines = %d\n", &temp_int);
        ui->spinBox_12->setValue(temp_int);
        fscanf(fp, "Para_aco_search.y_aixs_values = %d\n", &temp_int);
        ui->spinBox_13->setValue(temp_int);
        fscanf(fp, "Para_aco_search.max_iterations = %d\n", &temp_int);
        ui->spinBox_14->setValue(temp_int);
        fscanf(fp, "Para_aco_search.pheromone_initial_value = %f\n", &temp_float);
        ui->doubleSpinBox_18->setValue(temp_float);
        fscanf(fp, "Para_aco_search.alpha = %f\n", &temp_float);
        ui->doubleSpinBox_19->setValue(temp_float);
        fscanf(fp, "Para_aco_search.beta = %f\n", &temp_float);
        ui->doubleSpinBox_20->setValue(temp_float);
        fscanf(fp, "Para_aco_search.rho = %f\n", &temp_float);
        ui->doubleSpinBox_21->setValue(temp_float);
        fscanf(fp, "Para_aco_search.Q = %f\n", &temp_float);
        ui->doubleSpinBox_25->setValue(temp_float);
        fscanf(fp, "Para_aco_search.flag_predict = %d\n", &temp_int);
        ui->spinBox_15->setValue(temp_int);
        fscanf(fp, "Para_aco_search.v_fold = %d\n", &temp_int);
        ui->spinBox_16->setValue(temp_int);
        fscanf(fp, "Para_aco_search.MAX_TAU = %f\n", &temp_float);
        ui->doubleSpinBox_22->setValue(temp_float);
        fscanf(fp, "Para_aco_search.MIN_TAU = %f\n", &temp_float);
        ui->doubleSpinBox_23->setValue(temp_float);
        fscanf(fp, "Para_aco_search.ERR = %f\n", &temp_float);
        ui->doubleSpinBox_24->setValue(temp_float);
        fscanf(fp, "Para_aco_search.best_path = %s\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，
        fscanf(fp, "Para_aco_search.output_file = %s\n\n", temp_str);
        //此处的数据已由其他变量（svm_all_filepath_on_interface）代为传递，

        fclose(fp);
        fp = NULL;

    }
    else
    {
        ui->textBrowser->append(tr("错误：读取参数文件失败！"));
        return ERROR;
    }
    return SUCCESS;
}

/* 解析各个结构体（SVM_ALL_INPUT_FILEPATH_ON_INTERFACE、SVM_FILE_SOURCE）中的数据，
 * 并将最后的算法能够接受的数据填写到SVM_ALL_FILEPATH、PARA_SVM_SCALE、PARA_SVM_TRAIN、
 * PARA_SVM_PREDICT、PARA_GRID_SEARCH、PARA_ACO_SEARCH中 */
int Widget::parse_svm_parameters()
{
    QByteArray ba;

    /* 解析缩放模块参数 */
    if(svm_task.svm_scale_task)//如果进行数据缩放，
    {
        /* 解析训练样本文件名 */
        if(svm_all_filepath_on_interface.train_data_filepath != "")
        {
            ba = svm_all_filepath_on_interface.train_data_filepath.toLatin1();
            Svm_all_filepath.train_data_filepath = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.train_data_filepath, ba.data());

            Para_svm_scale.data_set = Svm_all_filepath.train_data_filepath;

            Svm_all_filepath.train_data_scaled_filepath = (char *)malloc(strlen(Svm_all_filepath.train_data_filepath) + strlen(".scaled") + 1);
            strcpy(Svm_all_filepath.train_data_scaled_filepath, Svm_all_filepath.train_data_filepath);
            strcat(Svm_all_filepath.train_data_scaled_filepath, ".scaled");

            Para_svm_scale.result_filename = Svm_all_filepath.train_data_scaled_filepath;
        }
        else
        {
            ui->textBrowser->append(tr("缩放模块参数检查，错误：没有指定训练样本。"));
            return ERROR;
        }

        /* 有时候只有训练文件，不做测试 */
        /* 判断是否为空指针 */
        if(svm_all_filepath_on_interface.test_data_filepath != "")
        {
            ba = svm_all_filepath_on_interface.test_data_filepath.toLatin1();
            Svm_all_filepath.test_data_filepath = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.test_data_filepath, ba.data());

            Svm_all_filepath.test_data_scaled_filepath = (char *)malloc(strlen(Svm_all_filepath.test_data_filepath) + strlen(".scaled") + 1);
            strcpy(Svm_all_filepath.test_data_scaled_filepath, Svm_all_filepath.test_data_filepath);
            strcat(Svm_all_filepath.test_data_scaled_filepath, ".scaled");
        }
        else
        {
            ui->textBrowser->append(tr("缩放模块参数检查，警告：没有指定测试样本。"));
            Svm_all_filepath.test_data_filepath = NULL;
        }

        /* 解析缩放规则来源（文件？还是参数？） */
        if(svm_file_source.restore_filepath_source == SCALE_ACCORD_TO_PARA)//如果选择的是依据参数缩放，并保存规则文件
        {
            Svm_all_filepath.range_filepath = (char *)malloc(strlen(Svm_all_filepath.train_data_filepath) + strlen(".range") + 1);
            strcpy(Svm_all_filepath.range_filepath, Svm_all_filepath.train_data_filepath);
            strcat(Svm_all_filepath.range_filepath, ".range");

            Para_svm_scale.save_filename = Svm_all_filepath.range_filepath;
            Para_svm_scale.restore_filename = NULL;
        }
        else if(svm_file_source.restore_filepath_source == SCALE_ACCORD_TO_FILE)//如果是导入规则文件来缩放数据
        {
            Para_svm_scale.save_filename = NULL;

            ba = svm_all_filepath_on_interface.restore_filename.toLatin1();
            Svm_all_filepath.range_filepath = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.range_filepath, ba.data());

            Para_svm_scale.restore_filename = Svm_all_filepath.range_filepath;
        }

        /* 解析是否进行y_scaling */
        if(svm_file_source.y_scaling_flag == Y_SCALING_YES)
        {
            Para_svm_scale.y_scaling = 1;
            ui->textBrowser->append(tr("缩放模块参数检查，提示：进行y缩放，Para_svm_scale.y_scaling = 1;"));
        }
        else if(svm_file_source.y_scaling_flag == Y_SCALING_NO)
        {
            Para_svm_scale.y_scaling = 0;
            ui->textBrowser->append(tr("缩放模块参数检查，提示：不进行y缩放，Para_svm_scale.y_scaling = 0;"));
        }

        /* 依据参数缩放还是依据规则文件缩放将导致对训练文件和测试文件不同的处理 */
        //1、先依据参数对训练样本进行缩放并保存规则文件，再利用规则文件对测试样本进行缩放
        //2、依据规则文件分别对训练样本和测试样本进行缩放
        //svm_task.restore_filepath_source = svm_file_source.restore_filepath_source;

    }

    /* 解析训练模块参数 */
    if(svm_task.svm_train_task)
    {
        /* 解析训练样本文件来源 */
        if(svm_file_source.train_data_scaled_filepath_source == FILE_FROM_LAST_TASK)//如果训练样本来自于缩放步骤
        {
            if(!svm_task.svm_scale_task)
            {
                ui->textBrowser->append(tr("训练模块参数检查，错误：训练样本来自于缩放步骤，但缩放模块没有使能。"));
                return ERROR;
            }
            else
            {
                Para_svm_train.training_set_file = Svm_all_filepath.train_data_scaled_filepath;
            }
        }
        else if(svm_file_source.train_data_scaled_filepath_source == FILE_FROM_OTHER_FILE)//如果训练样本来自于其他文件
        {
            ba = svm_all_filepath_on_interface.train_data_scaled_filepath.toLatin1();
            Svm_all_filepath.train_data_scaled_filepath = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.train_data_scaled_filepath, ba.data());

            Para_svm_train.training_set_file = Svm_all_filepath.train_data_scaled_filepath;
        }

        /* 解析训练模块的输出文件，模型文件 */
        Svm_all_filepath.model_filepath = (char *)malloc(strlen(Svm_all_filepath.train_data_scaled_filepath) + strlen(".model") + 1);
        strcpy(Svm_all_filepath.model_filepath, Svm_all_filepath.train_data_scaled_filepath);
        strcat(Svm_all_filepath.model_filepath, ".model");

        Para_svm_train.model_file = Svm_all_filepath.model_filepath;

        /* 解析svm_train.label_and_weight */
        if(Para_svm_train.svm_train_parameter.nr_weight > 0)
        {
            if(Para_svm_train.svm_train_parameter.nr_weight != svm_all_filepath_on_interface.label_and_weight.count(":"))
            {
                ui->textBrowser->append(tr("训练模块参数检查，错误：如果变量nr_weight大于0，则nr_weight和label_and_weight中的类别数量一致。"));
                return ERROR;
            }
            else//解析
            {
                weight_label = (int *)malloc(sizeof(int) * Para_svm_train.svm_train_parameter.nr_weight);
                weight = (double *)malloc(sizeof(double) * Para_svm_train.svm_train_parameter.nr_weight);

                QStringList temp_split_comma;
                QStringList temp_split_semicolon;
                int i = 0;

                /* 以逗号为分隔符进行切割 */
                temp_split_comma = svm_all_filepath_on_interface.label_and_weight.split(QString(","));

                for(i = 0; i < temp_split_comma.length(); i++)
                {
                    /* 以分号为分隔符进行切割 */
                    temp_split_semicolon = temp_split_comma.at(i).split(":");
                    weight_label[i] = temp_split_semicolon.at(0).toInt();
                    weight[i] = temp_split_semicolon.at(1).toDouble();
                }
            }
        }
        else
        {
            ui->textBrowser->append(tr("训练模块参数检查，提示：没有设置惩罚系数权重系数。"));
            weight = NULL;
            weight_label = NULL;
        }

        /* 解析分类器类型以及核函数类型 */
        if(ui->comboBox->currentIndex() == C_SVC)
            Para_svm_train.svm_train_parameter.svm_type = C_SVC;
        else if(ui->comboBox->currentIndex() == NU_SVC)
            Para_svm_train.svm_train_parameter.svm_type = NU_SVC;
        else if(ui->comboBox->currentIndex() == ONE_CLASS)
            Para_svm_train.svm_train_parameter.svm_type = ONE_CLASS;
        else if(ui->comboBox->currentIndex() == EPSILON_SVR)
            Para_svm_train.svm_train_parameter.svm_type = EPSILON_SVR;
        else if(ui->comboBox->currentIndex() == NU_SVR)
            Para_svm_train.svm_train_parameter.svm_type = NU_SVR;

        if(ui->comboBox_2->currentIndex() == LINEAR)
            Para_svm_train.svm_train_parameter.kernel_type = LINEAR;
        else if(ui->comboBox_2->currentIndex() == POLY)
            Para_svm_train.svm_train_parameter.kernel_type = POLY;
        else if(ui->comboBox_2->currentIndex() == RBF)
            Para_svm_train.svm_train_parameter.kernel_type = RBF;
        else if(ui->comboBox_2->currentIndex() == SIGMOID)
            Para_svm_train.svm_train_parameter.kernel_type = SIGMOID;
        else if(ui->comboBox_2->currentIndex() == PRECOMPUTED)
            Para_svm_train.svm_train_parameter.kernel_type = PRECOMPUTED;

    }

    /* 解析预测模块参数 */
    if(svm_task.svm_predict_task)
    {
        /* 解析模型文件来源 */
        if(svm_file_source.model_filepath_source == FILE_FROM_LAST_TASK)//模型文件来自于训练步骤
        {
            if(!svm_task.svm_train_task)
            {
                ui->textBrowser->append(tr("预测模块参数检查，错误：分类器模型文件来自于训练步骤，但训练模块没有使能。"));
                return ERROR;
            }
            else
            {
                Para_svm_predict.model_file = Svm_all_filepath.model_filepath;
            }
        }
        else if(svm_file_source.model_filepath_source == FILE_FROM_OTHER_FILE)//模型文件来自于其他文件
        {
            ba = svm_all_filepath_on_interface.model_filepath.toLatin1();
            Svm_all_filepath.model_filepath = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.model_filepath, ba.data());

            Para_svm_predict.model_file = Svm_all_filepath.model_filepath;
        }

        /* 解析测试样本来源 */
        if(svm_file_source.test_data_scaled_filepath_source == FILE_FROM_LAST_TASK)
        {
            if(!svm_task.svm_scale_task)
            {
                ui->textBrowser->append(tr("预测模块参数检查，错误：测试样本来自于缩放步骤，但缩放模块没有使能。"));
                return ERROR;
            }
            else
            {
                if(Svm_all_filepath.test_data_filepath == NULL)//缩放模块的测试样本没有指定，为空
                {
                    ui->textBrowser->append(tr("预测模块参数检查，错误：测试样本来自于缩放步骤，但缩放模块的测试样本为空。"));
                    return ERROR;
                }
                else
                {
                    Para_svm_predict.test_file = Svm_all_filepath.test_data_scaled_filepath;

                    /* 解析预测输出结果文件 */
                    Svm_all_filepath.predict_accuracy_filepath = (char *)malloc(strlen(Svm_all_filepath.test_data_scaled_filepath) + strlen(".predict_accuracy") + 1);
                    strcpy(Svm_all_filepath.predict_accuracy_filepath, Svm_all_filepath.test_data_scaled_filepath);
                    strcat(Svm_all_filepath.predict_accuracy_filepath, ".predict.accuracy");

                    Para_svm_predict.output_file = Svm_all_filepath.predict_accuracy_filepath;
                }
            }
        }
        else if(svm_file_source.test_data_scaled_filepath_source == FILE_FROM_OTHER_FILE)
        {
            ba = svm_all_filepath_on_interface.test_data_scaled_filepath.toLatin1();
            Svm_all_filepath.test_data_scaled_filepath_other_file = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Svm_all_filepath.test_data_scaled_filepath_other_file, ba.data());

            Para_svm_predict.test_file = Svm_all_filepath.test_data_scaled_filepath_other_file;

            /* 解析预测输出结果文件 */
            Svm_all_filepath.predict_accuracy_filepath = (char *)malloc(strlen(Svm_all_filepath.test_data_scaled_filepath_other_file) + strlen(".predict_accuracy") + 1);
            strcpy(Svm_all_filepath.predict_accuracy_filepath, Svm_all_filepath.test_data_scaled_filepath_other_file);
            strcat(Svm_all_filepath.predict_accuracy_filepath, ".predict.accuracy");

            Para_svm_predict.output_file = Svm_all_filepath.predict_accuracy_filepath;
        }

    }

    /* 解析格点搜索模块参数 */
    if(svm_task.svm_grid_search_task)
    {
        /* 解析格点搜索输出结果文件 */
        if(Svm_all_filepath.train_data_scaled_filepath == NULL)
        {
            ui->textBrowser->append(tr("格点搜索模块参数检查，错误：格点搜索需要调用训练模块，但训练模块的训练样本文件没有指定，为空。"));
            return ERROR;
        }
        else
        {
            Svm_all_filepath.grid_search_result = (char *)malloc(strlen(Svm_all_filepath.train_data_scaled_filepath) + strlen(".grid_search_accuracy") + 1);
            strcpy(Svm_all_filepath.grid_search_result, Svm_all_filepath.train_data_scaled_filepath);
            strcat(Svm_all_filepath.grid_search_result, ".grid_search_accuracy");

            Para_grid_search.output_file = Svm_all_filepath.grid_search_result;
        }
    }

    /* 解析蚁群搜索模块参数 */
    if(svm_task.svm_aco_search_task)
    {
        /* 解析蚁群搜索输出结果文件 */
        if(Svm_all_filepath.train_data_scaled_filepath == NULL)
        {
            ui->textBrowser->append(tr("蚁群搜索模块参数检查，错误：蚁群搜索需要调用训练模块，但训练模块的训练样本文件没有指定，为空。"));
            return ERROR;
        }
        else
        {
            Svm_all_filepath.aco_search_result = (char *)malloc(strlen(Svm_all_filepath.train_data_scaled_filepath) + strlen(".aco_search_accuracy") + 1);
            strcpy(Svm_all_filepath.aco_search_result, Svm_all_filepath.train_data_scaled_filepath);
            strcat(Svm_all_filepath.aco_search_result, ".aco_search_accuracy");

            Para_aco_search.output_file = Svm_all_filepath.aco_search_result;
        }

        /* 解析经验最优路径 */
        if(svm_all_filepath_on_interface.empiri_best_path == "")
        {
            ui->textBrowser->append(tr("蚁群搜索模块参数检查，错误：没有指定经验最优路径。"));
            return ERROR;
        }
        else
        {
            ba = svm_all_filepath_on_interface.empiri_best_path.toLatin1();
            Para_aco_search.best_path = (char *)malloc(sizeof(char) * (ba.size() + 1));
            strcpy(Para_aco_search.best_path, ba.data());
        }
    }

    return SUCCESS;
}

void Widget::recei_fro_datapro_task_done()
{
    ui->pushButton_12->setEnabled(true);

    /************************** 释放空间 **************************/
    if(Svm_all_filepath.train_data_filepath)
    {
        free(Svm_all_filepath.train_data_filepath);
        Svm_all_filepath.train_data_filepath = NULL;
    }

    if(Svm_all_filepath.train_data_scaled_filepath)
    {
        free(Svm_all_filepath.train_data_scaled_filepath);
        Svm_all_filepath.train_data_scaled_filepath = NULL;
    }

    if(Svm_all_filepath.test_data_filepath)
    {
        free(Svm_all_filepath.test_data_filepath);
        Svm_all_filepath.test_data_filepath = NULL;
    }

    if(Svm_all_filepath.test_data_scaled_filepath)
    {
        free(Svm_all_filepath.test_data_scaled_filepath);
        Svm_all_filepath.test_data_scaled_filepath = NULL;
    }

    if(Svm_all_filepath.test_data_scaled_filepath_other_file)
    {
        free(Svm_all_filepath.test_data_scaled_filepath_other_file);
        Svm_all_filepath.test_data_scaled_filepath_other_file = NULL;
    }

    if(Svm_all_filepath.range_filepath)
    {
        free(Svm_all_filepath.range_filepath);
        Svm_all_filepath.range_filepath = NULL;
    }

    if(Svm_all_filepath.model_filepath)
    {
        free(Svm_all_filepath.model_filepath);
        Svm_all_filepath.model_filepath = NULL;
    }

    if(Svm_all_filepath.predict_accuracy_filepath)
    {
        free(Svm_all_filepath.predict_accuracy_filepath);
        Svm_all_filepath.predict_accuracy_filepath = NULL;
    }

    if(Svm_all_filepath.grid_search_result)
    {
        free(Svm_all_filepath.grid_search_result);
        Svm_all_filepath.grid_search_result = NULL;
    }

    if(Svm_all_filepath.aco_search_result)
    {
        free(Svm_all_filepath.aco_search_result);
        Svm_all_filepath.aco_search_result = NULL;
    }

    if(Para_aco_search.best_path)
        free(Para_aco_search.best_path);

    if(weight)
    {
        free(weight);
        weight = NULL;
    }

    if(weight_label)
    {
        free(weight_label);
        weight_label = NULL;
    }

    ui->textBrowser->append(tr("任务执行完毕！\n"));
}
