#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "parse.h"
#include "prim.h"

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
        node_t* n = read_string(line);

        /*
        printf("read_string = \n    ");
        DUMP(n, 0);
        printf("\n");
        */

        free(line);
        node_t* r = eval(n);
        printf("/o_O\\ => ");
        DUMP(r, 0);
        printf("\n");
    }
}
