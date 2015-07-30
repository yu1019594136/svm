#include "thread_data_proc.h"
#include <QDebug>
/* 支持向量机相关头文件 */
#include "svm.h"
#include "svm-scale.h"
#include "svm-predict.h"
#include "svm-train.h"
#include "grid_search.h"
#include "aco_search.h"

extern SVM_ALL_FILEPATH Svm_all_filepath;//配置所有文件路径相关的变量
extern PARA_SVM_SCALE Para_svm_scale;//缩放参数
extern PARA_SVM_TRAIN Para_svm_train;//训练参数
extern int *weight_label;//训练样本类别之间样本数量不平衡时可能需要对不同类别设置不同的权重，此时在weight_label和weight两
extern double *weight;   //个数组对应位置分别填入类别标签和权重,注意其数组长度必须和nr_weight相等!!!最后每个类别的惩罚系数=C * weight_i for label_i
extern PARA_SVM_PREDICT Para_svm_predict;//预测参数
extern PARA_GRID_SEARCH Para_grid_search;
extern PARA_ACO_SEARCH Para_aco_search;

extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;

extern SVM_FILE_SOURCE svm_file_source;

extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;

/*********************数据处理线程*****************************/
DataProcessThread::DataProcessThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    excute_svm_task = false;

    svm_task.svm_scale_task = false;
    //svm_task.restore_filepath_source = SCALE_ACCORD_TO_PARA;
    svm_task.svm_train_task = false;
    svm_task.svm_predict_task = false;
    svm_task.svm_grid_search_task = false;
    svm_task.svm_aco_search_task = false;

}

void DataProcessThread::run()
{
    char temp_str[MAX_STR_LEN] = {0};

    emit send_to_textbrower_display_output("DataProcessThread start!\n");

    while(!stopped)
    {
        if(excute_svm_task)
        {
            send_to_textbrower_display_output("DataProcessThread starts to excute svm_task!\n");

            //test();

            /* 1、检测是否进行数据缩放 */
            if(svm_task.svm_scale_task)
            {
                /* 缩放训练样本 */
                if(main_svm_scale(&Para_svm_scale) == SUCCESS)
                    send_to_textbrower_display_output(tr("训练数据压缩完成！"));
                else
                    send_to_textbrower_display_output(tr("训练数据压缩出错！请检查参数和数据文件格式"));

                /* 依据参数缩放还是依据规则文件缩放将导致对训练文件和测试文件不同的处理 */
                if(Svm_all_filepath.test_data_filepath != NULL)
                {
                    /* 检查是否有测试数据需要缩放 */
                    if(svm_file_source.restore_filepath_source == SCALE_ACCORD_TO_PARA)//1、先依据参数对训练样本进行缩放并保存规则文件，再利用规则文件对测试样本进行缩放
                    {
                        Para_svm_scale.data_set = Svm_all_filepath.test_data_filepath;
                        Para_svm_scale.restore_filename = Para_svm_scale.save_filename;
                        Para_svm_scale.save_filename = NULL;
                        Para_svm_scale.result_filename = Svm_all_filepath.test_data_scaled_filepath;
                    }
                    else if(svm_file_source.restore_filepath_source == SCALE_ACCORD_TO_FILE)//2、依据规则文件分别对训练样本和测试样本进行缩放
                    {
                        Para_svm_scale.data_set = Svm_all_filepath.test_data_filepath;
                        Para_svm_scale.result_filename = Svm_all_filepath.test_data_scaled_filepath;

                    }

                    if(main_svm_scale(&Para_svm_scale) == SUCCESS)
                        send_to_textbrower_display_output(tr("测试数据压缩完成！"));
                    else
                        send_to_textbrower_display_output(tr("测试数据压缩出错！请检查参数和数据文件格式"));
                }
            }

            /* 2、检测参是否进行参数搜索 */
//            if()

            /* 3、分类器训练 */
            if(svm_task.svm_train_task)
            {
                if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么.....
                    send_to_textbrower_display_output(tr("交叉验证...\n"));
                else
                    send_to_textbrower_display_output(tr("分类器训练 ...\n"));

                if((main_svm_train(&Para_svm_train)) == SUCCESS)
                    send_to_textbrower_display_output(tr("训练(交叉验证)完成!\n"));
                else
                    send_to_textbrower_display_output(tr("训练(交叉验证)出错!\n"));

                if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么此处输出正确率
                {
                    sprintf(temp_str, "correct: %d, total: %d\n",result_cross_validation_accuracy.correct, result_cross_validation_accuracy.total);
                    send_to_textbrower_display_output(temp_str);
                }

            }

            /* 4、样本预测训练 */
            if(svm_task.svm_predict_task)
            {
                send_to_textbrower_display_output(tr("预测...\n"));

                if((main_svm_predict(&Para_svm_predict)) == SUCCESS)
                    send_to_textbrower_display_output(tr("预测完成!\n"));
                else
                    send_to_textbrower_display_output(tr("预测出错!\n"));

                //预测，打印正确率
                sprintf(temp_str, "correct: %d, total: %d\n",result_predict_accuracy.correct,result_predict_accuracy.total);
                send_to_textbrower_display_output(temp_str);
            }

            excute_svm_task = false;
            emit task_done();
        }
    }

    emit send_to_textbrower_display_output("DataProcessThread done!\n");
}

