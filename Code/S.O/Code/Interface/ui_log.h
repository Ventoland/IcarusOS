#ifndef UI_LOG_H_GUARD
#define UI_LOG_H_GUARD

#include "../Ferramentas/list.h"

/**
 * @brief Inicializa e retorna uma nova lista de log para a interface.
 *
 * @param max_size Número máximo de mensagens que o log pode armazenar.
 * @return Ponteiro para a lista de log criada.
 */
List *UILog__create(int max_size);

/**
 * @brief Adiciona uma nova mensagem ao log da interface.
 *
 * Se o log atingir o tamanho máximo, a mensagem mais antiga é removida.
 *
 * @param log_list Ponteiro para a lista de log.
 * @param max_size Tamanho máximo do log.
 * @param message  Mensagem a ser adicionada (string).
 */
void UILog__add_message(List *log_list, int max_size, const char *message);

/**
 * @brief Obtém todas as mensagens do log para exibição na interface.
 *
 * Retorna uma lista de strings (char*) que deve ser liberada pela UI após o uso.
 *
 * @param log_list Ponteiro para a lista de log.
 * @return Lista de strings (char*).
 */
List *UILog__get_messages(List *log_list);

/**
 * @brief Trava o mutex de acesso ao log para garantir exclusão mútua.
 */
void UILog__lock(void);

/**
 * @brief Destrava o mutex de acesso ao log.
 */
void UILog__unlock(void);

#endif