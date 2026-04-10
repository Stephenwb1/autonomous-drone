/*
 * quad_sim.c — desktop single-axis quadcopter PID simulation
 *
 * Compiles on macOS/Linux with:
 *   gcc -O2 -o quad_sim quad_sim.c ../main/pid.c -I../main -lm
 *
 * Outputs CSV to stdout.  Pipe to a file then open in Excel / Google Sheets,
 * or plot with:
 *   ./quad_sim > run.csv && python3 plot_sim.py run.csv
 *
 * The physics model is intentionally simple — one rotational axis with a
 * torque input from the PID and optional disturbance.  Good enough to
 * verify PID behaviour and rough-tune gains before real hardware.
 */

#include <stdio.h>
#include <math.h>
#include "pid.h"

/* ---------- tuneable parameters (edit these) ---------- */

#define SIM_DURATION_S   10.0f
#define DT               0.01f   /* 100 Hz, matches the firmware loop */

/* PID gains — start with the values from main.c, adjust here */
#define KP   1.5f
#define KI   0.0f
#define KD   0.3f

#define PID_OUTPUT_LIMIT   100.0f
#define PID_INTEGRAL_LIMIT  50.0f

/*
 * Plant model for a single rotational axis.
 *
 * MOMENT_OF_INERTIA: how hard the frame is to rotate (kg*m^2).
 *   For a ~60 g micro quad with 55 mm props, order of magnitude is
 *   1e-5 to 5e-5.  Start with 3e-5 and adjust if the sim response
 *   doesn't match real behaviour.
 *
 * TORQUE_PER_UNIT: how much torque one unit of PID output produces (N*m).
 *   This lumps together motor thrust, prop efficiency, and lever arm.
 *   For 8520 brushed motors at 3.7 V with 55 mm props the max thrust
 *   per motor is roughly 25-30 g (~0.27 N).  With a ~40 mm arm the
 *   torque per motor is ~0.011 N*m.  PID output range is ±100 and the
 *   differential between two motors on opposite arms spans that range,
 *   so torque_per_unit ≈ 2 * 0.011 / 100 ≈ 2.2e-4.
 */
#define MOMENT_OF_INERTIA  3.0e-5f
#define TORQUE_PER_UNIT    2.2e-4f

/* drag coefficient (friction proportional to angular velocity) */
#define DRAG_COEFF         1.0e-5f

/* initial tilt in degrees — simulates the drone starting tilted */
#define INITIAL_ANGLE_DEG  15.0f

/*
 * Step disturbance: at DISTURBANCE_TIME_S a sudden torque kicks the drone
 * (e.g. a gust of wind).  Set to 0 to disable.
 */
#define DISTURBANCE_TIME_S   5.0f
#define DISTURBANCE_TORQUE   5.0e-4f  /* N*m, roughly 50 g-force on one arm */
#define DISTURBANCE_DURATION 0.05f    /* 50 ms impulse */

/* ---------- simulation ---------- */

int main(void)
{
    pid_ctrl_t pid = {
        .p = KP,
        .i = KI,
        .d = KD,
        .integral_limit = PID_INTEGRAL_LIMIT,
        .output_limit   = PID_OUTPUT_LIMIT,
    };
    pid_reset(&pid);

    float angle     = INITIAL_ANGLE_DEG;   /* degrees */
    float ang_vel   = 0.0f;                /* degrees / second */
    float setpoint  = 0.0f;                /* target: level */

    printf("time_s,angle_deg,angular_vel_dps,pid_output,setpoint\n");

    for (float t = 0.0f; t < SIM_DURATION_S; t += DT) {

        float output = pid_compute(&pid, setpoint, angle, DT);

        /* convert PID output to physical torque */
        float torque = output * TORQUE_PER_UNIT;

        /* optional disturbance impulse */
        if (t >= DISTURBANCE_TIME_S &&
            t <  DISTURBANCE_TIME_S + DISTURBANCE_DURATION) {
            torque += DISTURBANCE_TORQUE;
        }

        /* rotational dynamics: τ = I·α,  drag ∝ ω */
        float drag   = -DRAG_COEFF * ang_vel;
        float alpha   = (torque + drag) / MOMENT_OF_INERTIA;  /* deg/s^2 via torque in N*m... */

        /*
         * The torque is in N*m and inertia in kg*m^2, so alpha is in rad/s^2.
         * Convert to deg/s^2 so angle stays in degrees (matching the PID).
         */
        alpha *= (180.0f / (float)M_PI);

        ang_vel += alpha * DT;
        angle   += ang_vel * DT;

        printf("%.4f,%.4f,%.4f,%.4f,%.4f\n",
               t, angle, ang_vel, output, setpoint);
    }

    return 0;
}
