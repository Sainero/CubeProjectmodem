/*
 * persapp.c
 *
 *  Created on: 24 нояб. 2021 г.
 *      Author: kosty
 */

#include "Counter.h"
static int cnt = 0;

int cntReset()
{
	cnt = 0;
	return 0;
}
int cntInc()
{
	cnt++;
	return 0;
}
int cntGetValue()
{
	return cnt;
}
int cntSetValue(int Value1)
{
	cnt = Value1;
	return 0;
}
int cntdec() // me
{
	cnt--;
	return 0;
}
