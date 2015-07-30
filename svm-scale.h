#ifndef SVMSCALE_H
#define SVMSCALE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float l ;
    float u ;
    int y_scaling;
    float y_lower;
    float y_upper;
    char *save_filename;
    char *restore_filename;
    char *data_set;
    char *result_filename;
} PARA_SVM_SCALE;

/*  */
int main_svm_scale(PARA_SVM_SCALE *para_svm_scale);


#ifdef __cplusplus
     }
#endif

#endif // SVMSCALE_H
