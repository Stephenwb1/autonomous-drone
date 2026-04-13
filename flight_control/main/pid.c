//pid.c
//PID controller implementation

#include "pid.h"

//clamp a value to +/- limit
static float clampf(float val, float limit)
{
    if (val >  limit) return  limit;
    if (val < -limit) return -limit;
    return val;
}

void pid_reset(pid_ctrl_t *pid)
{
    pid->integral         = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->prev_derivative  = 0.0f;
}

float pid_compute(pid_ctrl_t *pid, float setpoint, float measured, float dt)
{
    if (dt <= 0.0f) return 0.0f;

    float error = setpoint - measured;

    pid->integral += error * dt;
    pid->integral = clampf(pid->integral, pid->integral_limit);

    // derivative on measurement (not error) to avoid derivative kick
    float raw_derivative = -(measured - pid->prev_measurement) / dt;
    pid->prev_measurement = measured;

    // low-pass filter: smooths out noise in the derivative
    float alpha = pid->d_filter_alpha;
    pid->prev_derivative = alpha * raw_derivative
                         + (1.0f - alpha) * pid->prev_derivative;

    float output = pid->p * error
                 + pid->i * pid->integral
                 + pid->d * pid->prev_derivative;

    return clampf(output, pid->output_limit);
}
