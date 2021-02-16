#ifndef _ECC_H
#define _ECC_H

#include <stdbool.h>
#include <stdlib.h>

#define CODE_SIZE	14

bool* encode(bool* data);
bool* decode(bool* code);

#endif