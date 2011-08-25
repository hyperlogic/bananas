#include "symbol.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// global interned symbol array.
#define MAX_SYMBOLS 1000
static const char* g_symbol_array[MAX_SYMBOLS];
static int g_symbol_len_array[MAX_SYMBOLS];
static int g_num_symbols = 0;

const char* symbol_get(int id)
{
    assert(id >= 0 && id < MAX_SYMBOLS);
    return g_symbol_array[id];
}

int symbol_find(const char* str, int len)
{
    int i;
    for (i = 0; i < g_num_symbols; i++) {
        if (len == g_symbol_len_array[i] && strncmp(g_symbol_array[i], str, len) == 0)
            return i;
    }
    return -1;
}

int symbol_add(const char* str, int len)
{
    assert(g_num_symbols < MAX_SYMBOLS);
    char* symbol_string = (char*)malloc(len + 1);
    memcpy(symbol_string, str, len);
    symbol_string[len] = 0;
    g_symbol_array[g_num_symbols] = symbol_string;
    g_symbol_len_array[g_num_symbols] = len;
    g_num_symbols++;
    return g_num_symbols - 1;
}
