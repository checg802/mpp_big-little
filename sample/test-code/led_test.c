
//串口相关的头文件  
#include<stdio.h>      /*标准输入输出定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h>  
   
   
//宏定义  
#define FALSE  -1  
#define TRUE   0  

int gpio_test_in(unsigned int gpio_chip_num,unsigned int gpio_offset_num)
{
	FILE *fp;
	char file_name[50];
	unsigned char buf[10];
	unsigned int gpio_num;

	gpio_num = gpio_chip_num * 8 + gpio_offset_num;

	sprintf(file_name,"/sys/class/gpio/export");
	fp = fopen(file_name,"w");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}

	fprintf(fp,"%d",gpio_num);
	fclose(fp);


	sprintf(file_name,"/sys/class/gpio/gpio%d/direction",gpio_num);
	fp = fopen(file_name,"rb+");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}

	fprintf(fp,"in");
	fclose(fp);


	sprintf(file_name,"/sys/class/gpio/gpio%d/value",gpio_num);
	fp = fopen(file_name,"rb+");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}
	memset(buf,0,10);
	fread(buf,sizeof(char),sizeof(buf) - 1,fp);
	printf("%s: gpio%d_%d = %d\n",__func__,gpio_chip_num,gpio_offset_num,buf[0]-48);
	fclose(fp);

	sprintf(file_name,"sys/class/gpio/unexport");
	fp = fopen(file_name,"w");
	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}	
	fprintf(fp,"%d",gpio_num);
	fclose(fp);

	return (int)(buf[0]-48);
	
}

int gpio_test_out(unsigned int gpio_chip_num,unsigned int gpio_offset_num,unsigned int gpio_out_val)
{
	FILE *fp;
	char file_name[50];
	unsigned char buf[10];
	unsigned int gpio_num;

	gpio_num = gpio_chip_num * 8 + gpio_offset_num;

	sprintf(file_name,"/sys/class/gpio/export");
	fp = fopen(file_name,"w");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}

	fprintf(fp,"%d",gpio_num);
	fclose(fp);


	sprintf(file_name,"/sys/class/gpio/gpio%d/direction",gpio_num);
	fp = fopen(file_name,"rb+");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}

	fprintf(fp,"out");
	fclose(fp);


	sprintf(file_name,"/sys/class/gpio/gpio%d/value",gpio_num);
	fp = fopen(file_name,"rb+");

	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}
	if(gpio_out_val)
		strcpy(buf,"1");
	else
		strcpy(buf,"0");
	fwrite(buf,sizeof(char),sizeof(buf) - 1,fp);
	printf("%s: gpio%d_%d = %s\n",__func__,gpio_chip_num,gpio_offset_num,buf);
	fclose(fp);

	sprintf(file_name,"/sys/class/gpio/unexport");
	fp = fopen(file_name,"w");
	if(fp == NULL){
		printf("Cannot open %s.\n",file_name);
		return -1;
	}	
	fprintf(fp,"%d",gpio_num);
	fclose(fp);

	return 0;
	
}

int main(int argc, char **argv) 
{

	if(*argv[1]-48==2){
		while(1){
			printf("*******\r\n");
			usleep(50000); //50ms
		}
	}
	else{
		printf("input err\r\n");
	}

	printf("################## end #############\n");

	return 0;


	

	
	
}
   

