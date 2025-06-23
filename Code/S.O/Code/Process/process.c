// Inclusão dos cabeçalhos de outros módulos do sistema e bibliotecas padrão.
#include "process.h"
#include "../Nucleo/kernel.h"
#include "../Nucleo/common_structs.h"
#include "../escalonador/scheduler.h"
#include "../CPU/cpu.h"
#include "../Clock/clock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define um Kbyte como 1024, para conversões de tamanho de segmento.
#ifndef KBYTE_DEF
#define KBYTE_DEF 1024
#endif

// -----------------------------------------------------------------------------
// Protótipos das funções auxiliares de parsing de arquivo de processo sintético.
// Essas funções são usadas internamente por este módulo.
// -----------------------------------------------------------------------------
Instruction *parse_instruction_from_string(char *instruction_line);
List *read_synthetic_file_instructions(FILE *fp);
void translate_string_to_opcode(Instruction *instruction_ptr, char *opcode_string);

// -----------------------------------------------------------------------------
// Função principal de execução de instrução do processo.
// É chamada pela CPU para executar a próxima instrução do processo ativo.
// -----------------------------------------------------------------------------
void execute_current_process_instruction(Bcp *active_process_bcp)
{
    // Validação: Garante que o processo existe e está no estado de 'EXECUTANDO'.
    if (!active_process_bcp || active_process_bcp->current_execution_state != PROCESS_STATE_RUNNING)
    {
        return; // Se não for válido, não faz nada.
    }

    // Verifica se o processo terminou (se o contador de programa ultrapassou o número de instruções).
    if (active_process_bcp->program_counter_val >= active_process_bcp->instructions_list_ptr->size)
    {
        // Se terminou, despacha um evento de finalização para o kernel.
        Kernel__dispatch_event(EVT_PROCESS_FINISH, active_process_bcp);
        return;
    }

    // Obtém a instrução atual com base no valor do Program Counter (PC).
    Node *instruction_node = active_process_bcp->instructions_list_ptr->head;
    for (int i = 0; i < active_process_bcp->program_counter_val; i++)
    {
        if (instruction_node)
            instruction_node = instruction_node->next;
    }

    // Segurança: Se, por algum motivo, o nó da instrução for nulo, encerra o processo.
    if (!instruction_node)
    {
        Kernel__dispatch_event(EVT_PROCESS_FINISH, active_process_bcp);
        return;
    }

    Instruction *current_instruction = (Instruction *)instruction_node->data;
    // Avança o Program Counter para a próxima instrução, preparando para a próxima execução.
    active_process_bcp->program_counter_val++;

    // Interpreta e executa a instrução com base no seu código de operação (opcode).
    switch (current_instruction->op_code_val)
    {
    case OP_CODE_EXEC:
    {
        // Instrução de uso de CPU: simula um trabalho computacional.
        long long exec_time = current_instruction->value_operand;
        CPU__set_busy(1);                                                           // Trava a CPU, indicando que está ocupada.
        Clock__schedule_event(exec_time, EVT_CPU_TIMER_FINISH, active_process_bcp); // Agenda um evento para destravar a CPU após o tempo de execução.
        break;
    }
    case OP_CODE_READ:
    case OP_CODE_WRITE:
    {
        // Instrução de I/O de Disco.
        // Atualiza as estatísticas de I/O para o escalonador (importante para o critério de prioridade).
        Scheduler__update_process_io_stats(active_process_bcp, (current_instruction->op_code_val == OP_CODE_READ));

        // Prepara os argumentos para o evento de requisição de disco.
        IOArgs *args = malloc(sizeof(IOArgs));
        args->process = active_process_bcp;
        args->value = current_instruction->value_operand; // O valor é a trilha do disco.
        Kernel__dispatch_event(EVT_DISK_REQUEST, args);   // Despacha o evento para o kernel.
        break;
    }
    case OP_CODE_PRINT:
    {
        // Instrução de I/O de Impressora.
        IOArgs *args = malloc(sizeof(IOArgs));
        args->process = active_process_bcp;
        args->value = current_instruction->value_operand; // O valor é o tempo de impressão.
        Kernel__dispatch_event(EVT_PRINT_REQUEST, args);
        break;
    }
    case OP_CODE_SEM_P:
    {
        // Operação P (wait) em um semáforo.
        SemaArgs *args = malloc(sizeof(SemaArgs));
        args->process = active_process_bcp;
        // Encontra o semáforo correspondente pelo seu caractere identificador.
        args->semaphore = find_semaphore_by_char_id(current_instruction->semaphore_id_char);
        if (args->semaphore) // Se o semáforo foi encontrado...
        {
            Kernel__dispatch_event(EVT_SEMAPHORE_P, args); // ...despacha o evento para o kernel.
        }
        else
        {
            free(args); // Libera a memória se o semáforo não existir.
        }
        break;
    }
    case OP_CODE_SEM_V:
    {
        // Operação V (signal) em um semáforo.
        SemaArgs *args = malloc(sizeof(SemaArgs));
        args->process = active_process_bcp;
        args->semaphore = find_semaphore_by_char_id(current_instruction->semaphore_id_char);
        if (args->semaphore)
        {
            Kernel__dispatch_event(EVT_SEMAPHORE_V, args);
        }
        else
        {
            free(args);
        }
        break;
    }
    default:
        // Se o opcode for desconhecido, ignora a instrução e continua a execução.
        break;
    }
}

