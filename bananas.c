#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "obj.h"
#include "parse.h"
#include "prim.h"

extern int g_num_stack_frames; // from obj.c
extern int g_num_stack_objs; // from obj.c

int main(int argc, char* argv[])
{
    obj_init();

    PUSHF();
    obj_t* repl_env = PUSH(obj_make_environment(KNULL, g_env));
    char* line = NULL;
    while (1) {

        printf("   before gc: %d used objs\n", g_num_used_objs);
        obj_gc();
        printf("   after gc: %d used objs\n", g_num_used_objs);
        printf("   g_num_stack_frames = %d\n", g_num_stack_frames);
        printf("   g_num_stack_objs = %d\n", g_num_stack_objs);

        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
        {
            free(line);
            break;
        }
        if (line && *line)
            add_history(line);

        PUSHF();
        obj_t* result = PUSH(obj_eval_str(line, repl_env));
        free(line);

        printf("  ");
        obj_dump(result, 0);
        printf("\n");
        POPF();
    }
    POPF();

    return 0;
}
