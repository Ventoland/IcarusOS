#include "list.h"
#include <stdlib.h>

// Cria e inicializa uma nova lista encadeada vazia.
// Retorna ponteiro para a lista criada.
List *create_list()
{
    List *new_list = malloc(sizeof(List));
    new_list->head = NULL; // Ponteiro para o primeiro nó
    new_list->tail = NULL; // Ponteiro para o último nó
    new_list->size = 0;    // Número de elementos na lista
    return new_list;
}

// Cria um novo nó para a lista, armazenando o ponteiro para os dados.
// Retorna ponteiro para o nó criado.
Node *create_node(void *data_content)
{
    Node *new_node_ptr = malloc(sizeof(Node));
    new_node_ptr->data = data_content; // Dados armazenados no nó
    new_node_ptr->next = NULL;         // Próximo nó (inicialmente NULL)
    return new_node_ptr;
}

// Verifica se um elemento existe na lista, usando função de comparação.
// Retorna 1 se encontrado, 0 caso contrário.
int exists_in_list(List *list_ptr, void *key_value, int (*compare_func)(void *, void *))
{
    Node *current_node = list_ptr->head;
    while (current_node != NULL)
    {
        if (compare_func(current_node->data, key_value))
        { // Se igual, retorna 1
            return 1;
        }
        current_node = current_node->next;
    }
    return 0;
}

// Adiciona um elemento à lista de forma ordenada, conforme função de comparação.
// Elementos menores (compare <= 0) vão para o início.
void add_to_list_sorted(List *list_ptr, void *data_content, int (*compare_func)(void *, void *))
{
    Node *new_node_ptr = create_node(data_content);
    if (list_ptr->head == NULL)
    {
        list_ptr->head = list_ptr->tail = new_node_ptr;
    }
    else if (compare_func(data_content, list_ptr->head->data) <= 0)
    {
        new_node_ptr->next = list_ptr->head;
        list_ptr->head = new_node_ptr;
    }
    else
    {
        Node *current_node = list_ptr->head->next;
        Node *previous_node = list_ptr->head;
        while (current_node != NULL && compare_func(data_content, current_node->data) > 0)
        {
            previous_node = current_node;
            current_node = current_node->next;
        }
        if (current_node == NULL)
        {
            previous_node->next = new_node_ptr;
            list_ptr->tail = new_node_ptr;
        }
        else
        {
            new_node_ptr->next = previous_node->next;
            previous_node->next = new_node_ptr;
        }
    }
    list_ptr->size++;
}

// Adiciona um elemento ao final da lista (sem ordenação).
void add_to_list(List *list_ptr, void *data_content)
{
    Node *new_node_ptr = create_node(data_content);
    if (new_node_ptr == NULL)
    {
        return;
    }
    if (list_ptr->head == NULL)
    {
        list_ptr->head = new_node_ptr;
    }
    if (list_ptr->tail != NULL)
    {
        list_ptr->tail->next = new_node_ptr;
    }
    list_ptr->tail = new_node_ptr;
    list_ptr->size++;
}

// Remove o primeiro elemento da lista que for igual (compare == 0) ao fornecido.
// Não libera o conteúdo apontado por data_content, apenas o nó.
void remove_from_list(List *list_ptr, void *data_content, int (*compare_func)(void *, void *))
{
    Node *current_node = list_ptr->head;
    Node *previous_node = NULL;
    while (current_node != NULL && compare_func(data_content, current_node->data) != 0)
    {
        previous_node = current_node;
        current_node = current_node->next;
    }
    if (current_node == NULL)
    {
        return;
    }
    else if (current_node == list_ptr->head)
    {
        list_ptr->head = current_node->next;
        if (list_ptr->head == NULL)
        { // Lista ficou vazia
            list_ptr->tail = NULL;
        }
    }
    else if (current_node == list_ptr->tail)
    {
        list_ptr->tail = previous_node;
        if (previous_node != NULL)
        {
            previous_node->next = NULL;
        }
    }
    else
    {
        previous_node->next = current_node->next;
    }
    // Atenção: não libera o conteúdo apontado por data_content!
    free(current_node);
    list_ptr->size--;
}

// Libera toda a memória da lista e seus nós (mas não dos dados armazenados).
void destroy_list(List *list_ptr)
{
    Node *current_node;
    while (list_ptr->head != NULL)
    {
        current_node = list_ptr->head;
        list_ptr->head = current_node->next;
        // Atenção: não libera o conteúdo apontado por data!
        free(current_node);
    }
    free(list_ptr);
}