#include <stdint.h>
#include <ai_talap.h>

#define PID_HEATING   1
#define PID_COOLING  -1

double PID(
    double setpoint,
    double current,
    int manual_mode,
    double manual_value,
    double Kp,
    double Ks,
    double Kd,
    double dead_zone,
    double output_min,
    double output_max,
    double dt,
    double max_rate,
    double *integral,
    double *previous,
    double *last_output,
    int *initialized,
    int direction)
{
    double error;
    double proportional;
    double derivative;
    double integral_add;
    double integral_new;
    double output;
    double max_change;
    int rate_limited;

    /*
     * Нормализация направления:
     *
     * PID_HEATING  =  1
     * PID_COOLING  = -1
     */
    if (direction >= 0)
        direction = PID_HEATING;
    else
        direction = PID_COOLING;

    /*
     * Для нагрева:
     * setpoint > current -> error > 0.
     *
     * Для охлаждения:
     * current > setpoint -> error > 0.
     */
    error =
        (double)direction *
        (setpoint - current);

    if (dead_zone < 0.0)
        dead_zone = 0.2;

    if (dt <= 0.0)
        dt = 0.1;

    if (max_rate < 0.0)
        max_rate = -max_rate;

    if (!*initialized)
    {
        *integral = 0.0;
        *previous = current;
        *last_output = 0.0;
        *initialized = 1;
    }

    /* Ручной режим */
    if (manual_mode)
    {
        output = manual_value;

        if (output > output_max)
            output = output_max;

        if (output < output_min)
            output = output_min;

        /*
         * Подготовка интеграла для плавного
         * перехода в автоматический режим.
         */
        *integral =
            output -
            Kp * error;

        *previous = current;
        *last_output = output;

        return output;
    }

    /* Мёртвая зона */
    if (error >= -dead_zone &&
        error <= dead_zone)
    {
        *previous = current;
        return *last_output;
    }

    proportional =
        Kp * error;

    /*
     * Производная по измеренному значению.
     */
    derivative =
        -(double)direction *
        Kd *
        (current - *previous) /
        dt;

    /*
     * Интегральная составляющая
     * с учётом времени цикла.
     */
    integral_add =
        Ks *
        error *
        dt;

    integral_new =
        *integral +
        integral_add;

    output =
        proportional +
        integral_new +
        derivative;

    /* Anti-windup */
    if ((output >= output_min &&
         output <= output_max) ||

        (output > output_max &&
         integral_add < 0.0) ||

        (output < output_min &&
         integral_add > 0.0))
    {
        *integral = integral_new;
    }

    output =
        proportional +
        *integral +
        derivative;

    /* Ограничение диапазона */
    if (output > output_max)
        output = output_max;

    if (output < output_min)
        output = output_min;

    /*
     * Ограничение скорости изменения выхода.
     * max_rate задаётся в единицах выхода в секунду.
     */
    max_change =
        max_rate * dt;

    rate_limited = 0;

    if (max_change > 0.0)
    {
        if (output >
            *last_output + max_change)
        {
            output =
                *last_output + max_change;

            rate_limited = 1;
        }
        else if (output <
                 *last_output - max_change)
        {
            output =
                *last_output - max_change;

            rate_limited = 1;
        }
    }

    if (output > output_max)
        output = output_max;

    if (output < output_min)
        output = output_min;

    /*
     * Подстройка интеграла под фактический выход,
     * если сработало ограничение скорости.
     */
    if (rate_limited)
    {
        *integral =
            output -
            proportional -
            derivative;
    }

    *previous = current;
    *last_output = output;

    return output;
}

double Scale(
    double value,
    double input_min,
    double input_max,
    double output_min,
    double output_max)
{
    if (input_max == input_min)
        return output_min;

    return output_min +
           (value - input_min) *
           (output_max - output_min) /
           (input_max - input_min);
}

