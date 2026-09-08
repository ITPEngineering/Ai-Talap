#include <stdint.h>
#include <ai_talap.h>

float PID(
    float setpoint,
    float current,
    int manual_mode,
    float manual_value,
    float Kp,
    float Ks,
    float Kd,
    float dead_zone,
    float output_min,
    float output_max,
    float *integral,
    float *previous,
    float *last_output,
    int *initialized)
{
    float error;
    float integral_add;
    float integral_new;
    float output;

    error = setpoint - current;

    if (dead_zone < 0.0 || dead_zone > 1.0)
        dead_zone = 0.2;

    if (!*initialized)
    {
        *integral = 0.0;
        *previous = current;
        *last_output = 0.0;
        *initialized = 1;
    }

    if (manual_mode)
    {
        output = manual_value;

        if (output > output_max)
            output = output_max;

        if (output < output_min)
            output = output_min;

        *integral = output - Kp * error;
        *previous = current;
        *last_output = output;

        return output;
    }

    if (error >= -dead_zone && error <= dead_zone)
    {
        *previous = current;
        return *last_output;
    }

    integral_add = Ks * error;
    integral_new = *integral + integral_add;

    output =
        Kp * error +
        integral_new -
        Kd * (current - *previous);

    /*
     * Интеграл обновляется, если:
     * - выход находится внутри диапазона;
     * - либо интеграл выводит регулятор из ограничения.
     */
    if ((output >= output_min && output <= output_max) ||
        (output > output_max && integral_add < 0.0) ||
        (output < output_min && integral_add > 0.0))
    {
        *integral = integral_new;
    }

    output =
        Kp * error +
        *integral -
        Kd * (current - *previous);

    if (output > output_max)
        output = output_max;

    if (output < output_min)
        output = output_min;

    *previous = current;
    *last_output = output;

    return output;
}

float Scale(
    float value,
    float input_min,
    float input_max,
    float output_min,
    float output_max)
{
    if (input_max == input_min)
        return output_min;

    return output_min +
           (value - input_min) *
           (output_max - output_min) /
           (input_max - input_min);
}

float manual_control_temp(
    int gr,
    int ls,
    float motor_speed,
    float step,
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

void run() {
  //Инициализация постоянных
  const int ai_in_out_hi = 10000;
  const int ai_in_out_lo = 0;
  const float motor_speed_hi = 100.0;
  const float motor_speed_lo = 0.0;
  const float temp_limit = 0.05;
  const float step = 10.0;
  //Инициализация входных переменных
  int manual = lbvar("di0");
  int auto_mode = lbvar("di1");
  int greater = lbvar("di2");
  int less = lbvar("di3");
  int alarm_reset = lbvar("di0");
  float temp_non_scale = lbvar("ai0");
  float motor_speed_non_scale = lbvar("ai1");
  float hi_sensor = lbvar("hi_sensor");
  float lo_sensor = lbvar("lo_sensor");
  float temp_setpoint = lbvar("nom_val");
  float Kp = lbvar("Kp");
  float Ks = lbvar("Ks");
  float Kd = lbvar("Kd");
  
  //Инициализация внутренних переменных
  int previous_gr = 0;
  int previous_ls = 0;
  float motor_speed_manual = 0.0;
  float pid_integral = 0.0;
  float pid_previous = 0.0;
  float pid_output = 0.0;
  int pid_initialized = 0;
  
  int flag_alarm = 0;
  //Инициализация выходных переменных
  int fault  = 0; //do0
  int normal = 0; //do1
  float temp = 0.0; //temp
  float motor_speed = 0.0; //motor_speed
  float motor_speed_ao = 0;
  float power_reostat = 10000; //ao3
  float motor_speed_sen = 0.0; //motor_speed_sen
  
  //Нормирование входов
  
  temp = Scale(temp_non_scale, ai_in_out_lo, ai_in_out_hi, lo_sensor, hi_sensor);
  motor_speed_sen = Scale(motor_speed_non_scale, ai_in_out_lo, ai_in_out_hi, motor_speed_lo, motor_speed_hi);
  
  //Определение аварии температуры
  if (alarm_reset == 1) flag_alarm = 1;
  if ((temp <= temp_setpoint*(1 + temp_limit)) && (temp >= temp_setpoint*(1 - temp_limit))) {
    if (flag_alarm) {
      normal = 1;
    }
    else normal = 0;
  }
  else if (temp >= temp_setpoint*(1 + temp_limit)) {
    fault = 1;
    flag_alarm = 0;
  }
  else {
    fault = 0;
    normal = 0;
    flag_alarm = 0;
  }
  
  //Управление температурой
  if ((manual !=0) || (auto_mode != 0)) {
    if (manual == 1) motor_speed_manual = manual_control_temp(greater, less, motor_speed, step, &previous_gr, &previous_ls);
    motor_speed = PID(temp_setpoint, temp, manual, motor_speed_manual, Kp, Ks, Kd, temp_limit, motor_speed_lo, motor_speed_hi,  &pid_integral, &pid_previous, &pid_output, &pid_initialized);
  
  }
  
  //Нормирование выходных переменных
  motor_speed_ao = (int)Scale(motor_speed, motor_speed_lo, motor_speed_hi, ai_in_out_lo, ai_in_out_hi);
  
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

}
