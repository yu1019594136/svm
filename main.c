#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "svm.h"
#include "svm-scale.h"
#include "svm-train.h"
#include "svm-predict.h"
#include "grid_search.h"
#include "common.h"

SVM_ALL_FILEPATH Svm_all_filepath;//配置所有文件路径相关的变量
PARA_SVM_SCALE Para_svm_scale;//缩放参数
PARA_SVM_TRAIN Para_svm_train;//训练参数
int weight_label[] = {1,2,3};//训练样本类别之间样本数量不平衡时可能需要对不同类别设置不同的权重，此时在weight_label和weight两
double weight[] = {3,4,5};   //个数组对应位置分别填入类别标签和权重,注意其数组长度必须和nr_weight相等!!!
PARA_SVM_PREDICT Para_svm_predict;//预测参数
PARA_GRID_SEARCH Para_grid_search;

extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;


/* 配置所有文件路径相关的变量 */
void filepath_config(SVM_ALL_FILEPATH *svm_all_filepath);

/* 参数配置 */
void para_config();

int main(void)
{
/************************** 参数配置 **************************/
    para_config();

/************************** 数据缩放 **************************/
//    printf("Scaling train data...\n");

//    /* 缩放训练样本 */
//    Para_svm_scale.save_filename = Svm_all_filepath.range_filepath;
//    Para_svm_scale.restore_filename = NULL;//save_filename和restore_filename参数必须至少有一个为NULL
//    Para_svm_scale.data_set = Svm_all_filepath.train_data_filepath;
//    Para_svm_scale.result_filename = Svm_all_filepath.train_data_scaled_filepath;

//    if(main_svm_scale(&Para_svm_scale) == SUCCESS)
//        printf("Train data scaled!\n");
//    else
//        printf("Train data scaling:something wrong! check the data format\n");

//    if(Svm_all_filepath.test_data_filepath != NULL)
//    {
//        printf("Scaling test data...\n");

//        /* 缩放测试样本 */
//        Para_svm_scale.save_filename = NULL;
//        Para_svm_scale.restore_filename = Svm_all_filepath.range_filepath;//save_filename和restore_filename参数必须至少有一个为NULL
//        Para_svm_scale.data_set = Svm_all_filepath.test_data_filepath;
//        Para_svm_scale.result_filename = Svm_all_filepath.test_data_scaled_filepath;

//        if(main_svm_scale(&Para_svm_scale) == SUCCESS)
//            printf("Test data scaled!\n");
//        else
//            printf("Test data scaling:something wrong! check the data format\n");
//    }
//    else
//        printf("Warning: No test data filepath, test_data scaling cancle!\n");


/************************** 格点搜索参数 **************************/
//    if(grid_search(&Para_grid_search, &Para_svm_train, &Para_svm_predict) == SUCCESS)
//        printf("Grid search done!\nResults stored in %s\n",Para_grid_search.output_file);
//    else
//        printf("Something wrong!\n");

/************************** 交叉验证 or 训练模型 **************************/
//    if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么.....
//        printf("Cross_validation...\n");
//    else
//        printf("Training ...\n");

//    if((main_svm_train(&Para_svm_train)) == SUCCESS)
//        printf("Done!\n");
//    else
//        printf("Something wrong!\n");

//    if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么此处输出正确率
//        printf("correct: %d, total: %d\n",result_cross_validation_accuracy.correct, result_cross_validation_accuracy.total);

/************************** 测试样本预测 **************************/
//    if(Svm_all_filepath.test_data_filepath != NULL)
//    {
//        printf("Predicting...\n");

//        if((main_svm_predict(&Para_svm_predict)) == SUCCESS)
//            printf("Done!\n");
//        else
//            printf("Something wrong!\n");

//        //预测，打印正确率
//        printf("correct: %d, total: %d\n",result_predict_accuracy.correct,result_predict_accuracy.total);
//    }
//    else
//        printf("Warning: No test data filepath, test data predicting cancle!\n");



/************************** 释放空间 **************************/
    free(Svm_all_filepath.train_data_scaled_filepath);
    free(Svm_all_filepath.range_filepath);
    free(Svm_all_filepath.model_filepath);
    free(Svm_all_filepath.grid_search_result);
    /* 判断是否为空指针 */
    if(Svm_all_filepath.test_data_filepath)
    {
        free(Svm_all_filepath.test_data_scaled_filepath);
        free(Svm_all_filepath.predict_accuracy_filepath);
    }

    return 0;
}

