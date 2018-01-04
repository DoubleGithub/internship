#include <stdio.h>
#include <assert.h>
int main(int argc,char *argv[])
{
    FILE *fpr=NULL;
    FILE *fpw=NULL;
    char string[20];
    fpr=fopen(argv[1],"r");
    fpw=fopen(argv[2],"w");
    assert(fpr!=NULL && fpw!=NULL);
    while(fgets(string,20,fpr)!=NULL)
    {
        //fputs(string,stdout);
        fputs(string,fpw);
    }
    fclose(fpr);
    fclose(fpw);
    return 0;
}
