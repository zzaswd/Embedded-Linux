#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>

#define DEVICE_FILENAME "/dev/ledkey_poll"

int main(int argc, char *argv[])
{
	int dev;
	char buff;
	int ret;
	int num = 1;
	struct pollfd Events[2];
	char keyStr[80];

    if(argc != 2)
    {
        printf("Usage : %s [led_data(0x0~0xf)]\n",argv[0]);
        return 1;
    }
    buff = (char)strtoul(argv[1],NULL,16);
    if((0 > buff) || ( 15 < buff))
    {
        printf("Usage : %s [led_data(0x0~0xf)]\n",argv[0]);
        return 2;
    }

//  dev = open(DEVICE_FILENAME, O_RDWR | O_NONBLOCK);
  	dev = open(DEVICE_FILENAME, O_RDWR );
	if(dev < 0)
	{
		perror("open");
		return 2;
	}
	write(dev,&buff,sizeof(buff));

	fflush(stdin); // 표준 입력 장치 stdin에 들어있는 값을 clear 해라.
	memset( Events, 0, sizeof(Events));
  	Events[0].fd = fileno(stdin);	// fileno는 파일 포인터를 fd로 바꿔준다. 즉, 키보드 장치를 등록
  	Events[0].events = POLLIN;	// read 이벤트를 감지하겠다.
	Events[1].fd = dev; // 디바이스 드라이버 장치 디스크립터를 등록
	Events[1].events = POLLIN;	// read 이벤트 감지하겠다.

	while(1)
	{
		ret = poll(Events, 2, 2000);	// Event에 있는 파일 디스크립터 2개를 확인한느데 2초마다 wake 하겠다.
		if(ret<0)	// 에러
		{
			perror("poll");
			exit(1);
		}
		else if(ret==0)	// ret == 0 이면 타임아웃
		{
  			printf("poll time out : %d Sec\n",2*num++);
			continue;
		}
		if(Events[0].revents & POLLIN)  //stdin. keyboard로부터 입력값이 감지된다면,
		{
			fgets(keyStr,sizeof(keyStr),stdin);
			if(keyStr[0] == 'q')
				break;
			keyStr[strlen(keyStr)-1] = '\0';	// \n을 없애기 위한 코드.
			printf("STDIN : %s\n",keyStr);
			buff = (char)atoi(keyStr);
			write(dev,&buff,sizeof(buff));
		}
		else if(Events[1].revents & POLLIN) //keyled 
		{
			ret = read(dev,&buff,sizeof(buff));
			printf("key_no : %d\n",buff);
			write(dev,&buff,sizeof(buff));
			if(buff == 8)
				break;
		}
	}
	close(dev);
	return 0;
}