/* 参数配置 */
void para_config()
{
    /* 配置缩放参数 */
    Para_svm_scale.l = 0.0;
    Para_svm_scale.u = 1.0;
    Para_svm_scale.y_scaling = 0;
    Para_svm_scale.y_lower = 0.0;
    Para_svm_scale.y_upper = 0.0;
    /* 手动配置 */
    Svm_all_filepath.train_data_filepath = "C:\\UCI_database\\wine\\wine.txt.train";//训练文件不能为NULL，"/home/zhouyu/UCI_database/wine/wine.txt.train"
    Svm_all_filepath.test_data_filepath = "C:\\UCI_database\\wine\\wine.txt.test";//NULL     如果没有测试文件，此处给NULL"/home/zhouyu/UCI_database/wine/wine.txt.test"
    /* 程序自动按规则自动配置 */
    filepath_config(&Svm_all_filepath);

    /* 配置训练参数 */
    Para_svm_train.svm_train_parameter.svm_type = C_SVC;//C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR,选择一个参数
    Para_svm_train.svm_train_parameter.kernel_type = RBF;//LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED, 选择一个参数
    Para_svm_train.svm_train_parameter.degree = 3;
    Para_svm_train.svm_train_parameter.gamma = 0.1;//0代表默认
    Para_svm_train.svm_train_parameter.coef0 = 0;
    Para_svm_train.svm_train_parameter.nu = 0.5;//
    Para_svm_train.svm_train_parameter.cache_size = 100;
    Para_svm_train.svm_train_parameter.C = 1;
    Para_svm_train.svm_train_parameter.eps = 1e-3;
    Para_svm_train.svm_train_parameter.p = 0.1;
    Para_svm_train.svm_train_parameter.shrinking = 1;
    Para_svm_train.svm_train_parameter.probability = 0;
    Para_svm_train.svm_train_parameter.nr_weight = 0;//0表示不设置权重，n表示有n个类别要设置权重系数，n个类别以及对应的权重系数由本文件开头处的数组weight_label和weight设定,注意其数组长度必须和n相等
    Para_svm_train.svm_train_parameter.weight_label = weight_label;//该数组在本文件开头处初始化
    Para_svm_train.svm_train_parameter.weight = weight;//该数组在本文件开头处初始化
    Para_svm_train.quiet_mode = 0;//0: outputs;  !0: no outputs
    Para_svm_train.cross_validation = 0;// 5;// 0;//输入0表示不进行交叉验证，输入n表示进行n折交叉验证（注意n必须大于2）
    Para_svm_train.training_set_file = Svm_all_filepath.train_data_scaled_filepath;
    Para_svm_train.model_file = Svm_all_filepath.model_filepath;

    /* 配置预测参数 */
    Para_svm_predict.predict_probability = 0;//probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported
    Para_svm_predict.quiet_mode = 0;//0: outputs;  !0: no outputs
    Para_svm_predict.test_file = Svm_all_filepath.test_data_scaled_filepath;
    Para_svm_predict.model_file = Svm_all_filepath.model_filepath;
    Para_svm_predict.output_file = Svm_all_filepath.predict_accuracy_filepath;

    /* 格点搜索相关参数(分类器类型和核函数类型在训练参数中配置) */
    Para_grid_search.d1_begin = -5;//底数为2，下同,搜索参数时第一个参数d1必须是核函数参数g，第二个参数d2是分类器参数(nu or C)
    Para_grid_search.d1_end = -14;//10;
    Para_grid_search.d1_step = -1;
    Para_grid_search.d2_begin = -5;//注意nu参数取值范围（0,1）开区间
    Para_grid_search.d2_end = 5;
    Para_grid_search.d2_step = 1;//1;
    Para_grid_search.v_fold = 4;
    Para_grid_search.flag_predict = 0;// 0;//1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_grid_search.output_file = Svm_all_filepath.grid_search_result;

}

/* 配置所有文件路径相关的变量 */
void filepath_config(SVM_ALL_FILEPATH *svm_all_filepath)
{
    /* 判断是否为空指针 */
    if(svm_all_filepath->train_data_filepath)
    {
        svm_all_filepath->train_data_scaled_filepath = (char *)malloc(strlen(svm_all_filepath->train_data_filepath) + strlen(".scaled") + 1);
        strcpy(svm_all_filepath->train_data_scaled_filepath, svm_all_filepath->train_data_filepath);
        strcat(svm_all_filepath->train_data_scaled_filepath, ".scaled");
    }
    else
    {
        printf("Error: No train data filepath!\n");
        exit(1);
    }

    /* 有时候只有训练文件，不做测试 */
    /* 判断是否为空指针 */
    if(svm_all_filepath->test_data_filepath)
    {
        svm_all_filepath->test_data_scaled_filepath = (char *)malloc(strlen(svm_all_filepath->test_data_filepath) + strlen(".scaled") + 1);
        strcpy(svm_all_filepath->test_data_scaled_filepath, svm_all_filepath->test_data_filepath);
        strcat(svm_all_filepath->test_data_scaled_filepath, ".scaled");

        svm_all_filepath->predict_accuracy_filepath = (char *)malloc(strlen(svm_all_filepath->test_data_scaled_filepath) + strlen(".predict_accuracy") + 1);
        strcpy(svm_all_filepath->predict_accuracy_filepath, svm_all_filepath->test_data_scaled_filepath);
        strcat(svm_all_filepath->predict_accuracy_filepath, ".predict.accuracy");
    }
    else
    {
        printf("Warning: No test data filepath!\n");
        svm_all_filepath->test_data_scaled_filepath = NULL;
        svm_all_filepath->predict_accuracy_filepath = NULL;
    }

    svm_all_filepath->range_filepath = (char *)malloc(strlen(svm_all_filepath->train_data_filepath) + strlen(".range") + 1);
    strcpy(svm_all_filepath->range_filepath, svm_all_filepath->train_data_filepath);
    strcat(svm_all_filepath->range_filepath, ".range");

    svm_all_filepath->model_filepath = (char *)malloc(strlen(svm_all_filepath->train_data_scaled_filepath) + strlen(".model") + 1);
    strcpy(svm_all_filepath->model_filepath, svm_all_filepath->train_data_scaled_filepath);
    strcat(svm_all_filepath->model_filepath, ".model");

    svm_all_filepath->grid_search_result = (char *)malloc(strlen(svm_all_filepath->train_data_scaled_filepath) + strlen(".grid_search_accuracy") + 1);
    strcpy(svm_all_filepath->grid_search_result, svm_all_filepath->train_data_scaled_filepath);
    strcat(svm_all_filepath->grid_search_result, ".grid_search_accuracy");

}
