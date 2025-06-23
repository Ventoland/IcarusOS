#include "cpu.h"
#include "../Nucleo/kernel.h"
#include <pthread.h>
#include <unistd.h>

// Estado da CPU
static Bcp *current_process = NULL;
static int is_busy = 0; // 1 se a CPU está "ocupada" com uma instrução de tempo (exec)

// Sincronização
static pthread_mutex_t cpu_mutex;
static pthread_cond_t cpu_cond;

// Thread principal da CPU, que executa instruções em loop
void *cpu_thread_runner(void *args)
{
    while (1)
    {
        pthread_mutex_lock(&cpu_mutex);
        // Se não há processo ou a CPU está ocupada (em um 'exec' longo), a thread dorme
        while (current_process == NULL || is_busy)
        {
            pthread_cond_wait(&cpu_cond, &cpu_mutex);
        }
        pthread_mutex_unlock(&cpu_mutex);

        // Se acordou e há um processo válido e no estado de execução...
        if (current_process != NULL && current_process->current_execution_state == PROCESS_STATE_RUNNING)
        {
            // ... executa UMA instrução do processo
            execute_current_process_instruction(current_process);
        }

        // Pequena pausa para não sobrecarregar o processador do computador real
        // e permitir que outras threads (como a da UI) rodem.
        usleep(200000); // 0.2 segundos para melhor visualização
    }
    return NULL;
}

void CPU__initialize()
{
    pthread_mutex_init(&cpu_mutex, NULL);
    pthread_cond_init(&cpu_cond, NULL);
    pthread_t cpu_tid;
    pthread_create(&cpu_tid, NULL, cpu_thread_runner, NULL);
    pthread_detach(cpu_tid);
}

// Define qual processo a CPU deve executar (ou NULL para ociosa)
void CPU__run_process(Bcp *process)
{
    pthread_mutex_lock(&cpu_mutex);
    current_process = process;
    is_busy = 0; // Por padrão, a CPU não está ocupada

    // Acorda a thread da CPU se houver um processo para rodar
    if (process != NULL)
    {
        pthread_cond_signal(&cpu_cond);
    }
    pthread_mutex_unlock(&cpu_mutex);
}

// Usado pela instrução 'exec' para travar a CPU por um tempo
void CPU__set_busy(int busy_status)
{
    pthread_mutex_lock(&cpu_mutex);
    is_busy = busy_status;

    // Se a CPU deixou de estar ocupada, acorda a thread para continuar
    if (!is_busy)
    {
        pthread_cond_signal(&cpu_cond);
    }
    pthread_mutex_unlock(&cpu_mutex);
}