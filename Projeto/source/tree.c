#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "entry.h"
#include "tree.h"
#include "tree-private.h"

struct tree_t;

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create(){
	struct tree_t *arvore = malloc(sizeof(struct tree_t));
	arvore->raiz = NULL;
	arvore->esquerda = NULL;
	arvore->direita = NULL;
	return arvore;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree){

	if(tree != NULL){

		if(tree->raiz != NULL){
			entry_destroy(tree->raiz);
		}

		if(tree->esquerda != NULL){
			tree_destroy(tree->esquerda);
		}

		if(tree->direita != NULL){
			tree_destroy(tree->direita);
		}
		free(tree);
		/*
		tree->esquerda = NULL;
		tree->direita = NULL;
		tree->raiz = NULL;
		tree = NULL;
		*/
	}
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value){

	struct data_t* dados;
	char* chave;

	if(key == NULL || value == NULL){
		return -1;
	}

	if (tree->raiz == NULL){
		//chave = malloc(sizeof(key));
		//strcpy(chave, key);
		chave = strdup(key);
		dados = data_dup(value);
		tree->raiz = entry_create(chave, dados);
		tree->esquerda = tree_create();
		tree->direita = tree_create();
		return 0;
	}
	if(strcmp(tree->raiz->key, key) == 0){
		//chave = malloc(sizeof(key));
		//strcpy(chave, key);
		chave = strdup(key);
		dados = data_dup(value);
		entry_replace(tree->raiz, key, dados);
		return 0;
	}
	if(strcmp(tree->raiz->key, key) < 0){
		tree_put(tree->direita, key, value);
	}
	if(strcmp(tree->raiz->key, key) > 0){
		tree_put(tree->esquerda, key, value);
	}
	return -1;
	}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função.
 * Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key){
	struct data_t* dados;

	if(key == NULL){
		return NULL;
	}
	if(tree->raiz == NULL){
		return NULL;
	}
	if(strcmp(tree->raiz->key, key) == 0){
		dados = data_dup(tree->raiz->value);
		return dados;
	}
	if(strcmp(tree->raiz->key, key) > 0){
		tree_get(tree->esquerda, key);
	}
	else{
		tree_get(tree->direita, key);
	}
	return NULL;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key){

	struct entry_t* min;

	if(key == NULL){
		return -1;
	}

	if(tree->raiz == NULL || tree_size(tree) > 1){
		return -1;
	}

	if(strcmp(tree->raiz->key, key) == 0){

		if(tree->esquerda->raiz == NULL && tree->direita->raiz == NULL){
			tree->raiz = NULL;
			//tree_destroy(tree);
			return 0;
		}
		else if (tree->esquerda->raiz == NULL)
        {
            memcpy(tree, tree->direita, sizeof(struct tree_t));
			tree_del(tree->direita, tree->direita->raiz->key);
			return 0;
        }
        else if (tree->direita->raiz == NULL)
        {
            memcpy(tree, tree->esquerda, sizeof(struct tree_t));
			tree_del(tree->esquerda, tree->esquerda->raiz->key);
			return 0;
        }
		else{
			min = searchMin(tree->direita);
			memcpy(tree->raiz, min, sizeof(struct entry_t));
			return tree_del(tree->direita, min->key);
		}
	}

	if(strcmp(tree->raiz->key, key) < 0){
			return tree_del(tree->direita, key);
		}
		if(strcmp(tree->raiz->key, key) > 0){
			return tree_del(tree->esquerda, key);
		}
		return -1;
}

struct entry_t* searchMin(struct tree_t* tree){
	if (tree->esquerda->raiz == NULL){
		return tree->raiz;
	}
	else{
		searchMin(tree->esquerda);
	}
	return NULL;
}

void destroyMin(struct tree_t* tree){
	if (tree->esquerda == NULL && tree->direita == NULL){
		tree_destroy(tree);
	}
	else if(tree-> esquerda == NULL){
		tree = tree->direita;
	}
	else{
		searchMin(tree->esquerda);
	}
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree){

	if(tree == NULL || tree->raiz == NULL){
		return 0;
	}
	else{
		return 1 + tree_size(tree->esquerda) + tree_size(tree->direita);
	}

}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree){
	int alturaEsquerda, alturaDireita;

	if(tree->raiz == NULL){
		return 0;
	}
	else{
		alturaEsquerda = tree_height(tree->esquerda);
		alturaDireita = tree_height(tree->direita);

		if(alturaEsquerda > alturaDireita)
			return alturaEsquerda + 1;
		else
			return alturaDireita + 1;
	}
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **tree_get_keys(struct tree_t *tree){

	char **chaves;
	int i = 0;
	chaves = malloc(sizeof(char*)*tree_size(tree));
	//chaves[i] = malloc(sizeof(char));
	chaves[i] = NULL;

	/*
	A estrutura tree não está a ser devidamente passada para a funcao auxiliar e
	como tal foi necessario comentar a linha:
	//i = tree_get_keys_aux(tree, chaves);
	*/

	return chaves;
}

int tree_get_keys_aux(struct tree_t *tree, char** chaves){
	int i;

	if(tree->raiz == NULL){
		return 0;
	}
	else{
		for(i = 0; chaves[i] != NULL; i++){
		}

		//chave = malloc(sizeof(tree->raiz->key));
		//strcpy(chave, tree->raiz->key);
		//chave = strdup(tree->raiz->key);
		//chaves[i] = malloc(sizeof(tree->raiz->key));
		//strcpy(chaves[i], chave);
		chaves[i] = strdup(tree->raiz->key);
		chaves[i+1] = NULL;
		if(tree->esquerda->raiz != NULL && tree->direita->raiz != NULL){
			return 1 + tree_get_keys_aux(tree->esquerda, chaves) +
			tree_get_keys_aux(tree->direita, chaves);
		}else if(tree->esquerda->raiz != NULL){
			return 1 + tree_get_keys_aux(tree->esquerda, chaves);
		}else{
			return 1 + tree_get_keys_aux(tree->direita, chaves);
		}
	}
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys){
	int i;
	for(i = 0; keys[i] != NULL; i++){
		free(keys[i]);
	}
	free(keys);
}
