#ifndef IO_MANAGER_H_GUARD
#define IO_MANAGER_H_GUARD

#include "../Process/process.h"

/**
 * @brief Inicializa o gerenciador de E/S (I/O).
 *
 * Cria as filas de pedidos de disco e impressora, inicializa mutexes e threads
 * responsáveis pelo processamento das filas.
 */
void IOManager__initialize();

/**
 * @brief Adiciona uma requisição de disco à fila.
 *
 * @param process Ponteiro para o processo solicitante.
 * @param track   Número da trilha a ser acessada.
 */
void IOManager__add_disk_request(Bcp *process, int track);

/**
 * @brief Adiciona uma requisição de impressão à fila.
 *
 * @param process    Ponteiro para o processo solicitante.
 * @param print_time Tempo de impressão simulado (em ms ou ticks).
 */
void IOManager__add_printer_request(Bcp *process, int print_time);

/**
 * @brief Retorna o número de pedidos atualmente na fila de disco.
 *
 * Função utilizada pela interface gráfica para exibir o estado da fila.
 */
int IOManager_get_disk_queue_size();

/**
 * @brief Retorna o número de pedidos atualmente na fila de impressora.
 *
 * Função utilizada pela interface gráfica para exibir o estado da fila.
 */
int IOManager_get_printer_queue_size();

#endif // IO_MANAGER_H_GUARD