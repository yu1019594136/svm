#ifndef SVMTRAIN_H
#define SVMTRAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "svm.h"

typedef struct {
   struct svm_parameter svm_train_parameter;
   int quiet_mode;//0: outputs;  !0: no outputs
   int cross_validation;//输入0表示不进行交叉验证，输入n表示进行n折交叉验证（注意n必须大于2）
   char *training_set_file;
   char *model_file;
}PARA_SVM_TRAIN;

typedef struct {
   int correct;
   int total;
}RESULT_CROSS_VALIDATION_ACCURACY;

int main_svm_train(PARA_SVM_TRAIN *para_svm_train);

#ifdef __cplusplus
     }
#endif

#endif // SVMTRAIN_H
