#ifndef CLOCK_H_GUARD
#define CLOCK_H_GUARD

#include "../Nucleo/events.h"

// Inicializa o módulo do relógio e sua thread
void Clock__initialize();

// Agenda um evento para ocorrer após um certo 'delay' (em unidades de tempo)
void Clock__schedule_event(long long delay, EventType type, void *data);

// Retorna o tempo atual do relógio virtual
long long Clock__get_time();

#endif // CLOCK_H_GUARD