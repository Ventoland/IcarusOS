#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Page.h"

#define USABLE_PHYSICAL_FRAMES (TOTAL_PHYSICAL_PAGES - OS_RESERVED_PAGES_COUNT) // Quadros físicos utilizáveis (exclui reservados para o SO)
#define MAX_SIMULTANEOUS_PROCESSES 64                                           // Máximo de processos simultâneos suportados

// -------------------- Estruturas globais de dados da memória -------------------

// Vetor de quadros físicos da memória principal
PhysicalFrame physical_memory_frames[USABLE_PHYSICAL_FRAMES];

// Registro de tabelas de páginas de todos os processos ativos
ProcessPageTable process_page_table_registry[MAX_SIMULTANEOUS_PROCESSES];

// Ponteiro do relógio para o algoritmo Segunda Chance
int second_chance_clock_hand = 0;

// -------------------- Inicialização das estruturas de memória --------------------

void initialize_memory_management()
{
    // Inicializa todos os quadros físicos como livres
    for (int i = 0; i < USABLE_PHYSICAL_FRAMES; i++)
    {
        physical_memory_frames[i].is_occupied = 0;
        physical_memory_frames[i].resident_process_id = -1;
        physical_memory_frames[i].stored_virtual_page_idx = -1;
        physical_memory_frames[i].frame_reference_bit = 0;
    }

    // Inicializa todas as tabelas de páginas dos processos como vazias
    for (int i = 0; i < MAX_SIMULTANEOUS_PROCESSES; i++)
    {
        process_page_table_registry[i].owner_process_id = -1;
        process_page_table_registry[i].virtual_page_count = 0;
        process_page_table_registry[i].entries_table = NULL;
    }
}

// -------------------- Algoritmo Segunda Chance para substituição de páginas ------------------

int second_chance_victim_selection()
{
    // Percorre os quadros físicos circularmente até encontrar um com bit de referência 0
    while (1)
    {
        if (physical_memory_frames[second_chance_clock_hand].frame_reference_bit == 0)
        {
            int chosen_frame_idx = second_chance_clock_hand;
            second_chance_clock_hand = (second_chance_clock_hand + 1) % USABLE_PHYSICAL_FRAMES;
            return chosen_frame_idx;
        }
        else
        {
            // Zera o bit de referência e avança o ponteiro do relógio
            physical_memory_frames[second_chance_clock_hand].frame_reference_bit = 0;
            second_chance_clock_hand = (second_chance_clock_hand + 1) % USABLE_PHYSICAL_FRAMES;
        }
    }
}

// -------------------- Atualização da tabela de páginas após carregamento --------------------

void update_page_table_on_load(int process_id_val, int virtual_page_index, int physical_frame_index)
{
    // Atualiza a entrada da tabela de páginas do processo após carregar uma página
    for (int i = 0; i < MAX_SIMULTANEOUS_PROCESSES; i++)
    {
        if (process_page_table_registry[i].owner_process_id == process_id_val)
        {
            process_page_table_registry[i].entries_table[virtual_page_index].physical_frame_idx = physical_frame_index;
            process_page_table_registry[i].entries_table[virtual_page_index].present_bit = 1;
            process_page_table_registry[i].entries_table[virtual_page_index].reference_bit = 1;
            return;
        }
    }
}

// ------------------ Carregamento de página virtual para quadro físico --------------------

int load_virtual_page_to_frame(int process_id_val, int virtual_page_index)
{
    // Busca a tabela de páginas do processo
    int i;
    for (i = 0; i < MAX_SIMULTANEOUS_PROCESSES; i++)
    {
        if (process_page_table_registry[i].owner_process_id == process_id_val)
            break;
    }
    if (i == MAX_SIMULTANEOUS_PROCESSES)
        return -1;

    ProcessPageTable *current_proc_pt = &process_page_table_registry[i];

    // Se a página já está presente, apenas atualiza o bit de referência
    if (current_proc_pt->entries_table[virtual_page_index].present_bit == 1)
    {
        int frame_idx = current_proc_pt->entries_table[virtual_page_index].physical_frame_idx;
        if (frame_idx >= 0 && frame_idx < USABLE_PHYSICAL_FRAMES)
        {
            physical_memory_frames[frame_idx].frame_reference_bit = 1;
        }
        return frame_idx;
    }

    // Procura um quadro físico livre
    int target_frame_idx = -1;
    for (int j = 0; j < USABLE_PHYSICAL_FRAMES; j++)
    {
        if (physical_memory_frames[j].is_occupied == 0)
        {
            target_frame_idx = j;
            break;
        }
    }

    // Se não houver quadro livre, seleciona uma vítima pelo algoritmo Segunda Chance
    if (target_frame_idx == -1)
    {
        target_frame_idx = second_chance_victim_selection();
    }

    // Se o quadro estava ocupado, atualiza a tabela do processo vítima
    if (physical_memory_frames[target_frame_idx].is_occupied)
    {
        int victim_proc_id = physical_memory_frames[target_frame_idx].resident_process_id;
        int victim_page_idx = physical_memory_frames[target_frame_idx].stored_virtual_page_idx;
        for (int p_idx = 0; p_idx < MAX_SIMULTANEOUS_PROCESSES; p_idx++)
        {
            if (process_page_table_registry[p_idx].owner_process_id == victim_proc_id)
            {
                process_page_table_registry[p_idx].entries_table[victim_page_idx].present_bit = 0;
                process_page_table_registry[p_idx].entries_table[victim_page_idx].physical_frame_idx = -1;
                break;
            }
        }
    }

    // Atualiza a tabela de páginas do processo atual e o quadro físico
    update_page_table_on_load(process_id_val, virtual_page_index, target_frame_idx);
    physical_memory_frames[target_frame_idx].is_occupied = 1;
    physical_memory_frames[target_frame_idx].resident_process_id = process_id_val;
    physical_memory_frames[target_frame_idx].stored_virtual_page_idx = virtual_page_index;
    physical_memory_frames[target_frame_idx].frame_reference_bit = 1;

    return target_frame_idx;
}

