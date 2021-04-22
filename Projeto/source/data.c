#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

/*
*	Grupo 27
*	Cláudio Lamelas, nº 52747
*	Raquel Chin, nº 52792
*	Pedro Bento, nº 52823
*/

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size){

	struct data_t* dados;
	if(size < 1){
		return NULL;
	}
	else{
		dados = malloc(sizeof(struct data_t));
		dados->datasize = size;
		dados->data = malloc(size);
		return dados;
	}
}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void *data){

	struct data_t* dados;
	if(size < 1){
		return NULL;
	}
	if(data == NULL){
		return NULL;
	}
	
	dados = malloc(sizeof(struct data_t));
	dados->datasize = size;
	dados->data = data;
	return dados;
	} 

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data){
	if(data != NULL){
		if(data->data != NULL){
			free(data->data);
			data->data = NULL;
		}
		free(data);
		data = NULL;
	}
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 */
struct data_t *data_dup(struct data_t *data){

	if(data == NULL || data->data == NULL)
		return NULL;
		
	struct data_t* novoDado;
	novoDado = data_create(data->datasize);
	
	if(novoDado == NULL)
		return NULL;
		
	memcpy(novoDado->data, data->data, data->datasize);
	return novoDado;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data){
	free(data->data);
	data->datasize = new_size;
	data->data = new_data;
}