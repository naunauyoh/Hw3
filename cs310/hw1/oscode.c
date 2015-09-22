#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
        exit(-1);
    }

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
        *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }

    return i;
}


int main()
{
    char *args[20];
    int bg;
    int cnt = getcmd("\n>>  ", args, &bg);

    for (int i = 0; i < cnt; i++)
        printf("\nArg[%d] = %s", i, args[i]);

    if (bg)
        printf("\nBackground enabled..\n");
    else
        printf("\nBackground not enabled \n");

    printf("\n\n");
}
