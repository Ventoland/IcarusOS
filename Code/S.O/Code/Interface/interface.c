// Inclusão dos cabeçalhos de outros módulos do sistema operacional e bibliotecas padrão.
#include "interface.h"
#include "../Nucleo/kernel.h"
#include "../Clock/clock.h"
#include "../IO/io_manager.h"
#include "../Memoria/Page.h"
#include "../Semaforo/semaf.h"
#include <ncurses.h> // Biblioteca para a interface gráfica no terminal
#include <string.h>  // Para manipulação de strings (strlen, strcmp, etc.)
#include <stdlib.h>  // Para funções padrão (malloc, free, etc.)
#include <unistd.h>  // Para funções como usleep() e sleep()
#include <pthread.h> // Para a criação e gerenciamento de threads
#include <dirent.h>  // Para manipulação de diretórios (ler arquivos da pasta synt/)

// -----------------------------------------------------------------------------
// Estruturas e variáveis globais da interface gráfica (UI).
// Incluem definições para janelas, cores, e estrutura auxiliar para exibição.
// -----------------------------------------------------------------------------

// Estrutura auxiliar apenas para a UI, para desacoplar a exibição dos dados internos do BCP.
typedef struct
{
    int id;         // PID do processo
    char name[32];  // Nome do processo
    int priority;   // Prioridade do processo
    int segment_id; // Identificador do segmento de memória
    int size_kb;    // Tamanho do segmento em KB
    int io_count;   // Total de operações de I/O realizadas
    char state[16]; // Estado atual do processo (ex: PRONTO, EXECUTANDO)
} ProcessUI;

// Ponteiros globais para as janelas (sub-telas) que o ncurses vai gerenciar.
WINDOW *header_win, *menu_win, *kernel_win, *memory_win, *process_win, *input_win;

// Protótipos de funções que tratam a entrada do usuário.
void handle_input();
void handle_load_all_processes();

// -----------------------------------------------------------------------------
// Funções auxiliares para inicialização de cores e janelas da interface.
// -----------------------------------------------------------------------------

/**
 * @brief Inicializa os pares de cores que serão usados na interface.
 * Cada par tem uma cor de texto e uma cor de fundo.
 */
void init_colors()
{
    start_color(); // Habilita o uso de cores no terminal
    // Define os pares de cores com IDs de 1 a 7.
    init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Header
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Menu
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Kernel/Log
    init_pair(4, COLOR_CYAN, COLOR_BLACK);   // Memória/Semáforos
    init_pair(5, COLOR_WHITE, COLOR_BLACK);  // Processos
    init_pair(6, COLOR_BLACK, COLOR_WHITE);  // Input
    init_pair(7, COLOR_RED, COLOR_BLACK);    // Barra de uso de memória
}

/**
 * @brief Cria e configura as dimensões e posições de todas as janelas na tela.
 */
void init_windows()
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x); // Obtém as dimensões máximas do terminal

    // Cria a janela do cabeçalho no topo da tela.
    header_win = newwin(3, max_x, 0, 0);
    wbkgd(header_win, COLOR_PAIR(1)); // Define a cor de fundo

    // Cria a janela do menu no lado esquerdo.
    menu_win = newwin(max_y - 6, 20, 3, 0);
    wbkgd(menu_win, COLOR_PAIR(2));

    // Cria a janela de logs do kernel.
    kernel_win = newwin((max_y - 6) / 2, (max_x - 20) / 2, 3, 20);
    wbkgd(kernel_win, COLOR_PAIR(3));

    // Cria a janela de informações de memória e semáforos.
    memory_win = newwin((max_y - 6) / 2, (max_x - 20) / 2, 3, 20 + (max_x - 20) / 2);
    wbkgd(memory_win, COLOR_PAIR(4));

    // Cria a janela da tabela de processos.
    process_win = newwin((max_y - 6) / 2, max_x - 20, 3 + (max_y - 6) / 2, 20);
    wbkgd(process_win, COLOR_PAIR(5));

    // Cria a janela de entrada de comandos do usuário na parte inferior.
    input_win = newwin(3, max_x, max_y - 3, 0);
    wbkgd(input_win, COLOR_PAIR(6));
    scrollok(kernel_win, TRUE); // Permite que a janela do kernel role para cima quando cheia.
}

/**
 * @brief Desenha o cabeçalho superior da interface.
 */
