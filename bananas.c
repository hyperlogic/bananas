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

    obj_stack_frame_push();
    obj_stack_push(obj_make_environment(KNULL, g_env));
    obj_t* repl_env = obj_stack_get(0);
    char* line = NULL;
    while (1) {

        printf("  before gc: %d used objs\n", g_num_used_objs);
        obj_gc();
        printf("   after gc: %d used objs\n", g_num_used_objs);

        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
        {
            free(line);
            break;
        }
        if (line && *line)
            add_history(line);

        obj_stack_frame_push();
        obj_stack_push(read(line));
        //obj_stack_push(obj_eval_str(line, repl_env));
        free(line);

        printf("  ");
        obj_dump(obj_stack_get(0), 0);
        printf("\n");
        obj_stack_frame_pop();
    }
    obj_stack_frame_pop();

    return 0;
}
