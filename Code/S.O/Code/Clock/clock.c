#include "clock.h"
#include "../Ferramentas/list.h"
#include "../Nucleo/kernel.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 

// Variável que armazena o tempo virtual da simulação
static long long virtual_clock_time = 0;
// Fila de eventos futuros (lista encadeada)
static List *future_events_queue;

// Sincronização para a fila de eventos futuros
static pthread_mutex_t clock_mutex; // Mutex para proteger acesso à fila
static pthread_cond_t clock_cond;   // Variável de condição para sinalizar novos eventos

// Thread principal que processa o tempo e os eventos futuros
void *clock_thread_runner(void *args)
{
    while (1)
    {
        pthread_mutex_lock(&clock_mutex); // Garante acesso exclusivo à fila

        // Espera até que haja pelo menos um evento na fila
        while (future_events_queue->size == 0)
        {
            pthread_cond_wait(&clock_cond, &clock_mutex);
        }

        // Pega o primeiro evento da fila
        Node *event_node = future_events_queue->head;
        KernelEvent *next_event = (KernelEvent *)event_node->data;

        // Atualiza o tempo virtual para o tempo do próximo evento, se necessário
        if (next_event->execution_time > virtual_clock_time)
        {
            virtual_clock_time = next_event->execution_time;
        }

        // Remove o evento da fila e o despacha para o kernel
        remove_from_list(future_events_queue, next_event, compare_event);
        Kernel__dispatch_event(next_event->type, next_event->data);

        pthread_mutex_unlock(&clock_mutex);

        free(next_event); // Libera o container do evento
    }
    return NULL;
}

// Inicializa o clock virtual e a thread de eventos
void Clock__initialize()
{
    future_events_queue = create_list();    // Cria a fila de eventos
    pthread_mutex_init(&clock_mutex, NULL); // Inicializa o mutex
    pthread_cond_init(&clock_cond, NULL);   // Inicializa a variável de condição

    pthread_t clock_tid;
    pthread_create(&clock_tid, NULL, clock_thread_runner, NULL); // Cria a thread do clock
    pthread_detach(clock_tid);                                   // Desanexa a thread (não precisa de join)
}

// Agenda um novo evento para ocorrer após um certo delay
void Clock__schedule_event(long long delay, EventType type, void *data)
{
    pthread_mutex_lock(&clock_mutex);

    KernelEvent *new_event = malloc(sizeof(KernelEvent));
    if (new_event == NULL)
    {
        pthread_mutex_unlock(&clock_mutex);
        return; // Falha na alocação
    }

    new_event->type = type;
    new_event->data = data;
    new_event->execution_time = virtual_clock_time + delay; // Define o tempo de execução

    add_to_list_sorted(future_events_queue, new_event, compare_event_time); // Insere ordenado

    pthread_cond_signal(&clock_cond); // Sinaliza que há um novo evento
    pthread_mutex_unlock(&clock_mutex);
}

// Retorna o tempo virtual atual
long long Clock__get_time()
{
    return virtual_clock_time;
}
