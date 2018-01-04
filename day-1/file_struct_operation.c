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
int main(int argc,char *argv[])
{
    student stu[3];
    printf("%d\n",sizeof(student));
    return 0;
}
