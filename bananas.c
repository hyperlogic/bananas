#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "parse.h"
#include "prim.h"

void test(const char* input)
{
    obj_t* obj = prim_eval(read_string(input));
    dump(obj, 0);
    printf("\n");
}

void test_suite()
{
    test("10");  // 1.0
    test("'(1 2 3)"); // (1 2 3)
    test("(+ 1 2)");  // 3
    test("(def ten 10)");  // 10
    test("ten");  // 10
    test("car");  // prim
    test("(def plus-ten (lambda (x) (+ x ten)))");  // closure
    test("(def ten 0)");  // 0
    test("(plus-ten 10)");  // 20
}

int main(int argc, char* argv[])
{
    init();

    test_suite();

    char* line = 0;
    while (1) {
        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
            return 0;
        if (line && *line)
            add_history(line);
        obj_t* n = read_string(line);

        /*
        printf("read_string = \n    ");
        dump(n, 0);
        printf("\n");
        */

        free(line);
        obj_t* r = prim_eval(n);
        printf("/o_O\\ => ");
        dump(r, 0);
        printf("\n");
    }
}