void DataProcessThread::stop()
{
    stopped = true;
}

void DataProcessThread::recei_fro_widget_svm_task(SVM_TASK svm_task_para)
{
    svm_task = svm_task_para;
    excute_svm_task = true;
    send_to_textbrower_display_output("DataProcessThread receives svm_task!\n");
}

void DataProcessThread::test()
{
    qDebug()  << "svm_task.svm_scale_task = " << svm_task.svm_scale_task << endl ;
    qDebug()  << "svm_task.svm_train_task = " << svm_task.svm_train_task << endl ;
    qDebug()  << "svm_task.svm_predict_task = " << svm_task.svm_predict_task << endl ;
    qDebug()  << "svm_task.svm_grid_search_task = " << svm_task.svm_grid_search_task << endl ;
    qDebug()  << "svm_task.svm_aco_search_task = " << svm_task.svm_aco_search_task << endl ;
    //qDebug()  << "svm_task.restore_filepath_source = " << svm_task.restore_filepath_source << endl ;


    if(svm_task.svm_scale_task)
    {
        qDebug("Para_svm_scale.l = %f\n", Para_svm_scale.l);
        qDebug("Para_svm_scale.u = %f\n", Para_svm_scale.u);
        qDebug("Para_svm_scale.y_scaling = %d\n", Para_svm_scale.y_scaling);
        qDebug("Para_svm_scale.y_lower = %f\n", Para_svm_scale.y_lower);
        qDebug("Para_svm_scale.y_upper = %f\n", Para_svm_scale.y_upper);
        qDebug("Para_svm_scale.data_set = %s\n", Para_svm_scale.data_set);
        qDebug("Para_svm_scale.save_filename = %s\n", Para_svm_scale.save_filename);
        qDebug("Para_svm_scale.restore_filename = %s\n", Para_svm_scale.restore_filename);
        qDebug("Para_svm_scale.result_filename = %s\n\n", Para_svm_scale.result_filename);

        qDebug("Svm_all_filepath.test_data_filepath = %s\n", Svm_all_filepath.test_data_filepath);
        qDebug("Svm_all_filepath.test_data_scaled_filepath = %s\n\n", Svm_all_filepath.test_data_scaled_filepath);
    }

    if(svm_task.svm_train_task)
    {
        qDebug("Para_svm_train.svm_train_parameter.svm_type = %d\n", Para_svm_train.svm_train_parameter.svm_type);
        qDebug("Para_svm_train.svm_train_parameter.kernel_type = %d\n", Para_svm_train.svm_train_parameter.kernel_type);
        qDebug("Para_svm_train.svm_train_parameter.degree = %d\n", Para_svm_train.svm_train_parameter.degree);
        qDebug("Para_svm_train.svm_train_parameter.gamma = %f\n", Para_svm_train.svm_train_parameter.gamma);
        qDebug("Para_svm_train.svm_train_parameter.coef0 = %f\n", Para_svm_train.svm_train_parameter.coef0);
        qDebug("Para_svm_train.svm_train_parameter.cache_size = %f\n", Para_svm_train.svm_train_parameter.cache_size);
        qDebug("Para_svm_train.svm_train_parameter.eps = %f\n", Para_svm_train.svm_train_parameter.eps);
        qDebug("Para_svm_train.svm_train_parameter.C = %f\n", Para_svm_train.svm_train_parameter.C);
        qDebug("Para_svm_train.svm_train_parameter.nr_weight = %d\n", Para_svm_train.svm_train_parameter.nr_weight);
//        qDebug("Para_svm_train.svm_train_parameter.weight_label = %d\n", Para_svm_train.svm_train_parameter.weight_label);
//        qDebug("Para_svm_train.svm_train_parameter.weight = %f\n", Para_svm_train.svm_train_parameter.weight);
        qDebug("Para_svm_train.svm_train_parameter.nu = %f\n", Para_svm_train.svm_train_parameter.nu);
        qDebug("Para_svm_train.svm_train_parameter.p = %f\n", Para_svm_train.svm_train_parameter.p);
        qDebug("Para_svm_train.svm_train_parameter.shrinking = %d\n", Para_svm_train.svm_train_parameter.shrinking);
        qDebug("Para_svm_train.svm_train_parameter.probability = %d\n", Para_svm_train.svm_train_parameter.probability);
        qDebug("Para_svm_train.quiet_mode = %d\n", Para_svm_train.quiet_mode);
        qDebug("Para_svm_train.cross_validation = %d\n", Para_svm_train.cross_validation);
        qDebug("Para_svm_train.training_set_file = %s\n", Para_svm_train.training_set_file);
        qDebug("Para_svm_train.model_file = %s\n\n", Para_svm_train.model_file);
    }

    if(svm_task.svm_predict_task)
    {
        qDebug("Para_svm_predict.predict_probability = %d\n", Para_svm_predict.predict_probability);
        qDebug("Para_svm_predict.quiet_mode = %d\n", Para_svm_predict.quiet_mode);
        qDebug("Para_svm_predict.test_file = %s\n", Para_svm_predict.test_file);
        qDebug("Para_svm_predict.model_file = %s\n", Para_svm_predict.model_file);
        qDebug("Para_svm_predict.output_file = %s\n\n", Para_svm_predict.output_file);
    }

    if(svm_task.svm_grid_search_task)
    {
        qDebug("Para_grid_search.d1_begin = %f\n", Para_grid_search.d1_begin);
        qDebug("Para_grid_search.d1_end = %f\n", Para_grid_search.d1_end);
        qDebug("Para_grid_search.d1_step = %f\n", Para_grid_search.d1_step);
        qDebug("Para_grid_search.d2_begin = %f\n", Para_grid_search.d2_begin);
        qDebug("Para_grid_search.d2_end = %f\n", Para_grid_search.d2_end);
        qDebug("Para_grid_search.d2_step = %f\n", Para_grid_search.d2_step);
        qDebug("Para_grid_search.v_fold = %d\n", Para_grid_search.v_fold);
        qDebug("Para_grid_search.flag_predict = %d\n", Para_grid_search.flag_predict);
        qDebug("Para_grid_search.output_file = %s\n\n", Para_grid_search.output_file);
    }

    if(svm_task.svm_aco_search_task)
    {
        qDebug("Para_aco_search.ants_amount = %d\n", Para_aco_search.ants_amount);
        qDebug("Para_aco_search.x_aixs_lines = %d\n", Para_aco_search.x_aixs_lines);
        qDebug("Para_aco_search.y_aixs_values = %d\n", Para_aco_search.y_aixs_values);
        qDebug("Para_aco_search.max_iterations = %d\n", Para_aco_search.max_iterations);
        qDebug("Para_aco_search.pheromone_initial_value = %f\n", Para_aco_search.pheromone_initial_value);
        qDebug("Para_aco_search.alpha = %f\n", Para_aco_search.alpha);
        qDebug("Para_aco_search.beta = %f\n", Para_aco_search.beta);
        qDebug("Para_aco_search.rho = %f\n", Para_aco_search.rho);
        qDebug("Para_aco_search.Q = %f\n", Para_aco_search.Q);
        qDebug("Para_aco_search.flag_predict = %d\n", Para_aco_search.flag_predict);
        qDebug("Para_aco_search.v_fold = %d\n", Para_aco_search.v_fold);
        qDebug("Para_aco_search.MAX_TAU = %f\n", Para_aco_search.MAX_TAU);
        qDebug("Para_aco_search.MIN_TAU = %f\n", Para_aco_search.MIN_TAU);
        qDebug("Para_aco_search.ERR = %f\n", Para_aco_search.ERR);
        qDebug("Para_aco_search.best_path = %s\n", Para_aco_search.best_path);
        qDebug("Para_aco_search.output_file = %s\n\n", Para_aco_search.output_file);
    }
}
