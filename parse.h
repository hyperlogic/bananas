#ifndef PARSE_H
#define PARSE_H

#include "obj.h"

obj_t* read(const char* str);
obj_t* read_file(const char* filename);

#endif
