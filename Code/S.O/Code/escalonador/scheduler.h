#ifndef SCHEDULER_H_GUARD
#define SCHEDULER_H_GUARD

#include "../Nucleo/kernel.h"
#include "../Ferramentas/list.h"
#include "../Ferramentas/compare.h"
#include <pthread.h> // Adicionado para pthread_mutex_t

/*
 * Estrutura principal do Escalonador.
 * Mantém a fila de processos prontos (ready_queue), ou seja,
 * processos que estão aptos a serem executados pela CPU.
 * Cada elemento da fila é um ponteiro para um BCP (Bloco de Controle de Processo).
 */
typedef struct Scheduler_s
{
    List *ready_queue; // Fila de Bcp* no estado PROCESS_STATE_READY
    pthread_mutex_t ready_queue_mutex; // Mutex para proteger a fila de prontos
} Scheduler;

/*
 * Cria e inicializa uma nova instância do escalonador.
 * Aloca memória para a estrutura e inicializa a fila de prontos.
 * Retorna: ponteiro para a nova struct Scheduler, ou NULL em caso de erro.
 */
Scheduler *Scheduler__create();

/*
 * Analisa a fila de prontos e seleciona o próximo processo a ser executado,
 * de acordo com o critério do projeto (menor número de operações de E/S).
 * Em caso de empate, escolhe o processo com menor PID.
 * Retorna: ponteiro para o BCP do processo escolhido, ou NULL se a fila estiver vazia.
 */
Bcp *Scheduler__get_next_process_to_run(void);

/*
 * Realiza a troca de contexto entre processos.
 * Remove o processo atualmente em execução (se necessário) e coloca-o de volta na fila de prontos.
 * Seleciona o próximo processo a ser executado e atualiza o estado do sistema.
 * Também registra a troca no log do escalonador.
 */
void Scheduler__perform_context_switch(void);

/*
 * Atualiza as estatísticas de operações de E/S (leitura ou escrita) de um processo.
 * Deve ser chamada sempre que um processo realiza uma operação de disco.
 * Parâmetros:
 * - target_process_bcp: ponteiro para o BCP do processo que realizou a operação.
 * - was_read_operation: 1 se foi leitura, 0 se foi escrita.
 */
void Scheduler__update_process_io_stats(Bcp *target_process_bcp, int was_read_operation);

#endif // SCHEDULER_H_GUARD