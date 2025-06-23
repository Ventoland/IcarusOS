#ifndef LIST_H
#define LIST_H

// Estrutura do nó da lista encadeada.
// Cada nó armazena um ponteiro genérico para dados e um ponteiro para o próximo nó.
typedef struct Node_t
{
    void *data;          // Ponteiro para o conteúdo armazenado no nó
    struct Node_t *next; // Ponteiro para o próximo nó da lista
} Node;

// Estrutura da lista encadeada.
// Mantém ponteiros para o início (head), fim (tail) e o tamanho da lista.
typedef struct List_t
{
    Node *head; // Ponteiro para o primeiro nó da lista
    Node *tail; // Ponteiro para o último nó da lista
    int size;   // Quantidade de elementos na lista
} List;

/* ---------- Operações Básicas da Lista ---------- */

// Cria e retorna uma nova lista vazia.
// Retorno: ponteiro para a lista criada.
List *create_list();

// Cria e retorna um novo nó com o conteúdo especificado.
// Parâmetro: ponteiro para os dados a serem armazenados no nó.
Node *create_node(void *data);

// Libera toda a memória utilizada pela lista e seus nós.
// Não libera o conteúdo apontado por data.
void destroy_list(List *list);

/* ---------- Operações de Inserção ---------- */

// Adiciona um elemento ao final da lista (sem ordenação).
// Parâmetros: ponteiro para a lista e ponteiro para os dados.
void add_to_list(List *list, void *data);

// Adiciona um elemento na lista de forma ordenada, conforme função de comparação.
// Parâmetros: ponteiro para a lista, ponteiro para os dados e função de comparação.
void add_to_list_sorted(List *list, void *data,
                        int (*compare)(void *, void *));

/* ---------- Operações de Remoção e Busca ---------- */

// Remove o primeiro elemento da lista que for igual ao fornecido (compare == 0).
// Não libera o conteúdo apontado por data.
// Parâmetros: ponteiro para a lista, ponteiro para os dados e função de comparação.
void remove_from_list(List *list, void *data,
                      int (*compare)(void *, void *));

// Verifica se um elemento existe na lista (retorna 1 se existir, 0 caso contrário).
// Parâmetros: ponteiro para a lista, ponteiro para a chave e função de comparação.
int exists_in_list(List *list, void *key,
                   int (*compare)(void *, void *));

#endif // LIST_H