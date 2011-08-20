#include "lex.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_TOKENS = 1000;
#define MAX_NUMBER_STRING_SIZE = 1000;

// don't copy the string, just point to it.
void token_init(token_t* token, enum token_type type, const char* start, const char* end)
{
    assert(token);
    token->type = type;
    token->start = start;
    token->end = end;
}

token_vec_t* token_vec_new()
{
    token_vec_t* vec = (token_vec_t*)malloc(sizeof(token_vec_t));

    // start off with 16 tokens
    const int initial_max_tokens = 16;
    vec->data = (token_t*)malloc(sizeof(token_t) * initial_max_tokens);
    vec->max_tokens = initial_max_tokens;
    vec->num_tokens = 0;

    return vec;
}

void token_vec_free(token_vec_t* vec)
{
    assert(vec);
    free(vec);
}

void token_vec_push(token_vec_t* vec, enum token_type type, const char* start, const char* end)
{
    if (vec->num_tokens == vec->max_tokens) {
        // realloc more tokens!
        int new_max_tokens = vec->max_tokens * vec->max_tokens;
        vec->data = (token_t*)realloc(vec->data, sizeof(token_t) * new_max_tokens);
        vec->max_tokens = new_max_tokens;
    }
    token_init(vec->data + vec->num_tokens, type, start, end);
    vec->num_tokens++;
}

#define PARSE_ERROR(str)                        \
    fprintf(stderr, "Error: %s\n", str);        \
    return p+1

const char* symbol_rule(const char* p, token_vec_t* vec)
{
    const char* symbol_start = p;

    // SYMBOL := ^[0-9()'.]^[()'.]*
    if (!isdigit(*p) && *p != '(' && *p != ')' && *p != '\'' && *p != '.') {
        p++;
    } else {
        PARSE_ERROR("Symbols illegal symbol start");
    }

    while (*p != 0) {
        if (!isspace(*p) && *p != '(' && *p != ')' && *p != '\'' && *p != '.') {
            p++;
        } else {
            token_vec_push(vec, SYMBOL_TOKEN, symbol_start, p);
            return p;
        }
    }

    token_vec_push(vec, SYMBOL_TOKEN, symbol_start, p);
    return p;
}

const char* number_rule(const char* p, token_vec_t* vec)
{
    const char* num_start = p;

    // NUMBER := [0-9]+\.[0-9]+
    if (isdigit(*p)) {
        p++;
    } else {
        PARSE_ERROR("Expected digit in number!");
    }

    while (*p != 0) {
        if (isdigit(*p)) {
            p++;
        } else {
            break;
        }
    }

    if (*p == '.') {
        p++;
    }
    else {
        token_vec_push(vec, NUMBER_TOKEN, num_start, p);
        return p;
    }

    if (isdigit(*p)) {
        p++;
    } else {
        PARSE_ERROR("Expected digit after radix!");
    }
    while (*p != 0) {
        if (isdigit(*p)) {
            p++;
        }
        else {
            token_vec_push(vec, NUMBER_TOKEN, num_start, p);
            return p;
        }
    }

    token_vec_push(vec, NUMBER_TOKEN, num_start, p);
    return p;
}

const char* token_rule(const char* p, token_vec_t* vec)
{
    // TOKENS := (TOKEN)*
    while (*p != 0)
    {
        // TOKEN := LPAREN | RPREN | QUOTE | NUMBER | WHITESPACE | SYMBOL
        if (*p == '(') {
            // LPAREN := '('
            token_vec_push(vec, LPAREN_TOKEN, p, p+1);
            p++;
        } else if (*p == ')') {
            // RPAREN := '('
            token_vec_push(vec, RPAREN_TOKEN, p, p+1);
            p++;
        } else if (*p == '\'') {
            // QUOTE := '('
            token_vec_push(vec, QUOTE_TOKEN, p, p+1);
            p++;
        } else if (isdigit(*p)) {
            // NUMBER
            p = number_rule(p, vec);
        } else if (isspace(*p)) {
            // skip whitespace
            p++;
        } else {
            // SYMBOL
            p = symbol_rule(p, vec);
        }
    }
    return p;
}

// Don't forget to free the vec.
// NOTE: tokens in the vec point directly into the source string.
token_vec_t* lex(const char* string)
{
    token_vec_t* vec = token_vec_new();
    const char* p = string;
    token_rule(p, vec);
    return vec;
}

