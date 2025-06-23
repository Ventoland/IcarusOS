#ifndef SEMAF_H_GUARD
#define SEMAF_H_GUARD

#include "../Ferramentas/list.h"
#include "../Process/process.h"
#include <pthread.h>

/**
 * @brief Estrutura do Semáforo.
 *
 * name_char: identificador do semáforo (caractere)
 * current_value: valor atual do semáforo
 * waiting_processes_q: fila de processos bloqueados aguardando o semáforo
 * mutex: proteção para operações concorrentes no semáforo
 */
typedef struct Semaphore_t
{
    char name_char;
    int current_value;
    List *waiting_processes_q;
    pthread_mutex_t mutex;
} Semaphore;

// -----------------------------------------------------------------------------
// Operações principais do semáforo
// -----------------------------------------------------------------------------

/**
 * @brief Inicializa um novo semáforo.
 * @param id_char Identificador do semáforo (caractere).
 * @param initial_count Valor inicial do semáforo.
 * @return Ponteiro para o semáforo criado.
 */
Semaphore *initialize_semaphore(char id_char, const int initial_count);

/**
 * @brief Operação P (wait) no semáforo.
 * Decrementa o valor e bloqueia o processo se necessário.
 * @param sem_instance Ponteiro para o semáforo.
 * @param requesting_process_bcp Processo solicitante.
 * @return 1 se o processo foi bloqueado, 0 caso contrário.
 */
int semaphore_P_operation(Semaphore *sem_instance, Bcp *requesting_process_bcp);

/**
 * @brief Operação V (signal) no semáforo.
 * Incrementa o valor e acorda um processo, se houver.
 * @param sem_instance Ponteiro para o semáforo.
 */
void semaphore_V_operation(Semaphore *sem_instance);

// -----------------------------------------------------------------------------
// Funções getter para a interface visualizar o estado dos semáforos
// -----------------------------------------------------------------------------

/**
 * @brief Retorna o valor atual do semáforo.
 */
int Semaphore_get_value(Semaphore *sem);

/**
 * @brief Retorna o número de processos aguardando no semáforo.
 */
int Semaphore_get_waiting_count(Semaphore *sem);

#endif // SEMAF_H_GUARD