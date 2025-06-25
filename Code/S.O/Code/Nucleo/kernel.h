#ifndef KERNEL_H_GUARD
#define KERNEL_H_GUARD

// Inclusão de cabeçalhos necessários
#include <pthread.h>             // Para tipos e funções de threads (pthread_t, pthread_mutex_t, etc.)
#include "../Ferramentas/list.h" // Estrutura de dados de lista encadeada
#include "events.h"              // Definições de eventos do sistema (EventType, KernelEvent)
#include "../Process/process.h"  // Definição da estrutura de processo (Bcp)

// Declarações avançadas (Forward Declarations) para evitar dependência circular de includes.
// Informa ao compilador que esses tipos existem, sem precisar incluir seus cabeçalhos completos aqui.
struct Scheduler_s;
struct Semaphore_t;

/**
 * @brief Estrutura principal do Kernel do sistema operacional.
 *
 * Agrega ponteiros e dados para todas as principais estruturas de gerenciamento do SO,
 * servindo como o "cérebro" central do sistema.
 */
typedef struct Kernel_s
{
    int proc_id_counter;                 // Contador global para gerar PIDs (Process IDs) únicos.
    struct Scheduler_s *scheduler;       // Ponteiro para a estrutura do escalonador.
    List *pcb_list;                      // Lista global de todos os processos (BCPs) no sistema.
    List *semaphore_table;               // Lista de todos os semáforos criados.
    Bcp *running_process;                // Ponteiro para o BCP do processo que está atualmente na CPU.

    // Mutex para proteger a tabela de semáforos contra acessos concorrentes.
    pthread_mutex_t semaphore_table_mutex;

    // Lista de mensagens de log geradas pelo escalonador para exibição na interface.
    List *scheduler_log;
    // Mutex para proteger a lista de logs.
    pthread_mutex_t scheduler_log_mutex;
} Kernel;

// -----------------------------------------------------------------------------
// Variáveis globais do kernel e de sincronização.
// 'extern' indica que estas variáveis são definidas em outro arquivo (kernel.c).
// -----------------------------------------------------------------------------
extern Kernel *kernel_instance;        // Instância global e única do kernel.
extern pthread_mutex_t pcb_list_mutex; // Mutex para proteger a lista global de processos (pcb_list).

// Fila principal de eventos do sistema e seus mecanismos de sincronização.
extern List *event_queue;                 // Fila de eventos pendentes a serem processados pelo kernel.
extern pthread_mutex_t event_queue_mutex; // Mutex para proteger a fila de eventos.
extern pthread_cond_t event_queue_cond;   // Variável de condição para sinalizar quando novos eventos chegam.

// -----------------------------------------------------------------------------
// Protótipos das funções principais do núcleo.
// -----------------------------------------------------------------------------

/**
 * @brief Inicializa o kernel e todos os seus módulos (memória, I/O, CPU, etc.).
 * Deve ser chamada uma única vez no início da simulação.
 */
void Kernel__initialize();

/**
 * @brief Loop principal de simulação do kernel.
 *
 * Fica continuamente esperando por eventos na 'event_queue', retira-os e
 * despacha cada um para a thread de tratamento apropriada.
 */
void Kernel__run_simulation();

/**
 * @brief Adiciona (despacha) um novo evento para a fila de eventos do kernel.
 *
 * @param type O tipo do evento (ex: EVT_PROCESS_CREATE).
 * @param data Ponteiro para os dados associados ao evento (pode ser um BCP, IOArgs, etc.).
 */
void Kernel__dispatch_event(EventType type, void *data);

// -----------------------------------------------------------------------------
// Protótipos dos Handlers de Thread para cada tipo de evento do kernel.
// Cada uma dessas funções é o ponto de entrada para uma nova thread que tratará
// um tipo específico de evento, permitindo o processamento concorrente.
// -----------------------------------------------------------------------------
void *thread_handler_process_create(void *args);   // Trata a criação de um novo processo.
void *thread_handler_process_finish(void *args);   // Trata a finalização de um processo.
void *thread_handler_semaphore_p(void *args);      // Trata uma operação P (wait) em um semáforo.
void *thread_handler_semaphore_v(void *args);      // Trata uma operação V (signal) em um semáforo.
void *thread_handler_disk_request(void *args);     // Trata uma requisição de I/O de disco.
void *thread_handler_disk_finish(void *args);      // Trata a conclusão de uma operação de disco.
void *thread_handler_print_request(void *args);    // Trata uma requisição de impressão.
void *thread_handler_print_finish(void *args);     // Trata a conclusão de uma impressão.
void *thread_handler_mem_load_req(void *args);     // Trata a requisição para carregar um processo na memória.
void *thread_handler_mem_load_finish(void *args);  // Trata a conclusão do carregamento na memória.
void *thread_handler_cpu_timer_finish(void *args); // Trata o fim de uma fatia de tempo da CPU (quantum).

#endif // Fim do include guard KERNEL_H_GUARD
