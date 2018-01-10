#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char *argv[])
{
	int fd =0;
	int i=0;
	int j=0;
	int retur=0;
	int fbeep=0;
	fd_set rdfs;
	int key_value[4]={0};
	fd=open(argv[1],O_RDONLY,0x777);
	fbeep=open(argv[2],O_RDONLY,0x777);
	assert(fd>=0&&fbeep>=0);
	
	FD_ZERO(&rdfs);
	FD_SET(fd,&rdfs);
	retur = select(fd+1,&rdfs,NULL,NULL,NULL);
	if(retur<0)
	{
		printf("failed in select\n");
		return 0;
	}
	while(1)
	{
		if(FD_ISSET(fd,&rdfs))
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
