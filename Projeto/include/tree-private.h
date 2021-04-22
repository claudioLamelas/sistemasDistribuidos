#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"
#include "entry.h"

struct tree_t {
	struct entry_t *raiz;
	struct tree_t* esquerda;
	struct tree_t* direita;
};

struct entry_t* searchMin(struct tree_t* tree);

int tree_get_keys_aux(struct tree_t* tree, char** chaves);
#endif
