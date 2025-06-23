#include "kernel.h"
#include "../IO/io_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Clock/clock.h"
#include "../CPU/cpu.h"
#include "../Memoria/Page.h"
#include "common_structs.h"
#include "../escalonador/scheduler.h"
#include "../Ferramentas/compare.h"

// -----------------------------------------------------------------------------
// Definição das variáveis globais do núcleo do sistema operacional
// -----------------------------------------------------------------------------
Kernel *kernel_instance;           // Instância principal do kernel
List *event_queue;                 // Fila de eventos pendentes
pthread_mutex_t event_queue_mutex; // Mutex para proteger a fila de eventos
pthread_cond_t event_queue_cond;   // Condição para sinalizar novos eventos
pthread_mutex_t pcb_list_mutex;    // Mutex para proteger a lista de PCBs

// -----------------------------------------------------------------------------
// Inicialização do kernel e de todos os módulos do sistema operacional
// -----------------------------------------------------------------------------
void Kernel__initialize()
{
    kernel_instance = malloc(sizeof(Kernel));
    if (kernel_instance == NULL)
    {
        exit(1);
    }
    kernel_instance->proc_id_counter = 1;
    kernel_instance->pcb_list = create_list();
    kernel_instance->semaphore_table = create_list();
    kernel_instance->scheduler = Scheduler__create();
    kernel_instance->running_process = NULL;
    kernel_instance->scheduler_log = create_list();
    pthread_mutex_init(&kernel_instance->scheduler_log_mutex, NULL);
    pthread_mutex_init(&kernel_instance->semaphore_table_mutex, NULL);
    event_queue = create_list();
    pthread_mutex_init(&pcb_list_mutex, NULL);
    pthread_mutex_init(&event_queue_mutex, NULL);
    pthread_cond_init(&event_queue_cond, NULL);
    initialize_memory_management();
    IOManager__initialize();
    CPU__initialize();
    Clock__initialize();
}

// -----------------------------------------------------------------------------
// Função para despachar (agendar) um novo evento para o kernel
// -----------------------------------------------------------------------------
void Kernel__dispatch_event(EventType type, void *data)
{
    KernelEvent *event = malloc(sizeof(KernelEvent));
    event->type = type;
    event->data = data;
    pthread_mutex_lock(&event_queue_mutex);
    add_to_list(event_queue, event);
    pthread_cond_signal(&event_queue_cond);
    pthread_mutex_unlock(&event_queue_mutex);
}

// -----------------------------------------------------------------------------
// Loop principal de simulação do kernel: processa eventos da fila
// Cada evento é tratado por uma thread específica, conforme seu tipo
// -----------------------------------------------------------------------------
void Kernel__run_simulation()
{
    while (1)
    {
        pthread_mutex_lock(&event_queue_mutex);
        while (event_queue->size == 0)
        {
            pthread_cond_wait(&event_queue_cond, &event_queue_mutex);
        }

        Node *event_node = event_queue->head;
        KernelEvent *current_event = (KernelEvent *)event_node->data;
        remove_from_list(event_queue, current_event, compare_event);
        pthread_mutex_unlock(&event_queue_mutex);

        void *(*thread_handler)(void *) = NULL;

        // Seleciona o handler apropriado para o tipo de evento
        switch (current_event->type)
        {
        case EVT_PROCESS_CREATE:
            thread_handler = thread_handler_process_create;
            break;
        case EVT_PROCESS_FINISH:
            thread_handler = thread_handler_process_finish;
            break;
        case EVT_CPU_TIMER_FINISH:
            thread_handler = thread_handler_cpu_timer_finish;
            break;
        case EVT_DISK_REQUEST:
            thread_handler = thread_handler_disk_request;
            break;
        case EVT_DISK_FINISH:
            thread_handler = thread_handler_disk_finish;
            break;
        case EVT_PRINT_REQUEST:
            thread_handler = thread_handler_print_request;
            break;
        case EVT_PRINT_FINISH:
            thread_handler = thread_handler_print_finish;
            break;
        case EVT_SEMAPHORE_P:
            thread_handler = thread_handler_semaphore_p;
            break;
        case EVT_SEMAPHORE_V:
            thread_handler = thread_handler_semaphore_v;
            break;
        case EVT_MEM_LOAD_REQ:
            thread_handler = thread_handler_mem_load_req;
            break;
        case EVT_MEM_LOAD_FINISH:
            thread_handler = thread_handler_mem_load_finish;
            break;
        default:
            break;
        }

        // Cria uma thread para tratar o evento, se houver handler definido
        if (thread_handler)
        {
            pthread_t tid;
            pthread_create(&tid, NULL, thread_handler, current_event->data);
            pthread_detach(tid);
        }
        free(current_event);
    }
}

/* --- Implementação dos Handlers de Thread para cada tipo de evento --- */

// Handler para criação de processo
void *thread_handler_process_create(void *args)
{
    char *file_path = (char *)args;
    Bcp *new_pcb = Process__create(file_path);
    if (new_pcb)
    {
        pthread_mutex_lock(&pcb_list_mutex);
        add_to_list(kernel_instance->pcb_list, new_pcb);
        pthread_mutex_unlock(&pcb_list_mutex);
    }
    free(file_path);
    return NULL;
}

