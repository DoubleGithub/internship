#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
int main(int argc,char *argv[])
{
	char string[]="hello,first module";
	//FILE *fpr=fopen(argv[1],"r+");
	//fread(string,sizeof(string),1,fpr);
	//fwrite(string,sizeof(string),1,fpr);
	//fclose(fpr);

	int fp=open(argv[1],O_RDWR|O_NONBLOCK,0X777);
	if(fp<0)
	{
		printf("failed open file\n");
		return 0;
	}

	write(fp,string,sizeof(string));
	memset(string,'\0',sizeof(string));
	read(fp,string,sizeof(string));
	printf("user space :%s\n",string);
	ioctl(fp,1);
	close(fp);
	return 0;
}
