/*
 * common.c
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */

#include "common.h"

int max_index_func(double *arr, int len)
{
    int i;
    int index = 0;
    double temp = arr[0];

    for(i = 1; i < len; i++)
    {
        if(temp < arr[i])
        {
            temp = arr[i];
            index = i;
        }
    }
    return index;
}
