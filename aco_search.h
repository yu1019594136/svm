#ifndef ACO_SEARCH_H
#define ACO_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "svm-train.h"
#include "svm-predict.h"

typedef struct {
    int ants_amount;//蚂蚁数量
    int x_aixs_lines;//Li，线段数量，即参数位数
    int y_aixs_values;//每一位的有效数字个数
    int max_iterations;//最大迭代步数
    double pheromone_initial_value;//信息素初始值
    double alpha;//蚁群算法参数,下同
    double beta;
    double rho;
    double Q;
    int flag_predict;
    int v_fold;
    double MAX_TAU;
    double MIN_TAU;
    double ERR;
    char *best_path;
    char *output_file;
} PARA_ACO_SEARCH;

typedef struct {
    double cross_validation_accuracy;
    double predict_accuracy;
} ACO_SEARCH_RESULT;

int aco_search(PARA_ACO_SEARCH *para_aco_search, PARA_SVM_TRAIN *para_svm_train, PARA_SVM_PREDICT *para_svm_predict);

/* 传入一个二维数组，判断是否达到收敛条件，达到收敛返回err */
double Is_convergence(int **arr, int x, int y);

#ifdef __cplusplus
     }
#endif

#endif // ACO_SEARCH_H

