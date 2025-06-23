#include "events.h"
#include <stdlib.h> // Necessário para definição de NULL

// -----------------------------------------------------------------------------
// Função de comparação de ponteiros de eventos.
// Retorna 0 se os ponteiros forem iguais (mesmo evento), 1 caso contrário.
// Retorna -1 se algum dos ponteiros for NULL.
// -----------------------------------------------------------------------------
int compare_event(void *a, void *b)
{
    if (a == NULL || b == NULL)
        return -1;
    return (a == b) ? 0 : 1;
}

// -----------------------------------------------------------------------------
// Função de comparação de eventos pelo tempo de execução.
// Retorna -1 se a < b, 1 se a > b, 0 se iguais.
// Usada para ordenação de eventos futuros na fila do clock.
// -----------------------------------------------------------------------------
int compare_event_time(void *a, void *b)
{
    KernelEvent *event_a = (KernelEvent *)a;
    KernelEvent *event_b = (KernelEvent *)b;
    if (event_a->execution_time < event_b->execution_time)
    {
        return -1;
    }
    if (event_a->execution_time > event_b->execution_time)
    {
        return 1;
    }
    return 0;
}