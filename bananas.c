#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "obj.h"
#include "parse.h"
#include "prim.h"

int main(int argc, char* argv[])
{
    obj_init();

    obj_t* repl_env = obj_make_environment(KNULL, g_env);
    char* line = NULL;
    while (1) {

        printf("  %d used objs\n", g_num_used_objs);

        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
        {
            free(line);
            break;
        }
        if (line && *line)
            add_history(line);

        obj_t* result = obj_eval_str(line, repl_env);
        free(line);

        printf("  ");
        obj_dump(result, 0);
        printf("\n");
    }

    return 0;
}
