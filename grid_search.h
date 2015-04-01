#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "svm-train.h"
#include "svm-predict.h"

typedef struct {
    double gamma_begin;
    double gamma_end;
    double gamma_step;
    double nu_begin;
    double nu_end;
    double nu_step;
    unsigned int v_fold;
    unsigned int flag_predict;
    char *output_file;
} PARA_GRID_SEARCH;

typedef struct {
    double gamma;
    double nu;
    double cross_validation_accuracy;
    double predict_accuracy;
} GRID_SEARCH_RESULT;

int grid_search(PARA_GRID_SEARCH *para_grid_search, PARA_SVM_TRAIN *para_svm_train, PARA_SVM_PREDICT *para_svm_predict);

#ifdef __cplusplus
     }
#endif

#endif // GRID_SEARCH_H
