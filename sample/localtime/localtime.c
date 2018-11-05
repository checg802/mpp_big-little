
//串口相关的头文件  
#include<stdio.h>      /*标准输入输出定义*/  
#include<time.h>     /*标准函数库定义*/  
   
   
//宏定义  
#define FALSE  -1  
#define TRUE   0  



int main(int argc, char **argv) 
{
	struct tm *t;
	time_t tt;
	time_t ts;
 
	struct tm tr = {0};
 
	time(&tt);
	t = localtime(&tt);
	printf("localtime %4d%02d%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
 
	localtime_r(&tt, &tr);
	printf("localtime_r %4d%02d%02d %02d:%02d:%02d\n", tr.tm_year + 1900, tr.tm_mon + 1, tr.tm_mday, tr.tm_hour, tr.tm_min, tr.tm_sec);
 
	ts = tt + 1800;
	t = localtime(&ts);
	printf("localtime %4d%02d%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
 
	localtime_r(&ts, &tr);
	printf("localtime_r %4d%02d%02d %02d:%02d:%02d\n", tr.tm_year + 1900, tr.tm_mon + 1, tr.tm_mday, tr.tm_hour, tr.tm_min, tr.tm_sec);
 
	return 0;
	
}
   

