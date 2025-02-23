#include <stdio.h>

int main()
{   
    FILE* cmdp = popen("ps -ef | grep root","r");

    if(!cmdp)
    {
        perror ("popen");
        return 1;
    }

    char result [256];

    while(fgets(result,sizeof(result), cmdp))
        fputs(result, stdout);/*Вывод всех строк*/

    pclose(cmdp);
    return 0;
}