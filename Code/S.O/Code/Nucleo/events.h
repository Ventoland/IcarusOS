#ifndef EVENTS_H_GUARD
#define EVENTS_H_GUARD

#include <stdlib.h>

/**
 * @brief Enumeração dos tipos de eventos do núcleo.
 *
 * Cada valor representa um tipo de evento que pode ser agendado ou tratado pelo kernel,
 * como criação/terminação de processo, requisições de disco, operações de semáforo, etc.
 */
typedef enum
{
    EVT_PROCESS_INTERRUPT = 1, // Interrupção de processo (ex: timer)
    EVT_PROCESS_CREATE = 2,    // Criação de novo processo
    EVT_PROCESS_FINISH = 3,    // Finalização de processo
    EVT_DISK_REQUEST = 4,      // Requisição de disco
    EVT_DISK_FINISH = 5,       // Conclusão de operação de disco
    EVT_MEM_LOAD_REQ = 6,      // Início da carga de processo na memória
    EVT_MEM_LOAD_FINISH = 7,   // Fim da carga de processo na memória
    EVT_SEMAPHORE_P = 10,      // Operação P (wait) em semáforo
    EVT_SEMAPHORE_V = 11,      // Operação V (signal) em semáforo
    EVT_PRINT_REQUEST = 14,    // Requisição de impressão
    EVT_PRINT_FINISH = 15,     // Conclusão de impressão
    EVT_CPU_TIMER_FINISH       // Evento de término de fatia de tempo da CPU
} EventType;

/**
 * @brief Estrutura que representa um evento do kernel.
 *
 * type: Tipo do evento (EventType)
 * data: Ponteiro genérico para dados adicionais do evento
 * execution_time: Momento (clock virtual) em que o evento deve ser processado
 */
typedef struct
{
    EventType type;
    void *data;
    long long execution_time;
} KernelEvent;

/**
 * @brief Compara dois eventos pelo ponteiro.
 * Retorna 0 se forem o mesmo evento, 1 caso contrário, -1 se algum for NULL.
 */
int compare_event(void *a, void *b);

/**
 * @brief Compara dois eventos pelo tempo de execução.
 * Retorna -1 se a < b, 1 se a > b, 0 se iguais.
 */
int compare_event_time(void *a, void *b);

#endif // EVENTS_H_GUARD