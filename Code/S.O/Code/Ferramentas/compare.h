#ifndef COMPARE_H
#define COMPARE_H

#define BUFFER_SIZE 256

// Declarações de funções de comparação para uso em listas, ordenações e buscas.
// Cada função retorna 0 se os elementos forem considerados iguais, valor diferente de zero caso contrário.

// Compara prioridade de I/O entre dois processos (usado em algoritmos de escalonamento por prioridade de I/O)
int compare_io_priority(const void *a, const void *b);

// Compara dois BCPs pelo PID. Retorna 0 se os PIDs forem iguais.
int compare_pid(void *a, void *b);

// Compara dois processos (BCP) por algum critério específico (ex: prioridade, tempo, etc.)
int compare_process(void *a, void *b);

// Compara dois segmentos de memória.
int compare_segment(void *a, void *b);

// Compara duas páginas de memória.
int compare_page(void *a, void *b);

// Compara duas strings. Retorna 0 se forem iguais.
int compare_strings_func(void *a, void *b);

#endif // COMPARE_H