//参考:https://blog.csdn.net/wcl719236538/article/details/55251368

//串口相关的头文件  
#include<stdio.h>      /*标准输入输出定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<termios.h>    /*PPSIX 终端控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h>  
   
   
//宏定义  
#define FALSE  -1  
#define TRUE   0  
  
void ptr_usage()
{
		printf("Usage: system_test 4g/2450/wifi/sensor  \n"); 
		printf("Usage: system_test led gpio_chip_num  gpio_offset_num  val   \n"); 
		printf("Sample:  \n"); 
		printf("        system_test 4g  \n"); 
		printf("        system_test led 7 4 0  \n");
}
int main(int argc, char **argv)  
{  
    int fd;                            //文件描述符  
    int err;                           //返回调用函数的状态  
    int len;                          

    if(argc < 2)  
	{  
/*  
		printf("Usage: %s 4g/2450/wifi/sensor/  \n",argv[0]); 
		printf("Usage: %s led gpio_chip_num  gpio_offset_num  val   \n",argv[0]); 
		printf("Sample:  \n"); 
		printf("        %s 4g  \n",argv[0]);  
		printf("        %s led 7 4 0  \n",argv[0]);
*/
		ptr_usage();
		return FALSE;  
	} 	
	if(strcmp(argv[1],"4g")==0)
	{
		system(" ifconfig wwan0 up");
		system(" ../4g/serial_test /dev/ttyUSB2");
		system(" udhcpc -i wwan0");		
	}
	else if(strcmp(argv[1],"2450")==0)
	{
	//	system(" pwd");
	//	system(" env && cd ../../lib/ && pwd && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD && env");
		system(" ../2450/main-2450");
	}
	else if(strcmp(argv[1],"wifi")==0)
	{
		system(" ../wifi/wifi_test_send /dev/ttyAMA2");
	}
	else if(strcmp(argv[1],"sensor")==0)
	{
		system(" cd ../../Hi3519V101_Stream_V1.0.3.0/ && ./HiIspTool.sh -s imx185");
	}

	else if(strcmp(argv[1],"led")==0)
	{
		if(argc < 5) 
		{
			ptr_usage();
			return FALSE; 
		}
		system(" ../led/led-test 7 4 2");
						
	}

	else
	{
		ptr_usage();
	}
/*
	switch(argv[1])
	{
		case "4g":
			system(" ifconfig wwan0 up");
			system(" ../4g/serial_test /dev/ttyUSB2");
			system(" udhcpc -i wwan0");
			break;
		case "2450":
			system(" ../2450/main-2450");
			break;
		case "wifi":
			system(" ../wifi/wifi_test_send /dev/ttyAMA2");
			break;
		default:
			ptr_usage();
			break;
	}
*/
	return TRUE;
//	system(" pwd && cd ../ && pwd && cd ../ && pwd && sample/led/led-test 7 5 0");
//	system(" ../4g/serial_test /dev/ttyUSB2");
//	system(" udhcpc -i wwan0");   
//	system(" ifconfig wwan0 up");
//	system(" ../4g/serial_test /dev/ttyUSB2");
//	system(" udhcpc -i wwan0");


}  
