#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "obj.h"
#include "parse.h"
#include "prim.h"

// return 1 on failure
int run_test(const char* input, obj_t* expected)
{
    obj_t* result = eval(read(input), g_env);
    if (!is_equal(result, expected)) {
        fprintf(stderr, "FAILED: evaluating \"%s\"\n", input);
        fprintf(stderr, "    expected = \n");
        fprintf(stderr, "        ");
        dump(expected, 1);
        fprintf(stderr, "\n    result = \n");
        fprintf(stderr,"        ");
        dump(result, 1);
        fprintf(stderr, "\n");
        return 1;
    } else
        return 0;
}

typedef enum { TEST = 0, EVAL, END } test_type;

typedef struct {
    test_type type;
    const char* string;
    obj_t* expected;
} unit_test_t;

void test_suite()
{
    unit_test_t g_unit_tests[] = {
        {TEST, "10", make_number(10)},
        {TEST, "'(1 2 3)", list3(make_number(1), make_number(2), make_number(3))},
        {TEST, "(+ 1 2)", make_number(3)},
        {EVAL, "(def ten 10)", NULL},
        {TEST, "ten", make_number(10)},
        {EVAL, "(def plus-ten (lambda (x) (+ x ten)))", NULL},
        {EVAL, "(def ten 0)", NULL},
        {TEST, "ten", make_number(0)},
        {TEST, "(plus-ten 10)", make_number(20)},
        {TEST, "'(1 . 2)", cons(make_number(1), make_number(2))},
        {EVAL, "(def ten nil)"},
        {EVAL, "(def plus-ten nil)"},
        //{EVAL, "(def factorial (lambda (n) (if (eq? n 0) 1 (* n (factorial (- n 1))))))"},
        //{TEST, "(factorial 0)", make_number(1)},
        {END, NULL, NULL}
    };

    int num_tests = 0;
    int num_fails = 0;
    unit_test_t* test = g_unit_tests;
    while (test->type != END) {
        if (test->type == TEST) {
            num_tests++;
            num_fails += run_test(test->string, test->expected);
        } else if (test->type == EVAL) {
            eval(read(test->string), g_env);
        }
        test++;
    }

    if (num_fails > 0) {
        fprintf(stderr, "( %d / %d ) tests failed!\n", num_fails, num_tests);
    }
}

int main(int argc, char* argv[])
{
    init();

    test_suite();

    char* line = 0;
    while (1) {

        printf("  %d used objs\n", g_num_used_objs);

        line = readline("\\O_o/ > ");
        if (strcmp(line, "quit") == 0)
            return 0;
        if (line && *line)
            add_history(line);
        obj_t* n = read(line);
        ref(n);
        free(line);
        obj_t* r = eval(n, g_env);
        ref(r);
        unref(n);
        printf("  ");
        dump(r, 0);
        printf("\n");
        unref(r);
    }
}
