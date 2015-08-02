#ifndef QCOMMON_H
#define QCOMMON_H

#include <QString>

//参数文件
#define SVM_PARAMETER_FILE "svm_parameter.txt"
//从文件读取字符串数据时，要提前分配的字符长度
#define MAX_STR_LEN 2048

enum RADIOBUTTON_FILEPATH_SOURCE{
    FILE_FROM_LAST_TASK = 0,
    FILE_FROM_OTHER_FILE,
    SCALE_ACCORD_TO_FILE,
    SCALE_ACCORD_TO_PARA,
    Y_SCALING_YES,
    Y_SCALING_NO
};

typedef struct {
    int restore_filepath_source;
    int train_data_scaled_filepath_source;
    int model_filepath_source;
    int test_data_scaled_filepath_source;
    int y_scaling_flag;
} SVM_FILE_SOURCE;

typedef struct {
    bool svm_scale_task;
//    int restore_filepath_source;
    bool svm_train_task;
    bool svm_predict_task;
    bool svm_grid_search_task;
    bool svm_aco_search_task;
} SVM_TASK;
/*
 * 1、布尔变量为true的任务将被执行，否则不执行
 * 2、搜索算法将被优先执行，并且此时不会单独执行训练或者预测任务
 *
*/
typedef struct {
    QString train_data_filepath;
    QString test_data_filepath;
    QString restore_filename;
    QString train_data_scaled_filepath;
    QString model_filepath;
    QString test_data_scaled_filepath;

    QString label_and_weight;
    QString empiri_best_path;

} SVM_ALL_INPUT_FILEPATH_ON_INTERFACE;

/*
nr_weight is the number of elements in the array weight_label and
 weight. Each weight[i] corresponds to weight_label[i], meaning that
 the penalty of class weight_label[i] is scaled by a factor of weight[i].
*/


#endif // QCOMMON_H
