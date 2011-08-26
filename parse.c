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
        return make_number2(start, *pp);
    
    // [0-9]*
    while(1) {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    }

    return make_number2(start, *pp);
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

    // LPAREN ( RPAREN | EXPR+ (PERIOD EXPR)? RPAREN )

    // LPAREN 
    if (PEEK(0) != '(')
        PARSE_ERROR("List must start with left-parenthesis");
    else
        ADVANCE();

    if (PEEK(0) == ')')
        return cons(0, 0);  // empty list
    else {
        // EXPR+
        obj_t* root = make_nil();
        obj_t* pair = make_nil();
        do {
            obj_t* obj = parse_expr(pp);
            if (is_nil(pair)) {
                root = cons(obj, 0);
                pair = root;
            } else {
                obj_t* temp = pair;
                pair = cons(obj, 0);
                set_cdr(temp, pair);
            }

            while (isspace(PEEK(0)))
                ADVANCE();
            if (PEEK(0) == 0)
                PARSE_ERROR("Unexpected NULL character");

            if (PEEK(0) == '.' || PEEK(0) == ')')
                break;
            
        } while (1);

        // (PERIOD EXPR)?
        if (PEEK(0) == '.') {
            ADVANCE();
            obj_t* obj = parse_expr(pp);
            assert(pair);
            set_cdr(pair, obj);
        }

        while (isspace(PEEK(0)))
            ADVANCE();
        if (PEEK(0) == 0)
            PARSE_ERROR("Unexpected NULL character");

        if (PEEK(0) != ')')
            PARSE_ERROR("Expected ) after dotted expr");
        ADVANCE();

        return root;
    }
}

obj_t* parse_quoted_expr(const char** pp)
{
    if (PEEK(0) == '\'')
        ADVANCE();
    else
        PARSE_ERROR("Expected a quote");

    obj_t* e = parse_expr(pp);
    return quote(e);
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

obj_t* read(const char* str)
{
    return parse_expr(&str);
}
