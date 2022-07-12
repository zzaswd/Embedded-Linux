#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ioctl_test.h"

#define DEVICE_FILENAME  "/dev/ledkey_block"

int main()
{
	ioctl_test_info info = {0,{0}};
	long now_j = 0;
    int dev;
    char buff = 15;
    int ret;
    int key_old = 0;
	int cnt = 0;
	long temp = 0;
//    dev = open( DEVICE_FILENAME, O_RDWR|O_NDELAY );
//    dev = open( DEVICE_FILENAME, O_RDWR|O_NONBLOCK);
      dev = open( DEVICE_FILENAME, O_RDWR);   //block mode
	if(dev<0)
	{
		perror("open()");
		return 1;
	}

	long pre_j= 0;
	long dif_j = 0;
    ret = write(dev,&buff,sizeof(buff));
	if(ret < 0)
		perror("write()");
	buff = 0;
	do {
    	ret = read(dev,&buff,sizeof(buff));              
		ioctl(dev,IOCTLTEST_INTVAL,&temp);
		//ioctl(dev,IOCTLTEST_INTVAL,now_j);
		printf("temp = %ld\n",temp);
		
		dif_j = temp;

  		printf("ret : %d, cnt : %d\n",ret,cnt++);
		printf("time_dif_hz : %d\n",dif_j);
		printf("time_dif_s : %.2lf\n",(dif_j*1.0)/100.0);
		pre_j = now_j;

		if(ret < 0)
		{
  			perror("read()");
			return 1;
		}

		if(buff == 0) //sw_no : 0
			continue;
		if(buff != key_old)
		{
			printf("key_no : %d\n",buff);
			write(dev,&buff,sizeof(buff));
			if(buff == 8)
				break;
			key_old = buff;
		}
	} while(1);
    close(dev);
    return 0;
}
