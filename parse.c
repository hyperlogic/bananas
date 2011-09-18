#include "parse.h"
#include "prim.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define PARSE_ERROR(err)                                        \
    do {                                                        \
        fprintf(stderr, "%s\n", err);                           \
        exit(1);                                                \
    } while(0)

#define ADVANCE() *pp = *pp + 1
#define PEEK(i) *(*pp + i)

void parse_skip_whitespace(const char** pp)
{
    do {
        while (isspace(PEEK(0)))
            ADVANCE();

        if (PEEK(0) == 0)
            break;

        if (PEEK(0) == ';') {
            do {
                ADVANCE();
            } while(PEEK(0) != '\n' && PEEK(0) != '\0');
        }
    } while(isspace(PEEK(0)));
}

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

    // # values
    if (*start == '#') {
        int len = *pp - start;
        if (len == 2 && start[1] == 't')
            return KTRUE;
        else if (len == 2 && start[1] == 'f')
            return KFALSE;
        else if (len == 6 && strncmp(start + 1, "inert", len - 1) == 0)
            return KINERT;
        else if (len == 7 && strncmp(start + 1, "ignore", len - 1) == 0)
            return KIGNORE;
        else if (len == 11 && strncmp(start + 1, "e-infinity", len - 1) == 0)
            return obj_make_number(-INFINITY);
        else if (len == 11 && strncmp(start + 1, "e+infinity", len - 1) == 0)
            return obj_make_number(INFINITY);
    }

    return obj_make_symbol2(start, *pp);
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
        return obj_make_number2(start, *pp);

    // [0-9]*
    while(1) {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    }

    return obj_make_number2(start, *pp);
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

    parse_skip_whitespace(pp);

    if (PEEK(0) == ')') {
        ADVANCE();
        return KNULL;  // empty list
    }
    else {
        // EXPR+
        obj_stack_frame_push();
        obj_stack_push(KNULL); // 0 : root
        obj_stack_push(KNULL); // 1 : pair
        obj_stack_push(KNULL); // 2 : expr
        obj_stack_push(KNULL); // 3 : temp
        do {
            obj_stack_set(2, parse_expr(pp));
            if (obj_is_null(obj_stack_get(1))) {
                obj_stack_set(0, obj_cons(obj_stack_get(2), KNULL));
                obj_stack_set(1, obj_stack_get(0));
            } else {
                obj_stack_set(3, obj_stack_get(1));
                obj_stack_set(1, obj_cons(obj_stack_get(2), KNULL));
                obj_set_cdr(obj_stack_get(3), obj_stack_get(1));
            }
            parse_skip_whitespace(pp);
            if (PEEK(0) == '.' || PEEK(0) == ')')
                break;
        } while (1);

        // (PERIOD EXPR)?
        if (PEEK(0) == '.') {
            ADVANCE();
            obj_stack_set(2, parse_expr(pp));
            obj_set_cdr(obj_stack_get(1), obj_stack_get(2));
        }

        parse_skip_whitespace(pp);

        if (PEEK(0) != ')')
            PARSE_ERROR("Expected ) after dotted expr");
        ADVANCE();

        obj_t* result = obj_stack_get(0);
        obj_stack_frame_pop();
        return result;
    }
}

obj_t* parse_quoted_expr(const char** pp)
{
    if (PEEK(0) == '\'')
        ADVANCE();
    else
        PARSE_ERROR("Expected a quote");

    obj_stack_frame_push();
    obj_stack_push(parse_expr(pp));
    obj_stack_push(obj_make_symbol("$quote"));
    obj_stack_push(obj_cons(obj_stack_get(0), KNULL));
    obj_stack_push(obj_cons(obj_stack_get(1), obj_stack_get(2)));
    obj_t* result = obj_stack_get(3);
    obj_stack_frame_pop();
    return result;
}

obj_t* parse_expr(const char** pp)
{
    parse_skip_whitespace(pp);

    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (PEEK(0) == '(')
        return parse_list(pp);
    else if (PEEK(0) == '\'')
        return parse_quoted_expr(pp);
    else
        return parse_atom(pp);
}

obj_t* parse_expr_sequence(const char** pp)
{
    // EXPR*
    obj_stack_frame_push();
    obj_stack_push(obj_make_symbol("$sequence"));   // 0
    obj_stack_push(obj_cons(obj_stack_get(0), KNULL));  // 1
    obj_stack_push(KNULL); // 2
    obj_t* pair = obj_stack_get(1);
    while (1) {
        parse_skip_whitespace(pp);
        if (PEEK(0) == 0)
            break;
        obj_stack_set(2, parse_expr(pp));
        obj_set_cdr(pair, obj_cons(obj_stack_get(2), KNULL));
        pair = obj_cdr(pair);
    }
    obj_t* result = obj_stack_get(1);
    obj_stack_frame_pop();
    return result;
}

obj_t* read(const char* str)
{
    return parse_expr(&str);
}

obj_t* read_file(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp)
        return KNULL;

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* str = (char*)malloc(file_size + 1);
	int bytes_read = (int)fread(str, sizeof(char), file_size, fp);
	assert(file_size == bytes_read);
	fclose(fp);
	str[file_size] = 0;  // make sure it's null terminated.
    const char* p = str;
    obj_t* result = parse_expr_sequence(&p);
    free(str);
    return result;
}
