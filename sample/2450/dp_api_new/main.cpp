#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <iostream>
#include "dp_api.h"

using namespace std;

int main()
{
	int ret;
	ret = dp_init();
	if(ret != 0 )
	{
		cout << "dp_init failed!" << endl;
		return ret;
	}
	
	ret = dp_ping();
	if(ret == 0)
	{
		cout << "dp_ping successfully!" << endl;
	}
	else
	{
		cout << "dp_ping failed" << endl;
		return ret;
	}
	
	dp_hw_status_t status;
	ret = dp_hardware_test(&status);
	cout << "dp_hw_status" << status << endl;
	if(ret == 0)
	{
		cout << "dp_hardware_test successfully!" << endl; 
	}
	else
	{
		cout << "dp_hardware_test failed" << endl;
		return ret;
	}
}