#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "svm-train.h"
#include "svm-predict.h"

typedef struct {
    double d1_begin;
    double d1_end;
    double d1_step;
    double d2_begin;
    double d2_end;
    double d2_step;
    unsigned int v_fold;
    unsigned int flag_predict;
    char *output_file;
} PARA_GRID_SEARCH;
//d1表示搜索的第一维参数
//d2表示搜索的第二维参数

typedef struct {
    double d1_lg;
    double d1;
    double d2_lg;
    double d2;
    double cross_validation_accuracy;
    double predict_accuracy;
} GRID_SEARCH_RESULT;
//lg表示保存对应的对数值即log2 d1、log2 d2

int grid_search(PARA_GRID_SEARCH *para_grid_search, PARA_SVM_TRAIN *para_svm_train, PARA_SVM_PREDICT *para_svm_predict);

#ifdef __cplusplus
     }
#endif

#endif // GRID_SEARCH_H
