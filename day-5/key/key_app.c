#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char *argv[])
{
	int i=0;
	int j=0;
	int retur=0,fbeep=0;	//定义两个文件描述符
	int fd =0;
	fd_set rdfs;	//定义一个文件描述符集
	int key_value[4]={0};
	fd=open(argv[1],O_RDONLY,0x777);   //O_NONBLOCK非阻塞
	fbeep=open(argv[2],O_RDONLY,0x777);	//以只读方式打开
	assert(fd>=0&&fbeep>=0);	//判断是否正确打开设备，否则，退出
	
	FD_ZERO(&rdfs);		//清除一个文件描述符集
	FD_SET(fd,&rdfs);	//将一个文件描述符加入到文件描述符集中
		printf("before select,retur=%d\n",retur);
	retur = select(fd+1,&rdfs,NULL,NULL,NULL);	//调用轮循
		printf("after  select,retur =%d\n",retur);
	if(retur<0)
	{
		printf("failed in select\n");
		return 0;
	}
	while(1)
	{
		if(FD_ISSET(fd,&rdfs))	//判断文件描述符是否被置位
		{
			memset(key_value,0,sizeof(key_value));
			read(fd,key_value,sizeof(key_value));
			for(i=0;i<sizeof(key_value)/sizeof(int);i++)
			{
				if(key_value[i]==1)
				{
					printf("key%d is pushed down\n",i);
					for(j=0;j<=i;j++)
					{
						ioctl(fbeep,1,0);
						sleep(1);
						ioctl(fbeep,0,0);
						sleep(1);
					}
				}
			}
		}
	}
	close(fd);
	close(fbeep);
	return 0;
}
