#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_FILENAME "/dev/calldev"

int main(void){
	int dev;
	char buff[128];
	int ret;
	printf("1) device file open\n");
	
	dev = open(DEVICE_FILENAME, O_RDWR | O_NDELAY);

	if( dev >= 0 )
	{
		printf("2) seek function call dev:%d\n",dev);
		ret = lseek(dev,0x20,SEEK_SET);
		printf("ret = %08x\n",ret);
		printf("3) read function\n");
		ret =  read(dev,(char *)0x30,0x31);
		printf("ret = %08x\n",ret);
		printf("4) write function call\n");
		ret = write(dev,(char *)0x40, 0x41);
		printf("ret = %08x\n",ret);
		printf("5) ioctl function call\n");
		ret = ioctl(dev,0x51,0x52);
		printf("ret = %08x\n",ret);
		printf("6) device file close\n");
		ret = close(dev);
		printf("ret = %08x\n",ret);
	}
	else
		perror("open");
	return 0;
}

