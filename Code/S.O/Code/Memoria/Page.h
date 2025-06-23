#ifndef PAGE_DEF_H // Include guard para evitar conflitos com outros Page.h
#define PAGE_DEF_H

// -------------------- Definições de parâmetros de memória --------------------

// Tamanho total da memória física em bytes (1MB)
#define TOTAL_MEMORY_BYTES (1024 * 1024)

// Tamanho de cada página em bytes (1KB)
#define PAGE_SIZE_IN_BYTES 1024

// Número total de páginas físicas disponíveis
#define TOTAL_PHYSICAL_PAGES 1024

// Número de páginas reservadas para o Sistema Operacional
#define OS_RESERVED_PAGES_COUNT 64

// Máximo de páginas residentes permitidas por processo
#define MAX_RESIDENT_PAGES_PER_PROCESS 16

// -------------------- Estruturas de dados para gerenciamento de memória --------------------

// Representa uma entrada na tabela de páginas de um processo (página virtual)
typedef struct
{
    int physical_frame_idx; // Índice do quadro físico (-1 se não carregada)
    int present_bit;        // 1 se carregada na RAM, 0 se está no swap
    int reference_bit;      // 1 se foi recentemente usada (Segunda Chance)
} PageTableEntry;

// Representa a tabela de páginas de um processo
typedef struct
{
    int owner_process_id;          // PID do processo dono da tabela
    int virtual_page_count;        // Quantidade de páginas virtuais necessárias
    PageTableEntry *entries_table; // Vetor de entradas de páginas virtuais
} ProcessPageTable;

// Representa um quadro de memória física
typedef struct
{
    int is_occupied;             // 1 se ocupado, 0 se livre
    int resident_process_id;     // PID do processo dono da página
    int stored_virtual_page_idx; // Número da página virtual armazenada
    int frame_reference_bit;     // Bit de referência (Segunda Chance)
} PhysicalFrame;

// -------------------- Funções de gerenciamento de memória --------------------

/**
 * @brief Inicializa todas as estruturas de memória e tabelas.
 */
void initialize_memory_management();

/**
 * @brief Carrega uma página virtual para um quadro físico, usando Segunda Chance se necessário.
 * @param owner_process_id PID do processo dono da página.
 * @param virtual_page_idx Índice da página virtual a ser carregada.
 * @return Índice do quadro físico utilizado.
 */
int load_virtual_page_to_frame(int owner_process_id, int virtual_page_idx);

/**
 * @brief Libera todas as páginas de um processo quando ele termina.
 * @param owner_process_id PID do processo a ser liberado.
 */
void release_process_frames(int owner_process_id);

/**
 * @brief Seleciona uma página vítima para remoção usando o algoritmo Segunda Chance.
 * @return Índice do quadro físico selecionado como vítima.
 */
int second_chance_victim_selection();

/**
 * @brief Atualiza a tabela de páginas de um processo após carregamento de página.
 * @param owner_process_id PID do processo.
 * @param virtual_page_idx Índice da página virtual.
 * @param physical_frame_idx Índice do quadro físico.
 */
void update_page_table_on_load(int owner_process_id, int virtual_page_idx, int physical_frame_idx);

/**
 * @brief Inicializa a tabela de páginas de um processo a partir do arquivo de definição.
 * @param owner_process_id PID do processo.
 * @param process_definition_file Caminho para o arquivo de definição do processo.
 */
void initialize_process_specific_page_table(int owner_process_id, const char *process_definition_file);

/**
 * @brief Exibe o estado da memória física (para debug ou visualização).
 */
void display_physical_memory_state();

/**
 * @brief Obtém o estado atual dos quadros de memória física.
 * @param buffer Vetor de PhysicalFrame a ser preenchido.
 * @param buffer_size Tamanho do vetor.
 * @return Número de quadros copiados para o buffer.
 */
int get_memory_frames_state(PhysicalFrame *buffer, int buffer_size);

#endif // PAGE_DEF_H