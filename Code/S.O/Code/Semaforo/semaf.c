#include "semaf.h"
#include "../Nucleo/kernel.h"
#include "../escalonador/scheduler.h"
#include <stdlib.h>

// -----------------------------------------------------------------------------
// Inicializa um novo semáforo com valor inicial e fila de espera vazia
// -----------------------------------------------------------------------------
Semaphore *initialize_semaphore(char id_char, const int initial_count)
{
    Semaphore *new_semaphore = malloc(sizeof(Semaphore));
    if (!new_semaphore)
        return NULL;

    new_semaphore->name_char = id_char;
    new_semaphore->current_value = initial_count;
    new_semaphore->waiting_processes_q = create_list();
    if (!new_semaphore->waiting_processes_q)
    {
        free(new_semaphore);
        return NULL;
    }

    pthread_mutex_init(&new_semaphore->mutex, NULL);
    return new_semaphore;
}

// -----------------------------------------------------------------------------
// Operação P (wait) no semáforo: decrementa valor e bloqueia processo se necessário
// Retorna 1 se o processo foi bloqueado, 0 caso contrário
// -----------------------------------------------------------------------------
int semaphore_P_operation(Semaphore *sem_instance, Bcp *requesting_process_bcp)
{
    if (!sem_instance || !requesting_process_bcp)
        return -1;

    pthread_mutex_lock(&sem_instance->mutex);
    sem_instance->current_value--;

    int was_blocked_flag = 0;
    if (sem_instance->current_value < 0)
    {
        requesting_process_bcp->current_execution_state = PROCESS_STATE_WAITING;
        add_to_list(sem_instance->waiting_processes_q, requesting_process_bcp);
        was_blocked_flag = 1;
    }

    pthread_mutex_unlock(&sem_instance->mutex);
    return was_blocked_flag;
}

// -----------------------------------------------------------------------------
// Operação V (signal) no semáforo: incrementa valor e acorda processo se necessário
// -----------------------------------------------------------------------------
void semaphore_V_operation(Semaphore *sem_instance)
{
    if (!sem_instance)
        return;

    pthread_mutex_lock(&sem_instance->mutex);
    sem_instance->current_value++;

    // Se houver processos esperando, acorda o primeiro da fila
    if (sem_instance->current_value <= 0 && sem_instance->waiting_processes_q && sem_instance->waiting_processes_q->size > 0)
    {
        Node *first_waiting_node = sem_instance->waiting_processes_q->head;
        if (first_waiting_node)
        {
            Bcp *process_to_wake = (Bcp *)first_waiting_node->data;

            remove_from_list(sem_instance->waiting_processes_q, process_to_wake, compare_pid);

            process_to_wake->current_execution_state = PROCESS_STATE_READY;

            if (kernel_instance && kernel_instance->scheduler && kernel_instance->scheduler->ready_queue)
            {
                add_to_list(kernel_instance->scheduler->ready_queue, process_to_wake);
            }
        }
    }
    pthread_mutex_unlock(&sem_instance->mutex);
}

// -----------------------------------------------------------------------------
// Funções getter para valor do semáforo e quantidade de processos esperando
// -----------------------------------------------------------------------------
int Semaphore_get_value(Semaphore *sem)
{
    if (!sem)
        return 0;
    pthread_mutex_lock(&sem->mutex);
    int value = sem->current_value;
    pthread_mutex_unlock(&sem->mutex);
    return value;
}

int Semaphore_get_waiting_count(Semaphore *sem)
{
    if (!sem || !sem->waiting_processes_q)
        return 0;
    pthread_mutex_lock(&sem->mutex);
    int count = sem->waiting_processes_q->size;
    pthread_mutex_unlock(&sem->mutex);
    return count;
}