#include <stdio.h>
#include "string.h"
#include "svm.h"
#include "svm-scale.h"
#include "svm-train.h"
#include "svm-predict.h"
#include "grid_search.h"

PARA_SVM_SCALE Para_svm_scale;//缩放参数
PARA_SVM_TRAIN Para_svm_train;//训练参数
int weight_label[] = {1,2,3};//训练样本类别之间样本数量不平衡时可能需要对不同类别设置不同的权重，此时在weight_label和weight两
double weight[] = {3,4,5};   //个数组对应位置分别填入类别标签和权重,注意其数组长度必须和nr_weight相等!!!
PARA_SVM_PREDICT Para_svm_predict;//预测参数
PARA_GRID_SEARCH Para_grid_search;

extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;

/* 参数配置 */
void para_config()
{
    /* 配置缩放参数 */
    Para_svm_scale.l = 0.0;
    Para_svm_scale.u = 1.0;
    Para_svm_scale.y_scaling = 0;
    Para_svm_scale.y_lower = 0.0;
    Para_svm_scale.y_upper = 0.0;
    Para_svm_scale.save_filename = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\range";
    Para_svm_scale.restore_filename = NULL;//save_filename和restore_filename参数必须至少有一个为NULL
    Para_svm_scale.data_set = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file2";
    Para_svm_scale.result_filename = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file2.scaled";

    /* 配置训练参数 */
    Para_svm_train.svm_train_parameter.svm_type = ONE_CLASS;//C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR,选择一个参数
    Para_svm_train.svm_train_parameter.kernel_type = RBF;//LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED, 选择一个参数
    Para_svm_train.svm_train_parameter.degree = 3;
    Para_svm_train.svm_train_parameter.gamma = 0;//0.001953;//0.0009765625;//0;//0代表默认
    Para_svm_train.svm_train_parameter.coef0 = 0;
    Para_svm_train.svm_train_parameter.nu = 0.5;//0.406126;//0.0051543278;//0.5;
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
    Para_svm_train.training_set_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file1.scaled";
    Para_svm_train.model_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file1.model";// or NULL

    /* 配置预测参数 */
    Para_svm_predict.predict_probability = 0;//probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported
    Para_svm_predict.quiet_mode = 0;//0: outputs;  !0: no outputs
    Para_svm_predict.test_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file2.scaled";
    Para_svm_predict.model_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file1.model";
    Para_svm_predict.output_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file_predict.result";

    /* 每次进行交叉验证前先清零 */
    result_cross_validation_accuracy.correct = 0;
    result_cross_validation_accuracy.total = 0;

    /* 每次进行预测前先清零 */
    result_predict_accuracy.correct = 0;
    result_predict_accuracy.total = 0;

    /* 格点搜索相关参数 */
    Para_grid_search.gamma_begin = -10;//底数为2，下同
    Para_grid_search.gamma_end = 10;//10;
    Para_grid_search.gamma_step = 1;
    Para_grid_search.nu_begin = -10;//注意nu参数取值范围（0,1）开区间
    Para_grid_search.nu_end = -0.05;
    Para_grid_search.nu_step = 0.1;//1;
    Para_grid_search.v_fold = 5;
    Para_grid_search.flag_predict = 1;// 0;//1,表示每个参数在交叉验证之后，再进行模型训练，然后在对测试样本做预测；0表示不做预测
    Para_grid_search.output_file = "C:\\Users\\L Z Hua\\Desktop\\qt_workspace\\0330svm\\build-svm-PC-Debug\\debug\\file.grid_search_result.txt";//保存数据格式，第一列g参数，第二列nu参数，第三列交叉验证正确率，第四列预测正确率

}

int main(void)
{
/************************** 参数配置 **************************/
    para_config();

/************************** 数据缩放 **************************/
//    printf("Scaling...\n");

//    if(main_svm_scale(&Para_svm_scale) == 0)
//        printf("Done!\n");
//    else
//        printf("Something wrong!\n");

/************************** 格点搜索参数 **************************/

    if(grid_search(&Para_grid_search, &Para_svm_train, &Para_svm_predict) == 0)
        printf("Grid search done!\nResults stored in %s\n",Para_grid_search.output_file);
    else
        printf("Something wrong!\n");
/************************** 交叉验证 or 训练模型 **************************/
//    if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么.....
//        printf("Cross_validation...\n");
//    else
//        printf("Training ...\n");

//    if((main_svm_train(&Para_svm_train)) == 0)
//        printf("Done!\n");
//    else
//        printf("Something wrong!\n");

//    if(Para_svm_train.cross_validation)//如果参数中选择了交叉验证那么此处输出正确率
//        printf("correct: %d, total: %d\n",result_cross_validation_accuracy.correct, result_cross_validation_accuracy.total);

/************************** 测试样本预测 **************************/
//    printf("Predicting...\n");

//    if((main_svm_predict(&Para_svm_predict)) == 0)
//        printf("Done!\n");
//    else
//        printf("Something wrong!\n");

//    //预测，打印正确率
//    printf("correct: %d, total: %d\n",result_predict_accuracy.correct,result_predict_accuracy.total);

    return 0;
}
