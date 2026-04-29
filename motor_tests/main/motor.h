#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MOTOR_1 = 0,
    MOTOR_2 = 1,
    MOTOR_3 = 2,
    MOTOR_4 = 3
} motor_t;

void motors_init(void);
void motor_set_throttle(motor_t motor, int throttle_pct);
void motors_stop_all(void);
void motors_tick(void);

/*
 * Configure ESC spin direction for a single motor and persist it.
 *
 * Must be called AFTER motors_init() (so the ESC is armed) and BEFORE
 * the main throttle loop. Sends DSHOT special commands 20/21 (direction)
 * followed by 12 (save settings), so the new direction survives a power
 * cycle. Other motors keep getting disarm frames during the burst.
 *
 * reversed = false -> SPIN_DIRECTION_NORMAL   (cmd 20)
 * reversed = true  -> SPIN_DIRECTION_REVERSED (cmd 21)
 */
void motor_set_direction(motor_t motor, bool reversed);
