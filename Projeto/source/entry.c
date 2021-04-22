#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "entry.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados passados.
 */
struct entry_t *entry_create(char *key, struct data_t *data){

	if(key == NULL){
		return NULL;
	}

	struct entry_t* entrada;
	entrada = malloc(sizeof(struct entry_t));
	entrada->key = key;
	entrada->value = data;
	return entrada;
}

/* Função que inicializa os elementos de uma entry com o
 * valor NULL.
 */
void entry_initialize(struct entry_t *entry){

	entry->key = NULL;
	entry->value = NULL;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry){
	if(entry != NULL){
		if(entry->value != NULL){
			data_destroy(entry->value);
		}
		free(entry->key);
		free(entry);
		/*
		entry->key = NULL;
		entry->value = NULL;
		entry = NULL;
		*/
	}
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry){

	struct entry_t* novaEntrada;
	novaEntrada = malloc(sizeof(struct entry_t));

	//novaEntrada->key = malloc(strlen(entry->key) + 1);
	novaEntrada->key = strdup(entry->key);

	novaEntrada->value = data_dup(entry->value);

	if(novaEntrada == NULL)
		return NULL;

	return novaEntrada;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
	data_destroy(entry->value);
	free(entry->key);
	entry->key = new_key;
	entry->value = new_value;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
	int valor;
	valor = strcmp(entry1->key,entry2->key);
	if(valor > 0){
		return 1;
	}
	else if(valor < 0){
		return -1;
	}
	else{
		return 0;
	}
}
