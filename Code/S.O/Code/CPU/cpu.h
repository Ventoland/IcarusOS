#ifndef CPU_H_GUARD
#define CPU_H_GUARD

#include "../Process/process.h"

// Inicializa a CPU (configurações iniciais, se necessário)
void CPU__initialize();

// Executa um processo na CPU
void CPU__run_process(Bcp *process);

// Retorna se a CPU está ocupada (1) ou livre (0)
int CPU__is_busy();

// Define o status de ocupação da CPU (1 para ocupada, 0 para livre)
void CPU__set_busy(int busy_status);

#endif