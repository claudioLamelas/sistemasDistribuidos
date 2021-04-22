#ifndef _SERIALIZATION_PRIVATE_H
#define _SERIALIZATION_PRIVATE_H

#include "data.h"
#include "entry.h"
#include "tree.h"

int tree_to_buffer_aux(struct tree_t* tree, char** tree_buf, int i);

#endif
