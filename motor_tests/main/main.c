#include "motor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "motor_test";

/*
 * Set this to 1 to write the spin-direction settings below to each ESC's
 * flash. Flash, run once, watch the serial log for the four
 * "Motor N: setting direction = ..." lines, power-cycle, then set this
 * back to 0 and re-flash so we don't keep rewriting ESC flash on every boot.
 */
#define RUN_DIRECTION_SETUP 1

/*
 * Fill these four in based on what you observed in the identification test.
 * `true`  = ESC will be told to spin REVERSED from its current direction.
 * `false` = ESC will be told to spin NORMAL.
 *
 * Tip: if a motor was already spinning the wrong way for the convention you
 *      want, set its entry below. If it was already correct, leave it false.
 */
#define MOTOR_1_REVERSED   false
#define MOTOR_2_REVERSED   true
#define MOTOR_3_REVERSED   false
#define MOTOR_4_REVERSED   true

static void run_for(int duration_us)
{
    int64_t end = esp_timer_get_time() + duration_us;
    while (esp_timer_get_time() < end)
        motors_tick();
}

void app_main(void)
{
    motors_init();

#if RUN_DIRECTION_SETUP
    ESP_LOGW(TAG, "RUN_DIRECTION_SETUP=1 — writing spin direction to ESCs.");
    ESP_LOGW(TAG, "After this boot completes, set RUN_DIRECTION_SETUP back to 0.");

    motor_set_direction(MOTOR_1, MOTOR_1_REVERSED);
    motor_set_direction(MOTOR_2, MOTOR_2_REVERSED);
    motor_set_direction(MOTOR_3, MOTOR_3_REVERSED);
    motor_set_direction(MOTOR_4, MOTOR_4_REVERSED);

    ESP_LOGW(TAG, "Direction setup done. Power-cycle the ESCs, then re-flash with RUN_DIRECTION_SETUP=0.");
    while (1) {
        motors_tick();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
#endif

    /* Identification mode: spin one motor at a time at 10% so you can see
     * which physical motor is which logical index, and which way it spins.
     * Once you've recorded that info, replace this loop with the throttle
     * ramp (or set RUN_DIRECTION_SETUP=1 to write directions to ESC flash). */
    const char *names[4] = {"MOTOR_1 (GPIO3)",
                            "MOTOR_2 (GPIO4)",
                            "MOTOR_3 (GPIO5)",
                            "MOTOR_4 (GPIO6)"};

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            ESP_LOGI(TAG, ">>> %s spinning at 10%% — note position and direction (CW/CCW from above)",
                     names[i]);

            motors_stop_all();
            motor_set_throttle((motor_t)i, 10);
            run_for(3000000);

            motors_stop_all();
            run_for(1000000);
        }

        ESP_LOGI(TAG, "--- end of pass, repeating ---");
    }
}
