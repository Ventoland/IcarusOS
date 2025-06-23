#ifndef COMMON_STRUCTS_H_GUARD
#define COMMON_STRUCTS_H_GUARD

#include "../Process/process.h"
#include "../Semaforo/semaf.h"

// -----------------------------------------------------------------------------
// Estruturas auxiliares para argumentos de eventos e threads do núcleo.
// Centraliza os tipos usados para passagem de parâmetros entre módulos.
// -----------------------------------------------------------------------------

/**
 * @brief Argumentos para carregamento de memória de processo.
 *
 * Utilizado em eventos que envolvem carregar um processo na memória.
 */
typedef struct
{
    Bcp *pcb;        // Ponteiro para o bloco de controle do processo
    char *file_path; // Caminho do arquivo de definição do processo
} MemLoadArgs;

/**
 * @brief Argumentos para operações envolvendo semáforos.
 *
 * Utilizado em eventos de wait/signal de semáforos.
 */
typedef struct
{
    Semaphore *semaphore; // Ponteiro para o semáforo envolvido
    Bcp *process;         // Processo solicitante
} SemaArgs;

/**
 * @brief Argumentos para operações de I/O (disco, impressora, etc).
 *
 * Utilizado em eventos de requisição de disco ou impressão.
 */
typedef struct
{
    Bcp *process; // Processo solicitante
    int value;    // Parâmetro específico: trilha de disco, tempo de impressão, etc.
} IOArgs;

#endif // COMMON_STRUCTS_H_GUARD