// -----------------------------------------------------------------------------
// Função de criação de processo a partir de um arquivo sintético (.synt).
// Lê o arquivo, inicializa o BCP e agenda o evento para carregar na memória.
// -----------------------------------------------------------------------------
Bcp *Process__create(char *synthetic_file_path)
{
    FILE *file_pointer = fopen(synthetic_file_path, "r");
    if (!file_pointer) // Verifica se o arquivo pôde ser aberto.
        return NULL;

    Bcp *new_pcb = malloc(sizeof(Bcp));
    if (!new_pcb) // Verifica se a alocação de memória para o BCP foi bem-sucedida.
    {
        fclose(file_pointer);
        return NULL;
    }

    // Inicializa o BCP com zeros e define os valores iniciais.
    memset(new_pcb, 0, sizeof(Bcp));
    new_pcb->pid = kernel_instance->proc_id_counter++; // Atribui um PID único e incrementa o contador global.
    new_pcb->current_execution_state = PROCESS_STATE_NEW;

    // Lê os campos do cabeçalho do arquivo sintético.
    char buffer[256];
    fgets(buffer, sizeof(buffer), file_pointer);
    buffer[strcspn(buffer, "\n\r")] = 0; // Remove quebras de linha.
    new_pcb->name_str = strdup(buffer);  // Copia o nome do processo.
    fgets(buffer, sizeof(buffer), file_pointer);
    new_pcb->segment_identifier = atoi(buffer); // Converte string para inteiro.
    fgets(buffer, sizeof(buffer), file_pointer);
    new_pcb->priority_level = atoi(buffer);
    fgets(buffer, sizeof(buffer), file_pointer);
    new_pcb->segment_size_bytes = atoi(buffer) * KBYTE_DEF; // Converte o tamanho para bytes.
    fgets(buffer, sizeof(buffer), file_pointer);
    // Lê e inicializa os semáforos que este processo utilizará.
    Semaph__read_and_init_semaphores_from_line(buffer);

    // Lê todas as instruções do arquivo e as armazena na lista de instruções do BCP.
    new_pcb->instructions_list_ptr = read_synthetic_file_instructions(file_pointer);
    fclose(file_pointer);

    // Despacha um evento para que o kernel requisite o carregamento do processo na memória.
    Kernel__dispatch_event(EVT_MEM_LOAD_REQ, (void *)new_pcb);
    return new_pcb; // Retorna o BCP recém-criado.
}

// -----------------------------------------------------------------------------
// Função auxiliar que lê todas as linhas de instrução do arquivo e as monta
// em uma lista de estruturas 'Instruction'.
// -----------------------------------------------------------------------------
List *read_synthetic_file_instructions(FILE *fp)
{
    List *instructions = create_list();
    char line_buffer[256];
    // Lê o arquivo linha por linha até o final.
    while (fgets(line_buffer, sizeof(line_buffer), fp))
    {
        // Converte cada linha em uma estrutura de instrução.
        Instruction *instr = parse_instruction_from_string(line_buffer);
        if (instr) // Se a instrução for válida...
            add_to_list(instructions, instr); // ...a adiciona à lista.
    }
    return instructions;
}

