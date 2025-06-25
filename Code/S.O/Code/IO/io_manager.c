#include "io_manager.h"
#include "../Nucleo/kernel.h"
#include "../Ferramentas/list.h"
#include <pthread.h> // Para threads (pthreads)
#include <stdlib.h>  // Para funções padrão como malloc, free, abs
#include <unistd.h>  // Para a função usleep (não utilizada neste arquivo, mas comum em I/O)
#include <limits.h>  // Para constantes como INT_MAX (usado no algoritmo SSTF)
#include <stdio.h>   // Para funções de entrada/saída padrão
#include "../Clock/clock.h" // Para agendar eventos futuros

// Estrutura interna, específica deste módulo, para representar um pedido de I/O.
// Ela encapsula o processo que fez a requisição e um valor genérico.
typedef struct
{
    Bcp *process; // Ponteiro para o Bloco de Controle do Processo associado ao pedido.
    int value;    // Para disco: representa o número da trilha. Para impressora: o tempo de impressão.
} IORequest;

// -----------------------------------------------------------------------------
// Variáveis estáticas do módulo: filas, posição do disco e mecanismos de sincronização.
// 'static' significa que estas variáveis só são visíveis dentro deste arquivo.
// -----------------------------------------------------------------------------
static List *disk_queue;                   // Fila de pedidos de acesso ao disco.
static List *printer_queue;                // Fila de pedidos para a impressora.
static int current_disk_head_position = 0; // Posição atual da cabeça de leitura/escrita do disco.

// Mutexes e variáveis de condição para garantir a segurança em ambiente concorrente (thread-safe).
static pthread_mutex_t disk_mutex;    // Mutex para proteger o acesso à fila de disco.
static pthread_cond_t disk_cond;      // Variável de condição para sinalizar que um novo pedido de disco chegou.
static pthread_mutex_t printer_mutex; // Mutex para proteger o acesso à fila da impressora.
static pthread_cond_t printer_cond;   // Variável de condição para sinalizar que um novo pedido de impressão chegou.

// Protótipos das funções que serão executadas pelas threads de processamento.
void *process_disk_queue_thread(void *args);
void *process_printer_queue_thread(void *args);

/**
 * @brief Função de comparação para remover um IORequest de uma lista.
 * Compara os ponteiros para verificar se são o mesmo objeto na memória.
 * @return 0 se os ponteiros forem iguais, 1 caso contrário.
 */
int compare_io_request(void *a, void *b)
{
    return (a == b) ? 0 : 1;
}

/**
 * @brief Inicializa o Gerenciador de I/O.
 * Esta função é chamada uma vez no início do sistema operacional.
 */
void IOManager__initialize()
{
    // Inicializa as filas de pedidos de disco e impressora como listas vazias.
    disk_queue = create_list();
    printer_queue = create_list();

    // Inicializa os mutexes e variáveis de condição.
    pthread_mutex_init(&disk_mutex, NULL);
    pthread_cond_init(&disk_cond, NULL);
    pthread_mutex_init(&printer_mutex, NULL);
    pthread_cond_init(&printer_cond, NULL);

    // Cria e inicia as threads que processarão as filas de I/O em segundo plano.
    pthread_t disk_tid, printer_tid;
    pthread_create(&disk_tid, NULL, process_disk_queue_thread, NULL);
    pthread_create(&printer_tid, NULL, process_printer_queue_thread, NULL);
    // Desanexa as threads para que rodem de forma independente, sem a necessidade de um join.
    pthread_detach(disk_tid);
    pthread_detach(printer_tid);
}

/**
 * @brief Adiciona uma nova requisição de acesso ao disco.
 * @param process O processo que está fazendo a requisição.
 * @param track A trilha do disco a ser acessada.
 */
void IOManager__add_disk_request(Bcp *process, int track)
{
    // Cria um novo pedido de disco.
    IORequest *req = malloc(sizeof(IORequest));
    req->process = process;
    req->value = track;

    // Trava o mutex para adicionar o pedido à fila de forma segura.
    pthread_mutex_lock(&disk_mutex);
    add_to_list(disk_queue, req);
    // Sinaliza para a thread de processamento de disco que há um novo item na fila.
    pthread_cond_signal(&disk_cond);
    // Libera o mutex.
    pthread_mutex_unlock(&disk_mutex);
}

/**
 * @brief Adiciona uma nova requisição de impressão.
 * @param process O processo que está fazendo a requisição.
 * @param print_time O tempo necessário para a impressão.
 */
