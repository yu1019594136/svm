#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "grid_search.h"


GRID_SEARCH_RESULT grid_search_result;
extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;

int grid_search(PARA_GRID_SEARCH *para_grid_search, PARA_SVM_TRAIN *para_svm_train, PARA_SVM_PREDICT *para_svm_predict)
{
    int i;
    int j;
    FILE *output;

    /* 保存结果到文件 */
    if((output = fopen(para_grid_search->output_file, "w")) == NULL)
    {
        fprintf(stderr,"can't open output file %s\n",para_grid_search->output_file);
        exit(1);
    }

    for(i = 0; para_grid_search->gamma_begin + para_grid_search->gamma_step * i <= para_grid_search->gamma_end; i++)
    {
        for(j = 0; para_grid_search->nu_begin + para_grid_search->nu_step * j <= para_grid_search->nu_end; j++)
        {
            /* 保存交叉验证参数到结构体 */
            grid_search_result.gamma = pow(2, para_grid_search->gamma_begin + para_grid_search->gamma_step * i);
            grid_search_result.nu = pow(2, para_grid_search->nu_begin + para_grid_search->nu_step * j);

            /* 设置交叉验证相关的参数 */
            para_svm_train->svm_train_parameter.gamma = grid_search_result.gamma;
            para_svm_train->svm_train_parameter.nu = grid_search_result.nu;//
            para_svm_train->cross_validation = para_grid_search->v_fold;//设置交叉验证参数，进行交叉验证

            /* 调用接口进行交叉验证 */
            if(main_svm_train(para_svm_train) == 0)
            {
                /* 记录交叉验证结果的正确率 */
                grid_search_result.cross_validation_accuracy = 100 * result_cross_validation_accuracy.correct / result_cross_validation_accuracy.total;

                /* 如果需要进行预测 */
                if(para_grid_search->flag_predict)
                {
                    /* 重新设置训练参数 */
                    para_svm_train->cross_validation = 0;//将验证折数设为0，表示不进行交叉验证而是进行模型训练

                    /* 调用接口进行模型训练 */
                    if(main_svm_train(para_svm_train) == 0)
                        printf("Model done!\n");

                    /* 调用接口进行测试 */
                    if(main_svm_predict(para_svm_predict) == 0)
                        printf("Predict done!\n");

                    /* 记录预测样本的正确率 */
                    grid_search_result.predict_accuracy = 100 * result_predict_accuracy.correct / result_predict_accuracy.total;
                }
                else
                    grid_search_result.predict_accuracy =0;//不进行预测时，预测样本的正确率全部填写0
            }

            /* 记录结果到文件，第一列g参数，第二列nu参数，第三列该参数下的交叉验证正确率，第四列该参数下训练模型后对测试样本的预测正确率 */
            fprintf(output,"%f\t%f\t%f\t%f\n",grid_search_result.gamma, grid_search_result.nu, grid_search_result.cross_validation_accuracy, grid_search_result.predict_accuracy);

        }
    }
    fclose(output);
    return 0;
}
