#include "scheduler.h"
#include "../CPU/cpu.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>                 // Para strdup
#include "../Ferramentas/compare.h" // Para compare_strings_func

#define MAX_LOG_MESSAGES 15 // Define o tamanho máximo do log da UI

// Cria e inicializa uma instância do escalonador
Scheduler *Scheduler__create()
{
    Scheduler *new_scheduler_instance = malloc(sizeof(Scheduler));
    if (new_scheduler_instance == NULL)
    {
        return NULL;
    }
    new_scheduler_instance->ready_queue = create_list(); // Fila de processos prontos
    pthread_mutex_init(&new_scheduler_instance->ready_queue_mutex, NULL); // Inicializa o mutex
    return new_scheduler_instance;
}

// Atualiza estatísticas de I/O do processo (read/write)
void Scheduler__update_process_io_stats(Bcp *target_process_bcp, int was_read_operation)
{
    if (target_process_bcp == NULL)
        return;

    // Conforme especificação, o critério é a soma de "read" e "write"
    // Ambos os contadores são incrementados.
    if (was_read_operation)
    {
        target_process_bcp->total_read_ops++;
    }
    else
    {
        target_process_bcp->total_write_ops++;
    }
}

// Seleciona o próximo processo a ser executado, baseado no menor número de operações de I/O
Bcp *Scheduler__get_next_process_to_run()
{
    // A trava já deve ter sido adquirida por quem chamou esta função (Scheduler__perform_context_switch)
    if (kernel_instance == NULL || kernel_instance->scheduler->ready_queue->size == 0)
    {
        return NULL; // Fila de prontos vazia
    }

    Node *current_node = kernel_instance->scheduler->ready_queue->head;
    Bcp *chosen_process = NULL;
    int lowest_io_count = INT_MAX;

    // Percorre a fila de prontos para encontrar o processo com menos I/O
    while (current_node != NULL)
    {
        Bcp *p = (Bcp *)current_node->data;
        int current_io = p->total_read_ops + p->total_write_ops;

        if (current_io < lowest_io_count)
        {
            lowest_io_count = current_io;
            chosen_process = p;
        }
        else if (current_io == lowest_io_count)
        {
            // Critério de desempate: menor PID
            if (chosen_process == NULL || (p->pid < chosen_process->pid))
            {
                chosen_process = p;
            }
        }
        current_node = current_node->next;
    }

    return chosen_process;
}

// Realiza o escalonamento e troca de contexto entre processos
void Scheduler__perform_context_switch()
{
    if (kernel_instance == NULL)
        return;

    // Trava o mutex para garantir acesso atômico à fila de prontos
    pthread_mutex_lock(&kernel_instance->scheduler->ready_queue_mutex);

    Bcp *previously_running_bcp = kernel_instance->running_process;
    Bcp *next_bcp_to_run = Scheduler__get_next_process_to_run();

    char log_buffer[256];

    // Se o processo que estava rodando não foi bloqueado ou terminado, ele volta para a fila de prontos.
    if (previously_running_bcp != NULL && previously_running_bcp->current_execution_state == PROCESS_STATE_RUNNING)
    {
        previously_running_bcp->current_execution_state = PROCESS_STATE_READY;
        add_to_list(kernel_instance->scheduler->ready_queue, previously_running_bcp);
    }

    // Prepara a mensagem de log
    if (next_bcp_to_run != NULL)
    {
        int io_count = next_bcp_to_run->total_read_ops + next_bcp_to_run->total_write_ops;
        sprintf(log_buffer, "SCHED: PID %d eleito (I/O: %d).", next_bcp_to_run->pid, io_count);
    }
    else
    {
        sprintf(log_buffer, "SCHED: Fila de prontos vazia. CPU ociosa.");
    }

    // Adiciona a mensagem ao log de forma segura (o log tem seu próprio mutex)
    pthread_mutex_lock(&kernel_instance->scheduler_log_mutex);
    // Mantém o log com um tamanho gerenciável
    while (kernel_instance->scheduler_log->size >= MAX_LOG_MESSAGES)
    {
        Node *old_node = kernel_instance->scheduler_log->head;
        void *data_to_free = old_node->data;
        remove_from_list(kernel_instance->scheduler_log, data_to_free, compare_strings_func);
        free(data_to_free); // Libera a string antiga
    }
    add_to_list(kernel_instance->scheduler_log, strdup(log_buffer));
    pthread_mutex_unlock(&kernel_instance->scheduler_log_mutex);

    // Se um novo processo foi escolhido, remove-o da fila de prontos e o define como 'running'.
    if (next_bcp_to_run != NULL)
    {
        remove_from_list(kernel_instance->scheduler->ready_queue, next_bcp_to_run, compare_pid);
        next_bcp_to_run->current_execution_state = PROCESS_STATE_RUNNING;
    }

    // Atualiza o processo em execução no kernel e instrui a CPU a executá-lo.
    kernel_instance->running_process = next_bcp_to_run;

    // Destrava o mutex após todas as operações na fila de prontos
    pthread_mutex_unlock(&kernel_instance->scheduler->ready_queue_mutex);

    CPU__run_process(next_bcp_to_run);
}