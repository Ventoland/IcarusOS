#ifndef PROCESS_H_GUARD
#define PROCESS_H_GUARD

#include "../Ferramentas/list.h"
#include <stdio.h>

// Forward declaration para evitar dependências circulares com semaf.h
struct Semaphore_t;

// -----------------------------------------------------------------------------
// Tipos e enums relacionados a processos e instruções
// -----------------------------------------------------------------------------

// Enumeração dos códigos de operação suportados por instruções de processo
typedef enum
{
    OP_CODE_EXEC = 0,  // exec <tempo>
    OP_CODE_READ = 1,  // read <trilha>
    OP_CODE_WRITE = 2, // write <trilha>
    OP_CODE_PRINT = 3, // print <tempo>
    OP_CODE_SEM_P = 4, // P(<id>)
    OP_CODE_SEM_V = 5  // V(<id>)
} OperationCodeValue;

// Estrutura de uma instrução de processo
typedef struct Instruction_s
{
    OperationCodeValue op_code_val; // Código da operação
    int value_operand;              // Operando numérico (tempo, trilha, etc.)
    char semaphore_id_char;         // Identificador do semáforo (se aplicável)
} Instruction;

// Enumeração dos estados possíveis de um processo
typedef enum
{
    PROCESS_STATE_NEW = 0,        // Processo recém-criado
    PROCESS_STATE_READY = 1,      // Pronto para executar
    PROCESS_STATE_WAITING = 3,    // Aguardando I/O ou semáforo
    PROCESS_STATE_TERMINATED = 4, // Finalizado
    PROCESS_STATE_RUNNING = 5     // Em execução
} ProcessCurrentState;

// Estrutura do Bloco de Controle de Processo (BCP)
typedef struct Bcp_t
{
    int pid;                                     // Identificador do processo
    char *name_str;                              // Nome do processo
    ProcessCurrentState current_execution_state; // Estado atual
    int program_counter_val;                     // Program Counter (índice da próxima instrução)
    int segment_identifier;                      // Identificador do segmento de memória
    int priority_level;                          // Prioridade do processo
    int segment_size_bytes;                      // Tamanho do segmento em bytes
    int total_read_ops;                          // Total de operações de leitura realizadas
    int total_write_ops;                         // Total de operações de escrita realizadas
    List *instructions_list_ptr;                 // Lista de instruções do processo
} Bcp;

// -----------------------------------------------------------------------------
// Funções do módulo de processo
// -----------------------------------------------------------------------------

/**
 * @brief Cria um novo processo a partir de um arquivo sintético.
 * @param synthetic_file_path Caminho para o arquivo de definição do processo.
 * @return Ponteiro para o BCP criado.
 */
Bcp *Process__create(char *synthetic_file_path);

/**
 * @brief Finaliza um processo e libera seus recursos.
 * @param target_bcp Ponteiro para o BCP do processo a ser finalizado.
 */
void Process__finish(Bcp *target_bcp);

/**
 * @brief Executa a próxima instrução do processo ativo.
 * @param active_process_bcp Ponteiro para o BCP do processo em execução.
 */
void execute_current_process_instruction(Bcp *active_process_bcp);

/**
 * @brief Traduz uma string de opcode para o valor numérico correspondente.
 */
void translate_string_to_opcode(Instruction *instruction_ptr, char *opcode_string);

/**
 * @brief Faz o parsing de uma linha de instrução para a estrutura Instruction.
 */
Instruction *parse_instruction_from_string(char *instruction_line);

/**
 * @brief Inicializa semáforos a partir de uma linha do arquivo sintético.
 */
void Semaph__read_and_init_semaphores_from_line(char *line_with_sem_ids);

/**
 * @brief Busca um semáforo pelo seu identificador de caractere.
 */
struct Semaphore_t *find_semaphore_by_char_id(char target_sem_char_id);

#endif // PROCESS_H_GUARD