void draw_header()
{
    werase(header_win); // Limpa o conteúdo anterior da janela
    box(header_win, 0, 0); // Desenha uma borda ao redor da janela
    const char *title = "Icarus OS";
    int max_x = getmaxx(header_win);
    int title_pos = (max_x - strlen(title)) / 2; // Calcula a posição para centralizar o título
    mvwprintw(header_win, 1, title_pos, "%s", title); // Imprime o título centralizado
    wrefresh(header_win); // Atualiza a janela na tela real
}

/**
 * @brief Desenha a janela do menu com as opções disponíveis.
 */
void draw_menu()
{
    werase(menu_win);
    box(menu_win, 0, 0);
    wattron(menu_win, COLOR_PAIR(2) | A_BOLD); // Ativa a cor e o negrito
    mvwprintw(menu_win, 1, 2, "CONTROLES");
    wattroff(menu_win, COLOR_PAIR(2) | A_BOLD); // Desativa a cor e o negrito
    mvwprintw(menu_win, 3, 2, "1. Carregar Proc.");
    mvwprintw(menu_win, 4, 2, "2. Carregar Todos");
    mvwprintw(menu_win, getmaxy(menu_win) - 2, 2, "Q - Sair");
    wrefresh(menu_win);
}

/**
 * @brief Desenha a janela de log do kernel e as filas de I/O.
 */
