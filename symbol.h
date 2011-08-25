#ifndef SYMBOL_H
#define SYMBOL_H

// interned symbols
int symbol_add(const char* str, int len);
const char* symbol_get(int id);
int symbol_find(const char* str, int len);

#endif
