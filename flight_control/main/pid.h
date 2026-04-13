//pid.h
//single-axis PID controller with integral anti-windup and output clamping

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float p;
    float i;
    float d;
    float integral;
    float prev_measurement;
    float prev_derivative;
    float integral_limit;   //caps |integral| to prevent windup
    float output_limit;     //caps |output| to the actuator range
    float d_filter_alpha;   //low-pass filter on D term (0–1, lower = smoother)
} pid_ctrl_t;

//zeros out the accumulated state (integral and previous error)
//call this when the controller is first enabled or after a mode change
void pid_reset(pid_ctrl_t *pid);

//runs one iteration of the PID loop
//setpoint: desired value (e.g. 0 degrees for level flight)
//measured: current sensor reading
//dt:       time since last call in seconds
//returns the control output, clamped to +/- output_limit
float pid_compute(pid_ctrl_t *pid, float setpoint, float measured, float dt);

#ifdef __cplusplus
}
#endif