void draw_kernel()
{
    werase(kernel_win);
    box(kernel_win, 0, 0);
    wattron(kernel_win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(kernel_win, 1, 2, "LOG DO ESCALONADOR E EVENTOS");
    wattroff(kernel_win, COLOR_PAIR(3) | A_BOLD);

    // Trava o mutex para ler a lista de logs de forma segura (evitar race condition).
    pthread_mutex_lock(&kernel_instance->scheduler_log_mutex);
    Node *log_node = kernel_instance->scheduler_log->head;
    int line = 3;
    // Itera sobre a lista de logs e imprime cada mensagem.
    while (log_node != NULL && line < getmaxy(kernel_win) - 4)
    { 
        mvwprintw(kernel_win, line, 2, "> %s", (char *)log_node->data);
        log_node = log_node->next;
        line++;
    }
    pthread_mutex_unlock(&kernel_instance->scheduler_log_mutex); // Libera o mutex

    // Exibe o tamanho atual das filas de disco e impressora.
    mvwprintw(kernel_win, getmaxy(kernel_win) - 4, 2, "DISCO (SSTF) Fila: %d", IOManager_get_disk_queue_size());
    mvwprintw(kernel_win, getmaxy(kernel_win) - 3, 2, "IMPRESSORA Fila..: %d", IOManager_get_printer_queue_size());
    wrefresh(kernel_win);
}

/**
 * @brief Desenha a janela de status da memória e dos semáforos.
 */
void draw_memory()
{
    werase(memory_win);
    box(memory_win, 0, 0);
    wattron(memory_win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(memory_win, 1, 2, "GERENCIAMENTO DE MEMORIA E SEMAFOROS");
    wattroff(memory_win, COLOR_PAIR(4) | A_BOLD);

    // Obtém o estado atual dos frames de memória física.
    PhysicalFrame frames[TOTAL_PHYSICAL_PAGES];
    int frame_count = get_memory_frames_state(frames, TOTAL_PHYSICAL_PAGES);
    int used_frames = 0;
    for (int i = 0; i < frame_count; i++)
    {
        if (frames[i].is_occupied)
            used_frames++;
    }
    int total_mem_kb = (frame_count * PAGE_SIZE_IN_BYTES) / 1024;
    int used_mem_kb = (used_frames * PAGE_SIZE_IN_BYTES) / 1024;

    // Exibe o uso da memória em KB e porcentagem.
    mvwprintw(memory_win, 3, 2, "MEMORIA FISICA:");
    mvwprintw(memory_win, 4, 2, "Total: %d KB", total_mem_kb);
    mvwprintw(memory_win, 5, 2, "Usado: %d KB (%d%%)", used_mem_kb, total_mem_kb > 0 ? (used_mem_kb * 100) / total_mem_kb : 0);

    // Desenha uma barra de progresso para representar o uso da memória.
    int bar_width = getmaxx(memory_win) - 6;
    int used_bars = total_mem_kb > 0 ? (used_mem_kb * bar_width) / total_mem_kb : 0;
    mvwprintw(memory_win, 7, 2, "[");
    wattron(memory_win, COLOR_PAIR(7)); // Usa a cor vermelha para a parte usada
    for (int i = 0; i < used_bars; i++)
        wprintw(memory_win, "#");
    wattroff(memory_win, COLOR_PAIR(7));
    for (int i = used_bars; i < bar_width; i++)
        wprintw(memory_win, "-");
    wprintw(memory_win, "]");

    // Exibe o estado de cada semáforo no sistema.
    mvwprintw(memory_win, 9, 2, "SEMAFOROS:");
    pthread_mutex_lock(&kernel_instance->semaphore_table_mutex); // Trava para ler a lista de semáforos
    Node *s_node = kernel_instance->semaphore_table->head;
    int line = 10;
    while (s_node != NULL && line < getmaxy(memory_win) - 1)
    {
        Semaphore *sem = (Semaphore *)s_node->data;
        mvwprintw(memory_win, line, 2, "- Sem: [%c] | Valor: %2d | Fila: %d", sem->name_char, Semaphore_get_value(sem), Semaphore_get_waiting_count(sem));
        s_node = s_node->next;
        line++;
    }
    pthread_mutex_unlock(&kernel_instance->semaphore_table_mutex); // Libera o mutex
    wrefresh(memory_win);
}

/**
 * @brief Desenha a tabela com todos os processos ativos no sistema.
 */
void draw_process()
{
    werase(process_win);
    box(process_win, 0, 0);
    wattron(process_win, COLOR_PAIR(5) | A_BOLD);
    mvwprintw(process_win, 1, 2, "TABELA DE PROCESSOS (BCP)");
    wattroff(process_win, COLOR_PAIR(5) | A_BOLD);

    mvwprintw(process_win, 3, 2, "PID | Nome      | Prio | Seg | Size | I/O | Estado");
    mvwprintw(process_win, 4, 2, "----+-----------+------+-----+------+-----+-----------");

    // Trava o mutex da lista de PCBs para garantir uma leitura consistente.
    pthread_mutex_lock(&pcb_list_mutex);
    Node *p_node = kernel_instance->pcb_list->head;
    int line = 5;

    // Se a lista de processos estiver vazia, exibe uma mensagem.
    if (p_node == NULL)
    {
        mvwprintw(process_win, 6, 2, "Nenhum processo no sistema.");
    }
    else
    {
        // Itera sobre a lista de processos e imprime os detalhes de cada um.
        while (p_node != NULL && line < getmaxy(process_win) - 1)
        {
            Bcp *pcb = (Bcp *)p_node->data;
            const char *state_str;
            // Converte o enum de estado do processo para uma string legível.
            switch (pcb->current_execution_state)
            {
            case PROCESS_STATE_RUNNING:
                state_str = "EXECUTANDO";
                break;
            case PROCESS_STATE_READY:
                state_str = "PRONTO";
                break;
            case PROCESS_STATE_WAITING:
                state_str = "ESPERANDO";
                break;
            default:
                state_str = "NOVO";
                break;
            }
            // Imprime a linha formatada com os dados do processo.
            mvwprintw(process_win, line, 2, "%3d | %-9s | %4d | %3d | %4d | %3d | %s",
                      pcb->pid, pcb->name_str, pcb->priority_level, pcb->segment_identifier,
                      pcb->segment_size_bytes / 1024, pcb->total_read_ops + pcb->total_write_ops, state_str);
            p_node = p_node->next;
            line++;
        }
    }

    pthread_mutex_unlock(&pcb_list_mutex); // Libera o mutex
    wrefresh(process_win);
}

/**
 * @brief Desenha a área de entrada de comandos.
 */
void draw_input()
{
    werase(input_win);
    box(input_win, 0, 0);
    wattron(input_win, COLOR_PAIR(6) | A_BOLD);
    mvwprintw(input_win, 1, 2, "Comando: ");
    wattroff(input_win, COLOR_PAIR(6) | A_BOLD);
    wrefresh(input_win);
}

/**
 * @brief Lida com a entrada do usuário para carregar um único processo.
 */
void handle_input()
{
    char filename[256];

    // Prepara a janela de input para receber texto do usuário.
    wattron(input_win, COLOR_PAIR(6) | A_BOLD);
    mvwprintw(input_win, 1, 11, "Digite o nome do arquivo em 'synt/' e tecle ENTER: ");
    wattroff(input_win, COLOR_PAIR(6) | A_BOLD);
    echo();      // Habilita a exibição do que o usuário digita
    curs_set(1); // Mostra o cursor
    mvwgetstr(input_win, 1, 63, filename); // Captura a string digitada pelo usuário
    curs_set(0); // Esconde o cursor
    noecho();    // Desabilita a exibição do que o usuário digita

    // Se um nome de arquivo foi digitado...
    if (strlen(filename) > 0)
    {
        // Monta o caminho completo do arquivo.
        char *file_path = malloc(strlen("synt/") + strlen(filename) + 1);
        sprintf(file_path, "synt/%s", filename);

        // Envia um evento de criação de processo para o kernel.
        Kernel__dispatch_event(EVT_PROCESS_CREATE, file_path);

        // Exibe uma mensagem de confirmação para o usuário.
        mvwprintw(input_win, 1, 11, "Evento para '%s' enviado ao kernel!                            ", filename);
        wrefresh(input_win);
        sleep(1); // Pausa para que a mensagem possa ser lida.
    }
}

/**
 * @brief Lida com o comando para carregar todos os processos da pasta "synt/".
 */
void handle_load_all_processes()
{
    DIR *d;
    struct dirent *dir;
    const char *dir_path = "synt/";
    d = opendir(dir_path); // Abre o diretório
    int count = 0;

    if (d) // Se o diretório foi aberto com sucesso..
    {
        // Itera sobre cada entrada no diretório.
        while ((dir = readdir(d)) != NULL)
        {
            // Ignora os diretórios especiais "." e "..".
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }

            // Aloca memória para o caminho completo do arquivo.
            char *file_path = malloc(strlen(dir_path) + strlen(dir->d_name) + 1);
            if (file_path)
            {
                sprintf(file_path, "%s%s", dir_path, dir->d_name);

                // Dispara o evento de criação para cada arquivo encontrado.
                Kernel__dispatch_event(EVT_PROCESS_CREATE, file_path);
                count++;
            }
        }
        closedir(d); // Fecha o diretório.
    }

    // Exibe uma mensagem de feedback para o usuário.
    werase(input_win);
    box(input_win, 0, 0);
    wattron(input_win, COLOR_PAIR(6) | A_BOLD);
    mvwprintw(input_win, 1, 2, "Comando: ");
    wattroff(input_win, COLOR_PAIR(6) | A_BOLD);

    if (count > 0)
    {
        mvwprintw(input_win, 1, 11, "%d processo(s) enviado(s) para o kernel!", count);
    }
    else
    {
        mvwprintw(input_win, 1, 11, "Nenhum arquivo de processo encontrado em '%s'.", dir_path);
    }
    wrefresh(input_win);
    sleep(2); // Pausa para que a mensagem possa ser lida.
}

/**
 * @brief Ponto de entrada principal do programa da interface
 */
int main()
{
    // Inicialização do ncurses e do ambiente do terminal.
    initscr();  // Inicia o modo ncurses
    noecho();   // Não mostra a entrada do usuário na tela
    cbreak();   // Desabilita o buffer de linha, tornando os caracteres disponíveis imediatamente
    curs_set(0); // Esconde o cursor do terminal
    keypad(stdscr, TRUE); // Habilita a leitura de teclas especiais (F1, setas, etc.)

    // Inicializa as cores se o terminal suportar.
    if (has_colors())
    {
        init_colors();
    }

    // Inicializa o kernel e o executa em uma thread separada.
    Kernel__initialize();
    pthread_t kernel_thread;
    pthread_create(&kernel_thread, NULL, (void *(*)(void *))Kernel__run_simulation, NULL);
    pthread_detach(kernel_thread); // Desanexa a thread para que ela rode em segundo plano.

    // Inicia o loop principal da interface.
    UI_run_loop();

    // Finaliza o modo ncurses antes de sair do programa.
    endwin();
    return 0;
}

/**
 * @brief Loop principal da interface do usuário.
 * Responsável por redesenhar a tela e capturar a entrada do usuário continuamente.
 */
void UI_run_loop()
{
    init_windows(); // Inicializa todas as janelas
    nodelay(stdscr, TRUE); // getch() não bloqueia a execução, permitindo que a tela atualize

    int ch;
    while (1) // Loop infinito que roda até o usuário pressionar 'q'.
    {
        // Chama todas as funções de desenho para atualizar a tela.
        draw_header();
        draw_menu();
        draw_kernel();
        draw_memory();
        draw_process();
        draw_input();

        ch = getch(); // Captura a tecla pressionada pelo usuário.
        if (ch == 'q' || ch == 'Q')
        {
            break; // Sai do loop se a tecla for 'q' ou 'Q'.
        }
        if (ch == '1')
        {
            handle_input(); // Chama o handler para carregar um processo.
        }
        if (ch == '2') 
        {
            handle_load_all_processes(); // Chama o handler para carregar todos os processos.
        }

        usleep(100000); // Pausa de 100ms para uma taxa de atualização de 10Hz.
    }
}
