
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

	sprintf(file_name,"/sys/class/gpio/unexport");
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

void bling(unsigned int gpio_chip_num,unsigned int gpio_offset_num)
{
	static int val = 0;

	gpio_test_out(gpio_chip_num,gpio_offset_num,val);
	if(val)
		val = 0;
	else
		val = 1;
}

int main(int argc, char **argv)
{
	int alarmX_val=0;
    if(argc != 2){
		
		printf("Usage: %s alarm_id  ;alarm_id = 1 or 2 or 3\n",argv[0]); 
		return FALSE;  
	} 
	if(*argv[1]-48==1){
		alarmX_val = gpio_test_in(1,7);
		gpio_test_out(16,2,alarmX_val);
		printf("alarm 1 test succeed ,alarmX_val = %d \n",alarmX_val);
	}
	else if(*argv[1]-48==2){
		alarmX_val = gpio_test_in(1,6);
		gpio_test_out(16,3,alarmX_val);
		printf("alarm 1 test succeed ,alarmX_val = %d \n",alarmX_val);
	}
	else if(*argv[1]-48==3){
		alarmX_val = gpio_test_in(1,7);
		gpio_test_out(16,2,alarmX_val);
		printf("alarm 1 2 test succeed ,alarm1_val = %d \n",alarmX_val);
		alarmX_val = gpio_test_in(1,6);
		gpio_test_out(16,3,alarmX_val);
		printf("alarm 1 2 test succeed ,alarm2_val = %d \n",alarmX_val);
	}
	else{
		printf("Usage: %s alarm_id  ;alarm_id = 1 or 2 or 3\n",argv[0]); 
		return FALSE;
	}
	return TRUE;
}

#if 0
int main(int argc, char **argv) 
{
    if(argc != 4){  

		//printf("Usage: %s /dev/ttySn 0(send data)/1 (receive data) \n",argv[0]);  
		printf("Usage: %s gpio_chip_num  gpio_offset_num  val  ;val = 0 or 1 or 2 bling\n",argv[0]); 
		return FALSE;  
	} 

	if(*argv[3]-48>2){
		printf("Usage: val  must be  0 or 1 or 2 bling\n"); 
		printf("Usage: %s gpio_chip_num  gpio_offset_num  val  ;val = 0 or 1 or 2 bling\n",argv[0]); 
		return FALSE;
	}
	
	if(*argv[3]-48==2){
		while(1){
			bling(*argv[1]-48,*argv[2]-48);
			usleep(100000);
		}
	}
	

//	gpio_test_out(1,6,*argv[1]);
	gpio_test_out(*argv[1]-48,*argv[2]-48,*argv[3]-48);
	printf("GPIO%d_%d =  %d.\n",*argv[1]-48,*argv[2]-48,*argv[3]-48);

	return 0;
	
}
   
#endif


