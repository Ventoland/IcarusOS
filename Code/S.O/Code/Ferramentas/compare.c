#include "compare.h"
#include "../Process/process.h"
#include <string.h>

// Função de comparação para BCPs baseada no PID.
// Retorna 0 se os PIDs forem iguais (usado para busca/remoção em listas), 1 caso contrário.
int compare_pid(void *a, void *b)
{
    Bcp *proc_a = (Bcp *)a;
    Bcp *proc_b = (Bcp *)b;
    return (proc_a->pid == proc_b->pid) ? 0 : 1;
}

// Função de comparação para strings.
// Retorna 0 se as strings forem iguais, valor diferente de zero caso contrário.
int compare_strings_func(void *a, void *b)
{
    return strcmp((char *)a, (char *)b);
}
