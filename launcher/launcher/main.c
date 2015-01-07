#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    printf("starting... %s\n",argv[1]);
    if (0==vfork())
    {
        if (argc==2)
            execlp(argv[1], NULL);
        else
        {
            char **args = malloc(sizeof(char*)*(argc-2+1));
            int i;
            for (i=2;i<argc;i++)
            {
                args[i-2]=argv[i];
            }
            args[argc-2] = 0;
            execvp(argv[1], args);
        }
    }
    return 0;
}
