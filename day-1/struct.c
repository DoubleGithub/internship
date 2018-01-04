#include<stdio.h>
#include<string.h>
#include<assert.h>
struct student
{
	int num;
	char name[20];
	float score;
};
void initial_stu(struct student *pload,int num)
{
	int i=0;
	for(i=0;i<num;i++)
	{
		printf("please input num name score\n");
		scanf("%d %s %f",&(pload+i)->num,(pload+i)->name,&(pload+i)->score);
	}
}
void output_stu(struct student *pload,int num)
{
	int i=0;
	for(i=0;i<num;i++)
	{
		printf("%d %s %6.2f\n",pload[i].num,pload[i].name,(pload+i)->score);
	}	
}
void write_stu_file(struct student *pload,int num,char *path)
{
	int i=0;
	int retur=0;
	FILE * fpw=fopen(path,"wb");
	assert(fpw!=NULL);
	for(i=0;i<num;i++)
	{
		retur=fwrite(pload+i,sizeof(struct student),1,fpw);
		printf("retur = %d\n",retur);
	}
	fclose(fpw);
}
void read_stu_file(struct student *pload,int num,char *path)
{
	int i=0;
	FILE * fpr=fopen(path,"rb");
	assert(fpr!=NULL);
	while(fread(pload+i,sizeof(struct student),1,fpr)==1)
	{
		printf("%d %s %6.2f\n",pload[i].num,pload[i].name,(pload+i)->score);
	}
	fclose(fpr);
}
int main(int argc ,char *argv[])
{
	struct student stu[5],stu1[5];
	struct student *pstu=stu;
	initial_stu(pstu,5);
	write_stu_file(stu,5,argv[1]);
	memset(stu,0,sizeof(stu));
	//read_stu_file(stu,5,argv[1]);
	read_stu_file(stu1,5,argv[1]);
	return 1;	
}