// Handler para finalização de processo (VERSÃO FINAL E ROBUSTA)
void *thread_handler_process_finish(void *args)
{
    Bcp *pcb_to_terminate = (Bcp *)args;
    int must_call_scheduler = 0;

    // Inicia uma seção crítica para alterar o estado do processo e a lista global
    pthread_mutex_lock(&pcb_list_mutex);

    // Verifica se o processo que está terminando era o que estava em execução na CPU.
    if (kernel_instance->running_process == pcb_to_terminate)
    {
        kernel_instance->running_process = NULL;
        must_call_scheduler = 1; // Marca que o escalonador DEVE ser chamado
    }

    // Remove o processo da lista global de BCPs.
    // A interface agora não o verá mais.
    remove_from_list(kernel_instance->pcb_list, pcb_to_terminate, compare_pid);

    // Encerra a seção crítica
    pthread_mutex_unlock(&pcb_list_mutex);

    // Se a CPU ficou ociosa por causa desta finalização, chama o escalonador.
    // Esta chamada agora ocorre fora da seção crítica para evitar deadlocks.
    if (must_call_scheduler)
    {
        Scheduler__perform_context_switch();
    }

    // Por fim, libera toda a memória associada ao processo que foi removido.
    release_process_frames(pcb_to_terminate->pid);
    free(pcb_to_terminate->name_str);

    Node *instr_node = pcb_to_terminate->instructions_list_ptr->head;
    while (instr_node != NULL)
    {
        free(instr_node->data);
        instr_node = instr_node->next;
    }
    destroy_list(pcb_to_terminate->instructions_list_ptr);
    free(pcb_to_terminate); // Libera o BCP

    return NULL;
}


// Handler para iniciar a carga na memória (simula tempo de carregamento)
void *thread_handler_mem_load_req(void *args)
{
    Bcp *pcb = (Bcp *)args;
    // Emula tempo de carregamento de processo na memória
    Clock__schedule_event(150, EVT_MEM_LOAD_FINISH, pcb);
    return NULL;
}

// Handler para finalizar a carga na memória
void *thread_handler_mem_load_finish(void *args)
{
    Bcp *pcb = (Bcp *)args;
    pcb->current_execution_state = PROCESS_STATE_READY;

    // Protege a adição à fila de prontos
    pthread_mutex_lock(&kernel_instance->scheduler->ready_queue_mutex);
    add_to_list(kernel_instance->scheduler->ready_queue, pcb);
    pthread_mutex_unlock(&kernel_instance->scheduler->ready_queue_mutex);

    // Se a CPU está ociosa, chama o escalonador para iniciar o processo
    if (kernel_instance->running_process == NULL)
    {
        Scheduler__perform_context_switch();
    }
    return NULL;
}

// Handler para término de fatia de tempo da CPU (timer)
void *thread_handler_cpu_timer_finish(void *args)
{
    Bcp *pcb = (Bcp *)args;
    CPU__set_busy(0);
    // Se o processo ainda está rodando, a CPU continua sua execução
    if (kernel_instance->running_process == pcb && pcb->current_execution_state == PROCESS_STATE_RUNNING)
    {
        CPU__run_process(pcb);
    }
    return NULL;
}

// Handler para requisição de disco
void *thread_handler_disk_request(void *args)
{
    IOArgs *io_args = (IOArgs *)args;
    io_args->process->current_execution_state = PROCESS_STATE_WAITING;
    IOManager__add_disk_request(io_args->process, io_args->value);
    Scheduler__perform_context_switch();
    free(io_args);
    return NULL;
}

// Handler para término de operação de disco
void *thread_handler_disk_finish(void *args)
{
    Bcp *pcb = (Bcp *)args;
    pcb->current_execution_state = PROCESS_STATE_READY;

    // Protege a adição à fila de prontos
    pthread_mutex_lock(&kernel_instance->scheduler->ready_queue_mutex);
    add_to_list(kernel_instance->scheduler->ready_queue, pcb);
    pthread_mutex_unlock(&kernel_instance->scheduler->ready_queue_mutex);

    Scheduler__perform_context_switch();
    return NULL;
}

// Handler para requisição de impressão
void *thread_handler_print_request(void *args)
{
    IOArgs *io_args = (IOArgs *)args;
    io_args->process->current_execution_state = PROCESS_STATE_WAITING;
    IOManager__add_printer_request(io_args->process, io_args->value);
    Scheduler__perform_context_switch();
    free(io_args);
    return NULL;
}

// Handler para término de impressão
void *thread_handler_print_finish(void *args)
{
    Bcp *pcb = (Bcp *)args;
    pcb->current_execution_state = PROCESS_STATE_READY;
    
    // Protege a adição à fila de prontos
    pthread_mutex_lock(&kernel_instance->scheduler->ready_queue_mutex);
    add_to_list(kernel_instance->scheduler->ready_queue, pcb);
    pthread_mutex_unlock(&kernel_instance->scheduler->ready_queue_mutex);

    Scheduler__perform_context_switch();
    return NULL;
}

// Handler para operação P (wait) em semáforo
void *thread_handler_semaphore_p(void *args)
{
    SemaArgs *s_args = (SemaArgs *)args;
    if (semaphore_P_operation(s_args->semaphore, s_args->process))
    {
        Scheduler__perform_context_switch();
    }
    free(s_args);
    return NULL;
}

// Handler para operação V (signal) em semáforo
void *thread_handler_semaphore_v(void *args)
{
    SemaArgs *s_args = (SemaArgs *)args;
    semaphore_V_operation(s_args->semaphore);
    Scheduler__perform_context_switch();
    free(s_args);
    return NULL;
}