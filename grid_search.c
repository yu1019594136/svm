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


    for(i = 0; para_grid_search->d1_begin + para_grid_search->d1_step * i <= para_grid_search->d1_end; i++)
    {
        /* 保存交叉验证参数到结构体 */
        grid_search_result.d1_lg = para_grid_search->d1_begin + para_grid_search->d1_step * i;
        grid_search_result.d1 = pow(2, grid_search_result.d1_lg);

        for(j = 0; para_grid_search->d2_begin + para_grid_search->d2_step * j <= para_grid_search->d2_end; j++)
        {
            /* 保存交叉验证参数到结构体 */
            grid_search_result.d2_lg = para_grid_search->d2_begin + para_grid_search->d2_step * j;
            grid_search_result.d2 = pow(2, grid_search_result.d2_lg);

            /* 设置交叉验证相关的参数 */
            if(para_svm_train->svm_train_parameter.svm_type == ONE_CLASS)
            {
                para_svm_train->svm_train_parameter.gamma = grid_search_result.d1;//搜索参数时第一个参数必须是核函数参数，第二个参数是分类器参数
                para_svm_train->svm_train_parameter.nu = grid_search_result.d2;
            }
            else if(para_svm_train->svm_train_parameter.svm_type == C_SVC)
            {
                para_svm_train->svm_train_parameter.gamma = grid_search_result.d1;//搜索参数时第一个参数必须是核函数参数，第二个参数是分类器参数
                para_svm_train->svm_train_parameter.C = grid_search_result.d2;
            }

            /* 设置交叉验证参数，进行交叉验证 */
            para_svm_train->cross_validation = para_grid_search->v_fold;

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
                //else
                //   grid_search_result.predict_accuracy =0;//不进行预测时，预测样本的正确率全部填写0
            }

            /* 记录结果到文件，第一列log2 d1参数，第二列log2 d2参数，第三列参数d1,第四列参数d2，第五列该参数下的交叉验证正确率，第六列该参数下训练模型后对测试样本的预测正确率(如果flag_predict==0，第六列参数将没有) */
            if(para_grid_search->flag_predict)
                fprintf(output,"%f\t%f\t%f\t%f\t%f\t%f\n",grid_search_result.d1_lg, grid_search_result.d2_lg, grid_search_result.d1, grid_search_result.d2, grid_search_result.cross_validation_accuracy, grid_search_result.predict_accuracy);
            else
                fprintf(output,"%f\t%f\t%f\t%f\t%f\n",grid_search_result.d1_lg, grid_search_result.d2_lg, grid_search_result.d1, grid_search_result.d2, grid_search_result.cross_validation_accuracy);
        }
    }
    fclose(output);
    return 0;
}
