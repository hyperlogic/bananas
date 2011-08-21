#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "parse.h"

int main(int argc, char* argv[])
{
    init();
    char* line = 0;
    while (1) {
        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
            return 0;
        if (line && *line)
            add_history(line);
        node_t* expr = read(line);
        free(line);
        print(expr);
        /*
        node_t* result = eval(expr);
        printf("=> ");
        print(result);
        */
        printf("\n");
    }
}