// -----------------------------------------------------------------------------
// Função auxiliar que converte uma única linha de texto em uma estrutura 'Instruction'.
// -----------------------------------------------------------------------------
Instruction *parse_instruction_from_string(char *instruction_line)
{
    // Ignora linhas vazias ou inválidas.
    if (!instruction_line || instruction_line[0] == '\n' || instruction_line[0] == '\r')
        return NULL;

    Instruction *instr = malloc(sizeof(Instruction));
    if (!instr)
        return NULL;
    memset(instr, 0, sizeof(Instruction));

    char opcode_str[32];
    int value = 0;
    // Extrai a palavra-chave da operação e o valor numérico da linha.
    sscanf(instruction_line, "%s %d", opcode_str, &value);

    instr->value_operand = value;
    // Converte a string do opcode (ex: "exec") para o valor enum correspondente.
    translate_string_to_opcode(instr, opcode_str);

    return instr;
}

// -----------------------------------------------------------------------------
// Função auxiliar que traduz a string do opcode para o valor enum correspondente.
// -----------------------------------------------------------------------------
void translate_string_to_opcode(Instruction *target_instruction, char *opcode_keyword)
{
    if (!strcmp("exec", opcode_keyword))
        target_instruction->op_code_val = OP_CODE_EXEC;
    else if (!strcmp("read", opcode_keyword))
        target_instruction->op_code_val = OP_CODE_READ;
    else if (!strcmp("write", opcode_keyword))
        target_instruction->op_code_val = OP_CODE_WRITE;
    else if (!strcmp("print", opcode_keyword))
        target_instruction->op_code_val = OP_CODE_PRINT;
    // Tratamento especial para semáforos, para extrair o caractere identificador.
    else if (!strncmp("P(", opcode_keyword, 2))
    {
        target_instruction->op_code_val = OP_CODE_SEM_P;
        target_instruction->semaphore_id_char = opcode_keyword[2]; // Pega o 3º caractere, ex: 'S' de "P(S)".
    }
    else if (!strncmp("V(", opcode_keyword, 2))
    {
        target_instruction->op_code_val = OP_CODE_SEM_V;
        target_instruction->semaphore_id_char = opcode_keyword[2]; // Pega o 3º caractere, ex: 'S' de "V(S)".
    }
}

// -----------------------------------------------------------------------------
// Lê a linha de semáforos do arquivo .synt e os inicializa no sistema se
// ainda não existirem.
// -----------------------------------------------------------------------------
void Semaph__read_and_init_semaphores_from_line(char *line_with_sem_ids)
{
    if (!line_with_sem_ids || !kernel_instance || !kernel_instance->semaphore_table)
        return;
    // Itera sobre a string de identificadores (ex: "SRA").
    for (int i = 0; line_with_sem_ids[i] != '\0'; i++)
    {
        char id = line_with_sem_ids[i];
        if (id > ' ') // Ignora espaços e caracteres de controle.
        {
            // Verifica se o semáforo já existe no sistema.
            if (!find_semaphore_by_char_id(id))
            {
                // Se não existir, cria um novo semáforo com valor inicial 1 e o adiciona à lista global.
                add_to_list(kernel_instance->semaphore_table, initialize_semaphore(id, 1));
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Função de busca que encontra um semáforo na lista global do kernel
// pelo seu caractere identificador.
// -----------------------------------------------------------------------------
Semaphore *find_semaphore_by_char_id(char target_sem_char_id)
{
    if (!kernel_instance || !kernel_instance->semaphore_table)
        return NULL;
    Node *node = kernel_instance->semaphore_table->head;
    while (node) // Itera sobre a lista de semáforos.
    {
        Semaphore *sem = (Semaphore *)node->data;
        if (sem->name_char == target_sem_char_id)
            return sem; // Retorna o ponteiro para o semáforo se encontrado.
        node = node->next;
    }
    return NULL; // Retorna NULL se não for encontrado.
}
