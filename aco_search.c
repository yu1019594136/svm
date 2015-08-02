#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "common.h"
#include "aco_search.h"

extern RESULT_CROSS_VALIDATION_ACCURACY result_cross_validation_accuracy;
extern RESULT_PREDICT_ACCURACY result_predict_accuracy;

int aco_search(PARA_ACO_SEARCH *para_aco_search, PARA_SVM_TRAIN *para_svm_train, PARA_SVM_PREDICT *para_svm_predict)
{
    int i = 0;
    int j = 0;

    /* 算法相关变量 */
    /* iterations:      记录迭代步数
     * x_aixs_lines:    记录每一次搜寻中的横坐标位置,哪一列
     * tau:             存储所有节点的信息素
     * delta_tau:       存储所有节点的信息素增量
     * eta:             存储所有节点的能见度
     * path:            存储每一个蚂蚁的路径
     * psum:            每一列上所有节点概率和，计算公式的分母
     * pro:             临时存储每一列格点的概率值pro = (...)/(...)
     * pro_sum          临时变量
     * ppro:            存储赌轮随机概率值分布ppro[i] = pro[0] + pro[1] + ... + pro[i]
     * cross_validation_accuracy    交叉验证正确率
     * predict_accuracy 存储路径转化成参数后，训练模型，对样本的预测正确率
     * para1            参数1
     * para2            参数2
     */
    int iterations = 0;
    int x_aixs_lines = 0;
    double random_0_1 = 0.0;
    int *best_path = NULL;
    double **tau = NULL;
    double **delta_tau = NULL;
    double **eta = NULL;
    int **path = NULL;
    double psum = 0.0;
    double pro_sum = 0.0;
    double *pro = NULL;
    double *ppro = NULL;
    double *cross_validation_accuracy = NULL;
    double *predict_accuracy = NULL;
    double *para1 = NULL;
    double *para2 = NULL;
    double err = 0.0;

    /* 参数检查相关辅助变量 */
    char ch = 0;
    int best_path_length = 0;//记录(para_aco_search->best_path的长度，即字符个数
    int best_path_non_num = 0;//标记para_aco_search->best_path中是否含有非数字字符
    int d1_length_befor_dot = -1;//第一个参数小数点之前的位数,此参数便于后面将最优蚂蚁路径转化成浮点参数输入到分类器算法
    int d1_length_after_dot = -1;//第一个参数小数点之后的位数
    int d2_length_befor_dot = -1;//第二个参数小数点之前的位数
//    int d2_length_after_dot = -1;//第二个参数小数点之后的位数
    int dot_num = 0;//字符串中小数点数量
    int comma_num = 0;//字符串中逗号数量
    FILE *output = NULL;//记录结果
    int index_cross = 0;
    int index_predict = 0;



    /* para_aco_search->best_path参数的检查以及best_path的初始化
     * best_path:每一次循环后，所有蚂蚁的路径中将产生一个最优路径
     * 首先，para_aco_search->best_path长度必须和para_aco_search->x_aixs_lines + 3长度相等
     * 其次，para_aco_search->best_path中的.和,数量必须分别为2，1
     * 最后，para_aco_search->best_path不能包含除.和,之外的非数字字符
     */

    /* 分配内存空间存储经验值最优路径 */
    best_path = (int *)malloc(sizeof(int) * para_aco_search->x_aixs_lines);

    best_path_length = strlen(para_aco_search->best_path);

    if(best_path_length == para_aco_search->x_aixs_lines + 3)//长度检查，字符串中去掉2个.和1个,后再长度应该等于para_aco_search->x_aixs_lines
    {
        best_path_non_num = 0;
        for(i = 0; i < best_path_length; i++)
        {
            ch = para_aco_search->best_path[i];

            if(ch == '.')//小数点数量检查
                dot_num++;
            else if(ch == ',')//逗号数量检查
                comma_num++;
            else if(!((ch >= '0' && ch <= '9') || ch == '.' || ch == ','))//除.和,之外的非数字字符检查
            {
                printf("best_path string must contain nothing except '0'-'9' and '.' and ','.  Wrong charater %c\n",ch);
                best_path_non_num++;
            }
        }

        if(best_path_non_num)//para_aco_search->best_path包含除.和,之外的非数字字符
            printf("%d unexpexted charaters occurred.\n",best_path_non_num);
        if(dot_num != 2)//小数点数量不对
            printf("The amount of dot(%d) is incorrect. It should equal 2\n",dot_num);
        if(comma_num != 1)//逗号数量不对
            printf("The amount of comma(%d) is incorrect. It should equal 1\n",comma_num);

        if(best_path_non_num || dot_num != 2 || comma_num != 1)
            return ERROR;
        else//读取数据
        {
//            /* 每只蚂蚁进入网格节点之前的原点坐标 */
//            best_path[0] = 0;
            j = 0;

            /* 将经验最优路径保存 */
            for(i = 0; i < best_path_length; i++)
            {
                if(para_aco_search->best_path[i] == '.' && d1_length_befor_dot == -1)
                    d1_length_befor_dot = i;//记录第一个参数小数点之前的位数
                else if(para_aco_search->best_path[i] == ',' && d1_length_befor_dot != -1)
                    d1_length_after_dot = i - d1_length_befor_dot - 1;//记录第一个参数小数点之后的位数
                else if(para_aco_search->best_path[i] == '.' && d1_length_after_dot != -1)
                    d2_length_befor_dot = i - d1_length_befor_dot - d1_length_after_dot - 2;//第二个参数小数点之前的位数
                else
                    best_path[j++] = para_aco_search->best_path[i] - '0';//保存最优路径到数组，将数字字符转化为整型数字,如 9 = '9' - '0'
            }
            //d2_length_after_dot = i  - d2_length_befor_dot - d1_length_befor_dot - d1_length_after_dot - 3;//第二个参数小数点之后的位数
        }
    }
    else//best_path长度不对
    {
        printf("the length of best_way string must equal para_aco_search->x_aixs_lines+3 = %d, but not %d!\n",para_aco_search->x_aixs_lines + 3, best_path_length);
        return ERROR;
    }

    /* 保存结果到文件 */
    if((output = fopen(para_aco_search->output_file, "w")) == NULL)
    {
        fprintf(stderr,"can't open output file %s\n",para_aco_search->output_file);
        exit(1);
    }

//    for(i=0;i<para_aco_search->x_aixs_lines;i++)
//        printf("best_path[%d] = %d\n",i,best_path[i]);
//    printf("%d\t%d\t%d\n",d1_length_befor_dot, d1_length_after_dot, d2_length_befor_dot);

    /* 二维数组的内存分配以及初始化 */
    tau = (double **)malloc(sizeof(double *) * para_aco_search->y_aixs_values);
    for(i = 0; i < para_aco_search->y_aixs_values; i++)
    {
        tau[i] = (double *)malloc(sizeof(double) * para_aco_search->x_aixs_lines);
        for(j = 0; j < para_aco_search->x_aixs_lines; j++)
        {
            tau[i][j] = para_aco_search->pheromone_initial_value;//初始化信息素矩阵元素的值为初始值C
        }
    }

    delta_tau = (double **)malloc(sizeof(double *) * para_aco_search->y_aixs_values);
    for(i = 0; i < para_aco_search->y_aixs_values; i++)
    {
        delta_tau[i] = (double *)malloc(sizeof(double) * para_aco_search->x_aixs_lines);
        memset(delta_tau[i], 0, sizeof(double) * para_aco_search->x_aixs_lines);//初始化信息素增量矩阵元素的值为0
    }

    eta = (double **)malloc(sizeof(double *) * para_aco_search->y_aixs_values);
    for(i = 0; i < para_aco_search->y_aixs_values; i++)
    {
        eta[i] = (double *)malloc(sizeof(double) * para_aco_search->x_aixs_lines);
        memset(eta[i], 0, sizeof(double) * para_aco_search->x_aixs_lines);//初始化能见度矩阵元素的值为0
    }

    path = (int **)malloc(sizeof(int *) * para_aco_search->ants_amount);
    for(i = 0; i < para_aco_search->ants_amount; i++)
    {
        path[i] = (int *)malloc(sizeof(int) * para_aco_search->x_aixs_lines);
        memset(path[i], 0, sizeof(int) * para_aco_search->x_aixs_lines);//初始化蚂蚁路径矩阵元素的值为0
    }

    pro = (double *)malloc(sizeof(double) * para_aco_search->y_aixs_values);
    memset(pro, 0, sizeof(double) * para_aco_search->y_aixs_values);

    ppro = (double *)malloc(sizeof(double) * para_aco_search->y_aixs_values);
    memset(ppro, 0, sizeof(double) * para_aco_search->y_aixs_values);

    cross_validation_accuracy = (double *)malloc(sizeof(double) * para_aco_search->ants_amount);
    memset(cross_validation_accuracy, 0, sizeof(double) * para_aco_search->ants_amount);

    predict_accuracy = (double *)malloc(sizeof(double) * para_aco_search->ants_amount);
    memset(predict_accuracy, 0, sizeof(double) * para_aco_search->ants_amount);

    para1 = (double *)malloc(sizeof(double) * para_aco_search->ants_amount);
    memset(para1, 0, sizeof(double) * para_aco_search->ants_amount);

    para2 = (double *)malloc(sizeof(double) * para_aco_search->ants_amount);
    memset(para2, 0, sizeof(double) * para_aco_search->ants_amount);

    /* 产生随机数种子，为下面产生随机数做准备 */
    srand(time(NULL));

    /* 开始迭代 */
    while(iterations++ < para_aco_search->max_iterations)
    {
        /*将x_aixs_lines清零 */
        x_aixs_lines = 0;

        /*将path清零 */
        for(i = 0; i < para_aco_search->ants_amount; i++)
        {
            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
            {
                path[i][j] = 0;
            }
        }

        /* 计算所有蚂蚁走过的路径 */
        /* 遍历每一个横坐标 */
        while(x_aixs_lines < para_aco_search->x_aixs_lines)
        {
            /* 遍历每一只蚂蚁 */
            for(i = 0; i < para_aco_search->ants_amount; i++)
            {
                /* 遍历横坐标上的每一个纵坐标值 */
                for(j = 0; j < para_aco_search->y_aixs_values; j++)
                {
                    eta[j][x_aixs_lines] = (10 - abs(j - best_path[x_aixs_lines])) / 10.0;
                    pro[j] = pow(tau[j][x_aixs_lines], para_aco_search->alpha) * pow(eta[j][x_aixs_lines], para_aco_search->beta);
                    psum += pro[j];
                }

                for(j = 0; j < para_aco_search->y_aixs_values; j++)
                {
                    pro[j] = pro[j] / psum;
                }

                /* 使用完后一定要清零 */
                psum = 0.0;

                /* 产生一个0-1之间的随机数 */
                random_0_1 = rand() / (RAND_MAX * 1.0);

                for(j = 0; j < para_aco_search->y_aixs_values; j++)
                {
                    pro_sum += pro[j];
                    ppro[j] = pro_sum;
                }

                /* 使用完后一定要清零 */
                pro_sum = 0.0;

                /* 计算蚂蚁下一步路径 */
                for(j = 0; j < para_aco_search->y_aixs_values; j++)
                {
                    if(random_0_1 <= ppro[j])//计算每一只蚂蚁的下一步路径
                    {
                        path[i][x_aixs_lines] = j;
                        //printf("path[%d][%d] = %d\trandom = %f\tppro[%d] = %f\n",i,x_aixs_lines,j,random_0_1,j,ppro[j]);
                        break;
                    }
                }

            }//对每只蚂蚁
            x_aixs_lines++;
        }//对每一个横坐标

        /* 打印本次蚂蚁的路劲 */
//        for(i = 0; i < para_aco_search->ants_amount; i++)
//        {
//            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
//            {
//                printf("path[%d][%d] = %d\n",i, j, path[i][j]);
//            }
//        }

        /* 将存储参数的数组清零 */
        memset(para1, 0, sizeof(double) * para_aco_search->ants_amount);
        memset(para2, 0, sizeof(double) * para_aco_search->ants_amount);

        /* 将所有蚂蚁的路径转化成参数，训练模型，对样本进行预测，得到正确率 */
        for(i = 0; i < para_aco_search->ants_amount; i++)
        {
            /* 转化参数 */
            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
            {
                if(j < d1_length_befor_dot + d1_length_after_dot)
                    para1[i] += path[i][j] * pow(10, d1_length_befor_dot - 1 - j);
                else
                    para2[i] += path[i][j] * pow(10, d2_length_befor_dot + d1_length_befor_dot + d1_length_after_dot - j - 1);
            }

            printf("para1[%d] = %f\tpara2[%d] = %f\n",i,para1[i],i,para2[i]);

            /* 代入支持向量机算法训练结构体 */
            if(para_svm_train->svm_train_parameter.svm_type == ONE_CLASS)
            {
                para_svm_train->svm_train_parameter.gamma = para1[i];
                para_svm_train->svm_train_parameter.nu = para2[i];
            }
            else if(para_svm_train->svm_train_parameter.svm_type == C_SVC)
            {
                para_svm_train->svm_train_parameter.gamma = para1[i];
                para_svm_train->svm_train_parameter.C = para2[i];
            }

            /* 设置交叉验证参数，进行交叉验证 */
            para_svm_train->cross_validation = para_aco_search->v_fold;

            /* 调用接口进行交叉验证 */
            if(main_svm_train(para_svm_train) == SUCCESS)
            {
                /* 记录第i只蚂蚁路径转化为参数后的交叉验证结果的正确率 */
                cross_validation_accuracy[i] = 100.0 * result_cross_validation_accuracy.correct / result_cross_validation_accuracy.total;

                /* 如果没有测试文件或者flag_predict为0，将不会使用该参数下的模型对测试样本做预测 */
                if(para_svm_predict->test_file && para_aco_search->flag_predict)
                {
                    /* 重新设置训练参数 */
                    para_svm_train->cross_validation = 0;//将验证折数设为0，表示不进行交叉验证而是进行模型训练

                    /* 调用接口进行模型训练 */
                    if(main_svm_train(para_svm_train) == SUCCESS)
                        printf("Model done!\n");

                    /* 调用接口进行测试 */
                    if(main_svm_predict(para_svm_predict) == SUCCESS)
                        printf("Predict done!\n");

                    /* 记录第i只蚂蚁路径转化为参数后的预测样本的正确率 */
                    predict_accuracy[i] = 100.0 * result_predict_accuracy.correct / result_predict_accuracy.total;
                }
            }

            /* 将第i只蚂蚁经过的节点的信息素增量更新 */
            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
                delta_tau[path[i][j]][j] = para_aco_search->Q * predict_accuracy[i] / 100.0;
        }

//        for(i = 0; i < para_aco_search->ants_amount; i++)
//        {
//            printf("cross_validation_accuracy[%d] = %f\n",i, cross_validation_accuracy[i]);
//            printf("predict_accuracy[%d] = %f\n",i, predict_accuracy[i]);
//        }

        /* 迭代结束，判断是否达到收敛条件 */
        err = Is_convergence(path, para_aco_search->ants_amount, para_aco_search->x_aixs_lines);

        /* 本轮所有蚂蚁的路径参数全部测试完成，选取最高的正确率记录到文件 */
        index_cross = max_index_func(cross_validation_accuracy, para_aco_search->ants_amount);
        index_predict = max_index_func(predict_accuracy, para_aco_search->ants_amount);
//        printf("index_cross = %d\tindex_predict = %d\n",index_cross, index_predict);

        /* 第一列数据：第i次迭代；
         * 第二列参数：第i次迭代中第index_cross只蚂蚁的交叉验证正确率最高；
         * 第三列参数：第i次迭代中第index_cross只蚂蚁的第一维参数
         * 第四列参数：第i次迭代中第index_cross只蚂蚁的第二维参数
         * 第五列参数：第i次迭代中第index_cross只蚂蚁的交叉验证正确率
         * 第六列参数：第i次迭代中第index_cross只蚂蚁的样本预测正确率
         * 第七列参数：第i次迭代中第index_predict只蚂蚁的预测正确率最高
         * 第八列参数：第i次迭代中第index_predict只蚂蚁的第一维参数
         * 第九列参数：第i次迭代中第index_predict只蚂蚁的第二维参数
         * 第十列参数：第i次迭代中第index_predict只蚂蚁的交叉验证正确率
         * 第十一列参数：第i次迭代中第index_predict只蚂蚁的样本预测正确率
         * 第十二列参数：第i次迭代中所有路径的方差收敛ERR
         */
        if(para_aco_search->flag_predict)
            fprintf(output, "%d\t%d\t%f\t%f\t%f\t%f\t%d\t%f\t%f\t%f\t%f\t%f\n",iterations,index_cross, para1[index_cross], para2[index_cross], cross_validation_accuracy[index_cross],predict_accuracy[index_cross], index_predict, para1[index_predict], para2[index_predict], cross_validation_accuracy[index_predict],predict_accuracy[index_predict],err);
        else
            fprintf(output, "%d\t%d\t%f\t%f\t%f\n",iterations,index_cross, para1[index_cross], para2[index_cross], cross_validation_accuracy[index_cross]);

        /* 判断是否达到收敛条件 */
        if(err < para_aco_search->ERR)
        {
            printf("iterations = %d\n\nThe ants converged automatically!\n",iterations);
            break;
        }
        else
            printf("iterations = %d\n\n",iterations);

        /* 将预测正确率最高的路径更新复制到best_path，目标函数为max{predict_accuracy[i]} i=0...ants_amount */
        for(i = 0; i < para_aco_search->x_aixs_lines; i++)
            best_path[i] = path[index_predict][i];

        /* 将所有节点的信息素更新 */
        for(i = 0; i < para_aco_search->y_aixs_values; i++)
        {
            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
            {
                tau[i][j] = tau[i][j] * para_aco_search->rho + delta_tau[i][j];
                if(tau[i][j] < para_aco_search->MIN_TAU)
                    tau[i][j] = para_aco_search->MIN_TAU;
                else if(tau[i][j] > para_aco_search->MAX_TAU)
                    tau[i][j] = para_aco_search->MAX_TAU;
            }
        }

        /* 将delta_atu信息素增量清零 */
        for(i = 0; i < para_aco_search->y_aixs_values; i++)
        {
            for(j = 0; j < para_aco_search->x_aixs_lines; j++)
            {
                delta_tau[i][j] = 0.0;
            }
        }
    }

    /* 使用完后释放空间 */
    free(tau);
    free(delta_tau);
    free(eta);
    free(path);
    free(best_path);
    free(pro);
    free(ppro);
    free(cross_validation_accuracy);
    free(predict_accuracy);

    fclose(output);
    return SUCCESS;
}

