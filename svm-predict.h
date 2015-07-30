#ifndef SVMPREDICT_H
#define SVMPREDICT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int predict_probability;//probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported
    int quiet_mode;//0: outputs;  !0: no outputs
    char *test_file;
    char *model_file;
    char *output_file;
} PARA_SVM_PREDICT;

typedef struct {
   int correct;
   int total;
} RESULT_PREDICT_ACCURACY;

int main_svm_predict(PARA_SVM_PREDICT *para_svm_predict);

#ifdef __cplusplus
     }
#endif

#endif // SVMPREDICT_H
