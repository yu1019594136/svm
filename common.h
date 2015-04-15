/*
 * common.h
 *
 *  Created on: 2014年10月4日
 *      Author: zhouyu
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

enum RESULT{
	ERROR = 0,
	SUCCESS
};

enum SWITCH{
    CLOSE = 0,
    OPEN
};

enum STATUS{
    NO = 0,
    YES
};

int max_index_func(double *arr, int len);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
