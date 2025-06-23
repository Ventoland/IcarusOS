#ifndef INTERFACE_H_GUARD
#define INTERFACE_H_GUARD

/**
 * @brief Inicializa e executa o loop principal da interface com o usuário (UI).
 *
 * Responsável por desenhar as janelas, atualizar informações do sistema,
 * capturar comandos do usuário e exibir logs e estados dos processos.
 *
 * Observação: O Kernel deve estar inicializado e rodando em paralelo.
 */
void UI_run_loop();

#endif // INTERFACE_H_GUARD