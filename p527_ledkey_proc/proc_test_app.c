#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_LED_FILENAME "/proc/ledkey/led"
#define DEVICE_KEY_FILENAME "/proc/ledkey/key"

int main(int argc, char *argv[])
{
	int ledFd;
	int keyFd;

	int dev;
	char buff[10];

	if(argc < 2)
	{
		printf("Usage : %s [0~127]\n",argv[0]);
		return 1;
	}

	ledFd = open(DEVICE_LED_FILENAME, O_WRONLY);
	if(ledFd < 0)
	{
		perror("open");
		return 2;
	}

	keyFd = open(DEVICE_KEY_FILENAME, O_RDONLY);
	if(keyFd < 0)
	{
		perror("open");
		return 3;
	}

	strcpy(buff,argv[1]);
	//write(dev,buff,sizeof(buff));
	write(ledFd,buff,strlen(buff)); // buff 값은 널까지의 길이를 알아올 수 있으므로 strlen
	printf("write : %s\n",buff);

	//read(dev,buff,sizeof(buff));
	read(keyFd,buff,sizeof(buff)); // key
	printf("read : %s\n",buff);

	//close(dev);
	close(ledFd);
	close(keyFd);
	return 0;
}