void IOManager__add_printer_request(Bcp *process, int print_time)
{
    // Cria um novo pedido de impressão.
    IORequest *req = malloc(sizeof(IORequest));
    req->process = process;
    req->value = print_time;

    // Trava o mutex para adicionar o pedido à fila de forma segura.
    pthread_mutex_lock(&printer_mutex);
    add_to_list(printer_queue, req);
    // Sinaliza para a thread de processamento da impressora que há um novo item na fila.
    pthread_cond_signal(&printer_cond);
    // Libera o mutex.
    pthread_mutex_unlock(&printer_mutex);
}

/**
 * @brief Thread que processa a fila de disco em um loop infinito.
 */
void *process_disk_queue_thread(void *args)
{
    while (1) // Loop eterno da thread.
    {
        // Trava o mutex para acessar a fila.
        pthread_mutex_lock(&disk_mutex);
        // Se a fila estiver vazia, a thread dorme até ser acordada por um sinal (pthread_cond_signal).
        while (disk_queue->size == 0)
        {
            pthread_cond_wait(&disk_cond, &disk_mutex);
        }

        // --- Início do Algoritmo SSTF (Shortest Seek Time First) ---
        Node *current = disk_queue->head;
        IORequest *chosen_req = NULL;
        int min_seek = INT_MAX; // Começa com a maior distância possível.

        // Itera sobre toda a fila para encontrar o pedido mais próximo da posição atual da cabeça do disco.
        while (current != NULL)
        {
            IORequest *req = (IORequest *)current->data;
            int seek = abs(req->value - current_disk_head_position); // Calcula a distância.
            if (seek < min_seek) // Se a distância for menor que a mínima encontrada até agora...
            {
                min_seek = seek;      // ...atualiza a distância mínima.
                chosen_req = req;     // ...e seleciona este como o próximo pedido a ser atendido.
            }
            current = current->next;
        }
        // --- Fim do Algoritmo SSTF ---

        // Remove o pedido escolhido da fila.
        remove_from_list(disk_queue, chosen_req, compare_io_request);
        // Libera o mutex, permitindo que novas requisições cheguem.
        pthread_mutex_unlock(&disk_mutex);

        // Simula o movimento da cabeça do disco e o tempo de acesso.
        current_disk_head_position = chosen_req->value; // Atualiza a posição da cabeça.
        // Agenda um evento no clock para sinalizar o fim da operação de I/O após um tempo fixo.
        Clock__schedule_event(4000, EVT_DISK_FINISH, chosen_req->process);

        // Libera a memória da estrutura da requisição.
        free(chosen_req);
    }
    return NULL;
}

/**
 * @brief Thread que processa a fila da impressora em um loop infinito.
 * Utiliza uma estratégia FCFS (First-Come, First-Served), pois simplesmente pega o primeiro da fila.
 */
void *process_printer_queue_thread(void *args)
{
    while (1) // Loop eterno da thread.
    {
        // Trava o mutex para acessar a fila.
        pthread_mutex_lock(&printer_mutex);
        // Se a fila estiver vazia, a thread dorme até ser acordada por um sinal.
        while (printer_queue->size == 0)
        {
            pthread_cond_wait(&printer_cond, &printer_mutex);
        }

        // Pega o primeiro pedido da fila (FCFS).
        Node *req_node = printer_queue->head;
        IORequest *req_to_process = (IORequest *)req_node->data;

        // Remove o pedido da fila.
        remove_from_list(printer_queue, req_to_process, compare_io_request);
        // Libera o mutex.
        pthread_mutex_unlock(&printer_mutex);

        // Agenda um evento no clock para sinalizar o fim da impressão.
        // O tempo de impressão é variável, definido pelo próprio processo.
        Clock__schedule_event(req_to_process->value, EVT_PRINT_FINISH, req_to_process->process);

        // Libera a memória da estrutura da requisição.
        free(req_to_process);
    }
    return NULL;
}

/**
 * @brief Retorna o número de pedidos na fila de disco.
 * Função segura para ser chamada pela UI, pois usa mutex.
 */
int IOManager_get_disk_queue_size()
{
    pthread_mutex_lock(&disk_mutex);
    int size = disk_queue->size;
    pthread_mutex_unlock(&disk_mutex);
    return size;
}

/**
 * @brief Retorna o número de pedidos na fila da impressora.
 * Função segura para ser chamada pela UI, pois usa mutex.
 */
int IOManager_get_printer_queue_size()
{
    pthread_mutex_lock(&printer_mutex);
    int size = printer_queue->size;
    pthread_mutex_unlock(&printer_mutex);
    return size;
}