double manual_control_temp(
    int gr,
    int ls,
    double motor_speed,
    double step,
    int *previous_gr,
    int *previous_ls)
{
    int gr_pressed;
    int ls_pressed;

    gr_pressed = gr && !*previous_gr;
    ls_pressed = ls && !*previous_ls;

    *previous_gr = gr;
    *previous_ls = ls;

    if (step <= 0.0)
        return motor_speed;

    if (gr_pressed && !ls_pressed)
    {
        motor_speed = motor_speed + step;
        motor_speed = (int)(motor_speed / step + 0.5) * step;
    }
    else if (ls_pressed && !gr_pressed)
    {
        motor_speed = motor_speed - step;
        motor_speed = (int)(motor_speed / step + 0.5) * step;
    }

    if (motor_speed > 100.0)
        motor_speed = 100.0;

    if (motor_speed < 0.0)
        motor_speed = 0.0;

    return motor_speed;
}

  //Инициализация внутренних переменных
  static int previous_gr;
  static int previous_ls;
  static double motor_speed_manual;
  static double pid_integral;
  static double pid_previous;
  static double pid_output;
  static int pid_initialized;
  
  static int flag_alarm;
  
  static int fault; //do0
  static int normal; //do1
  static double temp; //temp
  static double motor_speed; //motor_speed
  static double motor_speed_ao;
  static double power_reostat; //ao3
  static double motor_speed_sen; //motor_speed_sen

void run() {
  //Инициализация постоянных
  const double ai_in_out_hi = 10000.01;
  const double ai_in_out_lo = 0.01;
  const double motor_speed_hi = 100.0;
  const double motor_speed_lo = 0.0;
  const double temp_limit = 0.05;
  const double step = 10.0;
  //Инициализация входных переменных
  int manual = lbvar("di0");
  int auto_mode = lbvar("di1");
  int greater = lbvar("di2");
  int less = lbvar("di3");
  int alarm_reset = lbvar("di4");
  double temp_non_scale = lbvar("ai0");
  double motor_speed_non_scale = lbvar("ai1");
  double hi_sensor = lbvar("hi_sensor");
  double lo_sensor = lbvar("lo_sensor");
  double temp_setpoint = lbvar("nom_val");
  double Kp = lbvar("Kp");
  double Ks = lbvar("Ks");
  double Kd = lbvar("Kd");
  

  //Инициализация выходных переменных

  
  //Нормирование входов
  
  temp = Scale(temp_non_scale, ai_in_out_lo, ai_in_out_hi, lo_sensor, hi_sensor);
  motor_speed_sen = Scale(motor_speed_non_scale, ai_in_out_lo, ai_in_out_hi, motor_speed_lo, motor_speed_hi);
  
  //Определение аварии температуры
  if (alarm_reset == 1) flag_alarm = 1;
  if ((temp <= temp_setpoint*(1 + temp_limit)) && (temp >= temp_setpoint*(1 - temp_limit))) {
    normal = 1;
    fault = 0;
  }
  else if ((temp >= temp_setpoint*(1 + temp_limit)) && (flag_alarm == 0)) {
    fault = 1;
    normal = 0;
    
  }
  else {
    fault = 0;
    normal = 0;
  
  }

  if (temp <= temp_setpoint*(1 + temp_limit)) flag_alarm = 0;
  
  //Управление температурой
  if ((manual !=0) || (auto_mode != 0)) {
    if (manual == 1) motor_speed_manual = manual_control_temp(greater, less, motor_speed, step, &previous_gr, &previous_ls);
    motor_speed = PID(temp_setpoint, temp, manual, motor_speed_manual, Kp, Ks, Kd, temp_limit, motor_speed_lo, motor_speed_hi, 0.1, 5,  &pid_integral, &pid_previous, &pid_output, &pid_initialized, PID_COOLING);
  
  }
  
  //Нормирование выходных переменных
  motor_speed_ao = Scale(motor_speed, motor_speed_lo, motor_speed_hi, ai_in_out_lo, ai_in_out_hi);
  
  //Запись выходных переменных
  lbvar_out("do0", fault);
  lbvar_out("do1", normal);
  lbvar_out("ao1", motor_speed_ao);
  lbvar_out("ao3", power_reostat);
  lbvar_out("temp", temp);
  lbvar_out("motor_speed", motor_speed);
  lbvar_out("motor_speed_sen", motor_speed_sen);
}

void init() {
  //lbint_init("di2", 0); 
  //lbint_init("do0", 0);
  previous_gr = 0;
  previous_ls = 0;
  motor_speed_manual = 0;
  pid_integral = 0;
  pid_previous = 0;
  pid_output = 0;
  pid_initialized = 0;
  flag_alarm = 0;
  
  fault  = 0; //do0
  normal = 0; //do1
  temp = 0.0; //temp
  motor_speed = 0.0; //motor_speed
  motor_speed_ao = 0;
  power_reostat = 10000; //ao3
  motor_speed_sen = 0.0; //motor_speed_sen
}