/* 传入一个二维数组(x * y)，x行y列，判断是否达到收敛条件，达到收敛返回err
   判断方法：比较二维数组的每一列的方差，所有列的方差求和，如果小于ERR则认为收敛
*/
double Is_convergence(int **arr, int x, int y)
{
    int i,j;
    double *average = NULL;
    double *variance = NULL;
    double sum_variance = 0.0;

    average = (double *)malloc(sizeof(double) * y);
    memset(average, 0, sizeof(double) * y);
    variance = (double *)malloc(sizeof(double) * y);
    memset(variance, 0, sizeof(double) * y);

    for(i = 0; i < y; i++)
    {
        for(j = 0; j < x; j++)
        {
            average[i] += *(*(arr + j) + i) * 1.0;
        }
        average[i] = average[i] / x;
    }

//    for(i = 0; i < y; i++)
//        printf("average[%d] = %f\n",i, average[i]);

    for(i = 0; i < y; i++)
    {
        for(j = 0; j < x; j++)
        {
            variance[i] += (average[i] -  *(*(arr + j) + i) * 1.0) * (average[i] -  *(*(arr + j) + i) * 1.0) ;
        }
        sum_variance += variance[i];
    }

    sum_variance = sum_variance / y;

//    for(i = 0; i < y; i++)
//        printf("variance[%d] = %f\n",i, variance[i]);

    free(average);
    free(variance);

    printf("ERR = %f\n",sum_variance);

    /* 该数值表达了path矩阵中，不同蚂蚁的路径中处在同一列上之间的位置距离平均偏差平方，数值越小，表明路径差异越小 */
    return sum_variance;
}
