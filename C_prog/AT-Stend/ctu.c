#include <stdint.h>
#include <ai_talap.h>

int CTU(
    int signal,
    int reset,
    int initial_value,
    int *count,
    int *previous_signal,
    int *front,
    int *initialized)
{
    if (!*initialized)
    {
        *count = initial_value;
        *previous_signal = signal;
        *front = 0;
        *initialized = 1;

        return *count;
    }

    *front = 0;

    if (reset)
    {
        *count = initial_value;
        *previous_signal = signal;

        return *count;
    }

    if (signal && !*previous_signal)
    {
        *count = *count + 1;
        *front = 1;
    }

    *previous_signal = signal;

    return *count;
}

  //Инициализация выходных переменных
  static int count_val; //number
  static int front; //do2

//Инициализация внутренних переменных
  static int counter_initialized;
  static int counter_previous_signal;

void run() {
  //Инициализация входных переменных 
  int count_trig = lbvar("di5");
  int reset = lbvar("di6");
  
  


  
  //Счет
  count_val = CTU(count_trig, reset, 0, &count_val, &counter_previous_signal, &front, &counter_initialized);
  //Запись выходных переменных
  lbvar_out("number", count_val);
  lbvar_out("do2", front);
  
}

void init() {
  //Использовать при необходимости
  counter_initialized = 0;
  counter_previous_signal = 0;
  count_val = 0;
  front = 0;
}
