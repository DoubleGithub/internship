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
void file_write(student *pstu,int count)
{
    int i=0;
    int retur=0;
    //char string[20];
    FILE *fpw=fopen("datasave","w");
    FILE *fpwb=fopen("datasave_b","wb");
    assert(fpw!=NULL&&fpwb!=NULL);
    while(i<count)
    {
        fprintf(fpw,"%d\t%s\t%f\n",(pstu+i)->num,(pstu+i)->name,(pstu+i)->score);
        i++;
    }
    for(i=0;i<count;i++)
    {
        retur=fwrite(pstu+i,sizeof(student),1,fpwb);
        printf("retur = %d\n",retur);
    }
    fclose(fpw);
    fclose(fpwb);
}
void file_read(student *pstu,int count)
{
    int i=0;
    FILE *fpr=fopen("datasave","r");
    FILE *fprb=fopen("datasave_b","rb");
    assert(fpr!=NULL&&fprb!=NULL);
    printf("read datasave\n");
    while(fscanf(fpr,"%d\t%s\t%f\n",&(pstu+i)->num,(pstu+i)->name,&(pstu+i)->score)!=EOF)
    {
        printf("%d\t%s\t%f\n",(pstu+i)->num,(pstu+i)->name,(pstu+i)->score);
        i++;
    }
    i=0;
    printf("read datasave_b\n");
    while(fread(pstu+i,sizeof(student),1,fprb)==1)
    {
        printf("%d\t%s\t%f\n",(pstu+i)->num,(pstu+i)->name,(pstu+i)->score);
        i++;
    }

    fclose(fpr);
    fclose(fprb);
}
int main(int argc,char *argv[])
{
    student stu[3];  
    initial_struct(stu,3);
    output_struct(stu,3);
    printf("\n");
    file_write(stu,3);
    file_read(stu,3);
    //output_struct(stu,3);
    //printf("%d\n",sizeof(student));
    return 0;
}
