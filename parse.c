#include "parse.h"
#include "prim.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#define PARSE_ERROR(err)                                        \
    do {                                                        \
        fprintf(stderr, "%s\n", err);                           \
        exit(1);                                                \
    } while(0)

// TODO: ref counting

env_t* g_env = NULL;
obj_t* g_true = NULL;

// TODO: find a better place.
void init()
{
    g_env = env_new((env_t*)NULL);
    g_true = make_symbol("t");
    prim_init();
}

#define ADVANCE() *pp = *pp + 1
#define PEEK(i) *(*pp + i)

obj_t* parse_symbol(const char** pp)
{
    const char* start = *pp;
    
    // ^[()\s\d]
    if (PEEK(0) == '(' || PEEK(0) == ')' || isspace(PEEK(0)) || isdigit(PEEK(0)))
        PARSE_ERROR("Symbol cannot begin with parenthesis, digits or white-space");
    else
        ADVANCE();

    // ^[()\s]*
    while (1) {
        if (PEEK(0) == '(' || PEEK(0) == ')' || isspace(PEEK(0)) || PEEK(0) == '\0')
            break;
        else
            ADVANCE();
    }

    return make_symbol2(start, *pp);
}

obj_t* make_number_obj_from_string(const char* start, const char* end)
{
    // sneaky, lets null terminate the string.
    char orig = *(end + 1);
    char* non_const_end = (char*)end;
    *(non_const_end + 1) = 0;

    // lets lean on libc to do the conversion.
    double num = atof(start);

    // restore original charater.
    *(non_const_end + 1) = orig;
    
    return make_number(num);
}

obj_t* parse_number(const char** pp)
{
    const char* start = *pp;

    // [+-]?
    if (PEEK(0) == '+')
        ADVANCE();
    else if (PEEK(0) == '-')
        ADVANCE();

    // [0-9]+
    do {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    } while(1);

    // \.?
    if (PEEK(0) == '.')
        ADVANCE();
    else
        return make_number_obj_from_string(start, *pp);
    
    // [0-9]*
    while(1) {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    }

    return make_number_obj_from_string(start, *pp);
}

obj_t* parse_atom(const char** pp)
{
    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (((PEEK(0) == '+' || PEEK(0) == '-') && isdigit(PEEK(1))) || isdigit(PEEK(0)))
        return parse_number(pp);
    else
        return parse_symbol(pp);
}

obj_t* parse_expr(const char** pp);

obj_t* parse_list(const char** pp)
{
    // ( EXPR* )
    if (PEEK(0) != '(')
        PARSE_ERROR("List must start with left-parenthesis");
    else
        ADVANCE();

    obj_t* root = CONS(0, 0);
    obj_t* p = root;

    while (1) {
        while (isspace(PEEK(0)))
            ADVANCE();

        if (PEEK(0) == 0)
            PARSE_ERROR("Unexpected NULL character");
        else if (PEEK(0) == ')')
            break;

        obj_t* car = parse_expr(pp);
        if (p->data.pair.car == 0)
            p->data.pair.car = car;
        else {
            p->data.pair.cdr = CONS(car, 0);
            p = p->data.pair.cdr;
        }
    }

    if (PEEK(0) == ')')
        ADVANCE();

    return root;
}

obj_t* parse_quoted_expr(const char** pp)
{
    if (PEEK(0) == '\'')
        ADVANCE();
    else
        PARSE_ERROR("Expected a quote");

    obj_t* e = parse_expr(pp);
    return QUOTE(e);
}

obj_t* parse_expr(const char** pp)
{
    while (isspace(PEEK(0)))
        ADVANCE();

    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (PEEK(0) == '(')
        return parse_list(pp);
    else if (PEEK(0) == '\'')
        return parse_quoted_expr(pp);
    else
        return parse_atom(pp);
}

obj_t* read_string(const char* str)
{
    return parse_expr(&str);
}