// -------------------- Liberação de quadros físicos de um processo --------------------

void release_process_frames(int process_id_val)
{
    // Libera todos os quadros físicos ocupados pelo processo
    for (int i = 0; i < USABLE_PHYSICAL_FRAMES; i++)
    {
        if (physical_memory_frames[i].is_occupied && physical_memory_frames[i].resident_process_id == process_id_val)
        {
            physical_memory_frames[i].is_occupied = 0;
            physical_memory_frames[i].resident_process_id = -1;
            physical_memory_frames[i].stored_virtual_page_idx = -1;
            physical_memory_frames[i].frame_reference_bit = 0;
        }
    }

    // Libera a tabela de páginas do processo
    for (int i = 0; i < MAX_SIMULTANEOUS_PROCESSES; i++)
    {
        if (process_page_table_registry[i].owner_process_id == process_id_val)
        {
            if (process_page_table_registry[i].entries_table != NULL)
            {
                free(process_page_table_registry[i].entries_table);
            }
            process_page_table_registry[i].entries_table = NULL;
            process_page_table_registry[i].owner_process_id = -1;
            process_page_table_registry[i].virtual_page_count = 0;
            break;
        }
    }
}

// -------------------- Inicialização da tabela de páginas de um processo --------------------

void initialize_process_specific_page_table(int process_id_val, const char *process_file_path)
{
    // Lê o arquivo de definição do processo para calcular o número de páginas necessárias
    FILE *fp = fopen(process_file_path, "r");
    if (!fp)
    {
        return;
    }

    char line_buffer[128];
    int total_exec_instruction_time = 0;
    int header_lines_to_skip = 5;

    // Ignora as linhas de cabeçalho do arquivo
    for (int i = 0; i < header_lines_to_skip && fgets(line_buffer, sizeof(line_buffer), fp); i++)
        ;

    // Soma o tempo total das instruções 'exec'
    while (fgets(line_buffer, sizeof(line_buffer), fp))
    {
        char instruction_word[16];
        int value_param;
        if (sscanf(line_buffer, "%s %d", instruction_word, &value_param) == 2)
        {
            if (strcmp(instruction_word, "exec") == 0)
            {
                total_exec_instruction_time += value_param;
            }
        }
    }
    fclose(fp);

    // Calcula o número de páginas necessárias para o processo
    int num_of_pages = (total_exec_instruction_time + PAGE_SIZE_IN_BYTES - 1) / PAGE_SIZE_IN_BYTES;
    if (num_of_pages == 0 && total_exec_instruction_time > 0)
        num_of_pages = 1;
    if (num_of_pages > MAX_RESIDENT_PAGES_PER_PROCESS)
        num_of_pages = MAX_RESIDENT_PAGES_PER_PROCESS;

    // Procura um slot livre no registro de tabelas de páginas
    for (int i = 0; i < MAX_SIMULTANEOUS_PROCESSES; i++)
    {
        if (process_page_table_registry[i].owner_process_id == -1)
        {
            process_page_table_registry[i].owner_process_id = process_id_val;
            process_page_table_registry[i].virtual_page_count = num_of_pages;
            process_page_table_registry[i].entries_table = malloc(sizeof(PageTableEntry) * num_of_pages);

            if (process_page_table_registry[i].entries_table == NULL && num_of_pages > 0)
            {
                process_page_table_registry[i].owner_process_id = -1;
                return;
            }
            // Inicializa todas as entradas da tabela de páginas como não presentes
            for (int j = 0; j < num_of_pages; j++)
            {
                process_page_table_registry[i].entries_table[j].physical_frame_idx = -1;
                process_page_table_registry[i].entries_table[j].present_bit = 0;
                process_page_table_registry[i].entries_table[j].reference_bit = 0;
            }
            break;
        }
    }
}

// -------------------- Função getter para a UI visualizar o estado da memória --------------------

// Copia o estado dos quadros físicos para um buffer fornecido pela UI
int get_memory_frames_state(PhysicalFrame *buffer, int buffer_size)
{
    int count = 0;
    for (int i = 0; i < USABLE_PHYSICAL_FRAMES && i < buffer_size; i++)
    {
        buffer[i] = physical_memory_frames[i];
        count++;
    }
    return count;
}