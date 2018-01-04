#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
typedef struct Student
{
    int num;
    char name[20];
    float score;
} student;
void initial_struct(student *pstu,int count)
{
    int i;
    for(i=0;i<count;i++)
    {
        printf("please input num name score\n");
        scanf("%d%s%f",&(pstu+i)->num,(pstu+i)->name,&(pstu+i)->score);
    }
}
void output_struct(student *pstu,int count)
{
    int i;
    for(i=0;i<count;i++)
    {
        printf("%d\t%s\t%f\n",(pstu+i)->num,(pstu+i)->name,(pstu+i)->score);
    }
}
void file_write(student *pstu,int count,char *path)
{
    int i;

}
int main(int argc,char *argv[])
{
    student stu[3];
    FILE *fpr=NULL;
    FILE *fpw=NULL;
    FILE *fprb=NULL;
    FILE *fpwb=NULL;
    
    initial_struct(stu,3);
    output_struct(stu,3);
    //printf("%d\n",sizeof(student));
    return 0;
}
